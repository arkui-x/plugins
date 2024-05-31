/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "plugins/common_event_manager/ios/common_event_impl.h"

#include "inner_api/plugin_utils_inner.h"
#include "log.h"

#import <Foundation/Foundation.h>
#import "SubscribeManager.h"

namespace OHOS::Plugin {
std::unique_ptr<CommonEventInterface> CommonEventInterface::Create()
{
    return std::make_unique<CommonEventImpl>();
}

void CommonEventImpl::PublishCommonEvent(
    const std::string& event, const std::string& commonEventPublishData, AsyncCallbackInfo* ptr) {}

void CommonEventImpl::SubscribeCommonEvent(const std::string& key, const std::vector<std::string>& events)
{
    LOGI("CommonEventImpl SubscribeCommonEvent called, key is %{public}s.", key.c_str());
    NSMutableArray* arr = [NSMutableArray array];
    for(int i = 0; i < events.size(); i++) {
        std::string str = events[i];
        NSString* ocString = [NSString stringWithCString:str.c_str() encoding:NSUTF8StringEncoding];
        if (ocString != nil) {
            [arr addObject:ocString];
        }
    }
    NSString* ocString = [NSString stringWithCString:key.c_str() encoding:NSUTF8StringEncoding];
    [[SubscribeManager shareManager] registeSubscriber:(id)ocString events:(id)arr];
}

void CommonEventImpl::UnSubscribeCommonEvent(const std::string& key, AsyncCallbackInfo* ptr)
{
    LOGI("CommonEventImpl UnSubscribeCommonEvent called, key is %{public}s.", key.c_str());
    NSString* ocString = [NSString stringWithCString:key.c_str() encoding:NSUTF8StringEncoding];
    [[SubscribeManager shareManager] removeSubscriber:(id)ocString callbackInfo:(void*)ptr];
}
} // namespace OHOS::Plugin
