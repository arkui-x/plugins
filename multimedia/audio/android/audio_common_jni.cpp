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

#include "audio_common_jni.h"
#include "audio_convert_util.h"
#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"

using namespace OHOS::AudioStandard;

namespace OHOS::Plugin {

DeviceRole AudioCommonJni::GetDeviceRole(JNIEnv* env, jclass jDeviceInfoCls, jobject jDeviceInfo)
{
    jmethodID jIsSourceId = env->GetMethodID(jDeviceInfoCls, "isSource", "()Z");
    jmethodID jIsSinkId = env->GetMethodID(jDeviceInfoCls, "isSink", "()Z");
    if (!jIsSourceId || !jIsSinkId) {
        LOGD("AudioCommonJni::GetDeviceRole jmethodID is null.");
        return DEVICE_ROLE_NONE;
    }
    jboolean isSource = env->CallBooleanMethod(jDeviceInfo, jIsSourceId);
    jboolean isSink = env->CallBooleanMethod(jDeviceInfo, jIsSinkId);

    DeviceRole role = isSource && !isSink ? INPUT_DEVICE : OUTPUT_DEVICE;

    LOGD("AudioCommonJni::GetDeviceRole before convert:isSource:%{public}d, isSink:%{public}d;\
        after convert: deviceRole:%{public}d;",
        isSource, isSink, role);

    return role;
}

DeviceType AudioCommonJni::GetDeviceType(JNIEnv* env, jclass jDeviceInfoCls, jobject jDeviceInfo)
{
    jmethodID jGetTypeId = env->GetMethodID(jDeviceInfoCls, "getType", "()I");
    if (!jGetTypeId) {
        LOGD("AudioCommonJni::GetDeviceType jmethodID is null.");
        return DEVICE_TYPE_NONE;
    }
    jint deviceType = env->CallIntMethod(jDeviceInfo, jGetTypeId);
    DeviceType type = ConvertDeviceTypeToOh(static_cast<AudioDeviceType>(deviceType));

    LOGD("AudioCommonJni::GetDeviceType before convert:deviceType:%{public}d; after convert: type:%{public}d;",
        deviceType, type);
    return type;
}

int32_t AudioCommonJni::GetDeviceId(JNIEnv* env, jclass jDeviceInfoCls, jobject jDeviceInfo)
{
    jmethodID jGetDeivceId = env->GetMethodID(jDeviceInfoCls, "getId", "()I");
    if (!jGetDeivceId) {
        LOGD("AudioCommonJni::GetDeviceId jmethodID is null.");
        return 0;
    }
    jint deviceId = env->CallIntMethod(jDeviceInfo, jGetDeivceId);

    return static_cast<int32_t>(deviceId);
}

std::string AudioCommonJni::GetDeviceName(JNIEnv* env, jclass jDeviceInfoCls, jobject jDeviceInfo)
{
    std::string name = "";
    jclass jCharSeqCls = env->FindClass("java/lang/CharSequence");
    jmethodID jToStringId = env->GetMethodID(jCharSeqCls, "toString", "()Ljava/lang/String;");
    jmethodID jGetNameId = env->GetMethodID(jDeviceInfoCls, "getProductName", "()Ljava/lang/CharSequence;");
    env->DeleteLocalRef(jCharSeqCls);
    if (!jToStringId || !jGetNameId) {
        LOGD("AudioCommonJni::GetDeviceName jmethodID is null.");
        return name;
    }
    jobject jCharName = env->CallObjectMethod(jDeviceInfo, jGetNameId);
    if (!jCharName) {
        LOGD("AudioCommonJni::GetDeviceName jCharName is null.");
        return name;
    }
    jstring jName = static_cast<jstring>(env->CallObjectMethod(jCharName, jToStringId));
    const char* deviceName = env->GetStringUTFChars(jName, NULL);
    if (deviceName) {
        name = deviceName;
        env->ReleaseStringUTFChars(jName, deviceName);
    }
    env->DeleteLocalRef(jName);
    env->DeleteLocalRef(jCharName);

    return name;
}

std::string AudioCommonJni::GetDeviceAddress(JNIEnv* env, jclass jDeviceInfoCls, jobject jDeviceInfo)
{
    std::string address = "";
    jmethodID jGetAddressId = env->GetMethodID(jDeviceInfoCls, "getAddress", "()Ljava/lang/String;");
    if (!jGetAddressId) {
        LOGD("AudioCommonJni::GetDeviceAddress jmethodID is null.");
        return address;
    }
    jstring jAddress = static_cast<jstring>(env->CallObjectMethod(jDeviceInfo, jGetAddressId));
    const char* macAddress = env->GetStringUTFChars(jAddress, NULL);
    if (macAddress) {
        address = macAddress;
        env->ReleaseStringUTFChars(jAddress, macAddress);
    }
    env->DeleteLocalRef(jAddress);

    return address;
}

std::set<AudioSamplingRate> AudioCommonJni::GetDeviceSampleRates(
    JNIEnv* env, jclass jDeviceInfoCls, jobject jDeviceInfo)
{
    std::set<AudioSamplingRate> samplingRates;
    jmethodID jGetSampleRatesId = env->GetMethodID(jDeviceInfoCls, "getSampleRates", "()[I");
    if (!jGetSampleRatesId) {
        LOGD("AudioCommonJni::GetDeviceSampleRates jmethodID is null.");
        return samplingRates;
    }
    jintArray jSampleRates = static_cast<jintArray>(env->CallObjectMethod(jDeviceInfo, jGetSampleRatesId));
    if (!jSampleRates) {
        LOGD("AudioCommonJni::GetDeviceSampleRates jSampleRates is null.");
        return samplingRates;
    }
    int32_t* sampleRates = env->GetIntArrayElements(jSampleRates, NULL);
    if (sampleRates) {
        int32_t sampleRateSize = env->GetArrayLength(jSampleRates);
        for (int32_t i = 0; i < sampleRateSize; i++) {
            LOGD("AudioCommonJni::GetDeviceSampleRates sampleRates[%{public}d]:%{public}d", i, sampleRates[i]);
            samplingRates.insert(static_cast<AudioSamplingRate>(sampleRates[i]));
        }
        env->ReleaseIntArrayElements(jSampleRates, sampleRates, 0);
    }
    env->DeleteLocalRef(jSampleRates);

    return samplingRates;
}

std::set<AudioChannel> AudioCommonJni::GetDeviceAudioChannels(JNIEnv* env, jclass jDeviceInfoCls, jobject jDeviceInfo)
{
    std::set<AudioChannel> channels;
    jmethodID jGetChannelCntId = env->GetMethodID(jDeviceInfoCls, "getChannelCounts", "()[I");
    if (!jGetChannelCntId) {
        LOGD("AudioCommonJni::GetDeviceAudioChannels jmethodID is null.");
        return channels;
    }
    jintArray jChannelCounts = static_cast<jintArray>(env->CallObjectMethod(jDeviceInfo, jGetChannelCntId));
    if (!jChannelCounts) {
        LOGD("AudioCommonJni::GetDeviceAudioChannels jChannelCounts is null.");
        return channels;
    }
    int32_t* channelCnts = env->GetIntArrayElements(jChannelCounts, NULL);
    if (channelCnts) {
        int32_t channelCntSize = env->GetArrayLength(jChannelCounts);
        for (int32_t i = 0; i < channelCntSize; i++) {
            LOGD("AudioCommonJni::GetDeviceAudioChannels channelCnts[%{public}d]:%{public}d", i, channelCnts[i]);
            channels.insert(static_cast<AudioChannel>(channelCnts[i]));
        }
        env->ReleaseIntArrayElements(jChannelCounts, channelCnts, 0);
    }
    env->DeleteLocalRef(jChannelCounts);

    return channels;
}

int32_t AudioCommonJni::GetDeviceChannelMask(JNIEnv* env, jclass jDeviceInfoCls, jobject jDeviceInfo)
{
    int32_t mask = 0;
    jmethodID jGetChannelMaskId = env->GetMethodID(jDeviceInfoCls, "getChannelMasks", "()[I");
    if (!jGetChannelMaskId) {
        LOGD("AudioCommonJni::GetDeviceChannelMask jmethodID is null.");
        return mask;
    }
    jintArray jChannelMasks = static_cast<jintArray>(env->CallObjectMethod(jDeviceInfo, jGetChannelMaskId));
    if (!jChannelMasks) {
        LOGD("AudioCommonJni::GetDeviceChannelMask jChannelMasks is null.");
        return mask;
    }
    int32_t* channelMasks = env->GetIntArrayElements(jChannelMasks, NULL);
    if (channelMasks) {
        int32_t channelMaskSize = env->GetArrayLength(jChannelMasks);
        if (channelMaskSize > 0) {
            mask = channelMasks[0];
        }
        env->ReleaseIntArrayElements(jChannelMasks, channelMasks, 0);
    }
    env->DeleteLocalRef(jChannelMasks);
    LOGD("AudioCommonJni::GetDeviceChannelMask mask:%{public}d", mask);
    return mask;
}

AudioEncodingType AudioCommonJni::GetDeviceEncoding(JNIEnv* env, jclass jDeviceInfoCls, jobject jDeviceInfo)
{
    AudioEncodingType encode = AudioEncodingType::ENCODING_INVALID;
    jmethodID jGetEncodingId = env->GetMethodID(jDeviceInfoCls, "getEncodings", "()[I");
    if (!jGetEncodingId) {
        LOGD("AudioCommonJni::GetDeviceEncoding jmethodID is null.");
        return encode;
    }
    jintArray jEncodings = static_cast<jintArray>(env->CallObjectMethod(jDeviceInfo, jGetEncodingId));
    if (!jEncodings) {
        LOGD("AudioCommonJni::GetDeviceEncoding jEncodings is null.");
        return encode;
    }
    int32_t* encodings = env->GetIntArrayElements(jEncodings, NULL);
    if (encodings) {
        int32_t encodingSize = env->GetArrayLength(jEncodings);
        for (int32_t i = 0; i < encodingSize; i++) {
            LOGD("AudioCommonJni::GetDeviceEncoding encodings[%{public}d]:%{public}d", i, encodings[i]);
            if (IsPCMFormat(encodings[i])) {
                encode = AudioEncodingType::ENCODING_PCM;
                break;
            }
        }
        env->ReleaseIntArrayElements(jEncodings, encodings, 0);
    }
    env->DeleteLocalRef(jEncodings);

    return encode;
}

AudioDeviceDescriptor AudioCommonJni::GetDeviceInfo(jobject jDeviceInfo)
{
    AudioDeviceDescriptor deviceInfo;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, deviceInfo);
    CHECK_NULL_RETURN(jDeviceInfo, deviceInfo);

    jclass jDeviceInfoCls = env->FindClass("android/media/AudioDeviceInfo");
    // Device role
    deviceInfo.deviceRole_ = GetDeviceRole(env, jDeviceInfoCls, jDeviceInfo);
    // Device type
    deviceInfo.deviceType_ = GetDeviceType(env, jDeviceInfoCls, jDeviceInfo);
    // id
    deviceInfo.deviceId_ = GetDeviceId(env, jDeviceInfoCls, jDeviceInfo);
    // device name
    deviceInfo.deviceName_ = GetDeviceName(env, jDeviceInfoCls, jDeviceInfo);
    // address
    deviceInfo.macAddress_ = GetDeviceAddress(env, jDeviceInfoCls, jDeviceInfo);
    DeviceStreamInfo deviceStreamInfo;
    // sample rates
    deviceStreamInfo.samplingRate = GetDeviceSampleRates(env, jDeviceInfoCls, jDeviceInfo);
    // channel counts
    deviceStreamInfo.SetChannels(GetDeviceAudioChannels(env, jDeviceInfoCls, jDeviceInfo));
    // channel masks
    deviceInfo.channelMasks_ = GetDeviceChannelMask(env, jDeviceInfoCls, jDeviceInfo);
    // Encodings
    deviceStreamInfo.encoding = GetDeviceEncoding(env, jDeviceInfoCls, jDeviceInfo);

    deviceInfo.audioStreamInfo_.push_back(deviceStreamInfo);
    env->DeleteLocalRef(jDeviceInfoCls);

    return deviceInfo;
}
} // namespace OHOS::Plugin
