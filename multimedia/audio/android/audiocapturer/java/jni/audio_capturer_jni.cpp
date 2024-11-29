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

#include "audio_capturer_jni.h"

#include <codecvt>
#include <jni.h>
#include <locale>
#include <securec.h>

#include "audio_common_jni.h"
#include "audio_convert_util.h"
#include "audio_errors.h"
#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"

using namespace OHOS::AudioStandard;

namespace OHOS::Plugin {
namespace {
const char CLASS_NAME[] = "ohos/ace/plugin/audiocapturer/AudioCapturerPlugin";
const char AUDIOINFO_CLASS[] = "ohos/ace/plugin/audiocapturer/AudioCapturerPlugin/AudioCapturerInfo";

static const JNINativeMethod METHODS[] = { { "nativeInit", "()V",
                                               reinterpret_cast<void*>(AudioCapturerJni::NativeInit) },
    { "nativeOnMarkerReached", "(JI)V", reinterpret_cast<void*>(AudioCapturerJni::NativeOnMarkerReached) },
    { "nativeOnPeriodicNotification", "(JI)V",
        reinterpret_cast<void*>(AudioCapturerJni::NativeOnPeriodicNotification) },
    { "nativeOnRoutingChanged", "(JLandroid/media/AudioDeviceInfo;)V",
        reinterpret_cast<void*>(AudioCapturerJni::NativeOnRoutingChanged) },
    { "nativeOnInfoChanged", "(JLandroid/media/AudioDeviceInfo;)V",
        reinterpret_cast<void*>(AudioCapturerJni::NativeOnInfoChanged) } };

struct {
    jmethodID createAudioRecord;
    jmethodID startRecording;
    jmethodID stop;
    jmethodID release;
    jmethodID getMinBufferSize;
    jmethodID read;
    jmethodID getAudioSource;
    jmethodID setNotificationMarkerPosition;
    jmethodID setRecordPositionUpdateListener;
    jmethodID unsetRecordPositionUpdateListener;
    jmethodID setPositionNotificationPeriod;
    jmethodID addOnRoutingChangedListener;
    jmethodID removeOnRoutingChangedListener;
    jmethodID addOnInfoChangedListener;
    jmethodID removeOnInfoChangedListener;
    jmethodID getFormat;
    jmethodID getAudioSessionId;
    jmethodID getTimestamp;
    jmethodID getRoutedDevice;
    jmethodID finalize;
    jobject globalRef;
} g_audioCapturerPluginClass;
} // namespace

bool AudioCapturerJni::hasInit_ = false;
std::map<long, std::shared_ptr<CapturerPositionCallback>> AudioCapturerJni::capturerPositionObserver_;
std::map<long, std::shared_ptr<CapturerPeriodPositionCallback>> AudioCapturerJni::periodPositionObserver_;
std::map<long, std::list<std::shared_ptr<AudioCapturerDeviceChangeCallback>>> AudioCapturerJni::deviceChangeObserver_;
std::map<long, std::list<std::shared_ptr<AudioStandard::AudioCapturerInfoChangeCallback>>>
    AudioCapturerJni::capturerInfoChangeObserver_;
std::map<long, AudioStandard::CapturerState> AudioCapturerJni::capturerStateMap_;

bool AudioCapturerJni::Register(void* env)
{
    LOGD("AudioCapturerJni::Register");
    if (hasInit_) {
        return hasInit_;
    }
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    if (!ret) {
        jniEnv->DeleteLocalRef(cls);
        LOGE("AudioCapturerJni JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void AudioCapturerJni::NativeInit(JNIEnv* env, jobject jobj)
{
    LOGD("AudioCapturerJni::NativeInit");
    CHECK_NULL_VOID(env);
    g_audioCapturerPluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_audioCapturerPluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_audioCapturerPluginClass.createAudioRecord = env->GetMethodID(cls, "createAudioRecord", "(JIIII)I");
    CHECK_NULL_VOID(g_audioCapturerPluginClass.createAudioRecord);

    g_audioCapturerPluginClass.startRecording = env->GetMethodID(cls, "startRecording", "(J)V");
    CHECK_NULL_VOID(g_audioCapturerPluginClass.startRecording);

    g_audioCapturerPluginClass.stop = env->GetMethodID(cls, "stop", "(J)V");
    CHECK_NULL_VOID(g_audioCapturerPluginClass.stop);

    g_audioCapturerPluginClass.release = env->GetMethodID(cls, "release", "(J)V");
    CHECK_NULL_VOID(g_audioCapturerPluginClass.release);

    g_audioCapturerPluginClass.getMinBufferSize = env->GetMethodID(cls, "getMinBufferSize", "(JIII)I");
    CHECK_NULL_VOID(g_audioCapturerPluginClass.getMinBufferSize);

    g_audioCapturerPluginClass.read = env->GetMethodID(cls, "read", "(J[BI)I");
    CHECK_NULL_VOID(g_audioCapturerPluginClass.read);

    g_audioCapturerPluginClass.getAudioSource = env->GetMethodID(cls, "getAudioSource", "(J)I");
    CHECK_NULL_VOID(g_audioCapturerPluginClass.getAudioSource);

    g_audioCapturerPluginClass.setNotificationMarkerPosition =
        env->GetMethodID(cls, "setNotificationMarkerPosition", "(JI)I");
    CHECK_NULL_VOID(g_audioCapturerPluginClass.setNotificationMarkerPosition);

    g_audioCapturerPluginClass.setRecordPositionUpdateListener =
        env->GetMethodID(cls, "setRecordPositionUpdateListener", "(J)V");
    CHECK_NULL_VOID(g_audioCapturerPluginClass.setRecordPositionUpdateListener);

    g_audioCapturerPluginClass.unsetRecordPositionUpdateListener =
        env->GetMethodID(cls, "unsetRecordPositionUpdateListener", "(J)V");
    CHECK_NULL_VOID(g_audioCapturerPluginClass.unsetRecordPositionUpdateListener);

    g_audioCapturerPluginClass.setPositionNotificationPeriod =
        env->GetMethodID(cls, "setPositionNotificationPeriod", "(JI)I");
    CHECK_NULL_VOID(g_audioCapturerPluginClass.setPositionNotificationPeriod);

    g_audioCapturerPluginClass.addOnRoutingChangedListener =
        env->GetMethodID(cls, "addOnRoutingChangedListener", "(J)V");
    CHECK_NULL_VOID(g_audioCapturerPluginClass.addOnRoutingChangedListener);

    g_audioCapturerPluginClass.removeOnRoutingChangedListener =
        env->GetMethodID(cls, "removeOnRoutingChangedListener", "(J)V");
    CHECK_NULL_VOID(g_audioCapturerPluginClass.removeOnRoutingChangedListener);

    g_audioCapturerPluginClass.addOnInfoChangedListener = env->GetMethodID(cls, "addOnInfoChangedListener", "(J)V");
    CHECK_NULL_VOID(g_audioCapturerPluginClass.addOnInfoChangedListener);

    g_audioCapturerPluginClass.removeOnInfoChangedListener =
        env->GetMethodID(cls, "removeOnInfoChangedListener", "(J)V");
    CHECK_NULL_VOID(g_audioCapturerPluginClass.removeOnInfoChangedListener);

    g_audioCapturerPluginClass.getFormat = env->GetMethodID(cls, "getFormat", "(J)Landroid/media/AudioFormat;");
    CHECK_NULL_VOID(g_audioCapturerPluginClass.getFormat);

    g_audioCapturerPluginClass.getAudioSessionId = env->GetMethodID(cls, "getAudioSessionId", "(J)I");
    CHECK_NULL_VOID(g_audioCapturerPluginClass.getAudioSessionId);

    g_audioCapturerPluginClass.getTimestamp =
        env->GetMethodID(cls, "getTimestamp", "(JLandroid/media/AudioTimestamp;)I");
    CHECK_NULL_VOID(g_audioCapturerPluginClass.getTimestamp);

    g_audioCapturerPluginClass.getRoutedDevice =
        env->GetMethodID(cls, "getRoutedDevice", "(J)Landroid/media/AudioDeviceInfo;");
    CHECK_NULL_VOID(g_audioCapturerPluginClass.getRoutedDevice);

    g_audioCapturerPluginClass.finalize = env->GetMethodID(cls, "finalize", "(J)V");
    CHECK_NULL_VOID(g_audioCapturerPluginClass.finalize);

    env->DeleteLocalRef(cls);
    hasInit_ = true;
}

void AudioCapturerJni::NativeOnMarkerReached(JNIEnv* env, jobject jobj, jlong capturerPtr, jint position)
{
    LOGD("AudioCapturerJni::NativeOnMarkerReached, key:%ld, position:%d", capturerPtr, position);
    long key = static_cast<long>(capturerPtr);
    auto iter = capturerPositionObserver_.find(key);
    if (iter != capturerPositionObserver_.end()) {
        (iter->second)->OnMarkReached(static_cast<int64_t>(position));
    }
}

void AudioCapturerJni::NativeOnPeriodicNotification(JNIEnv* env, jobject jobj, jlong capturerPtr, jint period)
{
    LOGD("AudioCapturerJni::NativeOnPeriodicNotification, key:%ld, position:%d", capturerPtr, period);
    long key = static_cast<long>(capturerPtr);
    auto iter = periodPositionObserver_.find(key);
    if (iter != periodPositionObserver_.end()) {
        (iter->second)->OnPeriodReached(static_cast<int64_t>(period));
    }
}

void AudioCapturerJni::NativeOnRoutingChanged(JNIEnv* env, jobject jobj, jlong capturerPtr, jobject jDeviceInfo)
{
    LOGD("AudioCapturerJni::NativeOnRoutingChanged, key:%ld", capturerPtr);
    long key = static_cast<long>(capturerPtr);
    auto iter = deviceChangeObserver_.find(key);
    if (iter != deviceChangeObserver_.end()) {
        AudioDeviceDescriptor deviceInfo = AudioCommonJni::GetDeviceInfo(jDeviceInfo);
        if (deviceInfo.deviceRole_ == INPUT_DEVICE) {
            for (auto deviceChangeCallback : iter->second) {
                deviceChangeCallback->OnStateChange(deviceInfo);
            }
        }
    }
}

void AudioCapturerJni::NativeOnInfoChanged(JNIEnv* env, jobject jobj, jlong capturerPtr, jobject jDeviceInfo)
{
    LOGD("AudioCapturerJni::NativeOnInfoChanged, key:%ld", capturerPtr);
    long key = static_cast<long>(capturerPtr);
    AudioCapturerChangeInfo changeInfo;
    changeInfo.inputDeviceInfo = AudioCommonJni::GetDeviceInfo(jDeviceInfo);
    if (changeInfo.inputDeviceInfo.deviceRole_ == INPUT_DEVICE) {
        uint32_t sessionID;
        AudioCapturerInfo capturerInfo;
        int32_t ret = GetAudioSessionId(key, sessionID);
        if (ret != SUCCESS) {
            LOGE("AudioCapturerJni::NativeOnInfoChanged GetAudioSessionId fail");
            return;
        }
        ret = GetCapturerInfo(key, capturerInfo);
        if (ret != SUCCESS) {
            LOGE("AudioCapturerImpl::NativeOnInfoChanged GetCapturerInfo fail");
            return;
        }
        changeInfo.sessionId = sessionID;
        changeInfo.capturerInfo = capturerInfo;
        auto iter = capturerStateMap_.find(key);
        if (iter != capturerStateMap_.end()) {
            changeInfo.capturerState = iter->second;
        }
        LOGD("AudioCapturerJni::NativeOnInfoChanged, call NotifyCapturerInfoChanged");
        NotifyCapturerInfoChanged(key, changeInfo);
    }
}

int32_t AudioCapturerJni::CreateAudioRecord(long capturerPtr, const AudioCapturerOptions& capturerOptions)
{
    LOGD("AudioCapturerJni::CreateAudioRecord");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.createAudioRecord, ERROR);

    LOGD("AudioCapturerJni createAudioRecord before convert: sampleRate:%d,channel:%d format:%d sourceType:%d",
        capturerOptions.streamInfo.samplingRate, capturerOptions.streamInfo.channels, capturerOptions.streamInfo.format,
        capturerOptions.capturerInfo.sourceType);
    jint sampleRate = capturerOptions.streamInfo.samplingRate;
    jint channel = (1 << capturerOptions.streamInfo.channels) - 1;
    jint encoding = ConvertAudioSampleFormatToAn(capturerOptions.streamInfo.format);
    jint sourceType = ConvertSourceTypeToAn(capturerOptions.capturerInfo.sourceType);
    LOGD("AudioCapturerJni createAudioRecord after convert: sampleRate:%d,channel:%d encoding:%d sourceType:%d",
        sampleRate, channel, encoding, sourceType);
    jint jRet = env->CallIntMethod(g_audioCapturerPluginClass.globalRef, g_audioCapturerPluginClass.createAudioRecord,
        capturerPtr, sampleRate, channel, encoding, sourceType);
    if (env->ExceptionCheck() || jRet != SUCCESS) {
        LOGE("AudioCapturerJni JNI: call CreateAudioRecord has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    return SUCCESS;
}

bool AudioCapturerJni::Start(long capturerPtr)
{
    LOGD("AudioCapturerJni::startRecord");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.globalRef, false);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.startRecording, false);

    env->CallVoidMethod(g_audioCapturerPluginClass.globalRef, g_audioCapturerPluginClass.startRecording, capturerPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioCapturerJni JNI: call startRecord has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return true;
}

bool AudioCapturerJni::Stop(long capturerPtr)
{
    LOGD("AudioCapturerJni::Stop");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.globalRef, false);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.stop, false);

    env->CallVoidMethod(g_audioCapturerPluginClass.globalRef, g_audioCapturerPluginClass.stop, capturerPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioCapturerJni JNI: call stop has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return true;
}

bool AudioCapturerJni::Release(long capturerPtr)
{
    LOGD("AudioCapturerJni::Release");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.globalRef, false);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.release, false);

    env->CallVoidMethod(g_audioCapturerPluginClass.globalRef, g_audioCapturerPluginClass.release, capturerPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioCapturerJni JNI: call release has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return true;
}

int32_t AudioCapturerJni::GetBufferSize(long capturerPtr, const AudioStreamInfo& streamInfo, size_t& bufferSize)
{
    LOGD("AudioCapturerJni::GetBufferSize capturerPtr:%ld, samplingRate:%d, channelLayout:%d, format:%d", capturerPtr,
        streamInfo.samplingRate, streamInfo.channelLayout, streamInfo.format);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.getMinBufferSize, ERROR);

    LOGD("AudioCapturerJni::GetBufferSize before convert: samplingRate:%d, channelLayout:%d, format:%d",
        streamInfo.samplingRate, streamInfo.channelLayout, streamInfo.format);
    jint sampleRate = streamInfo.samplingRate;
    jint format = ConvertAudioSampleFormatToAn(streamInfo.format);
    jint channelLayout = ConvertCapturerAudioChannelLayoutToAn(streamInfo.channelLayout);
    LOGD("AudioCapturerJni::GetBufferSize after convert: samplingRate:%d, channelLayout:%d, format:%d", sampleRate,
        channelLayout, format);

    jint size = env->CallIntMethod(g_audioCapturerPluginClass.globalRef, g_audioCapturerPluginClass.getMinBufferSize,
        capturerPtr, sampleRate, channelLayout, format);
    if (env->ExceptionCheck()) {
        LOGE("AudioCapturerJni JNI: call getMinBufferSize has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    LOGD("AudioCapturerJni::GetBufferSize bufferSize:%zu", size);
    if (size > 0) {
        bufferSize = static_cast<size_t>(size);
        return SUCCESS;
    }
    return ERROR;
}

int32_t AudioCapturerJni::Read(long capturerPtr, uint8_t* buffer, size_t bufferSize)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, 0);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.globalRef, 0);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.read, 0);

    jbyteArray jBufferArray = env->NewByteArray(bufferSize);

    jint size = env->CallIntMethod(
        g_audioCapturerPluginClass.globalRef, g_audioCapturerPluginClass.read, capturerPtr, jBufferArray, bufferSize);
    if (env->ExceptionCheck() || (size < 0)) {
        LOGE("AudioCapturerJni JNI: call read has exception");
        env->DeleteLocalRef(jBufferArray);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return 0;
    }
    jbyte* jBuffer = env->GetByteArrayElements(jBufferArray, NULL);
    if (memcpy_s(buffer, bufferSize * sizeof(uint8_t), jBuffer, bufferSize * sizeof(jbyte)) != EOK) {
        LOGE("[AudioCapturerJni::Read] memory copy failed");
        env->ReleaseByteArrayElements(jBufferArray, jBuffer, 0);
        env->DeleteLocalRef(jBufferArray);
        return 0;
    }
    env->ReleaseByteArrayElements(jBufferArray, jBuffer, 0);
    env->DeleteLocalRef(jBufferArray);
    return static_cast<int32_t>(size);
}

int32_t AudioCapturerJni::GetCapturerInfo(long capturerPtr, AudioCapturerInfo& capturerInfo)
{
    LOGD("AudioCapturerJni::GetCapturerInfo capturerPtr:%ld", capturerPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.getAudioSource, ERROR);

    LOGD("AudioCapturerJni JNI: call getAudioSource");
    jint jSourceType = env->CallIntMethod(
        g_audioCapturerPluginClass.globalRef, g_audioCapturerPluginClass.getAudioSource, capturerPtr);
    if (env->ExceptionCheck() || (jSourceType < 0)) {
        LOGE("AudioCapturerJni JNI: call getAudioSource has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }

    LOGD("AudioCapturerJni::GetCapturerInfo before convert: jSourceType:%d", jSourceType);
    capturerInfo.sourceType = ConvertSourceTypeToOh(static_cast<AudioSourceType>(jSourceType));
    capturerInfo.capturerFlags = 0;
    LOGD("AudioCapturerJni::GetCapturerInfo after convert: capturerInfo.sourceType:%d", capturerInfo.sourceType);

    return SUCCESS;
}

int32_t AudioCapturerJni::SetNotificationMarkerPosition(long capturerPtr, int64_t markPosition)
{
    LOGD("AudioCapturerJni::SetNotificationMarkerPosition capturerPtr:%ld, markPosition:%d", capturerPtr, markPosition);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.setNotificationMarkerPosition, ERROR);

    jint result = env->CallIntMethod(g_audioCapturerPluginClass.globalRef,
        g_audioCapturerPluginClass.setNotificationMarkerPosition, capturerPtr, markPosition);
    if (env->ExceptionCheck()) {
        LOGE("AudioCapturerJni JNI: call setNotificationMarkerPosition has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    LOGD("AudioCapturerJni::SetNotificationMarkerPosition result:%d", result);
    return result != SUCCESS ? ERR_OPERATION_FAILED : SUCCESS;
}

int32_t AudioCapturerJni::SetCapturerPositionUpdateListener(
    long capturerPtr, const std::shared_ptr<CapturerPositionCallback>& callback)
{
    LOGD("AudioCapturerJni::SetCapturerPositionUpdateListener capturerPtr:%ld", capturerPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.setRecordPositionUpdateListener, ERROR);

    capturerPositionObserver_[capturerPtr] = callback;
    env->CallVoidMethod(
        g_audioCapturerPluginClass.globalRef, g_audioCapturerPluginClass.setRecordPositionUpdateListener, capturerPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioCapturerJni JNI: call setRecordPositionUpdateListener has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    return SUCCESS;
}

void AudioCapturerJni::UnsetRecordPositionUpdateListener(long capturerPtr)
{
    auto periodIter = periodPositionObserver_.find(capturerPtr);
    auto markerIter = capturerPositionObserver_.find(capturerPtr);
    if ((periodIter != periodPositionObserver_.end()) || (markerIter != capturerPositionObserver_.end())) {
        LOGD("There are still listeners present, will not unregister.");
        return;
    }

    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_audioCapturerPluginClass.globalRef);
    CHECK_NULL_VOID(g_audioCapturerPluginClass.unsetRecordPositionUpdateListener);

    env->CallVoidMethod(g_audioCapturerPluginClass.globalRef,
        g_audioCapturerPluginClass.unsetRecordPositionUpdateListener, capturerPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioCapturerJni JNI: call unsetRecordPositionUpdateListener has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void AudioCapturerJni::UnsetCapturerPositionUpdateListener(long capturerPtr)
{
    LOGD("AudioCapturerJni::UnsetCapturerPositionUpdateListener capturerPtr:%ld", capturerPtr);
    auto iter = capturerPositionObserver_.find(capturerPtr);
    if (iter != capturerPositionObserver_.end()) {
        capturerPositionObserver_.erase(iter);
    }
    UnsetRecordPositionUpdateListener(capturerPtr);
}

int32_t AudioCapturerJni::SetPositionNotificationPeriod(long capturerPtr, int64_t frameNumber)
{
    LOGD("AudioCapturerJni::SetPositionNotificationPeriod capturerPtr:%ld, frameNumber:%d", capturerPtr, frameNumber);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.setPositionNotificationPeriod, ERROR);

    jint result = env->CallIntMethod(g_audioCapturerPluginClass.globalRef,
        g_audioCapturerPluginClass.setPositionNotificationPeriod, capturerPtr, frameNumber);
    if (env->ExceptionCheck()) {
        LOGE("AudioCapturerJni JNI: call setPositionNotificationPeriod has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    LOGD("AudioCapturerJni::setPositionNotificationPeriod result:%d", result);
    return result != SUCCESS ? ERR_OPERATION_FAILED : SUCCESS;
}

int32_t AudioCapturerJni::SetPeriodPositionUpdateListener(
    long capturerPtr, const std::shared_ptr<CapturerPeriodPositionCallback>& callback)
{
    LOGD("AudioCapturerJni::SetPeriodPositionUpdateListener capturerPtr:%ld", capturerPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.setRecordPositionUpdateListener, ERROR);

    periodPositionObserver_[capturerPtr] = callback;
    env->CallVoidMethod(
        g_audioCapturerPluginClass.globalRef, g_audioCapturerPluginClass.setRecordPositionUpdateListener, capturerPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioCapturerJni JNI: call setRecordPositionUpdateListener has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    return SUCCESS;
}

void AudioCapturerJni::UnsetPeriodPositionUpdateListener(long capturerPtr)
{
    LOGD("AudioCapturerJni::UnsetPeriodPositionUpdateListener capturerPtr:%ld", capturerPtr);
    auto iter = periodPositionObserver_.find(capturerPtr);
    if (iter != periodPositionObserver_.end()) {
        periodPositionObserver_.erase(iter);
    }
    UnsetRecordPositionUpdateListener(capturerPtr);
}

int32_t AudioCapturerJni::RegisterCapturerDeviceChangeListener(
    long capturerPtr, const std::shared_ptr<AudioCapturerDeviceChangeCallback>& callback)
{
    LOGD("AudioCapturerJni::RegisterCapturerDeviceChangeListener capturerPtr:%ld", capturerPtr);
    auto iter = deviceChangeObserver_.find(capturerPtr);
    if (iter != deviceChangeObserver_.end()) {
        (iter->second).push_back(callback);
    } else {
        std::list<std::shared_ptr<AudioCapturerDeviceChangeCallback>> newDeviceChangeCallbackList;
        newDeviceChangeCallbackList.push_back(callback);
        deviceChangeObserver_[capturerPtr] = newDeviceChangeCallbackList;
        auto env = ARKUI_X_Plugin_GetJniEnv();
        CHECK_NULL_RETURN(env, ERROR);
        CHECK_NULL_RETURN(g_audioCapturerPluginClass.globalRef, ERROR);
        CHECK_NULL_RETURN(g_audioCapturerPluginClass.addOnRoutingChangedListener, ERROR);

        env->CallVoidMethod(
            g_audioCapturerPluginClass.globalRef, g_audioCapturerPluginClass.addOnRoutingChangedListener, capturerPtr);
        if (env->ExceptionCheck()) {
            LOGE("AudioCapturerJni JNI: call addOnRoutingChangedListener has exception");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return ERROR;
        }
    }
    return SUCCESS;
}

int32_t AudioCapturerJni::SetAudioCapturerInfoChangeCallback(
    long capturerPtr, const std::shared_ptr<AudioCapturerInfoChangeCallback>& callback)
{
    LOGD("AudioCapturerJni::SetAudioCapturerInfoChangeCallback capturerPtr:%ld", capturerPtr);
    auto iter = capturerInfoChangeObserver_.find(capturerPtr);
    if (iter != capturerInfoChangeObserver_.end()) {
        (iter->second).push_back(callback);
    } else {
        std::list<std::shared_ptr<AudioCapturerInfoChangeCallback>> newInfoChangeCallbackList;
        newInfoChangeCallbackList.push_back(callback);
        capturerInfoChangeObserver_[capturerPtr] = newInfoChangeCallbackList;
        auto env = ARKUI_X_Plugin_GetJniEnv();
        CHECK_NULL_RETURN(env, ERROR);
        CHECK_NULL_RETURN(g_audioCapturerPluginClass.globalRef, ERROR);
        CHECK_NULL_RETURN(g_audioCapturerPluginClass.addOnInfoChangedListener, ERROR);

        env->CallVoidMethod(
            g_audioCapturerPluginClass.globalRef, g_audioCapturerPluginClass.addOnInfoChangedListener, capturerPtr);
        if (env->ExceptionCheck()) {
            LOGE("AudioCapturerJni JNI: call addOnInfoChangedListener has exception");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return ERROR;
        }
    }
    return SUCCESS;
}

int32_t AudioCapturerJni::RemoveAudioCapturerDeviceChangeListener(
    long capturerPtr, const std::shared_ptr<AudioCapturerDeviceChangeCallback>& callback)
{
    LOGD("AudioCapturerJni::RemoveAudioCapturerDeviceChangeListener capturerPtr:%ld", capturerPtr);
    auto iter = deviceChangeObserver_.find(capturerPtr);
    if (iter != deviceChangeObserver_.end()) {
        (iter->second).remove(callback);
        if (iter->second.empty()) {
            deviceChangeObserver_.erase(iter);

            auto env = ARKUI_X_Plugin_GetJniEnv();
            CHECK_NULL_RETURN(env, ERROR);
            CHECK_NULL_RETURN(g_audioCapturerPluginClass.globalRef, ERROR);
            CHECK_NULL_RETURN(g_audioCapturerPluginClass.removeOnRoutingChangedListener, ERROR);

            env->CallVoidMethod(g_audioCapturerPluginClass.globalRef,
                g_audioCapturerPluginClass.removeOnRoutingChangedListener, capturerPtr);
            if (env->ExceptionCheck()) {
                LOGE("AudioCapturerJni JNI: call removeOnRoutingChangedListener has exception");
                env->ExceptionDescribe();
                env->ExceptionClear();
                return ERROR;
            }
        }
    }
    return SUCCESS;
}

int32_t AudioCapturerJni::RemoveAudioCapturerInfoChangeCallback(
    long capturerPtr, const std::shared_ptr<AudioCapturerInfoChangeCallback>& callback)
{
    LOGD("AudioCapturerJni::RemoveAudioCapturerInfoChangeCallback capturerPtr:%ld", capturerPtr);
    auto iter = capturerInfoChangeObserver_.find(capturerPtr);
    if (iter != capturerInfoChangeObserver_.end()) {
        (iter->second).remove(callback);
        if (iter->second.empty()) {
            capturerInfoChangeObserver_.erase(iter);
            auto env = ARKUI_X_Plugin_GetJniEnv();
            CHECK_NULL_RETURN(env, ERROR);
            CHECK_NULL_RETURN(g_audioCapturerPluginClass.globalRef, ERROR);
            CHECK_NULL_RETURN(g_audioCapturerPluginClass.removeOnInfoChangedListener, ERROR);

            env->CallVoidMethod(g_audioCapturerPluginClass.globalRef,
                g_audioCapturerPluginClass.removeOnInfoChangedListener, capturerPtr);
            if (env->ExceptionCheck()) {
                LOGE("AudioCapturerJni JNI: call removeOnInfoChangedListener has exception");
                env->ExceptionDescribe();
                env->ExceptionClear();
                return ERROR;
            }
        }
    }
    return SUCCESS;
}

int32_t AudioCapturerJni::GetStreamInfo(long capturerPtr, AudioStreamInfo& streamInfo)
{
    LOGD("AudioCapturerJni::GetStreamInfo capturerPtr:%ld", capturerPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.getFormat, ERROR);

    jobject jFormat =
        env->CallObjectMethod(g_audioCapturerPluginClass.globalRef, g_audioCapturerPluginClass.getFormat, capturerPtr);
    if (env->ExceptionCheck() || (!jFormat)) {
        LOGE("AudioCapturerJni JNI: call getFormat has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    jclass jFormatCls = env->FindClass("android/media/AudioFormat");
    // Sample Rate
    jmethodID jGetSampleRateId = env->GetMethodID(jFormatCls, "getSampleRate", "()I");
    jint sampleRate = env->CallIntMethod(jFormat, jGetSampleRateId);
    streamInfo.samplingRate = static_cast<AudioSamplingRate>(sampleRate);
    // Channels
    jmethodID jGetChannelCntId = env->GetMethodID(jFormatCls, "getChannelCount", "()I");
    jint channelCnt = env->CallIntMethod(jFormat, jGetChannelCntId);
    streamInfo.channels = static_cast<AudioChannel>(channelCnt);
    // Encoding
    jmethodID jGetEncodingId = env->GetMethodID(jFormatCls, "getEncoding", "()I");
    jint encoding = env->CallIntMethod(jFormat, jGetEncodingId);
    streamInfo.encoding = AudioEncodingType::ENCODING_INVALID;
    if (IsPCMFormat(encoding)) {
        streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    }
    // Sample format
    streamInfo.format = ConvertAudioSampleFormatToOh(static_cast<AudioFormatEncoding>(encoding));

    // Channel layout
    env->DeleteLocalRef(jFormatCls);
    env->DeleteLocalRef(jFormat);

    LOGD("AudioCapturerJni::GetStreamInfo before convert:samplingRate:%d, channels:%d, encoding:%d;after "
         "convert:samplingRate:%d, channels:%d, encoding:%d, format:%d",
        sampleRate, channelCnt, encoding, streamInfo.samplingRate, streamInfo.channels, streamInfo.encoding,
        streamInfo.format);

    return SUCCESS;
}

int32_t AudioCapturerJni::GetAudioSessionId(long capturerPtr, uint32_t& sessionID)
{
    LOGD("AudioCapturerJni::GetAudioSessionId capturerPtr:%ld", capturerPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.getAudioSessionId, ERROR);

    jint jSessionId = env->CallIntMethod(
        g_audioCapturerPluginClass.globalRef, g_audioCapturerPluginClass.getAudioSessionId, capturerPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioCapturerJni JNI: call getAudioSessionId has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    if (jSessionId < 0) {
        LOGD("AudioCapturerJni::GetAudioSessionId SessionId is invalid.");
        return ERR_ILLEGAL_STATE;
    }
    sessionID = jSessionId;
    LOGD("AudioCapturerJni::GetAudioSessionId jSessionId:%ld", jSessionId);
    return SUCCESS;
}

bool AudioCapturerJni::GetTimestamp(long capturerPtr, Timestamp& timestamp)
{
    LOGD("AudioCapturerJni::GetTimestamp capturerPtr:%ld", capturerPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.globalRef, false);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.getTimestamp, false);

    jclass jTimestampCls = env->FindClass("android/media/AudioTimestamp");
    jobject jTimestamp = env->AllocObject(jTimestampCls);
    jint jAvailable = env->CallBooleanMethod(
        g_audioCapturerPluginClass.globalRef, g_audioCapturerPluginClass.getTimestamp, capturerPtr, jTimestamp);
    if (env->ExceptionCheck()) {
        LOGE("AudioCapturerJni JNI: call getTimestamp has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteLocalRef(jTimestamp);
        env->DeleteLocalRef(jTimestampCls);
        return false;
    }
    LOGD("AudioCapturerJni::GetTimestamp jAvailable:%d", jAvailable);
    if (jAvailable == SUCCESS) {
        jfieldID jFramePositionId = env->GetFieldID(jTimestampCls, "framePosition", "J");
        jfieldID jNanoTimeId = env->GetFieldID(jTimestampCls, "nanoTime", "J");
        timestamp.framePosition = env->GetLongField(jTimestamp, jFramePositionId);
        timestamp.time.tv_sec = 0;
        timestamp.time.tv_nsec = env->GetLongField(jTimestamp, jNanoTimeId);
        LOGD("AudioCapturerJni::GetTimestamp framePosition:%ld, tv_nsec:%ld", timestamp.framePosition,
            timestamp.time.tv_nsec);
    }
    env->DeleteLocalRef(jTimestamp);
    env->DeleteLocalRef(jTimestampCls);
    return jAvailable != SUCCESS ? false : true;
}

int32_t AudioCapturerJni::GetCurrentInputDevices(long capturerPtr, AudioDeviceDescriptor& deviceInfo)
{
    LOGD("AudioCapturerJni::GetCurrentInputDevices capturerPtr:%ld", capturerPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioCapturerPluginClass.getRoutedDevice, ERROR);

    jobject jDeviceInfo = env->CallObjectMethod(
        g_audioCapturerPluginClass.globalRef, g_audioCapturerPluginClass.getRoutedDevice, capturerPtr);
    if (env->ExceptionCheck() || (!jDeviceInfo)) {
        LOGE("AudioCapturerJni JNI: call getRoutedDevice has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }

    AudioDeviceDescriptor deviceInfoTemp = AudioCommonJni::GetDeviceInfo(jDeviceInfo);
    if (deviceInfoTemp.deviceRole_ == INPUT_DEVICE) {
        deviceInfo = deviceInfoTemp;
    }
    env->DeleteLocalRef(jDeviceInfo);
    return SUCCESS;
}

void AudioCapturerJni::SetCapturerState(long capturerPtr, const AudioStandard::CapturerState capturerState)
{
    capturerStateMap_[capturerPtr] = capturerState;
}

void AudioCapturerJni::NotifyCapturerInfoChanged(long capturerPtr, AudioCapturerChangeInfo changeInfo)
{
    LOGD("AudioCapturerJni::NotifyCapturerInfoChanged, key:%ld", capturerPtr);
    auto iter = capturerInfoChangeObserver_.find(capturerPtr);
    if (iter != capturerInfoChangeObserver_.end()) {
        for (auto infoChangeCallback : iter->second) {
            infoChangeCallback->OnStateChange(changeInfo);
        }
    }
}

void AudioCapturerJni::Finalize(long capturerPtr)
{
    LOGD("AudioCapturerJni::Finalize capturerPtr:%ld", capturerPtr);
    // clear deviceChangeObserver_
    auto deviceChangeIter = deviceChangeObserver_.find(capturerPtr);
    if (deviceChangeIter != deviceChangeObserver_.end()) {
        deviceChangeObserver_.erase(deviceChangeIter);
    }
    // clear capturerInfoChangeObserver_
    auto capturerInfoChangeIter = capturerInfoChangeObserver_.find(capturerPtr);
    if (capturerInfoChangeIter != capturerInfoChangeObserver_.end()) {
        capturerInfoChangeObserver_.erase(capturerInfoChangeIter);
    }
    // clear periodPositionObserver_
    auto periodPositionIter = periodPositionObserver_.find(capturerPtr);
    if (periodPositionIter != periodPositionObserver_.end()) {
        periodPositionObserver_.erase(periodPositionIter);
    }
    // clear capturerPositionObserver_
    auto capturerPositionIter = capturerPositionObserver_.find(capturerPtr);
    if (capturerPositionIter != capturerPositionObserver_.end()) {
        capturerPositionObserver_.erase(capturerPositionIter);
    }
    // clear capturerStateMap_
    auto capturerStateIter = capturerStateMap_.find(capturerPtr);
    if (capturerStateIter != capturerStateMap_.end()) {
        capturerStateMap_.erase(capturerStateIter);
    }
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_audioCapturerPluginClass.globalRef);
    CHECK_NULL_VOID(g_audioCapturerPluginClass.finalize);

    env->CallVoidMethod(g_audioCapturerPluginClass.globalRef, g_audioCapturerPluginClass.finalize, capturerPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioCapturerJni JNI: call finalize has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}
} // namespace OHOS::Plugin
