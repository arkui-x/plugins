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
#include "file_utils.h"

#include <cinttypes>
#include <sys/stat.h>
#include <unistd.h>

#include "securec.h"
#include "sensors_errors.h"

#undef LOG_TAG
#define LOG_TAG "MiscdeviceFileUtils"

namespace OHOS {
namespace Sensors {
namespace {
const std::string CONFIG_DIR = "/vendor/etc/vibrator/";
constexpr int32_t FILE_SIZE_MAX = 0x5000;
constexpr int64_t READ_DATA_BUFF_SIZE = 256;
constexpr int32_t INVALID_FILE_SIZE = -1;
constexpr int32_t FILE_PATH_MAX = 1024;
} // namespace

std::string ReadJsonFile(const std::string &filePath)
{
    if (filePath.empty()) {
        MISC_HILOGE("Path is empty");
        return {};
    }
    char realPath[PATH_MAX] = {};
    if (realpath(filePath.c_str(), realPath) == nullptr) {
        MISC_HILOGE("Path is error, %{public}d", errno);
        return {};
    }
    if (!CheckFileDir(realPath, CONFIG_DIR)) {
        MISC_HILOGE("File dir is invalid");
        return {};
    }
    if (!CheckFileExtendName(realPath, "json")) {
        MISC_HILOGE("Unable to parse files other than json format");
        return {};
    }
    if (!IsFileExists(realPath)) {
        MISC_HILOGE("File not exist");
        return {};
    }
    if (!CheckFileSize(realPath)) {
        MISC_HILOGE("File size out of read range");
        return {};
    }
    FILE *fp = fopen(realPath, "r");
    CHKPS(fp);
    std::string dataStr;
    char buf[READ_DATA_BUFF_SIZE] = { '\0' };
    while (fgets(buf, sizeof(buf), fp) != nullptr) {
        dataStr += buf;
    }
    if (fclose(fp) != 0) {
        MISC_HILOGW("Close file failed, errno:%{public}d", errno);
    }
    return dataStr;
}

int32_t GetFileSize(const std::string &filePath)
{
    struct stat statbuf = { 0 };
    if (stat(filePath.c_str(), &statbuf) != 0) {
        MISC_HILOGE("Get file size error");
        return INVALID_FILE_SIZE;
    }
    return statbuf.st_size;
}

int64_t GetFileSize(int32_t fd)
{
    if (fd < 0) {
        MISC_HILOGE("fd is invalid, fd:%{public}d", fd);
        return INVALID_FILE_SIZE;
    }
    struct stat statbuf = { 0 };
    if (fstat(fd, &statbuf) != 0) {
        MISC_HILOGE("fstat error, errno:%{public}d", errno);
        return INVALID_FILE_SIZE;
    }
    return statbuf.st_size;
}

int32_t GetFileName(const int32_t &fd, std::string &fileName)
{
    if (fd < 0) {
        MISC_HILOGE("fd is invalid, fd:%{public}d", fd);
        return ERROR;
    }
    char buf[FILE_PATH_MAX] = {'\0'};
    char filePath[FILE_PATH_MAX] = {'\0'};

    int ret = snprintf_s(buf, sizeof(buf), (sizeof(buf) - 1), "/proc/self/fd/%d", fd);
    if (ret < 0) {
        MISC_HILOGE("snprintf failed with %{public}d", errno);
        return ERROR;
    }

    ret = readlink(buf, filePath, FILE_PATH_MAX);
    if (ret < 0 || ret >= FILE_PATH_MAX) {
        MISC_HILOGE("readlink failed with %{public}d", errno);
        return ERROR;
    }

    fileName = filePath;
    std::size_t firstSlash = fileName.rfind("/");
    if (firstSlash == fileName.npos) {
        MISC_HILOGE("Get error path");
        return ERROR;
    }
    fileName = fileName.substr(firstSlash + 1, fileName.size() - firstSlash);
    return SUCCESS;
}

int32_t GetFileExtName(const int32_t &fd, std::string &extName)
{
    if (fd < 0) {
        MISC_HILOGE("fd is invalid, fd:%{public}d", fd);
        return ERROR;
    }
    std::string fileName = "";
    if (GetFileName(fd, fileName) == ERROR) {
        MISC_HILOGE("GetFileName failed");
        return ERROR;
    }
    extName = fileName.substr(fileName.find_last_of(".") + 1);
    return SUCCESS;
}

bool CheckFileDir(const std::string &filePath, const std::string &dir)
{
    if (filePath.compare(0, CONFIG_DIR.size(), CONFIG_DIR) != 0) {
        MISC_HILOGE("filePath dir is invalid");
        return false;
    }
    return true;
}

bool CheckFileSize(const std::string &filePath)
{
    int32_t fileSize = GetFileSize(filePath);
    if ((fileSize <= 0) || (fileSize > FILE_SIZE_MAX)) {
        MISC_HILOGE("File size out of read range");
        return false;
    }
    return true;
}

bool CheckFileExtendName(const std::string &filePath, const std::string &checkExtension)
{
    std::string::size_type pos = filePath.find_last_of('.');
    if (pos == std::string::npos) {
        MISC_HILOGE("File is not find extension");
        return false;
    }
    return (filePath.substr(pos + 1, filePath.npos) == checkExtension);
}

bool IsFileExists(const std::string &fileName)
{
    return (access(fileName.c_str(), F_OK) == 0);
}

std::string ReadFd(const RawFileDescriptor &rawFd)
{
    if (rawFd.fd < 0) {
        MISC_HILOGE("fd is invalid, fd:%{public}d", rawFd.fd);
        return {};
    }
    int64_t fdSize = GetFileSize(rawFd.fd);
    if ((rawFd.offset < 0) || (rawFd.offset > fdSize)) {
        MISC_HILOGE("offset is invalid, offset:%{public}" PRId64, rawFd.offset);
        return {};
    }
    if ((rawFd.length <= 0) || (rawFd.length > fdSize - rawFd.offset)) {
        MISC_HILOGE("length is invalid, length:%{public}" PRId64, rawFd.length);
        return {};
    }
    int dupFd = dup(rawFd.fd);
    if (dupFd < 0) {
        MISC_HILOGE("dup fd failed, fd:%{public}d, errno:%{public}d", rawFd.fd, errno);
        return {};
    }
    FILE *fp = fdopen(dupFd, "r");
    if (fp == nullptr) {
        MISC_HILOGE("fdopen failed, fd:%{public}d, errno:%{public}d", dupFd, errno);
        close(dupFd);
        return {};
    }
    if (fseek(fp, rawFd.offset, SEEK_SET) != 0) {
        MISC_HILOGE("fseek failed, errno:%{public}d", errno);
        if (fclose(fp) != 0) {
            MISC_HILOGW("Close file failed, errno:%{public}d", errno);
        }
        return {};
    }
    std::string dataStr;
    char buf[READ_DATA_BUFF_SIZE] = { '\0' };
    int64_t alreadyRead = 0;
    while (alreadyRead < rawFd.length) {
        int64_t onceRead = std::min(rawFd.length - alreadyRead, READ_DATA_BUFF_SIZE - 1);
        fgets(buf, onceRead + 1, fp);
        dataStr += buf;
        alreadyRead = ftell(fp) - rawFd.offset;
    }
    if (fclose(fp) != 0) {
        MISC_HILOGW("Close file failed, errno:%{public}d", errno);
    }
    return dataStr;
}

std::string GetFileSuffix(int32_t fd)
{
    std::string fdPath = "/proc/self/fd/" + std::to_string(fd);
    char filePath[FILE_PATH_MAX + 1] = { '\0' };
    ssize_t ret = readlink(fdPath.c_str(), filePath, FILE_PATH_MAX);
    if (ret < 0 || ret > FILE_PATH_MAX) {
        MISC_HILOGE("Readlink failed, errno:%{public}d", errno);
        return {};
    }
    std::string fileAbsolutePath(filePath);
    size_t pos = fileAbsolutePath.find_last_of('.');
    if (pos == std::string::npos) {
        MISC_HILOGE("File suffix is invalid, fileAbsolutePath:%{public}s", fileAbsolutePath.c_str());
        return {};
    }
    return fileAbsolutePath.substr(pos + 1);
}
} // namespace Sensors
} // namespace OHOS
