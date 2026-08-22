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

/**
 * Task timeout configuration.
 * Matches the ArkTS TimeoutOptions interface.
 *
 * @since 1.0.0
 */
public class TimeoutOptions {
    /** Network availability check timeout in seconds. Default: 20. Range: [0, 20]. */
    private int networkCheckTimeout = 20;

    /** Complete HTTP request-response cycle timeout in seconds. Default: 60. Min: 1. */
    private int httpTotalTimeout = 60;

    /**
     * Gets the network check timeout.
     *
     * @return The network check timeout in seconds.
     */
    public int getNetworkCheckTimeout() {
        return networkCheckTimeout;
    }

    /**
     * Sets the network check timeout.
     *
     * @param networkCheckTimeout The network check timeout in seconds. Range: [0, 20].
     */
    public void setNetworkCheckTimeout(int networkCheckTimeout) {
        this.networkCheckTimeout = networkCheckTimeout;
    }

    /**
     * Gets the HTTP total timeout.
     *
     * @return The HTTP total timeout in seconds.
     */
    public int getHttpTotalTimeout() {
        return httpTotalTimeout;
    }

    /**
     * Sets the HTTP total timeout.
     *
     * @param httpTotalTimeout The HTTP total timeout in seconds. Min: 1.
     */
    public void setHttpTotalTimeout(int httpTotalTimeout) {
        this.httpTotalTimeout = httpTotalTimeout;
    }
}
