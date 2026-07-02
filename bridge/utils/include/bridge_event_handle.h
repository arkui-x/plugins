/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_BRIDGE_UTILS_INCLUDE_BRIDGE_EVENT_HANDLE_H
#define PLUGINS_BRIDGE_UTILS_INCLUDE_BRIDGE_EVENT_HANDLE_H

#include <functional>
#include <map>
#include <memory>
#include <mutex>

#include "event_handler.h"
#include "napi/native_api.h"

namespace OHOS::Plugin::Bridge {
using Task = std::function<void()>;
class BridgeEventHandle {
public:
    BridgeEventHandle();
    ~BridgeEventHandle();
    static std::shared_ptr<BridgeEventHandle> GetInstance();
    static std::shared_ptr<BridgeEventHandle> GetInstance(napi_env env);
    static void CreateInstance(napi_env env);
    void RunTaskOnBridgeThread(const Task& task);
    void RunTaskOnMainThread(const Task& task);
    void RunTaskOnJsThread(const Task& task);

private:
    explicit BridgeEventHandle(napi_env env);
    static void CallJs(napi_env env, napi_value jsCallback, void* context, void* data);
    static void Finalize(napi_env env, void* data, void* hint);
    static void EnvCleanup(void* data);

    std::shared_ptr<AppExecFwk::EventHandler> mainEventHandler_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> bridgeEventHandler_ = nullptr;
    napi_env env_ = nullptr;
    napi_threadsafe_function tsfn_ = nullptr;

    static std::shared_ptr<BridgeEventHandle> instance_;
    static std::mutex mutex_;
    static std::mutex envMutex_;
    static std::map<napi_env, std::shared_ptr<BridgeEventHandle>> envInstances_;
};
}
#endif // PLUGINS_BRIDGE_UTILS_INCLUDE_BRIDGE_EVENT_HANDLE_H
