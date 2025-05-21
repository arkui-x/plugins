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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_READERITERATOR_N_EXPORTER_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_READERITERATOR_N_EXPORTER_H

#include "filemgmt_libn.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace LibN;

class ReaderIteratorNExporter final : public NExporter {
public:
    static inline const std::string className = "ReaderIterator";
    
    bool Export() override;
    std::string GetClassName() override;

    static napi_value Constructor(napi_env env, napi_callback_info info);
    static napi_value Next(napi_env env, napi_callback_info info);

    ReaderIteratorNExporter(napi_env env, napi_value exports);
    ~ReaderIteratorNExporter() override;
};
} // namespace OHOS::FileManagement::ModuleFileIO
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_READERITERATOR_N_EXPORTER_H