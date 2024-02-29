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

#include <cstring>

#include <map>
#include <mutex>
#include "log.h"

#include "wifi_device_impl.h"
#include "wifi_device_jni.h"

namespace OHOS::Plugin {
std::shared_ptr<WifiDevice> WifiDevice::GetInstance(int systemAbilityId, int instId)
{
    return std::make_shared<WifiDeviceImpl>();
}

ErrCode WifiDeviceImpl::GetLinkedInfo(WifiLinkedInfo& info)
{
    return WiFiDeviceJni::GetInstance().GetLinkedInfo(info);
}

ErrCode WifiDeviceImpl::IsWifiActive(bool& bActive)
{
    return WiFiDeviceJni::GetInstance().IsWifiActive(bActive);
}

ErrCode WifiDeviceImpl::IsConnected(bool& isConnected)
{
    return WiFiDeviceJni::GetInstance().IsConnected(isConnected);
}

ErrCode WifiDeviceImpl::On(const std::string& key)
{
    return WiFiDeviceJni::GetInstance().On(key);
}

ErrCode WifiDeviceImpl::Off(const std::string& key)
{
    return WiFiDeviceJni::GetInstance().Off(key);
}
} // namespace OHOS::Plugin
