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


#include <codecvt>
#include <jni.h>
#include <locale>

#include "log.h"
#include "wifi_device_jni.h"
#include "wifi_receive_callback.h"

#include "plugins/interfaces/native/inner_api/plugin_utils_inner.h"
#include "plugins/interfaces/native/plugin_utils.h"
#include "nlohmann/json.hpp"

namespace OHOS::Plugin {
namespace {
const char WIFIMANAGER_CLASS_NAME[] = "ohos/ace/plugin/wifimanager/WifiDevicePlugin";
static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(WiFiDeviceJni::NativeInit) },
    { "nativeReceiveCallback", "(Ljava/lang/String;J)V",
        reinterpret_cast<void*>(WiFiDeviceJni::NativeReceiveCallback) },
};

static const char METHOD_GET_LINKED_INFO[] = "getLinkedInfo";
static const char METHOD_IS_WIFI_ACTIVE[] = "isWifiActive";
static const char METHOD_IS_CONNECTED[] = "isConnected";
static const char METHOD_ON[] = "on";
static const char METHOD_OFF[] = "off";

static const char SIGNATURE_GET_LINKED_INFO[] = "()Ljava/lang/String;";
static const char SIGNATURE_IS_WIFI_ACTIVE[] = "()Z";
static const char SIGNATURE_IS_CONNECTED[] = "()Z";
static const char SIGNATURE_ON[] = "(Ljava/lang/String;)V";
static const char SIGNATURE_OFF[] = "(Ljava/lang/String;)V";

struct {
    jmethodID getLinkedInfo;
    jmethodID isWifiActive;
    jmethodID isConnected;
    jmethodID on;
    jmethodID off;
    jobject globalRef;
} g_pluginClass;
} // namespace
WiFiDeviceJni& WiFiDeviceJni::GetInstance()
{
    static WiFiDeviceJni instance;
    return instance;
}

bool WiFiDeviceJni::Register(void* env)
{
    LOGI("WiFiDeviceJni JNI: Register");
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(WIFIMANAGER_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("WiFiDeviceJni JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void WiFiDeviceJni::NativeInit(JNIEnv* env, jobject jobj)
{
    LOGI("WiFiDeviceJni JNI: NativeInit");
    CHECK_NULL_VOID(env);
    g_pluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_pluginClass.getLinkedInfo = env->GetMethodID(cls, METHOD_GET_LINKED_INFO, SIGNATURE_GET_LINKED_INFO);
    CHECK_NULL_VOID(g_pluginClass.getLinkedInfo);

    g_pluginClass.isWifiActive = env->GetMethodID(cls, METHOD_IS_WIFI_ACTIVE, SIGNATURE_IS_WIFI_ACTIVE);
    CHECK_NULL_VOID(g_pluginClass.isWifiActive);

    g_pluginClass.isConnected = env->GetMethodID(cls, METHOD_IS_CONNECTED, SIGNATURE_IS_CONNECTED);
    CHECK_NULL_VOID(g_pluginClass.isConnected);

    g_pluginClass.on = env->GetMethodID(cls, METHOD_ON, SIGNATURE_ON);
    CHECK_NULL_VOID(g_pluginClass.on);

    g_pluginClass.off = env->GetMethodID(cls, METHOD_OFF, SIGNATURE_OFF);
    CHECK_NULL_VOID(g_pluginClass.off);

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

ErrCode WiFiDeviceJni::GetLinkedInfo(WifiLinkedInfo& info)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();

    CHECK_NULL_RETURN(env, ErrCode::WIFI_OPT_FAILED);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, ErrCode::WIFI_OPT_FAILED);
    CHECK_NULL_RETURN(g_pluginClass.getLinkedInfo, ErrCode::WIFI_OPT_FAILED);
    jstring jinfo = (jstring)env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getLinkedInfo);
    if (env->ExceptionCheck()) {
        LOGE("WiFiDeviceJni JNI: call GetLinkedInfo has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ErrCode::WIFI_OPT_FAILED;
    }
    std::string jsonobject = JavaStringToString(env, jinfo);
    JavaWifiLinkedInfoToOh(jsonobject, info);
    return ErrCode::WIFI_OPT_SUCCESS;
}

ErrCode WiFiDeviceJni::IsWifiActive(bool& bActive)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();

    CHECK_NULL_RETURN(env, ErrCode::WIFI_OPT_FAILED);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, ErrCode::WIFI_OPT_FAILED);
    CHECK_NULL_RETURN(g_pluginClass.isWifiActive, ErrCode::WIFI_OPT_FAILED);
    jboolean jisWfifActibe = env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.isWifiActive);
    if (env->ExceptionCheck()) {
        LOGE("WiFiDeviceJni JNI: call IsWifiActive has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        bActive = false;
        return ErrCode::WIFI_OPT_FAILED;
    }
    bActive = jisWfifActibe == JNI_TRUE ? true : false;
    return ErrCode::WIFI_OPT_SUCCESS;
}

ErrCode WiFiDeviceJni::IsConnected(bool& isConnected)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();

    CHECK_NULL_RETURN(env, ErrCode::WIFI_OPT_FAILED);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, ErrCode::WIFI_OPT_FAILED);
    CHECK_NULL_RETURN(g_pluginClass.isConnected, ErrCode::WIFI_OPT_FAILED);
    jboolean jisConnected = env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.isConnected);
    if (env->ExceptionCheck()) {
        LOGE("WiFiDeviceJni JNI: call IsConnected has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        isConnected = false;
        return ErrCode::WIFI_OPT_FAILED;
    }
    isConnected = jisConnected == JNI_TRUE ? true : false;
    return ErrCode::WIFI_OPT_SUCCESS;
}

ErrCode WiFiDeviceJni::On(const std::string& key)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();

    CHECK_NULL_RETURN(env, ErrCode::WIFI_OPT_FAILED);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, ErrCode::WIFI_OPT_FAILED);
    CHECK_NULL_RETURN(g_pluginClass.on, ErrCode::WIFI_OPT_FAILED);
    jstring jMethodOnKey = env->NewStringUTF(key.c_str());
    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.on, jMethodOnKey);
    if(env->ExceptionCheck()) {
        LOGE("WiFiDeviceJni JNI: call On has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ErrCode::WIFI_OPT_FAILED;
    }
    LOGI("WiFiDeviceJni JNI: call On key is %s", key.c_str());
    return ErrCode::WIFI_OPT_SUCCESS;
}

ErrCode WiFiDeviceJni::Off(const std::string& key)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();

    CHECK_NULL_RETURN(env, ErrCode::WIFI_OPT_FAILED);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, ErrCode::WIFI_OPT_FAILED);
    CHECK_NULL_RETURN(g_pluginClass.off, ErrCode::WIFI_OPT_FAILED);
    jstring jMethodOffKey = env->NewStringUTF(key.c_str());
    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.off , jMethodOffKey);
    if(env->ExceptionCheck()) {
        LOGE("WiFiDeviceJni JNI: call Off has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ErrCode::WIFI_OPT_FAILED;
    }
    LOGI("WiFiDeviceJni JNI: call off key is %s", key.c_str());
    return ErrCode::WIFI_OPT_SUCCESS;
}

void WiFiDeviceJni::JavaWifiLinkedInfoToOh(const std::string& event, WifiLinkedInfo& info)
{
    if (!event.empty()) {
        nlohmann::json jsonData = nlohmann::json::parse(event);
        if (jsonData.is_discarded()) {
            LOGE("WiFiDeviceJni JNI: nlohmann json Parse failed.");
            return;
        }
        info.ssid = jsonData["ssid"];
        info.bssid = jsonData["bssid"];
        info.networkId = jsonData["networkId"];
        info.rssi = jsonData["rssi"];
        info.linkSpeed = jsonData["linkSpeed"];
        info.frequency = jsonData["frequency"];
        info.ifHiddenSSID = jsonData["isHidden"];
    } else {
        LOGE("WiFiDeviceJni JNI: nlohmann json event is empty");
    }
}

void WiFiDeviceJni::NativeReceiveCallback(JNIEnv* env, jobject jobj, jstring key, jint code)
{
    std::string skey = JavaStringToString(env, key);
    auto nativecode = static_cast<int32_t>(code);
    LOGI("WiFiDeviceJni JNI: NativeReceiveCallback skey is %s code is %d", skey.c_str(), nativecode);
    WifiReceiveCallback::ReceiveCallback(skey, nativecode);
}
} // namespace OHOS::Plugin
