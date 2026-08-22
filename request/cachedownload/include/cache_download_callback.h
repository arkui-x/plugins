/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_DOWNLOAD_CALLBACK_H
#define PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_DOWNLOAD_CALLBACK_H

#include <functional>
#include <string>

#include "cache_error.h"

namespace OHOS::Plugin::CacheDownload {

/**
 * Success callback type — matches ArkTS onDownloadSuccess: Callback<void>.
 * Called when a download completes successfully.
 */
using SuccessCallback = std::function<void()>;

/**
 * Error callback type — matches ArkTS onDownloadError: Callback<DownloadError>.
 * Called when a download fails.
 */
using ErrorCallback = std::function<void(const CacheDownloadError& error)>;

} // namespace OHOS::Plugin::CacheDownload

#endif // PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_DOWNLOAD_CALLBACK_H
