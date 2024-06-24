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

#ifndef PLUGINS_MULTIMEDIA_MEDIA_JNI_RECORDER_JNI_H
#define PLUGINS_MULTIMEDIA_MEDIA_JNI_RECORDER_JNI_H

#include <jni.h>
#include <memory>
#include <map>

#include "recorder.h"

namespace OHOS::Plugin {
class RecorderJni final {
public:
    RecorderJni() = delete;
    ~RecorderJni() = delete;
    static bool Register(void *env);
    // Called by Java
    static void NativeInit(JNIEnv *env, jobject jobj);
    static void NativeOnError(JNIEnv *env, jobject jobj, jlong key, jint what, jint extra);
    static void NativeOnInfo(JNIEnv *env, jobject jobj, jlong key, jint what, jint extra);

    static void CreateMediaRecorder(long key);
    static void ReleaseMediaRecorder(long key);
    static int32_t SetVideoSource(long key, Media::VideoSourceType source);
    static int32_t SetVideoEncoder(long key, Media::VideoCodecFormat encoder);
    static int32_t SetVideoSize(long key, int32_t width, int32_t height);
    static int32_t SetVideoFrameRate(long key, int32_t frameRate);
    static int32_t SetVideoEncodingBitRate(long key, int32_t rate);
    static int32_t SetVideoIsHdr(long key, bool isHdr);
    static sptr<OHOS::Surface> GetSurface(long key);
    static int32_t SetAudioSource(long key, Media::AudioSourceType source);
    static int32_t SetAudioEncoder(long key, Media::AudioCodecFormat encoder);
    static int32_t SetAudioSampleRate(long key, int32_t rate);
    static int32_t SetAudioChannels(long key, int32_t num);
    static int32_t SetAudioEncodingBitRate(long key, int32_t bitRate);
    static int32_t SetOutputFormat(long key, Media::OutputFormatType format);
    static int32_t SetOutputFile(long key, int32_t fd);
    static void SetLocation(long key, float latitude, float longitude);
    static void SetOrientationHint(long key, int32_t rotation);

    static int32_t Prepare(long key);
    static int32_t Start(long key);
    static int32_t Pause(long key);
    static int32_t Resume(long key);
    static int32_t Stop(long key);
    static int32_t Reset(long key);
    static int32_t Release(long key);
    static int32_t SetRecorderCallback(long key, const std::shared_ptr<Media::RecorderCallback> &callback);
private:
    static std::map<long, std::shared_ptr<Media::RecorderCallback>> recorderCallbacks_;
};
} // namespace OHOS::Plugin
#endif // PLUGINS_MULTIMEDIA_MEDIA_JNI_RECORDER_JNI_H