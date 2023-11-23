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

#ifndef PLUGINS_REQUEST_DOWNLOAD_IOS_DOWNLOAD_ADP_IMPL_H
#define PLUGINS_REQUEST_DOWNLOAD_IOS_DOWNLOAD_ADP_IMPL_H

#include <mutex>
#include "ios_download_adp.h"
#import "OHNetworkKit.h"

namespace OHOS::Plugin::Request::Download {
class IosDownloadAdpImpl final : public IosDownloadAdp {
public:
    IosDownloadAdpImpl();
    ~IosDownloadAdpImpl();
    // IosDownloadAdp
    void Download(const DownloadConfig &config, IosDownloadAdpCallback *callback) override;
    bool Remove() override;
    bool Suspend(bool isSuspendByNetwork) override;
    bool Restore(IosDownloadAdpCallback *callback) override;
    bool IsCompleted() override;

private:
    std::string GetFileName(const std::string &fileName);
    void PushNotification(BOOL isFailed);
    void CompletionHandler(IosDownloadAdpCallback *callback, NSURLResponse *response, NSURL *filePath, NSError *error);
    void PushNotification(NSString *fileName, NSValue *result);
    void ReportMimeType(IosDownloadAdpCallback *callback, NSURLResponse *response);

private:
    OHSessionManager *sessionCtrl_ = nil;
    NSURLSessionDownloadTask *downloadTask_ = nil;
    NSData *resumeData_ = nil;
    std::string fileName_;
    DownloadConfig config_;
    bool isSuspendByNetwork_ = false;
    bool isCompleted_ = false;
    bool isMimeReported_ = false;
    std::mutex mutex_;
};
} // namespace OHOS::Plugin::Request::Download
#endif // PLUGINS_REQUEST_DOWNLOAD_IOS_DOWNLOAD_ADP_IMPL_H