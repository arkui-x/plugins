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

#include "plugins/web/webview/android/java/jni/geolocation_permissions_jni.h"

#include <jni.h>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"
#include "business_error.h"

using namespace OHOS::NWebError;

namespace OHOS::Plugin {
namespace {
const char WEB_WEBVIEW_CLASS_NAME[] = "ohos/ace/adapter/capability/web/AceGeolocationPermissions";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(GeolocationPermissionsJni::NativeInit) },
    { "onReceiveGetAccessibleGeolocationValue", "(ZJ)V", 
        reinterpret_cast<void*>(GeolocationPermissionsJni::OnReceiveGetAccessibleGeolocationValue) },
    { "onReceiveGetAccessibleGeolocationError", "(Ljava/lang/String;J)V", 
        reinterpret_cast<void*>(GeolocationPermissionsJni::OnReceiveGetAccessibleGeolocationError) },
    { "onReceiveGetStoredGeolocationValue", "([Ljava/lang/String;J)V", 
        reinterpret_cast<void*>(GeolocationPermissionsJni::OnReceiveGetStoredGeolocationValue) },
};
static const char METHOD_ALLOW_GEOLOCATION[] = "allowGeolocation";
static const char METHOD_DELETE_GEOLOCATION[] = "deleteGeolocation";
static const char METHOD_DELETE_ALL_GEOLOCATION[] = "deleteAllGeolocation";
static const char METHOD_GET_ACCESSIBLE_GEOLOCATION[] = "getAccessibleGeolocation";
static const char METHOD_GET_STORED_GEOLOCATION[] = "getStoredGeolocation";

static const char SIGNATURE_ALLOW_GEOLOCATION[] = "(Ljava/lang/String;ZZ)V";
static const char SIGNATURE_DELETE_GEOLOCATION[] = "(Ljava/lang/String;Z)V";
static const char SIGNATURE_DELETE_ALL_GEOLOCATION[] = "(Z)V";
static const char SIGNATURE_GET_ACCESSIBLE_GEOLOCATION[] = "(Ljava/lang/String;JZ)V";
static const char SIGNATURE_GET_STORED_GEOLOCATION[] = "(JZ)V";

struct {
    jmethodID allowGeolocation;
    jmethodID deleteGeolocation;
    jmethodID deleteAllGeolocation;
    jmethodID getAccessibleGeolocation;
    jmethodID getStoredGeolocation;
    jobject globalRef;
} g_webWebviewClass;
} // namespace

bool GeolocationPermissionsJni::Register(void* env)
{
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(WEB_WEBVIEW_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("GeolocationPermissionsJni JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void GeolocationPermissionsJni::NativeInit(JNIEnv* env, jobject jobj)
{
    LOGI("GeolocationPermissionsJni JNI: NativeInit");
    CHECK_NULL_VOID(env);
    g_webWebviewClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_webWebviewClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);
    g_webWebviewClass.allowGeolocation = env->GetMethodID(cls, METHOD_ALLOW_GEOLOCATION, SIGNATURE_ALLOW_GEOLOCATION);
    g_webWebviewClass.deleteGeolocation = env->GetMethodID(cls, METHOD_DELETE_GEOLOCATION,
        SIGNATURE_DELETE_GEOLOCATION);
    g_webWebviewClass.deleteAllGeolocation = env->GetMethodID(cls, METHOD_DELETE_ALL_GEOLOCATION,
        SIGNATURE_DELETE_ALL_GEOLOCATION);
    g_webWebviewClass.getAccessibleGeolocation = env->GetMethodID(cls, METHOD_GET_ACCESSIBLE_GEOLOCATION,
        SIGNATURE_GET_ACCESSIBLE_GEOLOCATION);
    g_webWebviewClass.getStoredGeolocation = env->GetMethodID(cls, METHOD_GET_STORED_GEOLOCATION,
        SIGNATURE_GET_STORED_GEOLOCATION);
    env->DeleteLocalRef(cls);
}

void GeolocationPermissionsJni::AllowGeolocation(const std::string& origin, const bool incognito)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);

    jstring jOrigin = env->NewStringUTF(origin.c_str());
    CHECK_NULL_VOID(jOrigin);

    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.allowGeolocation, jOrigin, true, incognito);
    if (env->ExceptionCheck()) {
        LOGE("GeolocationPermissionsJni JNI: call AllowGeolocation has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteLocalRef(jOrigin);
        return;
    }
    env->DeleteLocalRef(jOrigin);
}

void GeolocationPermissionsJni::DeleteGeolocation(const std::string& origin, const bool incognito)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);

    jstring jOrigin = env->NewStringUTF(origin.c_str());
    CHECK_NULL_VOID(jOrigin);

    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.deleteGeolocation, jOrigin, incognito);
    if (env->ExceptionCheck()) {
        LOGE("GeolocationPermissionsJni JNI: call DeleteGeolocation has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteLocalRef(jOrigin);
        return;
    }
    env->DeleteLocalRef(jOrigin);
}

void GeolocationPermissionsJni::DeleteAllGeolocation(const bool incognito)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);

    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.deleteAllGeolocation, incognito);
    if (env->ExceptionCheck()) {
        LOGE("GeolocationPermissionsJni JNI: call DeleteAllGeolocation has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
}

void GeolocationPermissionsJni::GetAccessibleGeolocation(const std::string& origin, int32_t asyncCallbackInfoId, const bool incognito)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    
    jstring jOrigin = env->NewStringUTF(origin.c_str());
    CHECK_NULL_VOID(jOrigin);

    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.getAccessibleGeolocation, jOrigin, asyncCallbackInfoId, incognito);
    if (env->ExceptionCheck()) {
        LOGE("GeolocationPermissionsJni JNI: call GetAccessibleGeolocation has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
                env->DeleteLocalRef(jOrigin);
        return;
    }
    env->DeleteLocalRef(jOrigin);
}

void GeolocationPermissionsJni::GetStoredGeolocation(int32_t asyncCallbackInfoId, const bool incognito)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);

    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.getStoredGeolocation, asyncCallbackInfoId, incognito);
    if (env->ExceptionCheck()) {
        LOGE("GeolocationPermissionsJni JNI: call GetStoredGeolocation has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
}

void GeolocationPermissionsJni::OnReceiveGetAccessibleGeolocationValue(JNIEnv* env, jobject jobj, jboolean jResult, jint jId)
{
    CHECK_NULL_VOID(env);
    auto nativeId = static_cast<int32_t>(jId);
    GeolocationPermissions::OnGetAccessibleReceiveValue(jResult, nativeId);
}

void GeolocationPermissionsJni::OnReceiveGetAccessibleGeolocationError(JNIEnv* env, jobject jobj, jstring jErrCode, jint jId)
{
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(jErrCode);
    std::string result;
    int errcodeId = NWebError::NO_ERROR;
    const char* content = env->GetStringUTFChars(jErrCode, nullptr);
    if (content != nullptr) {
        result.assign(content);
        env->ReleaseStringUTFChars(jErrCode, content);
    }
    if (result.compare("INVALID_ORIGIN") == 0) {
        errcodeId = NWebError::INVALID_ORIGIN;
    }
    auto nativeId = static_cast<int32_t>(jId);
    GeolocationPermissions::OnGetAccessibleReceiveError(errcodeId, nativeId);
}

void GeolocationPermissionsJni::OnReceiveGetStoredGeolocationValue(JNIEnv* env, jobject jobj, jobjectArray jStringArray, jint jId)
{
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(jStringArray);
    std::vector<std::string> result;
    jsize length = env->GetArrayLength(jStringArray);
    if (length > 0) {
        for (jsize i = 0; i < length; ++i) {
            jstring jResult = static_cast<jstring>(env->GetObjectArrayElement(jStringArray, i));
            const char* content = env->GetStringUTFChars(jResult, nullptr);
            if (content != nullptr) {
                result.push_back(std::string(content));
                env->ReleaseStringUTFChars(jResult, content);
            }
            env->DeleteLocalRef(jResult);
        }
    }
    auto nativeId = static_cast<int32_t>(jId);
    GeolocationPermissions::OnGetStoredReceiveValue(result, nativeId);
}

} // namespace OHOS::Plugin
