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

#include "log.h"

#include "wifi_receive_callback.h"
#include "plugins/wifi_manager/wifi_callback.h"
#include "plugins/wifi_manager/wifi_device.h"

namespace OHOS::Plugin {
void WifiReceiveCallback::ReceiveCallback(const std::string& key, int code)
{
    WifiCallback::GetInstance().SendCallback(key, code);
}
} // namespace OHOS::Plugin
