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
import java.util.HashMap;
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
 *   SocketTimeoutException -> OTHERS (0xFF)
 *   SSLException         -> SSL  (0x20)
 *   other                -> OTHERS (0xFF)
 * Timeout maps to OTHERS to match OHOS netstack, where a total/read timeout
 * (CURLE_OPERATION_TIMEDOUT) is classified as ErrorKind::Others, not TCP.
 * HTTP status errors are reported with errorCode = HTTP status and message = status string.</p>
 *
 * <p>Performance timings follow the OHOS curl semantics: every value is the
 * cumulative milliseconds elapsed since request start (not per-stage cost).
 * HttpURLConnection cannot separate TCP connect from TLS handshake, so for
 * HTTPS both connectTime and tlsTime are the connect() completion timestamp
 * (tlsTime therefore includes DNS + TCP + TLS, approximating
 * CURLINFO_APPCONNECT_TIME_T); firstSendTime also uses that timestamp,
 * approximating CURLINFO_PRETRANSFER_TIME_T.</p>
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

    /** Too-many-redirects error code, matches OHOS
     *  DownloadErrorCode::ERROR_TOO_MANY_REDIRECTS (6). */
    private static final int ERROR_TOO_MANY_REDIRECTS = 6;

    /** Max concurrent download threads — bounded to prevent unbounded thread
     *  spawn during batch pre-download. 4x CPU cores, minimum 4. */
    private static final int MAX_CONCURRENT =
        Math.max(4, Runtime.getRuntime().availableProcessors() * 4);

    /** Max redirect hops to follow manually — 30 matches the libcurl default
     *  used by the OHOS netstack backend. */
    private static final int MAX_REDIRECTS = 30;

    private final ExecutorService executor = new ThreadPoolExecutor(
        0, MAX_CONCURRENT,
        60L, TimeUnit.SECONDS,
        new SynchronousQueue<Runnable>());

    /**
     * Returns monotonic milliseconds for elapsed-time measurement, immune to
     * NTP adjustments (matching curl/iOS timing).
     *
     * @return Milliseconds on a monotonic clock; only differences are
     *         meaningful.
     */
    private static long elapsedMs() {
        return System.nanoTime() / 1_000_000L;
    }

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
        // Monotonic clock (immune to NTP adjustments), matching curl/iOS timing.
        TimingState timing = new TimingState(elapsedMs());
        try {
            URL requestUrl = new URL(url);
            timing.serverIp = resolveServerIp(requestUrl.getHost());
            // Cumulative since request start (curl NAMELOOKUP semantics).
            timing.dnsTimeMs = elapsedMs() - timing.startMs;
            connection = setupConnection(requestUrl, config);
            connection.connect();
            // Cumulative since request start (curl CONNECT semantics). For
            // HTTPS, connect() also completes the TLS handshake, so TCP and
            // TLS cannot be separated — both timestamps share this value.
            timing.connectTimeMs = elapsedMs() - timing.startMs;
            if (connection instanceof HttpsURLConnection) {
                timing.tlsTimeMs = timing.connectTimeMs;
            }
            // Approximates curl PRETRANSFER: the request starts going out as
            // soon as the connection is ready.
            timing.firstSendTimeMs = timing.connectTimeMs;
            int responseCode = connection.getResponseCode();

            RedirectResult result = followRedirects(requestUrl, connection, responseCode,
                config, timing);
            connection = result.connection;

            if (result.tooManyRedirects) {
                // The final response of the redirect chain was still a 3xx:
                // report metrics, then fail without retrying (A->B->A loops).
                timing.firstRecvTimeMs = elapsedMs() - timing.startMs;
                reportMetrics(callbackPtr, timing);
                onFailed(callbackPtr, ERROR_TOO_MANY_REDIRECTS, "Too many redirects");
                return;
            }

            // Cumulative since request start to the first byte of the FINAL
            // response (curl STARTTRANSFER semantics — after redirects).
            timing.firstRecvTimeMs = elapsedMs() - timing.startMs;
            processResponse(callbackPtr, connection, result.responseCode);
            reportMetrics(callbackPtr, timing);
            reportResult(callbackPtr, result.responseCode);
        } catch (java.net.ConnectException e) {
            reportFailure(callbackPtr, ERROR_TCP, safeMessage(e, "Connection failed"), timing);
        } catch (java.net.SocketTimeoutException e) {
            // Read/total timeout maps to OTHERS, matching OHOS netstack where
            // CURLE_OPERATION_TIMEDOUT is ErrorKind::Others, not TCP.
            reportFailure(callbackPtr, ERROR_OTHERS, safeMessage(e, "Timeout"), timing);
        } catch (java.net.UnknownHostException e) {
            reportFailure(callbackPtr, ERROR_DNS, safeMessage(e, "Unknown host"), timing);
        } catch (javax.net.ssl.SSLException e) {
            reportFailure(callbackPtr, ERROR_SSL, safeMessage(e, "SSL error"), timing);
        } catch (IOException | RuntimeException e) {
            reportFailure(callbackPtr, ERROR_OTHERS, safeMessage(e, "Unknown error"), timing);
        } finally {
            if (connection != null) {
                connection.disconnect();
            }
        }
    }

    /**
     * Follows the redirect chain of a request whose first hop already
     * completed. Each hop re-issues the request manually (redirects are not
     * auto-followed) so per-hop timings can be captured, and sensitive
     * headers are stripped once the chain leaves the original origin.
     *
     * @param requestUrl URL of the completed first hop; also the redirect
     *                   chain origin for cross-origin header checks.
     * @param connection Connection holding the current (3xx) response.
     * @param statusCode Status code of the current response.
     * @param config Request configuration.
     * @param timing Mutable per-request timing state, updated per hop.
     * @return The final connection, status code and redirect-limit flag.
     * @throws IOException If a redirect hop fails at the network layer.
     */
    private RedirectResult followRedirects(URL requestUrl, HttpURLConnection connection,
        int statusCode, RequestConfig config, TimingState timing) throws IOException {
        URL currentUrl = requestUrl;
        HttpURLConnection current = connection;
        int responseCode = statusCode;
        int redirectCount = 0;
        // Set when the loop exits because a 3xx response carried no Location
        // header (curl semantics: that response is delivered as the final
        // one), so the limit check below does not misfire at hop 30.
        boolean missingLocation = false;
        while (isRedirect(responseCode) && redirectCount < MAX_REDIRECTS) {
            String location = current.getHeaderField("Location");
            if (location == null || location.isEmpty()) {
                missingLocation = true;
                break;
            }
            current.disconnect();
            URL redirectUrl = new URL(currentUrl, location);
            current = openRedirectHop(redirectUrl, requestUrl, responseCode, config, timing);
            redirectCount++;
            currentUrl = redirectUrl;
            refreshHopMetrics(current, redirectUrl, timing);
            responseCode = current.getResponseCode();
        }
        if (redirectCount > 0) {
            // Cumulative since request start until the final redirect request
            // was issued (curl REDIRECT_TIME_T semantics). Stays 0 without
            // redirects.
            timing.redirectTimeMs = timing.lastRedirectStartMs;
        }
        // Only the limit itself (not a Location-less 3xx break) turns the
        // still-3xx tail into ERROR_TOO_MANY_REDIRECTS.
        boolean tooManyRedirects = !missingLocation && isRedirect(responseCode)
            && redirectCount >= MAX_REDIRECTS;
        return new RedirectResult(current, responseCode, tooManyRedirects);
    }

    /**
     * Opens the connection for one redirect hop, stripping sensitive headers
     * (Authorization/Proxy-Authorization/Cookie) when the hop leaves the
     * original origin (cross-host or https->http downgrade), so credentials
     * cannot leak to another origin. The caller's config map is untouched.
     *
     * @param redirectUrl Target URL of this hop.
     * @param originalUrl Chain origin for cross-origin checks.
     * @param statusCode Status code of the redirect response (303 forces GET).
     * @param config Request configuration.
     * @param timing Timing state; records when this hop request was issued.
     * @return The connected hop connection.
     * @throws IOException If the hop fails at the network layer.
     */
    private HttpURLConnection openRedirectHop(URL redirectUrl, URL originalUrl, int statusCode,
        RequestConfig config, TimingState timing) throws IOException {
        Map<String, String> hopHeaders = filterSensitiveHeaders(redirectUrl, originalUrl,
            config.headers);
        HttpURLConnection hopConnection = setupConnection(redirectUrl, config, hopHeaders);
        if (statusCode == 303) {
            // 303 See Other: the redirect target must be fetched with GET
            // per HTTP semantics.
            hopConnection.setRequestMethod("GET");
        }
        // Elapsed when this (possibly final) redirect request was issued.
        timing.lastRedirectStartMs = elapsedMs() - timing.startMs;
        try {
            hopConnection.connect();
        } catch (IOException | RuntimeException e) {
            // Do not abandon a half-open socket: the caller's finally only
            // sees the previous, already-disconnected connection.
            hopConnection.disconnect();
            throw e;
        }
        return hopConnection;
    }

    /**
     * Copies the header map without credentials when the redirect leaves the
     * original origin; otherwise returns the original map. A null host
     * (e.g. "Location: file:///x") is treated as cross-host.
     *
     * @param redirectUrl Target URL of this hop.
     * @param originalUrl Chain origin for cross-origin checks.
     * @param headers Original request headers, may be null.
     * @return Headers safe to send to the redirect target.
     */
    private Map<String, String> filterSensitiveHeaders(URL redirectUrl, URL originalUrl,
        Map<String, String> headers) {
        if (headers == null || !leavesOrigin(redirectUrl, originalUrl)) {
            return headers;
        }
        Map<String, String> filtered = new HashMap<String, String>();
        for (Map.Entry<String, String> entry : headers.entrySet()) {
            String name = entry.getKey();
            if (!"Authorization".equalsIgnoreCase(name)
                && !"Proxy-Authorization".equalsIgnoreCase(name)
                && !"Cookie".equalsIgnoreCase(name)) {
                filtered.put(name, entry.getValue());
            }
        }
        return filtered;
    }

    /**
     * Returns true when the redirect hop targets a different host or
     * downgrades HTTPS to HTTP.
     *
     * @param redirectUrl Target URL of this hop.
     * @param originalUrl Chain origin for cross-origin checks.
     * @return True if the hop crosses to another origin.
     */
    private boolean leavesOrigin(URL redirectUrl, URL originalUrl) {
        String redirectHost = redirectUrl.getHost();
        String originalHost = originalUrl.getHost();
        // A null host cannot be the original origin, so treat it as cross-host.
        boolean crossHost = redirectHost == null || originalHost == null
            || !redirectHost.equals(originalHost);
        boolean schemeDowngrade = "https".equalsIgnoreCase(originalUrl.getProtocol())
            && "http".equalsIgnoreCase(redirectUrl.getProtocol());
        return crossHost || schemeDowngrade;
    }

    /**
     * Refreshes the cumulative timing state after a redirect hop connected,
     * so the metrics reflect the final hop's stages.
     *
     * @param hopConnection The just-connected hop connection.
     * @param redirectUrl Target URL of this hop.
     * @param timing Mutable per-request timing state.
     */
    private void refreshHopMetrics(HttpURLConnection hopConnection, URL redirectUrl,
        TimingState timing) {
        // Refresh the per-hop metrics to this hop's cumulative values; if
        // another redirect follows, they are simply overwritten again.
        timing.connectTimeMs = elapsedMs() - timing.startMs;
        if (hopConnection instanceof HttpsURLConnection) {
            timing.tlsTimeMs = timing.connectTimeMs;
        } else {
            // Plain-HTTP hop: no TLS handshake on that connection.
            timing.tlsTimeMs = 0L;
        }
        timing.firstSendTimeMs = timing.connectTimeMs;
        timing.serverIp = resolveServerIp(redirectUrl.getHost());
    }

    /**
     * Emits the collected request metrics with a freshly computed total time.
     *
     * @param callbackPtr Native pointer to the C++ request context.
     * @param timing Timing state collected for this request.
     */
    private void reportMetrics(long callbackPtr, TimingState timing) {
        long totalTimeMs = elapsedMs() - timing.startMs;
        RequestMetrics metrics = new RequestMetrics(timing.serverIp, timing.dnsTimeMs,
            timing.connectTimeMs, timing.tlsTimeMs, timing.firstSendTimeMs,
            timing.firstRecvTimeMs, totalTimeMs, timing.redirectTimeMs);
        onMetrics(callbackPtr, metrics);
    }

    /**
     * Reports the collected metrics before the failure callback. Order
     * matters: the native side tears the request context down in onFailed,
     * so metrics emitted after it would be dropped.
     *
     * @param callbackPtr Native pointer to the C++ request context.
     * @param errorCode Error code (DNS/TCP/SSL/TOO_MANY_REDIRECTS/OTHERS).
     * @param message Error message string.
     * @param timing Timing state collected for this request.
     */
    private void reportFailure(long callbackPtr, int errorCode, String message, TimingState timing) {
        reportMetrics(callbackPtr, timing);
        onFailed(callbackPtr, errorCode, message);
    }

    private String resolveServerIp(String host) {
        try {
            return InetAddress.getByName(host).getHostAddress();
        } catch (IOException | RuntimeException ignored) {
            return "";
        }
    }

    private HttpURLConnection setupConnection(URL requestUrl, RequestConfig config) throws IOException {
        return setupConnection(requestUrl, config, config.headers);
    }

    private HttpURLConnection setupConnection(URL requestUrl, RequestConfig config,
        Map<String, String> headers) throws IOException {
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
        // Follow redirects manually in runRequest so each hop can be timed
        // (redirectTime would be unavailable with auto-follow).
        connection.setInstanceFollowRedirects(false);
        connection.setDoInput(true);
        if (headers != null) {
            for (Map.Entry<String, String> entry : headers.entrySet()) {
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

    private boolean isRedirect(int responseCode) {
        return responseCode == 301 || responseCode == 302 || responseCode == 303
            || responseCode == 307 || responseCode == 308;
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
     * Called with request metrics upon completion. All timings are cumulative
     * milliseconds elapsed since request start (OHOS curl semantics), not
     * per-stage costs.
     *
     * @param callbackPtr Native pointer to the C++ request context.
     * @param metrics Collected metrics for the finished request.
     */
    public native void onMetrics(long callbackPtr, RequestMetrics metrics);

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

    /**
     * Bundles the finished request's metrics for the onMetrics callback. All
     * timings are cumulative milliseconds elapsed since request start (OHOS
     * curl semantics), not per-stage costs. Immutable snapshot: the native
     * side reads the fields via JNI, so they must not be reassigned after
     * construction.
     */
    public static final class RequestMetrics {
        /** Resolved server IP address, or "". */
        private final String serverIp;

        /** Start -> DNS resolution complete (first hop only). */
        private final long dnsTimeMs;

        /** Start -> TCP connect complete (for HTTPS this is the same
         *  timestamp as tlsTimeMs — see class javadoc). */
        private final long connectTimeMs;

        /** Start -> TLS handshake complete (0 for plain HTTP). */
        private final long tlsTimeMs;

        /** Start -> request about to be sent. */
        private final long firstSendTimeMs;

        /** Start -> first response byte received. */
        private final long firstRecvTimeMs;

        /** Start -> request complete. */
        private final long totalTimeMs;

        /** Start -> final redirect request issued (0 without redirects). */
        private final long redirectTimeMs;

        RequestMetrics(String serverIp, long dnsTimeMs, long connectTimeMs, long tlsTimeMs,
            long firstSendTimeMs, long firstRecvTimeMs, long totalTimeMs, long redirectTimeMs) {
            this.serverIp = serverIp;
            this.dnsTimeMs = dnsTimeMs;
            this.connectTimeMs = connectTimeMs;
            this.tlsTimeMs = tlsTimeMs;
            this.firstSendTimeMs = firstSendTimeMs;
            this.firstRecvTimeMs = firstRecvTimeMs;
            this.totalTimeMs = totalTimeMs;
            this.redirectTimeMs = redirectTimeMs;
        }
    }

    /**
     * Carries the mutable timing and resolution state collected while a
     * request and its redirect chain execute. All timings are cumulative
     * milliseconds elapsed since {@link #startMs}.
     */
    private static final class TimingState {
        /** Monotonic clock reading at request start. */
        final long startMs;

        /** Resolved server IP of the most recent hop, or "". */
        String serverIp = "";

        /** Start -> DNS resolution complete (first hop only). */
        long dnsTimeMs = 0L;

        /** Start -> TCP connect complete (final hop). */
        long connectTimeMs = 0L;

        /** Start -> TLS handshake complete (0 for plain HTTP). */
        long tlsTimeMs = 0L;

        /** Start -> request about to be sent (final hop). */
        long firstSendTimeMs = 0L;

        /** Start -> first response byte received (final response). */
        long firstRecvTimeMs = 0L;

        /** Start -> final redirect request issued (0 without redirects). */
        long redirectTimeMs = 0L;

        /** Start -> the most recent redirect request was issued. */
        long lastRedirectStartMs = 0L;

        TimingState(long startMs) {
            this.startMs = startMs;
        }
    }

    /**
     * Bundles the outcome of a request whose redirect chain was followed
     * manually.
     */
    private static final class RedirectResult {
        /** Connection of the final response, for reading and disconnecting. */
        final HttpURLConnection connection;

        /** HTTP status code of the final response. */
        final int responseCode;

        /** True when the redirect limit was hit while still on a 3xx. */
        final boolean tooManyRedirects;

        RedirectResult(HttpURLConnection connection, int responseCode, boolean tooManyRedirects) {
            this.connection = connection;
            this.responseCode = responseCode;
            this.tooManyRedirects = tooManyRedirects;
        }
    }
}
