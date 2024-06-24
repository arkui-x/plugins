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

#ifndef FRAMEWORKS_INNERKITSIMPL_MEDIA_LIBRARY_INCLUDE_MEDIA_FILE_UTILS_H_
#define FRAMEWORKS_INNERKITSIMPL_MEDIA_LIBRARY_INCLUDE_MEDIA_FILE_UTILS_H_

#include <memory>
#include <string>
#include <unordered_set>

#include "userfile_manager_types.h"

namespace OHOS::Media {
#define EXPORT __attribute__ ((visibility ("default")))

/**
 * @brief Utility class for file operations
 *
 * @since 1.0
 * @version 1.0
 */
class MediaFileUtils {
public:
    EXPORT static std::string GetFileName(const std::string &filePath);
    EXPORT static std::string GetTitleFromDisplayName(const std::string &displayName);
    EXPORT static bool IsDirectory(const std::string &dirName, std::shared_ptr<int> errCodePtr = nullptr);
    EXPORT static int32_t CheckAlbumName(const std::string &albumName);
    EXPORT static int32_t CheckDisplayName(const std::string &displayName);
    EXPORT static int32_t CheckFileDisplayName(const std::string &displayName);
    EXPORT static void FormatRelativePath(std::string &relativePath);
    EXPORT static std::string GetNetworkIdFromUri(const std::string &uri);
    EXPORT static std::string UpdatePath(const std::string &path, const std::string &uri);
    EXPORT static MediaType GetMediaType(const std::string &filePath);
    EXPORT static std::string SplitByChar(const std::string &str, const char split);
    EXPORT static std::string GetExtensionFromPath(const std::string &path);
    EXPORT static std::string GetMediaTypeUri(MediaType mediaType);
    EXPORT static bool StartsWith(const std::string &str, const std::string &prefix);
    EXPORT static void UriAppendKeyValue(std::string &uri, const std::string &key, std::string value = "10");
    EXPORT static std::string GetExtraUri(const std::string &displayName, const std::string &path,
        const bool isNeedEncode = true);
    EXPORT static std::string GetUriByExtrConditions(const std::string &prefix, const std::string &fileId,
        const std::string &suffix = "");
    EXPORT static std::string Encode(const std::string &uri);
    EXPORT static std::string RemoveDocsFromRelativePath(const std::string &relativePath);

private:
    static int32_t CheckStringSize(const std::string &str, const size_t max);
};
} // namespace OHOS::Media

#endif // FRAMEWORKS_INNERKITSIMPL_MEDIA_LIBRARY_INCLUDE_MEDIA_FILE_UTILS_H_
