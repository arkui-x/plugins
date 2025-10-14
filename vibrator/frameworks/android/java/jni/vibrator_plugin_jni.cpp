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
#include <algorithm>
#include <cmath>
#include <jni.h>
#include <string>
#include "plugins/vibrator/frameworks/android/java/jni/vibrator_plugin_jni.h"
#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"

namespace OHOS::Sensors {
namespace {
const char VIBRATOR_PLUGIN_CLASS_NAME[] = "ohos/ace/plugin/vibratorplugin/VibratorPlugin";
static const JNINativeMethod METHODS[] = {
    {
        .name = "nativeInit",
        .signature = "()V",
        .fnPtr = reinterpret_cast<void*>(VibratorJni::NativeInit)
    },
    {
        .name = "nativeGetEffectInfo",
        .signature = "(IZ)V",
        .fnPtr = reinterpret_cast<void*>(VibratorJni::NativeGetEffectInfo)
    },
    {
        .name = "nativeGetMultiVibratorInfo",
        .signature = "(Ljava/util/List;)V",
        .fnPtr = reinterpret_cast<void*>(VibratorJni::NativeGetMultiVibratorInfo)
    }
};

const char METHOD_VIBRATE[] = "vibrate";
const char METHOD_PLAY_VIBRATOR_EFFECT[] = "playVibratorEffect";
const char METHOD_STOP_VIBRATOR[] = "stopVibrator";
const char METHOD_STOP_VIBRATOR_BY_MODE[] = "stopVibratorByMode";
const char METHOD_IS_SUPPORT_EFFECT[] = "isSupportEffect";
const char METHOD_PLAY_PATTERN[] = "playPattern";
const char METHOD_GET_VIBRATOR_LIST[] = "getVibratorList";
const char METHOD_GET_EFFECT_INFO[] = "getEffectInfo";
const char METHOD_GET_VIBRATOR_CAPACITY[] = "getVibratorCapacity";

const char SIGNATURE_VIBRATE[] = "(JI)I";
const char SIGNATURE_PLAY_VIBRATOR_EFFECT[] = "(Ljava/lang/String;II)I";
const char SIGNATURE_STOP_VIBRATOR[] = "(I)I";
const char SIGNATURE_STOP_VIBRATOR_BY_MODE[] = "(Ljava/lang/String;)I";
const char SIGNATURE_IS_SUPPORT_EFFECT[] = "(Ljava/lang/String;[Z)I";
const char SIGNATURE_PLAY_PATTERN[] = "([J[III)I";
const char SIGNATURE_GET_VIBRATOR_LIST[] = "()I";
const char SIGNATURE_GET_EFFECT_INFO[] = "(Ljava/lang/String;)I";
const char SIGNATURE_GET_VIBRATOR_CAPACITY[] = "()Z";

struct {
    jmethodID vibrate;
    jmethodID playVibratorEffect;
    jmethodID stopVibrator;
    jmethodID stopVibratorByMode;
    jmethodID isSupportEffect;
    jmethodID playPattern;
    jmethodID getVibratorList;
    jmethodID getEffectInfo;
    jmethodID getVibratorCapacity;
    jobject globalRef;
} g_pluginClass;
} // namespace

EffectInfoTrans VibratorJni::m_effectInfo{};
std::vector<VibratorInfoTrans> VibratorJni::m_vibratorInfos{};
std::mutex VibratorJni::m_effectInfoMutex{};
std::mutex VibratorJni::m_vibratorInfosMutex{};

bool VibratorJni::Register(void* env)
{
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(VIBRATOR_PLUGIN_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("VibratorJni: RegisterNatives fail.");
        return false;
    }
    return true;
}

void VibratorJni::NativeInit(JNIEnv* env, jobject jobj)
{
    CHECK_NULL_VOID(env);
    g_pluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);
    g_pluginClass.vibrate = env->GetMethodID(cls, METHOD_VIBRATE, SIGNATURE_VIBRATE);
    CHECK_NULL_VOID_DELREF(g_pluginClass.vibrate, env, cls);
    g_pluginClass.playVibratorEffect = env->GetMethodID(cls, METHOD_PLAY_VIBRATOR_EFFECT,
                                                        SIGNATURE_PLAY_VIBRATOR_EFFECT);
    CHECK_NULL_VOID_DELREF(g_pluginClass.playVibratorEffect, env, cls);
    g_pluginClass.stopVibrator = env->GetMethodID(cls, METHOD_STOP_VIBRATOR, SIGNATURE_STOP_VIBRATOR);
    CHECK_NULL_VOID_DELREF(g_pluginClass.stopVibrator, env, cls);
    g_pluginClass.stopVibratorByMode = env->GetMethodID(cls, METHOD_STOP_VIBRATOR_BY_MODE,
                                                        SIGNATURE_STOP_VIBRATOR_BY_MODE);
    CHECK_NULL_VOID_DELREF(g_pluginClass.stopVibratorByMode, env, cls);
    g_pluginClass.isSupportEffect = env->GetMethodID(cls, METHOD_IS_SUPPORT_EFFECT,
                                                     SIGNATURE_IS_SUPPORT_EFFECT);
    CHECK_NULL_VOID_DELREF(g_pluginClass.isSupportEffect, env, cls);
    g_pluginClass.playPattern = env->GetMethodID(cls, METHOD_PLAY_PATTERN, SIGNATURE_PLAY_PATTERN);
    CHECK_NULL_VOID_DELREF(g_pluginClass.playPattern, env, cls);
    g_pluginClass.getVibratorList = env->GetMethodID(cls, METHOD_GET_VIBRATOR_LIST, SIGNATURE_GET_VIBRATOR_LIST);
    CHECK_NULL_VOID_DELREF(g_pluginClass.getVibratorList, env, cls);
    g_pluginClass.getEffectInfo = env->GetMethodID(cls, METHOD_GET_EFFECT_INFO, SIGNATURE_GET_EFFECT_INFO);
    CHECK_NULL_VOID_DELREF(g_pluginClass.getEffectInfo, env, cls);
    g_pluginClass.getVibratorCapacity = env->GetMethodID(cls, METHOD_GET_VIBRATOR_CAPACITY,
                                                         SIGNATURE_GET_VIBRATOR_CAPACITY);
    CHECK_NULL_VOID_DELREF(g_pluginClass.getVibratorCapacity, env, cls);
    env->DeleteLocalRef(cls);
}

void VibratorJni::NativeGetEffectInfo(JNIEnv* env, jobject thiz, jint duration, jboolean isSupportEffect)
{
    std::lock_guard<std::mutex> lock(m_effectInfoMutex);
    m_effectInfo = {};
    int32_t c_duration = static_cast<int32_t>(duration);
    bool c_isSupport = (isSupportEffect == JNI_TRUE);
    m_effectInfo.duration = c_duration;
    m_effectInfo.isSupportEffect = c_isSupport;
}

VibratorInfoTrans VibratorJni::ParseSingleVibratorInfo(JNIEnv* env, jobjectArray vibratorInfo)
{
    VibratorInfoTrans info{};
    if (vibratorInfo == nullptr || !(env)) {
        return info;
    }
    jsize arrayLen = env->GetArrayLength(vibratorInfo);
    if (arrayLen != VIBRATOR_LIST_LENGTH) {
        return info;
    }
    jobject objDeviceId = env->GetObjectArrayElement(vibratorInfo, 0);
    jint deviceId = env->CallIntMethod(objDeviceId,
        env->GetMethodID(env->GetObjectClass(objDeviceId), "intValue", "()I"));
    int32_t c_deviceId = static_cast<int32_t>(deviceId);
    env->DeleteLocalRef(objDeviceId);
    jobject objVibratorInfo = env->GetObjectArrayElement(vibratorInfo, 1);
    jint vibratorId = env->CallIntMethod(objVibratorInfo,
        env->GetMethodID(env->GetObjectClass(objVibratorInfo), "intValue", "()I"));
    int32_t c_vibratorId = static_cast<int32_t>(vibratorId);
    env->DeleteLocalRef(objVibratorInfo);
    jobject objDeviceName = env->GetObjectArrayElement(vibratorInfo, 2);
    jstring jDeviceName = static_cast<jstring>(objDeviceName);
    const char* cstr = env->GetStringUTFChars(jDeviceName, nullptr);
    std::string c_deviceName = (cstr != nullptr) ? std::string(cstr) : "";
    env->ReleaseStringUTFChars(jDeviceName, cstr);
    env->DeleteLocalRef(objDeviceName);
    jobject objIsSupportHdHaptic = env->GetObjectArrayElement(vibratorInfo, 3);
    jboolean isSupportHdHaptic = env->CallBooleanMethod(objIsSupportHdHaptic,
                                                        env->GetMethodID(env->GetObjectClass(objIsSupportHdHaptic),
                                                        "booleanValue", "()Z"));
    bool c_isSupportHdHaptic = (isSupportHdHaptic == JNI_TRUE);
    env->DeleteLocalRef(objIsSupportHdHaptic);
    jobject objIsLocalVibrator = env->GetObjectArrayElement(vibratorInfo, 4);
    jboolean isLocalVibrator = env->CallBooleanMethod(objIsLocalVibrator,
                                                      env->GetMethodID(env->GetObjectClass(objIsLocalVibrator),
                                                      "booleanValue", "()Z"));
    bool c_isLocalVibrator = (isLocalVibrator == JNI_TRUE);
    env->DeleteLocalRef(objIsLocalVibrator);
    jobject objPosition = env->GetObjectArrayElement(vibratorInfo, 5);
    jint position = env->CallIntMethod(objPosition,
        env->GetMethodID(env->GetObjectClass(objPosition), "intValue", "()I"));
    int32_t c_position = static_cast<int32_t>(position);
    env->DeleteLocalRef(objPosition);
    info.deviceId = c_deviceId;
    info.deviceName = c_deviceName;
    info.isLocalVibrator = c_isLocalVibrator;
    info.isSupportHdHaptic = c_isSupportHdHaptic;
    info.position = c_position;
    info.vibratorId = c_vibratorId;
    return info;
}

void VibratorJni::NativeGetMultiVibratorInfo(JNIEnv* env, jobject thiz, jobject vibratorInfoList)
{
    std::lock_guard<std::mutex> lock(m_vibratorInfosMutex);
    if (!(env)) {
        return;
    }
    m_vibratorInfos.clear();
    if (vibratorInfoList == nullptr) {
        return;
    }
    jclass listClass = env->FindClass("java/util/List");
    if (listClass == nullptr) {
        env->ThrowNew(env->FindClass("java/lang/ClassNotFoundException"), "List class not found");
    }
    jmethodID sizeMethod = env->GetMethodID(listClass, "size", "()I");
    jmethodID getMethod = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");
    jmethodID toArrayMethod = env->GetMethodID(listClass, "toArray", "()[Ljava/lang/Object;");
    if (sizeMethod == nullptr || getMethod == nullptr || toArrayMethod == nullptr) {
        env->ThrowNew(env->FindClass("java/lang/NoSuchMethodException"), "List methods not found");
        env->DeleteLocalRef(listClass);
    }
    jint outerSize = env->CallIntMethod(vibratorInfoList, sizeMethod);
    jclass objectClass = env->FindClass("java/lang/Object");
    jobjectArray outerArray = env->NewObjectArray(outerSize, objectClass, nullptr);
    env->DeleteLocalRef(objectClass);
    if (outerArray == nullptr) {
        env->DeleteLocalRef(listClass);
    }
    for (jint i = 0; i < outerSize; i++) {
        jobject innerList = env->CallObjectMethod(vibratorInfoList, getMethod, i);
        if (innerList == nullptr) {
            jobjectArray emptyArray = env->NewObjectArray(0, env->FindClass("java/lang/Object"), nullptr);
            env->SetObjectArrayElement(outerArray, i, emptyArray);
            env->DeleteLocalRef(emptyArray);
            continue;
        }

        jobjectArray innerArray = static_cast<jobjectArray>(env->CallObjectMethod(innerList, toArrayMethod));
        env->SetObjectArrayElement(outerArray, i, innerArray);

        env->DeleteLocalRef(innerList);
        env->DeleteLocalRef(innerArray);
    }
    env->DeleteLocalRef(listClass);
    jsize groupCount = env->GetArrayLength(outerArray);
    for (jsize i = 0; i < groupCount; i++) {
        jobjectArray singleGroup = static_cast<jobjectArray>(env->GetObjectArrayElement(outerArray, i));
        VibratorInfoTrans info = ParseSingleVibratorInfo(env, singleGroup);
        m_vibratorInfos.push_back(info);
        env->DeleteLocalRef(singleGroup);
    }
    env->DeleteLocalRef(outerArray);
}

int32_t VibratorJni::Vibrate(const VibratorIdentifierTrans &identifier, int32_t timeOut, int32_t usage,
    bool systemUsage)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.vibrate)) {
        LOGE("VibratorJni get none ptr error");
        return VIBRATE_FAILED;
    }
    jint ret = env->CallIntMethod(g_pluginClass.globalRef, g_pluginClass.vibrate, timeOut, usage);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return VIBRATE_FAILED;
    }
    return static_cast<int32_t>(ret);
}

int32_t VibratorJni::PlayPrimitiveEffect(const VibratorIdentifierTrans& identifier, const std::string &effect,
                                         const PrimitiveEffectTrans& primitiveEffect)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.playVibratorEffect)) {
        LOGE("VibratorJni get none ptr error");
        return VIBRATE_FAILED;
    }
    jstring jEffect = env->NewStringUTF(effect.c_str());
    if (jEffect == nullptr) {
        return VIBRATE_FAILED;
    }
    jint ret = env->CallIntMethod(g_pluginClass.globalRef, g_pluginClass.playVibratorEffect,
                                  jEffect, primitiveEffect.count, primitiveEffect.usage);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteLocalRef(jEffect);
        return VIBRATE_FAILED;
    }
    env->DeleteLocalRef(jEffect);
    return static_cast<int32_t>(ret);
}

int32_t VibratorJni::StopVibrator(const VibratorIdentifierTrans &identifier)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.stopVibrator)) {
        LOGE("VibratorJni get none ptr error");
        return VIBRATE_FAILED;
    }
    int32_t vibratorId = identifier.vibratorId;
    jint ret = env->CallIntMethod(g_pluginClass.globalRef, g_pluginClass.stopVibrator, vibratorId);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return VIBRATE_FAILED;
    }

    return static_cast<int32_t>(ret);
}

int32_t VibratorJni::StopVibratorByMode(const VibratorIdentifierTrans &identifier, const std::string &mode)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.stopVibratorByMode)) {
        LOGE("VibratorJni get none ptr error");
        return VIBRATE_FAILED;
    }
    jstring jMode = env->NewStringUTF(mode.c_str());
    if (jMode == nullptr) {
        return VIBRATE_FAILED;
    }
    jint ret = env->CallIntMethod(g_pluginClass.globalRef, g_pluginClass.stopVibratorByMode, jMode);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteLocalRef(jMode);
        return VIBRATE_FAILED;
    }
    env->DeleteLocalRef(jMode);
    return static_cast<int32_t>(ret);
}

int32_t VibratorJni::IsSupportEffect(const VibratorIdentifierTrans &identifier, const std::string &effect, bool &state)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.isSupportEffect)) {
        LOGE("VibratorJni get none ptr error");
        return VIBRATE_FAILED;
    }
    jstring jEffect = env->NewStringUTF(effect.c_str());
    if (jEffect == nullptr) {
        LOGE("Failed to create effect string");
        return VIBRATE_FAILED;
    }
    jbooleanArray jStateArray = env->NewBooleanArray(1);
    if (jStateArray == nullptr) {
        LOGE("Failed to create boolean array for state");
        env->DeleteLocalRef(jEffect);
        return VIBRATE_FAILED;
    }
    jint ret = env->CallIntMethod(g_pluginClass.globalRef, g_pluginClass.isSupportEffect,
                                  jEffect, jStateArray);
    jboolean jState;
    env->GetBooleanArrayRegion(jStateArray, 0, 1, &jState);
    state = (jState == JNI_TRUE);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteLocalRef(jStateArray);
        env->DeleteLocalRef(jEffect);
        return VIBRATE_FAILED;
    }
    env->DeleteLocalRef(jStateArray);
    env->DeleteLocalRef(jEffect);
    return static_cast<int32_t>(ret);
}

int32_t VibratorJni::PlayPattern(const VibratorIdentifierTrans &identifier, const VibratePatternTrans &pattern,
                                 const CustomHapticInfoTrans &customHapticInfo)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.playPattern)) {
        LOGE("VibratorJni get none ptr error");
        return VIBRATE_FAILED;
    }
    std::vector<int64_t> timings;
    std::vector<int32_t> amplitudes;
    int32_t repeat = -1;
    int32_t usage = customHapticInfo.usage;
    ConvertToWaveformParams(pattern, timings, amplitudes, repeat);
    repeat = -1;
    if (timings.empty() || amplitudes.empty() || timings.size() != amplitudes.size()) {
        return VIBRATE_FAILED;
    }
    jlongArray jTimings = env->NewLongArray(static_cast<jsize>(timings.size()));
    if (jTimings != nullptr) {
        env->SetLongArrayRegion(jTimings, 0, static_cast<jsize>(timings.size()),
                                reinterpret_cast<const jlong*>(timings.data()));
    }
    jintArray jAmplitudes = env->NewIntArray(static_cast<jsize>(amplitudes.size()));
    if (jAmplitudes != nullptr) {
        env->SetIntArrayRegion(jAmplitudes, 0, static_cast<jsize>(amplitudes.size()),
                               amplitudes.data());
    }
    jint ret = env->CallIntMethod(g_pluginClass.globalRef, g_pluginClass.playPattern,
                                  jTimings, jAmplitudes, repeat, usage);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteLocalRef(jTimings);
        env->DeleteLocalRef(jAmplitudes);
        return VIBRATE_FAILED;
    }
    env->DeleteLocalRef(jTimings);
    env->DeleteLocalRef(jAmplitudes);
    return static_cast<int32_t>(ret);
}

int32_t VibratorJni::GetVibratorList(const VibratorIdentifierTrans &identifier,
                                     std::vector<VibratorInfoTrans> &vibratorInfo)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getVibratorList)) {
        LOGE("VibratorJni get none ptr error");
        return VIBRATE_FAILED;
    }
    jint ret = env->CallIntMethod(g_pluginClass.globalRef, g_pluginClass.getVibratorList);
    std::lock_guard<std::mutex> lock(m_vibratorInfosMutex);
    vibratorInfo = m_vibratorInfos;
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return VIBRATE_FAILED;
    }
    return static_cast<int32_t>(ret);
}

int32_t VibratorJni::GetEffectInfo(const VibratorIdentifierTrans &identifier, const std::string &effectType,
                                   EffectInfoTrans &effectInfo)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getEffectInfo)) {
        LOGE("VibratorJni get none ptr error");
        return VIBRATE_FAILED;
    }
    jstring jEffect = env->NewStringUTF(effectType.c_str());
    if (jEffect == nullptr) {
        LOGE("Failed to create effectType string");
        return VIBRATE_FAILED;
    }
    jint ret = env->CallIntMethod(g_pluginClass.globalRef, g_pluginClass.getEffectInfo, jEffect);
    std::lock_guard<std::mutex> lock(m_effectInfoMutex);
    effectInfo = m_effectInfo;
    if (env->ExceptionCheck()) {
        LOGE("VibratorJni: call GetEffectInfo failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteLocalRef(jEffect);
        return VIBRATE_FAILED;
    }
    env->DeleteLocalRef(jEffect);
    return ret;
}

int32_t VibratorJni::GetVibratorCapacity(const VibratorIdentifierTrans &identifier, VibratorCapacityTrans &capacity)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getVibratorCapacity)) {
        LOGE("VibratorJni get none ptr error");
        return VIBRATE_FAILED;
    }
    jboolean ret = env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.getVibratorCapacity);
    capacity.isSupportHdHaptic = (ret == JNI_TRUE);
    capacity.isSupportPresetMapping = true;
    capacity.isSupportTimeDelay = true;
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return VIBRATE_FAILED;
    }
    return VIBRATE_SUCCESS;
}

int32_t VibratorJni::ClampAmplitude(int32_t intensity)
{
    int32_t realIntensity = intensity * JNI_INTENSITY_MAX / OH_INTENSITY_MAX;
    if (realIntensity < 0) {
        return VIBRATE_FAILED;
    } else if (realIntensity > JNI_INTENSITY_MAX) {
        return JNI_INTENSITY_MAX;
    }
    return realIntensity;
}

void VibratorJni::ConvertToWaveformParams(const VibratePatternTrans &pattern, std::vector<int64_t> &timings,
                                          std::vector<int32_t> &amplitudes, int32_t &repeat)
{
    timings.clear();
    amplitudes.clear();
    repeat = -1;
    if (pattern.events.empty()) {
        return;
    }
    std::vector<std::pair<int64_t, int32_t>> timeAmplitudePairs;
    timeAmplitudePairs.emplace_back(pattern.startTime, 0);
    for (const auto& event : pattern.events) {
        int32_t baseFrequency = event.frequency;
        if (!event.points.empty()) {
            for (int32_t i = 0; i < event.points.size() - 1; i++) {
                int32_t partAllDuration = event.points[i + 1].time - event.points[i].time;
                int32_t partAllAmplitude = ClampAmplitude(event.points[i].intensity);
                int32_t transFrequency = event.frequency + event.points[i].frequency;
                int32_t arraySize = transFrequency * partAllDuration * VIBRATOR_MIN_UNIT / VIBRATOR_DEFAULT_DURATION;
                arraySize = 0 ? 1 : arraySize;
                AddTimeAmplitudePairs(partAllDuration, partAllAmplitude, arraySize, timeAmplitudePairs);
            }
        } else {
            int32_t arraySize = event.frequency * event.duration * VIBRATOR_MIN_UNIT / VIBRATOR_DEFAULT_DURATION;
            arraySize = 0 ? 1 : arraySize;
            for (int32_t i = 0; i < arraySize; i += VIBRATOR_MIN_UNIT) {
                int32_t partDuration = event.duration / arraySize;
                timeAmplitudePairs.emplace_back(partDuration, event.intensity);
                timeAmplitudePairs.emplace_back(partDuration, 0);
            }
        }
    }
    for (const auto& pair : timeAmplitudePairs) {
        timings.push_back(pair.first);
        amplitudes.push_back(pair.second);
    }
}

void VibratorJni::AddTimeAmplitudePairs(int32_t totalDuration, int32_t amplitude, int32_t arraySize,
                                        std::vector<std::pair<int64_t, int32_t>> &timeAmplitudePairs)
{
    if (arraySize == 0) {
        return;
    }
    int32_t partDuration = totalDuration / arraySize;
    for (int32_t i = 0; i < arraySize; i += VIBRATOR_MIN_UNIT) {
        timeAmplitudePairs.emplace_back(partDuration, amplitude);
        timeAmplitudePairs.emplace_back(partDuration, 0);
    }
}

} // namespace OHOS::Plugin
