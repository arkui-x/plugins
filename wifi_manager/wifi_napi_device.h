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

#ifndef WIFI_NAPI_DEVICE_H_
#define WIFI_NAPI_DEVICE_H_

#include <iostream>
#include <map>
#include <set>
#include <shared_mutex>
#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "wifi_errcode.h"
#include "wifi_msg.h"
#include "wifi_napi_utils.h"

namespace OHOS::Plugin {
napi_value IsWifiActive(napi_env env, napi_callback_info info);
napi_value GetLinkedInfo(napi_env env, napi_callback_info info);
napi_value On(napi_env env, napi_callback_info cbinfo);
napi_value Off(napi_env env, napi_callback_info cbinfo);
napi_value IsConnected(napi_env env, napi_callback_info info);

class LinkedInfoAsyncContext : public AsyncContext {
public:
    WifiLinkedInfo linkedInfo;

    LinkedInfoAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred)
    {}

    LinkedInfoAsyncContext() = delete;

    virtual ~LinkedInfoAsyncContext() {}
};
} // namespace OHOS::Plugin

#endif // WIFI_NAPI_DEVICE_H_
