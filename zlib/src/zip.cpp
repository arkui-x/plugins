/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "zip.h"

#include <fcntl.h>
#include <list>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>

#include "appexecfwk_errors.h"
#include "bundle_errors.h"
#include "directory_ex.h"
#include "file_path.h"
#include "log.h"
#include "plugin_utils_inner.h"
#include "zip_internal.h"
#include "zip_reader.h"
#include "zip_writer.h"

using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
namespace LIBZIP {
namespace {
using FilterCallback = std::function<bool(const FilePath&)>;
using DirectoryCreator = std::function<bool(FilePath&, FilePath&)>;
using WriterFactory = std::function<std::unique_ptr<WriterDelegate>(FilePath&, FilePath&)>;

const std::string SEPARATOR = "/";
const char HIDDEN_SEPARATOR = '.';
const std::string ZIP = ".zip";

struct UnzipParam {
    FilterCallback filterCB = nullptr;
    bool logSkippedFiles = false;
};
bool IsHiddenFile(const FilePath& filePath)
{
    FilePath localFilePath = filePath;
    if (!localFilePath.Value().empty()) {
        return localFilePath.Value().c_str()[0] == HIDDEN_SEPARATOR;
    } else {
        return false;
    }
}
bool ExcludeNoFilesFilter(const FilePath& filePath)
{
    return true;
}

bool ExcludeHiddenFilesFilter(const FilePath& filePath)
{
    return !IsHiddenFile(filePath);
}

std::vector<FileAccessor::DirectoryContentEntry> ListDirectoryContent(const FilePath& filePath, bool& isSuccess)
{
    FilePath curPath = filePath;
    std::vector<FileAccessor::DirectoryContentEntry> fileDirectoryVector;
    std::vector<std::string> filelist;
    isSuccess = FilePath::GetZipAllDirFiles(curPath.Value(), filelist);
    if (isSuccess) {
        LOGI("ListDirectoryContent filelist =====filelist.size=%{public}zu====", filelist.size());
        for (size_t i = 0; i < filelist.size(); i++) {
            std::string str(filelist[i]);
            if (!str.empty()) {
                fileDirectoryVector.push_back(
                    FileAccessor::DirectoryContentEntry(FilePath(str), FilePath::DirectoryExists(FilePath(str))));
            }
        }
    }
    return fileDirectoryVector;
}

// Creates a directory at |extractDir|/|entryPath|, including any parents.
bool CreateDirectory(FilePath& extractDir, FilePath& entryPath)
{
    std::string path = extractDir.Value();
    if (EndsWith(path, SEPARATOR)) {
        return FilePath::CreateDirectory(FilePath(extractDir.Value() + entryPath.Value()));
    } else {
        return FilePath::CreateDirectory(FilePath(extractDir.Value() + "/" + entryPath.Value()));
    }
}

// Creates a WriterDelegate that can write a file at |extractDir|/|entryPath|.
std::unique_ptr<WriterDelegate> CreateFilePathWriterDelegate(FilePath& extractDir, FilePath entryPath)
{
    if (EndsWith(extractDir.Value(), SEPARATOR)) {
        return std::make_unique<FilePathWriterDelegate>(FilePath(extractDir.Value() + entryPath.Value()));
    } else {
        return std::make_unique<FilePathWriterDelegate>(FilePath(extractDir.Value() + "/" + entryPath.Value()));
    }
}
} // namespace

ZipParams::ZipParams(const FilePath& srcDir, const FilePath& destFile) : srcDir_(srcDir), destFile_(destFile) {}

// Does not take ownership of |fd|.
ZipParams::ZipParams(const FilePath& srcDir, int destFd) : srcDir_(srcDir), destFd_(destFd) {}

FilePath FilePathEndIsSeparator(FilePath paramPath)
{
    bool endIsSeparator = EndsWith(paramPath.Value(), SEPARATOR);
    if (FilePath::IsDir(paramPath)) {
        if (!endIsSeparator) {
            paramPath.AppendSeparator();
        }
    }
    return paramPath;
}

bool Zip(const ZipParams& params, const OPTIONS& options)
{
    const std::vector<FilePath>* filesToAdd = &params.GetFilesTozip();
    std::vector<FilePath> allRelativeFiles;
    FilePath paramPath = FilePathEndIsSeparator(params.SrcDir());
    if (filesToAdd->empty()) {
        filesToAdd = &allRelativeFiles;
        std::list<FileAccessor::DirectoryContentEntry> entries;
        if (EndsWith(paramPath.Value(), SEPARATOR)) {
            entries.push_back(FileAccessor::DirectoryContentEntry(params.SrcDir(), true));
            FilterCallback filterCallback = params.GetFilterCallback();
            for (auto iter = entries.begin(); iter != entries.end(); ++iter) {
                if (iter != entries.begin() && ((!params.GetIncludeHiddenFiles() && IsHiddenFile(iter->path)) ||
                                                   (filterCallback && !filterCallback(iter->path)))) {
                    continue;
                }
                if (iter != entries.begin()) {
                    FilePath relativePath;
                    FilePath paramsSrcPath = params.SrcDir();
                    if (paramsSrcPath.AppendRelativePath(iter->path, &relativePath)) {
                        allRelativeFiles.push_back(relativePath);
                    }
                }
                if (iter->isDirectory) {
                    bool isSuccess = false;
                    std::vector<FileAccessor::DirectoryContentEntry> subEntries =
                        ListDirectoryContent(iter->path, isSuccess);
                    entries.insert(entries.end(), subEntries.begin(), subEntries.end());
                }
            }
        } else {
            allRelativeFiles.push_back(paramPath.BaseName());
        }
    }
    std::unique_ptr<ZipWriter> zipWriter = nullptr;
    if (params.DestFd() != kInvalidPlatformFile) {
        zipWriter = std::make_unique<ZipWriter>(ZipWriter::InitZipFileWithFd(params.DestFd()), paramPath);
    } else {
        zipWriter = std::make_unique<ZipWriter>(ZipWriter::InitZipFileWithFile(params.DestFile()), paramPath);
    }
    if (zipWriter == nullptr) {
        LOGE("Init zipWriter failed!");
        return false;
    }
    return zipWriter->WriteEntries(*filesToAdd, options);
}

ErrCode UnzipWithFilterAndWriters(const PlatformFile& srcFile, FilePath& destDir, WriterFactory writerFactory,
    DirectoryCreator directoryCreator, UnzipParam& unzipParam)
{
    LOGI("destDir=%{private}s", destDir.Value().c_str());
    ZipReader reader;
    if (!reader.OpenFromPlatformFile(srcFile)) {
        LOGI("Failed to open srcFile.");
        return ERR_ZLIB_SRC_FILE_FORMAT_ERROR;
    }
    while (reader.HasMore()) {
        if (!reader.OpenCurrentEntryInZip()) {
            LOGI("Failed to open the current file in zip.");
            return ERR_ZLIB_SERVICE_DISABLED;
        }
        const FilePath& constEntryPath = reader.CurrentEntryInfo()->GetFilePath();
        FilePath entryPath = constEntryPath;
        if (reader.CurrentEntryInfo()->IsUnsafe()) {
            LOGI("Found an unsafe file in zip.");
            return ERR_ZLIB_SERVICE_DISABLED;
        }
        // callback
        if (unzipParam.filterCB(entryPath)) {
            if (reader.CurrentEntryInfo()->IsDirectory()) {
                if (!directoryCreator(destDir, entryPath)) {
                    LOGI("!!!directory_creator(%{private}s) Failed!!!.", entryPath.Value().c_str());
                    return ERR_ZLIB_DEST_FILE_DISABLED;
                }

            } else {
                std::unique_ptr<WriterDelegate> writer = writerFactory(destDir, entryPath);
                if (!reader.ExtractCurrentEntry(writer.get(), std::numeric_limits<uint64_t>::max())) {
                    LOGI("Failed to extract.");
                    return ERR_ZLIB_SERVICE_DISABLED;
                }
            }
        } else if (unzipParam.logSkippedFiles) {
            LOGI("Skipped file.");
        }

        if (!reader.AdvanceToNextEntry()) {
            LOGI("Failed to advance to the next file.");
            return ERR_ZLIB_SERVICE_DISABLED;
        }
    }
    return ERR_OK;
}

ErrCode UnzipWithFilterCallback(
    const FilePath& srcFile, const FilePath& destDir, const OPTIONS& options, UnzipParam& unzipParam)
{
    FilePath src = srcFile;
    if (!FilePathCheckValid(src.Value())) {
        LOGI("FilePathCheckValid returnValue is false.");
        return ERR_ZLIB_SRC_FILE_DISABLED;
    }

    FilePath dest = destDir;

    LOGI("srcFile=%{private}s, destFile=%{private}s", src.Value().c_str(), dest.Value().c_str());

    if (!FilePath::PathIsValid(srcFile)) {
        LOGI("PathIsValid return value is false.");
        return ERR_ZLIB_SRC_FILE_DISABLED;
    }

    PlatformFile zipFd = open(src.Value().c_str(), S_IREAD, O_CREAT);
    if (zipFd == kInvalidPlatformFile) {
        LOGI("Failed to open.");
        return ERR_ZLIB_SRC_FILE_DISABLED;
    }
    ErrCode ret = UnzipWithFilterAndWriters(zipFd, dest,
        std::bind(&CreateFilePathWriterDelegate, std::placeholders::_1, std::placeholders::_2),
        std::bind(&CreateDirectory, std::placeholders::_1, std::placeholders::_2), unzipParam);
    close(zipFd);
    return ret;
}

bool Unzip(const std::string& srcFile, const std::string& destFile, OPTIONS options,
    std::shared_ptr<ZlibCallbackInfo> zlibCallbackInfo)
{
    if (zlibCallbackInfo == nullptr) {
        LOGE("zlibCallbackInfo is nullptr!");
        return false;
    }
    FilePath srcFileDir(srcFile);
    FilePath destDir(destFile);
    if (destDir.Value().size() == 0) {
        zlibCallbackInfo->OnZipUnZipFinish(ERR_ZLIB_DEST_FILE_DISABLED);
        return false;
    }
    if (srcFileDir.Value().size() == 0) {
        LOGI("Unzip called fail, srcFile isn't Exist.");
        zlibCallbackInfo->OnZipUnZipFinish(ERR_ZLIB_SRC_FILE_DISABLED);
        return false;
    }
    if (!FilePath::PathIsValid(srcFileDir)) {
        LOGI("Unzip called fail, srcFile isn't Exist.");
        zlibCallbackInfo->OnZipUnZipFinish(ERR_ZLIB_SRC_FILE_DISABLED);
        return false;
    }
    if (FilePath::DirectoryExists(destDir)) {
        if (!FilePath::PathIsValid(destDir)) {
            LOGI("FilePath::PathIsValid(destDir) fail.");
            zlibCallbackInfo->OnZipUnZipFinish(ERR_ZLIB_DEST_FILE_DISABLED);
        }
    } else {
        LOGI("Unzip called fail, destDir isn't path.");
        zlibCallbackInfo->OnZipUnZipFinish(ERR_ZLIB_DEST_FILE_DISABLED);
        return false;
    }
    auto innerTask = [srcFileDir, destDir, options, zlibCallbackInfo]() {
        UnzipParam unzipParam { .filterCB = ExcludeNoFilesFilter, .logSkippedFiles = true };
        ErrCode err = UnzipWithFilterCallback(srcFileDir, destDir, options, unzipParam);
        if (zlibCallbackInfo != nullptr) {
            zlibCallbackInfo->OnZipUnZipFinish(err);
        }
    };
    PostTask(innerTask);
    return true;
}

ErrCode ZipWithFilterCallback(
    const FilePath& srcDir, const FilePath& destFile, const OPTIONS& options, FilterCallback filterCB)
{
    FilePath destPath = destFile;
    if (FilePath::DirectoryExists(destFile)) {
        if (!FilePath::PathIsValid(destFile)) {
            LOGI("ZipWithFilterCallback called fail, destFile isn't Exist.");
            return ERR_ZLIB_DEST_FILE_DISABLED;
        }
    } else if (!FilePath::PathIsValid(destPath.DirName())) {
        LOGI("ZipWithFilterCallback called fail, The path where destFile is located doesn't exist.");
        return ERR_ZLIB_DEST_FILE_DISABLED;
    }

    if (FilePath::DirectoryExists(srcDir)) {
        if (!FilePath::PathIsValid(srcDir)) {
            LOGI("ZipWithFilterCallback called fail, srcDir isn't Exist.");
            return ERR_ZLIB_SRC_FILE_DISABLED;
        }
    } else if (!FilePath::PathIsValid(srcDir)) {
        LOGI("ZipWithFilterCallback called fail, The path where srcDir is located doesn't exist.");
        return ERR_ZLIB_SRC_FILE_DISABLED;
    }

    ZipParams params(srcDir, FilePath(destPath.CheckDestDirTail()));
    params.SetFilterCallback(filterCB);
    bool result = Zip(params, options);
    if (result) {
        return ERR_OK;
    } else {
        return ERR_ZLIB_SERVICE_DISABLED;
    }
}

bool Zip(const std::string& srcPath, const std::string& destPath, bool includeHiddenFiles, const OPTIONS options,
    std::shared_ptr<ZlibCallbackInfo> zlibCallbackInfo)
{
    if (zlibCallbackInfo == nullptr) {
        return false;
    }
    FilePath srcDir(srcPath);
    FilePath destFile(destPath);
    LOGI("Zip called,  srcDir=%{private}s, destFile=%{private}s", srcDir.Value().c_str(), destFile.Value().c_str());

    if (srcDir.Value().size() == 0) {
        zlibCallbackInfo->OnZipUnZipFinish(ERR_ZLIB_SRC_FILE_DISABLED);
        return false;
    }
    if (destFile.Value().size() == 0) {
        zlibCallbackInfo->OnZipUnZipFinish(ERR_ZLIB_DEST_FILE_DISABLED);
        return false;
    }

    auto innerTask = [srcDir, destFile, includeHiddenFiles, options, zlibCallbackInfo]() {
        if (includeHiddenFiles) {
            ErrCode err = ZipWithFilterCallback(srcDir, destFile, options, ExcludeNoFilesFilter);
            if (zlibCallbackInfo != nullptr) {
                zlibCallbackInfo->OnZipUnZipFinish(err);
            }
        } else {
            ErrCode err = ZipWithFilterCallback(srcDir, destFile, options, ExcludeHiddenFilesFilter);
            if (zlibCallbackInfo != nullptr) {
                zlibCallbackInfo->OnZipUnZipFinish(err);
            }
        }
    };
    PostTask(innerTask);
    return true;
}
} // namespace LIBZIP
} // namespace AppExecFwk
} // namespace OHOS