/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "plugins/common_event/android/java/jni/broadcast_jni.h"

#include <locale>
#include <codecvt>
#include <jni.h>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugins/common_event/android/java/jni/common_event_receiver.h"

namespace OHOS::Plugin {
namespace {
const char BROADCAST_CLASS_NAME[] = "ohos/ace/plugin/broadcastplugin/BroadcastPlugin";

static const JNINativeMethod METHODS[] = {
    {"nativeInit", "()V", reinterpret_cast<void *>(BroadcastJni::NativeInit)},
    {"nativeReceiveBroadcast",
        "(Ljava/lang/String;Ljava/lang/String;)V",
        reinterpret_cast<void *>(BroadcastJni::NativeReceiveBroadcast)},
};

static const char METHOD_SEND_BROADCAST[] = "sendBroadcast";
static const char METHOD_REGISTER_BROADCAST[] = "registerBroadcast";
static const char METHOD_UNREGISTER_BROADCAST[] = "unRegisterBroadcast";

static const char SIGNATURE_SEND_BROADCAST[] = "(Ljava/lang/String;Ljava/lang/String;)Z";
static const char SIGNATURE_REGISTER_BROADCAST[] = "(Ljava/lang/String;[Ljava/lang/String;)Z";
static const char SIGNATURE_UNREGISTER_BROADCAST[] = "(Ljava/lang/String;)Z";

struct {
    jmethodID sendBroadcast;
    jmethodID registerBroadcast;
    jmethodID unRegisterBroadcast;
    jobject globalRef;
} g_pluginClass;
}  // namespace

static void RegisterCommonEvent()
{
    g_eventMap["usual.event.data.ACCOUNT_DELETED"] = "android.accounts.action.ACCOUNT_REMOVED";
    g_eventMap["usual.event.AIRPLANE_MODE"] = "android.intent.action.AIRPLANE_MODE";
    g_eventMap["usual.event.BATTERY_CHANGED"] = "android.intent.action.BATTERY_CHANGED";
    g_eventMap["usual.event.BATTERY_LOW"] = "android.intent.action.BATTERY_LOW";
    g_eventMap["usual.event.BATTERY_OKAY"] = "android.intent.action.BATTERY_OKAY";
    g_eventMap["usual.event.bluetooth.a2dpsink.AUDIO_STATE_UPDATE"] =
        "android.bluetooth.headset.profile.action.AUDIO_STATE_CHANGED";
    g_eventMap["usual.event.bluetooth.a2dpsink.CONNECT_STATE_UPDATE"] =
        "android.bluetooth.a2dp.profile.action.CONNECTION_STATE_CHANGED";
    g_eventMap["usual.event.bluetooth.a2dpsink.PLAYING_STATE_UPDATE"] =
        "android.bluetooth.a2dp.profile.action.PLAYING_STATE_CHANGED";
    g_eventMap["usual.event.bluetooth.remotedevice.UUID_VALUE"] = "android.bluetooth.device.action.UUID";
    g_eventMap["usual.event.BOOT_COMPLETED"] = "android.intent.action.BOOT_COMPLETED";
    g_eventMap["usual.event.BUNDLE_REMOVED"] = "android.intent.action.PACKAGE_REMOVED";
    g_eventMap["usual.event.CHARGING"] = "android.os.action.CHARGING";
    g_eventMap["usual.event.CLOSE_SYSTEM_DIALOGS"] = "android.intent.action.CLOSE_SYSTEM_DIALOGS";
    g_eventMap["usual.event.CONFIGURATION_CHANGED"] = "android.intent.action.CONFIGURATION_CHANGED";
    g_eventMap["usual.event.DATE_CHANGED"] = "android.intent.action.DATE_CHANGED";
    g_eventMap["usual.event.DEVICE_IDLE_MODE_CHANGED"] = "android.os.action.DEVICE_IDLE_MODE_CHANGED";
    g_eventMap["usual.event.DISCHARGING"] = "android.os.action.DISCHARGING";
    g_eventMap["usual.event.EXTERNAL_APPLICATIONS_AVAILABLE"] =
        "android.intent.action.EXTERNAL_APPLICATIONS_AVAILABLE";
    g_eventMap["usual.event.EXTERNAL_APPLICATIONS_UNAVAILABLE"] =
        "android.intent.action.EXTERNAL_APPLICATIONS_UNAVAILABLE";
    g_eventMap["usual.event.LOCALE_CHANGED"] = "android.intent.action.LOCALE_CHANGED";
    g_eventMap["usual.event.LOCKED_BOOT_COMPLETED"] = "android.intent.action.LOCKED_BOOT_COMPLETED";
    g_eventMap["usual.event.MANAGE_PACKAGE_STORAGE"] = "android.intent.action.MANAGE_PACKAGE_STORAGE";
    g_eventMap["usual.event.MY_PACKAGE_REPLACED"] = "android.intent.action.MY_PACKAGE_REPLACED";
    g_eventMap["usual.event.MY_PACKAGE_SUSPENDED"] = "android.intent.action.MY_PACKAGE_SUSPENDED";
    g_eventMap["usual.event.MY_PACKAGE_UNSUSPENDED"] = "android.intent.action.MY_PACKAGE_UNSUSPENDED";
    g_eventMap["usual.event.nfc.action.ADAPTER_STATE_CHANGED"] = "android.nfc.action.ADAPTER_STATE_CHANGED";
    g_eventMap["usual.event.PACKAGE_ADDED"] = "android.intent.action.PACKAGE_ADDED";
    g_eventMap["usual.event.PACKAGE_CHANGED"] = "android.intent.action.PACKAGE_CHANGED";
    g_eventMap["usual.event.PACKAGE_DATA_CLEARED"] = "android.intent.action.PACKAGE_DATA_CLEARED";
    g_eventMap["usual.event.PACKAGE_FIRST_LAUNCH"] = "android.intent.action.PACKAGE_FIRST_LAUNCH";
    g_eventMap["usual.event.PACKAGE_FULLY_REMOVED"] = "android.intent.action.PACKAGE_FULLY_REMOVED";
    g_eventMap["usual.event.PACKAGE_NEEDS_VERIFICATION"] = "android.intent.action.PACKAGE_NEEDS_VERIFICATION";
    g_eventMap["usual.event.PACKAGE_REMOVED"] = "android.intent.action.PACKAGE_REMOVED";
    g_eventMap["usual.event.PACKAGE_REPLACED"] = "android.intent.action.PACKAGE_REPLACED";
    g_eventMap["usual.event.PACKAGE_RESTARTED"] = "android.intent.action.PACKAGE_RESTARTED";
    g_eventMap["usual.event.PACKAGE_VERIFIED"] = "android.intent.action.PACKAGE_VERIFIED";
    g_eventMap["usual.event.PACKAGES_SUSPENDED"] = "android.intent.action.PACKAGES_SUSPENDED";
    g_eventMap["usual.event.PACKAGES_UNSUSPENDED"] = "android.intent.action.PACKAGES_UNSUSPENDED";
    g_eventMap["usual.event.POWER_CONNECTED"] = "android.intent.action.ACTION_POWER_CONNECTED";
    g_eventMap["usual.event.POWER_DISCONNECTED"] = "android.intent.action.ACTION_POWER_DISCONNECTED";
    g_eventMap["usual.event.POWER_SAVE_MODE_CHANGED"] = "android.os.action.POWER_SAVE_MODE_CHANGED";
    g_eventMap["usual.event.SCREEN_OFF"] = "android.intent.action.SCREEN_OFF";
    g_eventMap["usual.event.SCREEN_ON"] = "android.intent.action.SCREEN_ON";
    g_eventMap["usual.event.SHUTDOWN"] = "android.intent.action.ACTION_SHUTDOWN";
    g_eventMap["usual.event.SIM.DEFAULT_DATA_SUBSCRIPTION_CHANGED"] =
        "android.telephony.action.DEFAULT_SUBSCRIPTION_CHANGED";
    g_eventMap["usual.event.SIM.DEFAULT_MAIN_SUBSCRIPTION_CHANGED"] =
        "android.telephony.action.DEFAULT_SUBSCRIPTION_CHANGED";
    g_eventMap["usual.event.SIM.DEFAULT_SMS_SUBSCRIPTION_CHANGED"] =
        "android.telephony.action.DEFAULT_SMS_SUBSCRIPTION_CHANGED";
    g_eventMap["usual.event.SMS_RECEIVE_COMPLETED"] = "android.intent.action.DATA_SMS_RECEIVED";
    g_eventMap["usual.event.TIME_CHANGED"] = "android.intent.action.TIME_SET";
    g_eventMap["usual.event.TIME_TICK"] = "android.intent.action.TIME_TICK";
    g_eventMap["usual.event.TIMEZONE_CHANGED"] = "android.intent.action.TIMEZONE_CHANGED";
    g_eventMap["usual.event.UID_REMOVED"] = "android.intent.action.UID_REMOVED";
    g_eventMap["usual.event.hardware.usb.action.USB_ACCESSORY_ATTACHED"] =
        "android.hardware.usb.action.USB_ACCESSORY_ATTACHED";
    g_eventMap["usual.event.hardware.usb.action.USB_ACCESSORY_DETACHED"] =
        "android.hardware.usb.action.USB_ACCESSORY_DETACHED";
    g_eventMap["usual.event.hardware.usb.action.USB_DEVICE_ATTACHED"] =
        "android.hardware.usb.action.USB_DEVICE_ATTACHED";
    g_eventMap["usual.event.hardware.usb.action.USB_DEVICE_DETACHED"] =
        "android.hardware.usb.action.USB_DEVICE_DETACHED";
    g_eventMap["usual.event.USER_BACKGROUND"] = "android.intent.action.USER_BACKGROUND";
    g_eventMap["usual.event.USER_FOREGROUND"] = "android.intent.action.USER_FOREGROUND";
    g_eventMap["usual.event.USER_PRESENT"] = "android.intent.action.USER_PRESENT";
    g_eventMap["usual.event.USER_UNLOCKED"] = "android.intent.action.USER_UNLOCKED";
    g_eventMap["usual.event.wifi.RSSI_VALUE"] = "android.net.wifi.RSSI_CHANGED";
    g_eventMap["usual.event.wifi.SCAN_FINISHED"] = "android.net.wifi.SCAN_RESULTS";
}

bool BroadcastJni::Register(void* env)
{
    LOGI("Broadcast JNI: Register");
    RegisterCommonEvent();
    auto* jniEnv = static_cast<JNIEnv *>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(BROADCAST_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("Broadcast JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void BroadcastJni::NativeInit(JNIEnv* env, jobject jobj)
{
    LOGI("Broadcast JNI: NativeInit");
    CHECK_NULL_VOID(env);
    g_pluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_pluginClass.sendBroadcast = env->GetMethodID(cls, METHOD_SEND_BROADCAST, SIGNATURE_SEND_BROADCAST);
    CHECK_NULL_VOID(g_pluginClass.sendBroadcast);

    g_pluginClass.registerBroadcast = env->GetMethodID(cls, METHOD_REGISTER_BROADCAST, SIGNATURE_REGISTER_BROADCAST);
    CHECK_NULL_VOID(g_pluginClass.registerBroadcast);

    g_pluginClass.unRegisterBroadcast =
        env->GetMethodID(cls, METHOD_UNREGISTER_BROADCAST, SIGNATURE_UNREGISTER_BROADCAST);
    CHECK_NULL_VOID(g_pluginClass.unRegisterBroadcast);

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
        UTF16StringToUTF8String(reinterpret_cast<const char16_t *>(chars), env->GetStringLength(str));
    env->ReleaseStringChars(str, chars);
    return u8_string;
}

static std::string OhEventToJava(const std::string &event)
{
    auto it = g_eventMap.find(event);
    if (it != g_eventMap.end()) {
        return it->second;
    } else {
        return event;
    }
}

static std::string JavaEventToOh(const std::string &event)
{
    for (auto it = g_eventMap.begin(); it != g_eventMap.end(); it++) {
        if (event.compare(it->second) == 0) {
            return it->first;
        }
    }
    return event;
}

static jstring StringToJavaString(JNIEnv* env, const std::string &string)
{
    std::u16string str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(string);
    return env->NewString(reinterpret_cast<const jchar *>(str.data()), str.length());
}

void BroadcastJni::NativeReceiveBroadcast(JNIEnv* env, jobject jobj, jstring action, jstring json)
{
    LOGI("Broadcast JNI: NativeReceiveBroadcast");

    std::string saction = JavaStringToString(env, action);
    std::string sjson = JavaStringToString(env, json);
    LOGI("Broadcast JNI: NativeReceiveBroadcast saction is %{public}s, sjson is %{public}s",
        saction.c_str(),
        sjson.c_str());
    CommonEventReceiver::ReceiveCommonEvent(JavaEventToOh(saction), sjson);
}

void BroadcastJni::SendBroadcast(std::string action, std::string json, AsyncCallbackInfo* ptr)
{
    LOGI("Broadcast JNI: SendBroadcast");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    CHECK_NULL_VOID(g_pluginClass.sendBroadcast);

    jstring jaction = StringToJavaString(env, OhEventToJava(action));
    jstring jjson = StringToJavaString(env, json);

    jboolean jResult = env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.sendBroadcast, jaction, jjson);
    if (env->ExceptionCheck()) {
        LOGE("Broadcast JNI: call SendBroadcast has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    LOGI("Broadcast JNI: call back");
    ptr->status = jResult;
    CommonEventReceiver::ReceiveCallBack(ptr);
}

void BroadcastJni::RegisterBroadcast(std::string key, std::vector<std::string> actions)
{
    LOGI("Broadcast JNI: RegisterBroadcast");
    auto env = ARKUI_X_Plugin_GetJniEnv();

    jstring jkey = StringToJavaString(env, key);
    jstring jactions[actions.size()];
    jobjectArray array;
    jclass jcl = env->FindClass("java/lang/String");
    array = env->NewObjectArray(actions.size(), jcl, NULL);
    for (size_t i = 0; i < actions.size(); i++) {
        jactions[i] = StringToJavaString(env, OhEventToJava(actions[i]));
        env->SetObjectArrayElement(array, i, jactions[i]);
    }

    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    CHECK_NULL_VOID(g_pluginClass.registerBroadcast);
    env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.registerBroadcast, jkey, array);
    if (env->ExceptionCheck()) {
        LOGE("Broadcast JNI: call RegisterBroadcast has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
}

void BroadcastJni::UnRegisterBroadcast(std::string key, AsyncCallbackInfo* ptr)
{
    LOGI("Broadcast JNI: UnRegisterBroadcast");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    CHECK_NULL_VOID(g_pluginClass.unRegisterBroadcast);

    jstring jkey = StringToJavaString(env, key);

    jboolean jResult = env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.unRegisterBroadcast, jkey);
    if (env->ExceptionCheck()) {
        LOGE("Broadcast JNI: call UnRegisterBroadcast has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    ptr->status = jResult;
    CommonEventReceiver::ReceiveCallBack(ptr);
}
}  // namespace OHOS::Plugin
