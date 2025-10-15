/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "plugins/web/webview/android/java/jni/web_scheme_handler_jni.h"

#include <jni.h>
#include <unordered_map>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"

namespace OHOS::Plugin {
static constexpr const int PARAM_COUNT_ZERO = 0;
static constexpr const int PARAM_COUNT_ONE = 1;
static constexpr const int PARAM_COUNT_TWO = 2;
static constexpr const int PARAM_COUNT_THREE = 3;
static constexpr const int PARAM_COUNT_FOUR = 4;
std::map<std::string, ArkWeb_SchemeHandler*> WebSchemeHandlerJni::g_schemeRequestHandlerMap_;
ArkWeb_SchemeHandler* WebSchemeHandlerJni::handler_ = nullptr;
ArkWeb_ResourceRequest* WebSchemeHandlerJni::resourceRequest_ = nullptr;
bool WebSchemeHandlerJni::isIntercept_ = false;
namespace {
const char WEB_WEBVIEW_CLASS_NAME[] = "ohos/ace/adapter/capability/web/AceWebPluginBase";
static const JNINativeMethod METHODS[] = {
    { "nativeInitSchemeHandler", "()V", reinterpret_cast<void*>(WebSchemeHandlerJni::NativeInit) },
    { "onSchemeHandlerRequestStart", "(Ljava/lang/String;Ljava/lang/Object;)Z",
        reinterpret_cast<void*>(WebSchemeHandlerJni::OnSchemeHandlerRequestStart) },
    { "onSchemeHandlerRequestStop", "(Ljava/lang/String;Ljava/lang/Object;)V",
        reinterpret_cast<void*>(WebSchemeHandlerJni::OnSchemeHandlerRequestStop) },
};

struct {
    jobject globalRef;
} g_webWebviewClass;
} // namespace

bool WebSchemeHandlerJni::Register(void* env)
{
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(WEB_WEBVIEW_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("WebSchemeHandlerJni JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void WebSchemeHandlerJni::NativeInit(JNIEnv* env, jobject jobj)
{
    CHECK_NULL_VOID(env);
}

std::string WebSchemeHandlerJni::GetUTFString(JNIEnv* env, jstring jstr)
{
    CHECK_NULL_RETURN(env, "");
    CHECK_NULL_RETURN(jstr, "");
    const char* pString = env->GetStringUTFChars(jstr, nullptr);
    CHECK_NULL_RETURN(pString, "");
    std::string str(pString);
    env->ReleaseStringUTFChars(jstr, pString);
    return str;
}

bool WebSchemeHandlerJni::OnSchemeHandlerRequestStart(
    JNIEnv* env, jclass jcls, jstring scheme, jobject schemeRequestHandler)
{
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(scheme, false);
    std::string strScheme = WebSchemeHandlerJni::GetUTFString(env, scheme);
    auto it = g_schemeRequestHandlerMap_.find(strScheme);
    if (it == g_schemeRequestHandlerMap_.end()) {
        LOGE("WebSchemeHandlerJni::OnSchemeHandlerRequestStart: No handler found for scheme %{public}s",
            strScheme.c_str());
        return false;
    }

    handler_ = it->second;
    CHECK_NULL_RETURN(handler_, false);
    bool intercept = false;
    if (handler_->on_request_start != nullptr) {
        resourceRequest_ = CreateArkResourceRequest(env, schemeRequestHandler);
        CHECK_NULL_RETURN(resourceRequest_, false);
        ArkWeb_ResourceHandler* resourceHandler = new (std::nothrow) ArkWeb_ResourceHandler(env, schemeRequestHandler);
        CHECK_NULL_RETURN(resourceHandler, false);
        handler_->on_request_start(handler_, resourceRequest_, resourceHandler, &intercept);
        SetIsIntercept(intercept);
        DestroyArkResourceRequest(resourceRequest_);
        delete resourceHandler;
        resourceHandler = nullptr;
    }
    return intercept;
}

void WebSchemeHandlerJni::OnSchemeHandlerRequestStop(
    JNIEnv* env, jclass jcls, jstring scheme, jobject schemeRequestHandler)
{
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(scheme);
    std::string strScheme = WebSchemeHandlerJni::GetUTFString(env, scheme);
    auto it = g_schemeRequestHandlerMap_.find(strScheme);
    if (it == g_schemeRequestHandlerMap_.end()) {
        LOGE("WebSchemeHandlerJni::OnSchemeHandlerRequestStop: No handler found for scheme %{public}s",
            strScheme.c_str());
        return;
    }

    handler_ = it->second;
    CHECK_NULL_VOID(handler_);
    if (handler_->on_request_stop != nullptr) {
        resourceRequest_ = CreateArkResourceRequest(env, schemeRequestHandler);
        CHECK_NULL_VOID(resourceRequest_);
        handler_->on_request_stop(handler_, resourceRequest_);
        DestroyArkResourceRequest(resourceRequest_);
    }
}

template<typename T>
static bool IsNullAndRelease(T obj, ArkWeb_ResourceRequest*& request)
{
    if (obj == nullptr) {
        if (request != nullptr) {
            delete request;
            request = nullptr;
        }
        return true;
    }
    return false;
}

bool WebSchemeHandlerJni::GetStringFromJava(JNIEnv* env, jclass clazz, jobject jobj, ArkWeb_ResourceRequest*& request,
    const char* methodName, std::string& outStr)
{
    jmethodID methodId = env->GetMethodID(clazz, methodName, "()Ljava/lang/String;");
    if (IsNullAndRelease(methodId, request)) {
        return false;
    }
    jstring jstr = static_cast<jstring>(env->CallObjectMethod(jobj, methodId));
    if (IsNullAndRelease(jstr, request)) {
        return false;
    }
    outStr = WebSchemeHandlerJni::GetUTFString(env, jstr);
    env->DeleteLocalRef(jstr);
    return true;
}

bool WebSchemeHandlerJni::GetBooleanFromJava(JNIEnv* env, jclass clazz, jobject jobj, ArkWeb_ResourceRequest*& request,
    const char* methodName, bool& outBool)
{
    jmethodID methodId = env->GetMethodID(clazz, methodName, "()Z");
    if (IsNullAndRelease(methodId, request)) {
        return false;
    }
    outBool = static_cast<bool>(env->CallBooleanMethod(jobj, methodId));
    return true;
}

ArkWeb_ResourceRequest* WebSchemeHandlerJni::CreateArkResourceRequest(JNIEnv* env, jobject jRequest)
{
    CHECK_NULL_RETURN(env, nullptr);
    ArkWeb_ResourceRequest* request = new (std::nothrow) ArkWeb_ResourceRequest();
    CHECK_NULL_RETURN(request, nullptr);
    jclass schemeRequestClass = env->GetObjectClass(jRequest);
    if (IsNullAndRelease(schemeRequestClass, request)) {
        return nullptr;
    }
    if (!GetStringFromJava(env, schemeRequestClass, jRequest, request, "getRequestUrl", request->url_)) {
        return nullptr;
    }
    if (!GetStringFromJava(env, schemeRequestClass, jRequest, request, "getMethod", request->method_)) {
        return nullptr;
    }
    if (!GetBooleanFromJava(env, schemeRequestClass, jRequest, request, "isRequestGesture", request->hasGesture_)) {
        return nullptr;
    }
    if (!GetBooleanFromJava(env, schemeRequestClass, jRequest, request, "isMainFrame", request->isMainFrame_)) {
        return nullptr;
    }
    if (!GetBooleanFromJava(env, schemeRequestClass, jRequest, request, "isRedirect", request->isRedirect_)) {
        return nullptr;
    }
    jmethodID getRequestHeaderId = env->GetMethodID(schemeRequestClass, "getRequestHeader", "()Ljava/util/Map;");
    if (IsNullAndRelease(getRequestHeaderId, request)) {
        return nullptr;
    }
    jobject jHeadersMap = env->CallObjectMethod(jRequest, getRequestHeaderId);
    if (IsNullAndRelease(jHeadersMap, request)) {
        return nullptr;
    }
    std::map<std::string, std::string> headers = GetRequestHeader(env, jHeadersMap);
    request->headerList_ = WebHeaderList(headers.begin(), headers.end());
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return request;
}

void WebSchemeHandlerJni::DestroyArkResourceRequest(ArkWeb_ResourceRequest* request)
{
    if (request != nullptr) {
        delete request;
        request = nullptr;
    }
}

void WebSchemeHandlerJni::IterateAndFillHeaders(JNIEnv* env, jobject iterator, const jmethodID* methodIds,
    std::map<std::string, std::string>& headers)
{
    while (env->CallBooleanMethod(iterator, methodIds[PARAM_COUNT_ZERO])) {
        jobject entry = env->CallObjectMethod(iterator, methodIds[PARAM_COUNT_ONE]);
        jobject key = env->CallObjectMethod(entry, methodIds[PARAM_COUNT_TWO]);
        jobject value = env->CallObjectMethod(entry, methodIds[PARAM_COUNT_THREE]);
        if (key != nullptr && value != nullptr) {
            const char* keyStr = env->GetStringUTFChars(static_cast<jstring>(key), nullptr);
            const char* valueStr = env->GetStringUTFChars(static_cast<jstring>(value), nullptr);
            if (keyStr != nullptr && valueStr != nullptr) {
                headers[keyStr] = valueStr;
            }
            if (keyStr != nullptr) {
                env->ReleaseStringUTFChars(static_cast<jstring>(key), keyStr);
            }
            if (valueStr != nullptr) {
                env->ReleaseStringUTFChars(static_cast<jstring>(value), valueStr);
            }
        }
        env->DeleteLocalRef(key);
        env->DeleteLocalRef(value);
        env->DeleteLocalRef(entry);
    }
}

bool WebSchemeHandlerJni::GetIteratorAndEntryMethodIds(
    JNIEnv* env, jclass iteratorClass, jclass entryClass, jmethodID* methodIds)
{
    if (methodIds == nullptr) {
        return false;
    }
    methodIds[PARAM_COUNT_ZERO] = env->GetMethodID(iteratorClass, "hasNext", "()Z");
    methodIds[PARAM_COUNT_ONE] = env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");
    methodIds[PARAM_COUNT_TWO] = env->GetMethodID(entryClass, "getKey", "()Ljava/lang/Object;");
    methodIds[PARAM_COUNT_THREE] = env->GetMethodID(entryClass, "getValue", "()Ljava/lang/Object;");
    return (methodIds[PARAM_COUNT_ZERO] != nullptr) && (methodIds[PARAM_COUNT_ONE] != nullptr) &&
           (methodIds[PARAM_COUNT_TWO] != nullptr) && (methodIds[PARAM_COUNT_THREE] != nullptr);
}

std::map<std::string, std::string> WebSchemeHandlerJni::GetRequestHeader(JNIEnv* env, jobject jHeadersMap)
{
    std::map<std::string, std::string> headers;
    CHECK_NULL_RETURN(env, headers);
    CHECK_NULL_RETURN(jHeadersMap, headers);
    jobject iterator = WebSchemeHandlerJni::GetHeaderIterator(env, jHeadersMap);
    CHECK_NULL_RETURN(iterator, headers);

    jclass iteratorClass = env->FindClass("java/util/Iterator");
    jclass entryClass = env->FindClass("java/util/Map$Entry");
    if (iteratorClass == nullptr || entryClass == nullptr) {
        env->DeleteLocalRef(iterator);
        env->DeleteLocalRef(iteratorClass);
        env->DeleteLocalRef(entryClass);
        return headers;
    }

    jmethodID methodIds[PARAM_COUNT_FOUR] = { nullptr };
    if (GetIteratorAndEntryMethodIds(env, iteratorClass, entryClass, methodIds)) {
        IterateAndFillHeaders(env, iterator, methodIds, headers);
    }

    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(iterator);
    env->DeleteLocalRef(iteratorClass);
    env->DeleteLocalRef(entryClass);
    return headers;
}

jobject WebSchemeHandlerJni::GetHeaderIterator(JNIEnv* env, jobject jHeadersMap)
{
    CHECK_NULL_RETURN(env, nullptr);
    CHECK_NULL_RETURN(jHeadersMap, nullptr);
    jclass mapClass = env->FindClass("java/util/Map");
    CHECK_NULL_RETURN(mapClass, nullptr);
    jmethodID entrySetId = env->GetMethodID(mapClass, "entrySet", "()Ljava/util/Set;");
    if (entrySetId == nullptr) {
        env->DeleteLocalRef(mapClass);
        return nullptr;
    }
    jobject entrySet = env->CallObjectMethod(jHeadersMap, entrySetId);
    if (entrySet == nullptr) {
        env->DeleteLocalRef(mapClass);
        return nullptr;
    }

    jclass setClass = env->FindClass("java/util/Set");
    if (setClass == nullptr) {
        env->DeleteLocalRef(entrySet);
        env->DeleteLocalRef(mapClass);
        return nullptr;
    }
    jmethodID iteratorId = env->GetMethodID(setClass, "iterator", "()Ljava/util/Iterator;");
    if (iteratorId == nullptr) {
        env->DeleteLocalRef(entrySet);
        env->DeleteLocalRef(mapClass);
        env->DeleteLocalRef(setClass);
        return nullptr;
    }
    jobject iterator = env->CallObjectMethod(entrySet, iteratorId);
    env->DeleteLocalRef(entrySet);
    env->DeleteLocalRef(mapClass);
    env->DeleteLocalRef(setClass);
    return iterator;
}

ArkWeb_SchemeHandler* WebSchemeHandlerJni::CreateArkSchemeHandler()
{
    ArkWeb_SchemeHandler* handler = new (std::nothrow) ArkWeb_SchemeHandler();
    return handler;
}

void WebSchemeHandlerJni::DeleteArkSchemeHandler(ArkWeb_SchemeHandler* handler)
{
    if (handler != nullptr) {
        delete handler;
        handler = nullptr;
    }
}

void WebSchemeHandlerJni::SetOnRequestStart(ArkWeb_SchemeHandler* handler, ArkWeb_OnRequestStart callback)
{
    if (handler != nullptr) {
        handler->on_request_start = callback;
    }
}

void WebSchemeHandlerJni::SetOnRequestStop(ArkWeb_SchemeHandler* handler, ArkWeb_OnRequestStop callback)
{
    if (handler != nullptr) {
        handler->on_request_stop = callback;
    }
}

void WebSchemeHandlerJni::InsertSchemeRequestHandler(std::string& scheme, ArkWeb_SchemeHandler* handler)
{
    if (!scheme.empty() && handler != nullptr) {
        g_schemeRequestHandlerMap_[scheme] = handler;
    }
}

void WebSchemeHandlerJni::ClearSchemeRequestHandler()
{
    g_schemeRequestHandlerMap_.clear();
}

bool WebSchemeHandlerJni::GetIsIntercept()
{
    return isIntercept_;
}

void WebSchemeHandlerJni::SetIsIntercept(bool intercept)
{
    isIntercept_ = intercept;
}

ErrCode ArkWeb_ResourceHandler::SetResponseUrl(const std::string& url)
{
    CHECK_NULL_RETURN(env_, NWebError::PARAM_CHECK_ERROR);
    CHECK_NULL_RETURN(schemeRequestHandler_, NWebError::PARAM_CHECK_ERROR);
    jclass schemeRequestClass = env_->GetObjectClass(schemeRequestHandler_);
    CHECK_NULL_RETURN(schemeRequestClass, NWebError::PARAM_CHECK_ERROR);
    jmethodID setResponseUrlMethod = env_->GetMethodID(schemeRequestClass, "setResponseUrl", "(Ljava/lang/String;)V");
    CHECK_NULL_RETURN(setResponseUrlMethod, NWebError::PARAM_CHECK_ERROR);
    jstring jsUrl = env_->NewStringUTF(url.c_str());
    CHECK_NULL_RETURN(jsUrl, NWebError::PARAM_CHECK_ERROR);
    env_->CallVoidMethod(schemeRequestHandler_, setResponseUrlMethod, jsUrl);
    ErrCode ret = NWebError::NO_ERROR;
    if (env_->ExceptionCheck()) {
        env_->ExceptionDescribe();
        env_->ExceptionClear();
        ret = NWebError::PARAM_CHECK_ERROR;
    }
    env_->DeleteLocalRef(jsUrl);
    return ret;
}

ErrCode ArkWeb_ResourceHandler::SetResponseErrcode(ArkWeb_NetError errorCode, const std::string& errorDescription)
{
    CHECK_NULL_RETURN(env_, NWebError::PARAM_CHECK_ERROR);
    CHECK_NULL_RETURN(schemeRequestHandler_, NWebError::PARAM_CHECK_ERROR);
    jclass schemeRequestClass = env_->GetObjectClass(schemeRequestHandler_);
    CHECK_NULL_RETURN(schemeRequestClass, NWebError::PARAM_CHECK_ERROR);
    jmethodID setResponseErrorMethod =
        env_->GetMethodID(schemeRequestClass, "setResponseFail", "(ILjava/lang/String;)V");
    CHECK_NULL_RETURN(setResponseErrorMethod, NWebError::PARAM_CHECK_ERROR);
    jstring jsErrorDescription = env_->NewStringUTF(errorDescription.c_str());
    CHECK_NULL_RETURN(jsErrorDescription, NWebError::PARAM_CHECK_ERROR);
    env_->CallVoidMethod(
        schemeRequestHandler_, setResponseErrorMethod, static_cast<jint>(errorCode), jsErrorDescription);
    ErrCode ret = NWebError::NO_ERROR;
    if (env_->ExceptionCheck()) {
        env_->ExceptionDescribe();
        env_->ExceptionClear();
        ret = NWebError::PARAM_CHECK_ERROR;
    }
    env_->DeleteLocalRef(jsErrorDescription);
    return ret;
}

ErrCode ArkWeb_ResourceHandler::DidReceiveResponse(const ArkWeb_Response* response)
{
    CHECK_NULL_RETURN(response, NWebError::PARAM_CHECK_ERROR);
    CHECK_NULL_RETURN(env_, NWebError::PARAM_CHECK_ERROR);
    CHECK_NULL_RETURN(schemeRequestHandler_, NWebError::PARAM_CHECK_ERROR);
    jclass schemeRequestClass = env_->GetObjectClass(schemeRequestHandler_);
    CHECK_NULL_RETURN(schemeRequestClass, NWebError::PARAM_CHECK_ERROR);
    if (SetResponseUrl(response->url_) != NWebError::NO_ERROR) {
        return NWebError::PARAM_CHECK_ERROR;
    }

    if (SetResponseErrcode(static_cast<ArkWeb_NetError>(response->errorCode_), response->errorDescription_) !=
        NWebError::NO_ERROR) {
        return NWebError::PARAM_CHECK_ERROR;
    }
    jmethodID setResponseMethod = env_->GetMethodID(schemeRequestClass, "setResponse",
        "(Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;Ljava/util/HashMap;Ljava/lang/String;)V");
    CHECK_NULL_RETURN(setResponseMethod, NWebError::PARAM_CHECK_ERROR);
    jstring jsMimeType = env_->NewStringUTF(response->mimeType_.c_str());
    CHECK_NULL_RETURN(jsMimeType, NWebError::PARAM_CHECK_ERROR);
    jstring jsEncoding = env_->NewStringUTF(response->encoding_.c_str());
    if (jsEncoding == nullptr) {
        env_->DeleteLocalRef(jsMimeType);
        return NWebError::PARAM_CHECK_ERROR;
    }
    jstring jsReasonPhrase = env_->NewStringUTF(response->statusText_.c_str());
    jstring jsData = env_->NewStringUTF(response->data_.c_str());
    if (jsReasonPhrase == nullptr || jsData == nullptr) {
        env_->DeleteLocalRef(jsMimeType);
        env_->DeleteLocalRef(jsEncoding);
        if (jsReasonPhrase != nullptr) {
            env_->DeleteLocalRef(jsReasonPhrase);
        }
        return NWebError::PARAM_CHECK_ERROR;
    }
    jobject headers = ConvertMapToJavaMap(env_, response->headers_);
    env_->CallVoidMethod(schemeRequestHandler_, setResponseMethod, jsMimeType, jsEncoding, response->status_,
        jsReasonPhrase, headers, jsData);
    env_->DeleteLocalRef(jsMimeType);
    env_->DeleteLocalRef(jsEncoding);
    env_->DeleteLocalRef(jsReasonPhrase);
    env_->DeleteLocalRef(jsData);
    env_->DeleteLocalRef(headers);
    if (env_->ExceptionCheck()) {
        env_->ExceptionDescribe();
        env_->ExceptionClear();
    }
    return NWebError::NO_ERROR;
}

jobject ArkWeb_ResourceHandler::ConvertMapToJavaMap(
    JNIEnv* env, const std::unordered_map<std::string, std::string>& headersMap)
{
    CHECK_NULL_RETURN(env, nullptr);
    jclass hashMapClass = env->FindClass("java/util/HashMap");
    CHECK_NULL_RETURN(hashMapClass, nullptr);
    jmethodID hashMapConstructor = env->GetMethodID(hashMapClass, "<init>", "()V");
    jmethodID hashMapPut =
        env->GetMethodID(hashMapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    if (hashMapConstructor == nullptr || hashMapPut == nullptr) {
        env->DeleteLocalRef(hashMapClass);
        return nullptr;
    }
    jobject hashMap = env->NewObject(hashMapClass, hashMapConstructor);
    for (const auto& entry : headersMap) {
        jstring key = env->NewStringUTF(entry.first.c_str());
        jstring value = env->NewStringUTF(entry.second.c_str());
        env->CallObjectMethod(hashMap, hashMapPut, key, value);
        env->DeleteLocalRef(key);
        env->DeleteLocalRef(value);
    }
    env->DeleteLocalRef(hashMapClass);
    return hashMap;
}

ErrCode ArkWeb_ResourceHandler::DidReceiveData(const uint8_t* buffer, int64_t buflen)
{
    CHECK_NULL_RETURN(buffer, NWebError::PARAM_CHECK_ERROR);
    CHECK_NULL_RETURN(buflen > 0, NWebError::PARAM_CHECK_ERROR);
    CHECK_NULL_RETURN(env_, NWebError::PARAM_CHECK_ERROR);
    CHECK_NULL_RETURN(schemeRequestHandler_, NWebError::PARAM_CHECK_ERROR);
    std::string data(reinterpret_cast<const char*>(buffer), static_cast<size_t>(buflen));
    jclass schemeRequestClass = env_->GetObjectClass(schemeRequestHandler_);
    CHECK_NULL_RETURN(schemeRequestClass, NWebError::PARAM_CHECK_ERROR);

    jmethodID setResponseDataMethod = env_->GetMethodID(schemeRequestClass, "setResponseData", "(Ljava/lang/String;)V");
    CHECK_NULL_RETURN(setResponseDataMethod, NWebError::PARAM_CHECK_ERROR);
    jstring jsResponseData = env_->NewStringUTF(data.c_str());
    CHECK_NULL_RETURN(jsResponseData, NWebError::PARAM_CHECK_ERROR);
    env_->CallVoidMethod(schemeRequestHandler_, setResponseDataMethod, jsResponseData);
    ErrCode ret = NWebError::NO_ERROR;
    if (env_->ExceptionCheck()) {
        env_->ExceptionDescribe();
        env_->ExceptionClear();
        ret = NWebError::PARAM_CHECK_ERROR;
    }
    env_->DeleteLocalRef(jsResponseData);
    return ret;
}

ErrCode ArkWeb_ResourceHandler::DidFinish()
{
    CHECK_NULL_RETURN(env_, NWebError::PARAM_CHECK_ERROR);
    CHECK_NULL_RETURN(schemeRequestHandler_, NWebError::PARAM_CHECK_ERROR);

    jclass schemeRequestClass = env_->GetObjectClass(schemeRequestHandler_);
    CHECK_NULL_RETURN(schemeRequestClass, NWebError::PARAM_CHECK_ERROR);

    jmethodID setResponseFinishMethod = env_->GetMethodID(schemeRequestClass, "setResponseFinish", "(Z)V");
    CHECK_NULL_RETURN(setResponseFinishMethod, NWebError::PARAM_CHECK_ERROR);
    SetFinishFlag();
    env_->CallVoidMethod(schemeRequestHandler_, setResponseFinishMethod, isFinished_);
    ErrCode ret = NWebError::NO_ERROR;
    if (env_->ExceptionCheck()) {
        env_->ExceptionDescribe();
        env_->ExceptionClear();
        ret = NWebError::PARAM_CHECK_ERROR;
    }
    return ret;
}

ErrCode ArkWeb_ResourceHandler::DidFailWithError(
    ArkWeb_NetError errorCode, const std::string& errorDescription, bool completeIfNoResponse)
{
    CHECK_NULL_RETURN(env_, NWebError::PARAM_CHECK_ERROR);
    CHECK_NULL_RETURN(schemeRequestHandler_, NWebError::PARAM_CHECK_ERROR);

    jclass schemeRequestClass = env_->GetObjectClass(schemeRequestHandler_);
    CHECK_NULL_RETURN(schemeRequestClass, NWebError::PARAM_CHECK_ERROR);

    jmethodID didFailMethod = env_->GetMethodID(schemeRequestClass, "didFail", "(ILjava/lang/String;Z)V");
    if (didFailMethod == nullptr) {
        LOGE("ArkWeb_ResourceHandler::DidFailWithError get didFail method failed");
        return NWebError::PARAM_CHECK_ERROR;
    }
    jstring jsErrorDescription = env_->NewStringUTF(errorDescription.c_str());
    CHECK_NULL_RETURN(jsErrorDescription, NWebError::PARAM_CHECK_ERROR);
    env_->CallVoidMethod(schemeRequestHandler_, didFailMethod, static_cast<int32_t>(errorCode), jsErrorDescription,
        completeIfNoResponse);

    ErrCode ret = NWebError::NO_ERROR;
    if (env_->ExceptionCheck()) {
        env_->ExceptionDescribe();
        env_->ExceptionClear();
        ret = NWebError::PARAM_CHECK_ERROR;
    }
    env_->DeleteLocalRef(jsErrorDescription);
    return ret;
}

ArkWeb_ResourceHandler::~ArkWeb_ResourceHandler()
{
    if (schemeRequestHandler_ != nullptr && env_ != nullptr) {
        env_->DeleteLocalRef(schemeRequestHandler_);
        schemeRequestHandler_ = nullptr;
    }
    if (handler_ != nullptr) {
        delete handler_;
        handler_ = nullptr;
    }
}

void ArkWeb_ResourceHandler::DestroyArkWebResourceHandler()
{
    isFinished_ = false;
    if (schemeRequestHandler_ != nullptr && env_ != nullptr) {
        env_->DeleteLocalRef(schemeRequestHandler_);
        schemeRequestHandler_ = nullptr;
    }
}
} // namespace OHOS::Plugin