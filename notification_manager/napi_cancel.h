/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef BASE_NOTIFICATION_DISTRIBUTED_NOTIFICATION_SERVICE_FRAMEWORKS_JS_NAPI_INCLUDE_CANCEL_H
#define BASE_NOTIFICATION_DISTRIBUTED_NOTIFICATION_SERVICE_FRAMEWORKS_JS_NAPI_INCLUDE_CANCEL_H

#include "common.h"

namespace OHOS {
namespace NotificationNapi {
using namespace OHOS::Notification;

struct ParametersInfoCancel {
    int32_t id = 0;
    std::string label = "";
    napi_ref callback = nullptr;
    bool hasOption = false;
};

struct AsyncCallbackInfoCancel {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    int32_t id = 0;
    std::string label;
    CallbackPromiseInfo info;
    bool hasOption = false;
};

napi_value NapiCancel(napi_env env, napi_callback_info info);
napi_value NapiCancelAll(napi_env env, napi_callback_info info);
}  // namespace NotificationNapi
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_DISTRIBUTED_NOTIFICATION_SERVICE_FRAMEWORKS_JS_NAPI_INCLUDE_CANCEL_H