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

#ifndef PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_HTTP_CLIENT_IMPL_H
#define PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_HTTP_CLIENT_IMPL_H

#include "cache_http_client.h"

#include <jni.h>
#include <memory>

namespace OHOS::Plugin::CacheDownload {

/**
 * Platform implementation of CacheHttpClient backed by HttpURLConnection.
 */
class CacheHttpClientAndroid : public CacheHttpClient {
public:
    CacheHttpClientAndroid();
    ~CacheHttpClientAndroid() override;

    std::shared_ptr<CacheHttpRequest> NewRequest(
        const CacheHttpRequestOptions& options,
        const std::shared_ptr<CacheHttpRequestCallback>& callback) override;

    /**
     * Caches the CacheDownloadHttpClient jclass and method IDs from a
     * JNIEnv known to have the correct ClassLoader (i.e., the JNIEnv
     * passed to NativeInit from Java).
     *
     * This must be called before NewRequest to avoid FindClass failures
     * when ARKUI_X_Plugin_GetJniEnv() returns a JNIEnv whose FindClass
     * uses a ClassLoader that cannot see the plugin jar.
     */
    static void InitializeJni(JNIEnv* env);

    /**
     * Creates a Java RequestConfig object from request options.
     * Uses the cached jclass/jmethodID (correct ClassLoader).
     * Returns nullptr if JNI cache is not ready or construction fails.
     */
    static jobject CreateRequestConfig(JNIEnv* env, const CacheHttpRequestOptions& opts);

private:
    bool EnsureJniReady(JNIEnv* env);

    // Caches the RequestConfig inner class JNI refs; called by InitializeJni.
    static bool CacheRequestConfigJni(JNIEnv* env);

    jobject clientRef_ = nullptr;
    jmethodID executeRequestId_ = nullptr;

    // Static JNI cache — initialized once from NativeInit (correct ClassLoader).
    // Used by EnsureJniReady to avoid FindClass/GetMethodID with a potentially
    // wrong ClassLoader from ARKUI_X_Plugin_GetJniEnv().
    static jclass sHttpClientClass;
    static jmethodID sCtorMethodId;
    static jmethodID sExecuteMethodId;
    static jclass sConfigClass;
    static jmethodID sConfigCtorId;
    static bool sJniCacheReady;
};

} // namespace OHOS::Plugin::CacheDownload

#endif // PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_HTTP_CLIENT_IMPL_H
