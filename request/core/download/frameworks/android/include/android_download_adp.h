/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_REQUEST_DOWNLOAD_ANDROID_DOWNLOAD_ADP_H
#define PLUGINS_REQUEST_DOWNLOAD_ANDROID_DOWNLOAD_ADP_H

#include <memory>
#include "download_config.h"
#include "constant.h"

namespace OHOS::Plugin::Request::Download {
class AndroidDownloadAdp {
public:
    AndroidDownloadAdp() = default;
    virtual ~AndroidDownloadAdp() {}
    static std::shared_ptr<AndroidDownloadAdp> Instance();
    static bool IsPathValid(const std::string &filePath);
    static bool IsDirectory(const std::string &path);
    virtual void Download(const DownloadConfig &config, void *downloadProgress) = 0;
    virtual void Remove() = 0;
    virtual void Suspend() = 0;
    virtual bool Restore() = 0;
    virtual void GetNetworkType(void *network) = 0;
};
} // namespace OHOS::Plugin::Request::Download

#endif // PLUGINS_REQUEST_DOWNLOAD_ANDROID_DOWNLOAD_ADP_H