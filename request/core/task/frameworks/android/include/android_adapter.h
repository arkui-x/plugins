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

#ifndef PLUGINS_REQUEST_ANDROID_ADAPTER_H
#define PLUGINS_REQUEST_ANDROID_ADAPTER_H

#include <memory>
#include "i_task_manager_adp.h"
#include "i_task_adp.h"

namespace OHOS::Plugin::Request {
class AndroidAdapter : public ITaskManagerAdp, public ITaskAdp,
                       public std::enable_shared_from_this<AndroidAdapter> {
public:
    ITask *Create(const Config &config) override;
    int32_t Remove(int64_t taskId) override;
    int32_t GetTaskInfo(int64_t taskId, const std::string &token, TaskInfo &info) override;
    int32_t Search(const Filter &filter, std::vector<std::string> &taskIdList) override;
    int32_t Start(int64_t taskId) override;
    int32_t Pause(int64_t taskId) override;
    int32_t Resume(int64_t taskId) override;
    int32_t Stop(int64_t taskId) override;
    int32_t GetDefaultStoragePath(std::string& path) override;
};
} // namespace OHOS::Plugin::Request
#endif // PLUGINS_REQUEST_TASK_MANAGER_ADP_ANDROID_H