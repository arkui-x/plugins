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

#include "i_task_adp.h"
#include "constant.h"
#import "OHNetworkKit.h"
#import "ios_net_monitor.h"

namespace OHOS::Plugin::Request {
class DownloadProxy final : public ITaskAdp, public IosNetMonitorObserver {
public:
    DownloadProxy(int64_t taskId, const Config &config, OnRequestCallback callback);
    virtual ~DownloadProxy();

    int32_t Start(int64_t taskId) override;
    int32_t Pause(int64_t taskId) override;
    int32_t Resume(int64_t taskId) override;
    int32_t Stop(int64_t taskId) override;

public:
    // IosNetMonitorObserver
    void NetworkStatusChanged(NetworkType netType) override;

private:
    void ReachableViaWiFi();
    void ReachableViaWWAN();
    void NotReachable();

private:
    void InitTaskInfo(const Config &config, TaskInfo &info);
    void PushNotification(BOOL isFailed);
    void CompletionHandler(NSURLResponse *response, NSURL *filePath, NSError *error);
    void PushNotification(NSString *fileName, NSValue *result);
    void ReportMimeType(NSURLResponse *response);
    void OnCompletedCallback();
    void OnProgressCallback(NSProgress *downloadProgress);
    void OnResponseCallback(NSURLResponse *response);
    void OnFailedCallback();
    void OnPauseCallback();
    void SetProxy(const Config &config, NSURLSessionConfiguration **sessionConfig);
    void SetSizes(int64_t fileSize);
    void GetFileSize(const std::string &downloadUrl);
    void PushBackOcValues(NSArray<NSString *> *ocValues, std::vector<std::string> &cppValues);
    NSString* GetStandardHTTPReason(NSInteger statusCode);

private:
    Config config_;
    TaskInfo info_;
    OHSessionManager *sessionCtrl_ = nil;
    NSURLSessionDownloadTask *downloadTask_ = nil;
    NSData *resumeData_ = nil;
    bool isMimeReported_ = false;
    OnRequestCallback callback_ = nullptr;
    int64_t taskId_ = INVALID_TASK_ID;
    int64_t downloadTotalBytes_ = 0;
    int64_t currentTime_ = 0;
    bool isStopped_ = false;
    bool isPause_ = false;
};
} // namespace OHOS::Plugin::Request
#endif // PLUGINS_REQUEST_DOWNLOAD_PROXY_H