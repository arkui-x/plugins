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

#include "recorder_jni.h"

#include "interfaces/native/log.h"
#include "inner_api/plugin_utils_inner.h"
#include "plugin_utils.h"
#include "media_errors.h"

namespace OHOS::Plugin {
namespace {
const char RECORDER_CLASS_NAME[] = "ohos/ace/plugin/recorderplugin/RecorderPlugin";
static const JNINativeMethod METHODS[] = {
    {"nativeInit", "()V", reinterpret_cast<void *>(RecorderJni::NativeInit)},
    {"nativeOnError", "(JII)V", reinterpret_cast<void *>(RecorderJni::NativeOnError)},
    {"nativeOnInfo", "(JII)V", reinterpret_cast<void *>(RecorderJni::NativeOnInfo)},
};

static const char METHOD_CREATE_MEDIA_RECORDER[] = "createMediaRecorder";
static const char METHOD_RELEASE_MEDIA_RECORDER[] = "releaseMediaRecorder";
static const char METHOD_PREPARE[] = "prepare";
static const char METHOD_START[] = "start";
static const char METHOD_STOP[] = "stop";
static const char METHOD_PAUSE[] = "pause";
static const char METHOD_RESUME[] = "resume";
static const char METHOD_RESET[] = "reset";
static const char METHOD_RELEASE[] = "release";
static const char METHOD_SET_VIDEO_SIZE[] = "setVideoSize";
static const char METHOD_SET_VIDEO_ENCODER[] = "setVideoEncoder";
static const char METHOD_SET_VIDEO_ENCODING_BIT_RATE[] = "setVideoEncodingBitRate";
static const char METHOD_SET_VIDEO_FRAME_RATE[] = "setVideoFrameRate";
static const char METHOD_SET_VIDEO_SOURCE[] = "setVideoSource";
static const char METHOD_GET_SURFACE[] = "getSurface";
static const char METHOD_SET_AUDIO_SOURCE[] = "setAudioSource";
static const char METHOD_SET_AUDIO_SAMPLING_RATE[] = "setAudioSamplingRate";
static const char METHOD_SET_AUDIO_ENCODING_BIT_RATE[] = "setAudioEncodingBitRate";
static const char METHOD_SET_AUDIO_ENCODER[] = "setAudioEncoder";
static const char METHOD_SET_AUDIO_CHANNELS[] = "setAudioChannels";
static const char METHOD_SET_OUTPUT_FORMAT[] = "setOutputFormat";
static const char METHOD_SET_OUTPUT_FILE[] = "setOutputFile";
static const char METHOD_SET_LOCATION[] = "setLocation";
static const char METHOD_SET_ORIENTTATION_HINT[] = "setOrientationHint";

static const char SIGNATURE_CREATE_MEDIA_RECORDER[] = "(J)V";
static const char SIGNATURE_RELEASE_MEDIA_RECORDER[] = "(J)V";
static const char SIGNATURE_PREPARE[] = "(J)V";
static const char SIGNATURE_START[] = "(J)V";
static const char SIGNATURE_STOP[] = "(J)V";
static const char SIGNATURE_PAUSE[] = "(J)V";
static const char SIGNATURE_RESUME[] = "(J)V";
static const char SIGNATURE_RESET[] = "(J)V";
static const char SIGNATURE_RELEASE[] = "(J)V";
static const char SIGNATURE_SET_VIDEO_SIZE[] = "(JII)V";
static const char SIGNATURE_SET_VIDEO_ENCODER[] = "(JI)V";
static const char SIGNATURE_SET_VIDEO_ENCODING_BIT_RATE[] = "(JI)V";
static const char SIGNATURE_SET_VIDEO_FRAME_RATE[] = "(JI)V";
static const char SIGNATURE_SET_VIDEO_SOURCE[] = "(JI)V";
static const char SIGNATURE_GET_SURFACE[] = "(J)Landroid/view/Surface;";
static const char SIGNATURE_SET_AUDIO_SOURCE[] = "(JI)V";
static const char SIGNATURE_SET_AUDIO_SAMPLING_RATE[] = "(JI)V";
static const char SIGNATURE_SET_AUDIO_ENCODING_BIT_RATE[] = "(JI)V";
static const char SIGNATURE_SET_AUDIO_ENCODER[] = "(JI)V";
static const char SIGNATURE_SET_AUDIO_CHANNELS[] = "(JI)V";
static const char SIGNATURE_SET_OUTPUT_FORMAT[] = "(JI)V";
static const char SIGNATURE_SET_OUTPUT_FILE[] = "(JLjava/lang/String;)V";
static const char SIGNATURE_SET_LOCATION[] = "(JFF)V";
static const char SIGNATURE_SET_ORIENTTATION_HINT[] = "(JI)V";

struct {
    jmethodID createMediaRecorder;
    jmethodID releaseMediaRecorder;
    jmethodID prepare;
    jmethodID start;
    jmethodID stop;
    jmethodID pause;
    jmethodID resume;
    jmethodID reset;
    jmethodID release;
    jmethodID setVideoSize;
    jmethodID setVideoEncoder;
    jmethodID setVideoEncodingBitRate;
    jmethodID setVideoFrameRate;
    jmethodID setVideoSource;
    jmethodID getSurface;
    jmethodID setAudioSource;
    jmethodID setAudioSamplingRate;
    jmethodID setAudioEncodingBitRate;
    jmethodID setAudioEncoder;
    jmethodID setAudioChannels;
    jmethodID setOutputFormat;
    jmethodID setOutputFile;
    jmethodID setLocation;
    jmethodID setOrientationHint;
    jobject globalRef;
} g_recorderPluginClass;
}  // namespace
std::map<long, std::shared_ptr<Media::RecorderCallback>> RecorderJni::recorderCallbacks_;
bool RecorderJni::Register(void *env)
{
    auto *jniEnv = static_cast<JNIEnv *>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(RECORDER_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("RecorderJni JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void RecorderJni::NativeInit(JNIEnv *env, jobject jobj)
{
    CHECK_NULL_VOID(env);
    g_recorderPluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_recorderPluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_recorderPluginClass.createMediaRecorder = env->GetMethodID(
        cls, METHOD_CREATE_MEDIA_RECORDER, SIGNATURE_CREATE_MEDIA_RECORDER);
    CHECK_NULL_VOID(g_recorderPluginClass.createMediaRecorder);

    g_recorderPluginClass.releaseMediaRecorder = env->GetMethodID(
        cls, METHOD_RELEASE_MEDIA_RECORDER, SIGNATURE_RELEASE_MEDIA_RECORDER);
    CHECK_NULL_VOID(g_recorderPluginClass.releaseMediaRecorder);

    g_recorderPluginClass.prepare = env->GetMethodID(cls, METHOD_PREPARE, SIGNATURE_PREPARE);
    CHECK_NULL_VOID(g_recorderPluginClass.prepare);

    g_recorderPluginClass.start = env->GetMethodID(cls, METHOD_START, SIGNATURE_START);
    CHECK_NULL_VOID(g_recorderPluginClass.start);

    g_recorderPluginClass.stop = env->GetMethodID(cls, METHOD_STOP, SIGNATURE_STOP);
    CHECK_NULL_VOID(g_recorderPluginClass.stop);

    g_recorderPluginClass.pause = env->GetMethodID(cls, METHOD_PAUSE, SIGNATURE_PAUSE);
    CHECK_NULL_VOID(g_recorderPluginClass.pause);

    g_recorderPluginClass.resume = env->GetMethodID(cls, METHOD_RESUME, SIGNATURE_RESUME);
    CHECK_NULL_VOID(g_recorderPluginClass.resume);

    g_recorderPluginClass.reset = env->GetMethodID(cls, METHOD_RESET, SIGNATURE_RESET);
    CHECK_NULL_VOID(g_recorderPluginClass.reset);

    g_recorderPluginClass.release = env->GetMethodID(cls, METHOD_RELEASE, SIGNATURE_RELEASE);
    CHECK_NULL_VOID(g_recorderPluginClass.release);

    g_recorderPluginClass.setVideoSize = env->GetMethodID(cls, METHOD_SET_VIDEO_SIZE, SIGNATURE_SET_VIDEO_SIZE);
    CHECK_NULL_VOID(g_recorderPluginClass.setVideoSize);

    g_recorderPluginClass.setVideoEncoder = env->GetMethodID(
        cls, METHOD_SET_VIDEO_ENCODER, SIGNATURE_SET_VIDEO_ENCODER);
    CHECK_NULL_VOID(g_recorderPluginClass.setVideoEncoder);

    g_recorderPluginClass.setVideoEncodingBitRate = env->GetMethodID(
        cls, METHOD_SET_VIDEO_ENCODING_BIT_RATE, SIGNATURE_SET_VIDEO_ENCODING_BIT_RATE);
    CHECK_NULL_VOID(g_recorderPluginClass.setVideoEncodingBitRate);

    g_recorderPluginClass.setVideoFrameRate = env->GetMethodID(
        cls, METHOD_SET_VIDEO_FRAME_RATE, SIGNATURE_SET_VIDEO_FRAME_RATE);
    CHECK_NULL_VOID(g_recorderPluginClass.setVideoFrameRate);

    g_recorderPluginClass.setVideoSource = env->GetMethodID(cls, METHOD_SET_VIDEO_SOURCE, SIGNATURE_SET_VIDEO_SOURCE);
    CHECK_NULL_VOID(g_recorderPluginClass.setVideoSource);

    g_recorderPluginClass.getSurface = env->GetMethodID(cls, METHOD_GET_SURFACE, SIGNATURE_GET_SURFACE);
    CHECK_NULL_VOID(g_recorderPluginClass.getSurface);

    g_recorderPluginClass.setAudioSource = env->GetMethodID(cls, METHOD_SET_AUDIO_SOURCE, SIGNATURE_SET_AUDIO_SOURCE);
    CHECK_NULL_VOID(g_recorderPluginClass.setAudioSource);

    g_recorderPluginClass.setAudioSamplingRate = env->GetMethodID(
        cls, METHOD_SET_AUDIO_SAMPLING_RATE, SIGNATURE_SET_AUDIO_SAMPLING_RATE);
    CHECK_NULL_VOID(g_recorderPluginClass.setAudioSamplingRate);

    g_recorderPluginClass.setAudioEncodingBitRate = env->GetMethodID(
        cls, METHOD_SET_AUDIO_ENCODING_BIT_RATE, SIGNATURE_SET_AUDIO_ENCODING_BIT_RATE);
    CHECK_NULL_VOID(g_recorderPluginClass.setAudioEncodingBitRate);

    g_recorderPluginClass.setAudioEncoder = env->GetMethodID(
        cls, METHOD_SET_AUDIO_ENCODER, SIGNATURE_SET_AUDIO_ENCODER);
    CHECK_NULL_VOID(g_recorderPluginClass.setAudioEncoder);

    g_recorderPluginClass.setAudioChannels = env->GetMethodID(
        cls, METHOD_SET_AUDIO_CHANNELS, SIGNATURE_SET_AUDIO_CHANNELS);
    CHECK_NULL_VOID(g_recorderPluginClass.setAudioChannels);

    g_recorderPluginClass.setOutputFormat = env->GetMethodID(
        cls, METHOD_SET_OUTPUT_FORMAT, SIGNATURE_SET_OUTPUT_FORMAT);
    CHECK_NULL_VOID(g_recorderPluginClass.setOutputFormat);

    g_recorderPluginClass.setOutputFile = env->GetMethodID(cls, METHOD_SET_OUTPUT_FILE, SIGNATURE_SET_OUTPUT_FILE);
    CHECK_NULL_VOID(g_recorderPluginClass.setOutputFile);

    g_recorderPluginClass.setLocation = env->GetMethodID(cls, METHOD_SET_LOCATION, SIGNATURE_SET_LOCATION);
    CHECK_NULL_VOID(g_recorderPluginClass.setLocation);

    g_recorderPluginClass.setOrientationHint = env->GetMethodID(
        cls, METHOD_SET_ORIENTTATION_HINT, SIGNATURE_SET_ORIENTTATION_HINT);
    CHECK_NULL_VOID(g_recorderPluginClass.setOrientationHint);

    env->DeleteLocalRef(cls);
    return;
}

static jstring StringToJavaString(JNIEnv* env, const std::string &string)
{
    std::u16string str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(string);
    return env->NewString(reinterpret_cast<const jchar *>(str.data()), str.length());
}

void RecorderJni::NativeOnError(JNIEnv *env, jobject jobj, jlong key, jint what, jint extra)
{
    auto iter = recorderCallbacks_.find((long)key);
    if (iter == recorderCallbacks_.end()) {
        LOGD("RecorderJni NativeOnError callback not found");
        return;
    }
    (iter->second)->OnError((Media::RecorderErrorType)what, (int32_t)extra);
}

void RecorderJni::NativeOnInfo(JNIEnv *env, jobject jobj, jlong key, jint what, jint extra)
{
    auto iter = recorderCallbacks_.find((long)key);
    if (iter == recorderCallbacks_.end()) {
        LOGD("RecorderJni NativeOnInfo callback not found");
        return;
    }
    (iter->second)->OnInfo((int32_t)what, (int32_t)extra);
}

void RecorderJni::CreateMediaRecorder(long key)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_recorderPluginClass.globalRef);
    CHECK_NULL_VOID(g_recorderPluginClass.createMediaRecorder);

    env->CallVoidMethod(
        g_recorderPluginClass.globalRef, g_recorderPluginClass.createMediaRecorder, (jlong)key);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call CreateMediaRecorder has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
}

void RecorderJni::ReleaseMediaRecorder(long key)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_recorderPluginClass.globalRef);
    CHECK_NULL_VOID(g_recorderPluginClass.releaseMediaRecorder);

    env->CallVoidMethod(
        g_recorderPluginClass.globalRef, g_recorderPluginClass.releaseMediaRecorder, (jlong)key);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call ReleaseMediaRecorder has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }

    recorderCallbacks_.erase(key);
}

int32_t RecorderJni::SetVideoSource(long key, Media::VideoSourceType source)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.globalRef, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.setVideoSource, Media::MSERR_UNKNOWN);
    env->CallVoidMethod(
        g_recorderPluginClass.globalRef, g_recorderPluginClass.setVideoSource,
        (jlong)key, (jint)source);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call SetVideoSource has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

int32_t RecorderJni::SetVideoEncoder(long key, Media::VideoCodecFormat encoder)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.globalRef, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.setVideoEncoder, Media::MSERR_UNKNOWN);

    env->CallVoidMethod(
        g_recorderPluginClass.globalRef, g_recorderPluginClass.setVideoEncoder,
        (jlong)key, (jint)encoder);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call SetVideoEncoder has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

int32_t RecorderJni::SetVideoSize(long key, int32_t width, int32_t height)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.globalRef, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.setVideoSource, Media::MSERR_UNKNOWN);

    env->CallVoidMethod(
        g_recorderPluginClass.globalRef, g_recorderPluginClass.setVideoSize,
        (jlong)key, (jint)width, (jint)height);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call SetVideoSize has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

int32_t RecorderJni::SetVideoFrameRate(long key, int32_t frameRate)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.globalRef, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.setVideoFrameRate, Media::MSERR_UNKNOWN);

    env->CallVoidMethod(
        g_recorderPluginClass.globalRef, g_recorderPluginClass.setVideoFrameRate,
        (jlong)key, (jint)frameRate);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call SetVideoFrameRate has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

int32_t RecorderJni::SetVideoEncodingBitRate(long key, int32_t rate)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.globalRef, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.setVideoEncodingBitRate, Media::MSERR_UNKNOWN);

    env->CallVoidMethod(
        g_recorderPluginClass.globalRef, g_recorderPluginClass.setVideoEncodingBitRate,
        (jlong)key, (jint)rate);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call SetVideoEncodingBitRate has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

int32_t RecorderJni::SetVideoIsHdr(long key, bool isHdr)
{
}

sptr<OHOS::Surface> RecorderJni::GetSurface(long key)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, nullptr);
    CHECK_NULL_RETURN(g_recorderPluginClass.globalRef, nullptr);
    CHECK_NULL_RETURN(g_recorderPluginClass.getSurface, nullptr);

    jobject jsurface = env->CallObjectMethod(
        g_recorderPluginClass.globalRef, g_recorderPluginClass.getSurface, (jlong)key);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call GetSurface has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }

    return nullptr;
}

int32_t RecorderJni::SetAudioSource(long key, Media::AudioSourceType source)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.globalRef, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.setAudioSource, Media::MSERR_UNKNOWN);

    env->CallVoidMethod(
        g_recorderPluginClass.globalRef, g_recorderPluginClass.setAudioSource,
        (jlong)key, (jint)source);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call SetAudioSource has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

int32_t RecorderJni::SetAudioEncoder(long key, Media::AudioCodecFormat encoder)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.globalRef, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.setAudioEncoder, Media::MSERR_UNKNOWN);

    env->CallVoidMethod(
        g_recorderPluginClass.globalRef, g_recorderPluginClass.setAudioEncoder,
        (jlong)key, (jint)encoder);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call SetAudioEncoder has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

int32_t RecorderJni::SetAudioSampleRate(long key, int32_t rate)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.globalRef, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.setAudioSamplingRate, Media::MSERR_UNKNOWN);

    env->CallVoidMethod(
        g_recorderPluginClass.globalRef, g_recorderPluginClass.setAudioSamplingRate,
        (jlong)key, (jint)rate);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call SetAudioSampleRate has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

int32_t RecorderJni::SetAudioChannels(long key, int32_t num)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.globalRef, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.setAudioChannels, Media::MSERR_UNKNOWN);

    env->CallVoidMethod(
        g_recorderPluginClass.globalRef, g_recorderPluginClass.setAudioChannels,
        (jlong)key, (jint)num);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call SetAudioChannels has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

int32_t RecorderJni::SetAudioEncodingBitRate(long key, int32_t bitRate)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.globalRef, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.setAudioEncodingBitRate, Media::MSERR_UNKNOWN);

    env->CallVoidMethod(
        g_recorderPluginClass.globalRef, g_recorderPluginClass.setAudioEncodingBitRate,
        (jlong)key, (jint)bitRate);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call SetAudioEncodingBitRate has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

int32_t RecorderJni::SetOutputFormat(long key, Media::OutputFormatType format)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.globalRef, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.setOutputFormat, Media::MSERR_UNKNOWN);

    env->CallVoidMethod(
        g_recorderPluginClass.globalRef, g_recorderPluginClass.setOutputFormat,
        (jlong)key, (jint)format);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call SetOutputFormat has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

int32_t RecorderJni::SetOutputFile(long key, int32_t fd)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.globalRef, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.setOutputFile, Media::MSERR_UNKNOWN);

    char filePath[PATH_MAX] = {'\0'};
    char buf[128] = {'\0'};

    snprintf(buf, sizeof(buf), "proc/self/fd/%d", fd);
    CHECK_NULL_RETURN(readlink(buf, filePath, PATH_MAX) >= 0, Media::MSERR_SERVICE_DIED);

    env->CallVoidMethod(
        g_recorderPluginClass.globalRef, g_recorderPluginClass.setOutputFile,
        (jlong)key, StringToJavaString(env, filePath));
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call SetOutputFile has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

void RecorderJni::SetLocation(long key, float latitude, float longitude)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_recorderPluginClass.globalRef);
    CHECK_NULL_VOID(g_recorderPluginClass.setLocation);

    env->CallVoidMethod(
        g_recorderPluginClass.globalRef, g_recorderPluginClass.setLocation,
        (jlong)key, (jfloat)latitude, (jfloat)longitude);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call SetLocation has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return;
}

void RecorderJni::SetOrientationHint(long key, int32_t rotation)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_recorderPluginClass.globalRef);
    CHECK_NULL_VOID(g_recorderPluginClass.setOrientationHint);

    env->CallVoidMethod(
        g_recorderPluginClass.globalRef, g_recorderPluginClass.setOrientationHint,
        (jlong)key, (jint)rotation);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call SetOrientationHint has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return;
}

int32_t RecorderJni::Prepare(long key)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.globalRef,  Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.prepare, Media::MSERR_UNKNOWN);

    env->CallVoidMethod(g_recorderPluginClass.globalRef, g_recorderPluginClass.prepare, (jlong)key);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call Prepare has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

int32_t RecorderJni::Start(long key)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.globalRef,  Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.start, Media::MSERR_UNKNOWN);

    env->CallVoidMethod(g_recorderPluginClass.globalRef, g_recorderPluginClass.start, (jlong)key);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call Start has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

int32_t RecorderJni::Pause(long key)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.globalRef,  Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.pause, Media::MSERR_UNKNOWN);

    env->CallVoidMethod(g_recorderPluginClass.globalRef, g_recorderPluginClass.pause, (jlong)key);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call Pause has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

int32_t RecorderJni::Resume(long key)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.globalRef,  Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.resume, Media::MSERR_UNKNOWN);

    env->CallVoidMethod(g_recorderPluginClass.globalRef, g_recorderPluginClass.resume, (jlong)key);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call Resume has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

int32_t RecorderJni::Stop(long key)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.globalRef,  Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.stop, Media::MSERR_UNKNOWN);

    env->CallVoidMethod(g_recorderPluginClass.globalRef, g_recorderPluginClass.stop, (jlong)key);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call Stop has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

int32_t RecorderJni::Reset(long key)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.globalRef,  Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.reset, Media::MSERR_UNKNOWN);

    env->CallVoidMethod(g_recorderPluginClass.globalRef, g_recorderPluginClass.reset, (jlong)key);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call Reset has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

int32_t RecorderJni::Release(long key)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.globalRef,  Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_recorderPluginClass.release, Media::MSERR_UNKNOWN);

    env->CallVoidMethod(g_recorderPluginClass.globalRef, g_recorderPluginClass.release, (jlong)key);
    if (env->ExceptionCheck()) {
        LOGE("RecorderJni JNI: call Release has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

int32_t RecorderJni::SetRecorderCallback(long key, const std::shared_ptr<Media::RecorderCallback> &callback)
{
    auto iter = recorderCallbacks_.find(key);
    if (iter != recorderCallbacks_.end()) {
        LOGD("RecorderJni JNI: SetRecorderCallback  callback already exist");
        return Media::MSERR_OK;
    }
    recorderCallbacks_[key] = callback;
    return Media::MSERR_OK;
}
} // namespace OHOS::Plugin
