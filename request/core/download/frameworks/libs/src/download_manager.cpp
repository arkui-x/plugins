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

#include "download_manager.h"

#include "download_task.h"
#include "log.h"

namespace OHOS::Plugin::Request::Download {
DownloadManager& DownloadManager::GetInstance()
{
    static DownloadManager instance;
    return instance;
}

uint32_t DownloadManager::GetCurrentTaskId()
{
    std::lock_guard<std::recursive_mutex> autoLock(mutex_);
    return taskId_++;
}

DownloadTask* DownloadManager::Download(const DownloadConfig &config)
{
    int32_t taskId = GetCurrentTaskId();
    DOWNLOAD_HILOGD("DownloadManager Download taskId:%d.", taskId);

    DownloadTask *task = DownloadTask::CreateDownLoadTask(taskId, config);
    if (!task) {
        DOWNLOAD_HILOGE("create task failed taskId:%d.", taskId);
        return nullptr;
    }

    task->InstallCallback(NotifyHandler);

#ifdef SUPPORT_DOWNLOAD_CURL
    std::lock_guard<std::recursive_mutex> autoLock(mutex_);
    taskMap_[taskId] = task;
    task->ExecuteTask();
#endif

    return task;
}

void DownloadManager::NotifyHandler(const std::string& type, uint32_t taskId, uint32_t argv1, uint32_t argv2)
{
#ifdef SUPPORT_DOWNLOAD_CURL
    std::string combineType = type + "-" + std::to_string(taskId);
    DOWNLOAD_HILOGI("DownloadManager::NotifyHandler started %{public}s [%{public}d, %{public}d].",
                    combineType.c_str(), argv1, argv2);
    auto iter = DownloadManager::GetInstance().taskMap_.find(taskId);
    if (iter != DownloadManager::GetInstance().taskMap_.end()) {
        DOWNLOAD_HILOGI("DownloadManager::NotifyHandler type=%{public}s object message.", combineType.c_str());
        iter->second->OnCallBack(type, argv1, argv2);
    }
#endif
}

bool DownloadManager::Remove(uint32_t taskId)
{
    DOWNLOAD_HILOGD("Remove Task[%{public}d]", taskId);

#ifdef SUPPORT_DOWNLOAD_CURL
    std::lock_guard<std::recursive_mutex> autoLock(mutex_);
    auto it = taskMap_.find(taskId);
    if (it != taskMap_.end()) {
        bool result = it->second->Remove();
        if (result) {
            taskMap_.erase(it);
        }
        return result;
    }
    return false;
#else
    // adapter other platform
    return true;
#endif
}

bool DownloadManager::On(uint32_t taskId, const std::string &type,
    const std::shared_ptr<DownloadNotifyInterface> &listener)
{
    return true;
}

bool DownloadManager::Off(uint32_t taskId, const std::string &type)
{
    return true;
}
} // namespace OHOS::Plugin::Request::Download
