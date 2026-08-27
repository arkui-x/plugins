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
#include "file_uri.h"
#include "filemgmt_libhilog.h"
#include "uri_opener.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;
using namespace OHOS::AppFileService::ModuleFileUri;

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

static int OpenFileByPath(const string &path, unsigned int mode)
{
    unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> open_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!open_req) {
        HILOGE("Failed to request heap memory.");
        return -ENOMEM;
    }
    int ret = uv_fs_open(nullptr, open_req.get(), path.c_str(), mode,
                         S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, nullptr);
    return ret;
}

static tuple<int, string, bool> OpenFileByUri(const string &path, unsigned int mode)
{
    if (path.find("file://") != 0) {
        auto pf = OpenByPlatformNative(path, mode);
        if (pf.opened) {
            return { pf.fd, path, true };
        }
        HILOGE("Platform native open failed for non-file uri");
        return { -EINVAL, path, false };
    }

    AppFileService::ModuleFileUri::FileUri fileUri(path);
    string realPath = fileUri.GetRealPath();
    int ret = -1;
    if (!realPath.empty()) {
        ret = OpenFileByPath(realPath, mode);
    }
    if (ret >= 0) {
        return { ret, path, true };
    }

    HILOGI("libuv open failed for file uri, fallback to platform native, ret=%{public}d", ret);
    auto pf = OpenByPlatformNative(path, mode);
    if (pf.opened) {
        return { pf.fd, path, true };
    }
    HILOGE("Platform native fallback also failed");
    return { ret, path, false };
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
    std::string pathStr(path.get());

    if (pathStr.find("://") != string::npos) {
        auto [res, uriStr, isUri] = OpenFileByUri(pathStr, mode);
        if (res < 0) {
            NError(res).ThrowErr(env);
            return nullptr;
        }
        return InstantiateFile(env, res, uriStr, isUri).val_;
    }

    int ret = OpenFileByPath(pathStr, mode);
    if (ret < 0) {
        HILOGE("Failed to open file for libuv error %{public}d", ret);
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    return InstantiateFile(env, ret, pathStr, false).val_;
}

struct AsyncOpenFileArg {
    int fd = 0;
    string path;
    string uri;
    bool isUri = false;
};

static NError AsyncCbExec(shared_ptr<AsyncOpenFileArg> arg, const string &path, unsigned int mode)
{
    string pathStr(path);
    if (pathStr.find("://") != string::npos) {
        auto [res, uriStr, isUri] = OpenFileByUri(pathStr, mode);
        if (res < 0) {
            return NError(res);
        }
        arg->fd = res;
        arg->path = "";
        arg->uri = uriStr;
        arg->isUri = isUri;
        return NError(ERRNO_NOERR);
    }
    int ret = OpenFileByPath(pathStr, mode);
    if (ret < 0) {
        HILOGE("Failed to open file for libuv error %{public}d", ret);
        return NError(ret);
    }
    arg->fd = ret;
    arg->path = pathStr;
    arg->uri = "";
    return NError(ERRNO_NOERR);
}

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
        return AsyncCbExec(arg, path, mode);
    };
    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return InstantiateFile(env, arg->fd, arg->isUri ? arg->uri : arg->path, arg->isUri);
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