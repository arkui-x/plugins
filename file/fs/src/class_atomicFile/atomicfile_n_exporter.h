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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FILEIO_CLASS_ATOMIC_FILE_N_EXPORTER_H
#define INTERFACES_KITS_JS_SRC_MOD_FILEIO_CLASS_ATOMIC_FILE_N_EXPORTER_H

#include "filemgmt_libn.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace OHOS::FileManagement::LibN;
class AtomicFileNExporter final : public NExporter {
public:
    inline static const std::string className_ = "AtomicFile";

    bool Export() override;
    std::string GetClassName() override;

    static napi_value Constructor(napi_env env, napi_callback_info info);
    static napi_value GetBaseFile(napi_env env, napi_callback_info info);
    static napi_value OpenRead(napi_env env, napi_callback_info info);
    static napi_value ReadFully(napi_env env, napi_callback_info info);
    static napi_value StartWrite(napi_env env, napi_callback_info info);
    static napi_value FinishWrite(napi_env env, napi_callback_info info);
    static napi_value FailWrite(napi_env env, napi_callback_info info);
    static napi_value Delete(napi_env env, napi_callback_info info);

    AtomicFileNExporter(napi_env env, napi_value exports);
    ~AtomicFileNExporter() override;
};
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif