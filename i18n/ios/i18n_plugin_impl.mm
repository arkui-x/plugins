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
#include "plugins/i18n/ios/i18n_plugin_impl.h"

#import "i18n_plugin.h"

namespace OHOS::Plugin {
std::unique_ptr<I18N> I18N::Create()
{
    return std::make_unique<I18NPluginImpl>();
}

bool I18NPluginImpl::Is24HourClock()
{
    return [[iOSI18NPlugin shareinstance] is24HourClock];
}

std::string INTLPluginImpl::GetSystemLocale()
{
    NSString *origin = [[iOSINTLPlugin shareinstance] getSystemLocale]
    std::string result = std::string([origin UTF8String]);
    return result;
}

std::string INTLPluginImpl::GetSystemLanguage()
{
    NSString *origin = [[iOSINTLPlugin shareinstance] getSystemLanguage]
    std::string result = std::string([origin UTF8String]);
    return result;
}

std::string INTLPluginImpl::GetSystemRegion()
{
    NSString *origin = [[iOSINTLPlugin shareinstance] getSystemRegion]
    std::string result = std::string([origin UTF8String]);
    return result;
}

std::string INTLPluginImpl::GetSystemTimezone()
{
    NSString *origin = [[iOSINTLPlugin shareinstance] getSystemTimezone]
    std::string result = std::string([origin UTF8String]);
    return result;
}
} // namespace OHOS::Plugin