/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "time_ios.h"

#import <Foundation/Foundation.h>

namespace OHOS::Time {
int32_t TimeIOS::GetTimeZone(std::string& timezoneId)
{
    NSTimeZone *systemTimeZone = nil;
    systemTimeZone = [NSTimeZone systemTimeZone];
    if (systemTimeZone != nil) {
        timezoneId = std::string([systemTimeZone.name UTF8String]);
    } else {
        timezoneId = "";
    }
    return 0;
}

} // namespace OHOS::Time