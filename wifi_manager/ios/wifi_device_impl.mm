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

#include "log.h"
#include "wifi_device_impl.h"
#import "wifi_utils_ios.h"
#include "wifi_callback.h"

namespace OHOS::Plugin {
std::shared_ptr<WifiDevice> WifiDevice::GetInstance(int systemAbilityId, int instId) {
    return std::make_shared<WifiDeviceImpl>();
}

ErrCode WifiDeviceImpl::GetLinkedInfo(WifiLinkedInfo& info) {
    ErrCode errCode = [[wifi_utils_ios shareManager] getLinkedInfo:&info];
    return errCode;
}

ErrCode WifiDeviceImpl::IsWifiActive(bool& bActive) {
    ErrCode errCode = [[wifi_utils_ios shareManager] isWifiActive:&bActive];
    return errCode;
}

ErrCode WifiDeviceImpl::IsConnected(bool& isConnected) {
    ErrCode errcode = [[wifi_utils_ios shareManager] isConnected:&isConnected];
    return errcode;
}

ErrCode WifiDeviceImpl::On(const std::string& key) {
    NSString *ocKey = [NSString stringWithCString:key.c_str() encoding:[NSString defaultCStringEncoding]];
    [[wifi_utils_ios shareManager] onChange:ocKey];
    return ErrCode::WIFI_OPT_SUCCESS;
}

ErrCode WifiDeviceImpl::Off(const std::string& key) {
    NSString *ocKey = [NSString stringWithCString:key.c_str() encoding:[NSString defaultCStringEncoding]];
    [[wifi_utils_ios shareManager] offChange:ocKey];
    return ErrCode::WIFI_OPT_SUCCESS;
}
} // namespace OHOS::Plugin
