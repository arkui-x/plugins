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
#include "read_lines.h"

#include <unistd.h>

#include "class_readeriterator/readeriterator_entity.h"
#include "class_readeriterator/readeriterator_n_exporter.h"
#include "common_func.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace LibN;

static int CheckOptionArg(napi_env env, napi_value argOption)
{
    NVal option(env, argOption);
    if (option.HasProp("encoding")) {
        auto [succ, encoding, ignore] = option.GetProp("encoding").ToUTF8String("utf-8");
        string_view encodingStr(encoding.get());
        if (!succ || encodingStr != "utf-8") {
            return EINVAL;
        }
    }

    return ERRNO_NOERR;
}

static int GetFileSize(const string &path, int64_t &offset)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> stat_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!stat_req) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }

    int ret = uv_fs_stat(nullptr, stat_req.get(), path.c_str(), nullptr);
    if (ret < 0) {
        HILOGE("Failed to get file stat by path, error: %{public}s", uv_strerror(ret));
        return ret;
    }

    offset = static_cast<int64_t>(stat_req->statbuf.st_size);
    return ERRNO_NOERR;
}

static NVal InstantiateReaderIterator(napi_env env, FileReader *iterator, int64_t offset, bool async = false)
{
    if (iterator == nullptr) {
        HILOGE("Invalid argument iterator");
        if (async) {
            return {env, NError(EINVAL).GetNapiErr(env)};
        }
        NError(EINVAL).ThrowErr(env);
        return NVal();
    }
    napi_value objReaderIterator = NClass::InstantiateClass(env, ReaderIteratorNExporter::className, {});
    if (!objReaderIterator) {
        HILOGE("Failed to instantiate class ReaderIterator");
        if (async) {
            return {env, NError(UNKROWN_ERR).GetNapiErr(env)};
        }
        NError(UNKROWN_ERR).ThrowErr(env);
        return NVal();
    }

    auto readerIteratorEntity = NClass::GetEntityOf<ReaderIteratorEntity>(env, objReaderIterator);
    if (!readerIteratorEntity) {
        HILOGE("Failed to get readerIteratorEntity");
        if (async) {
            return {env, NError(UNKROWN_ERR).GetNapiErr(env)};
        }
        NError(UNKROWN_ERR).ThrowErr(env);
        return NVal();
    }

    readerIteratorEntity->iterator = iterator;
    readerIteratorEntity->offset = offset;
    return { env, objReaderIterator };
}

struct ReaderIteratorArg {
    FileReader *iterator = nullptr;
    int64_t offset = 0;
};

static NError AsyncExec(ReaderIteratorArg &readerIterator, const string &pathStr)
{
    readerIterator.iterator = ReaderIterator(pathStr.c_str());
    if (readerIterator.iterator == nullptr) {
        errno = ENOENT;
        HILOGE("Failed to read lines of the file, error: %{public}d", errno);
        return NError(errno);
    }
    int ret = GetFileSize(pathStr, readerIterator.offset);
    if (ret < 0) {
        HILOGE("Failed to get size of the file");
        return NError(ret);
    }

    return NError(ERRNO_NOERR);
}

napi_value ReadLines::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succPath, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8StringPath();
    if (!succPath) {
        HILOGE("Invalid path from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    if (funcArg.GetArgc() >= NARG_CNT::TWO) {
        int ret = CheckOptionArg(env, funcArg[NARG_POS::SECOND]);
        if (ret) {
            HILOGE("Invalid option.encoding parameter");
            NError(ret).ThrowErr(env);
            return nullptr;
        }
    }

    auto arg = CreateSharedPtr<ReaderIteratorArg>();
    if (arg == nullptr) {
        HILOGE("DFailed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [arg, pathStr = string(path.get())]() -> NError {
        return AsyncExec(*arg, pathStr);
    };

    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return InstantiateReaderIterator(env, arg->iterator, arg->offset, true);
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE || (funcArg.GetArgc() == NARG_CNT::TWO &&
        !NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_READLINES_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[((funcArg.GetArgc() == NARG_CNT::TWO) ? NARG_POS::SECOND : NARG_POS::THIRD)]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_READLINES_NAME, cbExec, cbCompl).val_;
    }
}

napi_value ReadLines::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succPath, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8StringPath();
    if (!succPath) {
        HILOGE("Invalid path from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    if (funcArg.GetArgc() == NARG_CNT::TWO) {
        int ret = CheckOptionArg(env, funcArg[NARG_POS::SECOND]);
        if (ret) {
            HILOGE("Invalid option.encoding parameter");
            NError(ret).ThrowErr(env);
            return nullptr;
        }
    }

    auto iterator = ReaderIterator(path.get());
    if (iterator == nullptr) {
        HILOGE("Failed to read lines of the file, error: %{public}d", errno);
        NError(ENOENT).ThrowErr(env);
        return nullptr;
    }

    int64_t offset = 0;
    int ret = GetFileSize(path.get(), offset);
    if (ret != 0) {
        HILOGE("Failed to get size of the file");
        NError(ret).ThrowErr(env);
        return nullptr;
    }

    return InstantiateReaderIterator(env, iterator, offset).val_;
}
} // ModuleFileIO
} // FileManagement
} // OHOS