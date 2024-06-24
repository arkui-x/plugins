/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef PLUGINS_I18N_LOCALE_MATCHER_H
#define PLUGINS_I18N_LOCALE_MATCHER_H

#include "locale_info.h"

namespace OHOS {
namespace Global {
namespace I18n {
class LocaleMatcher {
public:
    static int8_t IsMoreSuitable(const LocaleInfo *current, const LocaleInfo *other, const LocaleInfo *request);
    static bool Match(const LocaleInfo *current, const LocaleInfo *other);

public:
    static uint64_t EN_GB_ENCODE;
    static uint64_t EN_QAAG_ENCODE;
    static uint64_t ZH_HANT_MO_ENCODE;
    static uint64_t ZH_HK_ENCODE;
    static uint32_t HANT_ENCODE;
    static constexpr uint64_t ROOT_LOCALE = 0x0;
    static constexpr uint16_t NULL_LANGUAGE = 0x0;
    static constexpr uint16_t NULL_REGION = 0x0;
    static constexpr uint16_t NULL_SCRIPT = 0x0;
    static constexpr uint64_t NULL_LOCALE = 0x0;
    static constexpr uint8_t TRACKPATH_ARRAY_SIZE = 5;

private:
    static bool CompareLanguage(const LocaleInfo *current, const LocaleInfo *other);
    static bool CompareRegionWhenLangIsNotEqual(const LocaleInfo *current, const LocaleInfo *other,
        const LocaleInfo *request);
    static int8_t CompareRegionWhenQaag(const LocaleInfo *current, const LocaleInfo *other, const LocaleInfo *request);
    static bool IsSimilarToUsEnglish(const LocaleInfo *localeInfo);

    static void FindTrackPath(const LocaleInfo *request, size_t len, uint64_t encodedLocale, uint64_t *result);

    static uint64_t SearchParentLocale(uint64_t encodedLocale, const LocaleInfo *request);
    static uint64_t AddScript(uint64_t encodedLocale, uint32_t encodedScript);
    static bool IsContainRegion(uint64_t encodedLocale);
    static uint64_t ClearRegion(uint64_t encodedLocale);

    static int8_t CompareRegion(const LocaleInfo *current, const LocaleInfo *other, const LocaleInfo *request);
    static int8_t CompareWhenRegionIsNull(uint16_t currentEncodedRegion, uint16_t otherEncodedRegion,
        const LocaleInfo *current, const LocaleInfo *other, const LocaleInfo *request);

    static uint16_t FindDefaultRegionEncode(const char *language, const char *script);
    static int8_t AlphabeticallyCompare(const LocaleInfo *current, uint64_t currentEncodedLocale,
        const LocaleInfo *other, uint64_t otherEncodedLocale);

    static int8_t SearchTrackPathDistance(const uint64_t *paths, size_t len, uint64_t encodedLocale);
    static int8_t CompareDistance(uint64_t currentEncodedLocale, uint64_t otherEncodedLocale,
        const uint64_t *requestEncodedTrackPath, const LocaleInfo *request);

    static size_t ComputeTrackPathDistance(const uint64_t *requestPaths, const uint64_t *targetPaths, size_t len);
    static int8_t CompareDefaultRegion(const LocaleInfo *current, const LocaleInfo *other, const LocaleInfo *request);

    static bool IsDefaultLocale(const char *language, const char *script, const char *region);
    static int8_t CompareLanguageIgnoreOldNewCode(const LocaleInfo *current, const LocaleInfo *other,
        const LocaleInfo *request);
    static bool CompareScript(const LocaleInfo *current, const LocaleInfo *other);
    static uint32_t FindDefaultScriptEncode(const char *language, const char *region);
    static const constexpr uint8_t SCRIPT_OFFSET = 16;
};
} // namespace OHOS
} // namespace Global
} // namespace I18n
#endif // PLUGINS_I18N_LOCALE_MATCHER_H