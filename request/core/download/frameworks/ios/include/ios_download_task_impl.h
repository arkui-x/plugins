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

#ifndef PLUGINS_REQUEST_DOWNLOAD_IOS_DOWNLOAD_TASK_IMPL_H
#define PLUGINS_REQUEST_DOWNLOAD_IOS_DOWNLOAD_TASK_IMPL_H

#include <stddef.h>
#include <stdint.h>
#include <mutex>
#include <string>
#include <vector>
#include <iosfwd>
#include <thread>
#include "constant.h"
#include "download_config.h"
#include "i_download_task.h"
#include "ios_download_adp.h"
#include "ios_net_monitor.h"


namespace OHOS::Plugin::Request::Download {
class IosDownloadTaskImpl : public IDownloadTask, public IosDownloadAdpCallback, public IosNetMonitorObserver {
public:
    IosDownloadTaskImpl(uint32_t taskId, const DownloadConfig &config);
    ~IosDownloadTaskImpl();
    static void RunTask(IosDownloadTaskImpl *task);
    void RunTaskImpl();

public:
    // IDownloadTask
    void ExecuteTask() override;
    bool Remove() override;
    bool Suspend() override;
    bool Restore() override;
    bool GetTaskInfo(DownloadInfo &info) override;
    bool GetTaskMimeType(std::string &mimeType) override;

public:
    // IosDownloadAdpCallback
    void OnProgress(int64_t receivedSize, int64_t totalSize) override;
    void OnPause() override;
    void OnResume() override;
    void OnComplete() override;
    void OnFail(ErrorCode errorCode) override;
    void SetMimeType(const std::string &mimeType) override;

public:
    // IosNetMonitorObserver
    void NetworkStatusChanged(NetworkType netType) override;

private:
    void ReachableViaWiFi();
    void ReachableViaWWAN();
    void NotReachable();

private:
    std::shared_ptr<IosDownloadAdp> downloadAdp_ = nullptr;
    std::string mimeType_;
    int64_t receivedSize_;
    int64_t totalSize_;
    std::mutex queryMutex_;
    NetworkType networkType_ = NETWORK_INVALID;
    bool isSuspendByNetwork_ = false;
    std::unique_ptr<std::thread> thread_ {};
    bool isRemoved_ = false;
};
} // namespace OHOS::Plugin::Request::Download
#endif // PLUGINS_REQUEST_DOWNLOAD_IOS_DOWNLOAD_TASK_IMPL_H