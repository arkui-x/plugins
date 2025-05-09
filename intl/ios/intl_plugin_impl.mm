/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "plugins/intl/ios/intl_plugin_impl.h"

#include <string>

#import "intl_plugin.h"

namespace OHOS::Plugin {
std::unique_ptr<INTL> INTL::Create()
{
    return std::make_unique<INTLPluginImpl>();
}

bool INTLPluginImpl::Is24HourClock()
{
    return [[iOSINTLPlugin shareinstance] is24HourClock];
}

std::string INTLPluginImpl::GetSystemLocale()
{
    NSString *origin = [[iOSINTLPlugin shareinstance] getSystemLocale];
    std::string result = std::string([origin UTF8String]);
    return result;
}

std::string INTLPluginImpl::GetSystemTimezone()
{
    NSString *origin = [[iOSINTLPlugin shareinstance] getSystemTimezone];
    std::string result = std::string([origin UTF8String]);
    return result;
}

std::string INTLPluginImpl::GetSystemCalendar()
{
    NSString *origin = [[iOSINTLPlugin shareinstance] getSystemCalendar];
    std::string result = std::string([origin UTF8String]);
    return result;
}

std::string INTLPluginImpl::GetNumberingSystem()
{
    NSString *origin = [[iOSINTLPlugin shareinstance] getNumberingSystem];
    std::string result = std::string([origin UTF8String]);
    return result;
}

std::string INTLPluginImpl::GetDeviceType()
{
    NSString *origin = [[iOSINTLPlugin shareinstance] getDeviceType];
    std::string result = std::string([origin UTF8String]);
    return result;
}
} // namespace OHOS::Plugin