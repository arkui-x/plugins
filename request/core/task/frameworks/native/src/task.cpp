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

#include "task.h"
#include "log.h"
#include "request_utils.h"
#include "task_manager.h"
#include "task_notify_manager.h"
#include <unistd.h>

namespace OHOS::Plugin::Request {
Task::Task(const Config &config, std::shared_ptr<ITaskAdp> taskAdp) : ITask(config), adapter_(taskAdp)
{
    REQUEST_HILOGI("alloc task [%{public}ld]", GetId());
}

Task::~Task()
{
    REQUEST_HILOGI("free task [%{public}ld]", GetId());
    for (auto &file : config_.files) {
        if (file.fd >= 0) {
            close(file.fd);
            file.fd = 0;
        }
    }
    for (auto &it : notifyMap_) {
        TaskNotifyManager::Get().RemoveNotify(RequestUtils::GetEventType(GetId(), it.first));
    }
}

int32_t Task::Start()
{
    if (adapter_ == nullptr) {
        REQUEST_HILOGE("no task adapter");
        return E_SERVICE_ERROR;
    }
    if (adapter_->Start(GetId()) != E_OK) {
        REQUEST_HILOGE("fail to start task");
        return E_SERVICE_ERROR;
    }
    isStarted_ = true;
    return E_OK;
}

bool Task::IsStarted()
{
    return isStarted_;
}

int32_t Task::Pause()
{
    if (adapter_ == nullptr) {
        REQUEST_HILOGE("no task adapter");
        return E_SERVICE_ERROR;
    }
    if (adapter_->Pause(GetId()) != E_OK) {
        REQUEST_HILOGE("fail to pause task");
        return E_SERVICE_ERROR;
    }
    return E_OK;
}

int32_t Task::Resume()
{
    if (adapter_ == nullptr) {
        REQUEST_HILOGE("no task adapter");
        return E_SERVICE_ERROR;
    }
    if (adapter_->Resume(GetId()) != E_OK) {
        REQUEST_HILOGE("fail to resume task");
        return E_SERVICE_ERROR;
    }
    return E_OK;
}

int32_t Task::Stop()
{
    if (adapter_ == nullptr) {
        REQUEST_HILOGE("no task adapter");
        return E_SERVICE_ERROR;
    }
    if (adapter_->Stop(GetId()) != E_OK) {
        REQUEST_HILOGE("fail to stop task");
        return E_SERVICE_ERROR;
    }
    isStarted_ = false;
    return E_OK;
}

int32_t Task::AddListener(const std::string &type, std::shared_ptr<TaskNotifyInterface> listener)
{
    if (type == "") {
        REQUEST_HILOGE("AddListener: invalid event type");
        return E_PARAMETER_CHECK;
    }

    if (listener == nullptr) {
        REQUEST_HILOGE("AddListener: event listener is nullptr");
        return E_PARAMETER_CHECK;
    }

    auto it = notifyMap_.find(type);
    if (it == notifyMap_.end()) {
        auto proxy = std::make_shared<TaskNotifyProxy>(listener);
        if (proxy == nullptr) {
            REQUEST_HILOGE("AddListener: fail to create task notify proxy");
            return E_SERVICE_ERROR;
        }
        notifyMap_.emplace(type, proxy);
        TaskNotifyManager::Get().RegisterNotify(RequestUtils::GetEventType(GetId(), type), proxy);
        return E_OK;
    }
    if (it->second != nullptr) {
        it->second->AddSubListener(listener);
    }
    return E_OK;
}

int32_t Task::RemoveListener(const std::string &type, std::shared_ptr<TaskNotifyInterface> listener)
{
    if (type == "") {
        REQUEST_HILOGE("AddListener: invalid event type");
        return E_PARAMETER_CHECK;
    }

    auto it = notifyMap_.find(type);
    if (it == notifyMap_.end()) {
        REQUEST_HILOGE("AddListener: no task proxy");
        return E_PARAMETER_CHECK;
    }

    if (it->second == nullptr) {
        REQUEST_HILOGE("AddListener: invalid task proxy");
        return E_SERVICE_ERROR;
    }
    bool needRemoveProxy = false;
    if (listener == nullptr) {
        needRemoveProxy = true;
    } else {
        it->second->RemoveSubListener(listener);
        if (it->second->Size() == 0) {
            needRemoveProxy = true;
        }
    }
    if (needRemoveProxy) {
        TaskNotifyManager::Get().RemoveNotify(RequestUtils::GetEventType(GetId(), type));
        notifyMap_.erase(it);
    }
    return E_OK;
}
} // namespace OHOS::Plugin::Request
