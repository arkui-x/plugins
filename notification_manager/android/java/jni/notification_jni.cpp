/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "notification_jni.h"

#include <codecvt>
#include <jni.h>
#include <locale>

#include "log.h"
#include "nlohmann/json.hpp"

#include "plugins/interfaces/native/inner_api/plugin_utils_inner.h"
#include "plugins/interfaces/native/plugin_utils.h"
#include "plugins/notification_manager/ans_inner_errors.h"

namespace OHOS {
namespace Notification {
namespace {
const char MANAGER_CLASS_NAME[] = "ohos/ace/plugin/notificationmanager/NotificationPlugin";
static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(NotificationJni::NativeInit) },
    { "nativeReceiveCallback", "(Ljava/lang/String;J)V",
        reinterpret_cast<void*>(NotificationJni::NativeReceiveCallback) },
};

static const char METHOD_PUBLISH[] = "publish";
static const char METHOD_CANCEL[] = "cancel";
static const char METHOD_CANCEL_ALL[] = "cancelAll";
static const char METHOD_SET_BADGE_NUMBER[] = "setBadgeNumber";
static const char METHOD_REQUEST_ENABLE[] = "requestEnableNotification";
static const char METHOD_IS_HIGH_VERSION[] = "isAPITiramisuLater";
static const char METHOD_CHECK_PERMISSION[] = "checkPermission";

static const char SIGNATURE_PUBLISH_INFO[] = "(Ljava/lang/String;)V";
static const char SIGNATURE_CANCEL[] = "(ILjava/lang/String;)V";
static const char SIGNATURE_CANCEL_ALL[] = "()V";
static const char SIGNATURE_SET_BADGE_NUMBER[] = "(I)V";
static const char SIGNATURE_REQUEST_ENABLE[] = "()V";
static const char SIGNATURE_IS_HIGH_VERSION[] = "()Z";
static const char SIGNATURE_CHECK_PERMISSION[] = "()Z";

struct {
    jmethodID publish;
    jobject globalRef;
    jmethodID cancel;
    jmethodID cancelAll;
    jmethodID setBadgeNumber;
    jmethodID requestEnable;
    jmethodID isHighVersion;
    jmethodID isGranted;
} g_pluginClass;

} // namespace

NotificationJni& NotificationJni::GetInstance()
{
    static NotificationJni instance;
    return instance;
}

bool NotificationJni::Register(void* env)
{
    LOGI("NotificationJni JNI: Register");
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(MANAGER_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("NotificationJni JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void NotificationJni::NativeInit(JNIEnv* env, jobject jobj)
{
    LOGI("NotificationJni JNI: NativeInit");
    CHECK_NULL_VOID(env);
    g_pluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_pluginClass.publish = env->GetMethodID(cls, METHOD_PUBLISH, SIGNATURE_PUBLISH_INFO);
    CHECK_NULL_VOID(g_pluginClass.publish);

    g_pluginClass.cancel = env->GetMethodID(cls, METHOD_CANCEL, SIGNATURE_CANCEL);
    CHECK_NULL_VOID(g_pluginClass.cancel);

    g_pluginClass.cancelAll = env->GetMethodID(cls, METHOD_CANCEL_ALL, SIGNATURE_CANCEL_ALL);
    CHECK_NULL_VOID(g_pluginClass.cancelAll);

    g_pluginClass.setBadgeNumber = env->GetMethodID(cls, METHOD_SET_BADGE_NUMBER, SIGNATURE_SET_BADGE_NUMBER);
    CHECK_NULL_VOID(g_pluginClass.setBadgeNumber);

    g_pluginClass.requestEnable = env->GetMethodID(cls, METHOD_REQUEST_ENABLE, SIGNATURE_REQUEST_ENABLE);
    CHECK_NULL_VOID(g_pluginClass.requestEnable);

    g_pluginClass.isHighVersion = env->GetMethodID(cls, METHOD_IS_HIGH_VERSION, SIGNATURE_IS_HIGH_VERSION);
    CHECK_NULL_VOID(g_pluginClass.isHighVersion);

    g_pluginClass.isGranted = env->GetMethodID(cls, METHOD_CHECK_PERMISSION, SIGNATURE_CHECK_PERMISSION);
    CHECK_NULL_VOID(g_pluginClass.isGranted);

    env->DeleteLocalRef(cls);
}

static std::string UTF16StringToUTF8String(const char16_t* chars, size_t len)
{
    std::u16string u16_string(chars, len);
    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.to_bytes(u16_string);
}

static std::string JavaStringToString(JNIEnv* env, jstring str)
{
    if (env == nullptr || str == nullptr) {
        return "";
    }
    const jchar* chars = env->GetStringChars(str, NULL);
    if (chars == nullptr) {
        return "";
    }
    std::string u8_string =
        UTF16StringToUTF8String(reinterpret_cast<const char16_t*>(chars), env->GetStringLength(str));
    env->ReleaseStringChars(str, chars);
    return u8_string;
}

void NotificationJni::NativeReceiveCallback(JNIEnv* env, jobject jobj, jstring key, jint code)
{
    std::string skey = JavaStringToString(env, key);
    auto nativecode = static_cast<int32_t>(code);
    LOGI("NotificationJni JNI: NativeReceiveCallback skey is %s code is %d", skey.c_str(), nativecode);
}

ErrorCode NotificationJni::publish(const NotificationRequest& request)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    nlohmann::json jsonParser = nlohmann::json::object();
    request.ToJson(jsonParser);

    jstring requestStr = env->NewStringUTF(jsonParser.dump().c_str());
    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.publish, requestStr);
    if (env->ExceptionCheck()) {
        LOGE("NotificationJni JNI: call publish has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERR_ANS_SERVICE_NOT_CONNECTED;
    }
    return ERR_ANS_OK;
}

ErrorCode NotificationJni::cancel(int32_t id, std::string label)
{
    LOGD("NotificationJni JNI: cancel");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    jint intParam = static_cast<jint>(id);
    jstring stringParam = env->NewStringUTF(label.c_str());
    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.cancel, intParam, stringParam);
    if (env->ExceptionCheck()) {
        LOGE("NotificationJni JNI: call cancel has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERR_ANS_SERVICE_NOT_CONNECTED;
    }
    return ERR_ANS_OK;
}

ErrorCode NotificationJni::cancelAll()
{
    LOGD("NotificationJni JNI: cancelAll");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.cancelAll);
    if (env->ExceptionCheck()) {
        LOGE("NotificationJni JNI: call cancelAll has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERR_ANS_SERVICE_NOT_CONNECTED;
    }
    return ERR_ANS_OK;
}

ErrorCode NotificationJni::setBadgeNumber(int32_t number)
{
    LOGI("NotificationJni JNI: setBadgeNumber");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.setBadgeNumber, number);
    if (env->ExceptionCheck()) {
        LOGE("NotificationJni JNI: call cancel has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERR_ANS_SERVICE_NOT_CONNECTED;
    }
    return ERR_ANS_OK;
}

ErrorCode NotificationJni::RequestEnableNotification()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.requestEnable);
    if (env->ExceptionCheck()) {
        LOGE("NotificationJni JNI: call request has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERR_ANS_SERVICE_NOT_CONNECTED;
    }
    return ERR_ANS_OK;
}
bool NotificationJni::isHighVersion()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    jboolean result = env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.isHighVersion);
    return static_cast<bool>(result);
}
bool NotificationJni::isGranted()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    jboolean result = env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.isGranted);
    return static_cast<bool>(result);
}
} // namespace Notification
} // namespace OHOS
