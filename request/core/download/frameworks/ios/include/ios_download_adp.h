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

#ifndef PLUGINS_REQUEST_DOWNLOAD_IOS_DOWNLOAD_ADP_H
#define PLUGINS_REQUEST_DOWNLOAD_IOS_DOWNLOAD_ADP_H

#include <memory>
#include "download_config.h"
#include "constant.h"


namespace OHOS::Plugin::Request::Download {

class IosDownloadAdpCallback;
class IosDownloadAdp {
public:
    IosDownloadAdp() = default;
    virtual ~IosDownloadAdp() {}
    static std::shared_ptr<IosDownloadAdp> Instance();
    static bool IsDirectory(const std::string &path);
    virtual void Download(const DownloadConfig &config, IosDownloadAdpCallback *callback) = 0;
    virtual bool Remove() = 0;
    virtual bool Suspend(bool isSuspendByNetwork) = 0;
    virtual bool Restore(IosDownloadAdpCallback *callback) = 0;
    virtual bool IsCompleted() = 0;
};

class IosDownloadAdpCallback {
public:
    IosDownloadAdpCallback() = default;
    virtual ~IosDownloadAdpCallback() {}
    virtual void OnProgress(uint32_t receivedSize, uint32_t totalSize) = 0;
    virtual void OnComplete() = 0;
    virtual void OnFail(ErrorCode errorCode) = 0;
    virtual void SetMimeType(const std::string &mimeType) = 0;
};

} // namespace OHOS::Plugin::Request::Download

#endif // PLUGINS_REQUEST_DOWNLOAD_IOS_DOWNLOAD_ADP_H