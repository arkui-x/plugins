/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include <string>
#include <vector>

#include "application_context.h"
#include "locale_config_ext.h"
#include "locale_info.h"
#include "locale_matcher.h"
#include "localebuilder.h"
#include "locid.h"
#include "ohos/init_data.h"
#include "ucase.h"
#include "unicode/dcfmtsym.h"
#include "unicode/localebuilder.h"
#include "unicode/locdspnm.h"
#include "unicode/locid.h"
#include "unicode/numsys.h"
#include "unicode/smpdtfmt.h"
#include "unicode/ulocdata.h"
#include "unicode/unistr.h"
#include "unistr.h"
#include "utils.h"

namespace OHOS {
namespace Global {
namespace I18n {
using namespace std;

unique_ptr<Plugin::I18N> LocaleConfig::plugin = Plugin::I18N::Create();
std::unordered_map<std::string, std::vector<std::string>> dialectLanguages {
    { "en-Latn", { "en-Latn-US", "en-Latn-GB" } },
    { "pt-Latn", { "pt-Latn-PT", "pt-Latn-BR" } },
    { "zh-Hant", { "zh-Hant-HK", "zh-Hant-TW" } },
};

std::unordered_map<std::string, std::string> resourceIdMap {
    { "zh", "zh-Hans" },
    { "zh-HK", "zh-Hant-HK" },
    { "zh-TW", "zh-Hant" },
    { "az", "az-Latn" },
    { "bs", "bs-Latn" },
    { "jv", "jv-Latn" },
    { "uz", "uz-Latn" },
    { "mn", "mn-Cyrl" },
};

static std::unordered_map<std::string, WeekDay> NAME_TO_WEEK_DAY {
    { "mon", WeekDay::MON },
    { "tue", WeekDay::TUE },
    { "wed", WeekDay::WED },
    { "thu", WeekDay::THU },
    { "fri", WeekDay::FRI },
    { "sat", WeekDay::SAT },
    { "sun", WeekDay::SUN }
};

static std::unordered_map<std::string, TemperatureType> NAME_TO_TEMPERATURE_TYPE {
    { "celsius", TemperatureType::CELSIUS },
    { "fahrenhe", TemperatureType::FAHRENHEIT },
    { "kelvin", TemperatureType::KELVIN }
};

static std::unordered_set<std::string> FAHRENHEIT_USING_REGIONS {
    "BS",
    "BZ",
    "KY",
    "PW",
    "US"
};

std::unordered_map<icu::Calendar::EDaysOfWeek, WeekDay> LocaleConfigExt::eDaysOfWeekToWeekDay {
    { icu::Calendar::EDaysOfWeek::MONDAY, WeekDay::MON },
    { icu::Calendar::EDaysOfWeek::TUESDAY, WeekDay::TUE },
    { icu::Calendar::EDaysOfWeek::WEDNESDAY, WeekDay::WED },
    { icu::Calendar::EDaysOfWeek::THURSDAY, WeekDay::THU },
    { icu::Calendar::EDaysOfWeek::FRIDAY, WeekDay::FRI },
    { icu::Calendar::EDaysOfWeek::SATURDAY, WeekDay::SAT },
    { icu::Calendar::EDaysOfWeek::SUNDAY, WeekDay::SUN }
};

static const std::unordered_map<UMeasurementSystem, std::string> UMEASUREMENT_SYSTEM_TO_VALUE {
    { UMeasurementSystem::UMS_UK, "uksystem"},
    { UMeasurementSystem::UMS_SI, "metric"},
    { UMeasurementSystem::UMS_US, "ussystem"}
};

static std::unordered_map<TemperatureType, std::string> TEMPERATURE_TYPE_TO_NAME {
    { TemperatureType::CELSIUS, "celsius" },
    { TemperatureType::FAHRENHEIT, "fahrenhe" },
    { TemperatureType::KELVIN, "kelvin" }
};

const char NULL_CHAR = '\0';

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

std::unordered_set<std::string> LocaleConfig::GetSystemLanguages()
{
    if (!plugin) {
        return std::unordered_set<std::string>();
    }
    return plugin->GetSystemLanguages();
}

std::vector<std::string> LocaleConfig::GetPreferredLanguages()
{
    if (!plugin) {
        return std::vector<std::string>();
    }
    return plugin->GetPreferredLanguages();
}

std::string LocaleConfig::GetFirstPreferredLanguage()
{
    if (!plugin) {
        return std::string();
    }
    return plugin->GetFirstPreferredLanguage();
}

bool LocaleConfig::GetUsingLocalDigit()
{
    if (!plugin) {
        return false;
    }
    return plugin->GetUsingLocalDigit();
}

std::unordered_set<std::string> LocaleConfig::GetSystemCountries(const std::string& language)
{
    if (!plugin) {
        return std::unordered_set<std::string>();
    }
    return plugin->GetSystemCountries(language);
}

std::unordered_set<std::string> LocaleConfig::GetAvailableIDs()
{
    if (!plugin) {
        return std::unordered_set<std::string>();
    }
    return plugin->GetAvailableIDs();
}

std::string LocaleConfig::GetSimplifiedSystemLanguage()
{
    std::string locale = GetSystemLanguage();
    LocaleInfo localeInfo(locale);
    LocaleInfo localeInfoMax(localeInfo.Maximize());
    std::string language = localeInfoMax.GetLanguage();
    language = language + '-' + localeInfoMax.GetScript();
    if (dialectLanguages.find(language) != dialectLanguages.end()) {
        std::string systemRegion = localeInfo.GetRegion();
        if (!systemRegion.empty() && language.compare("en-Latn") != 0) {
            locale = language + '-' + systemRegion;
        } else {
            locale = GetSystemLocale();
        }
        LocaleInfo* requestLocale = new(std::nothrow) LocaleInfo(locale);
        if (requestLocale == nullptr) {
            LOGE("GetSimplifiedLanguage: %{public}s failed to construct LocaleInfo.", locale.c_str());
            return "";
        }
        std::vector<LocaleInfo*> candidateLocales;
        for (auto& candidate : dialectLanguages[language]) {
            LocaleInfo* candidateInfo = new(std::nothrow) LocaleInfo(candidate);
            if (candidateInfo == nullptr) {
                LOGE("GetSimplifiedLanguage: %{public}s failed to construct LocaleInfo.",
                    candidate.c_str());
                continue;
            }
            candidateLocales.push_back(candidateInfo);
        }
        locale = LocaleMatcher::GetBestMatchedLocale(requestLocale, candidateLocales);

        for (LocaleInfo* candidateInfo : candidateLocales) {
            if (candidateInfo != nullptr) {
                delete candidateInfo;
                candidateInfo = nullptr;
            }
        }
        delete requestLocale;
        requestLocale = nullptr;
    }
    LocaleInfo simplifiedLocale(locale);
    std::string ret = simplifiedLocale.Minimize();
    auto iter = resourceIdMap.find(ret);
    if (iter != resourceIdMap.end()) {
        ret = iter->second;
    }
    return ret;
}

std::string LocaleConfig::GetSimplifiedLanguage(const std::string& languageTag, int32_t& code)
{
    std::string simplifiedLanguage = "";
    if (IsValidTag(languageTag)) {
        LocaleInfo localeInfo(languageTag);
        simplifiedLanguage = localeInfo.Minimize();
    } else {
        code = 1;
    }
    return simplifiedLanguage;
}

WeekDay LocaleConfig::GetFirstDayOfWeek()
{
    std::string localeTag = GetSystemLocale();
    if (localeTag.empty()) {
        LOGE("LocaleConfig::GetFirstDayOfWeek: get system locale failed.");
        return WeekDay::MON;
    }

    UErrorCode status = U_ZERO_ERROR;
    icu::Locale tempLocale = icu::Locale::forLanguageTag(localeTag, status);
    if (U_FAILURE(status)) {
        LOGE("LocaleConfig::GetFirstDayOfWeek: create icu::Locale failed.");
        return WeekDay::MON;
    }

    icu::Calendar* calendar = icu::Calendar::createInstance(tempLocale, status);
    if (U_FAILURE(status)) {
        LOGE("LocaleConfig::GetFirstDayOfWeek: create icu::Calendar failed.");
        return WeekDay::MON;
    }

    icu::Calendar::EDaysOfWeek firstDay = calendar->getFirstDayOfWeek();
    delete calendar;
    return LocaleConfigExt::GetDaysOfWeekMap()[firstDay];
}

std::unordered_map<icu::Calendar::EDaysOfWeek, WeekDay> LocaleConfigExt::GetDaysOfWeekMap()
{
    return eDaysOfWeekToWeekDay;
}

TemperatureType LocaleConfig::GetTemperatureType()
{
    std::string regionTag = GetSystemRegion();
    if (FAHRENHEIT_USING_REGIONS.find(regionTag) != FAHRENHEIT_USING_REGIONS.end()) {
        return TemperatureType::FAHRENHEIT;
    }
    return TemperatureType::CELSIUS;
}

std::string LocaleConfig::GetTemperatureName(const TemperatureType& type)
{
    if (TEMPERATURE_TYPE_TO_NAME.find(type) == TEMPERATURE_TYPE_TO_NAME.end()) {
        LOGE("LocaleConfig::GetTemperatureName: temperature type failed.");
        return "";
    }
    if (type == TemperatureType::FAHRENHEIT) {
        return "fahrenheit";
    }
    return TEMPERATURE_TYPE_TO_NAME[type];
}

bool LocaleConfig::IsSuggested(const std::string &language, const std::string& region)
{
    if (!plugin) {
        return false;
    }
    return plugin->IsSuggested(language, region);
}

std::string LocaleConfig::GetUnicodeWrappedFilePath(const std::string& path, const char delimiter,
    std::shared_ptr<LocaleInfo> localeInfo, std::string& invalidField)
{
    std::string locale;
    if (localeInfo != nullptr) {
        locale = localeInfo->GetBaseName();
    }
    return LocaleConfig::GetUnicodeWrappedFilePathInner(path, delimiter, locale, invalidField);
}

std::string LocaleConfig::GetUnicodeWrappedFilePath(const std::string& path, const char delimiter,
    const std::string& localeTag, std::string& invalidField)
{
    return LocaleConfig::GetUnicodeWrappedFilePathInner(path, delimiter, localeTag, invalidField);
}

std::string LocaleConfig::GetUnicodeWrappedFilePathInner(const std::string& path, const char delimiter,
    const std::string& localeTag, std::string& invalidField)
{
    if (delimiter == NULL_CHAR) {
        invalidField = "delimiter";
        return path;
    }
    if (path.empty() || path.length() > PATH_MAX) {
        invalidField = "path";
        return path;
    }
    std::string locale = localeTag;
    if (!IsValidTag(locale)) {
        locale = GetSystemLocale();
    }
    if (!IsRTL(locale)) {
        return path;
    }
    std::string result;
    std::vector<string> dest;
    std::string sep(1, delimiter);
    Split(path, sep, dest);
    if (dest.size() == 1) {
        return path;
    }
    for (size_t i = 0 ; i < dest.size(); i++) {
        if (dest[i].empty()) {
            if (i == 0) {
                dest[i] = "\u200f";
            }
            continue;
        }
        if (i == 0 && dest[i].compare(sep) != 0) {
            dest[i] = "\u200f\u200e" + dest[i] + "\u200e";
            continue;
        }
        dest[i] = "\u200e" + dest[i] + "\u200e";
    }
    std::string newSep = "\u200f" + sep;
    Merge(dest, newSep, result);
    return result;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS
