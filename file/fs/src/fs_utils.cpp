/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "fs_utils.h"

#include <cerrno>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <uv.h>

#include "common_func.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
constexpr int32_t DIR_DEFAULT_PERM = 0770;

int32_t CreateSingleDir(const std::string& pathStr, uv_fs_t * req)
{
    if (pathStr.empty() || req == nullptr) {
        HILOGE("Invalid input, path is empty or req is null");
        return ENOENT;
    }

    int32_t slashPos = pathStr.find_last_of('/');
    if (slashPos != std::string::npos) {
        std::string parentDir = pathStr.substr(0, slashPos);
        int32_t statResult = uv_fs_stat(nullptr, req, parentDir.c_str(), nullptr);
        if (statResult < 0) {
            HILOGE("No such file or directory, errno: %d", statResult);
            return statResult;
        }
    }

    int32_t ret = uv_fs_mkdir(nullptr, req, pathStr.c_str(), DIR_DEFAULT_PERM, nullptr);
    if (ret != 0) {
        HILOGE("Failed to create directory: %s, error: %d", pathStr.c_str(), ret);
        return ret;
    }
    return 0;
}

int32_t CreateMultiDir(const std::string& pathStr, uv_fs_t * req)
{
    if (pathStr.empty() || req == nullptr) {
        HILOGE("Invalid input, path is empty or req is null");
        return ENOENT;
    }

    std::string currentPath = "";
    std::istringstream pathStream(pathStr);
    std::string segment;

    while (std::getline(pathStream, segment, '/')) {
        if (!segment.empty()) {
            currentPath += "/" + segment;
            int32_t ret = uv_fs_mkdir(nullptr, req, currentPath.c_str(), DIR_DEFAULT_PERM, nullptr);
            if (ret < 0 && ret != UV_EEXIST) {
                HILOGE("Failed to create directory: %s", currentPath.c_str());
                return ret;
            }
        }
    }
    return 0;
}

int32_t CreateDir(const char* path, enum MakeDirectionMode mode)
{
    if (path == nullptr) {
        HILOGE("Invalid input, path is null");
        return EINVAL;
    }

    std::string pathStr(path);
    if (pathStr.empty()) {
        HILOGE("Invalid input, path is empty");
        return ENOENT;
    }

    auto req = std::make_unique<uv_fs_t>();
    if (!req) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }

    std::unique_ptr<uv_fs_t, void(*)(uv_fs_t*)> mkdirReq(req.release(),
        OHOS::FileManagement::ModuleFileIO::CommonFunc::fs_req_cleanup);

    return (mode == SINGLE) ?
        CreateSingleDir(pathStr, mkdirReq.get()) :
        CreateMultiDir(pathStr, mkdirReq.get());
}


int32_t Mkdirs(const char* path, enum MakeDirectionMode mode)
{
    if (path == nullptr) {
        HILOGE("Invalid input, path is null");
        return EINVAL;
    }
    return CreateDir(path, mode);
}

FileReader* ReaderIterator(const char* path)
{
    if (path == nullptr || strlen(path) == 0) {
        HILOGE("Invalid file path: path is null or empty");
        return nullptr;
    }

    FileReader* reader = new (std::nothrow) FileReader(path);
    if (reader == nullptr) {
        HILOGE("Failed to create FileReader object");
        return nullptr;
    }

    return reader;
}

Str* NextLine(FileReader *iter)
{
    if (iter == nullptr) {
        HILOGE("Invalid iterator: nullptr");
        return nullptr;
    }
    std::string line = iter->ReadLine();
    if (line.empty() && iter->GetFile().eof()) {
        HILOGI("At the end of file");
        return nullptr;
    }

    if (line.empty() && iter->GetFile().fail()) {
        HILOGE("Failed to read line from file");
        return nullptr;
    }

    std::unique_ptr<Str> item = std::make_unique<Str>();
    if (!item) {
        HILOGE("Failed to allocate memory for Str");
        return nullptr;
    }

    item->str = strdup(line.c_str());
    if (!item->str) {
        HILOGE("Failed to duplicate string");
        return nullptr;
    }

    item->len = line.length();
    return item.release();
}

void DropReaderIterator(FileReader *iter)
{
    if (iter == nullptr) {
        HILOGI("DropReaderIterator: iter is nullptr");
        return;
    }
    delete iter;
}

void StrFree(Str *str)
{
    if (str == nullptr) {
        HILOGE("StrFree: str is nullptr, nothing to free.");
        return;
    }

    if (str->str != nullptr) {
        free(const_cast<char*>(str->str));
        str->str = nullptr;
    }

    delete str;
}

std::string GetParentDirectory(const std::string& path)
{
    if (path.empty()) {
        return ".";
    }

    std::string normalizedPath = path;
    for (char& ch : normalizedPath) {
        if (ch == '\\') {
            ch = '/';
        }
    }

    while (!normalizedPath.empty() && normalizedPath.back() == '/') {
        normalizedPath.pop_back();
    }

    if (normalizedPath.empty()) {
        return path;
    }

    size_t lastSlashPos = path.find_last_of("/");
    if (lastSlashPos == std::string::npos) {
        return ".";
    }
    if (lastSlashPos == 0) {
        return "/";
    }
    return path.substr(0, lastSlashPos);
}

bool FileIsExist(const char* filePath)
{
    uv_fs_t req;
    int result = uv_fs_stat(nullptr, &req, filePath, nullptr);
    if (result < 0) {
        if (result != UV_ENOENT) {
            HILOGE("Failed to get file stat: %{public}s", uv_strerror(result));
        }
        uv_fs_req_cleanup(&req);
        return false;
    }
    uv_fs_req_cleanup(&req);
    return true;
}

bool RemoveFile(const char* filePath)
{
    uv_fs_t req;
    int result = uv_fs_unlink(nullptr, &req, filePath, nullptr);
    if (result < 0) {
        HILOGE("Faile to remove file: %{public}s", uv_strerror(result));
        uv_fs_req_cleanup(&req);
        return false;
    }

    uv_fs_req_cleanup(&req);
    return true;
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS