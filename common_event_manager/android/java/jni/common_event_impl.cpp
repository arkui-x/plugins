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

#include "plugins/common_event_manager/android/java/jni/common_event_impl.h"

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugins/common_event_manager/android/java/jni/broadcast_jni.h"

namespace OHOS {
namespace Plugin {
std::unique_ptr<CommonEventInterface> CommonEventInterface::Create()
{
    return std::make_unique<CommonEventImpl>();
}

void CommonEventImpl::PublishCommonEvent(
    const std::string& event, const std::string& commonEventPublishData, AsyncCallbackInfo* ptr)
{
    LOGI("Called.");
    BroadcastJni::GetInstance().SendBroadcast(event, commonEventPublishData, ptr);
}

void CommonEventImpl::SubscribeCommonEvent(const std::string& key, const std::vector<std::string>& events)
{
    LOGI("Called.");
    BroadcastJni::GetInstance().RegisterBroadcast(key, events);
}

void CommonEventImpl::UnSubscribeCommonEvent(const std::string& key, AsyncCallbackInfo* ptr)
{
    LOGI("Called.");
    BroadcastJni::GetInstance().UnRegisterBroadcast(key, ptr);
}
} // namespace Plugin
} // namespace OHOS