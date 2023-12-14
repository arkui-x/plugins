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

#include "move.h"

#include <sys/stat.h>

#include <tuple>
#include <unistd.h>

#include "common_func.h"
#include "filemgmt_libhilog.h"
#include "uv.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static bool CheckDir(const string &path)
{
    struct stat fileInformation;
    if (stat(path.c_str(), &fileInformation) == 0) {
        if (fileInformation.st_mode & S_IFDIR) {
            return true;
        }
    } else {
        HILOGE("Failed to stat file");
    }
    return false;
}

static tuple<bool, unique_ptr<char[]>, unique_ptr<char[]>, int> ParseJsOperand(napi_env env, const NFuncArg& funcArg)
{
    auto [resGetFirstArg, src, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!resGetFirstArg || CheckDir(string(src.get()))) {
        HILOGE("Invalid src");
        return { false, nullptr, nullptr, 0 };
    }
    auto [resGetSecondArg, dest, unused] = NVal(env, funcArg[NARG_POS::SECOND]).ToUTF8String();
    if (!resGetSecondArg || CheckDir(string(dest.get()))) {
        HILOGE("Invalid dest");
        return { false, nullptr, nullptr, 0 };
    }
    int mode = 0;
    if (funcArg.GetArgc() >= NARG_CNT::THREE) {
        bool resGetThirdArg = false;
        tie(resGetThirdArg, mode) = NVal(env, funcArg[NARG_POS::THIRD]).ToInt32(mode);
        if (!resGetThirdArg || (mode != MODE_FORCE_MOVE && mode != MODE_THROW_ERR)) {
            HILOGE("Invalid mode");
            return { false, nullptr, nullptr, 0 };
        }
    }
    return { true, move(src), move(dest), mode };
}

static int CopyAndDeleteFile(const string &src, const string &dest)
{
    int ret = 0;
    uv_fs_t copyfile_req;
    ret = uv_fs_copyfile(nullptr, &copyfile_req, src.c_str(), dest.c_str(), MODE_FORCE_MOVE, nullptr);
    uv_fs_req_cleanup(&copyfile_req);
    if (ret < 0) {
        HILOGE("Failed to move file using copyfile interface.");
        return ret;
    }
    uv_fs_t unlink_req;
    ret = uv_fs_unlink(nullptr, &unlink_req, src.c_str(), nullptr);
    if (ret < 0) {
        HILOGE("Failed to unlink src file");
        ret = uv_fs_unlink(nullptr, &unlink_req, dest.c_str(), nullptr);
        if (ret < 0) {
            HILOGE("Failed to unlink dest file");
        }
        uv_fs_req_cleanup(&unlink_req);
        return ret;
    }
    uv_fs_req_cleanup(&unlink_req);
    return ERRNO_NOERR;
}

static int RenameFile(const string &src, const string &dest)
{
    int ret = 0;
    uv_fs_t rename_req;
    ret = uv_fs_rename(nullptr, &rename_req, src.c_str(), dest.c_str(), nullptr);
    if (ret < 0 && (string_view(uv_err_name(ret)) == "EXDEV")) {
        return CopyAndDeleteFile(src, dest);
    }
    if (ret < 0) {
        HILOGE("Failed to move file using rename syscall.");
        return ret;
    }
    return ERRNO_NOERR;
}

static int MoveFile(const string &src, const string &dest, int mode)
{
    if (mode == MODE_THROW_ERR) {
        uv_fs_t access_req;
        int ret = uv_fs_access(nullptr, &access_req, dest.c_str(), 0, nullptr);
        uv_fs_req_cleanup(&access_req);
        if (ret == 0) {
            HILOGE("Failed to move file due to existing destPath with MODE_THROW_ERR.");
            return EEXIST;
        }
        if (ret < 0 && (string_view(uv_err_name(ret)) != "ENOENT")) {
            HILOGE("Failed to access destPath with MODE_THROW_ERR.");
            return ret;
        }
    }
    return RenameFile(src, dest);
}

napi_value Move::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succ, src, dest, mode] = ParseJsOperand(env, funcArg);
    if (!succ) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    int ret = MoveFile(string(src.get()), string(dest.get()), mode);
    if (ret) {
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value Move::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succ, src, dest, mode] = ParseJsOperand(env, funcArg);
    if (!succ) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = [srcPath = string(src.get()), destPath = string(dest.get()), mode = mode]() -> NError {
        int ret = MoveFile(srcPath, destPath, mode);
        if (ret) {
            return NError(ret);
        }
        return NError(ERRNO_NOERR);
    };

    auto cbComplCallback = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUndefined(env) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    size_t argc = funcArg.GetArgc();
    if (argc == NARG_CNT::TWO || (argc == NARG_CNT::THREE &&
        !NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_MOVE_NAME, cbExec, cbComplCallback).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_MOVE_NAME, cbExec, cbComplCallback).val_;
    }
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS