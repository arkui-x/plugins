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

#include "task_notify_proxy.h"
#include "constant.h"
#include "log.h"

namespace OHOS::Plugin::Request {
TaskNotifyProxy::TaskNotifyProxy(std::shared_ptr<TaskNotifyInterface> listener)
{
    listenerSet_.emplace(listener);
}

TaskNotifyProxy::~TaskNotifyProxy()
{
    std::lock_guard<std::recursive_mutex> autoLock(mutex_);
    listenerSet_.clear();
}

int32_t TaskNotifyProxy::AddSubListener(std::shared_ptr<TaskNotifyInterface> listener)
{
    if (listener == nullptr) {
        REQUEST_HILOGE("invalid listener");
        return E_PARAMETER_CHECK;
    }

    std::lock_guard<std::recursive_mutex> autoLock(mutex_);
    auto it = listenerSet_.find(listener);
    if (it != listenerSet_.end()) {
        REQUEST_HILOGE("duplicate listener");
        return E_PARAMETER_CHECK;
    }
    listenerSet_.emplace(listener);
    return E_OK;
}

int32_t TaskNotifyProxy::RemoveSubListener(std::shared_ptr<TaskNotifyInterface> listener)
{
    if (listener == nullptr) {
        REQUEST_HILOGE("invalid listener");
        return E_PARAMETER_CHECK;
    }

    std::lock_guard<std::recursive_mutex> autoLock(mutex_);
    auto it = listenerSet_.find(listener);
    if (it == listenerSet_.end()) {
        REQUEST_HILOGE("invalid listener");
        return E_PARAMETER_CHECK;
    }
    listenerSet_.erase(it);
    return E_OK;
}

size_t TaskNotifyProxy::Size()
{
    std::lock_guard<std::recursive_mutex> autoLock(mutex_);
    return listenerSet_.size();
}

void TaskNotifyProxy::OnCallback(const std::string &params)
{
    std::lock_guard<std::recursive_mutex> autoLock(mutex_);
    for (auto &listener : listenerSet_) {
        if (listener != nullptr) {
            listener->OnCallback(params);
        }
    }
}
} // namespace OHOS::Plugin::Request
