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
#define MLOG_TAG "PhotoAlbum"

#include "photo_album.h"

#include "medialibrary_type_const.h"

using namespace std;

namespace OHOS {
namespace Media {
PhotoAlbum::PhotoAlbum()
{
    albumId_ = DEFAULT_ALBUM_ID;
    type_ = USER;
    subType_ = USER_GENERIC;
    count_ = DEFAULT_COUNT;
}

PhotoAlbum::~PhotoAlbum() = default;

void PhotoAlbum::SetAlbumId(const int32_t albumId)
{
    albumId_ = albumId;
}

int32_t PhotoAlbum::GetAlbumId() const
{
    return albumId_;
}

void PhotoAlbum::SetPhotoAlbumType(const PhotoAlbumType type)
{
    type_ = type;
}

PhotoAlbumType PhotoAlbum::GetPhotoAlbumType() const
{
    return type_;
}

void PhotoAlbum::SetPhotoAlbumSubType(const PhotoAlbumSubType subType)
{
    subType_ = subType;
}

PhotoAlbumSubType PhotoAlbum::GetPhotoAlbumSubType() const
{
    return subType_;
}

void PhotoAlbum::SetAlbumUri(const string &uri)
{
    uri_ = uri;
}

const string& PhotoAlbum::GetAlbumUri() const
{
    return uri_;
}

void PhotoAlbum::SetAlbumName(const string &albumName)
{
    albumName_ = albumName;
}

const string& PhotoAlbum::GetAlbumName() const
{
    return albumName_;
}

void PhotoAlbum::SetCoverUri(const string &coverUri)
{
    coverUri_ = coverUri;
}

const string& PhotoAlbum::GetCoverUri() const
{
    return coverUri_;
}

void PhotoAlbum::SetDateModified(const int64_t dateModified)
{
    dateModified_ = dateModified;
}

int64_t PhotoAlbum::GetDateModified() const
{
    return dateModified_;
}

void PhotoAlbum::SetCount(const int32_t count)
{
    count_ = count;
}

int32_t PhotoAlbum::GetCount() const
{
    return count_;
}

void PhotoAlbum::SetImageCount(const int32_t count)
{
    imageCount_ = count;
}

int32_t PhotoAlbum::GetImageCount() const
{
    return imageCount_;
}

void PhotoAlbum::SetVideoCount(const int32_t count)
{
    videoCount_ = count;
}

int32_t PhotoAlbum::GetVideoCount() const
{
    return videoCount_;
}

void PhotoAlbum::SetLatitude(const double latitude)
{
    latitude_ = latitude;
}

double PhotoAlbum::GetLatitude() const
{
    return latitude_;
}

void PhotoAlbum::SetLongitude(const double longitude)
{
    longitude_ = longitude;
}

double PhotoAlbum::GetLongitude() const
{
    return longitude_;
}

void PhotoAlbum::SetRelativePath(const string &relativePath)
{
    relativePath_ = relativePath;
}

const string& PhotoAlbum::GetRelativePath() const
{
    return relativePath_;
}

void PhotoAlbum::SetResultNapiType(const ResultNapiType resultNapiType)
{
    resultNapiType_ = resultNapiType;
}

ResultNapiType PhotoAlbum::GetResultNapiType() const
{
    return resultNapiType_;
}

void PhotoAlbum::SetHiddenOnly(const bool hiddenOnly)
{
    hiddenOnly_ = hiddenOnly;
}

bool PhotoAlbum::GetHiddenOnly() const
{
    return hiddenOnly_;
}

void PhotoAlbum::SetLocationOnly(const bool locationOnly)
{
    locationOnly_ = locationOnly;
}

bool PhotoAlbum::GetLocationOnly() const
{
    return locationOnly_;
}

void PhotoAlbum::SetDisplayLevel(const int32_t displayLevel)
{
    displayLevel_ = displayLevel;
}

int32_t PhotoAlbum::GetDisplayLevel() const
{
    return displayLevel_;
}

void PhotoAlbum::SetAlbumLocalIdentifier(const string &localIdentifier)
{
    localIdentifier_ = localIdentifier;
}

const string& PhotoAlbum::GetAlbumLocalIdentifier() const
{
    return localIdentifier_;
}

bool PhotoAlbum::IsUserPhotoAlbum(const PhotoAlbumType albumType, const PhotoAlbumSubType albumSubType)
{
    return (albumType == PhotoAlbumType::USER) && (albumSubType == PhotoAlbumSubType::USER_GENERIC);
}

bool PhotoAlbum::CheckPhotoAlbumType(const PhotoAlbumType albumType)
{
    return (albumType == PhotoAlbumType::USER) || (albumType == PhotoAlbumType::SYSTEM) ||
        (albumType == PhotoAlbumType::SMART) || (albumType == PhotoAlbumType::SOURCE);
}

bool PhotoAlbum::CheckPhotoAlbumSubType(const PhotoAlbumSubType albumSubType)
{
    return (albumSubType == PhotoAlbumSubType::USER_GENERIC) || (albumSubType == PhotoAlbumSubType::ANY) ||
        ((albumSubType >= PhotoAlbumSubType::SYSTEM_START) && (albumSubType <= PhotoAlbumSubType::SYSTEM_END)) ||
        ((albumSubType >= PhotoAlbumSubType::ANALYSIS_START) && (albumSubType <= PhotoAlbumSubType::ANALYSIS_END)) ||
        (albumSubType == PhotoAlbumSubType::SOURCE_GENERIC);
}
}  // namespace Media
}  // namespace OHOS
