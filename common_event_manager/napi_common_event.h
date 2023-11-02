/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_COMMON_EVENT_MANAGER_NAPI_COMMON_EVENT_H
#define PLUGINS_COMMON_EVENT_MANAGER_NAPI_COMMON_EVENT_H

#include <cstddef>
#include <map>

#include "common_event_manager.h"
#include "inner_api/plugin_utils_napi.h"
#include "log.h"
#include "plugin_utils.h"

namespace OHOS {
namespace Plugin {
class SubscriberInstance;
struct CallbackPromiseInfo {
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    bool isCallback = false;
    int8_t errorCode = 0;
};

struct CommonEventDataWorker {
    napi_env env = nullptr;
    napi_ref ref = nullptr;
    std::string event;
    std::string bundleName;
    int32_t code = 0;
    std::string data;
};

struct AsyncCallbackInfoCreate {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    napi_ref subscribeInfo = nullptr;
    CallbackPromiseInfo info;
    napi_value result = nullptr;
};

struct AsyncCallbackInfoSubscribeInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    std::shared_ptr<SubscriberInstance> subscriber = nullptr;
    std::vector<std::string> events;
    std::string permission;
    std::string deviceId;
    int32_t userId = 0;
    int32_t priority = 0;
    CallbackPromiseInfo info;
};
class SubscriberInstance : public CommonEventSubscriber {
public:
    explicit SubscriberInstance(const CommonEventSubscribeInfo& subscribeInfo);
    ~SubscriberInstance() override;

    void OnReceiveEvent(const CommonEventData& data) override;

    void SetEnv(const napi_env& env);
    void SetCallbackRef(const napi_ref& ref);

private:
    napi_env env_ = nullptr;
    napi_ref ref_ = nullptr;
};

class SubscriberInstanceWrapper {
public:
    explicit SubscriberInstanceWrapper(const CommonEventSubscribeInfo& info);
    std::shared_ptr<SubscriberInstance> GetSubscriber();

private:
    std::shared_ptr<SubscriberInstance> subscriber = nullptr;
};

napi_value CommonEventInit(napi_env env, napi_value exports);
napi_value CommonEventSubscriberInit(napi_env env, napi_value exports);
napi_value GetSubscribeInfo(napi_env env, napi_callback_info info);
napi_value CommonEventSubscriberConstructor(napi_env env, napi_callback_info info);
napi_value CreateSubscriber(napi_env env, napi_callback_info info);
napi_value PublishCommonEvent(napi_env env, napi_callback_info info);
napi_value SubscribeCommonEvent(napi_env env, napi_callback_info info);
napi_value UnSubscribeCommonEvent(napi_env env, napi_callback_info info);
} // namespace Plugin
} // namespace OHOS
#endif // PLUGINS_COMMON_EVENT_MANAGER_NAPI_COMMON_EVENT_H