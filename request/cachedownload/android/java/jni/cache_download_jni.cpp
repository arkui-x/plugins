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

#include "cache_download_jni.h"
#include "cache_download_interface.h"
#include "cache_types.h"
#include "cache_http_client_impl.h"

#include "plugin_utils.h"

#include <jni.h>
#include <atomic>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace OHOS::Plugin::CacheDownload {

// Static member initialization
jclass CacheDownloadJni::pluginClass_ = nullptr;
jmethodID CacheDownloadJni::onSuccessMethodId_ = nullptr;
jmethodID CacheDownloadJni::onErrorMethodId_ = nullptr;

namespace {

// Cached class/method IDs for Java option types and callbacks.
struct {
    jclass retryOptionsClass = nullptr;
    jfieldID retryMaxCountField = nullptr;

    jclass timeoutOptionsClass = nullptr;
    jfieldID timeoutNetworkField = nullptr;
    jfieldID timeoutHttpField = nullptr;

    jclass cacheStrategyClass = nullptr;
    jfieldID cacheStrategyValueField = nullptr;

    jclass downloadInfoClass = nullptr;
    jclass resourceInfoClass = nullptr;
    jclass networkInfoClass = nullptr;
    jclass performanceInfoClass = nullptr;
} g_jniCache;

// ============================================================
// JNI callback registry
//
// Owns the GlobalRef lifecycle for Java callback objects passed to
// nativeOnDownloadSuccess / nativeOnDownloadError. Each registration gets a
// unique token; the corresponding off call uses IsSameObject to find and
// remove the exact entry, then DeleteGlobalRef frees the reference.
//
// Fixes:
//   - L4/L5: off now removes only the specific callback (by identity)
//   - L6/L7: GlobalRef is freed when off is called (no permanent leak)
// ============================================================

struct JniCallbackEntry {
    int64_t token;
    jobject globalRef;
};

static std::mutex g_successCbMutex;
static std::unordered_map<std::string, std::vector<JniCallbackEntry>> g_successCbRegistry;
static std::mutex g_errorCbMutex;
static std::unordered_map<std::string, std::vector<JniCallbackEntry>> g_errorCbRegistry;
static std::atomic<int64_t> g_nextToken{1};

/** Converts a jstring to std::string. */
std::string JstringToString(JNIEnv* env, jstring jstr)
{
    if (jstr == nullptr) {
        return "";
    }
    const char* chars = env->GetStringUTFChars(jstr, nullptr);
    if (chars == nullptr) {
        // OOM: clear the pending OutOfMemoryError so subsequent JNI calls
        // (e.g., SafeFindClass/FindClass) don't trigger JniAbort → SIGABRT.
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
        }
        return "";
    }
    std::string result = chars;
    env->ReleaseStringUTFChars(jstr, chars);
    return result;
}

/** Converts a std::string to jstring. */
jstring StringToJstring(JNIEnv* env, const std::string& str)
{
    return env->NewStringUTF(str.c_str());
}

/** Reads a String field from a Java object. */
std::string GetStringField(JNIEnv* env, jobject obj, jfieldID fieldId)
{
    jstring value = static_cast<jstring>(env->GetObjectField(obj, fieldId));
    return JstringToString(env, value);
}

/** Reads an int field from a Java object. */
int GetIntField(JNIEnv* env, jobject obj, jfieldID fieldId)
{
    return env->GetIntField(obj, fieldId);
}

/** Reads a Map<String, String> field into a std::map. */
std::map<std::string, std::string> GetStringMapField(JNIEnv* env, jobject obj, jfieldID fieldId)
{
    std::map<std::string, std::string> result;
    jobject jmap = env->GetObjectField(obj, fieldId);
    if (jmap == nullptr) {
        return result;
    }

    jclass mapClass = SafeFindClass(env, "java/util/Map");
    jmethodID entrySetId = env->GetMethodID(mapClass, "entrySet", "()Ljava/util/Set;");
    jobject entrySet = env->CallObjectMethod(jmap, entrySetId);

    jclass setClass = SafeFindClass(env, "java/util/Set");
    jmethodID iteratorId = env->GetMethodID(setClass, "iterator", "()Ljava/util/Iterator;");
    jobject iter = env->CallObjectMethod(entrySet, iteratorId);

    jclass iteratorClass = SafeFindClass(env, "java/util/Iterator");
    jmethodID hasNextId = env->GetMethodID(iteratorClass, "hasNext", "()Z");
    jmethodID nextId = env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");

    jclass entryClass = SafeFindClass(env, "java/util/Map$Entry");
    jmethodID getKeyId = env->GetMethodID(entryClass, "getKey", "()Ljava/lang/Object;");
    jmethodID getValueId = env->GetMethodID(entryClass, "getValue", "()Ljava/lang/Object;");

    while (env->CallBooleanMethod(iter, hasNextId)) {
        jobject entry = env->CallObjectMethod(iter, nextId);
        jstring jkey = static_cast<jstring>(env->CallObjectMethod(entry, getKeyId));
        jstring jvalue = static_cast<jstring>(env->CallObjectMethod(entry, getValueId));
        result[JstringToString(env, jkey)] = JstringToString(env, jvalue);
        env->DeleteLocalRef(jkey);
        env->DeleteLocalRef(jvalue);
        env->DeleteLocalRef(entry);
    }

    env->DeleteLocalRef(iteratorClass);
    env->DeleteLocalRef(iter);
    env->DeleteLocalRef(setClass);
    env->DeleteLocalRef(entrySet);
    env->DeleteLocalRef(mapClass);
    env->DeleteLocalRef(jmap);
    return result;
}

static const JNINativeMethod g_nativeMethods[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(&CacheDownloadJni::NativeInit) },
    { "nativeDownload", "(Ljava/lang/String;Lohos/ace/plugin/cachedownloadplugin/CacheDownloadOptions;)V",
      reinterpret_cast<void*>(&CacheDownloadJni::NativeDownload) },
    { "nativeCancel", "(Ljava/lang/String;)V",
      reinterpret_cast<void*>(&CacheDownloadJni::NativeCancel) },
    { "nativeFetch", "(Ljava/lang/String;)[B",
      reinterpret_cast<void*>(&CacheDownloadJni::NativeFetch) },
    { "nativeSetMemoryCacheSize", "(J)V",
      reinterpret_cast<void*>(&CacheDownloadJni::NativeSetMemoryCacheSize) },
    { "nativeSetFileCacheSize", "(J)V",
      reinterpret_cast<void*>(&CacheDownloadJni::NativeSetFileCacheSize) },
    { "nativeGetDownloadInfo", "(Ljava/lang/String;)Lohos/ace/plugin/cachedownloadplugin/DownloadInfo;",
      reinterpret_cast<void*>(&CacheDownloadJni::NativeGetDownloadInfo) },
    { "nativeSetDownloadInfoListSize", "(J)V",
      reinterpret_cast<void*>(&CacheDownloadJni::NativeSetDownloadInfoListSize) },
    { "nativeClearMemoryCache", "()V",
      reinterpret_cast<void*>(&CacheDownloadJni::NativeClearMemoryCache) },
    { "nativeClearFileCache", "()V",
      reinterpret_cast<void*>(&CacheDownloadJni::NativeClearFileCache) },
    { "nativeOnDownloadSuccess", "(Ljava/lang/String;Ljava/lang/Runnable;)V",
      reinterpret_cast<void*>(&CacheDownloadJni::NativeOnDownloadSuccess) },
    { "nativeOnDownloadError",
      "(Ljava/lang/String;Lohos/ace/plugin/cachedownloadplugin/"
      "CacheDownloadPlugin$OnDownloadErrorCallback;)V",
      reinterpret_cast<void*>(&CacheDownloadJni::NativeOnDownloadError) },
    { "nativeOffDownloadSuccess", "(Ljava/lang/String;Ljava/lang/Runnable;)V",
      reinterpret_cast<void*>(&CacheDownloadJni::NativeOffDownloadSuccess) },
    { "nativeOffDownloadError",
      "(Ljava/lang/String;Lohos/ace/plugin/cachedownloadplugin/"
      "CacheDownloadPlugin$OnDownloadErrorCallback;)V",
      reinterpret_cast<void*>(&CacheDownloadJni::NativeOffDownloadError) },
    { "nativeSetGlobalRetryOptions", "(Lohos/ace/plugin/cachedownloadplugin/RetryOptions;)V",
      reinterpret_cast<void*>(&CacheDownloadJni::NativeSetGlobalRetryOptions) },
    { "nativeSetGlobalTimeoutOptions", "(Lohos/ace/plugin/cachedownloadplugin/TimeoutOptions;)V",
      reinterpret_cast<void*>(&CacheDownloadJni::NativeSetGlobalTimeoutOptions) },
};

/** Caches JNI class refs and field IDs for option/callback types. Called once from NativeInit. */
void CacheJniOptionTypes(JNIEnv* env)
{
    g_jniCache.retryOptionsClass = static_cast<jclass>(env->NewGlobalRef(
        SafeFindClass(env, "ohos/ace/plugin/cachedownloadplugin/RetryOptions")));
    g_jniCache.retryMaxCountField = env->GetFieldID(g_jniCache.retryOptionsClass, "maxRetryCount", "I");

    g_jniCache.timeoutOptionsClass = static_cast<jclass>(env->NewGlobalRef(
        SafeFindClass(env, "ohos/ace/plugin/cachedownloadplugin/TimeoutOptions")));
    g_jniCache.timeoutNetworkField = env->GetFieldID(g_jniCache.timeoutOptionsClass,
        "networkCheckTimeout", "I");
    g_jniCache.timeoutHttpField = env->GetFieldID(g_jniCache.timeoutOptionsClass,
        "httpTotalTimeout", "I");

    g_jniCache.cacheStrategyClass = static_cast<jclass>(env->NewGlobalRef(
        SafeFindClass(env, "ohos/ace/plugin/cachedownloadplugin/CacheDownloadOptions$CacheStrategy")));
    g_jniCache.cacheStrategyValueField = env->GetFieldID(g_jniCache.cacheStrategyClass, "value", "I");

    g_jniCache.downloadInfoClass = static_cast<jclass>(env->NewGlobalRef(
        SafeFindClass(env, "ohos/ace/plugin/cachedownloadplugin/DownloadInfo")));
    g_jniCache.resourceInfoClass = static_cast<jclass>(env->NewGlobalRef(
        SafeFindClass(env, "ohos/ace/plugin/cachedownloadplugin/DownloadInfo$ResourceInfo")));
    g_jniCache.networkInfoClass = static_cast<jclass>(env->NewGlobalRef(
        SafeFindClass(env, "ohos/ace/plugin/cachedownloadplugin/DownloadInfo$NetworkInfo")));
    g_jniCache.performanceInfoClass = static_cast<jclass>(env->NewGlobalRef(
        SafeFindClass(env, "ohos/ace/plugin/cachedownloadplugin/DownloadInfo$PerformanceInfo")));
}

} // anonymous namespace

bool CacheDownloadJni::Register(void* env)
{
    auto* jniEnv = static_cast<JNIEnv*>(env);
    if (jniEnv == nullptr) {
        return false;
    }

    // Clear any pending exception (e.g., NoClassDefFoundError from engine's
    // ace_env_jni.cpp) before calling FindClass.  JNI FindClass with a pending
    // exception triggers JniAbort -> Runtime::Abort -> SIGABRT.
    if (jniEnv->ExceptionCheck()) {
        jniEnv->ExceptionClear();
    }

    jclass cls = SafeFindClass(jniEnv, "ohos/ace/plugin/cachedownloadplugin/CacheDownloadPlugin");
    if (cls == nullptr) {
        return false;
    }
    if (jniEnv->RegisterNatives(cls, g_nativeMethods,
        sizeof(g_nativeMethods) / sizeof(g_nativeMethods[0])) != 0) {
        jniEnv->DeleteLocalRef(cls);
        return false;
    }

    // Create the Java plugin instance via the private no-arg constructor.
    // The constructor sets sInstance = this and calls nativeInit(), which
    // caches JNI refs and resolves the cache directory while the correct
    // ClassLoader is active. RegisterNatives above must run first so the
    // native method is bound before the constructor calls it.
    jmethodID ctor = jniEnv->GetMethodID(cls, "<init>", "()V");
    if (ctor == nullptr) {
        if (jniEnv->ExceptionCheck()) {
            jniEnv->ExceptionClear();
        }
        jniEnv->DeleteLocalRef(cls);
        return false;
    }
    jobject pluginObj = jniEnv->NewObject(cls, ctor);
    if (pluginObj == nullptr) {
        if (jniEnv->ExceptionCheck()) {
            jniEnv->ExceptionClear();
        }
        jniEnv->DeleteLocalRef(cls);
        return false;
    }
    jniEnv->DeleteLocalRef(pluginObj);

    jniEnv->DeleteLocalRef(cls);
    return true;
}

void CacheDownloadJni::NativeInit(JNIEnv* env, jobject thiz)
{
    if (env == nullptr) {
        return;
    }

    // Defensive: clear any pending exception before JNI calls (FindClass, etc.).
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }

    jclass clazz = env->GetObjectClass(thiz);
    pluginClass_ = static_cast<jclass>(env->NewGlobalRef(clazz));

    // Cache CacheDownloadPlugin.getCacheDir() and initialize the service.
    jmethodID getCacheDirId = env->GetMethodID(clazz, "getCacheDir", "()Ljava/lang/String;");
    if (getCacheDirId != nullptr) {
        jstring jcacheDir = static_cast<jstring>(env->CallObjectMethod(thiz, getCacheDirId));
        if (jcacheDir != nullptr) {
            std::string cacheDir = JstringToString(env, jcacheDir);
            if (!cacheDir.empty()) {
                CacheDownloadService::GetInstance().Initialize(cacheDir);
            }
            env->DeleteLocalRef(jcacheDir);
        } else if (env->ExceptionCheck()) {
            // getCacheDir() threw IllegalStateException — clear it so
            // subsequent JNI calls are safe (graceful: cache dir not set).
            env->ExceptionClear();
        }
    }

    env->DeleteLocalRef(clazz);

    // Cache Runnable.run()
    jclass runnableClass = SafeFindClass(env, "java/lang/Runnable");
    onSuccessMethodId_ = env->GetMethodID(runnableClass, "run", "()V");
    env->DeleteLocalRef(runnableClass);

    // Cache OnDownloadErrorCallback.onError(int, String)
    jclass errorCbClass = SafeFindClass(env,
        "ohos/ace/plugin/cachedownloadplugin/CacheDownloadPlugin$OnDownloadErrorCallback");
    onErrorMethodId_ = env->GetMethodID(errorCbClass, "onError", "(ILjava/lang/String;)V");
    env->DeleteLocalRef(errorCbClass);

    // Cache option/callback type class refs and field IDs.
    CacheJniOptionTypes(env);

    // Initialize the HTTP client JNI cache (class, method IDs) while we have
    // the correct ClassLoader from the Java caller. ARKUI_X_Plugin_GetJniEnv()
    // may return a JNIEnv whose FindClass uses the system ClassLoader, which
    // cannot find CacheDownloadHttpClient in the plugin jar.
    CacheHttpClientAndroid::InitializeJni(env);
}

// ============================================================
// Native method implementations
// ============================================================

void CacheDownloadJni::NativeDownload(JNIEnv* env, jobject /* thiz */,
    jstring url, jobject options)
{
    std::string urlStr = JstringToString(env, url);

    CacheDownloadOptions cdOptions;
    if (options != nullptr) {
        jclass optionsClass = SafeFindClass(env, "ohos/ace/plugin/cachedownloadplugin/CacheDownloadOptions");

        jfieldID headersField = env->GetFieldID(optionsClass, "headers", "Ljava/util/Map;");
        cdOptions.headers = GetStringMapField(env, options, headersField);

        jfieldID sslTypeField = env->GetFieldID(optionsClass, "sslType", "Ljava/lang/String;");
        jstring jsslType = static_cast<jstring>(env->GetObjectField(options, sslTypeField));
        std::string sslType = JstringToString(env, jsslType);
        if (sslType == "TLCP") {
            cdOptions.sslType = SslType::TLCP;
        } else {
            cdOptions.sslType = SslType::TLS;
        }
        env->DeleteLocalRef(jsslType);

        jfieldID caPathField = env->GetFieldID(optionsClass, "caPath", "Ljava/lang/String;");
        cdOptions.caPath = GetStringField(env, options, caPathField);

        jfieldID strategyField = env->GetFieldID(
            optionsClass, "cacheStrategy",
            "Lohos/ace/plugin/cachedownloadplugin/CacheDownloadOptions$CacheStrategy;");
        jobject jstrategy = env->GetObjectField(options, strategyField);
        if (jstrategy != nullptr) {
            int value = env->GetIntField(jstrategy, g_jniCache.cacheStrategyValueField);
            cdOptions.cacheStrategy = (value == 1) ? CacheStrategy::LAZY : CacheStrategy::FORCE;
            env->DeleteLocalRef(jstrategy);
        }

        jfieldID retryField = env->GetFieldID(optionsClass, "retry",
            "Lohos/ace/plugin/cachedownloadplugin/RetryOptions;");
        jobject jretry = env->GetObjectField(options, retryField);
        if (jretry != nullptr) {
            cdOptions.retry = RetryOptions{
                static_cast<size_t>(GetIntField(env, jretry, g_jniCache.retryMaxCountField))};
            env->DeleteLocalRef(jretry);
        }

        jfieldID timeoutField = env->GetFieldID(optionsClass, "timeout",
            "Lohos/ace/plugin/cachedownloadplugin/TimeoutOptions;");
        jobject jtimeout = env->GetObjectField(options, timeoutField);
        if (jtimeout != nullptr) {
            cdOptions.timeout = TimeoutOptions{
                static_cast<uint32_t>(GetIntField(env, jtimeout, g_jniCache.timeoutNetworkField)),
                static_cast<uint32_t>(GetIntField(env, jtimeout, g_jniCache.timeoutHttpField))};
            env->DeleteLocalRef(jtimeout);
        }

        env->DeleteLocalRef(optionsClass);
    }

    auto& service = CacheDownloadService::GetInstance();
    service.Download(urlStr, cdOptions);
}

void CacheDownloadJni::NativeCancel(JNIEnv* env, jobject /* thiz */, jstring url)
{
    std::string urlStr = JstringToString(env, url);
    CacheDownloadService::GetInstance().Cancel(urlStr);
}

jbyteArray CacheDownloadJni::NativeFetch(JNIEnv* env, jobject /* thiz */, jstring url)
{
    std::string urlStr = JstringToString(env, url);
    auto cache = CacheDownloadService::GetInstance().Fetch(urlStr);
    if (!cache || cache->Size() == 0) {
        return nullptr;
    }

    jbyteArray result = env->NewByteArray(static_cast<jsize>(cache->Size()));
    env->SetByteArrayRegion(result, 0, static_cast<jsize>(cache->Size()),
                            reinterpret_cast<const jbyte*>(cache->Data()));
    return result;
}

void CacheDownloadJni::NativeSetMemoryCacheSize(JNIEnv* /* env */, jobject /* thiz */, jlong bytes)
{
    CacheDownloadService::GetInstance().SetMemoryCacheSize(static_cast<uint64_t>(bytes));
}

void CacheDownloadJni::NativeSetFileCacheSize(JNIEnv* /* env */, jobject /* thiz */, jlong bytes)
{
    CacheDownloadService::GetInstance().SetFileCacheSize(static_cast<uint64_t>(bytes));
}

jobject CacheDownloadJni::NativeGetDownloadInfo(JNIEnv* env, jobject /* thiz */, jstring url)
{
    std::string urlStr = JstringToString(env, url);
    auto infoOpt = CacheDownloadService::GetInstance().GetDownloadInfo(urlStr);
    if (!infoOpt.has_value()) {
        return nullptr;
    }

    const DownloadInfo& info = infoOpt.value();

    jobject resourceInfo = env->AllocObject(g_jniCache.resourceInfoClass);
    env->SetLongField(resourceInfo,
        env->GetFieldID(g_jniCache.resourceInfoClass, "size", "J"),
        static_cast<jlong>(info.resource.size));

    jobject networkInfo = env->AllocObject(g_jniCache.networkInfoClass);
    env->SetObjectField(networkInfo,
        env->GetFieldID(g_jniCache.networkInfoClass, "ip", "Ljava/lang/String;"),
        StringToJstring(env, info.network.ip));

    jobject performanceInfo = env->AllocObject(g_jniCache.performanceInfoClass);
    jfieldID dnsTimeField = env->GetFieldID(g_jniCache.performanceInfoClass, "dnsTime", "D");
    jfieldID connectTimeField = env->GetFieldID(g_jniCache.performanceInfoClass, "connectTime", "D");
    jfieldID tlsTimeField = env->GetFieldID(g_jniCache.performanceInfoClass, "tlsTime", "D");
    jfieldID firstSendTimeField = env->GetFieldID(g_jniCache.performanceInfoClass, "firstSendTime", "D");
    jfieldID firstRecvTimeField = env->GetFieldID(g_jniCache.performanceInfoClass, "firstRecvTime", "D");
    jfieldID totalTimeField = env->GetFieldID(g_jniCache.performanceInfoClass, "totalTime", "D");
    jfieldID redirectTimeField = env->GetFieldID(g_jniCache.performanceInfoClass, "redirectTime", "D");

    env->SetDoubleField(performanceInfo, dnsTimeField, info.performance.dnsTime);
    env->SetDoubleField(performanceInfo, connectTimeField, info.performance.connectTime);
    env->SetDoubleField(performanceInfo, tlsTimeField, info.performance.tlsTime);
    env->SetDoubleField(performanceInfo, firstSendTimeField, info.performance.firstSendTime);
    env->SetDoubleField(performanceInfo, firstRecvTimeField, info.performance.firstRecvTime);
    env->SetDoubleField(performanceInfo, totalTimeField, info.performance.totalTime);
    env->SetDoubleField(performanceInfo, redirectTimeField, info.performance.redirectTime);

    jobject downloadInfo = env->AllocObject(g_jniCache.downloadInfoClass);
    env->SetObjectField(downloadInfo,
        env->GetFieldID(g_jniCache.downloadInfoClass, "resource",
            "Lohos/ace/plugin/cachedownloadplugin/DownloadInfo$ResourceInfo;"),
        resourceInfo);
    env->SetObjectField(downloadInfo,
        env->GetFieldID(g_jniCache.downloadInfoClass, "network",
            "Lohos/ace/plugin/cachedownloadplugin/DownloadInfo$NetworkInfo;"),
        networkInfo);
    env->SetObjectField(downloadInfo,
        env->GetFieldID(g_jniCache.downloadInfoClass, "performance",
            "Lohos/ace/plugin/cachedownloadplugin/DownloadInfo$PerformanceInfo;"),
        performanceInfo);

    env->DeleteLocalRef(resourceInfo);
    env->DeleteLocalRef(networkInfo);
    env->DeleteLocalRef(performanceInfo);

    return downloadInfo;
}

void CacheDownloadJni::NativeSetDownloadInfoListSize(JNIEnv* /* env */, jobject /* thiz */, jlong size)
{
    CacheDownloadService::GetInstance().SetDownloadInfoListSize(static_cast<uint16_t>(size));
}

void CacheDownloadJni::NativeClearMemoryCache(JNIEnv* /* env */, jobject /* thiz */)
{
    CacheDownloadService::GetInstance().ClearMemoryCache();
}

void CacheDownloadJni::NativeClearFileCache(JNIEnv* /* env */, jobject /* thiz */)
{
    CacheDownloadService::GetInstance().ClearFileCache();
}

void CacheDownloadJni::NativeOnDownloadSuccess(JNIEnv* env, jobject /* thiz */,
    jstring url, jobject callback)
{
    std::string urlStr = JstringToString(env, url);
    jobject globalCallback = env->NewGlobalRef(callback);
    if (globalCallback == nullptr) {
        return;
    }
    int64_t token = g_nextToken.fetch_add(1);

    {
        std::lock_guard<std::mutex> lock(g_successCbMutex);
        g_successCbRegistry[urlStr].push_back({token, globalCallback});
    }

    // The lambda captures globalCallback for fast access but does NOT own it.
    // The registry owns the GlobalRef lifecycle; it is freed in
    // NativeOffDownloadSuccess via DeleteGlobalRef.
    CacheDownloadService::GetInstance().OnDownloadSuccess(urlStr, token, [globalCallback]() {
        JNIEnv* cbEnv = ARKUI_X_Plugin_GetJniEnv();
        if (cbEnv != nullptr && globalCallback != nullptr && onSuccessMethodId_ != nullptr) {
            cbEnv->CallVoidMethod(globalCallback, onSuccessMethodId_);
        }
    });
}

void CacheDownloadJni::NativeOnDownloadError(JNIEnv* env, jobject /* thiz */,
    jstring url, jobject callback)
{
    std::string urlStr = JstringToString(env, url);
    jobject globalCallback = env->NewGlobalRef(callback);
    if (globalCallback == nullptr) {
        return;
    }
    int64_t token = g_nextToken.fetch_add(1);

    {
        std::lock_guard<std::mutex> lock(g_errorCbMutex);
        g_errorCbRegistry[urlStr].push_back({token, globalCallback});
    }

    // The lambda captures globalCallback for fast access but does NOT own it.
    // The registry owns the GlobalRef lifecycle; it is freed in
    // NativeOffDownloadError via DeleteGlobalRef.
    CacheDownloadService::GetInstance().OnDownloadError(urlStr, token,
        [globalCallback](const CacheDownloadError& error) {
            JNIEnv* cbEnv = ARKUI_X_Plugin_GetJniEnv();
            if (cbEnv != nullptr && globalCallback != nullptr && onErrorMethodId_ != nullptr) {
                jstring jmessage = StringToJstring(cbEnv, error.message);
                cbEnv->CallVoidMethod(globalCallback, onErrorMethodId_,
                    static_cast<jint>(error.code), jmessage);
                cbEnv->DeleteLocalRef(jmessage);
            }
        });
}

void CacheDownloadJni::NativeOffDownloadSuccess(JNIEnv* env, jobject /* thiz */,
    jstring url, jobject callback)
{
    std::string urlStr = JstringToString(env, url);

    // Java Javadoc: "If null, removes ALL callbacks for the URL."
    if (callback == nullptr) {
        std::vector<jobject> refsToDelete;
        {
            std::lock_guard<std::mutex> lock(g_successCbMutex);
            auto it = g_successCbRegistry.find(urlStr);
            if (it == g_successCbRegistry.end()) {
                return;
            }
            for (const auto& entry : it->second) {
                refsToDelete.push_back(entry.globalRef);
            }
            g_successCbRegistry.erase(it);
        }
        for (jobject ref : refsToDelete) {
            env->DeleteGlobalRef(ref);
        }
        CacheDownloadService::GetInstance().OffDownloadSuccess(urlStr, 0);
        return;
    }

    int64_t tokenToRemove = -1;
    jobject refToDelete = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_successCbMutex);
        auto it = g_successCbRegistry.find(urlStr);
        if (it == g_successCbRegistry.end()) {
            return;
        }
        auto& entries = it->second;
        for (auto eit = entries.begin(); eit != entries.end(); ++eit) {
            if (env->IsSameObject(eit->globalRef, callback)) {
                tokenToRemove = eit->token;
                refToDelete = eit->globalRef;
                entries.erase(eit);
                if (entries.empty()) {
                    g_successCbRegistry.erase(it);
                }
                break;
            }
        }
    }

    if (tokenToRemove >= 0) {
        CacheDownloadService::GetInstance().OffDownloadSuccess(urlStr, tokenToRemove);
    }
    if (refToDelete != nullptr) {
        env->DeleteGlobalRef(refToDelete);
    }
}

void CacheDownloadJni::NativeOffDownloadError(JNIEnv* env, jobject /* thiz */,
    jstring url, jobject callback)
{
    std::string urlStr = JstringToString(env, url);

    // Java Javadoc: "If null, removes ALL callbacks for the URL."
    if (callback == nullptr) {
        std::vector<jobject> refsToDelete;
        {
            std::lock_guard<std::mutex> lock(g_errorCbMutex);
            auto it = g_errorCbRegistry.find(urlStr);
            if (it == g_errorCbRegistry.end()) {
                return;
            }
            for (const auto& entry : it->second) {
                refsToDelete.push_back(entry.globalRef);
            }
            g_errorCbRegistry.erase(it);
        }
        for (jobject ref : refsToDelete) {
            env->DeleteGlobalRef(ref);
        }
        CacheDownloadService::GetInstance().OffDownloadError(urlStr, 0);
        return;
    }

    int64_t tokenToRemove = -1;
    jobject refToDelete = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_errorCbMutex);
        auto it = g_errorCbRegistry.find(urlStr);
        if (it == g_errorCbRegistry.end()) {
            return;
        }
        auto& entries = it->second;
        for (auto eit = entries.begin(); eit != entries.end(); ++eit) {
            if (env->IsSameObject(eit->globalRef, callback)) {
                tokenToRemove = eit->token;
                refToDelete = eit->globalRef;
                entries.erase(eit);
                if (entries.empty()) {
                    g_errorCbRegistry.erase(it);
                }
                break;
            }
        }
    }

    if (tokenToRemove >= 0) {
        CacheDownloadService::GetInstance().OffDownloadError(urlStr, tokenToRemove);
    }
    if (refToDelete != nullptr) {
        env->DeleteGlobalRef(refToDelete);
    }
}

void CacheDownloadJni::NativeSetGlobalRetryOptions(JNIEnv* env, jobject /* thiz */, jobject options)
{
    if (options == nullptr) {
        CacheDownloadService::GetInstance().SetGlobalRetryOptions(std::nullopt);
        return;
    }
    int maxRetry = GetIntField(env, options, g_jniCache.retryMaxCountField);
    CacheDownloadService::GetInstance().SetGlobalRetryOptions(
        static_cast<size_t>(std::max(0, maxRetry)));
}

void CacheDownloadJni::NativeSetGlobalTimeoutOptions(JNIEnv* env, jobject /* thiz */, jobject options)
{
    if (options == nullptr) {
        CacheDownloadService::GetInstance().SetGlobalTimeoutOptions(std::nullopt, std::nullopt);
        return;
    }
    uint32_t networkCheck = static_cast<uint32_t>(
        GetIntField(env, options, g_jniCache.timeoutNetworkField));
    uint32_t httpTotal = static_cast<uint32_t>(
        GetIntField(env, options, g_jniCache.timeoutHttpField));
    CacheDownloadService::GetInstance().SetGlobalTimeoutOptions(networkCheck, httpTotal);
}

} // namespace OHOS::Plugin::CacheDownload
