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

    void FilterClear()
    {
        this->suffix_.clear();
        this->displayName_.clear();
        this->mimeType_.clear();
        this->fileSizeOver_ = 0;
        this->lastModifiedAfter_ = 0;
        this->excludeMedia_ = false;
        this->hasFilter_ = false;
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

class FileFilterBuilder {
public:
    FileFilterBuilder() = default;
    ~FileFilterBuilder() = default;

    FileFilterBuilder& SetSuffix(const std::vector<std::string> &suffix)
    {
        fileFilter_.SetSuffix(suffix);
        return *this;
    }

    FileFilterBuilder& SetDisplayName(const std::vector<std::string> &displayName)
    {
        fileFilter_.SetDisplayName(displayName);
        return *this;
    }

    FileFilterBuilder& SetMimeType(const std::vector<std::string> &mimeType)
    {
        fileFilter_.SetMimeType(mimeType);
        return *this;
    }

    FileFilterBuilder& SetFileSizeOver(const int64_t &fileSizeOver)
    {
        fileFilter_.SetFileSizeOver(fileSizeOver);
        return *this;
    }

    FileFilterBuilder& SetLastModifiedAfter(const double &lastModifiedAfter)
    {
        fileFilter_.SetLastModifiedAfter(lastModifiedAfter);
        return *this;
    }

    FileFilterBuilder& SetExcludeMedia(const bool &excludeMedia)
    {
        fileFilter_.SetExcludeMedia(excludeMedia);
        return *this;
    }

    FileFilterBuilder& SetHasFilter(const bool &hasFilter)
    {
        fileFilter_.SetHasFilter(hasFilter);
        return *this;
    }

    FileFilter Build()
    {
        return fileFilter_;
    }

private:
    FileFilter fileFilter_;
};
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // FILE_FILTER_H