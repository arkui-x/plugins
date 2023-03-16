/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "plugins/interfaces/native/plugin_utils.h"

#include <future>

#include "frameworks/core/common/container.h"
#ifdef ANDROID_PLATFORM
#include "adapter/android/capability/java/jni/grantresult/grant_result_manager.h"
#include "adapter/android/capability/java/jni/plugin/plugin_manager_jni.h"
#endif

namespace OHOS::Plugin {

void PluginUtils::RegisterPlugin(RegisterCallback callback, const std::string& className)
{
#ifdef ANDROID_PLATFORM
    OHOS::Ace::Platform::PluginManagerJni::RegisterPlugin(callback, className);
#endif
}

void PluginUtils::RunTaskOnPlatform(const Task& task)
{
    auto taskExecutor = OHOS::Ace::Container::CurrentTaskExecutor();
    if (taskExecutor) {
        taskExecutor->PostTask(task, OHOS::Ace::TaskExecutor::TaskType::PLATFORM);
    }
}

void PluginUtils::RunSyncTaskOnLocal(const Task& task, std::chrono::milliseconds timeout)
{
    std::future<void> future = std::async(std::launch::async, task);
    while (future.wait_for(timeout) == std::future_status::timeout) {
        LOGE("RunSyncTaskOnLocal timeout ...");
        return;
    }
}

void PluginUtils::RunTaskOnJS(const Task& task)
{
    auto taskExecutor = OHOS::Ace::Container::CurrentTaskExecutor();
    if (taskExecutor) {
        taskExecutor->PostTask(task, OHOS::Ace::TaskExecutor::TaskType::JS);
    }
}

void PluginUtils::RunSyncTaskOnJS(const Task& task)
{
    auto taskExecutor = OHOS::Ace::Container::CurrentTaskExecutor();
    if (taskExecutor) {
        taskExecutor->PostSyncTask(task, OHOS::Ace::TaskExecutor::TaskType::JS);
    }
}

void PluginUtils::JSRegisterGrantResult(GrantResult grantResult)
{
#ifdef ANDROID_PLATFORM
    OHOS::Ace::Platform::GrantResultManager::JSRegisterGrantResult(grantResult);
#endif
}
} // namespace OHOS::Plugin
