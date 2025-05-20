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

#include "copydir.h"

#include <dirent.h>
#include <memory>
#include <string>
#include <tuple>
#include <uv.h>
#include <vector>
#include <sys/stat.h>

#include "common_func.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static int RecurCopyDir(const string &srcPath, const string &destPath, const int mode,
    vector<struct ConflictFiles> &errfiles);

static bool EndWithSlash(const string &src)
{
    if (src.empty()) {
        return false;
    }
    return src.back() == '/';
}

static bool IsDirectory(const std::string &path)
{
    uv_fs_t statReq;
    int ret = uv_fs_stat(nullptr, &statReq, path.c_str(), nullptr);
    uv_fs_req_cleanup(&statReq);
    if (ret < 0) {
        HILOGE("Failed to get path stat, error msg: %{public}s", uv_strerror(ret));
        return false;
    }
    return S_ISDIR(statReq.statbuf.st_mode);
}

static std::string GetParentPath(const std::string &path)
{
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos) {
        return "";
    }
    return path.substr(0, pos);
}

static bool PathExists(const std::string &path)
{
    uv_fs_t statReq;
    int ret = uv_fs_stat(nullptr, &statReq, path.c_str(), nullptr);
    uv_fs_req_cleanup(&statReq);
    if (ret < 0) {
        HILOGE("Failed to get path stat, error msg: %{public}s", uv_strerror(ret));
        return false;
    }
    return true;
}

static bool AllowToCopy(const std::string &src, const std::string &dest)
{
    if (src == dest) {
        HILOGE("Failed to copy file, the same path");
        return false;
    }
    if (EndWithSlash(src) ? dest.find(src) == 0 : dest.find(src + "/") == 0) {
        HILOGE("Failed to copy file, dest is under src");
        return false;
    }
    if (GetParentPath(src) == dest) {
        HILOGE("Failed to copy file, src's parent path is dest");
        return false;
    }
    return true;
}

static std::tuple<bool, std::unique_ptr<char[]>, std::unique_ptr<char[]>, int> ParseAndCheckJsOperand(
    napi_env env, const NFuncArg &funcArg)
{
    auto [resGetFirstArg, src, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8StringPath();
    if (!resGetFirstArg || !IsDirectory(src.get())) {
        HILOGE("Invalid src");
        return { false, nullptr, nullptr, 0 };
    }
    auto [resGetSecondArg, dest, unused] = NVal(env, funcArg[NARG_POS::SECOND]).ToUTF8StringPath();
    if (!resGetSecondArg || !IsDirectory(dest.get())) {
        HILOGE("Invalid dest");
        return { false, nullptr, nullptr, 0 };
    }
    if (!AllowToCopy(src.get(), dest.get())) {
        return { false, nullptr, nullptr, 0 };
    }
    int mode = 0;
    if (funcArg.GetArgc() >= NARG_CNT::THREE) {
        bool resGetThirdArg = false;
        std::tie(resGetThirdArg, mode) = NVal(env, funcArg[NARG_POS::THIRD]).ToInt32(mode);
        if (!resGetThirdArg || (mode < COPYMODE_MIN || mode > COPYMODE_MAX)) {
            HILOGE("Invalid mode");
            return { false, nullptr, nullptr, 0 };
        }
    }
    return { true, std::move(src), std::move(dest), mode };
}

struct NameList {
    struct dirent** namelist = { nullptr };
    int direntNum = 0;
};

static int MakeDir(const string &path)
{
    uv_fs_t mkdirReq;
    int ret = uv_fs_mkdir(nullptr, &mkdirReq, path.c_str(), 0755, nullptr);
    uv_fs_req_cleanup(&mkdirReq);
    if (ret < 0) {
        HILOGE("Failed to create directory, error msg: %{public}s", uv_strerror(ret));
        return ret;
    }
    return ERRNO_NOERR;
}

static int RemoveFile(const string& destPath)
{
    uv_fs_t unlinkReq;
    int ret = uv_fs_unlink(nullptr, &unlinkReq, destPath.c_str(), nullptr);
    uv_fs_req_cleanup(&unlinkReq);
    if (ret < 0) {
        HILOGE("Failed to remove file with path, error msg:  %{public}s", uv_strerror(ret));
        return ret;
    }
    return ERRNO_NOERR;
}

static void Deleter(struct NameList *arg)
{
    for (int i = 0; i < arg->direntNum; i++) {
        free((arg->namelist)[i]);
        (arg->namelist)[i] = nullptr;
    }
    free(arg->namelist);
    arg->namelist = nullptr;
    delete arg;
    arg = nullptr;
}

static int CheckAndRemoveExistingDest(const std::string &dest, int mode)
{
    uv_fs_t stat_req;
    int ret = uv_fs_stat(nullptr, &stat_req, dest.c_str(), nullptr);
    uv_fs_req_cleanup(&stat_req);

    if (ret == 0) {
        if (mode == DIRMODE_FILE_COPY_THROW_ERR) {
            HILOGE("Failed to copy file due to existing destPath with throw err");
            return EEXIST;
        } else {
            int removeRes = RemoveFile(dest);
            if (removeRes != ERRNO_NOERR) {
                HILOGE("Failed to remove dest file");
                return removeRes;
            }
        }
    }
    return ERRNO_NOERR;
}

#ifdef IOS_PLATFORM
mode_t GetFileMode(const string &src)
{
    uv_fs_t src_stat_req;
    int ret = uv_fs_stat(nullptr, &src_stat_req, src.c_str(), nullptr);
    if (ret < 0) {
        HILOGE("Failed to stat source file: %s", uv_strerror(ret));
        uv_fs_req_cleanup(&src_stat_req);
        return ret;
    }

    uv_stat_t* src_stat = static_cast<uv_stat_t*>(src_stat_req.ptr);
    if (!(src_stat->st_mode & S_IFREG)) {
        HILOGE("Source is not a regular file");
        uv_fs_req_cleanup(&src_stat_req);
        return -EINVAL;
    }
    mode_t src_mode = src_stat->st_mode & 0777;
    uv_fs_req_cleanup(&src_stat_req);
    return src_mode;
}

static int SafeClose(uv_file fd1, uv_file fd2)
{
    uv_fs_t req1;
    uv_fs_t req2;
    if (fd1 >= 0) {
        uv_fs_close(nullptr, &req1, fd1, nullptr);
        uv_fs_req_cleanup(&req1);
    }
    if (fd2 >= 0) {
        uv_fs_close(nullptr, &req2, fd2, nullptr);
        uv_fs_req_cleanup(&req2);
    }
    return ERRNO_NOERR;
}

static uv_file OpenFile(const string& path, int flags, mode_t mode)
{
    uv_fs_t open_req;
    uv_file fd = uv_fs_open(nullptr, &open_req, path.c_str(), flags, mode, nullptr);
    uv_fs_req_cleanup(&open_req);
    return fd;
}

static int HandleIOError(uv_file src, uv_file dest, const char* msg, int err)
{
    HILOGE("%s: %s", msg, uv_strerror(err));
    SafeClose(src, dest);
    return err;
}

static int CopyFile(const string &src, const string &dest, int mode)
{
    int check = CheckAndRemoveExistingDest(dest, mode);
    if (check != ERRNO_NOERR) return check;

    uv_file src_fd = OpenFile(src, O_RDONLY, 0);
    if (src_fd < 0)
        return HandleIOError(-1, -1, "Open source failed", src_fd);

    uv_file dest_fd = OpenFile(dest, O_WRONLY|O_CREAT|O_TRUNC, GetFileMode(src));
    if (dest_fd < 0)
        return HandleIOError(src_fd, -1, "Create dest failed", dest_fd);

    char buffer[4096];
    uv_buf_t buf;

    while (true) {
        buf = uv_buf_init(buffer, sizeof(buffer));
        uv_fs_t read_req;
        ssize_t read = uv_fs_read(nullptr, &read_req, src_fd, &buf, 1, -1, nullptr);
        uv_fs_req_cleanup(&read_req);
        if (read < 0)
            return HandleIOError(src_fd, dest_fd, "Read failed", read);
        if (read == 0)
            break;

        buf = uv_buf_init(buffer, read);
        uv_fs_t write_req;
        ssize_t written = uv_fs_write(nullptr, &write_req, dest_fd, &buf, 1, -1, nullptr);
        uv_fs_req_cleanup(&write_req);
        if (written < 0)
            return HandleIOError(src_fd, dest_fd, "Write failed", written);
        if (written != read)
            return HandleIOError(src_fd, dest_fd, "Partial write", -EIO);
    }

    return SafeClose(src_fd, dest_fd);
}
#else
static int CopyFile(const string &src, const string &dest, const int mode)
{
    int checkResult = CheckAndRemoveExistingDest(dest, mode);
    if (checkResult != ERRNO_NOERR) {
        return checkResult;
    }
    uv_fs_t copyfile_req;
    int ret = uv_fs_copyfile(nullptr, &copyfile_req, src.c_str(), dest.c_str(), 0, nullptr);
    uv_fs_req_cleanup(&copyfile_req);
    if (ret < 0) {
        HILOGE("Failed to copy file: %s", uv_strerror(ret));
        return ret;
    }
    return ERRNO_NOERR;
}
#endif

static int CopySubDir(const std::string& srcPath, const std::string& destPath, const int mode,
                      std::vector<struct ConflictFiles>& errfiles)
{
    if (!PathExists(destPath)) {
        int res = MakeDir(destPath);
        if (res != ERRNO_NOERR) {
            HILOGE("Failed to mkdir");
            return res;
        }
    } else if (!IsDirectory(destPath)) {
        HILOGE("Destination path is not a directory");
        return EINVAL;
    }
    return RecurCopyDir(srcPath, destPath, mode, errfiles);
}

static int FilterFunc(const struct dirent *filename)
{
    if (string_view(filename->d_name) == "." || string_view(filename->d_name) == "..") {
        return DISMATCH;
    }
    return MATCH;
}

static int RecurCopyDir(const string &srcPath, const string &destPath, const int mode,
                        vector<struct ConflictFiles> &errfiles)
{
    unique_ptr<struct NameList, decltype(Deleter)*> pNameList = {new (nothrow) struct NameList, Deleter};
    if (pNameList == nullptr) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int num = scandir(srcPath.c_str(), &(pNameList->namelist), FilterFunc, alphasort);
    if (num < 0) {
        HILOGE("scandir fail errno is %{public}d", errno);
        return errno;
    }
    pNameList->direntNum = num;
    for (int i = 0; i < num; i++) {
        if ((pNameList->namelist[i])->d_type == DT_DIR) {
            string srcTemp = srcPath + '/' + (pNameList->namelist[i])->d_name;
            string destTemp = destPath + '/' + (pNameList->namelist[i])->d_name;
            int res = CopySubDir(srcTemp, destTemp, mode, errfiles);
            if (res == ERRNO_NOERR) {
                continue;
            }
            return res;
        } else {
            string src = srcPath + '/' + (pNameList->namelist[i])->d_name;
            string dest = destPath + '/' + (pNameList->namelist[i])->d_name;
            int res = CopyFile(src, dest, mode);
            if (res == EEXIST) {
                errfiles.emplace_back(src, dest);
                continue;
            } else if (res == ERRNO_NOERR) {
                continue;
            } else {
                HILOGE("Failed to copy file for error %{public}d", res);
                return res;
            }
        }
    }
    return ERRNO_NOERR;
}

static int CopyDirFunc(const std::string &src, const std::string &dest, const int mode,
                       std::vector<struct ConflictFiles> &errfiles)
{
    size_t found = src.rfind('/');
    if (found == std::string::npos) {
        return EINVAL;
    }
    std::string dirName = src.substr(found);
    std::string destStr = dest + dirName;

    if (!PathExists(destStr)) {
        int res = MakeDir(destStr);
        if (res != ERRNO_NOERR) {
            HILOGE("Failed to mkdir");
            return res;
        }
    } else if (!IsDirectory(destStr)) {
        HILOGE("Destination path is not a directory");
        return EINVAL;
    }

    int res = RecurCopyDir(src, destStr, mode, errfiles);
    if (!errfiles.empty() && res == ERRNO_NOERR) {
        return EEXIST;
    }
    return res;
}

static napi_value PushErrFilesInData(napi_env env, vector<struct ConflictFiles> &errfiles)
{
    napi_value res = nullptr;
    napi_status status = napi_create_array(env, &res);
    if (status != napi_ok) {
        HILOGE("Failed to creat array");
        return nullptr;
    }
    for (size_t i = 0; i < errfiles.size(); i++) {
        NVal obj = NVal::CreateObject(env);
        obj.AddProp("srcFile", NVal::CreateUTF8String(env, errfiles[i].srcFiles).val_);
        obj.AddProp("destFile", NVal::CreateUTF8String(env, errfiles[i].destFiles).val_);
        status = napi_set_element(env, res, i, obj.val_);
        if (status != napi_ok) {
            HILOGE("Failed to set element on data");
            return nullptr;
        }
    }
    return res;
}

napi_value CopyDir::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succ, src, dest, mode] = ParseAndCheckJsOperand(env, funcArg);
    if (!succ) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    vector<struct ConflictFiles> errfiles = {};
    int ret = CopyDirFunc(src.get(), dest.get(), mode, errfiles);
    if (ret == EEXIST && mode == DIRMODE_FILE_COPY_THROW_ERR) {
        NError(ret).ThrowErrAddData(env, EEXIST, PushErrFilesInData(env, errfiles));
        return nullptr;
    } else if (ret) {
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

struct CopyDirArgs {
    vector<ConflictFiles> errfiles;
    int errNo = ERRNO_NOERR;
};

napi_value CopyDir::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succ, src, dest, mode] = ParseAndCheckJsOperand(env, funcArg);
    if (!succ) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto arg = CreateSharedPtr<CopyDirArgs>();
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [srcPath = string(src.get()), destPath = string(dest.get()), mode = mode, arg]() -> NError {
        arg->errNo = CopyDirFunc(srcPath, destPath, mode, arg->errfiles);
        if (arg->errNo) {
            return NError(arg->errNo);
        }
        return NError(ERRNO_NOERR);
    };

    auto cbComplCallback = [arg, mode = mode](napi_env env, NError err) -> NVal {
        if (arg->errNo == EEXIST && mode == DIRMODE_FILE_COPY_THROW_ERR) {
            napi_value data = err.GetNapiErr(env);
            napi_status status = napi_set_named_property(env, data, FILEIO_TAG_ERR_DATA.c_str(),
                PushErrFilesInData(env, arg->errfiles));
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
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_COPYDIR_NAME, cbExec, cbComplCallback).val_;
    } else {
        int cbIdex = ((funcArg.GetArgc() == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH);
        NVal cb(env, funcArg[cbIdex]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_COPYDIR_NAME, cbExec, cbComplCallback).val_;
    }
}

} // ModuleFileIO
} // FileManagement
} // OHOS