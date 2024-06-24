/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_INNERKITS_NATIVE_INCLUDE_PHOTO_ALBUM_ASSET_H_
#define INTERFACES_INNERKITS_NATIVE_INCLUDE_PHOTO_ALBUM_ASSET_H_

#include "userfile_manager_types.h"

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))
class PhotoAlbum {
public:
    EXPORT PhotoAlbum();
    EXPORT virtual ~PhotoAlbum();

    EXPORT void SetAlbumId(const int32_t photoAlbumId);
    EXPORT int32_t GetAlbumId() const;

    EXPORT void SetPhotoAlbumType(const PhotoAlbumType type);
    EXPORT PhotoAlbumType GetPhotoAlbumType() const;

    EXPORT void SetPhotoAlbumSubType(const PhotoAlbumSubType subType);
    EXPORT PhotoAlbumSubType GetPhotoAlbumSubType() const;

    EXPORT void SetAlbumUri(const std::string &uri);
    EXPORT const std::string& GetAlbumUri() const;

    EXPORT void SetAlbumName(const std::string &photoAlbumName);
    EXPORT const std::string& GetAlbumName() const;

    EXPORT void SetCoverUri(const std::string &coverUri);
    EXPORT const std::string& GetCoverUri() const;

    EXPORT void SetDateModified(const int64_t dateModified);
    EXPORT int64_t GetDateModified() const;

    EXPORT void SetCount(const int32_t count);
    EXPORT int32_t GetCount() const;

    EXPORT void SetImageCount(const int32_t count);
    EXPORT int32_t GetImageCount() const;

    EXPORT void SetVideoCount(const int32_t count);
    EXPORT int32_t GetVideoCount() const;

    EXPORT void SetLatitude(const double latitude);
    EXPORT double GetLatitude() const;

    EXPORT void SetLongitude(const double longitude);
    EXPORT double GetLongitude() const;

    EXPORT void SetRelativePath(const std::string &logicalRelativePath);
    EXPORT const std::string& GetRelativePath() const;

    EXPORT void SetResultNapiType(const ResultNapiType resultNapiType);
    EXPORT ResultNapiType GetResultNapiType() const;

    EXPORT void SetHiddenOnly(const bool hiddenOnly);
    EXPORT bool GetHiddenOnly() const;

    EXPORT void SetLocationOnly(const bool locationOnly);
    EXPORT bool GetLocationOnly() const;

    EXPORT void SetDisplayLevel(const int32_t displayLevel);
    EXPORT int32_t GetDisplayLevel() const;

    EXPORT void SetAlbumLocalIdentifier(const std::string &localIdentifier);
    EXPORT const std::string& GetAlbumLocalIdentifier() const;

    EXPORT static bool IsUserPhotoAlbum(const PhotoAlbumType albumType, const PhotoAlbumSubType albumSubType);
    EXPORT static bool CheckPhotoAlbumType(const PhotoAlbumType albumType);
    EXPORT static bool CheckPhotoAlbumSubType(const PhotoAlbumSubType albumSubType);
    EXPORT static bool IsSourceAlbum(const PhotoAlbumType albumType, const PhotoAlbumSubType albumSubType);

private:
    int32_t albumId_;
    PhotoAlbumType type_;
    PhotoAlbumSubType subType_;
    std::string uri_;
    std::string albumName_;
    std::string coverUri_;
    int64_t dateModified_;
    int32_t count_;
    int32_t imageCount_;
    int32_t videoCount_;
    double latitude_;
    double longitude_;
    std::string relativePath_;
    int32_t displayLevel_;
    std::string localIdentifier_;

    ResultNapiType resultNapiType_;
    bool hiddenOnly_ = false;
    bool locationOnly_ = false;
};
} // namespace Media
} // namespace OHOS

#endif  // INTERFACES_INNERKITS_NATIVE_INCLUDE_PHOTO_ALBUM_ASSET_H_
