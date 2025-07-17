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

#include "audio_manager_jni.h"

#include <jni.h>

#include "audio_common_jni.h"
#include "audio_convert_util.h"
#include "audio_errors.h"
#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"

using namespace OHOS::AudioStandard;

namespace OHOS::Plugin {
namespace {
const char CLASS_NAME[] = "ohos/ace/plugin/audiomanager/AudioManagerPlugin";
static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(AudioManagerJni::NativeInit) },
    { "nativeOnAudioDeviceChanged", "(Z[Landroid/media/AudioDeviceInfo;)V",
        reinterpret_cast<void*>(AudioManagerJni::NativeOnAudioDeviceChanged) },
    { "nativeOnAudioRendererChanged", "(Ljava/util/List;)V",
        reinterpret_cast<void*>(AudioManagerJni::NativeOnAudioRendererChanged) },
    { "nativeOnAudioCapturerChanged", "(Ljava/util/List;)V",
        reinterpret_cast<void *>(AudioManagerJni::NativeOnAudioCapturerChanged) }
};

struct {
    jmethodID getVolume;
    jmethodID getMaxVolume;
    jmethodID getMinVolume;
    jmethodID isStreamMute;
    jmethodID getRingerMode;
    jmethodID isMicrophoneMute;
    jmethodID isVolumeFixed;
    jmethodID getSystemVolumeInDb;
    jmethodID getAudioScene;
    jmethodID getDevices;
    jmethodID registerAudioDeviceCallback;
    jmethodID unregisterAudioDeviceCallback;
    jmethodID getCurrentAudioRendererInfoArray;
    jmethodID getCurrentAudioCapturerInfoArray;
    jmethodID registerAudioRendererEventListener;
    jmethodID unregisterAudioRendererEventListener;
    jmethodID registerAudioCapturerEventListener;
    jmethodID unregisterAudioCapturerEventListener;
    jmethodID setCommunicationDevice;
    jmethodID isCommunicationDeviceActive;
    jmethodID setDeviceActive;
    jmethodID isDeviceActive;
    jobject globalRef;
} g_audioManagerPluginClass;
} // namespace

bool AudioManagerJni::hasInit_ = false;
std::shared_ptr<AudioCapturerStateChangeCallback> AudioManagerJni::audioCapturerEventCallback_;
std::shared_ptr<AudioRendererStateChangeCallback> AudioManagerJni::audioRendererEventCallback_;
std::map<DeviceFlag, std::shared_ptr<AudioManagerDeviceChangeCallback>> AudioManagerJni::deviceChangeCallbacks_;

bool AudioManagerJni::Register(void* env)
{
    LOGI("AudioManagerJni::Register");
    if (hasInit_) {
        return hasInit_;
    }
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("AudioManagerJni JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void AudioManagerJni::NativeInit(JNIEnv* env, jobject jobj)
{
    LOGI("AudioManagerJni::NativeInit");
    CHECK_NULL_VOID(env);
    g_audioManagerPluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_audioManagerPluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_audioManagerPluginClass.getVolume = env->GetMethodID(cls, "getVolume", "(I)I");
    CHECK_NULL_VOID(g_audioManagerPluginClass.getVolume);

    g_audioManagerPluginClass.getMaxVolume = env->GetMethodID(cls, "getMaxVolume", "(I)I");
    CHECK_NULL_VOID(g_audioManagerPluginClass.getMaxVolume);

    g_audioManagerPluginClass.getMinVolume = env->GetMethodID(cls, "getMinVolume", "(I)I");
    CHECK_NULL_VOID(g_audioManagerPluginClass.getMinVolume);

    g_audioManagerPluginClass.isStreamMute = env->GetMethodID(cls, "isStreamMute", "(I)Z");
    CHECK_NULL_VOID(g_audioManagerPluginClass.isStreamMute);

    g_audioManagerPluginClass.getRingerMode = env->GetMethodID(cls, "getRingerMode", "()I");
    CHECK_NULL_VOID(g_audioManagerPluginClass.getRingerMode);

    g_audioManagerPluginClass.isMicrophoneMute = env->GetMethodID(cls, "isMicrophoneMute", "()Z");
    CHECK_NULL_VOID(g_audioManagerPluginClass.isMicrophoneMute);

    g_audioManagerPluginClass.isVolumeFixed = env->GetMethodID(cls, "isVolumeFixed", "()Z");
    CHECK_NULL_VOID(g_audioManagerPluginClass.isVolumeFixed);

    g_audioManagerPluginClass.getSystemVolumeInDb = env->GetMethodID(cls, "getSystemVolumeInDb", "(III)F");
    CHECK_NULL_VOID(g_audioManagerPluginClass.getSystemVolumeInDb);

    g_audioManagerPluginClass.getAudioScene = env->GetMethodID(cls, "getAudioScene", "()I");
    CHECK_NULL_VOID(g_audioManagerPluginClass.getAudioScene);

    g_audioManagerPluginClass.getDevices = env->GetMethodID(cls, "getDevices", "(I)[Landroid/media/AudioDeviceInfo;");
    CHECK_NULL_VOID(g_audioManagerPluginClass.getDevices);

    g_audioManagerPluginClass.registerAudioDeviceCallback = env->GetMethodID(cls, "registerAudioDeviceCallback", "()V");
    CHECK_NULL_VOID(g_audioManagerPluginClass.registerAudioDeviceCallback);

    g_audioManagerPluginClass.unregisterAudioDeviceCallback =
        env->GetMethodID(cls, "unregisterAudioDeviceCallback", "()V");
    CHECK_NULL_VOID(g_audioManagerPluginClass.unregisterAudioDeviceCallback);

    g_audioManagerPluginClass.getCurrentAudioRendererInfoArray =
        env->GetMethodID(cls, "getCurrentAudioRendererInfoArray", "()Ljava/util/List;");
    CHECK_NULL_VOID(g_audioManagerPluginClass.getCurrentAudioRendererInfoArray);

    g_audioManagerPluginClass.getCurrentAudioCapturerInfoArray =
        env->GetMethodID(cls, "getCurrentAudioCapturerInfoArray", "()Ljava/util/List;");
    CHECK_NULL_VOID(g_audioManagerPluginClass.getCurrentAudioCapturerInfoArray);

    g_audioManagerPluginClass.registerAudioRendererEventListener =
        env->GetMethodID(cls, "registerAudioRendererEventListener", "()V");
    CHECK_NULL_VOID(g_audioManagerPluginClass.registerAudioRendererEventListener);

    g_audioManagerPluginClass.unregisterAudioRendererEventListener =
        env->GetMethodID(cls, "unregisterAudioRendererEventListener", "()V");
    CHECK_NULL_VOID(g_audioManagerPluginClass.unregisterAudioRendererEventListener);

    g_audioManagerPluginClass.registerAudioCapturerEventListener =
        env->GetMethodID(cls, "registerAudioCapturerEventListener", "()V");
    CHECK_NULL_VOID(g_audioManagerPluginClass.registerAudioCapturerEventListener);

    g_audioManagerPluginClass.unregisterAudioCapturerEventListener =
        env->GetMethodID(cls, "unregisterAudioCapturerEventListener", "()V");
    CHECK_NULL_VOID(g_audioManagerPluginClass.unregisterAudioCapturerEventListener);

    g_audioManagerPluginClass.setDeviceActive = env->GetMethodID(cls, "setDeviceActive", "(IZ)Z");
    CHECK_NULL_VOID(g_audioManagerPluginClass.setDeviceActive);

    g_audioManagerPluginClass.isDeviceActive = env->GetMethodID(cls, "isDeviceActive", "(I)Z");
    CHECK_NULL_VOID(g_audioManagerPluginClass.isDeviceActive);

    env->DeleteLocalRef(cls);
    hasInit_ = true;
}

void AudioManagerJni::NativeOnAudioDeviceChanged(
    JNIEnv* env, jobject jobj, jboolean jIsAdded, jobjectArray jDeviceInfos)
{
    LOGD("AudioRendererJni::NativeOnAudioDeviceChanged, jIsAdded:%{public}d", jIsAdded);
    // device change
    for (const auto& it : deviceChangeCallbacks_) {
        DeviceChangeAction deviceChangeInfo;
        int32_t deviceCount = env->GetArrayLength(jDeviceInfos);
        for (int32_t i = 0; i < deviceCount; i++) {
            jobject jDeviceInfo = env->GetObjectArrayElement(jDeviceInfos, i);
            AudioDeviceDescriptor deviceInfo = AudioCommonJni::GetDeviceInfo(jDeviceInfo);
            env->DeleteLocalRef(jDeviceInfo);
            DeviceFlag flag = (deviceInfo.deviceRole_ == INPUT_DEVICE) ? INPUT_DEVICES_FLAG : OUTPUT_DEVICES_FLAG;
            if ((flag != it.first) && (it.first != ALL_DEVICES_FLAG)) {
                continue;
            }
            std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
            CHECK_NULL_VOID(audioDeviceDescriptor);
            ConvertDeviceInfoToAudioDeviceDescriptor(audioDeviceDescriptor, deviceInfo);
            deviceChangeInfo.deviceDescriptors.push_back(std::move(audioDeviceDescriptor));
        }
        if (deviceChangeInfo.deviceDescriptors.size() > 0) {
            deviceChangeInfo.flag = it.first;
            deviceChangeInfo.type = jIsAdded ? CONNECT : DISCONNECT;
            it.second->OnDeviceChange(deviceChangeInfo);
        }
    }
}

void AudioManagerJni::NativeOnAudioRendererChanged(JNIEnv* env, jobject jobj, jobject jRendererConf)
{
    auto infos = GetAudioRendererChangeInfo(jRendererConf);
    if (audioRendererEventCallback_) {
        audioRendererEventCallback_->OnRendererStateChange(infos);
    }
}

int32_t AudioManagerJni::GetVolume(AudioVolumeType volumeType)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.getVolume, ERROR);

    StreamType type = ConvertVolumeTypeToAn(volumeType);
    if (type == STREAM_INVALID) {
        LOGE("AudioManagerJni GetVolume: invalid stream type.");
        return ERR_INVALID_PARAM;
    }
    LOGD("AudioManagerJni GetVolume streamType:%{public}d", type);
    jint result = env->CallIntMethod(g_audioManagerPluginClass.globalRef, g_audioManagerPluginClass.getVolume, type);
    if (env->ExceptionCheck()) {
        LOGE("AudioManagerJni JNI: call getVolume has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    LOGD("AudioManagerJni::GetVolume Volume:%{public}d", result);
    return result;
}

int32_t AudioManagerJni::GetMaxVolume(AudioVolumeType volumeType)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.getMaxVolume, ERROR);

    StreamType type = ConvertVolumeTypeToAn(volumeType);
    if (type == STREAM_INVALID) {
        LOGE("AudioManagerJni GetMaxVolume: invalid stream type.");
        return ERR_INVALID_PARAM;
    }
    LOGD("AudioManagerJni GetMaxVolume streamType:%{public}d", type);
    jint result = env->CallIntMethod(g_audioManagerPluginClass.globalRef, g_audioManagerPluginClass.getMaxVolume, type);
    if (env->ExceptionCheck()) {
        LOGE("AudioManagerJni JNI: call getMaxVolume has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    LOGD("AudioManagerJni::GetMaxVolume Volume:%{public}d", result);
    return result;
}

int32_t AudioManagerJni::GetMinVolume(AudioVolumeType volumeType)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.getMinVolume, ERROR);

    StreamType type = ConvertVolumeTypeToAn(volumeType);
    if (type == STREAM_INVALID) {
        LOGE("AudioManagerJni GetMinVolume: invalid stream type.");
        return ERR_INVALID_PARAM;
    }
    LOGD("AudioManagerJni GetMinVolume streamType:%{public}d", type);
    jint result = env->CallIntMethod(g_audioManagerPluginClass.globalRef, g_audioManagerPluginClass.getMinVolume, type);
    if (env->ExceptionCheck()) {
        LOGE("AudioManagerJni JNI: call getMinVolume has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    if (result < 0) {
        return ERROR;
    }
    LOGD("AudioManagerJni::GetMinVolume Volume:%{public}d", result);
    return result;
}

int32_t AudioManagerJni::IsStreamMute(AudioVolumeType volumeType, bool& isMute)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.isStreamMute, ERROR);

    StreamType type = ConvertVolumeTypeToAn(volumeType);
    if (type == STREAM_INVALID) {
        LOGE("AudioManagerJni IsStreamMute: invalid stream type.");
        return ERR_INVALID_PARAM;
    }
    LOGD("AudioManagerJni IsStreamMute streamType:%{public}d", type);
    jboolean result =
        env->CallBooleanMethod(g_audioManagerPluginClass.globalRef, g_audioManagerPluginClass.isStreamMute, type);
    if (env->ExceptionCheck()) {
        LOGE("AudioManagerJni JNI: call isStreamMute has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    LOGD("AudioManagerJni IsStreamMute result:%{public}d", result);
    isMute = result;
    return SUCCESS;
}

AudioRingerMode AudioManagerJni::GetRingerMode()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, AudioRingerMode::RINGER_MODE_NORMAL);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.globalRef, AudioRingerMode::RINGER_MODE_NORMAL);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.getRingerMode, AudioRingerMode::RINGER_MODE_NORMAL);

    jint result = env->CallIntMethod(g_audioManagerPluginClass.globalRef, g_audioManagerPluginClass.getRingerMode);
    if (env->ExceptionCheck()) {
        LOGE("AudioManagerJni JNI: call getRingerMode has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return AudioRingerMode::RINGER_MODE_NORMAL;
    }
    AudioRingerMode mode = ConvertRingerModeToOh(static_cast<RingerMode>(result));
    return mode;
}

bool AudioManagerJni::IsMicrophoneMute()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.globalRef, false);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.isMicrophoneMute, false);

    jboolean result =
        env->CallBooleanMethod(g_audioManagerPluginClass.globalRef, g_audioManagerPluginClass.isMicrophoneMute);
    if (env->ExceptionCheck()) {
        LOGE("AudioManagerJni JNI: call isMicrophoneMute has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return result;
}

bool AudioManagerJni::IsVolumeFixed()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.globalRef, false);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.isVolumeFixed, false);

    jboolean result =
        env->CallBooleanMethod(g_audioManagerPluginClass.globalRef, g_audioManagerPluginClass.isVolumeFixed);
    if (env->ExceptionCheck()) {
        LOGE("AudioManagerJni JNI: call isVolumeFixed has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    LOGD("AudioManagerJni::IsVolumeFixed result:%{public}d", result);
    return result;
}

float AudioManagerJni::GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType)
{
    LOGD("AudioManagerJni::GetSystemVolumeInDb, volumeType:%{public}d, volumeLevel:%{public}d, deviceType:%{public}d",
        volumeType, volumeLevel, deviceType);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, 0.0f);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.globalRef, 0.0f);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.getSystemVolumeInDb, 0.0f);

    StreamType streamType = ConvertVolumeTypeToAn(volumeType);
    if (streamType == STREAM_INVALID) {
        LOGE("AudioManagerJni GetSystemVolumeInDb: invalid stream type.");
        return 0.0f;
    }
    AudioDeviceType deviceTypeAn = ConvertDeviceTypeToAn(deviceType);
    LOGD("AudioManagerJni GetMinVolume after convert:streamType:%{public}d, deviceType:%{public}d", streamType,
        deviceTypeAn);
    jfloat result = env->CallFloatMethod(g_audioManagerPluginClass.globalRef,
        g_audioManagerPluginClass.getSystemVolumeInDb, streamType, volumeLevel, deviceTypeAn);
    if (env->ExceptionCheck()) {
        LOGE("AudioManagerJni JNI: call getSystemVolumeInDb has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return 0.0f;
    }
    LOGD("AudioManagerJni GetSystemVolumeInDb result:%{public}f", result);
    return result;
}

AudioScene AudioManagerJni::GetAudioScene()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, AUDIO_SCENE_INVALID);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.globalRef, AUDIO_SCENE_INVALID);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.getAudioScene, AUDIO_SCENE_INVALID);

    jint result = env->CallIntMethod(g_audioManagerPluginClass.globalRef, g_audioManagerPluginClass.getAudioScene);
    if (env->ExceptionCheck()) {
        LOGE("AudioManagerJni JNI: call GetAudioScene has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return AUDIO_SCENE_INVALID;
    }
    AudioScene scene = ConvertSceneToOh(static_cast<AudioMode>(result));
    LOGD("AudioManagerJni::GetAudioScene before convert scene:%{public}d, after convert scene:%{public}d",
        static_cast<AudioScene>(result), scene);
    return scene;
}

int32_t AudioManagerJni::GetDevices(DeviceFlag deviceFlag, std::vector<std::shared_ptr<AudioDeviceDescriptor>>& desc)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.getDevices, ERROR);

    AudioDeviceFlags flag = ConvertDeviceFlagToAn(deviceFlag);
    if (flag == GET_DEVICES_INVALID) {
        LOGE("AudioManagerJni JNI: invalid device flag.");
        return ERR_INVALID_PARAM;
    }
    jobject jDevices =
        env->CallObjectMethod(g_audioManagerPluginClass.globalRef, g_audioManagerPluginClass.getDevices, flag);
    if (env->ExceptionCheck() || !jDevices) {
        LOGE("AudioManagerJni JNI: call getDevices has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    int32_t deviceCount = env->GetArrayLength(static_cast<jobjectArray>(jDevices));
    for (int32_t i = 0; i < deviceCount; i++) {
        jobject jDeviceInfo = env->GetObjectArrayElement(static_cast<jobjectArray>(jDevices), i);
        AudioDeviceDescriptor deviceInfo = AudioCommonJni::GetDeviceInfo(jDeviceInfo);
        env->DeleteLocalRef(jDeviceInfo);

        std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
        if (!audioDeviceDescriptor) {
            env->DeleteLocalRef(jDevices);
            LOGE("AudioManagerJni JNI: create AudioDeviceDescriptor failed.");
            return ERROR;
        }
        ConvertDeviceInfoToAudioDeviceDescriptor(audioDeviceDescriptor, deviceInfo);
        desc.push_back(std::move(audioDeviceDescriptor));
    }
    env->DeleteLocalRef(jDevices);
    return SUCCESS;
}

void AudioManagerJni::ConvertDeviceInfoToAudioDeviceDescriptor(
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor, const AudioDeviceDescriptor& deviceInfo)
{
    CHECK_NULL_VOID(audioDeviceDescriptor);
    audioDeviceDescriptor->deviceRole_ = deviceInfo.deviceRole_;
    audioDeviceDescriptor->deviceType_ = deviceInfo.deviceType_;
    audioDeviceDescriptor->deviceId_ = deviceInfo.deviceId_;
    audioDeviceDescriptor->channelMasks_ = deviceInfo.channelMasks_;
    audioDeviceDescriptor->channelIndexMasks_ = deviceInfo.channelIndexMasks_;
    audioDeviceDescriptor->deviceName_ = deviceInfo.deviceName_;
    audioDeviceDescriptor->macAddress_ = deviceInfo.macAddress_;
    audioDeviceDescriptor->interruptGroupId_ = deviceInfo.interruptGroupId_;
    audioDeviceDescriptor->volumeGroupId_ = deviceInfo.volumeGroupId_;
    audioDeviceDescriptor->networkId_ = deviceInfo.networkId_;
    audioDeviceDescriptor->displayName_ = deviceInfo.displayName_;
    audioDeviceDescriptor->audioStreamInfo_ = deviceInfo.audioStreamInfo_;
}

int32_t AudioManagerJni::AddDeviceChangeCallback(
    const DeviceFlag flag, const std::shared_ptr<AudioManagerDeviceChangeCallback>& callback)
{
    LOGD("AudioManagerJni AddDeviceChangeCallback: flag:%{public}d", flag);
    AudioDeviceFlags deviceflag = ConvertDeviceFlagToAn(flag);
    if (deviceflag == GET_DEVICES_INVALID) {
        LOGE("AudioManagerJni JNI: invalid device flag.");
        return ERR_INVALID_PARAM;
    }
    deviceChangeCallbacks_[flag] = callback;
    return RegisterAudioDeviceCallback();
}

int32_t AudioManagerJni::RemoveDeviceChangeCallback()
{
    deviceChangeCallbacks_.clear();
    return UnregisterAudioDeviceCallback();
}

int32_t AudioManagerJni::RegisterAudioDeviceCallback()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.registerAudioDeviceCallback, ERROR);

    env->CallVoidMethod(g_audioManagerPluginClass.globalRef, g_audioManagerPluginClass.registerAudioDeviceCallback);
    if (env->ExceptionCheck()) {
        LOGE("AudioManagerJni JNI: call registerAudioDeviceCallback has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    return SUCCESS;
}

int32_t AudioManagerJni::UnregisterAudioDeviceCallback()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.unregisterAudioDeviceCallback, ERROR);

    env->CallVoidMethod(g_audioManagerPluginClass.globalRef, g_audioManagerPluginClass.unregisterAudioDeviceCallback);
    if (env->ExceptionCheck()) {
        LOGE("AudioManagerJni JNI: call unregisterAudioDeviceCallback has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    return SUCCESS;
}

void AudioManagerJni::NativeOnAudioCapturerChanged(JNIEnv* env, jobject jobj, jobject jCapturerConf)
{
    auto infos = GetAudioCapturerChangeInfo(jCapturerConf);
    if (audioCapturerEventCallback_) {
        audioCapturerEventCallback_->OnCapturerStateChange(infos);
    }
}

std::vector<std::shared_ptr<AudioRendererChangeInfo>> AudioManagerJni::GetAudioRendererChangeInfo(jobject jRendererConf)
{
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> infos;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, infos);
    jclass jListPlaybackConfigurationCls = env->FindClass("java/util/List");
    jclass jAudioPlayCfgCls = env->FindClass("android/media/AudioPlaybackConfiguration");
    jclass jAttributeCls = env->FindClass("android/media/AudioAttributes");
    jmethodID jUsageId = env->GetMethodID(jAttributeCls, "getUsage", "()I");
    jmethodID jContentTypeId = env->GetMethodID(jAttributeCls, "getContentType", "()I");
    jmethodID sizeMethod = env->GetMethodID(jListPlaybackConfigurationCls, "size", "()I");
    jmethodID getMethod = env->GetMethodID(jListPlaybackConfigurationCls, "get", "(I)Ljava/lang/Object;");
    jmethodID jAudioID = env->GetMethodID(jAudioPlayCfgCls, "getAudioAttributes", "()Landroid/media/AudioAttributes;");
    jmethodID jDevID = env->GetMethodID(jAudioPlayCfgCls, "getAudioDeviceInfo", "()Landroid/media/AudioDeviceInfo;");
    if (!jUsageId || !jContentTypeId || !sizeMethod || !getMethod) {
        LOGE("The Android version does not support this interface, please use at least Android 12");
        return infos;
    }
    jint size = env->CallIntMethod(jRendererConf, sizeMethod);
    env->DeleteLocalRef(jListPlaybackConfigurationCls);
    env->DeleteLocalRef(jAudioPlayCfgCls);
    env->DeleteLocalRef(jAttributeCls);
    for (int i = 0; i < size; i++) {
        jobject objRenderer = env->CallObjectMethod(jRendererConf, getMethod, i);
        if (objRenderer) {
            jobject jAttribute = env->CallObjectMethod(objRenderer, jAudioID);
            if (!jAttribute) {
                env->DeleteLocalRef(objRenderer);
                continue;
            }
            jint jContentType = env->CallIntMethod(jAttribute, jContentTypeId);
            jint jUsage = env->CallIntMethod(jAttribute, jUsageId);
            env->DeleteLocalRef(jAttribute);
            jobject jDeviceInfo = env->CallObjectMethod(objRenderer, jDevID);
            if (!jDeviceInfo) {
                env->DeleteLocalRef(objRenderer);
                continue;
            }
            std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
            ConvertAudioUsageToOh(static_cast<AudioAttributesUsage>(jUsage),
                static_cast<AudioAttributesContenType>(jContentType), rendererChangeInfo->rendererInfo.streamUsage,
                rendererChangeInfo->rendererInfo.contentType);
            rendererChangeInfo->outputDeviceInfo = AudioCommonJni::GetDeviceInfo(jDeviceInfo);
            infos.push_back(std::move(rendererChangeInfo));
            rendererChangeInfo = nullptr;
            env->DeleteLocalRef(jDeviceInfo);
            env->DeleteLocalRef(objRenderer);
        }
    }
    return infos;
}

std::vector<std::shared_ptr<AudioCapturerChangeInfo>> AudioManagerJni::GetAudioCapturerChangeInfo(jobject jCapturerConf)
{
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> infos;

    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, infos);

    jclass jListRecordingConfigurationCls = env->FindClass("java/util/List");
    jclass jAudioRecordingConfigurationCls = env->FindClass("android/media/AudioRecordingConfiguration");
    jmethodID sizeMethod = env->GetMethodID(jListRecordingConfigurationCls, "size", "()I");
    jmethodID getMethod = env->GetMethodID(jListRecordingConfigurationCls, "get", "(I)Ljava/lang/Object;");
    jmethodID jSessionIdID = env->GetMethodID(jAudioRecordingConfigurationCls, "getClientAudioSessionId", "()I");
    jmethodID jSourceTypeID = env->GetMethodID(jAudioRecordingConfigurationCls, "getClientAudioSource", "()I");
    jmethodID jDeviceInfoID =
        env->GetMethodID(jAudioRecordingConfigurationCls, "getAudioDevice", "()Landroid/media/AudioDeviceInfo;");
    jmethodID jMutedID = env->GetMethodID(jAudioRecordingConfigurationCls, "isClientSilenced", "()Z");
    if (!jSessionIdID || !jSourceTypeID || !jDeviceInfoID || !jMutedID || !sizeMethod || !getMethod) {
        LOGE("The Android version does not support this interface, please use at least Android 12");
        return infos;
    }
    jint size = env->CallIntMethod(jCapturerConf, sizeMethod);
    env->DeleteLocalRef(jListRecordingConfigurationCls);
    env->DeleteLocalRef(jAudioRecordingConfigurationCls);
    for (int i = 0; i < size; i++) {
        jobject objCapturer = env->CallObjectMethod(jCapturerConf, getMethod, i);
        if (!objCapturer) {
            continue;
        }
        jint jSessionId = env->CallIntMethod(objCapturer, jSessionIdID);
        jint jSourceType = env->CallIntMethod(objCapturer, jSourceTypeID);
        jobject jDeviceInfo = env->CallObjectMethod(objCapturer, jDeviceInfoID);
        jboolean jMuted = env->CallBooleanMethod(objCapturer, jMutedID);
        if (!jDeviceInfo) {
            env->DeleteLocalRef(objCapturer);
            continue;
        }
        std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
        capturerChangeInfo->sessionId = static_cast<int32_t>(jSessionId);
        capturerChangeInfo->capturerInfo.sourceType = ConvertSourceTypeToOh(static_cast<AudioSourceType>(jSourceType));
        capturerChangeInfo->capturerInfo.capturerFlags = 0;
        capturerChangeInfo->inputDeviceInfo = AudioCommonJni::GetDeviceInfo(jDeviceInfo);
        capturerChangeInfo->muted = jMuted;
        infos.push_back(std::move(capturerChangeInfo));
        capturerChangeInfo = nullptr;
        env->DeleteLocalRef(jDeviceInfo);
        env->DeleteLocalRef(objCapturer);
    }
    return infos;
}

int32_t AudioManagerJni::RegisterAudioRendererEventListener(
    const std::shared_ptr<AudioRendererStateChangeCallback>& callback)
{
    audioRendererEventCallback_ = callback;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.registerAudioRendererEventListener, ERROR);

    jint result = env->CallIntMethod(
        g_audioManagerPluginClass.globalRef, g_audioManagerPluginClass.registerAudioRendererEventListener);
    if (env->ExceptionCheck()) {
        LOGE("AudioManagerJni JNI: call registerAudioRendererEventListener has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    return result != SUCCESS ? ERR_OPERATION_FAILED : SUCCESS;
}

int32_t AudioManagerJni::UnregisterAudioRendererEventListener()
{
    audioRendererEventCallback_ = nullptr;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.unregisterAudioRendererEventListener, ERROR);

    jint result = env->CallIntMethod(
        g_audioManagerPluginClass.globalRef, g_audioManagerPluginClass.unregisterAudioRendererEventListener);
    if (env->ExceptionCheck()) {
        LOGE("AudioManagerJni JNI: call unregisterAudioRendererEventListener has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    return result != SUCCESS ? ERR_OPERATION_FAILED : SUCCESS;
}

int32_t AudioManagerJni::RegisterAudioCapturerEventListener(
    const std::shared_ptr<AudioCapturerStateChangeCallback>& callback)
{
    audioCapturerEventCallback_ = callback;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.registerAudioCapturerEventListener, ERROR);

    jint result = env->CallIntMethod(
        g_audioManagerPluginClass.globalRef, g_audioManagerPluginClass.registerAudioCapturerEventListener);
    if (env->ExceptionCheck()) {
        LOGE("AudioManagerJni JNI: call registerAudioCapturerEventListener has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    return result != SUCCESS ? ERR_OPERATION_FAILED : SUCCESS;
}

int32_t AudioManagerJni::UnregisterAudioCapturerEventListener()
{
    audioCapturerEventCallback_ = nullptr;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.unregisterAudioCapturerEventListener, ERROR);

    jint result = env->CallIntMethod(
        g_audioManagerPluginClass.globalRef, g_audioManagerPluginClass.unregisterAudioCapturerEventListener);
    if (env->ExceptionCheck()) {
        LOGE("AudioManagerJni JNI: call unregisterAudioCapturerEventListener has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    return result != SUCCESS ? ERR_OPERATION_FAILED : SUCCESS;
}

int32_t AudioManagerJni::GetCurrentRendererChangeInfos(
    std::vector<std::shared_ptr<AudioRendererChangeInfo>>& audioRendererChangeInfos)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.getCurrentAudioRendererInfoArray, ERROR);
    jobject jRendererChangeInfos = env->CallObjectMethod(
        g_audioManagerPluginClass.globalRef, g_audioManagerPluginClass.getCurrentAudioRendererInfoArray);
    if (env->ExceptionCheck()) {
        LOGE("AudioManagerJni JNI: call getCurrentAudioRendererInfoArray has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    if (!jRendererChangeInfos) {
        LOGE("AudioManagerJni JNI: jRendererChangeInfos is null");
        return ERR_OPERATION_FAILED;
    }
    audioRendererChangeInfos = GetAudioRendererChangeInfo(jRendererChangeInfos);
    env->DeleteLocalRef(jRendererChangeInfos);
    return SUCCESS;
}

int32_t AudioManagerJni::GetCurrentCapturerChangeInfos(
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>>& audioCapturerChangeInfos)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.getCurrentAudioCapturerInfoArray, ERROR);
    jobject jCapturerChangeInfos = env->CallObjectMethod(
        g_audioManagerPluginClass.globalRef, g_audioManagerPluginClass.getCurrentAudioCapturerInfoArray);
    if (env->ExceptionCheck()) {
        LOGE("AudioManagerJni JNI: call getCurrentAudioCapturerInfoArray has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    if (!jCapturerChangeInfos) {
        LOGE("AudioManagerJni JNI: jCapturerChangeInfos is null");
        return ERR_OPERATION_FAILED;
    }
    audioCapturerChangeInfos = GetAudioCapturerChangeInfo(jCapturerChangeInfos);
    env->DeleteLocalRef(jCapturerChangeInfos);
    return SUCCESS;
}

AudioVolumeType GetVolumeTypeFromStreamUsage(StreamUsage streamUsage)
{
    switch (streamUsage) {
        case STREAM_USAGE_VOICE_MESSAGE:
        case STREAM_USAGE_VOICE_COMMUNICATION:
            return AudioStreamType::STREAM_VOICE_CALL;
        case STREAM_USAGE_RINGTONE:
        case STREAM_USAGE_NOTIFICATION:
            return AudioStreamType::STREAM_RING;
        case STREAM_USAGE_MUSIC:
        case STREAM_USAGE_MOVIE:
        case STREAM_USAGE_GAME:
        case STREAM_USAGE_AUDIOBOOK:
        case STREAM_USAGE_NAVIGATION:
            return AudioStreamType::STREAM_MUSIC;
        case STREAM_USAGE_VOICE_ASSISTANT:
            return STREAM_VOICE_ASSISTANT;
        case STREAM_USAGE_ALARM:
            return AudioStreamType::STREAM_ALARM;
        case STREAM_USAGE_ACCESSIBILITY:
            return AudioStreamType::STREAM_ACCESSIBILITY;
        default:
            return AudioStreamType::STREAM_MUSIC;
    }
}

bool AudioManagerJni::IsStreamActive(AudioVolumeType volumeType)
{
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    int32_t result = GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    if (result != SUCCESS) {
        LOGE("GetCurrentRendererChangeInfos failed");
        return false;
    }
    for (const auto& info : audioRendererChangeInfos) {
        if ((GetVolumeTypeFromStreamUsage(info->rendererInfo.streamUsage) == volumeType)) {
            return true;
        }
    }
    return false;
}

int32_t AudioManagerJni::SetDeviceActive(DeviceType deviceType, bool flag)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.setDeviceActive, ERROR);
    jint deviceTypeAn = ConvertActiveDeviceTypeToAn(deviceType);
    jboolean result = env->CallBooleanMethod(
        g_audioManagerPluginClass.globalRef, g_audioManagerPluginClass.setDeviceActive, deviceTypeAn, flag);
    if (env->ExceptionCheck()) {
        LOGE("AudioManagerJni JNI: call SetDeviceActive has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    return result ? SUCCESS : ERROR;
}

bool AudioManagerJni::IsDeviceActive(DeviceType deviceType)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioManagerPluginClass.isDeviceActive, ERROR);
    jint deviceTypeAn = ConvertActiveDeviceTypeToAn(deviceType);
    jboolean result = env->CallBooleanMethod(
        g_audioManagerPluginClass.globalRef, g_audioManagerPluginClass.isDeviceActive, deviceTypeAn);
    if (env->ExceptionCheck()) {
        LOGE("AudioManagerJni JNI: call isDeviceActive has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return result;
}
} // namespace OHOS::Plugin
