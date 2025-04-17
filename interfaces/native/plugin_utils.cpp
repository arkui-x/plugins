/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <cstddef>

#include "base/thread/task_executor.h"
#include "frameworks/core/common/container.h"
#include "plugins/interfaces/native/inner_api/plugin_utils_inner.h"

#ifdef ANDROID_PLATFORM
#include "adapter/android/entrance/java/jni/jni_environment.h"

JNIEnv* ARKUI_X_Plugin_GetJniEnv()
{
    return OHOS::Ace::Platform::JniEnvironment::GetInstance().GetJniEnv(nullptr, false).get();
}

void ARKUI_X_Plugin_RegisterJavaPlugin(bool (*func)(void*), const char* name)
{
    std::string className = name;
    OHOS::Plugin::PluginUtilsInner::RegisterPlugin(func, className);
}
#endif

void ARKUI_X_Plugin_RunAsyncTask(ARKUI_X_Plugin_Task task, ARKUI_X_Plugin_Thread_Mode mode)
{
    auto taskExecutor = OHOS::Ace::Container::CurrentTaskExecutor();
    if (taskExecutor) {
        if (mode == ARKUI_X_PLUGIN_PLATFORM_THREAD) {
            taskExecutor->PostTask([task]() { task(); }, OHOS::Ace::TaskExecutor::TaskType::PLATFORM, "ArkUI-XPluginUtilsRunAsyncTaskPlatform");
        } else if (mode == ARKUI_X_PLUGIN_JS_THREAD) {
            taskExecutor->PostTask([task]() { task(); }, OHOS::Ace::TaskExecutor::TaskType::JS, "ArkUI-XPluginUtilsRunAsyncTaskJs");
        } else {
            LOGE("The mode of thread is not support in the ARKUI_X_Plugin_RunAsyncTask method!");
        }
    } else {
        auto eventRunner = OHOS::AppExecFwk::EventRunner::Current();
        if (!eventRunner) {
            LOGE("RunTaskOnPlatform eventRunner is nullptr");
            return;
        }
        if (eventRunner->IsCurrentRunnerThread()) {
            task();
        } else {
            OHOS::Plugin::PluginUtilsInner::RunTaskOnEvent(task, eventRunner);
        }
    }
}
