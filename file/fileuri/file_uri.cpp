/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "file_uri.h"

#include <cerrno>
#include <sys/stat.h>

#include "common_func.h"
#include "log.h"
#include "sandbox_helper.h"
#include "uri.h"

namespace OHOS {
namespace AppFileService {
namespace ModuleFileUri {
const std::string BACKSLASH = "/";
const std::string FILE_SCHEME_PREFIX = "file://";

static std::string GetPathPartFromUri(const std::string& uri)
{
    if (uri.find(FILE_SCHEME_PREFIX) == 0) {
        return uri.substr(FILE_SCHEME_PREFIX.length());
    }
    return uri;
}

static std::string GetDirectoryPath(const std::string& path)
{
    if (path.empty()) {
        return "";
    }
    size_t pos = path.find_last_of(BACKSLASH);
    if (pos == std::string::npos) {
        return "";
    }
    if (pos == 0) {
        return BACKSLASH;
    }
    return path.substr(0, pos);
}

std::string FileUri::GetName()
{
    std::string realPath = GetRealPath();
    if (realPath.empty()) {
        LOGE("GetRealPath failed, realPath is empty");
        return "";
    }
    size_t posLast = realPath.find_last_of(BACKSLASH);
    if (posLast == std::string::npos || posLast == (realPath.size() - 1)) {
        LOGE("Invalid path format, cannot get name from path");
        return "";
    }
    return realPath.substr(posLast + 1);
}

std::string FileUri::GetPath()
{
    std::string encodedPath = uri_.GetPath();
    if (encodedPath.empty()) {
        encodedPath = GetPathPartFromUri(uri_.ToString());
        if (encodedPath.empty() || encodedPath.find(BACKSLASH) == std::string::npos) {
            LOGE("Encoded path is invalid, path is empty or has no path separator");
            return "";
        }
    }

    std::string decodedPath = SandboxHelper::Decode(encodedPath);
    if (!SandboxHelper::IsValidPath(decodedPath)) {
        LOGE("Invalid uri path.");
        return "";
    }
    return decodedPath;
}

std::string FileUri::GetRealPath()
{
    return GetPath();
}

std::string FileUri::ToString()
{
    return uri_.ToString();
}

std::string FileUri::GetFullDirectoryUri()
{
    std::string realPath = GetRealPath();
    if (realPath.empty()) {
        LOGE("GetRealPath failed, realPath is empty");
        return "";
    }

    struct stat fileInfo;
    if (stat(realPath.c_str(), &fileInfo) != 0) {
        LOGE("Get file info failed, errno: %{public}d", errno);
        return "";
    }

    if (S_ISDIR(fileInfo.st_mode)) {
        return CommonFunc::GetUriFromPath(realPath);
    }
    if (S_ISREG(fileInfo.st_mode)) {
        return CommonFunc::GetUriFromPath(GetDirectoryPath(realPath));
    }
    LOGE("File is neither a directory nor a regular file");
    return "";
}

FileUri::FileUri(const std::string& uriOrPath)
    : uri_((uriOrPath.find(FILE_SCHEME_PREFIX) == 0) ? uriOrPath : CommonFunc::GetUriFromPath(uriOrPath))
{}

bool FileUri::CheckUriFormat(const std::string& uri)
{
    if (uri.find(FILE_SCHEME_PREFIX) != 0) {
        LOGE("URI is missing file:// scheme");
        return false;
    }
    if (uri.size() == FILE_SCHEME_PREFIX.size()) {
        LOGE("URI is invalid, path part is empty");
        return false;
    }
    if (!SandboxHelper::IsValidPath(uri)) {
        LOGE("uri is invalid path, The uri contains '../' characters");
        return false;
    }
    return true;
}

} // namespace ModuleFileUri
} // namespace AppFileService
} // namespace OHOS