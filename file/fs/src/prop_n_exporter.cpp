/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "prop_n_exporter.h"

#include <cstring>
#include <ctime>
#include <iostream>
#include <memory>
#include <sstream>
#include <sys/xattr.h>
#include <unistd.h>

#include "common_func.h"
#include "class_file/file_entity.h"
#include "class_file/file_n_exporter.h"
#include "close.h"
#include "copy_file.h"
#include "create_stream.h"
#include "create_streamrw.h"
#include "dup.h"
#include "fdatasync.h"
#include "fdopen_stream.h"
#include "filemgmt_libn.h"
#include "fsync.h"
#include "fs_utils.h"
#include "file_utils.h"
#include "listfile.h"
#include "lseek.h"
#include "lstat.h"
#include "mkdtemp.h"
#include "move.h"
#include "open.h"
#include "read_text.h"
#include "read_lines.h"
#include "rename.h"
#include "rmdirent.h"
#include "stat.h"
#include "symlink.h"
#include "utimes.h"
#include "truncate.h"
#include "xattr.h"
#include "movedir.h"
#include "copydir.h"
#include "create_randomaccessfile.h"
#include "class_randomaccessfile/randomaccessfile_n_exporter.h"
#ifndef IOS_PLATFORM
#include "watcher.h"
#endif

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

constexpr int32_t MAX_MODE_VALUE = 7;

enum AccessFlag : int32_t {
    DEFAULT_FLAG = -1,
    LOCAL_FLAG,
};
struct AccessArgs {
    string path;
    int mode = -1;
    int flag = DEFAULT_FLAG;
};

static int AccessCore(const string &path, int mode)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup) *> access_req = {new (std::nothrow) uv_fs_t,
        CommonFunc::fs_req_cleanup};
    if (!access_req) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    return uv_fs_access(nullptr, access_req.get(), path.c_str(), mode, nullptr);
}

static int GetMode(NVal secondVar, bool *hasMode)
{
    if (secondVar.TypeIs(napi_number)) {
        bool succ = false;
        int mode = 0;
        *hasMode = true;
        tie(succ, mode) = secondVar.ToInt32();
        if (succ && mode >= 0 && mode <= MAX_MODE_VALUE && (mode & 0x06) == mode) {
            return mode;
        }
    }
    return -1;
}

static bool GetAccessArgs(napi_env env, const NFuncArg &funcArg, AccessArgs &args)
{
    auto [succ, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8StringPath();
    if (!succ) {
        HILOGE("Invalid path from JS first argument");
        return false;
    }
    args.path = path.get();

    bool hasMode = false;
    if (funcArg.GetArgc() >= NARG_CNT::TWO) {
        args.mode = GetMode(NVal(env, funcArg[NARG_POS::SECOND]), &hasMode);
    }
    if (args.mode < 0 && hasMode) {
        HILOGE("Invalid mode from JS second argument");
        return false;
    }
    args.mode = hasMode ? args.mode : 0;

    if (funcArg.GetArgc() == NARG_CNT::THREE) {
        tie(succ, args.flag) = NVal(env, funcArg[NARG_POS::THIRD]).ToInt32(args.flag);
        if (!succ) {
            HILOGE("Invalid flag from JS third argument");
            return false;
        }
    }
    return true;
}

napi_value PropNExporter::AccessSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    AccessArgs args;
    if (!GetAccessArgs(env, funcArg, args)) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool isAccess = false;
    int ret = AccessCore(args.path, args.mode);
    if (ret < 0 && (string_view(uv_err_name(ret)) != "ENOENT")) {
        HILOGE("Failed to access file by path");
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    if (ret == 0) {
        isAccess = true;
    }
    return NVal::CreateBool(env, isAccess).val_;
}

napi_value PropNExporter::Access(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    AccessArgs args;
    if (!GetAccessArgs(env, funcArg, args)) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto result = CreateSharedPtr<AsyncAccessArg>();
    if (result == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [path = args.path, result, mode = args.mode]() -> NError {
        int ret = AccessCore(path, mode);
        if (ret == 0) {
            result->isAccess = true;
        }
        return (ret < 0 && (string_view(uv_err_name(ret)) != "ENOENT")) ? NError(ret) : NError(ERRNO_NOERR);
    };

    auto cbComplete = [result](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateBool(env, result->isAccess);
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE || NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_number)) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_ACCESS_NAME, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_ACCESS_NAME, cbExec, cbComplete).val_;
    }
}

static NError CheckDir(const string &path)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> stat_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!stat_req) {
        HILOGE("Failed to request heap memory.");
        return NError(ENOMEM);
    }
    int ret = uv_fs_stat(nullptr, stat_req.get(), path.c_str(), nullptr);
    if (ret < 0) {
        HILOGE("Failed to stat file with path");
        return NError(ret);
    }
    if ((stat_req->statbuf.st_mode & S_IFMT) == S_IFDIR) {
        HILOGE("The path is a directory");
        return NError(EISDIR);
    }
    return NError(ERRNO_NOERR);
}

napi_value PropNExporter::Unlink(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of Arguments Unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, tmp, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid path from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = [path = string(tmp.get())]() -> NError {
        auto checkRes = CheckDir(path);
        if (checkRes) {
            return checkRes;
        }
        std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> unlink_req = {
            new uv_fs_t, CommonFunc::fs_req_cleanup };
        if (!unlink_req) {
            HILOGE("Failed to request heap memory.");
            return NError(ENOMEM);
        }
        int ret = uv_fs_unlink(nullptr, unlink_req.get(), path.c_str(), nullptr);
        if (ret < 0) {
            HILOGE("Failed to unlink with path");
            return NError(ret);
        }
        return NError(ERRNO_NOERR);
    };

    auto cbCompl = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUndefined(env) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_UNLINK_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_UNLINK_NAME, cbExec, cbCompl).val_;
    }
}

napi_value PropNExporter::UnlinkSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid path from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto checkRes = CheckDir(string(path.get()));
    if (checkRes) {
        checkRes.ThrowErr(env);
        return nullptr;
    }
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> unlink_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!unlink_req) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    int ret = uv_fs_unlink(nullptr, unlink_req.get(), path.get(), nullptr);
    if (ret < 0) {
        HILOGE("Failed to unlink with path");
        NError(ret).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

static int MkdirCore(const string &path)
{
    if (path.empty()) {
        HILOGE("Invalid path: path is empty");
        return EINVAL;
    }
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> mkdirReq = {
        new (std::nothrow) uv_fs_t, CommonFunc::fs_req_cleanup};

    if (!mkdirReq) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    return uv_fs_mkdir(nullptr, mkdirReq.get(), path.c_str(), DIR_DEFAULT_PERM, nullptr);
}

static NError MkdirExec(const string &path, bool recursion, bool hasOption)
{
    int ret = AccessCore(path, 0);
    if (ret == ERRNO_NOERR) {
        HILOGD("The path already exists");
        return NError(EEXIST);
    }
    if (ret != -ENOENT) {
        HILOGE("Failed to check for illegal path or request for heap memory");
        return NError(ret);
    }
    if (hasOption) {
        ret = OHOS::FileManagement::ModuleFileIO::Mkdirs(path.c_str(), static_cast<MakeDirectionMode>(recursion));
        if (ret != 0) {
            HILOGE("Failed to create directories, error: %{public}d", ret);
            ret = ENOENT;
            return NError(ret);
        }
        ret = AccessCore(path, 0);
        if (ret != 0) {
            HILOGE("Failed to verify the result of Mkdirs function");
            return NError(ret);
        }
        return NError(ERRNO_NOERR);
    }

    ret = MkdirCore(path);
    if (ret) {
        HILOGE("Failed to create directory");
        return NError(ret);
    }
    return NError(ERRNO_NOERR);
}

napi_value PropNExporter::Mkdir(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succ, tmp, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8StringPath();
    if (!succ) {
        HILOGE("Invalid path from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool recursion = false;
    bool hasOption = false;

    if (funcArg.GetArgc() >= NARG_CNT::TWO) {
        NVal option(env, funcArg[NARG_POS::SECOND]);
        if (!option.TypeIs(napi_function)) {
            tie(hasOption, recursion) = option.ToBool(false);
            if (!hasOption) {
                HILOGE("Invalid option argument");
                NError(EINVAL).ThrowErr(env);
                return nullptr;
            }
        }
    }

    auto cbExec = [path = string(tmp.get()), recursion, hasOption]() -> NError {
        return MkdirExec(path, recursion, hasOption);
    };

    auto cbCompl = [](napi_env env, NError err) -> NVal {
        return err ? NVal(env, err.GetNapiErr(env)) : NVal::CreateUndefined(env);
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE || (funcArg.GetArgc() == NARG_CNT::TWO &&
        !NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_MKDIR_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[funcArg.GetArgc() - 1]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_MKDIR_NAME, cbExec, cbCompl).val_;
    }
}

napi_value PropNExporter::MkdirSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8StringPath();
    if (!succ) {
        HILOGE("Invalid path from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    bool hasOption = false;
    bool recursion = false;

    if (funcArg.GetArgc() == NARG_CNT::TWO) {
        tie(hasOption, recursion) = NVal(env, funcArg[NARG_POS::SECOND]).ToBool(false);
        if (!hasOption) {
            HILOGE("Invalid recursion mode");
            NError(EINVAL).ThrowErr(env);
            return nullptr;
        }
    }

    auto err = MkdirExec(path.get(), recursion, hasOption);
    if (err) {
        err.ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::ReadSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool succ = false;
    int fd = 0;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ || fd < 0) {
        HILOGE("Invalid fd from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    void *buf = nullptr;
    size_t len = 0;
    int64_t offset = -1;
    tie(succ, buf, len, offset) =
        CommonFunc::GetReadArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        HILOGE("Failed to resolve buf and options");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    uv_buf_t buffer = uv_buf_init(static_cast<char *>(buf), static_cast<unsigned int>(len));
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> read_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!read_req) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    int ret = uv_fs_read(nullptr, read_req.get(), fd, &buffer, 1, offset, nullptr);
    if (ret < 0) {
        HILOGE("Failed to read file for %{public}d", ret);
        NError(ret).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, static_cast<int64_t>(ret)).val_;
}

static NError ReadExec(shared_ptr<AsyncIOReadArg> arg, char *buf, size_t len, int32_t fd, int64_t offset)
{
    uv_buf_t buffer = uv_buf_init(buf, len);
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> read_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!read_req) {
        HILOGE("Failed to request heap memory.");
        return NError(ENOMEM);
    }
    int ret = uv_fs_read(nullptr, read_req.get(), fd, &buffer, 1, offset, nullptr);
    if (ret < 0) {
        HILOGE("Failed to read file for %{public}d", ret);
        return NError(ret);
    }
    arg->lenRead = ret;
    return NError(ERRNO_NOERR);
}

napi_value PropNExporter::Read(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool succ = false;
    void *buf = nullptr;
    size_t len = 0;
    int32_t fd = 0;
    int64_t offset = -1;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ || fd < 0) {
        HILOGE("Invalid fd from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    tie(succ, buf, len, offset) =
        CommonFunc::GetReadArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        HILOGE("Failed to resolve buf and options");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto arg = make_shared<AsyncIOReadArg>(NVal(env, funcArg[NARG_POS::SECOND]));
    if (!arg) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = [arg, buf, len, fd, offset]() -> NError {
        return ReadExec(arg, static_cast<char *>(buf), len, fd, offset);
    };

    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt64(env, static_cast<int64_t>(arg->lenRead)) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::TWO || (funcArg.GetArgc() == NARG_CNT::THREE &&
        !NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_READ_NAME, cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_READ_NAME, cbExec, cbCompl).val_;
    }
}

static NError WriteExec(shared_ptr<AsyncIOWrtieArg> arg, char *buf, size_t len, int32_t fd, int64_t offset)
{
    uv_buf_t buffer = uv_buf_init(buf, static_cast<unsigned int>(len));
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> write_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!write_req) {
        HILOGE("Failed to request heap memory.");
        return NError(ENOMEM);
    }
    int ret = uv_fs_write(nullptr, write_req.get(), fd, &buffer, 1, offset, nullptr);
    if (ret < 0) {
        HILOGE("Failed to write file for %{public}d", ret);
        return NError(ret);
    }
    arg->actLen = ret;
    return NError(ERRNO_NOERR);
}

napi_value PropNExporter::Write(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool succ = false;
    int32_t fd = 0;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ || fd < 0) {
        HILOGE("Invalid fd from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    unique_ptr<char[]> bufGuard = nullptr;
    void *buf = nullptr;
    size_t len = 0;
    int64_t offset = -1;
    tie(succ, bufGuard, buf, len, offset) =
        CommonFunc::GetWriteArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        HILOGE("Failed to resolve buf and options");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto arg = make_shared<AsyncIOWrtieArg>(move(bufGuard));
    if (!arg) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = [arg, buf, len, fd, offset]() -> NError {
        return WriteExec(arg, static_cast<char *>(buf), len, fd, offset);
    };

    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return { NVal::CreateInt64(env, static_cast<int64_t>(arg->actLen)) };
        }
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::TWO || (funcArg.GetArgc() == NARG_CNT::THREE &&
        !NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_WRITE_NAME, cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_WRITE_NAME, cbExec, cbCompl).val_;
    }
}

napi_value PropNExporter::WriteSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool succ = false;
    int32_t fd = 0;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ || fd < 0) {
        HILOGE("Invalid fd from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    void *buf = nullptr;
    size_t len = 0;
    int64_t offset = -1;
    unique_ptr<char[]> bufGuard = nullptr;
    tie(succ, bufGuard, buf, len, offset) =
        CommonFunc::GetWriteArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        HILOGE("Failed to resolve buf and options");
        return nullptr;
    }

    uv_buf_t buffer = uv_buf_init(static_cast<char *>(buf), static_cast<unsigned int>(len));
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> write_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!write_req) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    int ret = uv_fs_write(nullptr, write_req.get(), fd, &buffer, 1, offset, nullptr);
    if (ret < 0) {
        HILOGE("Failed to write file for %{public}d", ret);
        NError(ret).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, static_cast<int64_t>(ret)).val_;
}

bool PropNExporter::ExportSync()
{
    return exports_.AddProp(
        {
            NVal::DeclareNapiFunction("accessSync", AccessSync),
            NVal::DeclareNapiFunction("closeSync", Close::Sync),
            NVal::DeclareNapiFunction("copyFileSync", CopyFile::Sync),
            NVal::DeclareNapiFunction("fdatasyncSync", Fdatasync::Sync),
            NVal::DeclareNapiFunction("fdopenStreamSync", FdopenStream::Sync),
            NVal::DeclareNapiFunction("fsyncSync", Fsync::Sync),
            NVal::DeclareNapiFunction("listFileSync", ListFile::Sync),
            NVal::DeclareNapiFunction("lseek", Lseek::Sync),
            NVal::DeclareNapiFunction("lstatSync", Lstat::Sync),
            NVal::DeclareNapiFunction("mkdirSync", MkdirSync),
            NVal::DeclareNapiFunction("mkdtempSync", Mkdtemp::Sync),
            NVal::DeclareNapiFunction("moveFileSync", Move::Sync),
            NVal::DeclareNapiFunction("openSync", Open::Sync),
            NVal::DeclareNapiFunction("readSync", ReadSync),
            NVal::DeclareNapiFunction("readLinesSync", ReadLines::Sync),
            NVal::DeclareNapiFunction("readTextSync", ReadText::Sync),
            NVal::DeclareNapiFunction("renameSync", Rename::Sync),
            NVal::DeclareNapiFunction("rmdirSync", Rmdirent::Sync),
            NVal::DeclareNapiFunction("statSync", Stat::Sync),
            NVal::DeclareNapiFunction("truncateSync", Truncate::Sync),
            NVal::DeclareNapiFunction("unlinkSync", UnlinkSync),
            NVal::DeclareNapiFunction("utimes", Utimes::Sync),
            NVal::DeclareNapiFunction("writeSync", WriteSync),
            NVal::DeclareNapiFunction("createStreamSync", CreateStream::Sync),
            NVal::DeclareNapiFunction("createReadStream", CreateStreamRw::Read),
            NVal::DeclareNapiFunction("createWriteStream", CreateStreamRw::Write),
            NVal::DeclareNapiFunction("dup", Dup::Sync),
            NVal::DeclareNapiFunction("setxattrSync", Xattr::SetSync),
            NVal::DeclareNapiFunction("getxattrSync", Xattr::GetSync),
            NVal::DeclareNapiFunction("symlinkSync", Symlink::Sync),
            NVal::DeclareNapiFunction("moveDirSync", MoveDir::Sync),
            NVal::DeclareNapiFunction("copyDirSync", CopyDir::Sync),
            NVal::DeclareNapiFunction("createRandomAccessFileSync", CreateRandomAccessFile::Sync),
        });
}

bool PropNExporter::ExportAsync()
{
    return exports_.AddProp(
        {
            NVal::DeclareNapiFunction("access", Access),
            NVal::DeclareNapiFunction("fsync", Fsync::Async),
            NVal::DeclareNapiFunction("listFile", ListFile::Async),
            NVal::DeclareNapiFunction("lstat", Lstat::Async),
            NVal::DeclareNapiFunction("mkdir", Mkdir),
            NVal::DeclareNapiFunction("mkdtemp", Mkdtemp::Async),
            NVal::DeclareNapiFunction("moveFile", Move::Async),
            NVal::DeclareNapiFunction("open", Open::Async),
            NVal::DeclareNapiFunction("read", Read),
            NVal::DeclareNapiFunction("readLines", ReadLines::Async),
            NVal::DeclareNapiFunction("readText", ReadText::Async),
            NVal::DeclareNapiFunction("rename", Rename::Async),
            NVal::DeclareNapiFunction("rmdir", Rmdirent::Async),
            NVal::DeclareNapiFunction("stat", Stat::Async),
            NVal::DeclareNapiFunction("truncate", Truncate::Async),
            NVal::DeclareNapiFunction("unlink", Unlink),
            NVal::DeclareNapiFunction("write", Write),
            NVal::DeclareNapiFunction("close", Close::Async),
            NVal::DeclareNapiFunction("createStream", CreateStream::Async),
            NVal::DeclareNapiFunction("copyFile", CopyFile::Async),
            NVal::DeclareNapiFunction("fdatasync", Fdatasync::Async),
            NVal::DeclareNapiFunction("setxattr", Xattr::SetAsync),
            NVal::DeclareNapiFunction("getxattr", Xattr::GetAsync),
            NVal::DeclareNapiFunction("symlink", Symlink::Async),
            NVal::DeclareNapiFunction("moveDir", MoveDir::Async),
            NVal::DeclareNapiFunction("copyDir", CopyDir::Async),
            NVal::DeclareNapiFunction("createRandomAccessFile", CreateRandomAccessFile::Async),
            NVal::DeclareNapiFunction("fdopenStream", FdopenStream::Async),
#ifndef IOS_PLATFORM
            NVal::DeclareNapiFunction("createWatcher", Watcher::CreateWatcher),
#endif
        });
}

bool PropNExporter::Export()
{
    return ExportSync() && ExportAsync();
}

string PropNExporter::GetClassName()
{
    return PropNExporter::className_;
}

PropNExporter::PropNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}

PropNExporter::~PropNExporter() {}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS