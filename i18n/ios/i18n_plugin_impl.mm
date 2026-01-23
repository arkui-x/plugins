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

#include <string>

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

std::string I18NPluginImpl::GetSystemLocale()
{
    NSString *origin = [[iOSI18NPlugin shareinstance] getSystemLocale];
    std::string result = std::string([origin UTF8String]);
    return result;
}

std::string I18NPluginImpl::GetSystemLanguage()
{
    NSString *origin = [[iOSI18NPlugin shareinstance] getSystemLanguage];
    std::string result = std::string([origin UTF8String]);
    return result;
}

std::string I18NPluginImpl::GetSystemRegion()
{
    NSString *origin = [[iOSI18NPlugin shareinstance] getSystemRegion];
    std::string result = std::string([origin UTF8String]);
    return result;
}

std::string I18NPluginImpl::GetSystemTimezone()
{
    NSString *origin = [[iOSI18NPlugin shareinstance] getSystemTimezone];
    std::string result = std::string([origin UTF8String]);
    return result;
}

std::string I18NPluginImpl::GetAppPreferredLanguage()
{
    NSString *origin = [[iOSI18NPlugin shareinstance] getAppPreferredLanguage];
    std::string result = std::string([origin UTF8String]);
    return result;
}

void I18NPluginImpl::SetAppPreferredLanguage(const std::string& languageTag)
{
    NSString *nsLanguageTag = [NSString stringWithUTF8String:languageTag.c_str()];
    [[iOSI18NPlugin shareinstance] setAppPreferredLanguage:nsLanguageTag];
}

std::unordered_set<std::string> I18NPluginImpl::GetSystemLanguages()
{
    std::unordered_set<std::string> result;
    NSArray *languages = [[iOSI18NPlugin shareinstance] getSystemLanguages];
    for (NSString *language in languages) {
        result.insert(std::string([language UTF8String]));
    }
    return result;
}

std::unordered_set<std::string> I18NPluginImpl::GetSystemCountries(const std::string& language)
{
    std::unordered_set<std::string> result;
    NSString *nsLanguageTag = [NSString stringWithUTF8String:language.c_str()];
    NSArray *countries = [[iOSI18NPlugin shareinstance] getSystemCountries:nsLanguageTag];
    for (NSString *country in countries) {
        result.insert(std::string([country UTF8String]));
    }
    return result;
}

std::unordered_set<std::string> I18NPluginImpl::GetAvailableIDs()
{
    std::unordered_set<std::string> result;
    NSArray *availableIDs = [[iOSI18NPlugin shareinstance] getAvailableIDs];
    for (NSString *availableID in availableIDs) {
        result.insert(std::string([availableID UTF8String]));
    }
    return result;
}

std::vector<std::string> I18NPluginImpl::GetPreferredLanguages()
{
    std::vector<std::string> result;
    NSArray *languages = [[iOSI18NPlugin shareinstance] getPreferredLanguages];
    for (NSString *language in languages) {
        NSLocale *locale = [NSLocale localeWithLocaleIdentifier:language];
        NSString *languageCode = [locale languageCode];
        NSString *scriptCode = [locale scriptCode];
        NSString *finalLanguage = languageCode;
        if (scriptCode) {
            finalLanguage = [NSString stringWithFormat:@"%@-%@", languageCode, scriptCode];
        }
        result.push_back(std::string([finalLanguage UTF8String]));
    }
    return result;
}

std::string I18NPluginImpl::GetFirstPreferredLanguage()
{
    NSArray *languages = [[iOSI18NPlugin shareinstance] getPreferredLanguages];
    if (!languages || languages.count == 0) {
        return std::string();
    }
    NSLocale *locale = [NSLocale localeWithLocaleIdentifier:languages.firstObject];
    NSString *languageCode = [locale languageCode];
    NSString *scriptCode = [locale scriptCode];
    NSString *finalLanguage = languageCode;
    if (scriptCode) {
        finalLanguage = [NSString stringWithFormat:@"%@-%@", languageCode, scriptCode];
    }
    return std::string([finalLanguage UTF8String]);
}

bool I18NPluginImpl::IsSuggested(const std::string& language, const std::string& region)
{
    NSString *nsLanguageTag = [NSString stringWithUTF8String:language.c_str()];
    NSString *nsRegionTag = [NSString stringWithUTF8String:region.c_str()];
    return [[iOSI18NPlugin shareinstance] isSuggested:nsLanguageTag regionCode:nsRegionTag];
}

bool I18NPluginImpl::GetUsingLocalDigit()
{
    return [[iOSI18NPlugin shareinstance] isUsingLocalDigit];
}
} // namespace OHOS::Plugin