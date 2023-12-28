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

#include "ios_adapter.h"
#include "download_proxy.h"
#include "log.h"
#include "plugin_utils.h"
#include "request_utils.h"
#include "task.h"
#include "task_notify_manager.h"
#include "upload_proxy.h"

namespace OHOS::Plugin::Request {
std::map<int64_t, std::shared_ptr<ITaskAdp>> IosAdapter::taskList_ = {};

std::shared_ptr<ITaskManagerAdp> ITaskManagerAdp::Get()
{
    return std::make_shared<IosAdapter>();
}

IosAdapter::~IosAdapter()
{
}

void RequestCallback(int64_t taskId, std::string eventType, std::string infoParam)
{
    REQUEST_HILOGI("RequestCallback taskId: %{public}lld, eventType:%{public}s, infoParam:%{public}s",
        taskId, eventType.c_str(), infoParam.c_str());
    TaskNotifyManager::Get().SendNotify(RequestUtils::GetEventType(taskId, eventType), infoParam);
}

ITask *IosAdapter::Create(const Config &config)
{
    auto task = new (std::nothrow) Task(config, shared_from_this());
    if (task == nullptr) {
        REQUEST_HILOGE("fail to allocate memory for task");
        return nullptr;
    }

    int64_t taskId = IosTaskDao::CreateTask(config);
    if (taskId == INVALID_TASK_ID) {
        REQUEST_HILOGE("fail to create ios task");
        return nullptr;
    }

    std::shared_ptr<ITaskAdp> proxy = nullptr;
    if (config.action == Action::UPLOAD) {
        proxy = std::make_shared<UploadProxy>(taskId, config, RequestCallback);
    } else {
        proxy = std::make_shared<DownloadProxy>(taskId, config, RequestCallback);
    }
    if (proxy == nullptr) {
        REQUEST_HILOGE("fail to allocate memory for task proxy");
        return nullptr;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    taskList_.emplace(taskId, proxy);
    task->SetId(taskId);
    REQUEST_HILOGI("Succeed to creat ios task, taskId:%{public}p", (void*)&taskList_);
    REQUEST_HILOGI("Succeed to creat ios task, taskId:%{public}lld", taskId);
    return task;
}

int32_t IosAdapter::Remove(int64_t taskId)
{
    REQUEST_HILOGI("Remove task, taskId:%{public}lld", taskId);
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = taskList_.find(taskId);
    if (it == taskList_.end() || it->second == nullptr) {
        REQUEST_HILOGE("invalid task id");
        return E_SERVICE_ERROR;
    }
    REQUEST_HILOGI("Remove ios taskList address, :%{public}p", (void*)&taskList_);
    taskList_.erase(it);
    REQUEST_HILOGI("Remove ios taskList erase success");


    int32_t result = IosTaskDao::RemoveTask(taskId);
    REQUEST_HILOGI("remove task: %{public}d", result);
    return result;
}

int32_t IosAdapter::GetTaskInfo(int64_t taskId, const std::string &token, TaskInfo &info)
{
    REQUEST_HILOGI("IosAdapter::GetTaskInfo, taskId:%{public}lld, token:%{public}s", taskId, token.c_str());
    return IosTaskDao::QueryTaskInfo(taskId, token, info);
}

int32_t IosAdapter::Search(const Filter &filter, std::vector<std::string> &taskIdList)
{
    return IosTaskDao::Search(filter, taskIdList);
}

int32_t IosAdapter::Start(int64_t taskId)
{
    REQUEST_HILOGI("Start taskId: %{public}lld", taskId);
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = taskList_.find(taskId);
    if (it == taskList_.end() || it->second == nullptr) {
        REQUEST_HILOGE("invalid task id");
        return E_SERVICE_ERROR;
    }
    auto result = it->second->Start(taskId);
    REQUEST_HILOGI("start task, result:%{public}d", result);
    return result;
}

int32_t IosAdapter::Pause(int64_t taskId)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = taskList_.find(taskId);
    if (it == taskList_.end() || it->second == nullptr) {
        REQUEST_HILOGE("invalid task id");
        return E_SERVICE_ERROR;
    }
    auto result = it->second->Pause(taskId);
    REQUEST_HILOGI("pause task: %{public}d", result);
    return result;
}

int32_t IosAdapter::Resume(int64_t taskId)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = taskList_.find(taskId);
    if (it == taskList_.end() || it->second == nullptr) {
        REQUEST_HILOGE("invalid task id");
        return E_SERVICE_ERROR;
    }
    auto result = it->second->Resume(taskId);
    REQUEST_HILOGI("resume task: %{public}d", result);
    return result;
}

int32_t IosAdapter::Stop(int64_t taskId)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = taskList_.find(taskId);
    if (it == taskList_.end() || it->second == nullptr) {
        REQUEST_HILOGE("invalid task id");
        return E_SERVICE_ERROR;
    }
    auto result = it->second->Stop(taskId);
    REQUEST_HILOGI("stop task: %{public}d", result);
    return result;
}

} // namespace OHOS::Plugin::Request