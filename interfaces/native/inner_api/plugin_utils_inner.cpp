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

#include "plugins/interfaces/native/inner_api/plugin_utils_inner.h"

#include <future>

#include "frameworks/core/common/container.h"
#ifdef ANDROID_PLATFORM
#include "adapter/android/capability/java/jni/grantresult/grant_result_manager.h"
#include "adapter/android/capability/java/jni/plugin/plugin_manager_jni.h"
#endif

namespace OHOS::Plugin {
std::shared_ptr<OHOS::AppExecFwk::EventHandler> PluginUtilsInner::eventHandler = nullptr;

void PluginUtilsInner::RegisterPlugin(RegisterCallback callback, const std::string& className)
{
#ifdef ANDROID_PLATFORM
    auto task = [callback, className]() {
        OHOS::Ace::Platform::PluginManagerJni::RegisterPlugin(callback, className);
    };
    auto taskExecutor = OHOS::Ace::Container::CurrentTaskExecutor();
    if (!taskExecutor) {
        RegisterPluginOnEvent(task);
    } else {
        if (taskExecutor->WillRunOnCurrentThread(OHOS::Ace::TaskExecutor::TaskType::PLATFORM)) {
           task();
        } else {
            taskExecutor->PostTask(task, OHOS::Ace::TaskExecutor::TaskType::PLATFORM, "ArkUI-XPluginUtilsInnerRegisterPlugin");
        }
    }
#endif
}

void PluginUtilsInner::RegisterPluginOnEvent(const Task& task)
{
#ifdef ANDROID_PLATFORM
    auto eventRunner = AppExecFwk::EventRunner::Current();
    if (!eventRunner) {
        LOGE("RegisterPluginOnEvent eventRunner is nullptr");
        return;
    }
    if (eventRunner->IsCurrentRunnerThread()) {
        task();
    } else {
        RunTaskOnEvent(task, eventRunner);
    }
#endif
}

void PluginUtilsInner::RunTaskOnEvent(const Task& task, std::shared_ptr<OHOS::AppExecFwk::EventRunner> eventRunner)
{
    eventHandler = std::make_shared<OHOS::AppExecFwk::EventHandler>(eventRunner);
    if (!eventHandler) {
        LOGE("RunTaskOnEvent eventHandler is nullptr");
        return;
    }
    eventHandler->PostTask(task);
}

void PluginUtilsInner::RunTaskOnPlatform(const Task& task)
{
    auto taskExecutor = OHOS::Ace::Container::CurrentTaskExecutor();
    if (taskExecutor) {
        taskExecutor->PostTask(task, OHOS::Ace::TaskExecutor::TaskType::PLATFORM, "ArkUI-XPluginUtilsInnerRunTaskOnPlatform");
    } else {
        auto eventRunner = AppExecFwk::EventRunner::Current();
        if (!eventRunner) {
            LOGE("RunTaskOnPlatform eventRunner is nullptr");
            return;
        }
        if (eventRunner->IsCurrentRunnerThread()) {
            task();
        } else {
            RunTaskOnEvent(task, eventRunner);
        }
    }
}

void PluginUtilsInner::RunSyncTaskOnLocal(const Task& task, std::chrono::milliseconds timeout)
{
    std::future<void> future = std::async(std::launch::async, task);
    while (future.wait_for(timeout) == std::future_status::timeout) {
        LOGE("RunSyncTaskOnLocal timeout ...");
        return;
    }
}

void PluginUtilsInner::RunTaskOnJS(const Task& task)
{
    auto taskExecutor = OHOS::Ace::Container::CurrentTaskExecutor();
    if (taskExecutor) {
        taskExecutor->PostTask(task, OHOS::Ace::TaskExecutor::TaskType::JS, "ArkUI-XPluginUtilsInnerRunTaskOnJS");
    }
}

void PluginUtilsInner::RunSyncTaskOnJS(const Task& task)
{
    auto taskExecutor = OHOS::Ace::Container::CurrentTaskExecutor();
    if (taskExecutor) {
        taskExecutor->PostSyncTask(task, OHOS::Ace::TaskExecutor::TaskType::JS, "ArkUI-XPluginUtilsInnerRunSyncTaskOnJS");
    }
}

void PluginUtilsInner::JSRegisterGrantResult(GrantResult grantResult)
{
#ifdef ANDROID_PLATFORM
    OHOS::Ace::Platform::GrantResultManager::JSRegisterGrantResult(grantResult);
#endif
}

int32_t PluginUtilsInner::GetInstanceId()
{
    return OHOS::Ace::Container::CurrentIdSafelyWithCheck();
}
} // namespace OHOS::Plugin
