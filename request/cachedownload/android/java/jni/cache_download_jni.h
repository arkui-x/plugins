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

#ifndef PLUGINS_REQUEST_CACHE_DOWNLOAD_JNI_CACHE_DOWNLOAD_JNI_H
#define PLUGINS_REQUEST_CACHE_DOWNLOAD_JNI_CACHE_DOWNLOAD_JNI_H

#include <jni.h>
#include <string>

namespace OHOS::Plugin::CacheDownload {

/**
 * SafeFindClass - Clears pending exception before and after FindClass.
 *
 * The engine's ace_env_jni.cpp:35 produces an uncleaned NoClassDefFoundError
 * (boot classloader cannot find app classes). Calling FindClass with a pending
 * exception triggers JniAbort -> SIGABRT. This helper clears any pending
 * exception before FindClass, and if FindClass itself fails (returns nullptr),
 * clears the new exception so subsequent JNI calls are safe.
 */
inline jclass SafeFindClass(JNIEnv* env, const char* name)
{
    if (env == nullptr) {
        return nullptr;
    }
    if (env->ExceptionCheck()) { env->ExceptionClear(); }
    jclass cls = env->FindClass(name);
    if (cls == nullptr && env->ExceptionCheck()) { env->ExceptionClear(); }
    return cls;
}

/**
 * JNI bridge between Java CacheDownloadPlugin and C++ CacheDownloadService.
 */
class CacheDownloadJni {
public:
    /** Registers all native methods with the JVM. */
    static bool Register(void* env);

    /** JNI native init — caches Java method/field IDs. */
    static void NativeInit(JNIEnv* env, jobject thiz);

    // ============================================================
    // Native methods called from Java
    // ============================================================

    static void NativeDownload(JNIEnv* env, jobject thiz,
        jstring url, jobject options);

    static void NativeCancel(JNIEnv* env, jobject thiz, jstring url);

    static jbyteArray NativeFetch(JNIEnv* env, jobject thiz, jstring url);

    static void NativeSetMemoryCacheSize(JNIEnv* env, jobject thiz, jlong bytes);

    static void NativeSetFileCacheSize(JNIEnv* env, jobject thiz, jlong bytes);

    static jobject NativeGetDownloadInfo(JNIEnv* env, jobject thiz, jstring url);

    static void NativeSetDownloadInfoListSize(JNIEnv* env, jobject thiz, jlong size);

    static void NativeClearMemoryCache(JNIEnv* env, jobject thiz);

    static void NativeClearFileCache(JNIEnv* env, jobject thiz);

    static void NativeOnDownloadSuccess(JNIEnv* env, jobject thiz,
        jstring url, jobject callback);

    static void NativeOnDownloadError(JNIEnv* env, jobject thiz,
        jstring url, jobject callback);

    static void NativeOffDownloadSuccess(JNIEnv* env, jobject thiz,
        jstring url, jobject callback);

    static void NativeOffDownloadError(JNIEnv* env, jobject thiz,
        jstring url, jobject callback);

    static void NativeSetGlobalRetryOptions(JNIEnv* env, jobject thiz, jobject options);

    static void NativeSetGlobalTimeoutOptions(JNIEnv* env, jobject thiz, jobject options);

private:
    static jclass pluginClass_;
    static jmethodID onSuccessMethodId_;
    static jmethodID onErrorMethodId_;
};

} // namespace OHOS::Plugin::CacheDownload

#endif // PLUGINS_REQUEST_CACHE_DOWNLOAD_JNI_CACHE_DOWNLOAD_JNI_H
