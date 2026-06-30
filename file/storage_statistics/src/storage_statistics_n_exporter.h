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

#ifndef STORAGE_STATISTICS_N_EXPORTER_H
#define STORAGE_STATISTICS_N_EXPORTER_H

#include "filemgmt_libn.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleStorageStatistics {
napi_value GetCurrentBundleStats(napi_env env, napi_callback_info info);

napi_value GetTotalSize(napi_env env, napi_callback_info info);
napi_value GetTotalSizeSync(napi_env env, napi_callback_info info);

napi_value GetFreeSize(napi_env env, napi_callback_info info);
napi_value GetFreeSizeSync(napi_env env, napi_callback_info info);

napi_value GetTotalInodes(napi_env env, napi_callback_info info);
napi_value GetFreeInodes(napi_env env, napi_callback_info info);
napi_value GetCurrentBundleInodes(napi_env env, napi_callback_info info);
} // namespace ModuleStorageStatistics
} // namespace FileManagement
} // namespace OHOS

#endif // STORAGE_STATISTICS_N_EXPORTER_H
