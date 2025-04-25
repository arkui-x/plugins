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

#ifndef PLUGINS_FS_UTILS_H
#define PLUGINS_FS_UTILS_H

#ifdef __cplusplus
#if __cplusplus
#include <cerrno>
#include <iostream>
#include <fstream>

#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {

class FileReader {
private:
    std::unique_ptr<std::ifstream> file;
public:
    explicit FileReader(const char *path)
    {
        if (path == nullptr) {
            HILOGE("Invalid file path!");
            return;
        }
        file = std::make_unique<std::ifstream>(path);
        if (!file->is_open()) {
            HILOGE("Failed to open file error: %d", errno);
            file.reset();
        }
    }
    ~FileReader()
    {
        if (file && file->is_open()) {
            file->close();
        }
    }
    std::string ReadLine()
    {
        std::string line = "";
        if (std::getline(*file, line)) {
            return line;
        }
        return "";
    }
    std::ifstream& GetFile()
    {
        return *file;
    }
};

extern "C" {
#endif
#endif

enum SeekPos {
    START,
    CURRENT,
    END
};

enum MakeDirectionMode {
    SINGLE,
    MULTIPLE
};

typedef struct {
    const char* str;
    unsigned int len;
} Str;

FileReader* ReaderIterator(const char* path);

void DropReaderIterator(FileReader* iter);

Str* NextLine(FileReader* iter);

long long int Lseek(int fd, long long offset, enum SeekPos pos);

int32_t Mkdirs(const char* path, enum MakeDirectionMode mode);

Str* GetParent(int fd);

Str* CutFileName(const char* path, size_t size);

void StrFree(Str* str);

std::string GetParentDirectory(const std::string& path);

bool FileIsExist(const char* filePath);

bool RemoveFile(const char* filePath);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
}
}
}
#endif //PLUGINS_FS_UTILS_H