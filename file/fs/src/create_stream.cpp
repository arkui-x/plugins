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

#include "create_stream.h"

#include <memory>
#include <tuple>

#include "class_stream/stream_entity.h"
#include "class_stream/stream_n_exporter.h"
#include "common_func.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static tuple<bool, string, string> GetCreateStreamArgs(napi_env env, const NFuncArg &funcArg)
{
    auto [resGetFirstArg, path, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8StringPath();
    if (!resGetFirstArg) {
        return { false, "", "" };
    }

    auto [resGetSecondArg, mode, useless] = NVal(env, funcArg[NARG_POS::SECOND]).ToUTF8String();
    if (!resGetSecondArg) {
        return { false, "", "" };
    }

    return { true, path.get(), mode.get() };
}

napi_value CreateStream::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetCreateStreamArgs, argPath, argMode] = GetCreateStreamArgs(env, funcArg);
    if (!resGetCreateStreamArgs) {
        HILOGE("Arg path and mode are required to be type of string");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    FILE *file = fopen(argPath.c_str(), argMode.c_str());
    int err = errno;
    if (!file) {
        HILOGE("Failed to fdopen file by path");
        NError(err).ThrowErr(env);
        return nullptr;
    }
    std::shared_ptr<FILE> fp(file, fclose);
    return CommonFunc::InstantiateStream(env, move(fp)).val_;
}

napi_value CreateStream::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetCreateStreamArgs, argPath, argMode] = GetCreateStreamArgs(env, funcArg);
    if (!resGetCreateStreamArgs) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto arg = CreateSharedPtr<AsyncCreateStreamArg>();
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [arg, argPath = move(argPath), argMode = move(argMode)]() -> NError {
        FILE *file = fopen(argPath.c_str(), argMode.c_str());
        int err = errno;
        if (!file) {
            HILOGE("Failed to fdopen file by path");
            return NError(err);
        }
        arg->fp = std::shared_ptr<FILE>(file, fclose);
        return NError(ERRNO_NOERR);
    };

    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return CommonFunc::InstantiateStream(env, move(arg->fp), true);
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::TWO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_CREATESTREAM_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::THIRD]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_CREATESTREAM_NAME, cbExec, cbCompl).val_;
    }
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
