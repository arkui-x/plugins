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

#ifndef PLUGINS_REQUEST_DOWNLOAD_ANDROID_DOWNLOAD_TASK_IMPL_H
#define PLUGINS_REQUEST_DOWNLOAD_ANDROID_DOWNLOAD_TASK_IMPL_H

#include <stddef.h>
#include <stdint.h>
#include <mutex>
#include <string>
#include <vector>
#include <iosfwd>
#include <thread>

#include "constant.h"
#include "download_config.h"
#include "android_download_adp.h"
#include "i_download_task.h"

namespace OHOS::Plugin::Request::Download {
class AndroidDownloadTaskImpl : public IDownloadTask {
public:
    AndroidDownloadTaskImpl(uint32_t taskId, const DownloadConfig &config);
    ~AndroidDownloadTaskImpl();
    static void RunTask(AndroidDownloadTaskImpl *task);
    void RunTaskImpl();

public:
    void ExecuteTask() override;
    bool Remove() override;
    bool Suspend() override;
    bool Restore() override;
    bool GetTaskInfo(DownloadInfo &info) override;
    bool GetTaskMimeType(std::string &mimeType) override;

public:
    void OnProgress(uint32_t receivedSize, uint32_t totalSize);
    void OnComplete();
    void OnFail(ErrorCode errorCode);
    void SetMimeType(const std::string &mimeType);
    void SetNetworkType(NetworkType networkType);

private:
    std::string mimeType_;
    uint32_t receivedSize_;
    uint32_t totalSize_;
    std::mutex queryMutex_;

    NetworkType networkType_ = NETWORK_INVALID;
    bool isRemoved_ = false;
    std::shared_ptr<AndroidDownloadAdp> AndroidDownloadAdp_ = nullptr;
    std::string sandBoxPath_;
};
} // namespace OHOS::Plugin::Request::Download
#endif