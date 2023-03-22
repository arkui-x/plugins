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

#include "plugins/interfaces/native/plugin_c_utils.h"

#include <cstddef>

#include "base/thread/task_executor.h"
#include "frameworks/core/common/container.h"
#include "plugins/interfaces/native/plugin_utils.h"

#ifdef ANDROID_PLATFORM
#include "adapter/android/entrance/java/jni/jni_environment.h"

JNIEnv* OH_Plugin_GetJniEnv()
{
    return OHOS::Ace::Platform::JniEnvironment::GetInstance().GetJniEnv(nullptr, false).get();
}

void OH_Plugin_RegisterPlugin(bool (*func)(void*), const char* name)
{
    std::string className = name;
    OHOS::Plugin::PluginUtils::RegisterPlugin(func, className);
}
#endif

void OH_Plugin_RunTaskOnPlatform(OH_Plugin_Task task)
{
    auto taskExecutor = OHOS::Ace::Container::CurrentTaskExecutor();
    if (taskExecutor) {
        taskExecutor->PostTask([task]() { task(); }, OHOS::Ace::TaskExecutor::TaskType::PLATFORM);
    }
}
