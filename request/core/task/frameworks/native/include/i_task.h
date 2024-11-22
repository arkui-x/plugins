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

#ifndef PLUGINS_REQUEST_I_TASK_H
#define PLUGINS_REQUEST_I_TASK_H

#include <memory>
#include <string>
#include "constant.h"
#include "i_task_notify.h"
namespace OHOS::Plugin::Request {
class ITask {
public:
    explicit ITask(const Config &config) : taskId_(INVALID_TASK_ID), config_(config) {}
    virtual ~ITask() {}
    void SetId(int64_t taskId) { taskId_ = taskId; }
    int64_t GetId() const { return taskId_; }
    std::string GetTid() const { return std::to_string(taskId_); }
    Version GetVersion() const { return config_.version; }
    Action GetAction() const { return config_.action; }
    std::string GetToken() const { return config_.token; }

    virtual int32_t Start() = 0;
    virtual bool IsStarted() = 0;
    virtual int32_t Pause() = 0;
    virtual int32_t Resume() = 0;
    virtual int32_t Stop() = 0;
    virtual int32_t AddListener(const std::string &type, std::shared_ptr<TaskNotifyInterface> listener) = 0;
    virtual int32_t RemoveListener(const std::string &type, std::shared_ptr<TaskNotifyInterface> listener) = 0;

protected:
    int64_t taskId_ = INVALID_TASK_ID;
    Config config_ {};
};
} // namespace OHOS::Plugin::Request
#endif // PLUGINS_REQUEST_I_TASK_H