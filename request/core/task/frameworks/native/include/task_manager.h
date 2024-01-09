/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_REQUEST_TASK_MANAGER_H
#define PLUGINS_REQUEST_TASK_MANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include "constant.h"
#include "i_task.h"
#include "i_task_manager_adp.h"

namespace OHOS::Plugin::Request {
class TaskManager {
public:
    static TaskManager& Get();
    ITask *Create(const Config &config);
    int32_t Remove(int64_t taskId);
    int32_t GetTaskInfo(int64_t taskId, const std::string &token, TaskInfo &info);
    int32_t Search(const Filter &filter, std::vector<std::string> &taskIdList);
    int32_t GetDefaultStoragePath(std::string& path);

private:
    TaskManager() = default;
    ~TaskManager() = default;
    TaskManager(const TaskManager&) = delete;
    TaskManager& operator=(const TaskManager&) = delete;

private:
    std::recursive_mutex mutex_;
    std::shared_ptr<ITaskManagerAdp> adapter_ = nullptr;
};
} // namespace OHOS::Plugin::Request
#endif // PLUGINS_REQUEST_DOWNLOAD_MANAGER_H