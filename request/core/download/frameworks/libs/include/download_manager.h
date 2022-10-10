/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_REQUEST_DOWNLOAD_MANAGER_H
#define PLUGINS_REQUEST_DOWNLOAD_MANAGER_H

#include <map>
#include <mutex>
#include <condition_variable>

#include "download_config.h"
#include "download_task.h"
#include "download_notify_interface.h"

namespace OHOS::Plugin::Request::Download {
class DownloadManager {
public:
    static DownloadManager& GetInstance();

    DownloadTask *Download(const DownloadConfig &config);

    bool Remove(uint32_t taskId);
    bool On(uint32_t taskId, const std::string &type, const std::shared_ptr<DownloadNotifyInterface> &listener);
    bool Off(uint32_t taskId, const std::string &type);

private:
    uint32_t GetCurrentTaskId();
    static void NotifyHandler(const std::string& type, uint32_t taskId, uint32_t argv1, uint32_t argv2);

    DownloadManager() = default;
    ~DownloadManager() = default;
    DownloadManager(const DownloadManager&) = delete;
    DownloadManager& operator=(const DownloadManager&) = delete;
    DownloadManager(DownloadManager &&) = delete;
    DownloadManager& operator=(DownloadManager &&) = delete;

private:
    std::recursive_mutex mutex_;
    uint32_t taskId_ = 0;

#ifdef SUPPORT_DOWNLOAD_CURL
    std::map<uint32_t, DownloadTask*> taskMap_;
#endif
};
} // namespace OHOS::Plugin::Request::Download
#endif // PLUGINS_REQUEST_DOWNLOAD_MANAGER_H
