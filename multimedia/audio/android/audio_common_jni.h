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

#ifndef PLUGINS_MULTIMEDIA_AUDIO_ANDROID_AUDIO_COMMON_JNI_H
#define PLUGINS_MULTIMEDIA_AUDIO_ANDROID_AUDIO_COMMON_JNI_H

#include <jni.h>

#include "audio_device_info.h"
#include "audio_device_descriptor.h"

namespace OHOS::Plugin {
using namespace OHOS::AudioStandard;
class AudioCommonJni final {
public:
    AudioCommonJni() = delete;
    ~AudioCommonJni() = delete;
    static AudioDeviceDescriptor GetDeviceInfo(jobject jDeviceInfo);

private:
    static DeviceRole GetDeviceRole(JNIEnv* env, jclass jDeviceInfoCls, jobject jDeviceInfo);
    static DeviceType GetDeviceType(JNIEnv* env, jclass jDeviceInfoCls, jobject jDeviceInfo);
    static int32_t GetDeviceId(JNIEnv* env, jclass jDeviceInfoCls, jobject jDeviceInfo);
    static std::string GetDeviceName(JNIEnv* env, jclass jDeviceInfoCls, jobject jDeviceInfo);
    static std::string GetDeviceAddress(JNIEnv* env, jclass jDeviceInfoCls, jobject jDeviceInfo);
    static std::set<AudioSamplingRate> GetDeviceSampleRates(JNIEnv* env, jclass jDeviceInfoCls, jobject jDeviceInfo);
    static std::set<AudioChannel> GetDeviceAudioChannels(JNIEnv* env, jclass jDeviceInfoCls, jobject jDeviceInfo);
    static int32_t GetDeviceChannelMask(JNIEnv* env, jclass jDeviceInfoCls, jobject jDeviceInfo);
    static AudioEncodingType GetDeviceEncoding(JNIEnv* env, jclass jDeviceInfoCls, jobject jDeviceInfo);
};
} // namespace OHOS::Plugin
#endif // PLUGINS_MULTIMEDIA_AUDIO_ANDROID_AUDIO_COMMON_JNI_H
