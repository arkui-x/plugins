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

#include "prop_n_exporter.h"

#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <unistd.h>

#include "../common_func.h"
#include "close.h"
#include "fstat.h"
#include "log.h"
#include "n_func_arg.h"
#include "n_async_work_promise.h"
#include "n_async_work_callback.h"
#include "open.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

struct AsyncIOReadArg {
    ssize_t lenRead { 0 };
    int offset { 0 };
    NRef refReadBuf;

    explicit AsyncIOReadArg(NVal jsReadBuf) : refReadBuf(jsReadBuf) {}
    ~AsyncIOReadArg() = default;
};

napi_value PropNExporter::Read(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    void *buf = nullptr;
    int64_t len;
    int fd;
    bool hasPos = false;
    int64_t pos;
    int offset;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    tie(succ, buf, len, hasPos, pos, offset) =
        CommonFunc::GetReadArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Failed GetReadArg");
        return nullptr;
    }

    auto arg = make_shared<AsyncIOReadArg>(NVal(env, funcArg[NARG_POS::SECOND]));
    auto cbExec = [arg, buf, len, fd, hasPos, pos, offset](napi_env env) -> UniError {
        ssize_t actLen;
        if (hasPos) {
            actLen = pread(fd, buf, len, pos);
        } else {
            actLen = read(fd, buf, len);
        }
        if (actLen == -1) {
            return UniError(errno);
        } else {
            arg->lenRead = actLen;
            arg->offset = offset;
            return UniError(ERRNO_NOERR);
        }
    };

    auto cbCompl = [arg](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        NVal obj = NVal::CreateObject(env);
        obj.AddProp(
            {
                NVal::DeclareNapiProperty("bytesRead", NVal::CreateInt64(env, arg->lenRead).val_),
                NVal::DeclareNapiProperty("buffer", arg->refReadBuf.Deref(env).val_),
                NVal::DeclareNapiProperty("offset", NVal::CreateInt64(env, arg->offset).val_)
            });
        return { obj };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    size_t argc = funcArg.GetArgc();
    bool hasOp = false;
    if (argc == NARG_CNT::THREE) {
        NVal op = NVal(env, funcArg[NARG_POS::THIRD]);
        if (op.HasProp("offset") || op.HasProp("position") || op.HasProp("length")) {
            hasOp = true;
        }
    }
    if (argc == NARG_CNT::TWO || (argc == NARG_CNT::THREE && hasOp)) {
        return NAsyncWorkPromise(env, thisVar).Schedule("FileIORead", cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((argc == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule("FileIORead", cbExec, cbCompl).val_;
    }

    return NVal::CreateUndefined(env).val_;
}

UniError PropNExporter::WriteExec(shared_ptr<AsyncIOWrtieArg> arg, void *buf, size_t len, int fd, size_t position)
{
    if (position == (size_t)INVALID_POSITION) {
        arg->actLen = write(fd, buf, len);
    } else {
        arg->actLen = pwrite(fd, buf, len, position);
    }

    if (arg->actLen == -1) {
        return UniError(errno);
    } else {
        return UniError(ERRNO_NOERR);
    }
}

napi_value PropNExporter::Write(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    int fd;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    unique_ptr<char[]> bufGuard;
    void *buf = nullptr;
    size_t len;
    size_t position;
    bool hasPos = false;
    tie(succ, bufGuard, buf, len, hasPos, position) =
        CommonFunc::GetWriteArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Failed GetWriteArg");
        return nullptr;
    }

    shared_ptr<AsyncIOWrtieArg> arg;
    if (bufGuard) {
        arg = make_shared<AsyncIOWrtieArg>(move(bufGuard));
    } else {
        arg = make_shared<AsyncIOWrtieArg>(NVal(env, funcArg[NARG_POS::SECOND]));
    }
    auto cbExec = [arg, buf, len, fd, position](napi_env env) -> UniError {
        return WriteExec(arg, buf, len, fd, position);
    };

    auto cbCompl = [arg](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return { NVal::CreateInt64(env, arg->actLen) };
        }
    };

    NVal thisVar(env, funcArg.GetThisVar());
    bool hasOp = false;
    size_t argc = funcArg.GetArgc();
    if (argc == NARG_CNT::THREE) {
        NVal op = NVal(env, funcArg[NARG_POS::THIRD]);
        if (op.HasProp("offset") || op.HasProp("position") || op.HasProp("length") || op.HasProp("encoding")) {
            hasOp = true;
        }
    }

    if (argc == NARG_CNT::TWO || (argc == NARG_CNT::THREE && hasOp)) {
        return NAsyncWorkPromise(env, thisVar).Schedule("FileIOWrite", cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((argc == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule("FileIOWrite", cbExec, cbCompl).val_;
    }

    return NVal::CreateUndefined(env).val_;
}

bool PropNExporter::Export()
{
    return exports_.AddProp(
        {
            NVal::DeclareNapiFunction("close", Close::Async),
            NVal::DeclareNapiFunction("fstat", Fstat::Async),
            NVal::DeclareNapiFunction("open", Open::Async),
            NVal::DeclareNapiFunction("read", Read),
            NVal::DeclareNapiFunction("write", Write),
        });
}

string PropNExporter::GetClassName()
{
    return PropNExporter::className_;
}

PropNExporter::PropNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}

PropNExporter::~PropNExporter() {}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
