/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "open.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <new>

#include "class_file/file_entity.h"
#include "class_file/file_n_exporter.h"
#include "common_func.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;
const std::string FILE_START = "file:///";

static tuple<bool, unsigned int> GetJsFlags(napi_env env, const NFuncArg &funcArg)
{
    unsigned int flags = O_RDONLY;
    if (funcArg.GetArgc() >= NARG_CNT::TWO && NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_number)) {
        auto [succ, mode] = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32(O_RDONLY);
        int32_t invalidMode = (O_WRONLY | O_RDWR);
        if (!succ || mode < 0 || ((mode & invalidMode) == invalidMode)) {
            HILOGE("Invalid mode");
            NError(EINVAL).ThrowErr(env);
            return { false, flags };
        }
        flags = static_cast<unsigned int>(mode);
        (void)CommonFunc::ConvertJsFlags(flags);
    }
    return { true, flags };
}

static NVal InstantiateFile(napi_env env, int fd, const string& pathOrUri, bool isUri)
{
    napi_value objFile = NClass::InstantiateClass(env, FileNExporter::className_, {});
    if (!objFile) {
        HILOGE("Failed to instantiate class");
        NError(EIO).ThrowErr(env);
        int ret = close(fd);
        if (ret < 0) {
            HILOGE("Failed to close fd");
        }
        return NVal();
    }

    auto fileEntity = NClass::GetEntityOf<FileEntity>(env, objFile);
    if (!fileEntity) {
        HILOGE("Failed to get fileEntity");
        NError(EIO).ThrowErr(env);
        int ret = close(fd);
        if (ret < 0) {
            HILOGE("Failed to close fd");
        }
        return NVal();
    }
    auto fdg = make_unique<DistributedFS::FDGuard>(fd, false);
    fileEntity->fd_.swap(fdg);
    if (isUri) {
        fileEntity->path_ = "";
        fileEntity->uri_ = pathOrUri;
    } else {
        fileEntity->path_ = pathOrUri;
        fileEntity->uri_ = "";
    }
    return { env, objFile };
}

static string GetSubstr(string path)
{
    string pathStr = string(path);
    std::size_t index = pathStr.find(FILE_START);
    if (index == 0) {
        pathStr = pathStr.substr(FILE_START.length());
    }
    return pathStr;
}

napi_value Open::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succPath, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succPath) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succMode, mode] = GetJsFlags(env, funcArg);
    if (!succMode) {
        HILOGE("Invalid mode");
        return nullptr;
    }
    string pathStr = GetSubstr(string(path.get()));
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> open_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!open_req) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    int ret = uv_fs_open(nullptr, open_req.get(), pathStr.c_str(), mode, S_IRUSR |
        S_IWUSR | S_IRGRP | S_IWGRP, nullptr);
    if (ret < 0) {
        HILOGE("Failed to open file for libuv error %{public}d", ret);
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    auto file = InstantiateFile(env, ret, pathStr, false).val_;
    return file;
}

struct AsyncOpenFileArg {
    int fd = 0;
    string path;
    string uri;
};

napi_value Open::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succPath, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succPath) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succMode, mode] = GetJsFlags(env, funcArg);
    if (!succMode) {
        HILOGE("Invalid mode");
        return nullptr;
    }
    auto arg = make_shared<AsyncOpenFileArg>();
    auto cbExec = [arg, path = string(path.get()), mode = mode]() -> NError {
        std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> open_req = {
            new (nothrow) uv_fs_t, CommonFunc::fs_req_cleanup };
        if (open_req == nullptr) {
            HILOGE("Failed to request heap memory.");
            return NError(ENOMEM);
        }
        int ret = uv_fs_open(nullptr, open_req.get(), GetSubstr(path).c_str(), mode, S_IRUSR |
            S_IWUSR | S_IRGRP | S_IWGRP, nullptr);
        if (ret < 0) {
            HILOGE("Failed to open file for libuv error %{public}d", ret);
            return NError(ret);
        }
        arg->fd = ret;
        arg->path = GetSubstr(path);
        return NError(ERRNO_NOERR);
    };
    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return InstantiateFile(env, arg->fd, arg->path, false);
    };
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE || (funcArg.GetArgc() == NARG_CNT::TWO &&
        NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_number))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_OPEN_NAME, cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::SECOND);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_OPEN_NAME, cbExec, cbCompl).val_;
    }
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS