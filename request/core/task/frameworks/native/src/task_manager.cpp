/*
 * Copyright (C) 2022-2023 Huawei Device Co., Ltd.
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

#include "task_manager.h"
#include "log.h"
#include "task.h"

namespace OHOS::Plugin::Request {
TaskManager& TaskManager::Get()
{
    static TaskManager instance;
    std::lock_guard<std::recursive_mutex> autoLock(instance.mutex_);
    if (instance.adapter_ == nullptr) {
        REQUEST_HILOGI("TaskManager new");
        instance.adapter_ = ITaskManagerAdp::Get();
    }
    return instance;
}

ITask *TaskManager::Create(const Config &config)
{
    if (adapter_ == nullptr) {
        REQUEST_HILOGE("TaskManager invalid adapter");
        return nullptr;
    }
    auto newTask = adapter_->Create(config);
    if (newTask == nullptr) {
        REQUEST_HILOGE("create task failed");
        return nullptr;
    }
    return newTask;
}

int32_t TaskManager::Remove(int64_t taskId)
{
    REQUEST_HILOGI("Remove Task[%{public}ld]", taskId);
    if (adapter_ == nullptr) {
        REQUEST_HILOGE("TaskManager invalid adapter");
        return E_SERVICE_ERROR;
    }
    return adapter_->Remove(taskId);
}

int32_t TaskManager::GetTaskInfo(int64_t taskId, const std::string &token, TaskInfo &info)
{
    REQUEST_HILOGI("Get Task Info: [%{public}ld], token: %{public}s", taskId, token.c_str());
    if (adapter_ == nullptr) {
        REQUEST_HILOGE("TaskManager invalid adapter");
        return E_SERVICE_ERROR;
    }
    return adapter_->GetTaskInfo(taskId, token, info);
}

int32_t TaskManager::Search(const Filter &filter, std::vector<std::string> &taskIdList)
{
    REQUEST_HILOGI("Search Task");
    REQUEST_HILOGI("filter before: %{public}lld", filter.before);
    REQUEST_HILOGI("filter after: %{public}lld", filter.after);
    REQUEST_HILOGI("filter state: %{public}d", static_cast<int32_t>(filter.state));
    REQUEST_HILOGI("filter action: %{public}d", static_cast<int32_t>(filter.action));
    REQUEST_HILOGI("filter mode: %{public}d", static_cast<int32_t>(filter.mode));

    if (adapter_ == nullptr) {
        REQUEST_HILOGE("TaskManager invalid adapter");
        return E_SERVICE_ERROR;
    }
    auto ret = adapter_->Search(filter, taskIdList);
    if (ret != E_OK) {
        REQUEST_HILOGE("fail to search task");
        return E_SERVICE_ERROR;
    }
    return E_OK;
}

int32_t TaskManager::GetDefaultStoragePath(std::string& path)
{
    REQUEST_HILOGI("get default storage path");
    if (adapter_ == nullptr) {
        REQUEST_HILOGE("TaskManager invalid adapter");
        return E_SERVICE_ERROR;
    }
    auto ret = adapter_->GetDefaultStoragePath(path);
    if (ret != E_OK) {
        REQUEST_HILOGE("fail to get default storage path");
        return E_SERVICE_ERROR;
    }
    return E_OK;
}
} // namespace OHOS::Plugin::Request
