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
#include "IosTaskDao.h"
#include "json_utils.h"
#include "request_utils.h"
#include "task.h"
#include "task_notify_manager.h"
#include "upload_proxy.h"

namespace OHOS::Plugin::Request {
std::shared_ptr<ITaskManagerAdp> ITaskManagerAdp::Get()
{
    return std::make_shared<IosAdapter>();
}

IosAdapter::~IosAdapter()
{
}

void RequestCallback(int64_t taskId, const std::string &eventType, const std::string &infoParam)
{
    NSLog(@"RequestCallback taskId: %lld, eventType:%s, infoParam:%s",
        taskId, eventType.c_str(), infoParam.c_str());
    TaskNotifyManager::Get().SendNotify(RequestUtils::GetEventType(taskId, eventType), infoParam);
}

ITask *IosAdapter::Create(const Config &config)
{
    auto task = new (std::nothrow) Task(config, shared_from_this());
    if (task == nullptr) {
        NSLog(@"fail to allocate memory for task");
        return nullptr;
    }

    int64_t taskId = IosTaskDao::CreateTask(config);
    if (taskId == INVALID_TASK_ID) {
        NSLog(@"fail to create ios task");
        return nullptr;
    }

    std::shared_ptr<ITaskAdp> proxy = nullptr;
    if (config.action == Action::UPLOAD) {
        proxy = std::make_shared<UploadProxy>(taskId, config, RequestCallback);
    } else {
        proxy = std::make_shared<DownloadProxy>(taskId, config, RequestCallback);
    }
    if (proxy == nullptr) {
        NSLog(@"fail to allocate memory for task proxy");
        return nullptr;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    taskList_.emplace(taskId, proxy);
    task->SetId(taskId);
    NSLog(@"Succeed to creat ios task, taskId:%lld", taskId);
    return task;
}

int32_t IosAdapter::Remove(int64_t taskId)
{
    NSLog(@"IosAdapter::Remove, taskId:%lld", taskId);
    auto result = Stop(taskId);
    if (result != E_OK) {
        NSLog(@"IosAdapter::Remove, stop failed");
    }

    std::unique_lock<std::mutex> lock(mutex_);
    auto it = taskList_.find(taskId);
    if (it == taskList_.end() || it->second == nullptr) {
        NSLog(@"invalid task id");
        return E_TASK_NOT_FOUND;
    }

    TaskInfo info;
    result = GetTaskInfo(taskId, "", info);
    if (result != E_OK) {
        NSLog(@"IosAdapter::Remove, GetTaskInfo failed");
    }
    info.progress.state = State::REMOVED;
    IosTaskDao::UpdateDB(info);
    RequestCallback(taskId, EVENT_REMOVE, JsonUtils::TaskInfoToJsonString(info));
    NSLog(@"IosAdapter::Remove, end");
    return result;
}

int32_t IosAdapter::GetTaskInfo(int64_t taskId, const std::string &token, TaskInfo &info)
{
    NSLog(@"IosAdapter::GetTaskInfo, taskId:%lld, token:%s", taskId, token.c_str());
    return IosTaskDao::QueryTaskInfo(taskId, token, info);
}

int32_t IosAdapter::Search(const Filter &filter, std::vector<std::string> &taskIdList)
{
    return IosTaskDao::Search(filter, taskIdList);
}

int32_t IosAdapter::Start(int64_t taskId)
{
    NSLog(@"ios adapter start taskId:%lld", taskId);
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = taskList_.find(taskId);
    if (it == taskList_.end() || it->second == nullptr) {
        NSLog(@"invalid task id");
        return E_TASK_NOT_FOUND;
    }
    auto result = it->second->Start(taskId);
    NSLog(@"ios adapter start result:%d", result);
    return result;
}

int32_t IosAdapter::Pause(int64_t taskId)
{
    NSLog(@"ios adapter pause enter");
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = taskList_.find(taskId);
    if (it == taskList_.end() || it->second == nullptr) {
        NSLog(@"invalid task id");
        return E_TASK_NOT_FOUND;
    }
    auto result = it->second->Pause(taskId);
    NSLog(@"ios adapter pause result:%d", result);
    return result;
}

int32_t IosAdapter::Resume(int64_t taskId)
{
    NSLog(@"ios adapter resume enter");
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = taskList_.find(taskId);
    if (it == taskList_.end() || it->second == nullptr) {
        NSLog(@"invalid task id");
        return E_TASK_NOT_FOUND;
    }
    auto result = it->second->Resume(taskId);
    NSLog(@"ios adapter resume result:%d", result);
    return result;
}

int32_t IosAdapter::Stop(int64_t taskId)
{
    NSLog(@"ios adapter stop enter");
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = taskList_.find(taskId);
    if (it == taskList_.end() || it->second == nullptr) {
        NSLog(@"invalid task id");
        return E_TASK_NOT_FOUND;
    }
    auto result = it->second->Stop(taskId);
    NSLog(@"ios adapter stop result:%d", result);
    return result;
}

int32_t IosAdapter::GetDefaultStoragePath(std::string& path)
{
    NSLog(@"ios adapter GetDefaultStoragePath enter");
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    NSString *cachesDir = [paths objectAtIndex:0];
    path = cachesDir.UTF8String;
    NSLog(@"ios adapter GetDefaultStoragePath path:%s", path.c_str());
    return E_OK;
}
} // namespace OHOS::Plugin::Request