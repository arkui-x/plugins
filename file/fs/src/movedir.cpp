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

#include "movedir.h"

#include <dirent.h>
#include <deque>
#include <memory>
#include <string_view>
#include <tuple>
#include <unistd.h>

#include "common_func.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "uv.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

constexpr int FILE_PERMISSIONS = 0755;

static int RecurMoveDir(const string &srcPath, const string &destPath, const int mode,
    deque<struct ErrFiles> &errfiles);

static tuple<bool, bool> JudgeExistAndEmpty(const string &path)
{
    uv_fs_t statReq;
    int ret = uv_fs_stat(nullptr, &statReq, path.c_str(), nullptr);
    uv_fs_req_cleanup(&statReq);
    if (ret < 0) {
        HILOGE("Failed to get path status: %{public}s", uv_strerror(ret));
        return { false, false };
    }

    uv_fs_t readdirReq;
    ret = uv_fs_scandir(nullptr, &readdirReq, path.c_str(), 0, nullptr);
    uv_fs_req_cleanup(&readdirReq);

    if (ret < 0) {
        HILOGE("Failed to read directory: %{public}s", uv_strerror(ret));
        return { true, false };
    }
    if (readdirReq.result == 0) {
        return { true, true };
    }
    return { true, false };
}

static int RmDirectory(const string &path)
{
    uv_fs_t statReq;
    int ret = uv_fs_stat(nullptr, &statReq, path.c_str(), nullptr);
    uv_fs_req_cleanup(&statReq);

    if (ret < 0) {
        if (ret == UV_ENOENT) {
            HILOGE("Failed to stat directory %{public}s: %{public}s", path.c_str(), uv_strerror(ret));
            return ERRNO_NOERR;
        }
        HILOGE("Failed to stat directory %{public}s: %{public}s", path.c_str(), uv_strerror(ret));
        return ret;
    }

    uv_fs_t readdirReq;
    ret = uv_fs_scandir(nullptr, &readdirReq, path.c_str(), 0, nullptr);
    if (ret < 0) {
        HILOGE("Failed to read directory %{public}s: %{public}s", path.c_str(), uv_strerror(ret));
        uv_fs_req_cleanup(&readdirReq);
        return ret;
    }

    uv_dirent_t dent;
    while (uv_fs_scandir_next(&readdirReq, &dent) != UV_EOF) {
        string entryPath = path + "/" + string(dent.name);
        if (dent.type == UV_DIRENT_DIR) {
            int res = RmDirectory(entryPath);
            if (res != ERRNO_NOERR) {
                HILOGE("Failed to remove directory %{public}s: %{public}s", path.c_str(), uv_strerror(ret));
                uv_fs_req_cleanup(&readdirReq);
                return res;
            }
        } else {
            uv_fs_t unlinkReq;
            ret = uv_fs_unlink(nullptr, &unlinkReq, entryPath.c_str(), nullptr);
            uv_fs_req_cleanup(&unlinkReq);
            if (ret < 0) {
                HILOGE("Failed to unlink file %{public}s: %{public}s", entryPath.c_str(), uv_strerror(ret));
                uv_fs_req_cleanup(&readdirReq);
                return ret;
            }
        }
    }

    uv_fs_req_cleanup(&readdirReq);
    uv_fs_t rmdirReq;
    ret = uv_fs_rmdir(nullptr, &rmdirReq, path.c_str(), nullptr);
    uv_fs_req_cleanup(&rmdirReq);
    if (ret < 0) {
        HILOGE("Failed to remove directory %{public}s: %{public}s", path.c_str(), uv_strerror(ret));
        return ret;
    }
    return ERRNO_NOERR;
}
static int RemovePath(const string& pathStr)
{
    uv_fs_t statReq;
    int ret = uv_fs_stat(nullptr, &statReq, pathStr.c_str(), nullptr);
    uv_fs_req_cleanup(&statReq);

    if (ret < 0) {
        if (ret == UV_ENOENT) {
            return ERRNO_NOERR;
        }
        HILOGE("Failed to stat path: %{public}s", uv_strerror(ret));
        return ret;
    }
    if (statReq.statbuf.st_mode & S_IFDIR) {
        return RmDirectory(pathStr);
    } else {
        uv_fs_t unlinkReq;
        ret = uv_fs_unlink(nullptr, &unlinkReq, pathStr.c_str(), nullptr);
        uv_fs_req_cleanup(&unlinkReq);
        if (ret < 0) {
            HILOGE("Failed to unlink file: %{public}s", uv_strerror(ret));
            return ret;
        }
    }
    return ERRNO_NOERR;
}

static tuple<bool, unique_ptr<char[]>, unique_ptr<char[]>, int> ParseJsOperand(napi_env env, const NFuncArg& funcArg)
{
    auto [resGetFirstArg, src, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8StringPath();
    uv_fs_t statReq;
    int ret = uv_fs_stat(nullptr, &statReq, src.get(), nullptr);
    uv_fs_req_cleanup(&statReq);
    if (!resGetFirstArg || ret < 0 || !(statReq.statbuf.st_mode & S_IFDIR)) {
        HILOGE("Invalid src: %{public}s", uv_strerror(ret));
        return { false, nullptr, nullptr, 0 };
    }

    auto [resGetSecondArg, dest, unused] = NVal(env, funcArg[NARG_POS::SECOND]).ToUTF8StringPath();
    ret = uv_fs_stat(nullptr, &statReq, dest.get(), nullptr);
    uv_fs_req_cleanup(&statReq);
    if (!resGetSecondArg || ret < 0 || !(statReq.statbuf.st_mode & S_IFDIR)) {
        HILOGE("Invalid dest: %{public}s", uv_strerror(ret));
        return { false, nullptr, nullptr, 0 };
    }

    int mode = 0;
    if (funcArg.GetArgc() >= NARG_CNT::THREE) {
        bool resGetThirdArg = false;
        tie(resGetThirdArg, mode) = NVal(env, funcArg[NARG_POS::THIRD]).ToInt32(mode);
        if (!resGetThirdArg || (mode < DIRMODE_MIN || mode > DIRMODE_MAX)) {
            HILOGE("Invalid mode");
            return { false, nullptr, nullptr, 0 };
        }
    }
    return { true, move(src), move(dest), mode };
}

static int CopyAndDeleteFile(const string &src, const string &dest)
{
    uv_fs_t statReq;
    int ret = uv_fs_stat(nullptr, &statReq, dest.c_str(), nullptr);
    uv_fs_req_cleanup(&statReq);
    if (ret == 0) {
        int removeRes = RemovePath(dest);
        if (removeRes != 0) {
            HILOGE("Failed to remove dest file");
            return removeRes;
        }
    }

    uv_fs_t copyfile_req;
    ret = uv_fs_copyfile(nullptr, &copyfile_req, src.c_str(), dest.c_str(), 0, nullptr);
    uv_fs_req_cleanup(&copyfile_req);
    if (ret < 0) {
        HILOGE("Failed to copy file: %{public}s", uv_strerror(ret));
        return ret;
    }

    return RemovePath(src);
}

static int RenameFile(const string &src, const string &dest, const int mode, deque<struct ErrFiles> &errfiles)
{
    uv_fs_t statReq;
    int ret = uv_fs_stat(nullptr, &statReq, dest.c_str(), nullptr);
    uv_fs_req_cleanup(&statReq);
    if (ret == 0) {
        if (statReq.statbuf.st_mode & S_IFDIR) {
            errfiles.emplace_front(src, dest);
            return ERRNO_NOERR;
        }
        if (mode == DIRMODE_FILE_THROW_ERR) {
            errfiles.emplace_back(src, dest);
            return ERRNO_NOERR;
        }
    }

    uv_fs_t renameReq;
    ret = uv_fs_rename(nullptr, &renameReq, src.c_str(), dest.c_str(), nullptr);
    uv_fs_req_cleanup(&renameReq);
    if (ret == UV_EXDEV) {
        return CopyAndDeleteFile(src, dest);
    }
    if (ret < 0) {
        HILOGE("Failed to rename file: %{public}s", uv_strerror(ret));
        return ret;
    }
    return ERRNO_NOERR;
}

static int32_t FilterFunc(const struct dirent *filename)
{
    if (string_view(filename->d_name) == "." || string_view(filename->d_name) == "..") {
        return FILE_DISMATCH;
    }
    return FILE_MATCH;
}

static int RenameDir(const string &src, const string &dest, const int mode, deque<struct ErrFiles> &errfiles)
{
    uv_fs_t statReq;
    int ret = uv_fs_stat(nullptr, &statReq, dest.c_str(), nullptr);
    uv_fs_req_cleanup(&statReq);
    if (ret == 0) {
        return RecurMoveDir(src, dest, mode, errfiles);
    } else if (ret != UV_ENOENT) {
        HILOGE("Failed to stat destination path: %{public}s", uv_strerror(ret));
        return ret;
    }

    uv_fs_t renameReq;
    ret = uv_fs_rename(nullptr, &renameReq, src.c_str(), dest.c_str(), nullptr);
    uv_fs_req_cleanup(&renameReq);
    if (ret == UV_EXDEV) {
        uv_fs_t mkdirReq;
        ret = uv_fs_mkdir(nullptr, &mkdirReq, dest.c_str(), FILE_PERMISSIONS, nullptr);
        uv_fs_req_cleanup(&mkdirReq);
        if (ret < 0) {
            HILOGE("Failed to create directory: %{public}s", uv_strerror(ret));
            return ret;
        }
        return RecurMoveDir(src, dest, mode, errfiles);
    } else if (ret < 0) {
        HILOGE("Failed to rename directory: %{public}s", uv_strerror(ret));
        return ret;
    }
    return ERRNO_NOERR;
}

struct NameListArg {
    struct dirent** namelist;
    int num;
};

static void Deleter(struct NameListArg *arg)
{
    for (int i = 0; i < arg->num; i++) {
        free((arg->namelist)[i]);
        (arg->namelist)[i] = nullptr;
    }
    free(arg->namelist);
    arg->namelist = nullptr;
    delete arg;
    arg = nullptr;
}

static int RecurMoveDir(const string &srcPath, const string &destPath, const int mode,
    deque<struct ErrFiles> &errfiles)
{
    uv_fs_t statReq;
    int ret = uv_fs_stat(nullptr, &statReq, destPath.c_str(), nullptr);
    uv_fs_req_cleanup(&statReq);
    if (ret < 0 || !(statReq.statbuf.st_mode & S_IFDIR)) {
        errfiles.emplace_front(srcPath, destPath);
        return ERRNO_NOERR;
    }

    uv_fs_t readdirReq;
    ret = uv_fs_scandir(nullptr, &readdirReq, srcPath.c_str(), 0, nullptr);
    if (ret < 0) {
        HILOGE("Failed to read directory '%{public}s': %{public}s", srcPath.c_str(), uv_strerror(ret));
        return ret;
    }

    uv_dirent_t dent;
    while (uv_fs_scandir_next(&readdirReq, &dent) != UV_EOF) {
        string entryName = string(dent.name);
        if (dent.type == UV_DIRENT_DIR) {
            string srcTemp = srcPath + '/' + entryName;
            string destTemp = destPath + '/' + entryName;
            size_t size = errfiles.size();
            int res = RenameDir(srcTemp, destTemp, mode, errfiles);
            if (res != ERRNO_NOERR) {
                return res;
            }
            if (size != errfiles.size()) {
                continue;
            }
            res = RemovePath(srcTemp);
            if (res) {
                return res;
            }
        } else {
            string src = srcPath + '/' + entryName;
            string dest = destPath + '/' + entryName;
            int res = RenameFile(src, dest, mode, errfiles);
            if (res != ERRNO_NOERR) {
                HILOGE("Failed to rename file for error %{public}d", res);
                return res;
            }
        }
    }
    uv_fs_req_cleanup(&readdirReq);
    return ERRNO_NOERR;
}

static int MoveDirFunc(const string &src, const string &dest, const int mode, deque<struct ErrFiles> &errfiles)
{
    size_t found = string(src).rfind('/');
    if (found == std::string::npos) {
        return EINVAL;
    }

    uv_fs_t accessReq;
    int ret = uv_fs_access(nullptr, &accessReq, src.c_str(), W_OK, nullptr);
    uv_fs_req_cleanup(&accessReq);
    if (ret != 0) {
        HILOGE("Failed to move src directory due to doesn't exist or hasn't write permission");
        return ret;
    }

    string dirName = string(src).substr(found);
    string destStr = dest + dirName;
    auto [destStrExist, destStrEmpty] = JudgeExistAndEmpty(destStr);
    if (destStrExist && !destStrEmpty) {
        if (mode == DIRMODE_DIRECTORY_REPLACE) {
            int removeRes = RmDirectory(destStr);
            if (removeRes) {
                HILOGE("Failed to remove dest directory in DIRMODE_DIRECTORY_REPLACE");
                return removeRes;
            }
        }
        if (mode == DIRMODE_DIRECTORY_THROW_ERR) {
            HILOGE("Failed to move directory in DIRMODE_DIRECTORY_THROW_ERR");
            return ENOTEMPTY;
        }
    }
    int res = RenameDir(src, destStr, mode, errfiles);
    if (res == ERRNO_NOERR) {
        if (!errfiles.empty()) {
            HILOGE("Failed to movedir with some conflicted files");
            return EEXIST;
        }
        int removeRes = RmDirectory(src);
        if (removeRes) {
            HILOGE("Failed to remove src directory");
            return removeRes;
        }
    }
    return res;
}

static napi_value GetErrData(napi_env env, deque<struct ErrFiles> &errfiles)
{
    napi_value res = nullptr;
    napi_status status = napi_create_array(env, &res);
    if (status != napi_ok) {
        HILOGE("Failed to create array");
        return nullptr;
    }
    size_t index = 0;
    for (auto &iter : errfiles) {
        NVal obj = NVal::CreateObject(env);
        obj.AddProp("srcFile", NVal::CreateUTF8String(env, iter.srcFiles).val_);
        obj.AddProp("destFile", NVal::CreateUTF8String(env, iter.destFiles).val_);
        status = napi_set_element(env, res, index++, obj.val_);
        if (status != napi_ok) {
            HILOGE("Failed to set element on data");
            return nullptr;
        }
    }
    return res;
}

napi_value MoveDir::Sync(napi_env env, napi_callback_info info)
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

    deque<struct ErrFiles> errfiles = {};
    int ret = MoveDirFunc(src.get(), dest.get(), mode, errfiles);
    if (ret == EEXIST) {
        NError(ret).ThrowErrAddData(env, EEXIST, GetErrData(env, errfiles));
        return nullptr;
    } else if (ret) {
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

struct MoveDirArgs {
    deque<ErrFiles> errfiles;
    int errNo = 0;
    ~MoveDirArgs() = default;
};

napi_value MoveDir::Async(napi_env env, napi_callback_info info)
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
    auto arg = CreateSharedPtr<MoveDirArgs>();
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [srcPath = string(src.get()), destPath = string(dest.get()), mode = mode, arg]() -> NError {
        arg->errNo = MoveDirFunc(srcPath, destPath, mode, arg->errfiles);
        if (arg->errNo) {
            return NError(arg->errNo);
        }
        return NError(ERRNO_NOERR);
    };

    auto cbComplCallback = [arg, mode = mode](napi_env env, NError err) -> NVal {
        if (arg->errNo == EEXIST) {
            napi_value data = err.GetNapiErr(env);
            napi_status status = napi_set_named_property(env, data, FILEIO_TAG_ERR_DATA.c_str(),
                GetErrData(env, arg->errfiles));
            if (status != napi_ok) {
                HILOGE("Failed to set data property on Error");
            }
            return { env, data };
        } else if (arg->errNo) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateUndefined(env);
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::TWO || (funcArg.GetArgc() == NARG_CNT::THREE &&
            !NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_MOVEDIR_NAME, cbExec, cbComplCallback).val_;
    } else {
        int cbIdex = ((funcArg.GetArgc() == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH);
        NVal cb(env, funcArg[cbIdex]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_MOVEDIR_NAME, cbExec, cbComplCallback).val_;
    }
}

} // ModuleFileIO
} // FileManagement
} // OHOS