/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include "plugins/i18n/android/java/jni/i18n_plugin_impl.h"

#include <string>
#include "plugins/i18n/android/java/jni/i18n_plugin_jni.h"


namespace OHOS::Plugin {
std::unique_ptr<I18N> I18N::Create()
{
    return std::make_unique<I18NPluginImpl>();
}

bool I18NPluginImpl::Is24HourClock()
{
    return I18NPluginJni::Is24HourClock();
}

std::string I18NPluginImpl::GetSystemLocale()
{
    return I18NPluginJni::GetSystemLocale();
}

std::string I18NPluginImpl::GetSystemLanguage()
{
    return I18NPluginJni::GetSystemLanguage();
}

std::string I18NPluginImpl::GetSystemRegion()
{
    return I18NPluginJni::GetSystemRegion();
}

std::string I18NPluginImpl::GetSystemTimezone()
{
    return I18NPluginJni::GetSystemTimezone();
}

std::string I18NPluginImpl::GetAppPreferredLanguage()
{
    return I18NPluginJni::GetAppPreferredLanguage();
}

void I18NPluginImpl::SetAppPreferredLanguage(const std::string& languageTag)
{
    I18NPluginJni::SetAppPreferredLanguage(languageTag);
}

std::unordered_set<std::string> I18NPluginImpl::GetSystemLanguages()
{
    return I18NPluginJni::GetSystemLanguages();
}

std::unordered_set<std::string> I18NPluginImpl::GetSystemCountries(const std::string& language)
{
    return I18NPluginJni::GetSystemCountries(language);
}

std::unordered_set<std::string> I18NPluginImpl::GetAvailableIDs()
{
    return I18NPluginJni::GetAvailableIDs();
}

std::vector<std::string> I18NPluginImpl::GetPreferredLanguages()
{
    return I18NPluginJni::GetPreferredLanguages();
}

std::string I18NPluginImpl::GetFirstPreferredLanguage()
{
    return I18NPluginJni::GetFirstPreferredLanguage();
}

bool I18NPluginImpl::IsSuggested(const std::string& language, const std::string& region)
{
    return I18NPluginJni::IsSuggested(language, region);
}

bool I18NPluginImpl::GetUsingLocalDigit()
{
    return I18NPluginJni::GetUsingLocalDigit();
}
} // namespace OHOS::Plugin
