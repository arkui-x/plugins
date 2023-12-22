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

#ifndef PLUGINS_REQUEST_TASK_NOTIFY_MANAGER_H
#define PLUGINS_REQUEST_TASK_NOTIFY_MANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include <set>
#include "i_task_notify.h"

namespace OHOS::Plugin::Request {
class TaskNotifyManager {
public:
    static TaskNotifyManager& Get();
    int32_t RegisterNotify(const std::string &eventType, std::shared_ptr<TaskNotifyInterface> listener);
    int32_t RemoveNotify(const std::string &eventType);
    void SendNotify(const std::string &eventType, const std::string &params);

private:
    TaskNotifyManager() = default;
    ~TaskNotifyManager();

private:
    std::recursive_mutex mutex_;
    std::map<std::string, std::shared_ptr<TaskNotifyInterface>> notifyMap_ {};
    std::map<std::string, std::string> queuedNotifyMap_ {};
};
} // namespace OHOS::Plugin::Request
#endif // PLUGINS_REQUEST_TASK_NOTIFY_MANAGER_H