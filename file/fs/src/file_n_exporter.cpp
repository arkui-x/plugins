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
        NVal::DeclareNapiGetter("path", GetPath),
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

string FileNExporter::GetClassName()
{
    return FileNExporter::className_;
}

FileNExporter::FileNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}
FileNExporter::~FileNExporter() {}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS