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
#ifndef PLUGINS_VIBRATOR_PLUGIN_JAVA_JNI_VIBRATOR_PLUGIN_JNI_H
#define PLUGINS_VIBRATOR_PLUGIN_JAVA_JNI_VIBRATOR_PLUGIN_JNI_H

#include <jni.h>
#include <memory>
#include <string>
#include <vector>
#include <mutex>

namespace OHOS::Sensors {

struct VibratorIdentifierTrans {
    int32_t deviceId = -1;
    int32_t vibratorId = -1;
    int32_t position = 0;
    bool isLocalVibrator;
};

struct PrimitiveEffectTrans {
    int32_t intensity = 0;
    int32_t usage = 0;
    bool systemUsage;
    int32_t count = 0;
};

struct VibrateParameterTrans {
    int32_t intensity = 100;
    int32_t frequency = 0;
    int32_t reserved = 0;
    uint32_t sessionId = 0;
};

struct CustomHapticInfoTrans {
    int32_t usage = 0;
    bool systemUsage;
    VibrateParameterTrans parameter;
};

struct EffectInfoTrans {
    int32_t duration = -1;
    bool isSupportEffect;
};

struct VibratorInfoTrans {
    int32_t deviceId = -1;
    int32_t vibratorId = -1;
    std::string deviceName = "";
    bool isSupportHdHaptic;
    bool isLocalVibrator;
    int32_t position = 0;
};

const int32_t VIBRATOR_LIST_LENGTH = 6;
const int32_t JNI_INTENSITY_MAX = 255;
const int32_t OH_INTENSITY_MAX = 100;
const int32_t OH_INTENSITY_MIN = -100;
const int32_t VIBRATOR_MIN_UNIT = 2;
const int32_t VIBRATOR_DEFAULT_DURATION = 1000;
const int32_t VIBRATE_FAILED = -1;
const int32_t VIBRATE_SUCCESS = 0;

struct VibrateCurvePointTrans {
    bool operator<(const VibrateCurvePointTrans &rhs) const
    {
        return time < rhs.time;
    }
    int32_t time = 0;
    int32_t intensity = 0;
    int32_t frequency = 0;
};

enum VibrateTagTrans {
    MY_EVENT_TAG_UNKNOWN = -1,
    MY_EVENT_TAG_CONTINUOUS = 0,
    MY_EVENT_TAG_TRANSIENT = 1,
};

struct VibrateEventTrans {
    bool operator<(const VibrateEventTrans &rhs) const
    {
        return time < rhs.time;
    }

    VibrateTagTrans tag;
    int32_t time = 0;
    int32_t duration = 0;
    int32_t intensity = 0;
    int32_t frequency = 0;
    int32_t index = 0;
    std::vector<VibrateCurvePointTrans> points;
};

struct VibratePatternTrans {
    bool operator<(const VibratePatternTrans &rhs) const
    {
        return startTime < rhs.startTime;
    }
    int32_t startTime = 0;
    int32_t patternDuration = 0;
    std::vector<VibrateEventTrans> events;
};

struct VibratorCapacityTrans {
    bool isSupportHdHaptic = false;
    bool isSupportPresetMapping = false;
    bool isSupportTimeDelay = false;
};

enum VibrateCustomModeTrans {
    MY_VIBRATE_MODE_HD = 0,
    MY_VIBRATE_MODE_MAPPING = 1,
    MY_VIBRATE_MODE_TIMES = 2,
};

class VibratorJni final {
public:
    VibratorJni() = delete;
    ~VibratorJni() = delete;
    static bool Register(void* env);
    static void NativeInit(JNIEnv* env, jobject jobj);
    static void NativeGetEffectInfo(JNIEnv* env, jobject thiz, jint duration, jboolean isSupportEffect);
    static VibratorInfoTrans ParseSingleVibratorInfo(JNIEnv* env, jobjectArray vibratorInfo);
    static void NativeGetMultiVibratorInfo(JNIEnv* env, jobject thiz, jobject vibratorInfoList);
    static int32_t Vibrate(const VibratorIdentifierTrans &identifier, int32_t timeOut, int32_t usage, bool systemUsage);
    static int32_t PlayPrimitiveEffect(const VibratorIdentifierTrans &identifier, const std::string &effect,
                                       const PrimitiveEffectTrans &primitiveEffect);
    static int32_t StopVibrator(const VibratorIdentifierTrans &identifier);
    static int32_t StopVibratorByMode(const VibratorIdentifierTrans &identifier, const std::string &mode);
    static int32_t IsSupportEffect(const VibratorIdentifierTrans &identifier, const std::string &effect, bool &state);
    static int32_t PlayPattern(const VibratorIdentifierTrans &identifier, const VibratePatternTrans &pattern,
                               const CustomHapticInfoTrans &customHapticInfo);
    static int32_t GetVibratorList(const VibratorIdentifierTrans &identifier,
        std::vector<VibratorInfoTrans> &vibratorInfo);
    static int32_t GetEffectInfo(const VibratorIdentifierTrans &identifier, const std::string &effectType,
                                 EffectInfoTrans &effectInfo);
    static int32_t GetVibratorCapacity(const VibratorIdentifierTrans &identifier, VibratorCapacityTrans &capacity);
    static int32_t ClampAmplitude(int32_t intensity);
    static int32_t CalculateRepeatFromFrequency(int32_t frequency);
    static void ConvertToWaveformParams(const VibratePatternTrans &pattern, std::vector<int64_t> &timings,
                                        std::vector<int32_t> &amplitudes, int32_t &repeat);
    static void AddTimeAmplitudePairs(int32_t totalDuration, int32_t amplitude, int32_t arraySize,
                                      std::vector<std::pair<int64_t, int32_t>> &timeAmplitudePairs);
    static EffectInfoTrans m_effectInfo;
    static std::vector<VibratorInfoTrans> m_vibratorInfos;
    static std::mutex m_effectInfoMutex;
    static std::mutex m_vibratorInfosMutex;
};
} // namespace OHOS::Plugin
#endif // PLUGINS_VIBRATOR_PLUGIN_JAVA_JNI_VIBRATOR_PLUGIN_JNI_H
