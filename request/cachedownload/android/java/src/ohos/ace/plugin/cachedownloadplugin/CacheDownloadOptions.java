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

import java.util.Map;

/**
 * Configuration options for cache download.
 * Matches the ArkTS CacheDownloadOptions interface.
 *
 * @since 1.0.0
 */
public class CacheDownloadOptions {

    /** HTTP request headers. */
    private Map<String, String> headers;

    /** Secure communication protocol. Default: "TLS". */
    private String sslType = "TLS";

    /** CA certificate path (.pem format). Null = use system default. */
    private String caPath;

    /** Cache update strategy. Default: FORCE. */
    private CacheStrategy cacheStrategy = CacheStrategy.FORCE;

    /** Per-task retry configuration. Null = use global/default. */
    private RetryOptions retry;

    /** Per-task timeout configuration. Null = use global/default. */
    private TimeoutOptions timeout;

    // ============================================================
    // Enums
    // ============================================================

    /**
     * Cache update strategy enum.
     * Determines when cached data is refreshed.
     */
    public enum CacheStrategy {
        FORCE(0),
        LAZY(1);

        final int value;

        CacheStrategy(int value) {
            this.value = value;
        }
    }

    /**
     * Gets the HTTP request headers.
     *
     * @return Header map, or null if not set.
     */
    public Map<String, String> getHeaders() {
        return headers;
    }

    /**
     * Sets the HTTP request headers.
     *
     * @param headers Header map.
     */
    public void setHeaders(Map<String, String> headers) {
        this.headers = headers;
    }

    /**
     * Gets the SSL/TLS type.
     *
     * @return "TLS" or "TLCP".
     */
    public String getSslType() {
        return sslType;
    }

    /**
     * Sets the SSL/TLS type.
     *
     * @param sslType "TLS" or "TLCP".
     */
    public void setSslType(String sslType) {
        this.sslType = sslType;
    }

    /**
     * Gets the CA certificate path.
     *
     * @return CA path string, or null for system default.
     */
    public String getCaPath() {
        return caPath;
    }

    /**
     * Sets the CA certificate path.
     *
     * @param caPath CA path in .pem format, or null for system default.
     */
    public void setCaPath(String caPath) {
        this.caPath = caPath;
    }

    /**
     * Gets the cache strategy.
     *
     * @return Cache strategy enum value.
     */
    public CacheStrategy getCacheStrategy() {
        return cacheStrategy;
    }

    /**
     * Sets the cache strategy.
     *
     * @param cacheStrategy Cache strategy enum value.
     */
    public void setCacheStrategy(CacheStrategy cacheStrategy) {
        this.cacheStrategy = cacheStrategy;
    }

    /**
     * Gets the retry options.
     *
     * @return Retry options, or null for global/default.
     */
    public RetryOptions getRetry() {
        return retry;
    }

    /**
     * Sets the retry options.
     *
     * @param retry Retry options, or null for global/default.
     */
    public void setRetry(RetryOptions retry) {
        this.retry = retry;
    }

    /**
     * Gets the timeout options.
     *
     * @return Timeout options, or null for global/default.
     */
    public TimeoutOptions getTimeout() {
        return timeout;
    }

    /**
     * Sets the timeout options.
     *
     * @param timeout Timeout options, or null for global/default.
     */
    public void setTimeout(TimeoutOptions timeout) {
        this.timeout = timeout;
    }
}
