/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <cstring>
#include <fcntl.h>
#include <tuple>
#include <unistd.h>

#include "common/napi/n_async/n_async_work_callback.h"
#include "common/napi/n_async/n_async_work_promise.h"
#include "common/napi/n_func_arg.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

int AdaptToAbi(int &flags)
{
    static constexpr int USR_O_RDONLY = 00;
    static constexpr int USR_O_WRONLY = 01;
    static constexpr int USR_O_RDWR = 02;
    static constexpr int USR_O_CREAT = 0100;
    static constexpr int USR_O_EXCL = 0200;
    static constexpr int USR_O_TRUNC = 01000;
    static constexpr int USR_O_APPEND = 02000;
    static constexpr int USR_O_NONBLOCK = 04000;
    static constexpr int USR_O_DIRECTORY = 0200000;
    static constexpr int USR_O_NOFOLLOW = 0400000;
    static constexpr int USR_O_SYNC = 04010000;

    int flagsABI = 0;
    flagsABI |= ((flags & USR_O_RDONLY) == USR_O_RDONLY) ? O_RDONLY : 0;
    flagsABI |= ((flags & USR_O_WRONLY) == USR_O_WRONLY) ? O_WRONLY : 0;
    flagsABI |= ((flags & USR_O_RDWR) == USR_O_RDWR) ? O_RDWR : 0;
    flagsABI |= ((flags & USR_O_CREAT) == USR_O_CREAT) ? O_CREAT : 0;
    flagsABI |= ((flags & USR_O_EXCL) == USR_O_EXCL) ? O_EXCL : 0;
    flagsABI |= ((flags & USR_O_TRUNC) == USR_O_TRUNC) ? O_TRUNC : 0;
    flagsABI |= ((flags & USR_O_APPEND) == USR_O_APPEND) ? O_APPEND : 0;
    flagsABI |= ((flags & USR_O_NONBLOCK) == USR_O_NONBLOCK) ? O_NONBLOCK : 0;
    flagsABI |= ((flags & USR_O_DIRECTORY) == USR_O_DIRECTORY) ? O_DIRECTORY : 0;
    flagsABI |= ((flags & USR_O_NOFOLLOW) == USR_O_NOFOLLOW) ? O_NOFOLLOW : 0;
    flagsABI |= ((flags & USR_O_SYNC) == USR_O_SYNC) ? O_SYNC : 0;
    flags = flagsABI;
    return flagsABI;
}

static std::string BuildFullPath(const std::string& path)
{
    if (path.empty()) {
        return std::string("");
    }

    auto retPath = path;
    return retPath;
}

napi_value Open::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    unique_ptr<char[]> path;
    tie(succ, path, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    int flags = O_RDONLY;
    if (funcArg.GetArgc() >= NARG_CNT::TWO) {
        tie(succ, flags) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Invalid flags");
            return nullptr;
        }
    }
    (void)AdaptToAbi(flags);

    int fd = -1;
    size_t argc = funcArg.GetArgc();
    auto fullPath = BuildFullPath(std::string(path.get()));
    if (argc != NARG_CNT::THREE) {
        size_t flagsFirst { flags };
#ifdef IOS_PLATFORM
        if (flagsFirst & O_CREAT) {
#else
        if ((flagsFirst & O_CREAT) || (flagsFirst & O_TMPFILE)) {
#endif
            UniError(EINVAL).ThrowErr(env, "called with O_CREAT/O_TMPFILE but no mode");
            return nullptr;
        }
        fd = open(fullPath.c_str(), flags);
    } else {
        int mode;
        tie(succ, mode) = NVal(env, funcArg.GetArg(NARG_POS::THIRD)).ToInt32();
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Invalid mode");
            return nullptr;
        }
        fd = open(fullPath.c_str(), flags, mode);
    }

    if (fd == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, fd).val_;
}

static UniError DoOpenExec(const std::string& path, const int flags, const int mode, shared_ptr<int32_t> arg)
{
    auto fullPath = BuildFullPath(path);
    int ret = open(fullPath.c_str(), flags, mode);
    *arg = ret;
    if (ret == -1) {
        return UniError(errno);
    } else {
        return UniError(ERRNO_NOERR);
    }
}

napi_value Open::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::FOUR)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    bool succ = false;
    size_t argc = funcArg.GetArgc();
    unique_ptr<char[]> path;
    tie(succ, path, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }
    int flags = O_RDONLY;
    if (funcArg.GetArgc() >= NARG_CNT::TWO && !NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function)) {
        tie(succ, flags) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Invalid flags");
            return nullptr;
        }
    }
    (void)AdaptToAbi(flags);
    int mode = 0;
    if (argc == NARG_CNT::FOUR ||
        (argc == NARG_CNT::THREE && NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_number))) {
        tie(succ, mode) = NVal(env, funcArg[NARG_POS::THIRD]).ToInt32();
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Invalid mode");
            return nullptr;
        }
    }
    auto arg = make_shared<int32_t>();
    auto cbExec = [path = string(path.get()), flags, mode, arg](napi_env env) -> UniError {
        return DoOpenExec(path, flags, mode, arg);
    };
    auto cbComplCallback = [arg](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt64(env, *arg) };
    };
    NVal thisVar(env, funcArg.GetThisVar());
    if (argc == NARG_CNT::ONE || (argc == NARG_CNT::TWO && NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_number)) ||
        (argc == NARG_CNT::THREE && (NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_number)))) {
        return NAsyncWorkPromise(env, thisVar).Schedule("FileIOOpen", cbExec, cbComplCallback).val_;
    } else {
        size_t cbIdx = argc - 1;
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule("FileIOOpen", cbExec, cbComplCallback).val_;
    }
}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
