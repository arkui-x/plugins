/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <sys/stat.h>
#include <fcntl.h>
#include <thread>
#include <cstdio>

#include "upload_task.h"

namespace OHOS::Plugin::Request::Upload {
ObtainFile::ObtainFile()
{
}
ObtainFile::~ObtainFile()
{
}

uint32_t ObtainFile::GetFile(FILE **file, std::string &fileUri,
    unsigned int &fileSize)
{
    uint32_t ret = UPLOAD_OK;

    if (!fileUri.empty()) {
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "GetStorageFile");
        ret = GetStorageFile(file, fileUri, fileSize);
    } else {
        UPLOAD_HILOGE(UPLOAD_MODULE_FRAMEWORK, "wrong path");
        ret = UPLOAD_ERRORCODE_UNSUPPORT_URI;
        *file = nullptr;
        fileSize = 0;
    }

    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK,
        "ObtainFile::GetFile, ret : %{public}d, size : %{public}d, pf : %{public}p", ret, fileSize, *file);
    return ret;
}

uint32_t ObtainFile::GetStorageFile(FILE **file, std::string &fileUri, uint32_t &fileSize)
{
    uint32_t ret = UPLOAD_OK;
    std::string filePath = fileUri;
    FILE *filePtr = nullptr;
    int32_t fileLength = 0;

    do {
        if (filePath.size() == 0) {
            UPLOAD_HILOGE(UPLOAD_MODULE_FRAMEWORK, "ObtainFile::GetStorageFile, storage to cache error");
            ret = UPLOAD_ERRORCODE_GET_FILE_ERROR;
            break;
        }
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "ObtainFile::GetStorageFile, storage file path = [%{public}s].",
            filePath.c_str());
        filePtr = fopen(filePath.c_str(), "r");
        if (filePtr == nullptr) {
            UPLOAD_HILOGE(UPLOAD_MODULE_FRAMEWORK, "ObtainFile::GetStorageFile, open file error");
            UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "ObtainFile::GetStorageFile, error info : %{public}d.", errno);
            ret = UPLOAD_ERRORCODE_GET_FILE_ERROR;
            break;
        }
        (void)fseek(filePtr, 0, SEEK_END);
        fileLength = ftell(filePtr);
        (void)fseek(filePtr, 0, SEEK_SET);
    } while (0);

    *file = filePtr;
    fileSize = fileLength;
    return ret;
}
} // namespace OHOS::Plugin::Request::Upload