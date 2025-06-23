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
#include "read_text.h"

#include <cinttypes>
#include <fcntl.h>
#include <sys/stat.h>
#include <tuple>
#include <unistd.h>

#include "common_func.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static tuple<bool, int64_t, bool, int64_t, unique_ptr<char[]>> GetReadTextArg(napi_env env, napi_value argOption)
{
    NVal op(env, argOption);
    int64_t offset = -1;
    int64_t len = 0;
    bool succ = false;
    bool hasLen = false;
    unique_ptr<char[]> encoding { new char[]{ "utf-8" } };

    if (op.HasProp("offset") && !op.GetProp("offset").TypeIs(napi_undefined)) {
        tie(succ, offset) = op.GetProp("offset").ToInt64();
        if (!succ || offset < 0) {
            HILOGE("Illegal option.offset parameter");
            return { false, offset, hasLen, len, nullptr };
        }
    }

    if (op.HasProp("length") && !op.GetProp("length").TypeIs(napi_undefined)) {
        tie(succ, len) = op.GetProp("length").ToInt64();
        if (!succ || len < 0 || len > UINT_MAX) {
            HILOGE("Illegal option.length parameter");
            return { false, offset, hasLen, len, nullptr };
        }
        hasLen = true;
    }

    if (op.HasProp("encoding")) {
        tie(succ, encoding, ignore) = op.GetProp("encoding").ToUTF8String("utf-8");
        string_view encodingStr(encoding.get());
        if (!succ || encodingStr != "utf-8") {
            HILOGE("Illegal option.encoding parameter");
            return { false, offset, hasLen, len, nullptr };
        }
    }

    return { true, offset, hasLen, len, move(encoding) };
}

static NError ReadTextAsync(const std::string &path, std::shared_ptr<AsyncReadTextArg> arg, int64_t offset,
                            bool hasLen, int64_t len)
{
    OHOS::DistributedFS::FDGuard sfd;
    struct stat statbf;
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> open_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!open_req) {
        HILOGE("Failed to request heap memory.");
        return NError(ENOMEM);
    }
    int ret = uv_fs_open(nullptr, open_req.get(), path.c_str(), O_RDONLY,
                         S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, nullptr);
    int err = errno;
    if (ret < 0) {
        HILOGE("Failed to open file, ret: %{public}d", ret);
        return NError(err);
    }

    sfd.SetFD(ret);
    if (sfd.GetFD() < 0) {
        HILOGE("Failed to open file by path");
        return NError(errno);
    }
    if (fstat(sfd.GetFD(), &statbf) < 0) {
        HILOGE("Failed to get stat of file by fd: %{public}d", sfd.GetFD());
        return NError(errno);
    }

    if (offset > statbf.st_size) {
        HILOGE("Invalid offset");
        return NError(EINVAL);
    }

    len = (!hasLen || len > statbf.st_size) ? statbf.st_size : len;
    string buffer(len, '\0');
    uv_buf_t readbuf = uv_buf_init(const_cast<char *>(buffer.c_str()), static_cast<unsigned int>(len));
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> read_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!read_req) {
        HILOGE("Failed to request heap memory.");
        return NError(ENOMEM);
    }
    arg->len = uv_fs_read(nullptr, read_req.get(), sfd.GetFD(), &readbuf, 1, offset, nullptr);
    err = errno;
    if (arg->len < 0) {
        HILOGE("Failed to read file by fd: %{public}d", sfd.GetFD());
        return NError(err);
    }
    arg->buffer = buffer;
    return NError(ERRNO_NOERR);
}

static int OpenFile(const std::string& path)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> open_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup
    };
    if (open_req == nullptr) {
        HILOGE("Failed to request heap memory.");
        return -ENOMEM;
    }

    return uv_fs_open(nullptr, open_req.get(), path.c_str(), O_RDONLY,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, nullptr);
}

static int ReadFromFile(int fd, int64_t offset, string& buffer)
{
    uv_buf_t readbuf = uv_buf_init(const_cast<char *>(buffer.c_str()), static_cast<unsigned int>(buffer.size()));
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> read_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (read_req == nullptr) {
        HILOGE("Failed to request heap memory.");
        return -ENOMEM;
    }
    return uv_fs_read(nullptr, read_req.get(), fd, &readbuf, 1, offset, nullptr);
}

napi_value ReadText::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetFirstArg, path, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8StringPath();
    if (!resGetFirstArg) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetReadTextArg, offset, hasLen, len, encoding] = GetReadTextArg(env, funcArg[NARG_POS::SECOND]);
    if (!resGetReadTextArg) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    OHOS::DistributedFS::FDGuard sfd;
    int fd = OpenFile(path.get());
    if (fd < 0) {
        HILOGD("Failed to open file by ret: %{public}d", fd);
        NError(fd).ThrowErr(env);
        return nullptr;
    }
    sfd.SetFD(fd);

    struct stat statbf;
    if ((!sfd) || (fstat(sfd.GetFD(), &statbf) < 0)) {
        HILOGE("Failed to get stat of file by fd: %{public}d", sfd.GetFD());
        NError(errno).ThrowErr(env);
        return nullptr;
    }

    if (offset > statbf.st_size) {
        HILOGE("Invalid offset: %{public}" PRIu64, offset);
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    len = (!hasLen || len > statbf.st_size) ? statbf.st_size : len;
    string buffer(len, '\0');
    int readRet = ReadFromFile(sfd.GetFD(), offset, buffer);
    if (readRet < 0) {
        HILOGE("Failed to read file by fd: %{public}d", fd);
        NError(readRet).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUTF8String(env, buffer.c_str(), readRet).val_;
}

napi_value ReadText::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetFirstArg, path, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8StringPath();
    if (!resGetFirstArg) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetSecondArg, offset, hasLen, len, encoding] = GetReadTextArg(env, funcArg[NARG_POS::SECOND]);
    if (!resGetSecondArg) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto arg = CreateSharedPtr<AsyncReadTextArg>(NVal(env, funcArg.GetThisVar()));
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [path = string(path.get()), arg, offset = offset, hasLen = hasLen, len = len]() -> NError {
        return ReadTextAsync(path, arg, offset, hasLen, len);
    };

    auto cbComplete = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return NVal::CreateUTF8String(env, arg->buffer.c_str(), arg->len);
        }
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE || (funcArg.GetArgc() == NARG_CNT::TWO &&
        !NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_READTEXT_NAME, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[((funcArg.GetArgc() == NARG_CNT::TWO) ? NARG_POS::SECOND : NARG_POS::THIRD)]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_READTEXT_NAME, cbExec, cbComplete).val_;
    }
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS