/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "common_func.h"

#include <memory>
#include <vector>

#include "file_n_exporter.h"
#include "filemgmt_libhilog.h"
#include "prop_n_exporter.h"
#include "stat_n_exporter.h"
#include "stream_n_exporter.h"
#include "class_randomaccessfile/randomaccessfile_n_exporter.h"
#include "class_readeriterator/readeriterator_n_exporter.h"
#include "class_atomicFile/atomicfile_n_exporter.h"
#ifndef IOS_PLATFORM
#include "class_watcher/watcher_n_exporter.h"
#endif
using namespace std;

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
static napi_value Export(napi_env env, napi_value exports)
{
    InitOpenMode(env, exports);
    InitWhenceType(env, exports);
    InitAccessModeType(env, exports);
    InitAccessFlagType(env, exports);
    InitLocationType(env, exports);
    std::vector<unique_ptr<NExporter>> products;
    products.emplace_back(make_unique<PropNExporter>(env, exports));
    products.emplace_back(make_unique<FileNExporter>(env, exports));
    products.emplace_back(make_unique<StatNExporter>(env, exports));
    products.emplace_back(make_unique<StreamNExporter>(env, exports));
    products.emplace_back(make_unique<RandomAccessFileNExporter>(env, exports));
    products.emplace_back(make_unique<ReaderIteratorNExporter>(env, exports));
    products.emplace_back(make_unique<AtomicFileNExporter>(env, exports));
#ifndef IOS_PLATFORM
    products.emplace_back(make_unique<WatcherNExporter>(env, exports));
#endif
    for (auto &&product : products) {
        if (!product->Export()) {
            HILOGE("INNER BUG. Failed to export class %{public}s for module fileio", product->GetClassName().c_str());
            return nullptr;
        } else {
            HILOGI("Class %{public}s for module fileio has been exported", product->GetClassName().c_str());
        }
    }
    return exports;
}

static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Export,
    .nm_modname = "file.fs",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void RegisterFSModule(void)
{
    napi_module_register(&_module);
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
