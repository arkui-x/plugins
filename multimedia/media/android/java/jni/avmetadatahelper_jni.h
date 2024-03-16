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

#ifndef PLUGINS_MULTIMEDIA_MEDIA_JNI_AVMETADATAHELPER_JNI_H
#define PLUGINS_MULTIMEDIA_MEDIA_JNI_AVMETADATAHELPER_JNI_H

#include <jni.h>
#include <memory>
#include <map>
#include <string>
#include <unordered_map>

#include "nocopyable.h"
#include "i_avmetadatahelper_service.h"

enum MetadataKeyCode : int32_t {
    /**
     * The metadata key to retrieve the information about the album title of the data source.
     */
    METADATA_KEY_ALBUM = 1,
    /**
     * The metadata key to retrieve the information about the performers or artist associated with the data source.
     */
    METADATA_KEY_ALBUMARTIST = 13,
    /**
     * The metadata key to retrieve the information about the artist of the data source.
     */
    METADATA_KEY_ARTIST = 2,
    /**
     * The metadata key to retrieve the information about the author of the data source.
     */
    METADATA_KEY_AUTHOR = 3,
    /**
     * The metadata key to retrieve the date when the data source was created or modified.
     */
    METADATA_KEY_YEAR = 8,
    /**
     * The metadata key to retrieve the information about the created time of
     * the media source. This keyword is provided for the media library.
     */
    METADATA_KEY_DATE = 5,
    /**
     * The metadata key to retrieve the information about the composer of the data source.
     */
    METADATA_KEY_COMPOSER = 4,
    /**
     * The metadata key to retrieve the playback duration (in ms) of the data source.
     */
    METADATA_KEY_DURATION = 9,
    /**
     * The metadata key to retrieve the content type or genre of the data source.
     */
    METADATA_KEY_GENRE = 6,
    /**
     * If this key exists the media contains audio content.
     */
    METADATA_KEY_HAS_AUDIO = 16,
    /**
     * If this key exists the media contains video content.
     */
    METADATA_KEY_HAS_VIDEO = 17,
    /**
     * The metadata key to retrieve the mime type of the media source. Some
     * example mime types include: "video/mp4", "audio/mp4", "audio/amr-wb",
     * etc.
     */
    METADATA_KEY_MIMETYPE = 12,
    /**
     * The metadata key to retrieve the number of tracks, such as audio, video,
     * text, in the media source, such as a mp4 or 3gpp file.
     */
    METADATA_KEY_NUM_TRACKS = 10,
    /**
     * This key retrieves the sample rate, if available.
     */
    METADATA_KEY_SAMPLERATE = 38,
    /**
     * The metadata key to retrieve the media source title.
     */
    METADATA_KEY_TITLE = 7,
    /**
     * If the media contains video, this key retrieves its height.
     */
    METADATA_KEY_VIDEO_HEIGHT = 19,
    /**
     * If the media contains video, this key retrieves its width.
     */
    METADATA_KEY_VIDEO_WIDTH = 18,
    /**
     * The metadata key to retrieve the information about the video
     * orientation.
     */
    METADATA_KEY_VIDEO_ROTATION = 24,
    /**
     * The sum of the metadata key.
     */
    METADATA_KEY_SUM = 18,
};

namespace OHOS::Plugin {
class AVMetadataHelperJni final {
public:
    AVMetadataHelperJni() = delete;
    ~AVMetadataHelperJni() = delete;
    static bool Register(void *env);
    // Called by Java
    static void NativeInit(JNIEnv *env, jobject jobj);
    static jint NativeReadAt(
        JNIEnv *env, jobject jthiz, jlong key, jlong position, jbyteArray buffer, jint offset, jint size);


    static void CreateMetadataRetriever(long key);
    static void ReleaseMetadataRetriever(long key);
    static int32_t SetSource(long key, const std::string &uri, int32_t usage);
    static int32_t SetSource(long key, int32_t fd, int64_t offset, int64_t size, int32_t usage);
    static int32_t SetSource(long key, const std::shared_ptr<Media::IMediaDataSource> &dataSrc);
    static std::unordered_map<int32_t, std::string> ResolveMetadata(long key);
    static std::shared_ptr<Media::AVSharedMemory> FetchArtPicture(long key);
    static void Release(long key);
    static int32_t SetHelperCallback(long key, const std::shared_ptr<Media::HelperCallback> &callback);
    static void NativeOnStateChanged(JNIEnv *env, jobject jthiz, jlong key, jint state);
private:
    static int32_t ConvertAVKeytoMetadataKey(int32_t key);
    static std::map<long, std::shared_ptr<Media::IMediaDataSource>> mediaDataSources_;
    static std::map<long, std::shared_ptr<Media::HelperCallback>> callbacks_;
};
} // namespace OHOS::Plugin
#endif // PLUGINS_MULTIMEDIA_MEDIA_JNI_AVMETADATAHELPER_JNI_H
