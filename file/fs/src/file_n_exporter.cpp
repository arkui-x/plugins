/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "file_entity.h"
#include "file_n_exporter.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <sys/file.h>
#include <tuple>

#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "filemgmt_libn.h"
#include "../common_func.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static FileEntity *GetFileEntity(napi_env env, napi_value raf_entity)
{
    auto rafEntity = NClass::GetEntityOf<FileEntity>(env, raf_entity);
    if (!rafEntity) {
        HILOGE("Failed to get file entity");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    if (!rafEntity->fd_) {
        HILOGE("rafEntity fd is not exist");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    return rafEntity;
}

static tuple<int, unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*>> RealPathCore(const string &srcPath)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> realpath_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!realpath_req) {
        HILOGE("Failed to request heap memory.");
        return { ENOMEM, move(realpath_req)};
    }
    int ret = uv_fs_realpath(nullptr, realpath_req.get(), srcPath.c_str(), nullptr);
    return { ret, move(realpath_req) };
}

napi_value FileNExporter::GetPath(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto fileEntity = GetFileEntity(env, funcArg.GetThisVar());
    if (!fileEntity) {
        HILOGE("Failed to get file entity");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [realPathRes, realPath] = RealPathCore(fileEntity->path_);
    if (realPathRes != ERRNO_NOERR) {
        HILOGE("Failed to get real path");
        NError(realPathRes).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUTF8String(env, string(static_cast<const char *>(realPath->ptr))).val_;
}

napi_value FileNExporter::GetName(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto fileEntity = GetFileEntity(env, funcArg.GetThisVar());
    if (!fileEntity) {
        HILOGE("Failed to get file entity");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [realPathRes, realPath] = RealPathCore(fileEntity->path_);
    if (realPathRes != ERRNO_NOERR) {
        HILOGE("Failed to get real path");
        NError(realPathRes).ThrowErr(env);
        return nullptr;
    }
    string path(static_cast<const char *>(realPath->ptr));
    auto pos = path.find_last_of('/');
    if (pos == string::npos) {
        HILOGE("Failed to split filename from path");
        NError(ENOENT).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUTF8String(env, path.substr(pos + 1)).val_;
}

napi_value FileNExporter::GetFD(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto rafEntity = GetFileEntity(env, funcArg.GetThisVar());
    if (!rafEntity) {
        HILOGE("Failed to get file entity");
        return nullptr;
    }
    return NVal::CreateInt32(env, rafEntity->fd_.get()->GetFD()).val_;
}

napi_value FileNExporter::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto rafEntity = CreateUniquePtr<FileEntity>();
    if (rafEntity == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    if (!NClass::SetEntityFor<FileEntity>(env, funcArg.GetThisVar(), move(rafEntity))) {
        HILOGE("Failed to set file entity");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    return funcArg.GetThisVar();
}

bool FileNExporter::Export()
{
    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiGetter("fd", GetFD),
        NVal::DeclareNapiFunction("lock", Lock),
        NVal::DeclareNapiGetter("path", GetPath),
        NVal::DeclareNapiFunction("tryLock", TryLock),
        NVal::DeclareNapiFunction("unlock", UnLock),
        NVal::DeclareNapiFunction("getParent", GetParent),
    };
    string className = GetClassName();
    bool succ = false;
    napi_value classValue = nullptr;
    tie(succ, classValue) = NClass::DefineClass(exports_.env_, className,
        FileNExporter::Constructor, move(props));
    if (!succ) {
        HILOGE("Define class exceptions");
        NError(EIO).ThrowErr(exports_.env_);
        return false;
    }
    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        HILOGE("Save class exceptions");
        NError(EIO).ThrowErr(exports_.env_);
        return false;
    }

    return exports_.AddProp(className, classValue);
}

static bool GetExclusive(napi_env env, NFuncArg &funcArg, bool &exclusive)
{
    if (funcArg.GetArgc() >= NARG_CNT::ONE) {
        bool succ = false;
        tie(succ, exclusive) = NVal(env, funcArg[NARG_POS::FIRST]).ToBool(exclusive);
        if (!succ) {
            HILOGE("Invalid exclusive");
            NError(EINVAL).ThrowErr(env);
            return false;
        }
    }
    return true;
}

napi_value FileNExporter::GetParent(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto fileEntity = GetFileEntity(env, funcArg.GetThisVar());
    if (!fileEntity) {
        HILOGE("Failed to get file entity");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    string path(fileEntity->path_);

    auto [realPathRes, realPath] = RealPathCore(path);
    if (realPathRes) {
        HILOGE("Failed to get real path");
        NError(realPathRes).ThrowErr(env);
        return nullptr;
    }
    path = static_cast<const char *>(realPath->ptr);
    auto pos = path.find_last_of('/');
    if (pos == string::npos) {
        HILOGE("Failed to split filename from path");
        NError(ENOENT).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUTF8String(env, path.substr(0, pos)).val_;
}

napi_value FileNExporter::Lock(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto fileEntity = GetFileEntity(env, funcArg.GetThisVar());
    if (!fileEntity) {
        HILOGE("Failed to get file entity");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool exclusive = false;
    if (!GetExclusive(env, funcArg, exclusive)) {
        return nullptr;
    }
    auto cbExec = [exclusive, fileEntity]() -> NError {
        if (!fileEntity || !fileEntity->fd_.get()) {
            HILOGE("File has been closed in Lock cbExec possibly");
            return NError(EIO);
        }
        int ret = 0;
        auto mode = exclusive ? LOCK_EX : LOCK_SH;
        ret = flock(fileEntity->fd_.get()->GetFD(), mode);
        if (ret < 0) {
            HILOGE("Failed to lock file");
            return NError(errno);
        } else {
            return NError(ERRNO_NOERR);
        }
    };

    auto cbCompl = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateUndefined(env);
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ZERO || (funcArg.GetArgc() == NARG_CNT::ONE &&
        !NVal(env, funcArg[NARG_POS::FIRST]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_LOCK_NAME, cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::TWO) ? NARG_POS::SECOND : NARG_POS::FIRST);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_LOCK_NAME, cbExec, cbCompl).val_;
    }
}

napi_value FileNExporter::TryLock(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto fileEntity = GetFileEntity(env, funcArg.GetThisVar());
    if (!fileEntity) {
        HILOGE("Failed to get file entity");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool exclusive = false;
    if (!GetExclusive(env, funcArg, exclusive)) {
        return nullptr;
    }

    int ret = 0;
    auto mode = exclusive ? LOCK_EX : LOCK_SH;
    ret = flock(fileEntity->fd_.get()->GetFD(), mode | LOCK_NB);
    if (ret < 0) {
        HILOGE("Failed to try to lock file");
        NError(errno).ThrowErr(env);
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value FileNExporter::UnLock(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto fileEntity = GetFileEntity(env, funcArg.GetThisVar());
    if (!fileEntity) {
        HILOGE("Failed to get file entity");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    int ret = 0;
    ret = flock(fileEntity->fd_.get()->GetFD(), LOCK_UN);
    if (ret < 0) {
        HILOGE("Failed to unlock file");
        NError(errno).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

string FileNExporter::GetClassName()
{
    return FileNExporter::className_;
}

FileNExporter::FileNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}
FileNExporter::~FileNExporter() {}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS