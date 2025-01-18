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
    : env_(env),
      guid(""),
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
      downloadPath(""),
      currentSpeed(0),
      percentComplete(0),
      totalBytes(0),
      receivedBytes(0),
      lastErrorCode(0),
      webDownloadId(0),
      nwebId(0)
{
}

WebDownloadItem::~WebDownloadItem()
{
}
} // namespace Plugin
} // namespace OHOS
