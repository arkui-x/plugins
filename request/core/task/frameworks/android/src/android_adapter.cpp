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

#include "android_adapter.h"
#include "inner_api/plugin_utils_inner.h"
#include "java/jni/task_manager_jni.h"
#include "log.h"
#include "plugin_utils.h"
#include "task.h"
#include "task_info_json.h"

namespace OHOS::Plugin::Request {

std::shared_ptr<ITaskManagerAdp> ITaskManagerAdp::Get()
{
    return std::make_shared<AndroidAdapter>();
}

ITask *AndroidAdapter::Create(const Config &config)
{
    auto task = new (std::nothrow) Task(config, shared_from_this());
    if (task == nullptr) {
        REQUEST_HILOGE("fail to allocate memory for task");
        return nullptr;
    }
    auto taskId = TaskManagerJni::Get().Create(config);
    if (taskId == INVALID_TASK_ID) {
        REQUEST_HILOGE("fail to create android task");
        return nullptr;
    }
    task->SetId(taskId);
    REQUEST_HILOGI("Succeed to create android task");
    return task;
}

int32_t AndroidAdapter::Remove(int64_t taskId)
{
    auto result = TaskManagerJni::Get().Remove(taskId);
    REQUEST_HILOGI("remove task result: %{public}d", result);
    return result;
}

int32_t AndroidAdapter::GetTaskInfo(int64_t taskId, const std::string &token, TaskInfo &info)
{
    int32_t result = E_OK;
    if (token == "") {
        result = TaskManagerJni::Get().Show(taskId, info);
    } else {
        result = TaskManagerJni::Get().Touch(taskId, token, info);
    }
    REQUEST_HILOGI("get task info: %{public}d", result);
    return result;
}

int32_t AndroidAdapter::Search(const Filter &filter, std::vector<std::string> &taskIdList)
{
    int32_t result = TaskManagerJni::Get().Search(filter, taskIdList);
    REQUEST_HILOGI("search task: %{public}d", result);
    return result;
}

int32_t AndroidAdapter::Start(int64_t taskId)
{
    int32_t result = TaskManagerJni::Get().Start(taskId);
    REQUEST_HILOGI("start task: %{public}d", result);
    return result;
}

int32_t AndroidAdapter::Pause(int64_t taskId)
{
    int32_t result = TaskManagerJni::Get().Pause(taskId);
    REQUEST_HILOGI("pause task: %{public}d", result);
    return result;
}

int32_t AndroidAdapter::Resume(int64_t taskId)
{
    int32_t result = TaskManagerJni::Get().Resume(taskId);
    REQUEST_HILOGI("resume task: %{public}d", result);
    return result;
}

int32_t AndroidAdapter::Stop(int64_t taskId)
{
    int32_t result = TaskManagerJni::Get().Stop(taskId);
    REQUEST_HILOGI("stop task: %{public}d", result);
    return result;
}

int32_t AndroidAdapter::GetDefaultStoragePath(std::string& path)
{
    int32_t result = TaskManagerJni::Get().GetDefaultStoragePath(path);
    REQUEST_HILOGI("get default storage path result: %{public}d", result);
    return result;
}

} // namespace OHOS::Plugin::Request