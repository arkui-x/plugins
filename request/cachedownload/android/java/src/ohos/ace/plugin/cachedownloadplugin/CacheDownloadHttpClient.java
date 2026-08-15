/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package ohos.ace.plugin.cachedownloadplugin;

import java.io.IOException;
import java.io.InputStream;
import java.io.FileInputStream;
import java.net.HttpURLConnection;
import java.net.InetAddress;
import java.net.URL;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

/**
 * HTTP client used by CacheDownloadService.
 *
 * <p>Uses standard HttpURLConnection so no extra dependencies are required.
 * Requests run on a background thread and deliver data through JNI callbacks.</p>
 *
 * <p>Error classification mirrors the iOS implementation (MapNSErrorToErrorCode):
 *   UnknownHostException -> DNS  (0x00)
 *   ConnectException     -> TCP  (0x10)
 *   SocketTimeoutException -> TCP (0x10)
 *   SSLException         -> SSL  (0x20)
 *   other                -> OTHERS (0xFF)
 * HTTP status errors are reported with errorCode = HTTP status and message = status string.</p>
 *
 * @since 1.0.0
 */
public class CacheDownloadHttpClient {
    private static final String TAG = "CDHttpClient";

    /** DNS error code, matches ErrorCode::DNS in cache_error.h. */
    private static final int ERROR_DNS = 0x00;

    /** TCP error code, matches ErrorCode::TCP in cache_error.h. */
    private static final int ERROR_TCP = 0x10;

    /** SSL error code, matches ErrorCode::SSL in cache_error.h. */
    private static final int ERROR_SSL = 0x20;

    /** OTHERS error code, matches ErrorCode::OTHERS in cache_error.h. */
    private static final int ERROR_OTHERS = 0xFF;

    /** Max concurrent download threads — bounded to prevent unbounded thread
     *  spawn during batch pre-download. 4x CPU cores, minimum 4. */
    private static final int MAX_CONCURRENT =
        Math.max(4, Runtime.getRuntime().availableProcessors() * 4);

    private final ExecutorService executor = new ThreadPoolExecutor(
        0, MAX_CONCURRENT,
        60L, TimeUnit.SECONDS,
        new SynchronousQueue<Runnable>());

    /**
     * Executes an HTTP request asynchronously.
     *
     * @param callbackPtr Native pointer to the C++ request context.
     * @param url Request URL.
     * @param config Request configuration (method, headers, timeouts, CA path).
     */
    public void executeRequest(long callbackPtr, String url, RequestConfig config) {
        executor.submit(() -> runRequest(callbackPtr, url, config));
    }

    private void runRequest(long callbackPtr, String url, RequestConfig config) {
        HttpURLConnection connection = null;
        long startTime = System.currentTimeMillis();
        String serverIp = "";

        try {
            URL requestUrl = new URL(url);
            serverIp = resolveServerIp(requestUrl.getHost());
            connection = setupConnection(requestUrl, config);

            int responseCode = connection.getResponseCode();
            long firstRecvTime = System.currentTimeMillis() - startTime;

            processResponse(callbackPtr, connection, responseCode);
            long totalTime = System.currentTimeMillis() - startTime;
            onMetrics(callbackPtr, serverIp, firstRecvTime, totalTime);
            reportResult(callbackPtr, responseCode);
        } catch (java.net.ConnectException e) {
            onFailed(callbackPtr, ERROR_TCP, safeMessage(e, "Connection failed"));
        } catch (java.net.SocketTimeoutException e) {
            onFailed(callbackPtr, ERROR_TCP, safeMessage(e, "Timeout"));
        } catch (java.net.UnknownHostException e) {
            onFailed(callbackPtr, ERROR_DNS, safeMessage(e, "Unknown host"));
        } catch (javax.net.ssl.SSLException e) {
            onFailed(callbackPtr, ERROR_SSL, safeMessage(e, "SSL error"));
        } catch (IOException | RuntimeException e) {
            onFailed(callbackPtr, ERROR_OTHERS, safeMessage(e, "Unknown error"));
        } finally {
            if (connection != null) {
                connection.disconnect();
            }
        }
    }

    private String resolveServerIp(String host) {
        try {
            return InetAddress.getByName(host).getHostAddress();
        } catch (IOException | RuntimeException ignored) {
            return "";
        }
    }

    private HttpURLConnection setupConnection(URL requestUrl, RequestConfig config) throws IOException {
        HttpURLConnection connection;
        java.net.URLConnection conn = requestUrl.openConnection();
        if (conn instanceof HttpURLConnection) {
            connection = (HttpURLConnection) conn;
        } else {
            throw new IOException("URL does not support HTTP: " + requestUrl);
        }
        connection.setRequestMethod(config.method);
        connection.setConnectTimeout(config.connectTimeoutMs);
        connection.setReadTimeout(config.readTimeoutMs);
        connection.setDoInput(true);
        if (config.headers != null) {
            for (Map.Entry<String, String> entry : config.headers.entrySet()) {
                connection.setRequestProperty(entry.getKey(), entry.getValue());
            }
        }
        // Apply custom CA certificate for HTTPS connections when caPath is set.
        // Without this, self-signed / private CA sites fail the TLS handshake
        // here (parity with iOS custom TLS verification).
        if (config.caPath != null && !config.caPath.isEmpty() && connection instanceof HttpsURLConnection) {
            try {
                setupCustomSsl((HttpsURLConnection) connection, config.caPath);
            } catch (IOException e) {
                throw e;
            } catch (Exception e) {
                throw new IOException("Failed to set up custom SSL: " + e.getMessage(), e);
            }
        }
        return connection;
    }

    /**
     * Configures a custom TrustManager on an HTTPS connection using the CA
     * certificate at {@code caPath}. The system default TrustManager is also
     * retained so that publicly-trusted CAs still work alongside the custom CA.
     *
     * @param httpsConn The HTTPS connection to configure.
     * @param caPath Filesystem path to the PEM-encoded CA certificate.
     * @throws Exception If the CA cannot be read or the SSLContext cannot be initialized.
     */
    private void setupCustomSsl(HttpsURLConnection httpsConn, String caPath) throws Exception {
        FileInputStream fis = new FileInputStream(caPath);
        X509Certificate caCert;
        try {
            CertificateFactory cf = CertificateFactory.getInstance("X.509");
            java.security.cert.Certificate genCert = cf.generateCertificate(fis);
            if (!(genCert instanceof X509Certificate)) {
                throw new java.security.cert.CertificateException("CA certificate is not X.509 format");
            }
            caCert = (X509Certificate) genCert;
        } finally {
            fis.close();
        }

        // TrustManager that accepts the custom CA in addition to the system's
        // trusted CAs. A dedicated X509TrustManager wrapping the system default
        // is used so that standard CA validation still applies.
        TrustManager[] trustManagers = {
            new CustomCaTrustManager(caCert)
        };

        SSLContext sslContext = SSLContext.getInstance("TLS");
        sslContext.init(null, trustManagers, new java.security.SecureRandom());
        httpsConn.setSSLSocketFactory(sslContext.getSocketFactory());
    }

    /**
     * TrustManager that trusts a custom CA certificate in addition to the
     * system-default trusted CAs. This preserves standard CA validation while
     * allowing self-signed / private CA connections.
     */
    private static final class CustomCaTrustManager implements X509TrustManager {
        private final X509TrustManager systemDefault;
        private final X509Certificate customCa;

        CustomCaTrustManager(X509Certificate customCa) throws Exception {
            this.customCa = customCa;
            // Obtain the system-default TrustManager for standard CA validation.
            javax.net.ssl.TrustManagerFactory tmf =
                javax.net.ssl.TrustManagerFactory.getInstance(
                    javax.net.ssl.TrustManagerFactory.getDefaultAlgorithm());
            java.security.KeyStore nullKeyStore = null;
            tmf.init(nullKeyStore);
            X509TrustManager tm = null;
            for (TrustManager m : tmf.getTrustManagers()) {
                if (m instanceof X509TrustManager) {
                    tm = (X509TrustManager) m;
                    break;
                }
            }
            this.systemDefault = (tm != null) ? tm : new NoopTrustManager();
        }

        @Override
        public void checkClientTrusted(X509Certificate[] chain, String authType)
            throws java.security.cert.CertificateException {
            systemDefault.checkClientTrusted(chain, authType);
        }

        @Override
        public void checkServerTrusted(X509Certificate[] chain, String authType)
            throws java.security.cert.CertificateException {
            try {
                systemDefault.checkServerTrusted(chain, authType);
            } catch (java.security.cert.CertificateException systemEx) {
                // System validation failed — try the custom CA.
                for (X509Certificate cert : chain) {
                    try {
                        cert.verify(customCa.getPublicKey());
                        return; // Verified by custom CA
                    } catch (java.security.cert.CertificateException
                             | java.security.NoSuchAlgorithmException
                             | java.security.InvalidKeyException
                             | java.security.SignatureException
                             | java.security.NoSuchProviderException ignored) {
                        // try next cert in chain
                    }
                }
                // Direct match: is any cert in the chain the custom CA itself?
                for (X509Certificate cert : chain) {
                    if (cert.equals(customCa)) {
                        return;
                    }
                }
                throw systemEx; // Neither system nor custom CA trusts this server
            }
        }

        @Override
        public X509Certificate[] getAcceptedIssuers() {
            return systemDefault.getAcceptedIssuers();
        }
    }

    /** Fallback TrustManager when the system default cannot be obtained. */
    private static final class NoopTrustManager implements X509TrustManager {
        @Override
        public void checkClientTrusted(X509Certificate[] chain, String authType) {}

        @Override
        public void checkServerTrusted(X509Certificate[] chain, String authType) {}

        @Override
        public X509Certificate[] getAcceptedIssuers() {
            return new X509Certificate[0];
        }
    }

    private void processResponse(long callbackPtr, HttpURLConnection connection,
                                 int responseCode) throws IOException {
        InputStream inputStream = (responseCode >= 200 && responseCode < 300)
            ? connection.getInputStream()
            : connection.getErrorStream();

        if (inputStream == null) {
            return;
        }

        try {
            byte[] buffer = new byte[8192];
            int bytesRead;
            long downloaded = 0L;
            long total = parseContentLength(connection.getHeaderField("Content-Length"));

            while ((bytesRead = inputStream.read(buffer)) != -1) {
                if (bytesRead > 0) {
                    byte[] chunk = new byte[bytesRead];
                    System.arraycopy(buffer, 0, chunk, 0, bytesRead);
                    onDataChunk(callbackPtr, chunk);
                    downloaded += bytesRead;
                    onProgress(callbackPtr, downloaded, total);
                }
            }
        } finally {
            inputStream.close();
        }

        for (Map.Entry<String, List<String>> entry : connection.getHeaderFields().entrySet()) {
            if (entry.getKey() != null && entry.getValue() != null && !entry.getValue().isEmpty()) {
                onHeader(callbackPtr, entry.getKey(), entry.getValue().get(0));
            }
        }
    }

    private void reportResult(long callbackPtr, int responseCode) {
        if (responseCode >= 200 && responseCode < 300) {
            onSuccess(callbackPtr, responseCode);
        } else {
            onFailed(callbackPtr, responseCode, String.valueOf(responseCode));
        }
    }

    private long parseContentLength(String contentLength) {
        try {
            return Long.parseLong(contentLength);
        } catch (NumberFormatException ignored) {
            return -1L;
        }
    }

    private String safeMessage(Exception e, String fallback) {
        return e.getMessage() != null ? e.getMessage() : fallback;
    }

    /**
     * Called when a data chunk is received.
     *
     * @param callbackPtr Native pointer to the C++ request context.
     * @param data Byte array chunk received from the server.
     */
    public native void onDataChunk(long callbackPtr, byte[] data);

    /**
     * Called when a response header is received.
     *
     * @param callbackPtr Native pointer to the C++ request context.
     * @param key Header name.
     * @param value Header value.
     */
    public native void onHeader(long callbackPtr, String key, String value);

    /**
     * Called when download progress updates.
     *
     * @param callbackPtr Native pointer to the C++ request context.
     * @param downloaded Bytes downloaded so far.
     * @param total Total bytes expected, or -1 if unknown.
     */
    public native void onProgress(long callbackPtr, long downloaded, long total);

    /**
     * Called when the request completes successfully.
     *
     * @param callbackPtr Native pointer to the C++ request context.
     * @param statusCode HTTP status code (2xx).
     */
    public native void onSuccess(long callbackPtr, int statusCode);

    /**
     * Called when the request fails.
     *
     * @param callbackPtr Native pointer to the C++ request context.
     * @param errorCode Error code (DNS/TCP/SSL/HTTP/OTHERS).
     * @param message Error message string.
     */
    public native void onFailed(long callbackPtr, int errorCode, String message);

    /**
     * Called with request metrics upon completion.
     *
     * @param callbackPtr Native pointer to the C++ request context.
     * @param serverIp Resolved server IP address.
     * @param firstRecvTimeMs Time to first response byte in milliseconds.
     * @param totalTimeMs Total request time in milliseconds.
     */
    public native void onMetrics(long callbackPtr, String serverIp, long firstRecvTimeMs, long totalTimeMs);

    /**
     * Bundles per-request configuration to keep method parameter lists compact.
     */
    public static final class RequestConfig {
        /** HTTP method (e.g. "GET", "POST"). */
        public final String method;

        /** Request headers map, or null for no headers. */
        public final Map<String, String> headers;

        /** Connect timeout in milliseconds. */
        public final int connectTimeoutMs;

        /** Read timeout in milliseconds. */
        public final int readTimeoutMs;

        /** CA certificate path for TLS, or null. */
        public final String caPath;

        public RequestConfig(String method, Map<String, String> headers,
                             int connectTimeoutMs, int readTimeoutMs, String caPath) {
            this.method = method;
            this.headers = headers;
            this.connectTimeoutMs = connectTimeoutMs;
            this.readTimeoutMs = readTimeoutMs;
            this.caPath = caPath;
        }
    }
}
