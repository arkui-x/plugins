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

#include "plugins/web/webview/android/java/jni/web_storage_jni.h"

#include <jni.h>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"
#include <string>

using namespace OHOS::NWebError;

namespace OHOS::Plugin {
namespace {
const char WEB_WEBVIEW_CLASS_NAME[] = "ohos/ace/plugin/webviewplugin/webstorage/WebStoragePlugin";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(WebStorageJni::NativeInit) },
    { "onReceiveStorageValue", "(JJ)V", reinterpret_cast<void*>(WebStorageJni::OnReceiveGetOriginQuotaValue) },
    { "onReceiveStorageUsageValue", "(JJ)V", reinterpret_cast<void*>(WebStorageJni::OnReceiveGetOriginUsageValue) },
    { "onReceiveStorageValue", "(Ljava/util/ArrayList;J)V", reinterpret_cast<void*>(WebStorageJni::OnReceiveGetOriginsValue) },
};

static const char METHOD_GET_ORIGINQUOTA[] = "getOriginQuota";
static const char METHOD_GET_ORIGINUSAGE[] = "getOriginUsage";
static const char METHOD_GET_ORIGINS[] = "getOrigins";
static const char METHOD_DELETE_ALL_DATA[] = "deleteAllData";
static const char METHOD_DELETE_ORIGIN[] = "deleteOrigin";

static const char SIGNATURE_GET_ORIGINQUOTA[] = "(Ljava/lang/String;J)V";
static const char SIGNATURE_GET_ORIGINUSAGE[] = "(Ljava/lang/String;J)V";
static const char SIGNATURE_GET_ORIGINS[] = "(J)V";
static const char SIGNATURE_DELETE_ALL_DATA[] = "()V";
static const char SIGNATURE_DELETE_ORIGIN[] = "(Ljava/lang/String;)V";

struct {
    jmethodID getOriginQuota;
    jmethodID getOriginUsage;
    jmethodID getOrigins;
    jmethodID deleteAllData;
    jmethodID deleteOrigin;
    jobject globalRef;
} g_webWebviewClass;
} // namespace

bool WebStorageJni::Register(void* env)
{
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(WEB_WEBVIEW_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("WebStorageJni JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void WebStorageJni::NativeInit(JNIEnv* env, jobject jobj)
{
    CHECK_NULL_VOID(env);
    g_webWebviewClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_webWebviewClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);
    g_webWebviewClass.getOriginQuota = env->GetMethodID(cls, METHOD_GET_ORIGINQUOTA, SIGNATURE_GET_ORIGINQUOTA);
    g_webWebviewClass.getOriginUsage = env->GetMethodID(cls, METHOD_GET_ORIGINUSAGE, SIGNATURE_GET_ORIGINUSAGE);
    g_webWebviewClass.getOrigins = env->GetMethodID(cls, METHOD_GET_ORIGINS, SIGNATURE_GET_ORIGINS);
    g_webWebviewClass.deleteAllData = env->GetMethodID(cls, METHOD_DELETE_ALL_DATA, SIGNATURE_DELETE_ALL_DATA);
    g_webWebviewClass.deleteOrigin = env->GetMethodID(cls, METHOD_DELETE_ORIGIN, SIGNATURE_DELETE_ORIGIN);
    env->DeleteLocalRef(cls);
}

void WebStorageJni::OnReceiveGetOriginQuotaValue(JNIEnv* env, jobject jobj, jlong jResult, jint jId)
{
    CHECK_NULL_VOID(env);
    auto result = static_cast<int64_t>(jResult); 
    auto nativeId = static_cast<int32_t>(jId);
    WebStorage::OnGetOriginQuotaReceiveValue(result, nativeId);
}

void WebStorageJni::OnReceiveGetOriginUsageValue(JNIEnv* env, jobject jobj, jlong jResult, jint jId)
{
    CHECK_NULL_VOID(env);
    auto result = static_cast<int32_t>(jResult); 
    auto nativeId = static_cast<int32_t>(jId);
    WebStorage::OnGetOriginUsageReceiveValue(result, nativeId);
}

void WebStorageJni::OnReceiveGetOriginsValue(JNIEnv* env, jobject jobj, jobject jResult, jint jId)
{
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(jResult);
    jclass arrayListClass = env->FindClass("java/util/ArrayList");
    if (arrayListClass == NULL) {
        return;
    }
    
    jmethodID sizeMethod = env->GetMethodID(arrayListClass, "size", "()I");
    if (sizeMethod == NULL) {
        return;
    }
    
    jmethodID getMethod = env->GetMethodID(arrayListClass, "get", "(I)Ljava/lang/Object;");
    if (getMethod == NULL) {
        return;
    }

    jclass jsonObjectClass = env->FindClass("org/json/JSONObject");
    if (jsonObjectClass == NULL) {
        return;
    }

    jmethodID jsonObjectGetMethod = env->GetMethodID(jsonObjectClass, "get", "(Ljava/lang/String;)Ljava/lang/Object;");
    if (jsonObjectGetMethod == NULL) {
        return;
    }
    std::vector<WebStorageStruct> getOriginsVector;
    jint size = env->CallIntMethod(jResult, sizeMethod);
    for (jint i = 0; i < size; i++) {
        jobject element = env->CallObjectMethod(jResult, getMethod, i);

        if (env->IsInstanceOf(element, jsonObjectClass)) {
            WebStorageStruct originParams;
            jstring key = env->NewStringUTF("origin");
            jobject originValue = env->CallObjectMethod(element, jsonObjectGetMethod, key);
            env->DeleteLocalRef(key);
            if (env->IsInstanceOf(originValue, env->FindClass("java/lang/String"))) {
                jstring jstr = (jstring) originValue;
                const char *str = env->GetStringUTFChars(jstr, nullptr);
                if (str != NULL) {
                    originParams.origin = std::string(str);
                    env->ReleaseStringUTFChars(jstr,str);
                }
            }

            key = env->NewStringUTF("usage");
            jobject usageValue = env->CallObjectMethod(element, jsonObjectGetMethod, key);
            env->DeleteLocalRef(key);
            if (env->IsInstanceOf(usageValue, env->FindClass("java/lang/String"))) {
                jstring jstr = (jstring) usageValue;
                const char *str = env->GetStringUTFChars(jstr, nullptr);
                if (str != NULL) {
                    originParams.usage = std::string(str);
                    env->ReleaseStringUTFChars(jstr,str);
                }
            }

            key = env->NewStringUTF("quota");
            jobject quotaValue = env->CallObjectMethod(element, jsonObjectGetMethod, key);
            env->DeleteLocalRef(key);
            if (env->IsInstanceOf(quotaValue, env->FindClass("java/lang/String"))) {
                jstring jstr = (jstring) quotaValue;
                const char *str = env->GetStringUTFChars(jstr, nullptr);
                if (str != NULL) {
                    originParams.quota = std::string(str);
                    env->ReleaseStringUTFChars(jstr,str);
                }
            }

            getOriginsVector.push_back(originParams);
        }
    }
    auto nativeId = static_cast<int32_t>(jId);
    WebStorage::OnGetOriginsReceiveValue(getOriginsVector, nativeId);
}

void WebStorageJni::GetOriginQuota(const std::string& origin, int32_t asyncCallbackInfoId)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env || !g_webWebviewClass.globalRef || !g_webWebviewClass.getOriginQuota) {
        return;
    }
    jstring jOrigin = env->NewStringUTF(origin.c_str());
    CHECK_NULL_VOID(jOrigin);
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.getOriginQuota, jOrigin, asyncCallbackInfoId);
    if (env->ExceptionCheck()) {
        LOGE("WebStorageJni JNI: call GetOriginQuota has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteLocalRef(jOrigin);
        return;
    }
    env->DeleteLocalRef(jOrigin);
}

void WebStorageJni::GetOriginUsage(const std::string& origin, int32_t asyncCallbackInfoId)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env || !g_webWebviewClass.globalRef || !g_webWebviewClass.getOriginUsage) {
        return;
    }
    jstring jOrigin = env->NewStringUTF(origin.c_str());
    CHECK_NULL_VOID(jOrigin);
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.getOriginUsage, jOrigin, asyncCallbackInfoId);
    if (env->ExceptionCheck()) {
        LOGE("WebStorageJni JNI: call getOriginUsage has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteLocalRef(jOrigin);
        return;
    }
    env->DeleteLocalRef(jOrigin);
}

void WebStorageJni::GetOrigins(int32_t asyncCallbackInfoId)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env || !g_webWebviewClass.globalRef || !g_webWebviewClass.getOrigins) {
        return;
    }
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.getOrigins, asyncCallbackInfoId);
    if (env->ExceptionCheck()) {
        LOGE("WebStorageJni JNI: call GetOrigins has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
}

void WebStorageJni::DeleteAllData()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env || !g_webWebviewClass.globalRef || !g_webWebviewClass.deleteAllData) {
        return;
    }
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.deleteAllData);
    if (env->ExceptionCheck()) {
        LOGE("WebStorageJni JNI: call deleteAllData has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
}

void WebStorageJni::DeleteOrigin(const std::string& origin)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env || !g_webWebviewClass.globalRef || !g_webWebviewClass.deleteOrigin) {
        return;
    }
    jstring jOrigin = env->NewStringUTF(origin.c_str());
    CHECK_NULL_VOID(jOrigin);
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.deleteOrigin, jOrigin);
    if (env->ExceptionCheck()) {
        LOGE("WebStorageJni JNI: call deleteOrigin has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteLocalRef(jOrigin);
        return;
    }
    env->DeleteLocalRef(jOrigin);

}

} // namespace OHOS::Plugin