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

#include "plugins/common_event_manager/ios/common_event_receiver.h"

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugins/common_event_manager/common_event.h"

namespace OHOS {
namespace Plugin {
void CommonEventReceiver::ReceiveCommonEvent(std::string key, std::string action, std::string json)
{
    LOGI("CommonEventReceiver ReceiveCommonEvent called.");
    CommonEvent::GetInstance().ReceiveCommonEvent(key, action, json);
}
} // namespace Plugin
} // namespace OHOS
