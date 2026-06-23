/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "storage_statistics_n_exporter.h"

#ifdef ANDROID_PLATFORM
#include "plugin_utils.h"
#include "storage_statistics_platform.h"
#endif

namespace OHOS {
namespace FileManagement {
namespace ModuleStorageStatistics {

#ifdef ANDROID_PLATFORM
static void StorageStatisticsJniRegister()
{
    const char className[] = "ohos.ace.plugin.storagestatistics.StorageStatisticsPlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&RegisterStorageStatisticsJavaPlugin, className);
}
#endif

napi_value StorageStatisticsExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getCurrentBundleStats", GetCurrentBundleStats),
        DECLARE_NAPI_FUNCTION("getTotalSize", GetTotalSize),
        DECLARE_NAPI_FUNCTION("getTotalSizeSync", GetTotalSizeSync),
        DECLARE_NAPI_FUNCTION("getFreeSize", GetFreeSize),
        DECLARE_NAPI_FUNCTION("getFreeSizeSync", GetFreeSizeSync),
        DECLARE_NAPI_FUNCTION("getTotalInodes", GetTotalInodes),
        DECLARE_NAPI_FUNCTION("getFreeInodes", GetFreeInodes),
        DECLARE_NAPI_FUNCTION("getCurrentBundleInodes", GetCurrentBundleInodes),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = StorageStatisticsExport,
    .nm_modname = "file.storageStatistics",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
#ifdef ANDROID_PLATFORM
    StorageStatisticsJniRegister();
#endif
    napi_module_register(&_module);
}
} // namespace ModuleStorageStatistics
} // namespace FileManagement
} // namespace OHOS
