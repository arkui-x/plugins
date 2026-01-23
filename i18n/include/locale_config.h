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
#ifndef OHOS_GLOBAL_I18N_LOCALE_CONFIG_H
#define OHOS_GLOBAL_I18N_LOCALE_CONFIG_H

#include <string>

#include "i18n_types.h"
#include "locale_info.h"
#include "plugins/i18n/I18N.h"

namespace OHOS {
namespace Global {
namespace I18n {
class LocaleConfig {
public:
    LocaleConfig() = default;
    virtual ~LocaleConfig() = default;
    static std::string GetSystemLocale();
    static std::string GetSystemLanguage();
    static std::string GetSystemRegion();
    static std::string GetSystemTimezone();
    static bool Is24HourClock();
    static bool IsRTL(const std::string &locale);
    static std::string GetDisplayLanguage(const std::string &language, const std::string &displayLocale,
        bool sentenceCase);
    static std::string GetDisplayRegion(const std::string &region, const std::string &displayLocale,
        bool sentenceCase);
    static bool IsValidRegion(const std::string &region);
    static bool IsValidLanguage(const std::string &language);
    static bool IsValidTag(const std::string &tag);
    static std::string GetAppPreferredLanguage();
    static void SetAppPreferredLanguage(const std::string &language);
    static bool IsSuggested(const std::string& language, const std::string& region = std::string(""));
    static std::unordered_set<std::string> GetSystemLanguages();
    static std::vector<std::string> GetPreferredLanguages();
    static std::string GetFirstPreferredLanguage();
    static bool GetUsingLocalDigit();
    static std::unordered_set<std::string> GetSystemCountries(const std::string& language);
    static std::unordered_set<std::string> GetAvailableIDs();
    static std::string GetSimplifiedSystemLanguage();
    static std::string GetSimplifiedLanguage(const std::string& languageTag, int32_t& code);
    static WeekDay GetFirstDayOfWeek();
    static TemperatureType GetTemperatureType();
    static std::string GetTemperatureName(const TemperatureType& type);
    static std::string GetUnicodeWrappedFilePath(const std::string& path, const char delimiter,
        std::shared_ptr<LocaleInfo> localeInfo, std::string& invalidField);
    static std::string GetUnicodeWrappedFilePath(const std::string& path, const char delimiter,
        const std::string& localeTag, std::string& invalidField);

private:
    static std::string GetUnicodeWrappedFilePathInner(const std::string& path, const char delimiter,
        const std::string& localeTag, std::string& invalidField);
    static constexpr uint32_t LANGUAGE_LEN = 2;
    static std::unique_ptr<Plugin::I18N> plugin;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif
