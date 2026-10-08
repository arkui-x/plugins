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

#include "cache_http_client_impl.h"
#include "cache_error.h"
#include "cache_download_jni.h"

#include "plugin_utils.h"
#include "cache_log.h"

#include <jni.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <unordered_map>

namespace OHOS::Plugin::CacheDownload {

// Static JNI cache initialization
jclass CacheHttpClientAndroid::sHttpClientClass = nullptr;
jmethodID CacheHttpClientAndroid::sCtorMethodId = nullptr;
jmethodID CacheHttpClientAndroid::sExecuteMethodId = nullptr;
jclass CacheHttpClientAndroid::sConfigClass = nullptr;
jmethodID CacheHttpClientAndroid::sConfigCtorId = nullptr;
bool CacheHttpClientAndroid::sJniCacheReady = false;

namespace {

// Synchronizes EnsureJniReady's lazy registration trigger with the PLATFORM
// thread that runs CacheDownloadJni::Register -> NativeInit -> InitializeJni.
// EnsureJniReady fires ARKUI_X_Plugin_RegisterJavaPlugin (async PostTask to
// PLATFORM thread) once, then waits on g_jniReadyCv until InitializeJni sets
// sJniCacheReady = true. This lets Image use the innerkits (SO1) without
// requiring SO2 (librequest_cachedownload.so) to be loaded by a JS import,
// mirroring the native HarmonyOS design where libpreload_native.so is
// self-contained.
std::mutex g_jniReadyMutex;
std::condition_variable g_jniReadyCv;
std::atomic<bool> g_jniRegisterTriggered { false };

const char HTTP_CLIENT_CLASS_NAME[] = "ohos/ace/plugin/cachedownloadplugin/CacheDownloadHttpClient";
const char EXEC_REQUEST_METHOD_NAME[] = "executeRequest";
const char EXEC_REQUEST_METHOD_SIG[] =
    "(JLjava/lang/String;Lohos/ace/plugin/cachedownloadplugin/CacheDownloadHttpClient$RequestConfig;)V";

std::atomic<bool> g_nativesRegistered{false};
std::mutex g_registerMutex;

class CacheHttpRequestAndroid;

// Guards active C++ request objects that may still receive Java callbacks.
std::mutex g_requestsMutex;
std::unordered_map<uintptr_t, std::shared_ptr<CacheHttpRequestAndroid>> g_requests;

std::string JstringToString(JNIEnv* env, jstring jstr)
{
    if (jstr == nullptr) {
        return {};
    }
    const char* chars = env->GetStringUTFChars(jstr, nullptr);
    if (chars == nullptr) {
        // OOM: clear the pending OutOfMemoryError so subsequent JNI calls
        // (e.g., SafeFindClass/FindClass) don't trigger JniAbort → SIGABRT.
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
        }
        return {};
    }
    std::string result = chars;
    env->ReleaseStringUTFChars(jstr, chars);
    return result;
}

class CacheHttpRequestAndroid : public CacheHttpRequest,
                                public std::enable_shared_from_this<CacheHttpRequestAndroid> {
public:
    CacheHttpRequestAndroid(jobject clientRef,
                            jmethodID executeRequestId,
                            const CacheHttpRequestOptions& options,
                            const std::shared_ptr<CacheHttpRequestCallback>& callback)
        : clientRef_(clientRef),
          executeRequestId_(executeRequestId),
          options_(options),
          callback_(callback)
    {
    }

    ~CacheHttpRequestAndroid() override
    {
        Finish();
    }

    bool Start() override;
    void Cancel() override;

    /** Marks the request as finished and removes it from the active map. Idempotent. */
    void Finish();

    std::shared_ptr<CacheHttpRequestCallback> Callback() const { return callback_; }

private:
    jobject clientRef_;
    jmethodID executeRequestId_;
    CacheHttpRequestOptions options_;
    std::shared_ptr<CacheHttpRequestCallback> callback_;
    std::atomic<bool> finished_{false};
};

CacheHttpRequestAndroid* FindRequest(JNIEnv* /* env */, jlong callbackPtr)
{
    if (callbackPtr == 0) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(g_requestsMutex);
    auto it = g_requests.find(static_cast<uintptr_t>(callbackPtr));
    if (it == g_requests.end()) {
        return nullptr;
    }
    return it->second.get();
}

/** Builds a java/util/HashMap from a C++ string map. Returns nullptr on failure. */
jobject BuildJniHeadersMap(JNIEnv* env, const std::map<std::string, std::string>& headers)
{
    jclass hashMapClass = SafeFindClass(env, "java/util/HashMap");
    if (hashMapClass == nullptr) {
        return nullptr;
    }
    jmethodID hashMapCtor = env->GetMethodID(hashMapClass, "<init>", "()V");
    jmethodID putId = env->GetMethodID(
        hashMapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    jobject jheaders = env->NewObject(hashMapClass, hashMapCtor);
    for (const auto& [key, value] : headers) {
        jstring jkey = env->NewStringUTF(key.c_str());
        jstring jvalue = env->NewStringUTF(value.c_str());
        env->CallObjectMethod(jheaders, putId, jkey, jvalue);
        env->DeleteLocalRef(jkey);
        env->DeleteLocalRef(jvalue);
    }
    env->DeleteLocalRef(hashMapClass);
    return jheaders;
}

} // namespace

// Native method implementations called from Java.

extern "C" JNIEXPORT void JNICALL
Java_ohos_ace_plugin_cachedownloadplugin_CacheDownloadHttpClient_onDataChunk(
    JNIEnv* env, jobject /* thiz */, jlong callbackPtr, jbyteArray data)
{
    auto* req = FindRequest(env, callbackPtr);
    if (req == nullptr || data == nullptr) {
        return;
    }
    jsize len = env->GetArrayLength(data);
    if (len <= 0) {
        return;
    }
    jbyte* bytes = env->GetByteArrayElements(data, nullptr);
    if (bytes == nullptr) {
        // OOM: GetByteArrayElements returned nullptr and threw OutOfMemoryError.
        // Clear it so subsequent JNI calls are safe, and abort this chunk.
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
        }
        return;
    }
    // Callback ignored for flow-control in this version.
    (void)req->Callback()->OnData(reinterpret_cast<const uint8_t*>(bytes), static_cast<size_t>(len));
    env->ReleaseByteArrayElements(data, bytes, JNI_ABORT);
}

extern "C" JNIEXPORT void JNICALL
Java_ohos_ace_plugin_cachedownloadplugin_CacheDownloadHttpClient_onHeader(
    JNIEnv* env, jobject /* thiz */, jlong callbackPtr, jstring key, jstring value)
{
    auto* req = FindRequest(env, callbackPtr);
    if (req == nullptr || key == nullptr || value == nullptr) {
        return;
    }
    req->Callback()->OnHeader(JstringToString(env, key), JstringToString(env, value));
}

extern "C" JNIEXPORT void JNICALL
Java_ohos_ace_plugin_cachedownloadplugin_CacheDownloadHttpClient_onProgress(
    JNIEnv* env, jobject /* thiz */, jlong callbackPtr, jlong downloaded, jlong total)
{
    auto* req = FindRequest(env, callbackPtr);
    if (req == nullptr) {
        return;
    }
    req->Callback()->OnProgress(static_cast<uint64_t>(downloaded),
        static_cast<uint64_t>(total));
}

extern "C" JNIEXPORT void JNICALL
Java_ohos_ace_plugin_cachedownloadplugin_CacheDownloadHttpClient_onSuccess(
    JNIEnv* env, jobject /* thiz */, jlong callbackPtr, jint statusCode)
{
    auto* req = FindRequest(env, callbackPtr);
    if (req == nullptr) {
        return;
    }
    CacheHttpResponseInfo info;
    info.statusCode = static_cast<int>(statusCode);
    req->Callback()->OnSuccess(info);
    req->Finish();
}

extern "C" JNIEXPORT void JNICALL
Java_ohos_ace_plugin_cachedownloadplugin_CacheDownloadHttpClient_onFailed(
    JNIEnv* env, jobject /* thiz */, jlong callbackPtr, jint errorCode, jstring message)
{
    auto* req = FindRequest(env, callbackPtr);
    if (req == nullptr) {
        return;
    }
    // Use a default message when Java passes null — otherwise the download
    // would hang forever (no callback, no cleanup).
    std::string msg = (message != nullptr) ? JstringToString(env, message) : "Unknown error";
    req->Callback()->OnFailed(static_cast<int>(errorCode), msg);
    req->Finish();
}

extern "C" JNIEXPORT void JNICALL
Java_ohos_ace_plugin_cachedownloadplugin_CacheDownloadHttpClient_onMetrics(
    JNIEnv* env, jobject /* thiz */, jlong callbackPtr, jobject metricsObj)
{
    auto* req = FindRequest(env, callbackPtr);
    if (req == nullptr || metricsObj == nullptr) {
        return;
    }
    // Read the RequestMetrics fields reflectively from the runtime class —
    // GetObjectClass avoids the ClassLoader considerations of FindClass.
    jclass metricsCls = env->GetObjectClass(metricsObj);
    if (metricsCls == nullptr) {
        return;
    }
    jfieldID serverIpId = env->GetFieldID(metricsCls, "serverIp", "Ljava/lang/String;");
    jfieldID dnsTimeId = env->GetFieldID(metricsCls, "dnsTimeMs", "J");
    jfieldID connectTimeId = env->GetFieldID(metricsCls, "connectTimeMs", "J");
    jfieldID tlsTimeId = env->GetFieldID(metricsCls, "tlsTimeMs", "J");
    jfieldID firstSendTimeId = env->GetFieldID(metricsCls, "firstSendTimeMs", "J");
    jfieldID firstRecvTimeId = env->GetFieldID(metricsCls, "firstRecvTimeMs", "J");
    jfieldID totalTimeId = env->GetFieldID(metricsCls, "totalTimeMs", "J");
    jfieldID redirectTimeId = env->GetFieldID(metricsCls, "redirectTimeMs", "J");
    if (env->ExceptionCheck()) {
        // A field lookup failed (signature drift): clear and drop the metrics.
        env->ExceptionClear();
        env->DeleteLocalRef(metricsCls);
        return;
    }
    CacheHttpMetrics metrics;
    jstring serverIp = static_cast<jstring>(env->GetObjectField(metricsObj, serverIpId));
    if (serverIp != nullptr) {
        metrics.serverIp = JstringToString(env, serverIp);
        env->DeleteLocalRef(serverIp);
    }
    // All timings are cumulative ms since request start (OHOS curl semantics).
    metrics.dnsTime = static_cast<double>(env->GetLongField(metricsObj, dnsTimeId));
    metrics.connectTime = static_cast<double>(env->GetLongField(metricsObj, connectTimeId));
    metrics.tlsTime = static_cast<double>(env->GetLongField(metricsObj, tlsTimeId));
    metrics.firstSendTime = static_cast<double>(env->GetLongField(metricsObj, firstSendTimeId));
    metrics.firstReceiveTime = static_cast<double>(env->GetLongField(metricsObj, firstRecvTimeId));
    metrics.totalTime = static_cast<double>(env->GetLongField(metricsObj, totalTimeId));
    metrics.redirectTime = static_cast<double>(env->GetLongField(metricsObj, redirectTimeId));
    env->DeleteLocalRef(metricsCls);
    req->Callback()->OnMetrics(metrics);
}

CacheHttpClientAndroid::CacheHttpClientAndroid() = default;

CacheHttpClientAndroid::~CacheHttpClientAndroid()
{
    if (clientRef_ != nullptr) {
        JNIEnv* env = ARKUI_X_Plugin_GetJniEnv();
        if (env != nullptr) {
            env->DeleteGlobalRef(clientRef_);
        }
    }
}

// Caches the RequestConfig inner class and its constructor (same ClassLoader
// consideration as the HTTP client class). Returns false on failure, in which
// case InitializeJni must abort (sJniCacheReady stays false).
bool CacheHttpClientAndroid::CacheRequestConfigJni(JNIEnv* env)
{
    jclass configCls = SafeFindClass(env,
        "ohos/ace/plugin/cachedownloadplugin/CacheDownloadHttpClient$RequestConfig");
    if (configCls == nullptr) {
        CDLOG_ERROR("InitializeJni SafeFindClass(RequestConfig) FAILED -> return");
        return false;
    }
    sConfigClass = static_cast<jclass>(env->NewGlobalRef(configCls));
    env->DeleteLocalRef(configCls);
    if (sConfigClass == nullptr) {
        return false;
    }
    sConfigCtorId = env->GetMethodID(sConfigClass, "<init>",
        "(Ljava/lang/String;Ljava/util/Map;IILjava/lang/String;)V");
    if (sConfigCtorId == nullptr && env->ExceptionCheck()) {
        env->ExceptionClear();
    }
    return true;
}

void CacheHttpClientAndroid::InitializeJni(JNIEnv* env)
{
    CDLOG_INFO("InitializeJni ENTER envNonNull=" CDLOG_FMT_D " sJniCacheReady=" CDLOG_FMT_D,
        env != nullptr ? 1 : 0, (int)sJniCacheReady);
    if (sJniCacheReady || env == nullptr) {
        CDLOG_INFO("InitializeJni early-return sJniCacheReady=" CDLOG_FMT_D, (int)sJniCacheReady);
        return;
    }

    // Use SafeFindClass to find the CacheDownloadHttpClient class. When called
    // from NativeInit, the JNIEnv comes from Java and has the correct
    // ClassLoader to see plugin jar classes. ARKUI_X_Plugin_GetJniEnv() may
    // return a JNIEnv whose FindClass uses the system ClassLoader, which
    // cannot find plugin classes — so we must cache the class here.
    jclass cls = SafeFindClass(env, HTTP_CLIENT_CLASS_NAME);
    CDLOG_INFO("InitializeJni SafeFindClass found=" CDLOG_FMT_D, cls != nullptr ? 1 : 0);
    if (cls == nullptr) {
        CDLOG_ERROR("InitializeJni SafeFindClass FAILED (wrong ClassLoader?) -> return");
        return;
    }
    sHttpClientClass = static_cast<jclass>(env->NewGlobalRef(cls));
    env->DeleteLocalRef(cls);

    if (sHttpClientClass == nullptr) {
        CDLOG_ERROR("InitializeJni NewGlobalRef FAILED -> return");
        return;
    }

    sCtorMethodId = env->GetMethodID(sHttpClientClass, "<init>", "()V");
    if (sCtorMethodId == nullptr && env->ExceptionCheck()) {
        env->ExceptionClear();
    }

    sExecuteMethodId = env->GetMethodID(sHttpClientClass,
        EXEC_REQUEST_METHOD_NAME, EXEC_REQUEST_METHOD_SIG);
    if (sExecuteMethodId == nullptr && env->ExceptionCheck()) {
        env->ExceptionClear();
    }

    if (!CacheRequestConfigJni(env)) {
        return;
    }

    sJniCacheReady = (sCtorMethodId != nullptr && sExecuteMethodId != nullptr
                      && sConfigClass != nullptr && sConfigCtorId != nullptr);
    CDLOG_INFO("InitializeJni result sJniCacheReady=" CDLOG_FMT_D
        " (ctorNonNull=" CDLOG_FMT_D " execNonNull=" CDLOG_FMT_D
        " configClsNonNull=" CDLOG_FMT_D " configCtorNonNull=" CDLOG_FMT_D ")",
        (int)sJniCacheReady, sCtorMethodId != nullptr ? 1 : 0, sExecuteMethodId != nullptr ? 1 : 0,
        sConfigClass != nullptr ? 1 : 0, sConfigCtorId != nullptr ? 1 : 0);
    if (sJniCacheReady) {
        // Notify any EnsureJniReady waiter that triggered registration
        // lazily from the innerkits (SO1) path.
        CDLOG_INFO("InitializeJni notify condvar (sJniCacheReady=true)");
        std::lock_guard<std::mutex> lock(g_jniReadyMutex);
        g_jniReadyCv.notify_all();
    }
}

jobject CacheHttpClientAndroid::CreateRequestConfig(JNIEnv* env, const CacheHttpRequestOptions& opts)
{
    if (env == nullptr || sConfigClass == nullptr || sConfigCtorId == nullptr) {
        return nullptr;
    }
    jstring jmethod = env->NewStringUTF(opts.method.c_str());
    jstring jcaPath = env->NewStringUTF(opts.caPath.c_str());
    jobject jheaders = BuildJniHeadersMap(env, opts.headers);
    jobject config = env->NewObject(sConfigClass, sConfigCtorId, jmethod, jheaders,
        static_cast<jint>(opts.connectTimeoutMs),
        static_cast<jint>(opts.readTimeoutMs),
        jcaPath);
    env->DeleteLocalRef(jmethod);
    env->DeleteLocalRef(jcaPath);
    if (jheaders != nullptr) {
        env->DeleteLocalRef(jheaders);
    }
    if (config == nullptr && env->ExceptionCheck()) {
        env->ExceptionClear();
    }
    return config;
}

// Registers native methods with the JVM using the cached class (correct
// ClassLoader). Called from EnsureJniReady under a double-checked lock.
bool RegisterNativeMethods(JNIEnv* env, jclass cls)
{
    static const JNINativeMethod methods[] = {
        { "onDataChunk", "(J[B)V",
          reinterpret_cast<void*>(&Java_ohos_ace_plugin_cachedownloadplugin_CacheDownloadHttpClient_onDataChunk) },
        { "onHeader", "(JLjava/lang/String;Ljava/lang/String;)V",
          reinterpret_cast<void*>(&Java_ohos_ace_plugin_cachedownloadplugin_CacheDownloadHttpClient_onHeader) },
        { "onProgress", "(JJJ)V",
          reinterpret_cast<void*>(&Java_ohos_ace_plugin_cachedownloadplugin_CacheDownloadHttpClient_onProgress) },
        { "onSuccess", "(JI)V",
          reinterpret_cast<void*>(&Java_ohos_ace_plugin_cachedownloadplugin_CacheDownloadHttpClient_onSuccess) },
        { "onFailed", "(JILjava/lang/String;)V",
          reinterpret_cast<void*>(&Java_ohos_ace_plugin_cachedownloadplugin_CacheDownloadHttpClient_onFailed) },
        { "onMetrics",
          "(JLohos/ace/plugin/cachedownloadplugin/CacheDownloadHttpClient$RequestMetrics;)V",
          reinterpret_cast<void*>(&Java_ohos_ace_plugin_cachedownloadplugin_CacheDownloadHttpClient_onMetrics) },
    };
    if (env->RegisterNatives(cls, methods, sizeof(methods) / sizeof(methods[0])) != 0) {
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
        }
        return false;
    }
    g_nativesRegistered.store(true);
    return true;
}

bool CacheHttpClientAndroid::EnsureJniReady(JNIEnv* env)
{
    // Use cached class/methodIDs if available (initialized from NativeInit
    // with the correct ClassLoader). If not cached yet, trigger the Java
    // plugin registration which runs CacheDownloadJni::Register on the
    // PLATFORM thread with the correct ClassLoader, then wait for
    // InitializeJni to set sJniCacheReady. This makes SO1 self-contained:
    // Image can use the innerkits without SO2 being loaded by a JS import,
    // matching the native HarmonyOS design where libpreload_native.so is
    // self-contained.
    CDLOG_INFO("EnsureJniReady ENTER envNonNull=" CDLOG_FMT_D " sJniCacheReady=" CDLOG_FMT_D
        " g_jniRegisterTriggered=" CDLOG_FMT_D,
        env != nullptr ? 1 : 0, (int)sJniCacheReady, (int)g_jniRegisterTriggered.load());
    if (!sJniCacheReady) {
        if (!g_jniRegisterTriggered.exchange(true)) {
            CDLOG_INFO("EnsureJniReady lazy-trigger ARKUI_X_Plugin_RegisterJavaPlugin (first time)");
            ARKUI_X_Plugin_RegisterJavaPlugin(
                &CacheDownloadJni::Register,
                "ohos.ace.plugin.cachedownloadplugin.CacheDownloadPlugin");
        } else {
            CDLOG_INFO("EnsureJniReady lazy-trigger already fired by prior call, waiting on condvar");
        }
        CDLOG_INFO("EnsureJniReady condvar wait_for 5s start");
        std::unique_lock<std::mutex> lock(g_jniReadyMutex);
        if (!g_jniReadyCv.wait_for(lock, std::chrono::seconds(5),
            [] { return sJniCacheReady; })) {
            CDLOG_ERROR("EnsureJniReady TIMEOUT 5s sJniCacheReady still false -> return false");
            return false; // timed out, JNI cache still not ready
        }
        CDLOG_INFO("EnsureJniReady condvar notified sJniCacheReady=true");
    }

    if (!g_nativesRegistered.load()) {
        std::lock_guard<std::mutex> lock(g_registerMutex);
        if (!g_nativesRegistered.load()) {
            if (!RegisterNativeMethods(env, sHttpClientClass)) {
                return false;
            }
        }
    }

    if (clientRef_ != nullptr) {
        return true;
    }

    // Create Java CacheDownloadHttpClient instance using cached class.
    jobject localRef = env->NewObject(sHttpClientClass, sCtorMethodId);
    if (localRef == nullptr) {
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
        }
        return false;
    }
    clientRef_ = env->NewGlobalRef(localRef);
    env->DeleteLocalRef(localRef);

    if (clientRef_ == nullptr) {
        return false;
    }

    executeRequestId_ = sExecuteMethodId;
    return executeRequestId_ != nullptr;
}

std::shared_ptr<CacheHttpRequest> CacheHttpClientAndroid::NewRequest(
    const CacheHttpRequestOptions& options,
    const std::shared_ptr<CacheHttpRequestCallback>& callback)
{
    JNIEnv* env = ARKUI_X_Plugin_GetJniEnv();
    CDLOG_INFO("NewRequest taskId=" CDLOG_FMT_PUBLIC_S " envNonNull=" CDLOG_FMT_D
        " sJniCacheReady=" CDLOG_FMT_D,
        GetBriefTaskId(options.url).c_str(), env != nullptr ? 1 : 0, (int)sJniCacheReady);
    if (env == nullptr || !EnsureJniReady(env)) {
        CDLOG_ERROR("NewRequest FAIL taskId=" CDLOG_FMT_PUBLIC_S
            " EnsureJniReady=false (Failed to create HTTP request)",
            GetBriefTaskId(options.url).c_str());
        return nullptr;
    }

    auto request = std::make_shared<CacheHttpRequestAndroid>(
        clientRef_, executeRequestId_, options, callback);
    CDLOG_INFO("NewRequest OK taskId=" CDLOG_FMT_PUBLIC_S, GetBriefTaskId(options.url).c_str());
    return request;
}

bool CacheHttpRequestAndroid::Start()
{
    CDLOG_INFO("CacheHttpRequestAndroid::Start taskId=" CDLOG_FMT_PUBLIC_S,
               GetBriefTaskId(options_.url).c_str());
    // TLCP (GM/T 0024) is not supported by the target platform's TLS stack.
    // Fail explicitly with an SSL-class error instead of silently downgrading
    // to plain TLS — matches iOS behavior and OHOS semantics where a TLCP
    // handshake against a non-TLCP peer fails.
    if (options_.sslType == SslType::TLCP) {
        if (callback_) {
            callback_->OnFailed(static_cast<int>(ErrorCode::SSL),
                "TLCP is not supported on this platform");
        }
        return false;
    }

    JNIEnv* env = ARKUI_X_Plugin_GetJniEnv();
    if (env == nullptr || clientRef_ == nullptr || executeRequestId_ == nullptr) {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(g_requestsMutex);
        g_requests[reinterpret_cast<uintptr_t>(this)] = shared_from_this();
    }

    jlong callbackPtr = static_cast<jlong>(reinterpret_cast<uintptr_t>(this));
    jstring jurl = env->NewStringUTF(options_.url.c_str());
    jobject config = CacheHttpClientAndroid::CreateRequestConfig(env, options_);
    if (config == nullptr) {
        env->DeleteLocalRef(jurl);
        return false;
    }
    env->CallVoidMethod(clientRef_, executeRequestId_, callbackPtr, jurl, config);

    // Check for Java exceptions after CallVoidMethod. If executeRequest threw,
    // the request will never complete — clear the exception and report failure.
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        env->DeleteLocalRef(config);
        env->DeleteLocalRef(jurl);
        return false;
    }

    env->DeleteLocalRef(config);
    env->DeleteLocalRef(jurl);
    return true;
}

void CacheHttpRequestAndroid::Cancel()
{
    CDLOG_INFO("CacheHttpRequestAndroid::Cancel taskId=" CDLOG_FMT_PUBLIC_S,
               GetBriefTaskId(options_.url).c_str());
    Finish();
}

void CacheHttpRequestAndroid::Finish()
{
    bool alreadyFinished = finished_.exchange(true);
    if (alreadyFinished) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_requestsMutex);
    g_requests.erase(reinterpret_cast<uintptr_t>(this));
}

std::shared_ptr<CacheHttpClient> CreateCacheHttpClient()
{
    return std::make_shared<CacheHttpClientAndroid>();
}

} // namespace OHOS::Plugin::CacheDownload
