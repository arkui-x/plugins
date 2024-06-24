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

#ifndef PLUGINS_WIFI_MANAGER_IOS_WIFI_DEVICE_IMPL_H
#define PLUGINS_WIFI_MANAGER_IOS_WIFI_DEVICE_IMPL_H

#include "wifi_device.h"

namespace OHOS::Plugin {
class WifiDeviceImpl : public WifiDevice {
public:
    WifiDeviceImpl() = default;
    virtual ~WifiDeviceImpl() = default;
    virtual ErrCode GetLinkedInfo(WifiLinkedInfo& info) override;
    virtual ErrCode IsWifiActive(bool& bActive) override;
    virtual ErrCode IsConnected(bool& isConnected) override;
    virtual ErrCode On(const std::string& key) override;
    virtual ErrCode Off(const std::string& key) override;
};
} // namespace OHOS::Plugin

#endif // PLUGINS_WIFI_MANAGER_IOS_WIFI_DEVICE_IMPL_H
