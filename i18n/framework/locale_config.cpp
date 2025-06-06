/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "locale_config.h"
#include "locale_info.h"

#include <string>
#include <vector>

#include "application_context.h"
#include "localebuilder.h"
#include "locid.h"
#include "ucase.h"
#include "unistr.h"
#include "utils.h"

namespace OHOS {
namespace Global {
namespace I18n {
using namespace std;

unique_ptr<Plugin::I18N> LocaleConfig::plugin = Plugin::I18N::Create();

string LocaleConfig::GetSystemLocale()
{
    if (!plugin) {
        return "";
    }
    return plugin->GetSystemLocale();
}

string LocaleConfig::GetSystemTimezone()
{
    if (!plugin) {
        return "";
    }
    return plugin->GetSystemTimezone();
}

bool LocaleConfig::Is24HourClock()
{
    if (!plugin) {
        return false;
    }
    return plugin->Is24HourClock();
}

string LocaleConfig::GetSystemLanguage()
{
    if (!plugin) {
        return "";
    }
    return plugin->GetSystemLanguage();
}

string LocaleConfig::GetSystemRegion()
{
    if (!plugin) {
        return "";
    }
    return plugin->GetSystemRegion();
}

bool LocaleConfig::IsRTL(const string &locale)
{
    icu::Locale curLocale(locale.c_str());
    return curLocale.isRightToLeft();
}

string LocaleConfig::GetDisplayLanguage(const string &language, const string &displayLocale, bool sentenceCase)
{
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale displayLoc = icu::Locale::forLanguageTag(displayLocale, status);
    if (status != U_ZERO_ERROR) {
        return "";
    }
    icu::Locale locale = icu::Locale::forLanguageTag(language, status);
    if (status != U_ZERO_ERROR) {
        return "";
    }
    icu::UnicodeString unistr;
    std::string lang(locale.getLanguage());
    std::string result;
    locale.getDisplayLanguage(displayLoc, unistr);
    unistr.toUTF8String(result);
    if (sentenceCase) {
        char ch = static_cast<char>(toupper(result[0]));
        return result.replace(0, 1, 1, ch);
    }
    return result;
}

string LocaleConfig::GetDisplayRegion(const string &region, const string &displayLocale, bool sentenceCase)
{
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale originLocale;
    if (IsValidRegion(region)) {
        icu::LocaleBuilder builder = icu::LocaleBuilder().setRegion(region);
        originLocale = builder.build(status);
    } else {
        originLocale = icu::Locale::forLanguageTag(region, status);
    }
    std::string country(originLocale.getCountry());
    if (country.length() == 0) {
        return "";
    }
    if (status != U_ZERO_ERROR) {
        return "";
    }
    icu::Locale locale = icu::Locale::forLanguageTag(displayLocale, status);
    if (status != U_ZERO_ERROR) {
        return "";
    }
    icu::UnicodeString displayRegion;
    originLocale.getDisplayCountry(locale, displayRegion);
    if (sentenceCase) {
        UChar32 ch = ucase_toupper(displayRegion.char32At(0));
        displayRegion.replace(0, 1, ch);
    }
    string temp;
    displayRegion.toUTF8String(temp);
    return temp;
}

bool LocaleConfig::IsValidRegion(const string &region)
{
    string::size_type size = region.size();
    if (size != LocaleInfo::REGION_LEN) {
        return false;
    }
    for (size_t i = 0; i < LocaleInfo::REGION_LEN; ++i) {
        if ((region[i] > 'Z') || (region[i] < 'A')) {
            return false;
        }
    }
    return true;
}

bool LocaleConfig::IsValidTag(const std::string &tag)
{
    if (!tag.size()) {
        return false;
    }
    std::vector<std::string> splits;
    Split(tag, "-", splits);
    if (!IsValidLanguage(splits[0])) {
        return false;
    }
    return true;
}

bool LocaleConfig::IsValidLanguage(const std::string &language)
{
    std::string::size_type size = language.size();
    if ((size != LANGUAGE_LEN) && (size != LANGUAGE_LEN + 1)) {
        return false;
    }
    for (size_t i = 0; i < size; ++i) {
        if ((language[i] > 'z') || (language[i] < 'a')) {
            return false;
        }
    }
    return true;
}

std::string LocaleConfig::GetAppPreferredLanguage()
{
    if (!plugin) {
        return "";
    }
    return plugin->GetAppPreferredLanguage();
}

void LocaleConfig::SetAppPreferredLanguage(const std::string &language)
{
    if (!plugin) {
        return;
    }
    if (language.compare("default") == 0) {
        plugin->SetAppPreferredLanguage("");
        return;
    }
    plugin->SetAppPreferredLanguage(language);
    auto appContext = AbilityRuntime::Platform::ApplicationContext::GetInstance();
    if (appContext != nullptr) {
        appContext->SetLanguage(language);
    }
}
} // namespace I18n
} // namespace Global
} // namespace OHOS
