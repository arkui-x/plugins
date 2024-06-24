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

#include "locale_data.h"
#include "locale_matcher.h"
#include "locale_util.h"

namespace OHOS {
namespace Global {
namespace I18n {
uint64_t LocaleMatcher::EN_GB_ENCODE = LocaleUtil::EncodeLocale("en", nullptr, "GB");
uint64_t LocaleMatcher::EN_QAAG_ENCODE = LocaleUtil::EncodeLocale("en", "Qaag", nullptr);
uint64_t LocaleMatcher::ZH_HANT_MO_ENCODE = LocaleUtil::EncodeLocale("zh", "Hant", "MO");
uint64_t LocaleMatcher::ZH_HK_ENCODE = LocaleUtil::EncodeLocale("zh", nullptr, "HK");
uint32_t LocaleMatcher::HANT_ENCODE = LocaleUtil::EncodeScript("Hant");

int8_t LocaleMatcher::IsMoreSuitable(const LocaleInfo *current, const LocaleInfo *other, const LocaleInfo *request)
{
    if (request == nullptr) {
        // if request ResLocale is nullptr, the candidate is also nullptr will be more suitable
        if (current != nullptr && other == nullptr) {
        // -1 means other is more suitable
            return -1;
        }
        if (current == nullptr && other != nullptr) {
            // 1 means current is more suitable
            return 1;
        }
        return 0;
    }
    if (current == nullptr && other == nullptr) {
        return 0;
    }
    bool isLangEqual = CompareLanguage(current, other);
    if (!isLangEqual) {
        // current or other language is null, not null language is better
        bool result = CompareRegionWhenLangIsNotEqual(current, other, request);
        return result ? 1 : -1;
    }
    uint16_t currentEncodedRegion =
        LocaleUtil::EncodeRegionByLocaleInfo(current);
    uint16_t otherEncodedRegion =
        LocaleUtil::EncodeRegionByLocaleInfo(other);
    if (currentEncodedRegion == otherEncodedRegion) {
        // same language,same script,same region
        return CompareLanguageIgnoreOldNewCode(current, other, request);
    }
    // equal request region is better
    uint16_t requestEncodedRegion = LocaleUtil::EncodeRegionByLocaleInfo(request);
    if (currentEncodedRegion == requestEncodedRegion) {
        return 1;
    }
    if (otherEncodedRegion == requestEncodedRegion) {
        return -1;
    }
    int8_t isRegionEqual = CompareRegion(current, other, request);
    if (isRegionEqual == 0) {
        return CompareLanguageIgnoreOldNewCode(current, other, request);
    }
    return isRegionEqual;
}

bool LocaleMatcher::CompareLanguage(const LocaleInfo *current, const LocaleInfo *other)
{
    uint16_t currentEncodedLanguage = LocaleUtil::EncodeLanguageByLocaleInfo(current);
    uint16_t otherEncodedLanguage = LocaleUtil::EncodeLanguageByLocaleInfo(other);
    // 1, 2, 3, 4 is the index.
    return ((currentEncodedLanguage == otherEncodedLanguage) ||
        ((currentEncodedLanguage == LocaleData::NEW_LANGUAGES_CODES[0]) &&
        (otherEncodedLanguage == LocaleData::OLD_LANGUAGES_CODES[0])) ||
        ((otherEncodedLanguage == LocaleData::NEW_LANGUAGES_CODES[0]) &&
        (currentEncodedLanguage == LocaleData::OLD_LANGUAGES_CODES[0])) ||
        ((currentEncodedLanguage == LocaleData::NEW_LANGUAGES_CODES[1]) &&
        (otherEncodedLanguage == LocaleData::OLD_LANGUAGES_CODES[1])) ||
        ((otherEncodedLanguage == LocaleData::NEW_LANGUAGES_CODES[1]) &&
        (currentEncodedLanguage == LocaleData::OLD_LANGUAGES_CODES[1])) ||
        ((currentEncodedLanguage == LocaleData::NEW_LANGUAGES_CODES[2]) && // 2 is index
        (otherEncodedLanguage == LocaleData::OLD_LANGUAGES_CODES[2])) || // 2 is index
        ((otherEncodedLanguage == LocaleData::NEW_LANGUAGES_CODES[2]) && // 2 is index
        (currentEncodedLanguage == LocaleData::OLD_LANGUAGES_CODES[2])) || // 2 is index
        ((currentEncodedLanguage == LocaleData::NEW_LANGUAGES_CODES[3]) && // 3 is index
        (otherEncodedLanguage == LocaleData::OLD_LANGUAGES_CODES[3])) || // 3 is index
        ((otherEncodedLanguage == LocaleData::NEW_LANGUAGES_CODES[3]) && // 3 is index
        (currentEncodedLanguage == LocaleData::OLD_LANGUAGES_CODES[3])) || // 3 is index
        ((currentEncodedLanguage == LocaleData::NEW_LANGUAGES_CODES[4]) && // 4 is index
        (otherEncodedLanguage == LocaleData::OLD_LANGUAGES_CODES[4])) || // 4 is index
        ((otherEncodedLanguage == LocaleData::NEW_LANGUAGES_CODES[4]) && // 4 is index
        (currentEncodedLanguage == LocaleData::OLD_LANGUAGES_CODES[4]))); // 4 is index
}

bool LocaleMatcher::CompareRegionWhenLangIsNotEqual(const LocaleInfo *current, const LocaleInfo *other,
    const LocaleInfo *request)
{
    int8_t qaagResult = CompareRegionWhenQaag(current, other, request);
    if (qaagResult != 0) {
        return qaagResult;
    }
    if (request != nullptr &&
        (LocaleUtil::EncodeLanguage(request->GetLanguage().c_str())) == LocaleUtil::EncodeLanguage("en")) {
        // when request is en-us,empty region is better
        if ((LocaleUtil::EncodeRegion(request->GetRegion().c_str())) == LocaleUtil::EncodeRegion("US")) {
            if (current != nullptr) {
                return (current->GetRegion().length() == 0) ||
                    ((LocaleUtil::EncodeRegion(current->GetRegion().c_str())) == LocaleUtil::EncodeRegion("US"));
            } else {
                return !(other->GetRegion().length() == 0 ||
                    ((LocaleUtil::EncodeRegion(other->GetRegion().c_str())) == LocaleUtil::EncodeRegion("US")));
            }
        } else if (IsSimilarToUsEnglish(request)) {
            if (current != nullptr) {
                return IsSimilarToUsEnglish(current);
            } else {
                return !IsSimilarToUsEnglish(other);
            }
        }
    }
    return current != nullptr;
}

int8_t LocaleMatcher::CompareRegionWhenQaag(const LocaleInfo *current, const LocaleInfo *other,
    const LocaleInfo *request)
{
    if ((request != nullptr) && (LocaleUtil::EncodeLocale(request->GetLanguage().c_str(), request->GetScript().c_str(),
        nullptr) == LocaleMatcher::EN_QAAG_ENCODE)) {
        if ((current != nullptr) && (LocaleUtil::EncodeLocale(current->GetLanguage().c_str(), nullptr,
            current->GetRegion().c_str()) == LocaleMatcher::EN_GB_ENCODE)) {
            return 1;
        }
        if ((other != nullptr) && (LocaleUtil::EncodeLocale(other->GetLanguage().c_str(), nullptr,
            other->GetRegion().c_str()) == LocaleMatcher::EN_GB_ENCODE)) {
            return -1;
        }
    }
    return 0;
}

bool LocaleMatcher::IsSimilarToUsEnglish(const LocaleInfo *localeInfo)
{
    uint64_t localeEncode = LocaleUtil::EncodeLocale("en", nullptr,
        (localeInfo == nullptr) ? nullptr : localeInfo->GetRegion().c_str());
    uint64_t loclaeEncodedTrackPath[LocaleMatcher::TRACKPATH_ARRAY_SIZE] = {0, 0, 0, 0, 0};
    FindTrackPath(nullptr, LocaleMatcher::TRACKPATH_ARRAY_SIZE, localeEncode, loclaeEncodedTrackPath);
    uint8_t len = LocaleMatcher::TRACKPATH_ARRAY_SIZE;
    for (uint8_t i = 0; i < len; ++i) {
        if (loclaeEncodedTrackPath[i] == LocaleUtil::EncodeLocale("en", nullptr, nullptr)) {
            return true;
        }
        if (loclaeEncodedTrackPath[i] == LocaleUtil::EncodeLocale("en", nullptr, "001")) {
            return false;
        }
    }
    return false;
}

void LocaleMatcher::FindTrackPath(const LocaleInfo *request, size_t len, uint64_t encodedLocale, uint64_t *result)
{
    uint64_t currentEncodedLocale = encodedLocale;
    size_t i = 0;
    do {
        result[i] = currentEncodedLocale;
        currentEncodedLocale = SearchParentLocale(currentEncodedLocale, request);
        ++i;
    } while (currentEncodedLocale != LocaleMatcher::ROOT_LOCALE);
    if (i < len) {
        result[i] = LocaleMatcher::ROOT_LOCALE;
    }
}

uint64_t LocaleMatcher::SearchParentLocale(uint64_t encodedLocale, const LocaleInfo *request)
{
    uint64_t tempEncodedLocale = encodedLocale;
    if (LocaleUtil::EncodeScriptByLocaleInfo(request) == LocaleMatcher::HANT_ENCODE) {
        tempEncodedLocale = AddScript(encodedLocale, LocaleMatcher::HANT_ENCODE);
        if (tempEncodedLocale == LocaleMatcher::ZH_HANT_MO_ENCODE) {
            return LocaleMatcher::ZH_HK_ENCODE;
        }
    }
    if (IsContainRegion(encodedLocale)) {
        for (size_t i = 0; i < LocaleData::LOCALE_PARENTS_KEY.size(); i++) {
            if (LocaleData::LOCALE_PARENTS_KEY[i] == tempEncodedLocale) {
                return LocaleData::LOCALE_PARENTS_VALUE[i];
            }
        }
        return ClearRegion(encodedLocale);
    }
    return LocaleMatcher::ROOT_LOCALE;
}

uint64_t LocaleMatcher::AddScript(uint64_t encodedLocale, uint32_t encodedScript)
{
    // 16 is the offset of script
    return (encodedLocale | ((static_cast<uint64_t>(encodedScript) & 0x00000000FFFFFFFFLU) << 16));
}

bool LocaleMatcher::IsContainRegion(uint64_t encodedLocale)
{
    return (encodedLocale & 0x000000000000FFFFLU) != 0;
}

uint64_t LocaleMatcher::ClearRegion(uint64_t encodedLocale)
{
    return encodedLocale & 0xFFFFFFFFFFFF0000LU;
}

int8_t LocaleMatcher::CompareRegion(const LocaleInfo *current, const LocaleInfo *other, const LocaleInfo *request)
{
    uint16_t currentEncodedRegion = LocaleUtil::EncodeRegionByLocaleInfo(current);
    uint16_t otherEncodedRegion = LocaleUtil::EncodeRegionByLocaleInfo(other);
    if (request == nullptr || request->GetRegion().size() == 0) {
        return CompareWhenRegionIsNull(currentEncodedRegion, otherEncodedRegion, current, other, request);
    }
    uint64_t requestEncodedLocale = LocaleUtil::EncodeLocale(request->GetLanguage().c_str(), nullptr,
        request->GetRegion().c_str());
    uint64_t requestEncodedTrackPath[LocaleMatcher::TRACKPATH_ARRAY_SIZE] = {0, 0, 0, 0, 0};
    FindTrackPath(request, LocaleMatcher::TRACKPATH_ARRAY_SIZE, requestEncodedLocale, requestEncodedTrackPath);
    uint64_t currentEncodedLocale = LocaleUtil::EncodeLocale(request->GetLanguage().c_str(), nullptr,
        (current == nullptr) ? nullptr : current->GetRegion().c_str());
    uint64_t otherEncodedLocale = LocaleUtil::EncodeLocale(request->GetLanguage().c_str(), nullptr,
        (other == nullptr) ? nullptr : other->GetRegion().c_str());
    int8_t currentMatchDistance = SearchTrackPathDistance(requestEncodedTrackPath, LocaleMatcher::TRACKPATH_ARRAY_SIZE,
        currentEncodedLocale);
    int8_t otherMatchDistance = SearchTrackPathDistance(requestEncodedTrackPath, LocaleMatcher::TRACKPATH_ARRAY_SIZE,
        otherEncodedLocale);
    if (currentMatchDistance < otherMatchDistance) {
        return 1;
    }
    if (currentMatchDistance > otherMatchDistance) {
        return -1;
    }
    int8_t result = CompareDistance(currentEncodedLocale, otherEncodedLocale, requestEncodedTrackPath, request);
    if (result != 0) {
        return result;
    }
    result = CompareDefaultRegion(current, other, request);
    if (result != 0) {
        return result;
    }
    uint16_t requestDefaultRegion =
        FindDefaultRegionEncode(request->GetLanguage().c_str(), request->GetScript().c_str());
    if (requestDefaultRegion == currentEncodedRegion) {
        return 1;
    }
    if (requestDefaultRegion == otherEncodedRegion) {
        return -1;
    }
    return AlphabeticallyCompare(current, currentEncodedLocale, other, otherEncodedLocale);
}

int8_t LocaleMatcher::CompareWhenRegionIsNull(uint16_t currentEncodedRegion, uint16_t otherEncodedRegion,
    const LocaleInfo *current, const LocaleInfo *other, const LocaleInfo *request)
{
    if (current == nullptr || current->GetRegion().length() == 0) {
        return 1;
    }
    if (other == nullptr || other->GetRegion().length() == 0) {
        return -1;
    }
    int8_t qaagResult = CompareRegionWhenQaag(current, other, request);
    if (qaagResult != 0) {
        return qaagResult;
    }
    // get request default region
    uint16_t requestDefaultRegion =
        FindDefaultRegionEncode((request == nullptr) ? nullptr : request->GetLanguage().c_str(),
            (request == nullptr) ? nullptr : request->GetScript().c_str());
    if (requestDefaultRegion == currentEncodedRegion) {
        return 1;
    }
    if (requestDefaultRegion == otherEncodedRegion) {
        return -1;
    }
    // current and other region is not null.alphabetically
    uint64_t currentEncodedLocale = LocaleUtil::EncodeLocale(
        (request == nullptr) ? nullptr : request->GetLanguage().c_str(), nullptr,
        (current == nullptr) ? nullptr : current->GetRegion().c_str());
    uint64_t otherEncodedLocale = LocaleUtil::EncodeLocale(
        (request == nullptr) ? nullptr : request->GetLanguage().c_str(), nullptr, other->GetRegion().c_str());
    return AlphabeticallyCompare(current, currentEncodedLocale, other, otherEncodedLocale);
}

uint16_t LocaleMatcher::FindDefaultRegionEncode(const char *language, const char *script)
{
    /* first try language and script */
    uint64_t encodedLocale = LocaleUtil::EncodeLocale(language, script, nullptr);
    if (encodedLocale == LocaleUtil::EncodeLocale("en", "Qaag", nullptr)) {
        encodedLocale = LocaleUtil::EncodeLocale("en", "Latn", nullptr);
    }
    for (size_t i = 0; i < LocaleData::LIKELY_TAGS_CODES_KEY.size(); i++) {
        if (LocaleData::LIKELY_TAGS_CODES_KEY[i] == encodedLocale) {
            return static_cast<uint16_t>((LocaleData::LIKELY_TAGS_CODES_VALUE[i] & 0x000000000000ffff));
        }
    }
    /* if not found and script is not null,try language */
    if (script != nullptr) {
        encodedLocale = LocaleUtil::EncodeLocale(language, nullptr, nullptr);
        for (size_t i = 0; i < LocaleData::LIKELY_TAGS_CODES_KEY.size(); i++) {
            if (LocaleData::LIKELY_TAGS_CODES_KEY[i] == encodedLocale) {
                return static_cast<uint16_t>((LocaleData::LIKELY_TAGS_CODES_VALUE[i] & 0x000000000000ffff));
            }
        }
    }
    return LocaleMatcher::NULL_REGION;
}

int8_t LocaleMatcher::AlphabeticallyCompare(const LocaleInfo *current, uint64_t currentEncodedLocale,
    const LocaleInfo *other, uint64_t otherEncodedLocale)
{
    if (currentEncodedLocale == otherEncodedLocale) {
        return 0;
    }
    if (current == nullptr || current->GetRegion().length() == 0) {
        return -1;
    }
    if (other == nullptr || other->GetRegion().length() == 0) {
        return 1;
    }
    // be here region is not null
    char currentFirstChar = (current->GetRegion())[0];
    char otherFirstChar = (other->GetRegion())[0];
    if (currentFirstChar >= '0' && currentFirstChar <= '9') {
        if (otherFirstChar < '0' || otherFirstChar > '9') {
            return -1;
        }
    } else {
        if (otherFirstChar >= '0' && otherFirstChar <= '9') {
            return 1;
        }
    }
    if (currentEncodedLocale > otherEncodedLocale) {
        return -1;
    }
    if (otherEncodedLocale > currentEncodedLocale) {
        return 1;
    }
    return 0;
}

int8_t LocaleMatcher::SearchTrackPathDistance(const uint64_t *paths, size_t len, uint64_t encodedLocale)
{
    size_t i = 0;
    for (i = 0; i < len; ++i) {
        if (paths[i] == LocaleMatcher::ROOT_LOCALE) {
            return i;
        }
        if (paths[i] == encodedLocale) {
            return i;
        }
    }
    return static_cast<int8_t>(i);
}

int8_t LocaleMatcher::CompareDistance(uint64_t currentEncodedLocale, uint64_t otherEncodedLocale,
    const uint64_t *requestEncodedTrackPath, const LocaleInfo *request)
{
    uint64_t currentEncodedTrackPath[LocaleMatcher::TRACKPATH_ARRAY_SIZE] = {0, 0, 0, 0, 0};
    FindTrackPath(request, LocaleMatcher::TRACKPATH_ARRAY_SIZE, currentEncodedLocale, currentEncodedTrackPath);
    uint64_t otherEncodedTrackPath[LocaleMatcher::TRACKPATH_ARRAY_SIZE] = {0, 0, 0, 0, 0};
    FindTrackPath(request, LocaleMatcher::TRACKPATH_ARRAY_SIZE, otherEncodedLocale, otherEncodedTrackPath);
    const size_t currentDistance = ComputeTrackPathDistance(requestEncodedTrackPath, currentEncodedTrackPath,
        LocaleMatcher::TRACKPATH_ARRAY_SIZE);
    const size_t targetDistance = ComputeTrackPathDistance(requestEncodedTrackPath, otherEncodedTrackPath,
        LocaleMatcher::TRACKPATH_ARRAY_SIZE);
    if (currentDistance < targetDistance) {
        return 1;
    }
    if (currentDistance > targetDistance) {
        return -1;
    }
    return 0;
}

size_t LocaleMatcher::ComputeTrackPathDistance(const uint64_t *requestPaths, const uint64_t *targetPaths, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        if (targetPaths[i] == LocaleMatcher::ROOT_LOCALE) {
            // targetpath not in request path,so distance is 2*len
            return len * 2;
        }
        for (size_t j = 0; j < len; ++j) {
            if (requestPaths[j] == targetPaths[i]) {
                return i + j;
            }
        }
    }
    return len * 2; // targetpath not in request path,so distance is 2*len
}

int8_t LocaleMatcher::CompareDefaultRegion(const LocaleInfo *current, const LocaleInfo *other,
    const LocaleInfo *request)
{
    int8_t qaagResult = CompareRegionWhenQaag(current, other, request);
    if (qaagResult != 0) {
        return qaagResult;
    } else {
        bool isCurrentDefaultRegion = IsDefaultLocale((request == nullptr) ? nullptr : request->GetLanguage().c_str(),
            (request == nullptr) ? nullptr : request->GetScript().c_str(),
            (current == nullptr) ? nullptr : current->GetRegion().c_str());
        bool isOtherDefaultRegion = IsDefaultLocale((request == nullptr) ? nullptr : request->GetLanguage().c_str(),
            (request == nullptr) ? nullptr : request->GetScript().c_str(),
            (other == nullptr) ? nullptr : other->GetRegion().c_str());
        if (isCurrentDefaultRegion != isOtherDefaultRegion) {
            if (isCurrentDefaultRegion) {
                return 1;
            } else {
                return -1;
            }
        }
    }
    return 0;
}

bool LocaleMatcher::IsDefaultLocale(const char *language, const char *script, const char *region)
{
    uint64_t encodedLocale = LocaleUtil::EncodeLocale(language, script, region);
    if (ClearRegion(encodedLocale) == LocaleMatcher::EN_QAAG_ENCODE) {
        encodedLocale = LocaleUtil::EncodeLocale("en", "Latn", region);
    }
    for (size_t i = 0; i < LocaleData::TYPICAL_CODES_VALUE.size(); i++) {
        if (LocaleData::TYPICAL_CODES_VALUE[i] == encodedLocale) {
            return true;
        }
    }
    return false;
}

int8_t LocaleMatcher::CompareLanguageIgnoreOldNewCode(const LocaleInfo *current, const LocaleInfo *other,
    const LocaleInfo *request)
{
    uint16_t currentLanguageEncode = LocaleUtil::EncodeLanguageByLocaleInfo(current);
    uint16_t otherLanguageEncode = LocaleUtil::EncodeLanguageByLocaleInfo(other);
    uint16_t requestLanguageEncode = LocaleUtil::EncodeLanguageByLocaleInfo(request);
    if ((currentLanguageEncode == requestLanguageEncode) && (otherLanguageEncode != requestLanguageEncode)) {
        return 1;
    }
    if ((otherLanguageEncode == requestLanguageEncode) && (currentLanguageEncode != requestLanguageEncode)) {
        return -1;
    }
    return 0;
}

bool LocaleMatcher::Match(const LocaleInfo *current, const LocaleInfo *other)
{
    if (current == nullptr || other == nullptr) {
        return true;
    }
    // language is not null.
    bool isLanguageEqual = CompareLanguage(current, other);
    if (!isLanguageEqual) {
        return false;
    }
    return CompareScript(current, other);
}

bool LocaleMatcher::CompareScript(const LocaleInfo *current, const LocaleInfo *other)
{
    uint32_t currentEncodedScript = 0;
    uint32_t otherEncodedScript = 0;
    if ((current != nullptr) && (current->GetScript().length() == 0)) {
        currentEncodedScript = FindDefaultScriptEncode(current->GetLanguage().c_str(), current->GetRegion().c_str());
    } else {
        currentEncodedScript = LocaleUtil::EncodeScriptByLocaleInfo(current);
    }
    if ((other != nullptr) && (other->GetScript().length() == 0)) {
        otherEncodedScript = FindDefaultScriptEncode(other->GetLanguage().c_str(), other->GetRegion().c_str());
    } else {
        otherEncodedScript = LocaleUtil::EncodeScriptByLocaleInfo(other);
    }
    if (current != nullptr && other != nullptr) {
        // when current locale is en-Qaag is equal en-Latn
        if (LocaleUtil::EncodeLocale(current->GetLanguage().c_str(), current->GetScript().c_str(), nullptr) ==
            LocaleUtil::EncodeLocale("en", "Qaag", nullptr)) {
            if (LocaleUtil::EncodeLocale(other->GetLanguage().c_str(), other->GetScript().c_str(), nullptr) ==
                LocaleUtil::EncodeLocale("en", "Latn", nullptr)) {
                return true;
            }
        }
    }
    bool compareRegion = false;
    if ((currentEncodedScript == LocaleMatcher::NULL_SCRIPT) || (otherEncodedScript == LocaleMatcher::NULL_SCRIPT)) {
        // if request script is null, region must be same
        compareRegion = true;
    }
    if (compareRegion) {
        uint16_t currentRegionEncode = LocaleUtil::EncodeRegionByLocaleInfo(current);
        uint16_t otherRegionEncode = LocaleUtil::EncodeRegionByLocaleInfo(other);
        return (otherRegionEncode == LocaleMatcher::NULL_REGION) || (currentRegionEncode == otherRegionEncode);
    }
    return currentEncodedScript == otherEncodedScript;
}

uint32_t LocaleMatcher::FindDefaultScriptEncode(const char *language, const char *region)
{
    uint64_t encodedLocale = LocaleUtil::EncodeLocale(language, nullptr, region);
    for (size_t i = 0; i < LocaleData::LIKELY_TAGS_CODES_KEY.size(); i++) {
        if (LocaleData::LIKELY_TAGS_CODES_KEY[i] == encodedLocale) {
            // 16 is the offset of script
            return static_cast<uint32_t>((LocaleData::LIKELY_TAGS_CODES_VALUE[i] & 0x0000ffffffff0000) >> 16);
        }
    }
    if (region != nullptr) {
        encodedLocale = LocaleUtil::EncodeLocale(language, nullptr, nullptr);
        for (size_t i = 0; i < LocaleData::LIKELY_TAGS_CODES_KEY.size(); i++) {
            if (LocaleData::LIKELY_TAGS_CODES_KEY[i] == encodedLocale) {
                // 16 is the offset of script
                return static_cast<uint32_t>((LocaleData::LIKELY_TAGS_CODES_VALUE[i] & 0x0000ffffffff0000) >> 16);
            }
        }
    }
    return LocaleMatcher::NULL_SCRIPT;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS