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

#ifndef PLUGINS_REQUEST_TASK_NOTIFY_PROXY_H
#define PLUGINS_REQUEST_TASK_NOTIFY_PROXY_H

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "i_task_notify.h"

namespace OHOS::Plugin::Request {
class TaskNotifyProxy : public TaskNotifyInterface {
public:
    explicit TaskNotifyProxy(std::shared_ptr<TaskNotifyInterface> listener);
    virtual ~TaskNotifyProxy();
    int32_t AddSubListener(std::shared_ptr<TaskNotifyInterface> listener);
    int32_t RemoveSubListener(std::shared_ptr<TaskNotifyInterface> listener);
    size_t Size();
    void OnCallback(const std::string &params) override;

private:
    std::recursive_mutex mutex_;
    std::vector<std::shared_ptr<TaskNotifyInterface>> listenerList_ {};
};
} // namespace OHOS::Plugin::Request
#endif // PLUGINS_REQUEST_TASK_NOTIFY_PROXY_H