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
 * Task retry configuration.
 * Matches the ArkTS RetryOptions interface.
 *
 * @since 1.0.0
 */
public class RetryOptions {
    /** Maximum number of retry attempts. Default: 1. Range: [0, 10]. */
    private int maxRetryCount = 1;

    /**
     * Gets the maximum retry count.
     *
     * @return The maximum number of retry attempts.
     */
    public int getMaxRetryCount() {
        return maxRetryCount;
    }

    /**
     * Sets the maximum retry count.
     *
     * @param maxRetryCount The maximum number of retry attempts. Range: [0, 10].
     */
    public void setMaxRetryCount(int maxRetryCount) {
        this.maxRetryCount = maxRetryCount;
    }
}
