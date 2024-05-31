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

#include "mkdtemp.h"

#include "common_func.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static bool CheckValidPath(const std::string &path)
{
    auto res = path.find_last_of("XXXXXX");
    return res == path.length() - 1;
}

napi_value Mkdtemp::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetFirstArg, tmp, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!resGetFirstArg) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    string path = tmp.get();
    if (!CheckValidPath(path)) {
        HILOGE("The invalid path doesn't have XXXXXX");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> mkdtemp_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!mkdtemp_req) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    int ret = uv_fs_mkdtemp(nullptr, mkdtemp_req.get(), const_cast<char *>(path.c_str()), nullptr);
    if (ret < 0) {
        HILOGE("Failed to create a temporary directory with path");
        NError(ret).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUTF8String(env, mkdtemp_req->path).val_;
}

static NError MkdTempExec(shared_ptr<string> arg, const string &path)
{
    if (!CheckValidPath(path)) {
        HILOGE("The invalid path doesn't have XXXXXX");
        return NError(EINVAL);
    }
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> mkdtemp_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!mkdtemp_req) {
        HILOGE("Failed to request heap memory.");
        return NError(ENOMEM);
    }
    int ret = uv_fs_mkdtemp(nullptr, mkdtemp_req.get(), path.c_str(), nullptr);
    if (ret < 0) {
        HILOGE("Failed to create a temporary directory with path");
        return NError(ret);
    } else {
        *arg = mkdtemp_req->path;
        return NError(ERRNO_NOERR);
    }
}

napi_value Mkdtemp::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetFirstArg, tmp, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!resGetFirstArg) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto arg = CreateSharedPtr<string>();
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [path = string(tmp.get()), arg]() -> NError {
        return MkdTempExec(arg, path);
    };
    auto cbComplete = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return NVal::CreateUTF8String(env, *arg);
        }
    };

    size_t argc = funcArg.GetArgc();
    NVal thisVar(env, funcArg.GetThisVar());
    if (argc == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_MKDTEMP_NAME, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_MKDTEMP_NAME, cbExec, cbComplete).val_;
    }
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
