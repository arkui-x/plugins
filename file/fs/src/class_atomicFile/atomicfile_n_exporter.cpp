/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "atomicfile_n_exporter.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <sys/stat.h>

#include "atomicfile_entity.h"
#include "class_file/file_entity.h"
#include "class_file/file_n_exporter.h"
#include "../fs_utils.h"
#include "common_func.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "filemgmt_libn.h"
#include "uv.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace OHOS::FileManagement::LibN;

namespace {
const std::string READ_STREAM_CLASS = "ReadStream";
const std::string WRITE_STREAM_CLASS = "WriteStream";
const std::string TEMP_FILE_SUFFIX = "_XXXXXX";

struct BufferData {
    uint8_t* buffer = nullptr;
    size_t length = 0;

    ~BufferData()
    {
        delete[] buffer;
    }
};
}

static void FinalizeCallback(napi_env env, void *finalizeData, void *finalizeHint)
{
    BufferData *bufferData = static_cast<BufferData *>(finalizeData);
    delete bufferData;
}

static napi_value CreateStream(napi_env env, napi_callback_info info, const std::string &streamName,
    const std::string &fileName)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    const char moduleName[] = "@ohos.file.streamrw";
    napi_value streamrw;
    napi_status status = napi_load_module(env, moduleName, &streamrw);
    if (status != napi_ok) {
        HILOGE("Failed to load module");
        NError(UNKROWN_ERR).ThrowErr(env, "Failed to load module");
        return nullptr;
    }

    napi_value constructor = nullptr;
    status = napi_get_named_property(env, streamrw, streamName.c_str(), &constructor);
    if (status != napi_ok) {
        HILOGE("Failed to get named property");
        NError(UNKROWN_ERR).ThrowErr(env, "Failed to get named property");
        return nullptr;
    }

    napi_value filePath = NVal::CreateUTF8String(env, fileName).val_;
    napi_value argv[NARG_CNT::ONE] = {filePath};
    napi_value streamObj;
    size_t argc = 1;
    status = napi_new_instance(env, constructor, argc, argv, &streamObj);
    if (status != napi_ok) {
        HILOGE("Failed to create napi new instance");
        NError(UNKROWN_ERR).ThrowErr(env, "Failed to create napi new instance");
        return nullptr;
    }

    return NVal(env, streamObj).val_;
}

static void CallFunctionByName(napi_env env, napi_value objStream, const std::string &funcName)
{
    napi_valuetype valuetype;
    napi_typeof(env, objStream, &valuetype);
    if (valuetype != napi_object) {
        HILOGE("Valuetype is unmatched");
        return;
    }

    napi_value key;
    napi_status status = napi_create_string_utf8(env, funcName.c_str(), funcName.length(), &key);
    if (status != napi_ok) {
        HILOGE("Failed to create string utf8");
        return;
    }

    napi_value value;
    status = napi_get_property(env, objStream, key, &value);
    if (status != napi_ok) {
        HILOGE("Failed to get property");
        return;
    }

    status = napi_call_function(env, objStream, value, 0, nullptr, nullptr);
    if (status != napi_ok) {
        HILOGE("Failed to call %{public}s function", funcName.c_str());
        return;
    }
}

static NVal InstantiateFile(napi_env env, int fd, std::string path, bool isUri)
{
    napi_value objFile = NClass::InstantiateClass(env, FileNExporter::className_, {});
    if (!objFile) {
        close(fd);
        HILOGE("Failed to instantiate class");
        NError(UNKROWN_ERR).ThrowErr(env, "Failed to instantiate class");
        return NVal();
    }

    auto fileEntity = NClass::GetEntityOf<FileEntity>(env, objFile);
    if (fileEntity == nullptr) {
        close(fd);
        HILOGE("Failed to get fileEntity");
        NError(UNKROWN_ERR).ThrowErr(env, "Failed to get fileEntity");
        return NVal();
    }
    auto fdg = CreateUniquePtr<DistributedFS::FDGuard>(fd, false);
    if (fdg == nullptr) {
        close(fd);
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return NVal();
    }
    fileEntity->fd_.swap(fdg);
    if (isUri) {
        fileEntity->path_ = "";
        fileEntity->uri_ = path;
    } else {
        fileEntity->path_ = path;
        fileEntity->uri_ = "";
    }
    return { env, objFile };
}

static std::tuple<AtomicFileEntity*, int32_t> GetAtomicFileEntity(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        return {nullptr, E_PARAMS};
    }

    auto rafEntity = NClass::GetEntityOf<AtomicFileEntity>(env, funcArg.GetThisVar());
    if (rafEntity == nullptr) {
        HILOGE("Failed to get atomicFile");
        return {nullptr, UNKROWN_ERR};
    }

    return {rafEntity, 0};
}

napi_value AtomicFileNExporter::GetBaseFile(napi_env env, napi_callback_info info)
{
    auto [rafEntity, errcode] = GetAtomicFileEntity(env, info);
    if (errcode != 0) {
        if (errcode == UNKROWN_ERR) {
            NError(errcode).ThrowErr(env, "Failed to get atomicFile");
        } else {
            NError(errcode).ThrowErr(env);
        }
        return nullptr;
    }

    if (rafEntity->baseFileName.size() >= PATH_MAX) {
        HILOGE("Base file name is too long");
        NError(UNKROWN_ERR).ThrowErr(env, "Base file name is too long");
        return nullptr;
    }

    char realPath[PATH_MAX];
    char *result = realpath(rafEntity->baseFileName.c_str(), realPath);
    int err = errno;
    if (result == nullptr) {
        HILOGE("Failed to resolve real path, err:%{public}d", errno);
        NError(err).ThrowErr(env);
        return nullptr;
    }

    int fd = open(result, O_RDONLY);
    if (fd < 0) {
        HILOGE("Failed to open file, err:%{public}d", errno);
        NError(errno).ThrowErr(env);
        return nullptr;
    }
    return InstantiateFile(env, fd, rafEntity->baseFileName, false).val_;
}

napi_value AtomicFileNExporter::OpenRead(napi_env env, napi_callback_info info)
{
    auto [rafEntity, errcode] = GetAtomicFileEntity(env, info);
    if (errcode != 0) {
        if (errcode == UNKROWN_ERR) {
            NError(errcode).ThrowErr(env, "Failed to get atomicFile");
        } else {
            NError(errcode).ThrowErr(env);
        }
        return nullptr;
    }

    return CreateStream(env, info, READ_STREAM_CLASS, rafEntity->baseFileName);
}

static std::tuple<std::unique_ptr<BufferData>, int32_t> ReadFileToBuffer(napi_env env, FILE* fp)
{
    int fd = fileno(fp);
    if (fd < 0) {
        HILOGE("Failed to get file descriptor, err:%{public}d", errno);
        return {nullptr, UNKROWN_ERR};
    }

    struct stat fileStat {};
    if (fstat(fd, &fileStat) < 0) {
        HILOGE("Failed to get file stats, err:%{public}d", errno);
        return {nullptr, errno};
    }

    long fileSize = fileStat.st_size;
    if (fileSize <= 0) {
        HILOGE("Invalid file size");
        return {nullptr, EIO};
    }

    auto bufferData = std::make_unique<BufferData>();
    bufferData->buffer = new(std::nothrow) uint8_t[fileSize];
    if (bufferData->buffer == nullptr) {
        HILOGE("Failed to allocate memory");
        return {nullptr, ENOMEM};
    }
    bufferData->length = fread(bufferData->buffer, sizeof(uint8_t), fileSize, fp);
    if ((bufferData->length != static_cast<size_t>(fileSize) && !feof(fp)) || ferror(fp)) {
        HILOGE("Failed to read file, actual length is:%zu, fileSize:%ld", bufferData->length, fileSize);
        delete[] bufferData->buffer;
        bufferData->buffer = nullptr;
        bufferData->length = 0;
        return {nullptr, EIO};
    }
    return {std::move(bufferData), 0};
}

napi_value AtomicFileNExporter::ReadFully(napi_env env, napi_callback_info info)
{
    auto [rafEntity, errcode] = GetAtomicFileEntity(env, info);
    if (errcode != 0) {
        if (errcode == UNKROWN_ERR) {
            NError(errcode).ThrowErr(env, "Failed to get atomicFile");
        } else {
            NError(errcode).ThrowErr(env);
        }
        return nullptr;
    }

    char realPath[PATH_MAX];
    char *result = realpath(rafEntity->baseFileName.c_str(), realPath);
    if (result == nullptr) {
        HILOGE("Failed to resolve file real path, err:%{public}d", errno);
        NError(errno).ThrowErr(env);
        return nullptr;
    }

    auto file = std::unique_ptr<FILE, decltype(&std::fclose)>(
        std::fopen(result, "rb"), &std::fclose);
    if (!file) {
        HILOGE("Failed to open file, err:%{public}d", errno);
        NError(errno).ThrowErr(env);
        return nullptr;
    }

    auto [bufferData, readErrcode] = ReadFileToBuffer(env, file.get());
    if (readErrcode != 0) {
        if (readErrcode == UNKROWN_ERR) {
            NError(readErrcode).ThrowErr(env, "Failed to read file to buffer");
        } else {
            NError(readErrcode).ThrowErr(env);
        }
        return nullptr;
    }

    napi_value externalBuffer = nullptr;
    size_t length = bufferData->length;
    napi_status status = napi_create_external_arraybuffer(
        env, bufferData->buffer, bufferData->length, FinalizeCallback, bufferData.release(), &externalBuffer);
    if (status != napi_ok) {
        NError(UNKROWN_ERR).ThrowErr(env, "Failed to create external arraybuffer");
        return nullptr;
    }

    napi_value outputArray = nullptr;
    status = napi_create_typedarray(env, napi_int8_array, length, externalBuffer, 0, &outputArray);
    if (status != napi_ok) {
        NError(UNKROWN_ERR).ThrowErr(env, "Failed to create typedarray");
        return nullptr;
    }

    return outputArray;
}

napi_value AtomicFileNExporter::StartWrite(napi_env env, napi_callback_info info)
{
    auto [rafEntity, errcode] = GetAtomicFileEntity(env, info);
    if (errcode != 0) {
        if (errcode == UNKROWN_ERR) {
            NError(errcode).ThrowErr(env, "Failed to get atomicFile");
        } else {
            NError(errcode).ThrowErr(env);
        }
        return nullptr;
    }

    std::string parentPath = GetParentDirectory(rafEntity->newFileName);
    if (access(parentPath.c_str(), F_OK) != 0) {
        HILOGE("Parent directory does not exist, err:%{public}d", errno);
        NError(ENOENT).ThrowErr(env);
        return nullptr;
    }

    char *tmpfile = const_cast<char *>(rafEntity->newFileName.c_str());
    if (mkstemp(tmpfile) == -1) {
        HILOGE("Fail to create tmp file err:%{public}d!", errno);
        NError(ENOENT).ThrowErr(env);
        return nullptr;
    }

    napi_value writeStream = CreateStream(env, info, WRITE_STREAM_CLASS, rafEntity->newFileName);
    if (writeStream == nullptr) {
        HILOGE("Failed to create write stream");
        return nullptr;
    }
    napi_status status = napi_create_reference(env, writeStream, 1, &rafEntity->writeStreamObj);
    if (status != napi_ok) {
        HILOGE("Failed to create reference");
        NError(UNKROWN_ERR).ThrowErr(env, "Failed to create reference");
        return nullptr;
    }
    return writeStream;
}

napi_value AtomicFileNExporter::FinishWrite(napi_env env, napi_callback_info info)
{
    auto [rafEntity, errcode] = GetAtomicFileEntity(env, info);
    if (errcode != 0) {
        if (errcode == UNKROWN_ERR) {
            NError(errcode).ThrowErr(env, "Failed to get atomicFile");
        } else {
            NError(errcode).ThrowErr(env);
        }
        return nullptr;
    }

    napi_value writeStream;
    napi_status status = napi_get_reference_value(env, rafEntity->writeStreamObj, &writeStream);
    if (status != napi_ok) {
        HILOGE("Failed to get reference value");
        NError(UNKROWN_ERR).ThrowErr(env, "Failed to get reference value");
        return nullptr;
    }

    CallFunctionByName(env, writeStream, "closeSync");

    int32_t result = std::rename(rafEntity->newFileName.c_str(), rafEntity->baseFileName.c_str());
    if (result != 0) {
        HILOGE("Failed to rename file, ret:%{public}d", result);
        status = napi_delete_reference(env, rafEntity->writeStreamObj);
        if (status != napi_ok) {
            HILOGE("Failed to delete reference");
            NError(UNKROWN_ERR).ThrowErr(env, "Failed to delete reference");
            return nullptr;
        }
        NError(UNKROWN_ERR).ThrowErr(env, "Failed to rename file");
        return nullptr;
    }
    std::string tmpNewFileName = rafEntity->baseFileName;
    rafEntity->newFileName = tmpNewFileName.append(TEMP_FILE_SUFFIX);
    status = napi_delete_reference(env, rafEntity->writeStreamObj);
    if (status != napi_ok) {
        HILOGE("Failed to delete reference");
        NError(UNKROWN_ERR).ThrowErr(env, "Failed to delete reference");
        return nullptr;
    }
    return nullptr;
}

napi_value AtomicFileNExporter::FailWrite(napi_env env, napi_callback_info info)
{
    auto [rafEntity, errcode] = GetAtomicFileEntity(env, info);
    if (errcode != 0) {
        if (errcode == UNKROWN_ERR) {
            NError(errcode).ThrowErr(env, "Failed to get atomicFile");
        } else {
            NError(errcode).ThrowErr(env);
        }
        return nullptr;
    }

    napi_value writeStream;
    napi_status status = napi_get_reference_value(env, rafEntity->writeStreamObj, &writeStream);
    if (status != napi_ok) {
        HILOGE("Failed to get reference value");
        NError(UNKROWN_ERR).ThrowErr(env, "Failed to get reference value");
        return nullptr;
    }

    CallFunctionByName(env, writeStream, "closeSync");

    if (!RemoveFile(rafEntity->newFileName.c_str())) {
        HILOGW("Failed to remove file");
        status = napi_delete_reference(env, rafEntity->writeStreamObj);
        if (status != napi_ok) {
            HILOGE("Failed to delete reference");
            NError(UNKROWN_ERR).ThrowErr(env, "Failed to delete reference");
            return nullptr;
        }
        NError(UNKROWN_ERR).ThrowErr(env, "Failed to remove file");
        return nullptr;
    }
    std::string tmpNewFileName = rafEntity->baseFileName;
    rafEntity->newFileName = tmpNewFileName.append(TEMP_FILE_SUFFIX);
    status = napi_delete_reference(env, rafEntity->writeStreamObj);
    if (status != napi_ok) {
        HILOGE("Failed to delete reference");
        NError(UNKROWN_ERR).ThrowErr(env, "Failed to delete reference");
    }
    return nullptr;
}

napi_value AtomicFileNExporter::Delete(napi_env env, napi_callback_info info)
{
    auto [rafEntity, errcode] = GetAtomicFileEntity(env, info);
    if (errcode != 0) {
        if (errcode == UNKROWN_ERR) {
            NError(errcode).ThrowErr(env, "Failed to get atomicFile");
        } else {
            NError(errcode).ThrowErr(env);
        }
        return nullptr;
    }

    bool errFlag = false;
    if (FileIsExist(rafEntity->newFileName.c_str()) && !RemoveFile(rafEntity->newFileName.c_str())) {
        errFlag = true;
    }
    if (FileIsExist(rafEntity->baseFileName.c_str()) && !RemoveFile(rafEntity->baseFileName.c_str())) {
        errFlag = true;
    }
    if (errFlag) {
        HILOGE("Failed to remove file");
    }

    rafEntity->newFileName.clear();
    rafEntity->baseFileName.clear();
    return nullptr;
}

napi_value AtomicFileNExporter::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto [resGetFirstArg, file, num] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!resGetFirstArg) {
        HILOGE("Invalid path");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto atomicFileEntity = CreateUniquePtr<AtomicFileEntity>();
    if (atomicFileEntity == nullptr) {
        HILOGE("Failed to request heap memory");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    std::string filePath = file.get();
    atomicFileEntity->baseFileName = filePath;
    atomicFileEntity->newFileName = filePath.append(TEMP_FILE_SUFFIX);
    if (!NClass::SetEntityFor<AtomicFileEntity>(env, funcArg.GetThisVar(), move(atomicFileEntity))) {
        HILOGE("Failed to wrap entity for obj AtomicFile");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }

    return funcArg.GetThisVar();
}

bool AtomicFileNExporter::Export()
{
    std::vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("getBaseFile", GetBaseFile),
        NVal::DeclareNapiFunction("openRead", OpenRead),
        NVal::DeclareNapiFunction("readFully", ReadFully),
        NVal::DeclareNapiFunction("startWrite", StartWrite),
        NVal::DeclareNapiFunction("finishWrite", FinishWrite),
        NVal::DeclareNapiFunction("failWrite", FailWrite),
        NVal::DeclareNapiFunction("delete", Delete),
    };

    std::string className = GetClassName();
    bool succ = false;
    napi_value classValue = nullptr;
    std::tie(succ, classValue) = NClass::DefineClass(
        exports_.env_, className, AtomicFileNExporter::Constructor, move(props));
    if (!succ) {
        HILOGE("INNER BUG. Failed to define class");
        NError(ENOMEM).ThrowErr(exports_.env_);
        return false;
    }
    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        HILOGE("INNER BUG. Failed to save class");
        NError(ENOMEM).ThrowErr(exports_.env_);
        return false;
    }

    return exports_.AddProp(className, classValue);
}

std::string AtomicFileNExporter::GetClassName()
{
    return AtomicFileNExporter::className_;
}

AtomicFileNExporter::AtomicFileNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}
AtomicFileNExporter::~AtomicFileNExporter() {}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
