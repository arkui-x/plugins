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

#include "web_download_item.h"

#include <string>

namespace OHOS {
namespace Plugin {
WebDownloadItem::WebDownloadItem(napi_env env)
    : guid(""),
      fullPath(""),
      url(""),
      etag(""),
      originalUrl(""),
      suggestedFileName(""),
      contentDisposition(""),
      mimeType(""),
      lastModified(""),
      method(""),
      receivedSlices(""),
      downloadPath("")
{
    this->env_ = env;
    this->currentSpeed = 0;
    this->percentComplete = 0;
    this->totalBytes = 0;
    this->receivedBytes = 0;
    this->lastErrorCode = 0;
    this->webDownloadId = 0;
    this->nwebId = 0;
}

WebDownloadItem::~WebDownloadItem()
{
}
} // namespace Plugin
} // namespace OHOS
