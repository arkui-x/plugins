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

#include "plugins/common_event/ios/common_event_impl.h"

namespace OHOS::Plugin {
std::unique_ptr<CommonEventInterface> CommonEventInterface::Create()
{
    return std::make_unique<CommonEventImpl>();
}

void CommonEventImpl::PublishCommonEvent(const std::string &event, OH_Plugin_AsyncCallbackInfo* ptr)
{}

void CommonEventImpl::SubscribeCommonEvent(const std::string &key, const std::vector<std::string> events)
{}

void CommonEventImpl::UnSubscribeCommonEvent(const std::string &key, OH_Plugin_AsyncCallbackInfo* ptr)
{}
}  // namespace OHOS::Plugin
