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

#ifndef PLUGINS_MULTIMEDIA_MEDIA_JNI_PLAYER_JNI_H
#define PLUGINS_MULTIMEDIA_MEDIA_JNI_PLAYER_JNI_H

#include <jni.h>
#include <memory>
#include <map>
#include "player_impl.h"

namespace OHOS::Plugin {
class PlayerJni final {
public:
    PlayerJni() = delete;
    ~PlayerJni() = delete;
    static bool Register(void *env);
    // Called by Java
    static void NativeInit(JNIEnv *env, jobject jobj);

    static int32_t CallVoidFunc(long key, std::string funcName);
    static int32_t GetCurrentPosition(long key, int32_t &value);
    static int32_t GetDuration(long key, int32_t &value);
    static int32_t GetVideoWidth(long key);
    static int32_t GetVideoHeight(long key);
    static int32_t Seek(long key, int32_t mSeconds, Media::PlayerSeekMode mode);
    static int32_t SetVolume(long key, float leftVolume, float rightVolume);
    static int32_t SetLooping(long key, bool loop);
    static bool IsLooping(long key);
    static int32_t SetSpeed(long key, Media::PlaybackRateMode mode);
    static int32_t GetSpeed(long key, Media::PlaybackRateMode &mode);
    static int32_t SelectBitRate(long key, uint32_t bitRate);
    static int32_t setDataSource(long key, std::string path);
    static int32_t setDataSource(long key, int32_t fd, int64_t offset, int64_t size);
    static int32_t setDataSource(long key, const std::shared_ptr<Media::IMediaDataSource> &dataSrc);
    static int32_t SetSurface(long key, int32_t instanceId, long id);
    static int32_t GetVideoTrackInfo(long key, std::vector<Media::Format> &videoTrack);
    static int32_t GetAudioTrackInfo(long key, std::vector<Media::Format> &audioTrack);
    static int32_t GetSubtitleTrackInfo(long key, std::vector<Media::Format> &subtitleTrack);
    static int32_t SetVideoScalingMode(long key, int32_t mode);
    static int32_t SetCallback(long key, const std::shared_ptr<Media::PlayerCallback> &callback);
    static void NativeOnInfo(JNIEnv *env, jobject jobj, jlong key, jint what, jint extra);
    static void NativeOnError(JNIEnv *env, jobject jobj, jlong key, jint code);
    static void NativeOnBufferingUpdate(JNIEnv *env, jobject jobj, jlong key, jint percent);
    static void NativeOnSeekComplete(JNIEnv *env, jobject jobj, jlong key, jint position);
    static void NativeOnVideoSizeChanged(JNIEnv *env, jobject jobj, jlong key, jint width, jint height);
    static void NativeOnVolumnChanged(JNIEnv *env, jobject jobj, jlong key, jfloat vol);
    static jint NativeReadAt(
        JNIEnv *env, jobject jthiz, jlong key, jlong position, jbyteArray buffer, jint offset, jint size);
private:
    static int32_t GetTrackInfo(long key, std::vector<Media::Format> &Track, int type);

    static std::map<long, std::shared_ptr<Media::PlayerCallback>> callbacks_;
    static std::map<long, std::shared_ptr<Media::IMediaDataSource>> mediaDataSources_;
};
} // namespace OHOS::Plugin
#endif // PLUGINS_MULTIMEDIA_MEDIA_JNI_PLAYER_JNI_H
