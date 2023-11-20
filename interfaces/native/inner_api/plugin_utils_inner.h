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

#ifndef PLUGIN_INTERFACE_NATIVE_PLUGIN_UTILS_H
#define PLUGIN_INTERFACE_NATIVE_PLUGIN_UTILS_H

#include <chrono>
#include <string>
#include <vector>

#include "event_handler.h"

#ifndef PLUGIN_EXPORT
#ifdef WINDOWS_PLATFORM
#define PLUGIN_EXPORT __declspec(dllexport)
#else
#define PLUGIN_EXPORT __attribute__((visibility("default")))
#endif
#endif

#define CHECK_NULL_VOID(ptr)                                            \
    do {                                                                \
        if (!(ptr)) {                                                   \
            LOGW(#ptr " is null, return on line %{public}d", __LINE__); \
            return;                                                     \
        }                                                               \
    } while (0)

#define CHECK_NULL_RETURN(ptr, ret)                                     \
    do {                                                                \
        if (!(ptr)) {                                                   \
            LOGW(#ptr " is null, return on line %{public}d", __LINE__); \
            return ret;                                                 \
        }                                                               \
    } while (0)

namespace OHOS::Plugin {
using RegisterCallback = bool (*)(void*);
using Task = std::function<void()>;
using GrantResult =
    std::function<void(const std::vector<std::string>& permissions, const std::vector<int>& grantResults)>;

class PLUGIN_EXPORT PluginUtilsInner {
public:
    static void RegisterPlugin(RegisterCallback callback, const std::string& packageName);
    static void RegisterPluginOnEvent(const Task& task);
    static void RunTaskOnEvent(const Task& task, std::shared_ptr<OHOS::AppExecFwk::EventRunner> eventRunner);
    static void RunTaskOnPlatform(const Task& task);
    static void RunSyncTaskOnLocal(const Task& task, std::chrono::milliseconds timeout);
    static void RunTaskOnJS(const Task& task);
    static void RunSyncTaskOnJS(const Task& task);
    static void JSRegisterGrantResult(GrantResult grantResult);
    static int32_t GetInstanceId();

private:
    static std::shared_ptr<OHOS::AppExecFwk::EventHandler> eventHandler;
};
} // namespace OHOS::Plugin

#endif // PLUGIN_INTERFACE_NATIVE_PLUGIN_UTILS_H
