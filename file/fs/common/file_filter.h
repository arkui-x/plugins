/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#ifndef FILE_FILTER_H
#define FILE_FILTER_H

#include <string>
#include <vector>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
class FileFilter {
public:
    FileFilter() = default;
    ~FileFilter() = default;

    FileFilter(std::vector<std::string> suffix, std::vector<std::string> displayName,
        std::vector<std::string> mimeType, int64_t fileSizeOver,
        double lastModifiedAfter, bool excludeMedia, bool hasFilter)
        : suffix_(suffix), displayName_(displayName), mimeType_(mimeType), fileSizeOver_(fileSizeOver),
        lastModifiedAfter_(lastModifiedAfter), excludeMedia_(excludeMedia), hasFilter_(hasFilter) {}
    explicit FileFilter(std::vector<std::string> suffix): suffix_(suffix) {}
    FileFilter(const FileFilter &filter) = default;
    FileFilter &operator=(const FileFilter& filter) = default;

    void SetSuffix(const std::vector<std::string> &suffix)
    {
        suffix_ = suffix;
    }

    std::vector<std::string> GetSuffix() const
    {
        return suffix_;
    }

    void SetDisplayName(const std::vector<std::string> &displayName)
    {
        displayName_ = displayName;
    }

    std::vector<std::string> GetDisplayName() const
    {
        return displayName_;
    }

    void SetMimeType(const std::vector<std::string> &mimeType)
    {
        mimeType_ = mimeType;
    }

    std::vector<std::string> GetMimeType() const
    {
        return mimeType_;
    }

    void SetFileSizeOver(const int64_t &fileSizeOver)
    {
        fileSizeOver_ = fileSizeOver;
    }

    int64_t GetFileSizeOver() const
    {
        return fileSizeOver_;
    }

    void SetLastModifiedAfter(const double &lastModifiedAfter)
    {
        lastModifiedAfter_ = lastModifiedAfter;
    }

    double GetLastModifiedAfter() const
    {
        return lastModifiedAfter_;
    }

    void SetExcludeMedia(const bool &excludeMedia)
    {
        excludeMedia_ = excludeMedia;
    }

    bool GetExcludeMedia() const
    {
        return excludeMedia_;
    }

    void SetHasFilter(const bool &hasFilter)
    {
        hasFilter_ = hasFilter;
    }

    bool GetHasFilter() const
    {
        return hasFilter_;
    }

private:
    std::vector<std::string> suffix_ = std::vector<std::string>();
    std::vector<std::string> displayName_ = std::vector<std::string>();
    std::vector<std::string> mimeType_ = std::vector<std::string>();
    int64_t fileSizeOver_ = 0;
    double lastModifiedAfter_ = 0;
    bool excludeMedia_ = false;
    bool hasFilter_ = false;
};
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // FILE_FILTER_H