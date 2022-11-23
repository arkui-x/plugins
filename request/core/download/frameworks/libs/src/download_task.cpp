/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "download_task.h"

#include <vector>
#include "log.h"

namespace OHOS::Plugin::Request::Download {
DownloadTask::DownloadTask(uint32_t taskId) : taskId_(taskId)
{
    supportEvents_[EVENT_COMPLETE] = true;
    supportEvents_[EVENT_PAUSE] = true;
    supportEvents_[EVENT_REMOVE] = true;
    supportEvents_[EVENT_PROGRESS] = true;
    supportEvents_[EVENT_FAIL] = true;
}

DownloadTask::~DownloadTask()
{
    DOWNLOAD_HILOGD("Destructed download task [%{public}d]", GetId());
    std::lock_guard<std::mutex> autoLock(mutex_);
    listenerMap_.clear();
    supportEvents_.clear();
}

uint32_t DownloadTask::GetId() const
{
    return taskId_;
}

bool DownloadTask::AddListener(const std::string &type, std::shared_ptr<DownloadNotifyInterface> listener)
{
    std::lock_guard<std::mutex> autoLock(mutex_);
    auto it = listenerMap_.find(type);
    if (it != listenerMap_.end()) {
        DOWNLOAD_HILOGE("replace event listener with %{public}s event", type.c_str());
    }
    listenerMap_[type] = listener;
    return true;
}

void DownloadTask::RemoveListener(const std::string &type, std::shared_ptr<DownloadNotifyInterface> listener)
{
    std::lock_guard<std::mutex> autoLock(mutex_);
    auto it = listenerMap_.find(type);
    if (it == listenerMap_.end() || it->second != listener) {
        DOWNLOAD_HILOGE("no event listener");
        return;
    }
    listenerMap_.erase(it);
}

void DownloadTask::RemoveListener(const std::string &type)
{
    std::lock_guard<std::mutex> autoLock(mutex_);
    auto it = listenerMap_.find(type);
    if (it == listenerMap_.end()) {
        DOWNLOAD_HILOGE("no event listener");
        return;
    }
    listenerMap_.erase(it);
}

void DownloadTask::OnCallBack(const std::string &type, uint32_t argv1, uint32_t argv2)
{
    std::lock_guard<std::mutex> autoLock(mutex_);
    auto it = listenerMap_.find(type);
    if (it == listenerMap_.end()) {
        DOWNLOAD_HILOGE("no event listener");
        return;
    }
    std::vector<uint32_t> params;
    params.push_back(argv1);
    params.push_back(argv2);
    it->second->OnCallBack(params);
}

bool DownloadTask::IsSupportType(const std::string &type)
{
    return supportEvents_.find(type) != supportEvents_.end();
}
} // namespace OHOS::Plugin::Request::Download
