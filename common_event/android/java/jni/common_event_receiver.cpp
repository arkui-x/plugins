/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "plugins/common_event/android/java/jni/common_event_receiver.h"

#include "log.h"
#include "plugin_utils.h"
#include "plugins/common_event/common_event.h"

namespace OHOS {
namespace Plugin {
void CommonEventReceiver::ReceiveCommonEvent(std::string action, std::string json)
{
    LOGI(" CommonEventReceiver ReceiveCommonEvent called.");
    CommonEvent::GetInstance()->ReceiveCommonEvent(action, json);
}

void CommonEventReceiver::ReceiveCallBack(OH_Plugin_AsyncCallbackInfo* ptr)
{
    PluginUtils::RunTaskOnJS([ptr]() {
        LOGI("OH_Plugin_EmitAsyncCallbackWork called.");
        OH_Plugin_EmitAsyncCallbackWork(ptr);
    });
}
}  // namespace Plugin
}  // namespace OHOS
