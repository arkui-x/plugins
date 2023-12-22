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

#ifndef PLUGINS_REQUEST_DOWNLOAD_PROXY_H
#define PLUGINS_REQUEST_DOWNLOAD_PROXY_H

#include <mutex>
#include "i_task_adp.h"
#include "constant.h"
#import "OHNetworkKit.h"
#import "IosTaskDao.h"

namespace OHOS::Plugin::Request {
class DownloadProxy final : public ITaskAdp {
public:
    DownloadProxy(int64_t taskId, const Config &config, OnRequestCallback callback);
    virtual ~DownloadProxy();

    int32_t Start(int64_t taskId) override;
    int32_t Pause(int64_t taskId) override;
    int32_t Resume(int64_t taskId) override;
    int32_t Stop(int64_t taskId) override;

private:
    void PushNotification(BOOL isFailed);
    void CompletionHandler(NSURLResponse *response, NSURL *filePath, NSError *error);
    void PushNotification(NSString *fileName, NSValue *result);
    void ReportMimeType(NSURLResponse *response);
    void OnCompletedCallback();
    void OnProgressCallback(NSProgress *downloadProgress);
    void GetFileSize(const std::string &fileUrl);

private:
    Config config_;
    TaskInfo info_;
    OHSessionManager *sessionCtrl_ = nil;
    NSURLSessionDownloadTask *downloadTask_ = nil;
    NSData *resumeData_ = nil;
    bool isSuspendByNetwork_ = false;
    bool isMimeReported_ = false;
    OnRequestCallback callback_ = nullptr;
    int64_t taskId_ = INVALID_TASK_ID;
};
} // namespace OHOS::Plugin::Request
#endif // PLUGINS_REQUEST_DOWNLOAD_PROXY_H