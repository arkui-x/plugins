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

#ifndef INTERFACES_INNERKITS_NATIVE_INCLUDE_FILE_ASSET_H_
#define INTERFACES_INNERKITS_NATIVE_INCLUDE_FILE_ASSET_H_

#include <memory>
#include <mutex>
#include <string>
#include <variant>
#include <unordered_map>
#include "medialibrary_type_const.h"

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))

constexpr int MEMBER_TYPE_INT32 = 0;
constexpr int MEMBER_TYPE_INT64 = 1;
constexpr int MEMBER_TYPE_STRING = 2;
constexpr int MEMBER_TYPE_DOUBLE = 3;

constexpr int OPEN_TYPE_READONLY = 0;
constexpr int OPEN_TYPE_WRITE = 1;

/**
 * @brief Class for filling all file asset parameters
 *
 * @since 1.0
 * @version 1.0
 */
class FileAsset {
public:
    EXPORT FileAsset();
    EXPORT virtual ~FileAsset() = default;

    EXPORT int32_t GetId() const;
    EXPORT void SetId(int32_t id);

    EXPORT int32_t GetCount() const;
    EXPORT void SetCount(int32_t count);

    EXPORT const std::string &GetUri() const;
    EXPORT void SetUri(const std::string &uri);

    EXPORT const std::string &GetPath() const;
    EXPORT void SetPath(const std::string &path);

    EXPORT const std::string &GetRelativePath() const;
    EXPORT void SetRelativePath(const std::string &relativePath);

    EXPORT const std::string &GetMimeType() const;
    EXPORT void SetMimeType(const std::string &mimeType);

    EXPORT MediaType GetMediaType() const;
    EXPORT void SetMediaType(MediaType mediaType);

    EXPORT const std::string &GetDisplayName() const;
    EXPORT void SetDisplayName(const std::string &displayName);

    EXPORT int64_t GetSize() const;
    EXPORT void SetSize(int64_t size);

    EXPORT int64_t GetDateAdded() const;
    EXPORT void SetDateAdded(int64_t dataAdded);

    EXPORT int64_t GetDateModified() const;
    EXPORT void SetDateModified(int64_t dateModified);

    EXPORT const std::string &GetTitle() const;
    EXPORT void SetTitle(const std::string &title);
    EXPORT int32_t GetPosition() const;
    EXPORT void SetPosition(int32_t position);
    EXPORT int32_t GetWidth() const;
    EXPORT void SetWidth(int32_t width);
    EXPORT int32_t GetHeight() const;
    EXPORT void SetHeight(int32_t height);
    EXPORT int32_t GetDuration() const;
    EXPORT void SetDuration(int32_t duration);
    EXPORT int32_t GetOrientation() const;
    EXPORT void SetOrientation(int32_t orientation);
    EXPORT int32_t GetAlbumId() const;
    EXPORT void SetAlbumId(int32_t albumId);
    EXPORT const std::string &GetAlbumName() const;
    EXPORT void SetAlbumName(const std::string &albumName);
    EXPORT const std::string &GetAlbumUri() const;
    EXPORT void SetAlbumUri(const std::string &albumUri);
    EXPORT int64_t GetDateTaken() const;
    EXPORT void SetDateTaken(int64_t dataTaken);

    EXPORT int64_t GetTimePending() const;
    EXPORT void SetTimePending(int64_t timePending);

    EXPORT bool IsFavorite() const;
    EXPORT void SetFavorite(bool isFavorite);

    EXPORT void SetLatitude(int64_t latitude);
    EXPORT int64_t GetLatitude();
    EXPORT void SetLongitude(int64_t longitude);
    EXPORT int64_t GetLongitude();

    EXPORT const std::string &GetSelfId() const;
    EXPORT void SetSelfId(const std::string &selfId);
    EXPORT int32_t GetIsTrash() const;
    EXPORT void SetIsTrash(int32_t isTrash);
    EXPORT const std::string GetOwnerPackage() const;
    EXPORT void SetOwnerPackage(const std::string &ownerPackage);
    EXPORT const std::string GetPackageName() const;
    EXPORT void SetPackageName(const std::string &packageName);
    EXPORT const std::string &GetRecyclePath() const;
    EXPORT void SetRecyclePath(const std::string &recyclePath);
    EXPORT ResultNapiType GetResultNapiType() const;
    EXPORT void SetResultNapiType(const ResultNapiType type);
    EXPORT int32_t GetPhotoSubType() const;
    EXPORT void SetPhotoSubType(int32_t photoSubType);
    EXPORT int32_t GetPhotoIndex() const;
    EXPORT const std::string &GetCameraShotKey() const;
    EXPORT void SetCameraShotKey(const std::string &cameraShotKey);
    EXPORT bool IsHidden() const;
    EXPORT void SetHidden(bool isHidden);
    EXPORT std::unordered_map<std::string, std::variant<int32_t, int64_t, std::string, double>> &GetMemberMap();
    EXPORT std::variant<int32_t, int64_t, std::string, double> &GetMemberValue(const std::string &name);
    EXPORT void SetResultTypeMap(const std::string &colName, ResultSetDataType type);
    EXPORT const std::string &GetFilePath() const;
    EXPORT void SetFilePath(const std::string &filePath);

    EXPORT int64_t GetPhotoEditTime() const;
    EXPORT void SetPhotoEditTime(int64_t photoEditTime);

private:
    const std::string &GetStrMember(const std::string &name) const;
    int32_t GetInt32Member(const std::string &name) const;
    int64_t GetInt64Member(const std::string &name) const;

    std::string albumUri_;
    ResultNapiType resultNapiType_;
    std::unordered_map<std::string, std::variant<int32_t, int64_t, std::string, double>> member_;
    std::mutex openStatusMapMutex_;
    std::shared_ptr<std::unordered_map<int32_t, int32_t>> openStatusMap_;
    std::unordered_map<std::string, ResultSetDataType> resultTypeMap_;
};
} // namespace Media
} // namespace OHOS

#endif  // INTERFACES_INNERKITS_NATIVE_INCLUDE_FILE_ASSET_H_
