/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_WIFI_DEVICE_H
#define OHOS_WIFI_DEVICE_H

#include <memory>
#include <string>

#include "inner_api/plugin_utils_napi.h"
#include "wifi_errcode.h"
#include "wifi_msg.h"
#include "wifi_napi_utils.h"

namespace OHOS::Plugin {
class WifiDevice {
public:
    static std::shared_ptr<WifiDevice> GetInstance(int systemAbilityId, int instId = 0);
    virtual ErrCode GetLinkedInfo(WifiLinkedInfo& info) = 0;
    virtual ErrCode IsWifiActive(bool& bActive) = 0;
    virtual ErrCode IsConnected(bool& isConnected) = 0;
    virtual ErrCode On(const std::string& key) = 0;
    virtual ErrCode Off(const std::string& key) = 0;
};
} // namespace OHOS::Plugin

#endif // OHOS_WIFI_DEVICE_H
