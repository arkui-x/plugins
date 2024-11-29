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

#include "player_jni.h"
#include "plugins/interfaces/native/inner_api/plugin_utils_inner.h"

#include "media_errors.h"
#include "media_log.h"
#include "plugin_utils.h"
#include "mock_avsharedmemory.h"
#include "uri_helper.h"
#include "directory_ex.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN_PLAYER, "PlayerJni"};
}

namespace OHOS::Plugin {
namespace {
const char Player_CLASS_NAME[] = "ohos/ace/plugin/playerplugin/PlayerPlugin";
static const JNINativeMethod METHODS[] = {
    {"nativeInit", "()V", reinterpret_cast<void *>(PlayerJni::NativeInit)},
    {"nativeOnError", "(JI)V", reinterpret_cast<void *>(PlayerJni::NativeOnError)},
    {"nativeOnInfo", "(JII)V", reinterpret_cast<void *>(PlayerJni::NativeOnInfo)},
    {"nativeOnBufferingUpdate", "(JI)V", reinterpret_cast<void *>(PlayerJni::NativeOnBufferingUpdate)},
    {"nativeOnSeekComplete", "(JI)V", reinterpret_cast<void *>(PlayerJni::NativeOnSeekComplete)},
    {"nativeOnVideoSizeChanged", "(JII)V", reinterpret_cast<void *>(PlayerJni::NativeOnVideoSizeChanged)},
    {"nativeOnVolumnChanged", "(JF)V", reinterpret_cast<void *>(PlayerJni::NativeOnVolumnChanged)},
    {"nativeReadAt", "(JJ[BII)I", reinterpret_cast<void *>(PlayerJni::NativeReadAt)},
};

static jstring StringToJavaString(JNIEnv* env, const std::string &string)
{
    std::u16string str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(string);
    return env->NewString(reinterpret_cast<const jchar *>(str.data()), str.length());
}

static const char METHOD_CREATE_PLAYER[] = "addMediaPlayer";
static const char METHOD_RELEASE_PLAYER[] = "releaseMediaPlayer";
static const char METHOD_PREPARE[] = "prepare";
static const char METHOD_PREPARE_ASYNC[] = "prepareAsync";
static const char METHOD_PLAY[] = "play";
static const char METHOD_PAUSE[] = "pause";
static const char METHOD_STOP[] = "stop";
static const char METHOD_RESET[] = "reset";
static const char METHOD_RELEASE[] = "release";
static const char METHOD_SEAK_TO[] = "seekTo";
static const char METHOD_SET_VOLUME[] = "setVolume";
static const char METHOD_GET_TRACKINDEX[] = "getTrackIndex";
static const char METHOD_GET_TRACKINFO_STRING[] = "getTrackInfoString";
static const char METHOD_SET_DATASOURCE[] = "setDataSource";
static const char METHOD_SET_DATASOURCE_WITH_URL[] = "setDataSourceWithUrl";
static const char METHOD_SET_DATASOURCE_WITH_FD[] = "setDataSourceWithFd";
static const char METHOD_IS_LOOPING[] = "isLooping";
static const char METHOD_SET_LOOPING[] = "setLooping";
static const char METHOD_GET_CURRENT_POSITION[] = "getCurrentPosition";
static const char METHOD_GET_DURATION[] = "getDuration";
static const char METHOD_SET_SURFACE[] = "setSurface";
static const char METHOD_GET_VIDEO_WIDTH[] = "getVideoWidth";
static const char METHOD_GET_VIDEO_HEIGHT[] = "getVideoHeight";
static const char METHOD_SET_VIDEO_SCALING_MODE[] = "setVideoScalingMode";
static const char METHOD_SET_PLAYBACK_PARAMS[] = "setPlaybackParams";
static const char METHOD_GET_PLAYBACK_PARAMS[] = "getPlaybackParams";
static const char METHOD_SELECT_BITRATE[] = "selectBitrate";

static const char SIGNATURE_PLAY[] = "(J)V";
static const char SIGNATURE_SEEK[] = "(JII)V";
static const char SIGNATURE_SET_SURFACE[] = "(JIJ)V";
static const char SIGNATURE_SET_MODE[] = "(JI)V";
static const char SIGNATURE_SET_PLAYBACKPARAM[] = "(JI)V";
static const char SIGNATURE_GET_PLAYBACKPARAM[] = "(J)I";
static const char SIGNATURE_SET_VOLUME[] = "(JFF)V";
static const char SIGNATURE_IS_LOOPING[] = "(J)Z";
static const char SIGNATURE_SET_DATESOURCE_URL[] = "(JLjava/lang/String;)V";
static const char SIGNATURE_SET_DATESOURCE_FD[] = "(JLjava/lang/String;JJ)V";
static const char SIGNATURE_SET_LOOPING[] = "(JZ)V";
static const char SIGNATURE_GET_CURRENT_POSITION[] = "(J)I";
static const char SIGNATURE_GET_TRACK_STR[] = "(JILjava/lang/String;)Ljava/lang/String;";
static const char SIGNATURE_GET_TRACK_INDEX[] = "(JI)I";
static const char SIGNATURE_REGISTER_NET_CONN_CALLBACK[] = "(J[I)V";
static const char SIGNATURE_UNREGISTER_NET_CONN_CALLBACK[] = "(J)V";
static const char SIGNATURE_IS_DEFAULT_NETWORK_ACTIVE[] = "()Z";

static const int TRACK_TYPE_VIDEO = 1;
static const int TRACK_TYPE_AUDIO = 2;
static const int TRACK_TYPE_SUBTITLE = 4;

static const std::string TRACK_KEYS[] = {
    "codec_mime",
    "duration",
    "bitrate",
    "width",
    "height",
    "frame_rate",
    "channel_count",
    "sample_rate",
};

struct {
    jmethodID createPlayer;
    jmethodID releasePlayer;
    jmethodID prepare;
    jmethodID prepareAsync;
    jmethodID play;
    jmethodID pause;
    jmethodID stop;
    jmethodID reset;
    jmethodID release;
    jmethodID seekTo;
    jmethodID setVolume;
    jmethodID getTrackIndex;
    jmethodID getTrackInfoString;
    jmethodID setDataSource;
    jmethodID setDataSourceWithUrl;
    jmethodID setDataSourceWithFd;
    jmethodID isLooping;
    jmethodID setLooping;
    jmethodID getCurrentPosition;
    jmethodID getDuration;
    jmethodID setSurface;
    jmethodID getVideoWidth;
    jmethodID getVideoHeight;
    jmethodID setVideoScalingMode;
    jmethodID setPlaybackParams;
    jmethodID getPlaybackParams;
    jmethodID selectBitrate;
    jobject globalRef;
} g_playerpluginClass;
static const std::map<std::string, jmethodID> voidFuncMap = {
    {"createPlayer", g_playerpluginClass.createPlayer},
    {"releasePlayer", g_playerpluginClass.releasePlayer},
    {"prepare", g_playerpluginClass.prepare},
    {"prepareAsync", g_playerpluginClass.prepareAsync},
    {"play", g_playerpluginClass.play},
    {"pause", g_playerpluginClass.pause},
    {"stop", g_playerpluginClass.stop},
    {"reset", g_playerpluginClass.reset},
    {"release", g_playerpluginClass.release},
};
}
std::map<long, std::shared_ptr<Media::PlayerCallback>> PlayerJni::callbacks_;
std::map<long, std::shared_ptr<Media::IMediaDataSource>> PlayerJni::mediaDataSources_;

bool PlayerJni::Register(void *env)
{
    MEDIA_LOGI("PlayerJni::Register");
    auto *jniEnv = static_cast<JNIEnv *>(env);
    CHECK_AND_RETURN_RET(jniEnv != nullptr, false);
    jclass cls = jniEnv->FindClass(Player_CLASS_NAME);
    CHECK_AND_RETURN_RET(cls != nullptr, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        MEDIA_LOGE("PlayerJni JNI: RegisterNatives fail.");
        return false;
    }

    return true;
}

void PlayerJni::NativeInit(JNIEnv *env, jobject jobj)
{
    MEDIA_LOGI("PlayerJni::NativeInit");
    CHECK_AND_RETURN(env != nullptr);
    g_playerpluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_AND_RETURN(g_playerpluginClass.globalRef != nullptr);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_AND_RETURN(cls != nullptr);

    g_playerpluginClass.createPlayer = env->GetMethodID(cls, METHOD_CREATE_PLAYER, SIGNATURE_PLAY);
    CHECK_AND_RETURN(g_playerpluginClass.createPlayer != nullptr);

    g_playerpluginClass.releasePlayer = env->GetMethodID(cls, METHOD_RELEASE_PLAYER, SIGNATURE_PLAY);
    CHECK_AND_RETURN(g_playerpluginClass.releasePlayer != nullptr);

    g_playerpluginClass.prepare = env->GetMethodID(cls, METHOD_PREPARE, SIGNATURE_PLAY);
    CHECK_AND_RETURN(g_playerpluginClass.prepare != nullptr);

    g_playerpluginClass.prepareAsync = env->GetMethodID(cls, METHOD_PREPARE_ASYNC, SIGNATURE_PLAY);
    CHECK_AND_RETURN(g_playerpluginClass.prepareAsync != nullptr);

    g_playerpluginClass.play = env->GetMethodID(cls, METHOD_PLAY, SIGNATURE_PLAY);
    CHECK_AND_RETURN(g_playerpluginClass.play != nullptr);

    g_playerpluginClass.pause = env->GetMethodID(cls, METHOD_PAUSE, SIGNATURE_PLAY);
    CHECK_AND_RETURN(g_playerpluginClass.pause != nullptr);

    g_playerpluginClass.stop = env->GetMethodID(cls, METHOD_STOP, SIGNATURE_PLAY);
    CHECK_AND_RETURN(g_playerpluginClass.stop != nullptr);

    g_playerpluginClass.reset = env->GetMethodID(cls, METHOD_RESET, SIGNATURE_PLAY);
    CHECK_AND_RETURN(g_playerpluginClass.reset != nullptr);

    g_playerpluginClass.release = env->GetMethodID(cls, METHOD_RELEASE, SIGNATURE_PLAY);
    CHECK_AND_RETURN(g_playerpluginClass.release != nullptr);

    g_playerpluginClass.seekTo = env->GetMethodID(cls, METHOD_SEAK_TO, SIGNATURE_SEEK);
    CHECK_AND_RETURN(g_playerpluginClass.seekTo != nullptr);

    g_playerpluginClass.setVolume = env->GetMethodID(cls, METHOD_SET_VOLUME, SIGNATURE_SET_VOLUME);
    CHECK_AND_RETURN(g_playerpluginClass.setVolume != nullptr);

    g_playerpluginClass.getTrackIndex = env->GetMethodID(cls, METHOD_GET_TRACKINDEX, SIGNATURE_GET_TRACK_INDEX);
    CHECK_AND_RETURN(g_playerpluginClass.getTrackIndex != nullptr);

    g_playerpluginClass.getTrackInfoString = env->GetMethodID(cls, METHOD_GET_TRACKINFO_STRING, SIGNATURE_GET_TRACK_STR);
    CHECK_AND_RETURN(g_playerpluginClass.getTrackInfoString != nullptr);

    g_playerpluginClass.setDataSource = env->GetMethodID(cls, METHOD_SET_DATASOURCE, SIGNATURE_PLAY);
    CHECK_AND_RETURN(g_playerpluginClass.setDataSource != nullptr);

    g_playerpluginClass.setDataSourceWithUrl = env->GetMethodID(
        cls, METHOD_SET_DATASOURCE_WITH_URL, SIGNATURE_SET_DATESOURCE_URL);
    CHECK_AND_RETURN(g_playerpluginClass.setDataSourceWithUrl != nullptr);

    g_playerpluginClass.setDataSourceWithFd = env->GetMethodID(
        cls, METHOD_SET_DATASOURCE_WITH_FD, SIGNATURE_SET_DATESOURCE_FD);
    CHECK_AND_RETURN(g_playerpluginClass.setDataSourceWithFd != nullptr);

    g_playerpluginClass.isLooping = env->GetMethodID(cls, METHOD_IS_LOOPING, SIGNATURE_IS_LOOPING);
    CHECK_AND_RETURN(g_playerpluginClass.isLooping != nullptr);

    g_playerpluginClass.setLooping = env->GetMethodID(cls, METHOD_SET_LOOPING, SIGNATURE_SET_LOOPING);
    CHECK_AND_RETURN(g_playerpluginClass.setLooping != nullptr);

    g_playerpluginClass.getCurrentPosition = env->GetMethodID(
        cls, METHOD_GET_CURRENT_POSITION, SIGNATURE_GET_CURRENT_POSITION);
    CHECK_AND_RETURN(g_playerpluginClass.getCurrentPosition != nullptr);

    g_playerpluginClass.getDuration = env->GetMethodID(cls, METHOD_GET_DURATION, SIGNATURE_GET_CURRENT_POSITION);
    CHECK_AND_RETURN(g_playerpluginClass.getDuration != nullptr);

    g_playerpluginClass.setSurface = env->GetMethodID(cls, METHOD_SET_SURFACE, SIGNATURE_SET_SURFACE);
    CHECK_AND_RETURN(g_playerpluginClass.setSurface != nullptr);

    g_playerpluginClass.getVideoWidth = env->GetMethodID(cls, METHOD_GET_VIDEO_WIDTH, SIGNATURE_GET_CURRENT_POSITION);
    CHECK_AND_RETURN(g_playerpluginClass.getVideoWidth != nullptr);

    g_playerpluginClass.getVideoHeight = env->GetMethodID(cls, METHOD_GET_VIDEO_HEIGHT, SIGNATURE_GET_CURRENT_POSITION);
    CHECK_AND_RETURN(g_playerpluginClass.getVideoHeight != nullptr);

    g_playerpluginClass.setVideoScalingMode = env->GetMethodID(cls, METHOD_SET_VIDEO_SCALING_MODE, SIGNATURE_SET_MODE);
    CHECK_AND_RETURN(g_playerpluginClass.setVideoScalingMode != nullptr);

    g_playerpluginClass.setPlaybackParams = env->GetMethodID(
        cls, METHOD_SET_PLAYBACK_PARAMS, SIGNATURE_SET_PLAYBACKPARAM);
    CHECK_AND_RETURN(g_playerpluginClass.setPlaybackParams != nullptr);

    g_playerpluginClass.getPlaybackParams = env->GetMethodID(
        cls, METHOD_GET_PLAYBACK_PARAMS, SIGNATURE_GET_PLAYBACKPARAM);
    CHECK_AND_RETURN(g_playerpluginClass.getPlaybackParams != nullptr);

    g_playerpluginClass.selectBitrate = env->GetMethodID(cls, METHOD_SELECT_BITRATE, SIGNATURE_SET_MODE);
    CHECK_AND_RETURN(g_playerpluginClass.selectBitrate != nullptr);

    return;
}

int32_t PlayerJni::CallVoidFunc(long key, std::string funcName)
{
    MEDIA_LOGD("PlayerJni CallVoidFunc");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_AND_RETURN_RET(env != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.globalRef != nullptr, Media::MSERR_SERVICE_DIED);
    jmethodID methodId = nullptr;
    auto iter = voidFuncMap.find(funcName);
    if (iter != voidFuncMap.end()) {
        methodId = iter->second;
    }
    CHECK_AND_RETURN_RET(methodId != nullptr, Media::MSERR_SERVICE_DIED);
    jlong playerId = (jlong)key;
    env->CallVoidMethod(g_playerpluginClass.globalRef, methodId, playerId);
    if (env->ExceptionCheck()) {
        MEDIA_LOGE("PlayerJni JNI: call %{public}s has exception", funcName.c_str());
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_SERVICE_DIED;
    }
    return Media::MSERR_OK;
}

int32_t PlayerJni::SetSurface(long key, int32_t instanceId, long id)
{
    MEDIA_LOGD("PlayerJni SetSurface");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_AND_RETURN_RET(env != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.globalRef != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.setSurface != nullptr, Media::MSERR_SERVICE_DIED);

    jlong playerId = (jlong)key;
    env->CallVoidMethod(
        g_playerpluginClass.globalRef,g_playerpluginClass.setSurface, playerId, (jint)instanceId, (jlong)id);
    if (env->ExceptionCheck()) {
        MEDIA_LOGE("PlayerJni JNI: call SetSurface has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_SERVICE_DIED;
    }
    return Media::MSERR_OK;
}

int32_t PlayerJni::GetCurrentPosition(long key, int32_t &value)
{
    MEDIA_LOGD("PlayerJni GetCurrentPosition");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_AND_RETURN_RET(env != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.globalRef != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.getCurrentPosition != nullptr, Media::MSERR_SERVICE_DIED);

    jlong playerId = (jlong)key;
    jint ret = env->CallIntMethod(
        g_playerpluginClass.globalRef,g_playerpluginClass.getCurrentPosition, playerId);
    if (env->ExceptionCheck()) {
        MEDIA_LOGE("PlayerJni JNI: call GetCurrentPosition has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_SERVICE_DIED;
    }
    value = static_cast<int32_t>(ret);
    return Media::MSERR_OK;
}

int32_t PlayerJni::GetDuration(long key, int32_t &value)
{
    MEDIA_LOGD("PlayerJni GetDuration");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_AND_RETURN_RET(env != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.globalRef != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.getDuration != nullptr, Media::MSERR_SERVICE_DIED);

    jlong playerId = (jlong)key;
    jint ret = env->CallIntMethod(
        g_playerpluginClass.globalRef,g_playerpluginClass.getDuration, playerId);
    if (env->ExceptionCheck()) {
        MEDIA_LOGE("PlayerJni JNI: call GetDuration has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_SERVICE_DIED;
    }
    value = static_cast<int32_t>(ret);
    return Media::MSERR_OK;
}

int32_t PlayerJni::GetVideoWidth(long key)
{
    MEDIA_LOGD("PlayerJni GetVideoWidth");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_AND_RETURN_RET(env != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.globalRef != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.getVideoWidth != nullptr, Media::MSERR_SERVICE_DIED);

    jlong playerId = (jlong)key;
    jint ret = env->CallIntMethod(
        g_playerpluginClass.globalRef,g_playerpluginClass.getVideoWidth, playerId);
    if (env->ExceptionCheck()) {
        MEDIA_LOGE("PlayerJni JNI: call GetVideoWidth has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_SERVICE_DIED;
    }

    return static_cast<int32_t>(ret);
}

int32_t PlayerJni::GetVideoHeight(long key)
{
    MEDIA_LOGD("PlayerJni GetVideoHeight");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_AND_RETURN_RET(env != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.globalRef != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.getVideoHeight != nullptr, Media::MSERR_SERVICE_DIED);

    jlong playerId = (jlong)key;
    jint ret = env->CallIntMethod(
        g_playerpluginClass.globalRef,g_playerpluginClass.getVideoHeight, playerId);
    if (env->ExceptionCheck()) {
        MEDIA_LOGE("PlayerJni JNI: call GetVideoHeight has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_SERVICE_DIED;
    }

    return static_cast<int32_t>(ret);
}

int32_t PlayerJni::Seek(long key, int32_t mSeconds, Media::PlayerSeekMode mode)
{
    MEDIA_LOGD("PlayerJni Seek");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_AND_RETURN_RET(env != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.globalRef != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.seekTo != nullptr, Media::MSERR_SERVICE_DIED);

    jlong playerId = (jlong)key;
    env->CallVoidMethod(
        g_playerpluginClass.globalRef, g_playerpluginClass.seekTo, playerId, (jint)mSeconds, (jint)mode);
    if (env->ExceptionCheck()) {
        MEDIA_LOGE("PlayerJni JNI: call Seek has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_SERVICE_DIED;
    }

    return Media::MSERR_OK;
}

int32_t PlayerJni::SetVolume(long key, float leftVolume, float rightVolume)
{
    MEDIA_LOGD("PlayerJni SetVolume");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_AND_RETURN_RET(env != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.globalRef != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.setVolume != nullptr, Media::MSERR_SERVICE_DIED);

    jlong playerId = (jlong)key;
    env->CallVoidMethod(g_playerpluginClass.globalRef,
        g_playerpluginClass.setVolume, playerId, (jfloat)leftVolume, (jfloat)rightVolume);
    if (env->ExceptionCheck()) {
        MEDIA_LOGE("PlayerJni JNI: call SetVolume has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_SERVICE_DIED;
    }

    return Media::MSERR_OK;
}

int32_t PlayerJni::SetLooping(long key, bool loop)
{
    MEDIA_LOGD("PlayerJni SetLooping");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_AND_RETURN_RET(env != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.globalRef != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.setLooping != nullptr, Media::MSERR_SERVICE_DIED);

    jlong playerId = (jlong)key;
    env->CallVoidMethod(g_playerpluginClass.globalRef,
        g_playerpluginClass.setLooping, playerId, (jboolean)loop);
    if (env->ExceptionCheck()) {
        MEDIA_LOGE("PlayerJni JNI: call SetLooping has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_SERVICE_DIED;
    }

    return Media::MSERR_OK;
}

int32_t PlayerJni::SetVideoScalingMode(long key, int32_t mode)
{
    MEDIA_LOGD("PlayerJni SetVideoScalingMode");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_AND_RETURN_RET(env != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.globalRef != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.setVideoScalingMode != nullptr, Media::MSERR_SERVICE_DIED);

    jlong playerId = (jlong)key;
    env->CallVoidMethod(g_playerpluginClass.globalRef, g_playerpluginClass.setVideoScalingMode, playerId, (jint)mode);
    if (env->ExceptionCheck()) {
        MEDIA_LOGE("PlayerJni JNI: call SetVideoScalingMode has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_SERVICE_DIED;
    }

    return Media::MSERR_OK;
}

bool PlayerJni::IsLooping(long key)
{
    MEDIA_LOGD("PlayerJni IsLooping");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_AND_RETURN_RET(env != nullptr, false);
    CHECK_AND_RETURN_RET(g_playerpluginClass.globalRef != nullptr, false);
    CHECK_AND_RETURN_RET(g_playerpluginClass.isLooping != nullptr, false);

    jlong playerId = (jlong)key;
    jboolean ret = env->CallBooleanMethod(g_playerpluginClass.globalRef, g_playerpluginClass.isLooping, playerId);
    if (env->ExceptionCheck()) {
        MEDIA_LOGE("PlayerJni JNI: call IsLooping has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }

    return static_cast<bool>(ret);
}

int32_t PlayerJni::SetSpeed(long key, Media::PlaybackRateMode mode)
{
    MEDIA_LOGD("PlayerJni SetSpeed");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_AND_RETURN_RET(env != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.globalRef != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.setPlaybackParams != nullptr, Media::MSERR_SERVICE_DIED);

    jlong playerId = (jlong)key;
    env->CallVoidMethod(g_playerpluginClass.globalRef,
        g_playerpluginClass.setPlaybackParams, playerId, (jint)mode);
    if (env->ExceptionCheck()) {
        MEDIA_LOGE("PlayerJni JNI: call SetSpeed has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_SERVICE_DIED;
    }

    return Media::MSERR_OK;
}

int32_t PlayerJni::GetSpeed(long key, Media::PlaybackRateMode &mode)
{
    MEDIA_LOGD("PlayerJni SetSpeed");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_AND_RETURN_RET(env != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.globalRef != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.getPlaybackParams != nullptr, Media::MSERR_SERVICE_DIED);

    jlong playerId = (jlong)key;
    jint value = env->CallIntMethod(g_playerpluginClass.globalRef,
        g_playerpluginClass.getPlaybackParams, playerId);
    if (env->ExceptionCheck()) {
        MEDIA_LOGE("PlayerJni JNI: call SetSpeed has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_SERVICE_DIED;
    }
    mode = static_cast<Media::PlaybackRateMode>(value);
    return Media::MSERR_OK;
}

int32_t PlayerJni::SelectBitRate(long key, uint32_t bitRate)
{
    MEDIA_LOGD("PlayerJni SelectBitRate");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_AND_RETURN_RET(env != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.globalRef != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.selectBitrate != nullptr, Media::MSERR_SERVICE_DIED);

    jlong playerId = (jlong)key;
    env->CallVoidMethod(g_playerpluginClass.globalRef,
        g_playerpluginClass.selectBitrate, playerId, (jint)bitRate);
    if (env->ExceptionCheck()) {
        MEDIA_LOGE("PlayerJni JNI: call SetSpeed has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_SERVICE_DIED;
    }

    return Media::MSERR_OK;
}

int32_t PlayerJni::setDataSource(long key, std::string path)
{
    MEDIA_LOGD("PlayerJni setDataSource");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_AND_RETURN_RET(env != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.globalRef != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.setDataSourceWithUrl != nullptr, Media::MSERR_SERVICE_DIED);

    jlong playerId = (jlong)key;
    env->CallVoidMethod(g_playerpluginClass.globalRef,
        g_playerpluginClass.setDataSourceWithUrl, playerId, StringToJavaString(env, path));
    if (env->ExceptionCheck()) {
        MEDIA_LOGE("PlayerJni JNI: call setDataSource has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_SERVICE_DIED;
    }

    return Media::MSERR_OK;
}

int32_t PlayerJni::setDataSource(long key, int32_t fd, int64_t offset, int64_t size)
{
    MEDIA_LOGD("PlayerJni setDataSource");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_AND_RETURN_RET(env != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.globalRef != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.setDataSourceWithFd != nullptr, Media::MSERR_SERVICE_DIED);

    char filePath[PATH_MAX] = {'\0'};
    char buf[128] = {'\0'};

    snprintf(buf, sizeof(buf), "proc/self/fd/%d", fd);

    CHECK_AND_RETURN_RET(readlink(buf, filePath, PATH_MAX) >= 0, Media::MSERR_SERVICE_DIED);
    MEDIA_LOGD("PlayerJni setDataSource   %{public}s", filePath);

    env->CallVoidMethod(g_playerpluginClass.globalRef, g_playerpluginClass.setDataSourceWithFd,
        (jlong)key, StringToJavaString(env, filePath), (jlong)offset, (jlong)size);
    if (env->ExceptionCheck()) {
        MEDIA_LOGE("PlayerJni JNI: call setDataSourceWithFd has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

int32_t PlayerJni::setDataSource(long key, const std::shared_ptr<Media::IMediaDataSource> &dataSrc)
{
    MEDIA_LOGD("PlayerJni setDataSource");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_AND_RETURN_RET(env != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.globalRef != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.setDataSource != nullptr, Media::MSERR_SERVICE_DIED);

    mediaDataSources_[key] = dataSrc;

    jlong playerId = (jlong)key;
    env->CallVoidMethod(g_playerpluginClass.globalRef, g_playerpluginClass.setDataSource, playerId);
    if (env->ExceptionCheck()) {
        MEDIA_LOGE("PlayerJni JNI: call setDataSource has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_SERVICE_DIED;
    }

    return Media::MSERR_OK;
}

int32_t PlayerJni::GetVideoTrackInfo(long key, std::vector<Media::Format> &videoTrack)
{
    return GetTrackInfo(key, videoTrack, TRACK_TYPE_VIDEO);
}

int32_t PlayerJni::GetAudioTrackInfo(long key, std::vector<Media::Format> &audioTrack)
{
    return GetTrackInfo(key, audioTrack, TRACK_TYPE_AUDIO);
}

int32_t PlayerJni::GetSubtitleTrackInfo(long key, std::vector<Media::Format> &subtitleTrack)
{
    return GetTrackInfo(key, subtitleTrack, TRACK_TYPE_SUBTITLE);
}

int32_t PlayerJni::GetTrackInfo(long key, std::vector<Media::Format> &Track, int type)
{
    MEDIA_LOGD("PlayerJni GetTrackInfo");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_AND_RETURN_RET(env != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.globalRef != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.getTrackInfoString != nullptr, Media::MSERR_SERVICE_DIED);
    CHECK_AND_RETURN_RET(g_playerpluginClass.getTrackIndex != nullptr, Media::MSERR_SERVICE_DIED);

    jlong playerId = (jlong)key;
    jint index = env->CallIntMethod(g_playerpluginClass.globalRef,
        g_playerpluginClass.getTrackIndex, playerId, (jint)type);
    if (env->ExceptionCheck()) {
        MEDIA_LOGE("PlayerJni JNI: call getTrackIndex has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_SERVICE_DIED;
    }

    if ((int)index < 0) {
        return Media::MSERR_OK;
    }

    Media::Format infoBody;
    infoBody.PutIntValue("track_type", type);
    infoBody.PutIntValue("track_index", (int)index);
    for (const auto &str : TRACK_KEYS) {
        jstring value = (jstring)env->CallObjectMethod(g_playerpluginClass.globalRef,
            g_playerpluginClass.getTrackInfoString, playerId, (jint)type, StringToJavaString(env, str));
        if (env->ExceptionCheck()) {
            MEDIA_LOGE("PlayerJni JNI: call GetTrackInfo has exception");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return Media::MSERR_SERVICE_DIED;
        }
        const char* utf8Str = env->GetStringUTFChars(value, NULL);
        int len = strlen(utf8Str);
        if (len > 0) {
            std::string result(utf8Str, len);
            infoBody.PutStringValue(str, result);
        }
    }
    Track.emplace_back(infoBody);
    return Media::MSERR_OK;
}

int32_t PlayerJni::SetCallback(long key, const std::shared_ptr<Media::PlayerCallback> &callback)
{
    MEDIA_LOGD("PlayerJni SetCallback");
    if (callbacks_.find(key) != callbacks_.end()) {
        return Media::MSERR_OK;
    }
    callbacks_[key] = callback;
    return Media::MSERR_OK;
}

void PlayerJni::NativeOnInfo(JNIEnv *env, jobject jobj, jlong key, jint what, jint extra)
{
    MEDIA_LOGD("PlayerJni NativeOnInfo type = %{public}d", what);
    auto iter = callbacks_.find((long)key);
    if (iter == callbacks_.end()) {
        MEDIA_LOGD("PlayerJni NativeOnInfo callback not found");
        return;
    }
    Media::Format infoBody;
    (iter->second)->OnInfo((Media::PlayerOnInfoType)what, (int32_t)extra, infoBody);
}

void PlayerJni::NativeOnError(JNIEnv *env, jobject jobj, jlong key, jint code)
{
    auto iter = callbacks_.find((long)key);
    if (iter == callbacks_.end()) {
        MEDIA_LOGD("PlayerJni NativeOnError callback not found");
        return;
    }
    (iter->second)->OnError((int32_t)code, " ");
}

void PlayerJni::NativeOnBufferingUpdate(JNIEnv *env, jobject jobj, jlong key, jint percent)
{
    auto iter = callbacks_.find((long)key);
    if (iter == callbacks_.end()) {
        MEDIA_LOGD("PlayerJni NativeOnBufferingUpdate callback not found");
        return;
    }
    Media::Format infoBody;
    infoBody.PutIntValue(std::string(Media::PlayerKeys::PLAYER_BUFFERING_PERCENT), percent);
    (iter->second)->OnInfo(Media::INFO_TYPE_BUFFERING_UPDATE, 0, infoBody);
}

void PlayerJni::NativeOnSeekComplete(JNIEnv *env, jobject jobj, jlong key, jint position)
{
    auto iter = callbacks_.find((long)key);
    if (iter == callbacks_.end()) {
        MEDIA_LOGD("PlayerJni NativeOnSeekComplete callback not found");
        return;
    }
    Media::Format infoBody;
    (iter->second)->OnInfo(Media::INFO_TYPE_SEEKDONE, position, infoBody);
}

void PlayerJni::NativeOnVideoSizeChanged(JNIEnv *env, jobject jobj, jlong key, jint width, jint height)
{
    auto iter = callbacks_.find((long)key);
    if (iter == callbacks_.end()) {
        MEDIA_LOGD("PlayerJni NativeOnVideoSizeChanged callback not found");
        return;
    }
    Media::Format infoBody;
    infoBody.PutIntValue(Media::PlayerKeys::PLAYER_WIDTH, width);
    infoBody.PutIntValue(Media::PlayerKeys::PLAYER_HEIGHT, height);
    (iter->second)->OnInfo(Media::INFO_TYPE_RESOLUTION_CHANGE, 0, infoBody);
}

void PlayerJni::NativeOnVolumnChanged(JNIEnv *env, jobject jobj, jlong key, jfloat vol)
{
    auto iter = callbacks_.find((long)key);
    if (iter == callbacks_.end()) {
        MEDIA_LOGD("PlayerJni NativeOnVolumnChanged callback not found");
        return;
    }
    Media::Format infoBody;
    infoBody.PutFloatValue(Media::PlayerKeys::PLAYER_VOLUME_LEVEL, vol);
    (iter->second)->OnInfo(Media::INFO_TYPE_VOLUME_CHANGE, 0, infoBody);
}

jint PlayerJni::NativeReadAt(
        JNIEnv *env, jobject jthiz, jlong key, jlong position, jbyteArray buffer, jint offset, jint size)
{
    auto iter = mediaDataSources_.find((long)key);
    if (iter == mediaDataSources_.end()) {
        MEDIA_LOGD("PlayerJni NativeReadAt callback not found");
        return 0;
    }
    std::shared_ptr<Media::MockAVSharedMemory> dataSrc_ptr =
        std::make_shared<Media::MockAVSharedMemory>((int)size, 2);

    (iter->second)->ReadAt(dataSrc_ptr, (uint32_t)size, (int64_t)position);

    jbyte *data = (jbyte *)dataSrc_ptr->GetBase();

    env->SetByteArrayRegion(buffer, 0, (jint)dataSrc_ptr->GetSize(), data);
    return (jint)dataSrc_ptr->GetSize();
}
} // namespace OHOS::Plugin
