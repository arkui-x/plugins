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

#include "readeriterator_n_exporter.h"

#include "filemgmt_libhilog.h"
#include "file_utils.h"
#include "readeriterator_entity.h"
#include "../fs_utils.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;
using namespace LibN;

napi_value ReaderIteratorNExporter::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto readerIteratorEntity = CreateUniquePtr<ReaderIteratorEntity>();
    if (readerIteratorEntity == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }

    if (!NClass::SetEntityFor<ReaderIteratorEntity>(env, funcArg.GetThisVar(), move(readerIteratorEntity))) {
        HILOGE("Failed to set reader iterator entity");
        NError(UNKROWN_ERR).ThrowErr(env);
        return nullptr;
    }
    return funcArg.GetThisVar();
}

bool ReaderIteratorNExporter::Export()
{
    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("next", Next),
    };

    string className = GetClassName();
    bool succ = false;
    napi_value classValue = nullptr;
    tie(succ, classValue) = NClass::DefineClass(exports_.env_, className, ReaderIteratorNExporter::Constructor,
        std::move(props));
    if (!succ) {
        HILOGE("Failed to define class");
        NError(UNKROWN_ERR).ThrowErr(exports_.env_);
        return false;
    }
    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        HILOGE("Failed to save class");
        NError(UNKROWN_ERR).ThrowErr(exports_.env_);
        return false;
    }

    return exports_.AddProp(className, classValue);
}

napi_value ReaderIteratorNExporter::Next(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto readerIteratorEntity = NClass::GetEntityOf<ReaderIteratorEntity>(env, funcArg.GetThisVar());
    if (!readerIteratorEntity) {
        HILOGE("Failed to get reader iterator entity");
        NError(UNKROWN_ERR).ThrowErr(env);
        return nullptr;
    }

    Str *str = NextLine(readerIteratorEntity->iterator);
    if (str == nullptr && readerIteratorEntity->offset != 0) {
        HILOGE("Failed to get next line, error:%{public}d", errno);
        NError(errno).ThrowErr(env);
        return nullptr;
    }

    NVal objReaderIteratorResult = NVal::CreateObject(env);
    objReaderIteratorResult.AddProp("done", NVal::CreateBool(env, (readerIteratorEntity->offset == 0)).val_);
    if (str != nullptr) {
        objReaderIteratorResult.AddProp("value", NVal::CreateUTF8String(env, str->str, str->len).val_);
        readerIteratorEntity->offset -= static_cast<int64_t>(str->len);
    } else {
        objReaderIteratorResult.AddProp("value", NVal::CreateUTF8String(env, "").val_);
        (void)NClass::RemoveEntityOfFinal<ReaderIteratorEntity>(env, funcArg.GetThisVar());
    }

    StrFree(str);
    return objReaderIteratorResult.val_;
}

string ReaderIteratorNExporter::GetClassName()
{
    return ReaderIteratorNExporter::className;
}

ReaderIteratorNExporter::ReaderIteratorNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}

ReaderIteratorNExporter::~ReaderIteratorNExporter() {}
} // namespace OHOS::FileManagement::ModuleFileIO