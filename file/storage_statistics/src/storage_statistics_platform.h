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

#ifndef STORAGE_STATISTICS_PLATFORM_H
#define STORAGE_STATISTICS_PLATFORM_H

#include <cstdint>

namespace OHOS {
namespace FileManagement {
namespace ModuleStorageStatistics {
constexpr int32_t PLATFORM_SUCCESS = 0;
constexpr int32_t PLATFORM_ERROR = -1;

struct BundleStatsValue {
    int64_t appSize = 0;
    int64_t cacheSize = 0;
    int64_t dataSize = 0;
};

#ifdef ANDROID_PLATFORM
bool RegisterStorageStatisticsJavaPlugin(void* env);
#endif

int32_t GetCurrentBundleStatsByPlatform(BundleStatsValue& stats);
int32_t GetCurrentBundleInodesByPlatform(int64_t& inodes);
} // namespace ModuleStorageStatistics
} // namespace FileManagement
} // namespace OHOS

#endif // STORAGE_STATISTICS_PLATFORM_H
