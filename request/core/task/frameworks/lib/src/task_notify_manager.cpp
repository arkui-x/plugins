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

#include "task_notify_manager.h"
#include "constant.h"
#include "log.h"

namespace OHOS::Plugin::Request {
TaskNotifyManager& TaskNotifyManager::Get()
{
    static TaskNotifyManager manager;
    return manager;
}

TaskNotifyManager::~TaskNotifyManager()
{
    std::lock_guard<std::recursive_mutex> autoLock(mutex_);
    notifyMap_.clear();
    queuedNotifyMap_.clear();
}

int32_t TaskNotifyManager::RegisterNotify(const std::string &eventType, std::shared_ptr<TaskNotifyInterface> listener)
{
    if (eventType == "") {
        REQUEST_HILOGE("invalid event type");
        return E_PARAMETER_CHECK;
    }

    if (listener == nullptr) {
        REQUEST_HILOGE("invalid listener");
        return E_PARAMETER_CHECK;
    }

    REQUEST_HILOGI("Register Notify [%{public}s]", eventType.c_str());
    std::lock_guard<std::recursive_mutex> autoLock(mutex_);
    auto it = notifyMap_.find(eventType);
    if (it != notifyMap_.end()) {
        REQUEST_HILOGE("duplicate listener");
        return E_PARAMETER_CHECK;
    }
    auto queuedIt = queuedNotifyMap_.find(eventType);
    if (queuedIt != queuedNotifyMap_.end()) {
        listener->OnCallback(queuedIt->second);
        queuedNotifyMap_.erase(queuedIt);
    }
    notifyMap_.emplace(eventType, listener);
    REQUEST_HILOGI("RegisterNotify notifyMap_.size: [%{public}zu]", notifyMap_.size());
    return E_OK;
}

int32_t TaskNotifyManager::RemoveNotify(const std::string &eventType)
{
    if (eventType == "") {
        REQUEST_HILOGE("invalid event type");
        return E_PARAMETER_CHECK;
    }

    REQUEST_HILOGI("Remove Notify [%{public}s]", eventType.c_str());
    std::lock_guard<std::recursive_mutex> autoLock(mutex_);
    auto it = notifyMap_.find(eventType);
    if (it == notifyMap_.end()) {
        REQUEST_HILOGE("invalid event type");
        return E_PARAMETER_CHECK;
    }
    notifyMap_.erase(it);
    REQUEST_HILOGI("RemoveNotify notifyMap_.size: [%{public}zu]", notifyMap_.size());
    return E_OK;
}

void TaskNotifyManager::SendNotify(const std::string &eventType, const std::string &params)
{
    REQUEST_HILOGI("Get Notify [%{public}s]", eventType.c_str());
    std::lock_guard<std::recursive_mutex> autoLock(mutex_);
    auto it = notifyMap_.find(eventType);
    if (it == notifyMap_.end()) {
        REQUEST_HILOGE("invalid event type: %{public}s", eventType.c_str());
        auto queuedIt = queuedNotifyMap_.find(eventType);
        if (queuedIt != queuedNotifyMap_.end()) {
            queuedIt->second = params;
        } else {
            queuedNotifyMap_.emplace(eventType, params);
        }
        return;
    }
    if (it->second != nullptr) {
        it->second->OnCallback(params);
    }
}
} // namespace OHOS::Plugin::Request
