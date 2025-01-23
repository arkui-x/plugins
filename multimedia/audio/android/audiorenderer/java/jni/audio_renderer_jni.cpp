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

#include "audio_renderer_jni.h"

#include <jni.h>
#include <locale>
#include <securec.h>

#include "audio_common_jni.h"
#include "audio_convert_util.h"
#include "audio_errors.h"
#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"

namespace OHOS::Plugin {
namespace {
const char CLASS_NAME[] = "ohos/ace/plugin/audiorenderer/AudioRendererPlugin";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(AudioRendererJni::NativeInit) },
    { "nativeOnMarkerReached", "(JI)V", reinterpret_cast<void*>(AudioRendererJni::NativeOnMarkerReached) },
    { "nativeOnPeriodicNotification", "(JI)V",
        reinterpret_cast<void*>(AudioRendererJni::NativeOnPeriodicNotification) },
    { "nativeOnRoutingChanged", "(JLandroid/media/AudioDeviceInfo;)V",
        reinterpret_cast<void*>(AudioRendererJni::NativeOnRoutingChanged) } };

struct {
    jmethodID createAudioTrack;
    jmethodID play;
    jmethodID pause;
    jmethodID stop;
    jmethodID release;
    jmethodID flush;
    jmethodID write;
    jmethodID getMinBufferSize;
    jmethodID setSpeed;
    jmethodID getSpeed;
    jmethodID setVolume;
    jmethodID getMinVolume;
    jmethodID getMaxVolume;
    jmethodID getUnderrunCount;
    jmethodID getAudioAttributes;
    jmethodID getFormat;
    jmethodID getAudioSessionId;
    jmethodID getTimestamp;
    jmethodID setDualMonoMode;
    jmethodID setNotificationMarkerPosition;
    jmethodID setPositionNotificationPeriod;
    jmethodID setPlaybackPositionUpdateListener;
    jmethodID unsetPlaybackPositionUpdateListener;
    jmethodID getCurrentOutputDevices;
    jmethodID addOnRoutingChangedListener;
    jmethodID removeOnRoutingChangedListener;
    jmethodID finalize;
    jobject globalRef;
} g_audioRendererPluginClass;
} // namespace

bool AudioRendererJni::hasInit_ = false;
std::map<long, std::shared_ptr<RendererPositionCallback>> AudioRendererJni::rendererPositionObserver_;
std::map<long, std::shared_ptr<RendererPeriodPositionCallback>> AudioRendererJni::periodPositionObserver_;
std::map<long, std::shared_ptr<AudioRendererOutputDeviceChangeCallback>> AudioRendererJni::deviceChangeObserver_;

bool AudioRendererJni::Register(void* env)
{
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
        LOGE("AudioRendererJni JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void AudioRendererJni::NativeInit(JNIEnv* env, jobject jobj)
{
    CHECK_NULL_VOID(env);
    g_audioRendererPluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_audioRendererPluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_audioRendererPluginClass.createAudioTrack = env->GetMethodID(cls, "createAudioTrack", "(JIIIIIIIF)V");
    CHECK_NULL_VOID(g_audioRendererPluginClass.createAudioTrack);

    g_audioRendererPluginClass.play = env->GetMethodID(cls, "play", "(J)V");
    CHECK_NULL_VOID(g_audioRendererPluginClass.play);

    g_audioRendererPluginClass.pause = env->GetMethodID(cls, "pause", "(J)V");
    CHECK_NULL_VOID(g_audioRendererPluginClass.pause);

    g_audioRendererPluginClass.stop = env->GetMethodID(cls, "stop", "(J)V");
    CHECK_NULL_VOID(g_audioRendererPluginClass.stop);

    g_audioRendererPluginClass.release = env->GetMethodID(cls, "release", "(J)V");
    CHECK_NULL_VOID(g_audioRendererPluginClass.release);

    g_audioRendererPluginClass.flush = env->GetMethodID(cls, "flush", "(J)V");
    CHECK_NULL_VOID(g_audioRendererPluginClass.flush);

    g_audioRendererPluginClass.getMinBufferSize = env->GetMethodID(cls, "getMinBufferSize", "(JIII)I");
    CHECK_NULL_VOID(g_audioRendererPluginClass.getMinBufferSize);

    g_audioRendererPluginClass.write = env->GetMethodID(cls, "write", "(J[BI)I");
    CHECK_NULL_VOID(g_audioRendererPluginClass.write);

    g_audioRendererPluginClass.setSpeed = env->GetMethodID(cls, "setSpeed", "(JF)V");
    CHECK_NULL_VOID(g_audioRendererPluginClass.setSpeed);

    g_audioRendererPluginClass.getSpeed = env->GetMethodID(cls, "getSpeed", "(J)F");
    CHECK_NULL_VOID(g_audioRendererPluginClass.getSpeed);

    g_audioRendererPluginClass.setVolume = env->GetMethodID(cls, "setVolume", "(JF)I");
    CHECK_NULL_VOID(g_audioRendererPluginClass.setVolume);

    g_audioRendererPluginClass.getMinVolume = env->GetMethodID(cls, "getMinVolume", "(J)F");
    CHECK_NULL_VOID(g_audioRendererPluginClass.getMinVolume);

    g_audioRendererPluginClass.getMaxVolume = env->GetMethodID(cls, "getMaxVolume", "(J)F");
    CHECK_NULL_VOID(g_audioRendererPluginClass.getMaxVolume);

    g_audioRendererPluginClass.setDualMonoMode = env->GetMethodID(cls, "setDualMonoMode", "(JI)Z");
    CHECK_NULL_VOID(g_audioRendererPluginClass.setDualMonoMode);

    g_audioRendererPluginClass.setNotificationMarkerPosition =
        env->GetMethodID(cls, "setNotificationMarkerPosition", "(JI)I");
    CHECK_NULL_VOID(g_audioRendererPluginClass.setNotificationMarkerPosition);

    g_audioRendererPluginClass.setPositionNotificationPeriod =
        env->GetMethodID(cls, "setPositionNotificationPeriod", "(JI)I");
    CHECK_NULL_VOID(g_audioRendererPluginClass.setPositionNotificationPeriod);

    g_audioRendererPluginClass.setPlaybackPositionUpdateListener =
        env->GetMethodID(cls, "setPlaybackPositionUpdateListener", "(J)V");
    CHECK_NULL_VOID(g_audioRendererPluginClass.setPlaybackPositionUpdateListener);

    g_audioRendererPluginClass.addOnRoutingChangedListener =
        env->GetMethodID(cls, "addOnRoutingChangedListener", "(J)V");
    CHECK_NULL_VOID(g_audioRendererPluginClass.addOnRoutingChangedListener);

    g_audioRendererPluginClass.removeOnRoutingChangedListener =
        env->GetMethodID(cls, "removeOnRoutingChangedListener", "(J)V");
    CHECK_NULL_VOID(g_audioRendererPluginClass.removeOnRoutingChangedListener);

    g_audioRendererPluginClass.unsetPlaybackPositionUpdateListener =
        env->GetMethodID(cls, "unsetPlaybackPositionUpdateListener", "(J)V");
    CHECK_NULL_VOID(g_audioRendererPluginClass.unsetPlaybackPositionUpdateListener);

    g_audioRendererPluginClass.getUnderrunCount = env->GetMethodID(cls, "getUnderrunCount", "(J)I");
    CHECK_NULL_VOID(g_audioRendererPluginClass.getUnderrunCount);

    g_audioRendererPluginClass.getAudioAttributes =
        env->GetMethodID(cls, "getAudioAttributes", "(J)Landroid/media/AudioAttributes;");
    CHECK_NULL_VOID(g_audioRendererPluginClass.getAudioAttributes);

    g_audioRendererPluginClass.getFormat = env->GetMethodID(cls, "getFormat", "(J)Landroid/media/AudioFormat;");
    CHECK_NULL_VOID(g_audioRendererPluginClass.getFormat);

    g_audioRendererPluginClass.getCurrentOutputDevices =
        env->GetMethodID(cls, "getCurrentOutputDevices", "(J)Landroid/media/AudioDeviceInfo;");
    CHECK_NULL_VOID(g_audioRendererPluginClass.getCurrentOutputDevices);

    g_audioRendererPluginClass.getAudioSessionId = env->GetMethodID(cls, "getAudioSessionId", "(J)I");
    CHECK_NULL_VOID(g_audioRendererPluginClass.getAudioSessionId);

    g_audioRendererPluginClass.getTimestamp =
        env->GetMethodID(cls, "getTimestamp", "(JLandroid/media/AudioTimestamp;)Z");
    CHECK_NULL_VOID(g_audioRendererPluginClass.getTimestamp);

    g_audioRendererPluginClass.finalize = env->GetMethodID(cls, "finalize", "(J)V");
    CHECK_NULL_VOID(g_audioRendererPluginClass.finalize);

    env->DeleteLocalRef(cls);
    hasInit_ = true;
}

void AudioRendererJni::NativeOnMarkerReached(JNIEnv* env, jobject jobj, jlong rendererPtr, jint position)
{
    LOGD("AudioRendererJni::NativeOnMarkerReached, key:%{public}ld, position:%{public}d", rendererPtr, position);
    long key = static_cast<long>(rendererPtr);
    auto iter = rendererPositionObserver_.find(key);
    if (iter != rendererPositionObserver_.end()) {
        (iter->second)->OnMarkReached(static_cast<int64_t>(position));
    }
}

void AudioRendererJni::NativeOnPeriodicNotification(JNIEnv* env, jobject jobj, jlong rendererPtr, jint period)
{
    LOGD("AudioRendererJni::NativeOnPeriodicNotification, key:%{public}ld, position:%{public}d", rendererPtr, period);
    long key = static_cast<long>(rendererPtr);
    auto iter = periodPositionObserver_.find(key);
    if (iter != periodPositionObserver_.end()) {
        (iter->second)->OnPeriodReached(static_cast<int64_t>(period));
    }
}

void AudioRendererJni::NativeOnRoutingChanged(JNIEnv* env, jobject jobj, jlong rendererPtr, jobject jDeviceInfo)
{
    LOGD("AudioRendererJni::NativeOnRoutingChanged, key:%{public}ld", rendererPtr);
    long key = static_cast<long>(rendererPtr);
    auto iter = deviceChangeObserver_.find(key);
    if (iter != deviceChangeObserver_.end()) {
        AudioDeviceDescriptor deviceInfo = AudioCommonJni::GetDeviceInfo(jDeviceInfo);
        if (deviceInfo.deviceRole_ == OUTPUT_DEVICE) {
            (iter->second)->OnOutputDeviceChange(deviceInfo, AudioStreamDeviceChangeReason::UNKNOWN);
        }
    }
}

int32_t AudioRendererJni::CreateAudioTrack(
    long rendererPtr, const AudioRendererOptions& rendererOptions, float maxSpeed)
{
    LOGD("AudioRendererJni::CreateAudioTrack rendererPtr:%{public}ld", rendererPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.createAudioTrack, ERROR);

    LOGD("AudioRendererJni:createAudioTrack before convert:\
        sampleRate:%{public}d, channel:%{public}d, encoding:%{public}d,\
        usage:%{public}d, privacyType:%{public}d, maxSpeed:%{public}f",
        rendererOptions.streamInfo.samplingRate, rendererOptions.streamInfo.channels, rendererOptions.streamInfo.format,
        rendererOptions.rendererInfo.streamUsage, rendererOptions.privacyType, maxSpeed);

    jint sampleRate = rendererOptions.streamInfo.samplingRate;
    jint channel = (1 << rendererOptions.streamInfo.channels) - 1;
    jint encoding = ConvertAudioSampleFormatToAn(rendererOptions.streamInfo.format);
    jint channelLayout = ConvertChannelCountToLayout(rendererOptions.streamInfo.channels);

    AudioAttributesUsage usage;
    AudioAttributesContenType content;
    ConvertAudioUsageToAn(rendererOptions.rendererInfo.streamUsage, usage, content);
    jint privacyType = ConvertPrivacyTypeToAn(rendererOptions.privacyType);

    LOGD("AudioRendererJni:createAudioTrack after convert:\
        sampleRate:%{public}d, channel:%{public}d, encoding:%{public}d,\
        channelLayout:%{public}d, usage:%{public}d, content:%{public}d, privacyType:%{public}d",
        sampleRate, channel, encoding, channelLayout, usage, content, privacyType);
    env->CallVoidMethod(g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.createAudioTrack, rendererPtr,
        sampleRate, channel, encoding, channelLayout, usage, content, privacyType, maxSpeed);

    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call CreateAudioTrack has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    return SUCCESS;
}

bool AudioRendererJni::Start(long rendererPtr)
{
    LOGD("AudioRendererJni::Start rendererPtr:%{public}ld", rendererPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, false);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.play, false);

    env->CallVoidMethod(g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.play, rendererPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call Start has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return true;
}

bool AudioRendererJni::Pause(long rendererPtr)
{
    LOGD("AudioRendererJni::Pause rendererPtr:%{public}ld", rendererPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, false);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.pause, false);

    env->CallVoidMethod(g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.pause, rendererPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call Pause has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return true;
}

bool AudioRendererJni::Stop(long rendererPtr)
{
    LOGD("AudioRendererJni::Stop rendererPtr:%{public}ld", rendererPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, false);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.stop, false);

    env->CallVoidMethod(g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.stop, rendererPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call Stop has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return true;
}

bool AudioRendererJni::Release(long rendererPtr)
{
    LOGD("AudioRendererJni::Release rendererPtr:%{public}ld", rendererPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, false);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.release, false);

    env->CallVoidMethod(g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.release, rendererPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call Release has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return true;
}

bool AudioRendererJni::Flush(long rendererPtr)
{
    LOGD("AudioRendererJni::Flush rendererPtr:%{public}ld", rendererPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, false);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.flush, false);

    env->CallVoidMethod(g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.flush, rendererPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call Flush has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return true;
}

int32_t AudioRendererJni::GetBufferSize(long rendererPtr, const AudioStreamInfo& formatInfo, size_t& bufferSize)
{
    LOGD("AudioRendererJni::GetBufferSize rendererPtr:%{public}ld", rendererPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.getMinBufferSize, ERROR);

    LOGD(
        "AudioRendererJni:GetBufferSize before convert:samplingRate:%{public}d, channels:%{public}d, format:%{public}d",
        formatInfo.samplingRate, formatInfo.channels, formatInfo.format);
    jint sampleRate = formatInfo.samplingRate;
    jint format = ConvertAudioSampleFormatToAn(formatInfo.format);
    jint channelLayout = ConvertChannelCountToLayout(formatInfo.channels);

    LOGD("AudioRendererJni:GetBufferSize after convert:samplingRate:%{public}d, channelLayout:%{public}d, "
         "format:%{public}d",
        sampleRate, channelLayout, format);
    jint size = env->CallIntMethod(g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.getMinBufferSize,
        rendererPtr, sampleRate, channelLayout, format);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call getMinBufferSize has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    LOGD("AudioRendererJni::GetBufferSize bufferSize:%{public}d", size);
    if (size > 0) {
        bufferSize = static_cast<size_t>(size);
        return SUCCESS;
    }
    return ERROR;
}

int32_t AudioRendererJni::Write(long rendererPtr, uint8_t* buffer, size_t bufferSize)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, 0);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, 0);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.write, 0);

    jbyteArray jBuffer = env->NewByteArray(bufferSize);
    env->SetByteArrayRegion(jBuffer, 0, bufferSize, reinterpret_cast<const jbyte*>(buffer));

    jint size = env->CallIntMethod(
        g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.write, rendererPtr, jBuffer, bufferSize);
    env->DeleteLocalRef(jBuffer);
    if (env->ExceptionCheck() || (size < 0)) {
        LOGE("AudioRendererJni JNI: call Write has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return 0;
    }
    return static_cast<int32_t>(size);
}

int32_t AudioRendererJni::SetSpeed(long rendererPtr, float speed)
{
    LOGD("AudioRendererJni::SetSpeed rendererPtr:%{public}ld, speed:%{public}f", rendererPtr, speed);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.setSpeed, ERROR);

    env->CallVoidMethod(g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.setSpeed, rendererPtr, speed);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call setSpeed has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    return SUCCESS;
}

float AudioRendererJni::GetSpeed(long rendererPtr)
{
    LOGD("AudioRendererJni::GetSpeed rendererPtr:%{public}ld", rendererPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, 0.0f);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, 0.0f);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.getSpeed, 0.0f);

    jfloat speed =
        env->CallFloatMethod(g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.getSpeed, rendererPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call getSpeed has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return 0.0f;
    }
    LOGD("AudioRendererJni::GetSpeed speed:%{public}f", speed);
    return speed;
}

int32_t AudioRendererJni::SetVolume(long rendererPtr, float volume)
{
    LOGD("AudioRendererJni::SetVolume rendererPtr:%{public}ld, volume:%{public}f", rendererPtr, volume);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.setVolume, ERROR);

    jint result = env->CallIntMethod(
        g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.setVolume, rendererPtr, volume);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call setVolume has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }

    return result != SUCCESS ? ERR_OPERATION_FAILED : SUCCESS;
}

float AudioRendererJni::GetMinVolume(long rendererPtr)
{
    LOGD("AudioRendererJni::GetMinVolume rendererPtr:%{public}ld", rendererPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, 0.0f);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, 0.0f);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.getMinVolume, 0.0f);

    jfloat minVolume = env->CallFloatMethod(
        g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.getMinVolume, rendererPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call getMinVolume has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return 0.0f;
    }
    LOGD("AudioRendererJni::getMinVolume minVolume:%{public}f", minVolume);
    return minVolume;
}

float AudioRendererJni::GetMaxVolume(long rendererPtr)
{
    LOGD("AudioRendererJni::GetMaxVolume rendererPtr:%{public}ld", rendererPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, 1.0f);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, 1.0f);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.getMaxVolume, 1.0f);

    jfloat maxVolume = env->CallFloatMethod(
        g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.getMaxVolume, rendererPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call getMaxVolume has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return 1.0f;
    }
    LOGD("AudioRendererJni::getMaxVolume maxVolume:%{public}f", maxVolume);
    return maxVolume;
}

uint32_t AudioRendererJni::GetUnderrunCount(long rendererPtr)
{
    LOGD("AudioRendererJni::GetUnderrunCount rendererPtr:%{public}ld", rendererPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, 0);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, 0);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.getUnderrunCount, 0);

    jint count = env->CallIntMethod(
        g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.getUnderrunCount, rendererPtr);
    if (env->ExceptionCheck() || (count < 0)) {
        LOGE("AudioRendererJni JNI: call getUnderrunCount has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return 0;
    }
    LOGD("AudioRendererJni::getUnderrunCount count:%{public}d", count);
    return count;
}

int32_t AudioRendererJni::GetCurrentOutputDevices(long rendererPtr, AudioDeviceDescriptor& deviceInfo)
{
    LOGD("AudioRendererJni::GetCurrentOutputDevices rendererPtr:%{public}ld", rendererPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.getCurrentOutputDevices, ERROR);

    jobject jDeviceInfo = env->CallObjectMethod(
        g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.getCurrentOutputDevices, rendererPtr);
    if (env->ExceptionCheck() || (!jDeviceInfo)) {
        LOGE("AudioRendererJni JNI: call getCurrentOutputDevices has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    AudioDeviceDescriptor deviceInfoTmp = AudioCommonJni::GetDeviceInfo(jDeviceInfo);
    if (deviceInfoTmp.deviceRole_ == OUTPUT_DEVICE) {
        deviceInfo = deviceInfoTmp;
    }
    env->DeleteLocalRef(jDeviceInfo);
    return SUCCESS;
}

int32_t AudioRendererJni::GetAudioSessionId(long rendererPtr, uint32_t& sessionID)
{
    LOGD("AudioRendererJni::GetAudioSessionId rendererPtr:%{public}ld", rendererPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.getAudioSessionId, ERROR);

    jint jSessionId = env->CallIntMethod(
        g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.getAudioSessionId, rendererPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call getAudioSessionId has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    if (jSessionId < 0) {
        LOGD("AudioRendererJni::GetAudioSessionId SessionId is invalid.");
        return ERR_ILLEGAL_STATE;
    }
    sessionID = jSessionId;
    LOGD("AudioRendererJni::GetAudioSessionId jSessionId:%{public}ld", jSessionId);
    return SUCCESS;
}

int32_t AudioRendererJni::GetRendererInfo(long rendererPtr, AudioRendererInfo& rendererInfo)
{
    LOGD("AudioRendererJni::GetRendererInfo rendererPtr:%{public}ld", rendererPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.getAudioAttributes, ERROR);

    jobject jAttribute = env->CallObjectMethod(
        g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.getAudioAttributes, rendererPtr);
    if (env->ExceptionCheck() || (!jAttribute)) {
        LOGE("AudioRendererJni JNI: call getAudioAttributes has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    jclass jAttributeCls = env->FindClass("android/media/AudioAttributes");
    // ContentType
    jmethodID jContentTypeId = env->GetMethodID(jAttributeCls, "getContentType", "()I");
    jint jContentType = env->CallIntMethod(jAttribute, jContentTypeId);
    // Usage
    jmethodID jUsageId = env->GetMethodID(jAttributeCls, "getUsage", "()I");
    jint jUsage = env->CallIntMethod(jAttribute, jUsageId);
    env->DeleteLocalRef(jAttributeCls);
    env->DeleteLocalRef(jAttribute);

    ConvertAudioUsageToOh(static_cast<AudioAttributesUsage>(jUsage),
        static_cast<AudioAttributesContenType>(jContentType), rendererInfo.streamUsage, rendererInfo.contentType);
    LOGD("AudioRendererJni::GetRendererInfo before convert:ContentType:%{public}d, Usage:%{public}d;\
        after convert:streamUsage:%{public}d, contentType:%{public}d",
        jContentType, jUsage, rendererInfo.streamUsage, rendererInfo.contentType);

    return SUCCESS;
}

int32_t AudioRendererJni::GetStreamInfo(long rendererPtr, AudioStreamInfo& streamInfo)
{
    LOGD("AudioRendererJni::GetStreamInfo rendererPtr:%{public}ld", rendererPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.getFormat, ERROR);

    jobject jFormat =
        env->CallObjectMethod(g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.getFormat, rendererPtr);
    if (env->ExceptionCheck() || (!jFormat)) {
        LOGE("AudioRendererJni JNI: call getFormat has exception");
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

    env->DeleteLocalRef(jFormatCls);
    env->DeleteLocalRef(jFormat);

    LOGD("AudioRendererJni::GetStreamInfo before convert:samplingRate:{public}%d, channels:%{public}d,\
        encoding:%{public}d; after convert:samplingRate:%{public}d,\
        channels:%{public}d, encoding:%{public}d, format:%{public}d",
        sampleRate, channelCnt, encoding, streamInfo.samplingRate, streamInfo.channels, streamInfo.encoding,
        streamInfo.format);
    return SUCCESS;
}

bool AudioRendererJni::GetTimestamp(long rendererPtr, Timestamp& timestamp)
{
    LOGD("AudioRendererJni::GetTimestamp rendererPtr:%{public}ld", rendererPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, false);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.getTimestamp, false);

    jclass jTimestampCls = env->FindClass("android/media/AudioTimestamp");
    jobject jTimestamp = env->AllocObject(jTimestampCls);
    jboolean jAvailable = env->CallBooleanMethod(
        g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.getTimestamp, rendererPtr, jTimestamp);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call getTimestamp has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    if (jAvailable) {
        jfieldID jFramePositionId = env->GetFieldID(jTimestampCls, "framePosition", "J");
        jfieldID jNanoTimeId = env->GetFieldID(jTimestampCls, "nanoTime", "J");
        timestamp.framePosition = env->GetLongField(jTimestamp, jFramePositionId);
        timestamp.time.tv_sec = 0;
        timestamp.time.tv_nsec = env->GetLongField(jTimestamp, jNanoTimeId);
        LOGD("AudioRendererJni::GetTimestamp framePosition:%{public}ld, tv_nsec:%{public}ld", timestamp.framePosition,
            timestamp.time.tv_nsec);
    }
    env->DeleteLocalRef(jTimestamp);
    env->DeleteLocalRef(jTimestampCls);
    return jAvailable;
}

int32_t AudioRendererJni::SetDualMonoMode(long rendererPtr, ChannelBlendMode blendMode)
{
    LOGD("AudioRendererJni::SetDualMonoMode rendererPtr:%{public}ld, blendMode:%{public}d", rendererPtr, blendMode);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.setDualMonoMode, ERROR);

    int dualMonoMode = ConvertBlendModeToAn(blendMode);
    LOGD("AudioRendererJni::SetDualMonoMode before convert:blendMode:%{public}d;\
        after convert:dualMonoMode:%{public}d",
        blendMode, dualMonoMode);
    jboolean result = env->CallBooleanMethod(
        g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.setDualMonoMode, rendererPtr, dualMonoMode);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call setDualMonoMode has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    return result ? SUCCESS : ERROR;
}

int32_t AudioRendererJni::SetNotificationMarkerPosition(long rendererPtr, int64_t markPosition)
{
    LOGD("AudioRendererJni::SetNotificationMarkerPosition rendererPtr:%{public}ld, markPosition:%{public}d",
        rendererPtr, markPosition);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.setNotificationMarkerPosition, ERROR);

    jint result = env->CallIntMethod(g_audioRendererPluginClass.globalRef,
        g_audioRendererPluginClass.setNotificationMarkerPosition, rendererPtr, markPosition);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call getUnderrunCount has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    return result != SUCCESS ? ERR_OPERATION_FAILED : SUCCESS;
}

int32_t AudioRendererJni::SetRendererPositionUpdateListener(
    long rendererPtr, const std::shared_ptr<RendererPositionCallback>& callback)
{
    LOGD("AudioRendererJni::SetRendererPositionUpdateListener rendererPtr:%{public}ld", rendererPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.setPlaybackPositionUpdateListener, ERROR);

    rendererPositionObserver_[rendererPtr] = callback;
    env->CallVoidMethod(g_audioRendererPluginClass.globalRef,
        g_audioRendererPluginClass.setPlaybackPositionUpdateListener, rendererPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call SetRendererPositionUpdateListener has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    return SUCCESS;
}

void AudioRendererJni::UnsetRendererPositionUpdateListener(long rendererPtr)
{
    LOGD("AudioRendererJni::UnsetRendererPositionUpdateListener rendererPtr:%{public}ld", rendererPtr);
    UnregisterRendererPositionCallback(rendererPtr);
    UnsetPlaybackPositionUpdateListener(rendererPtr);
}

void AudioRendererJni::UnregisterRendererPositionCallback(long rendererPtr)
{
    LOGD("AudioRendererJni::UnregisterRendererPositionCallback rendererPtr:%{public}ld", rendererPtr);
    auto iter = rendererPositionObserver_.find(rendererPtr);
    if (iter != rendererPositionObserver_.end()) {
        rendererPositionObserver_.erase(iter);
    }
}

int32_t AudioRendererJni::SetPositionNotificationPeriod(long rendererPtr, int64_t frameNumber)
{
    LOGD("AudioRendererJni::SetPositionNotificationPeriod rendererPtr:%{public}ld, frameNumber:%{public}d", rendererPtr,
        frameNumber);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.setPositionNotificationPeriod, ERROR);

    jint result = env->CallIntMethod(g_audioRendererPluginClass.globalRef,
        g_audioRendererPluginClass.setPositionNotificationPeriod, rendererPtr, frameNumber);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call setPositionNotificationPeriod has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    return result != SUCCESS ? ERR_OPERATION_FAILED : SUCCESS;
}

int32_t AudioRendererJni::SetPeriodPositionUpdateListener(
    long rendererPtr, const std::shared_ptr<RendererPeriodPositionCallback>& callback)
{
    LOGD("AudioRendererJni::SetRendererPositionUpdateListener rendererPtr:%{public}ld", rendererPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.setPlaybackPositionUpdateListener, ERROR);

    periodPositionObserver_[rendererPtr] = callback;
    env->CallVoidMethod(g_audioRendererPluginClass.globalRef,
        g_audioRendererPluginClass.setPlaybackPositionUpdateListener, rendererPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call SetRendererPositionUpdateListener has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    return SUCCESS;
}

void AudioRendererJni::UnregisterPeriodPositionCallback(long rendererPtr)
{
    LOGD("AudioRendererJni::UnregisterPeriodPositionCallback rendererPtr:%{public}ld", rendererPtr);
    auto iter = periodPositionObserver_.find(rendererPtr);
    if (iter != periodPositionObserver_.end()) {
        periodPositionObserver_.erase(iter);
    }
}

void AudioRendererJni::UnsetPeriodPositionUpdateListener(long rendererPtr)
{
    LOGD("AudioRendererJni::UnsetPeriodPositionUpdateListener rendererPtr:%{public}ld", rendererPtr);
    UnregisterPeriodPositionCallback(rendererPtr);
    UnsetPlaybackPositionUpdateListener(rendererPtr);
}

void AudioRendererJni::UnsetPlaybackPositionUpdateListener(long rendererPtr)
{
    auto periodIter = periodPositionObserver_.find(rendererPtr);
    auto markerIter = rendererPositionObserver_.find(rendererPtr);
    if ((periodIter != periodPositionObserver_.end()) || (markerIter != rendererPositionObserver_.end())) {
        LOGD("There are still listeners present, will not unregister.");
        return;
    }

    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_audioRendererPluginClass.globalRef);
    CHECK_NULL_VOID(g_audioRendererPluginClass.unsetPlaybackPositionUpdateListener);

    env->CallVoidMethod(g_audioRendererPluginClass.globalRef,
        g_audioRendererPluginClass.unsetPlaybackPositionUpdateListener, rendererPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call unsetPlaybackPositionUpdateListener has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

int32_t AudioRendererJni::RegisterRendererDeviceChangeListener(
    long rendererPtr, const std::shared_ptr<AudioRendererOutputDeviceChangeCallback>& callback)
{
    LOGD("AudioRendererJni::RegisterRendererDeviceChangeListener rendererPtr:%{public}ld", rendererPtr);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.addOnRoutingChangedListener, ERROR);

    deviceChangeObserver_[rendererPtr] = callback;

    env->CallVoidMethod(
        g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.addOnRoutingChangedListener, rendererPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call addOnRoutingChangedListener has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    return SUCCESS;
}

void AudioRendererJni::UnregisterDeviceChangeCallback(long rendererPtr)
{
    LOGD("AudioRendererJni::UnregisterDeviceChangeCallback rendererPtr:%{public}ld", rendererPtr);
    auto iter = deviceChangeObserver_.find(rendererPtr);
    if (iter != deviceChangeObserver_.end()) {
        deviceChangeObserver_.erase(iter);
    }
}

int32_t AudioRendererJni::UnregisterRendererDeviceChangeListener(long rendererPtr)
{
    UnregisterDeviceChangeCallback(rendererPtr);

    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.globalRef, ERROR);
    CHECK_NULL_RETURN(g_audioRendererPluginClass.removeOnRoutingChangedListener, ERROR);

    env->CallVoidMethod(
        g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.removeOnRoutingChangedListener, rendererPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call removeOnRoutingChangedListener has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ERROR;
    }
    return SUCCESS;
}

void AudioRendererJni::Finalize(long rendererPtr)
{
    UnregisterRendererPositionCallback(rendererPtr);
    UnregisterPeriodPositionCallback(rendererPtr);
    UnregisterDeviceChangeCallback(rendererPtr);

    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_audioRendererPluginClass.globalRef);
    CHECK_NULL_VOID(g_audioRendererPluginClass.finalize);

    env->CallVoidMethod(g_audioRendererPluginClass.globalRef, g_audioRendererPluginClass.finalize, rendererPtr);
    if (env->ExceptionCheck()) {
        LOGE("AudioRendererJni JNI: call finalize has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}
} // namespace OHOS::Plugin
