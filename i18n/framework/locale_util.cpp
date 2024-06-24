/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "locale_util.h"

namespace OHOS {
namespace Global {
namespace I18n {

uint16_t LocaleUtil::EncodeLanguageByLocaleInfo(const LocaleInfo *locale)
{
    if (locale == nullptr) {
        return NULL_LANGUAGE;
    }
    return EncodeLanguage(locale->GetLanguage().c_str());
}

uint32_t LocaleUtil::EncodeScriptByLocaleInfo(const LocaleInfo *locale)
{
    if (locale == nullptr) {
        return NULL_SCRIPT;
    }
    return EncodeScript(locale->GetScript().c_str());
}

uint16_t LocaleUtil::EncodeRegionByLocaleInfo(const LocaleInfo *locale)
{
    if (locale == nullptr) {
        return NULL_REGION;
    }
    return EncodeRegion(locale->GetRegion().c_str());
}

uint16_t LocaleUtil::EncodeLanguage(const char *language)
{
    if (IsStrEmpty(language)) {
            return NULL_LANGUAGE;
        }
    return EncodeLanguageOrRegion(language, 'a');
}

uint32_t LocaleUtil::EncodeScript(const char *script)
{
    if (IsStrEmpty(script)) {
        return NULL_SCRIPT;
    }
    // 0, 1, 2, 3 is index of characters in script, 24, 16, 8 is offset.
    return ((uint8_t)script[0] << 24) | ((uint8_t)script[1] << 16) | ((uint8_t)script[2] << 8) | (uint8_t)script[3];
}

uint16_t LocaleUtil::EncodeRegion(const char *region)
{
    if (IsStrEmpty(region)) {
        return NULL_REGION;
    }
    if (region[0] >= '0' && region[0] <= '9') {
        return EncodeLanguageOrRegion(region, '0');
    }
    return EncodeLanguageOrRegion(region, 'A');
}

uint64_t LocaleUtil::EncodeLocale(const char *language, const char *script, const char *region)
{
    uint16_t languageData = EncodeLanguage(language);
    uint32_t scriptData = EncodeScript(script);
    uint16_t regionData = EncodeRegion(region);
    // 48 is the offset of language.
    uint32_t languageOffset = 48;
    uint32_t scriptOffset = 16;
    return (uint64_t)(0xffff000000000000 & (((uint64_t)languageData) << languageOffset)) |
        (0x0000ffffffff0000 & (((uint64_t)scriptData) << scriptOffset)) |(0x000000000000ffff & (uint64_t)(regionData));
}

bool LocaleUtil::IsStrEmpty(const char *s)
{
    return (s == nullptr || *s == '\0');
}

void LocaleUtil::DecodeScript(uint32_t encodeScript, char *outValue)
{
    if (outValue == nullptr) {
        return;
    }
    outValue[0] = (encodeScript & 0xFF000000) >> 24; // 0 is index, 24 is first script character offset
    outValue[1] = (encodeScript & 0x00FF0000) >> 16; // 1 is index, 16 is second script character offset
    outValue[2] = (encodeScript & 0x0000FF00) >> 8; // 2 is index, 8 is third script character offset
    outValue[3] = (encodeScript & 0x000000FF); // 3 is index
}

bool LocaleUtil::IsAlphaString(const char *s, int32_t len)
{
    if (s == nullptr) {
        return false;
    }
    int32_t i;
    for (i = 0; i < len; i++) {
        char c = *(s + i);
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
            return false;
        }
    }
    return true;
}

bool LocaleUtil::IsNumericString(const char *s, int32_t len)
{
    if (s == nullptr) {
        return false;
    }
    int32_t i;
    for (i = 0; i < len; i++) {
        char c = *(s + i);
        if (!(c >= '0' && c <= '9')) {
            return false;
        }
    }

    return true;
}

uint16_t LocaleUtil::EncodeLanguageOrRegion(const char *str, char base)
{
    // 2 is index of splitor
    if (str[2] == 0 || str[2] == '-' || str[2] == '_') {
        // 0, 1 is index, 8 is offset
        return ((uint8_t)str[0] << 8) | ((uint8_t)str[1]);
    }
    uint8_t first = ((uint8_t)(str[0] - base)) & 0x7f; // 0 is index
    uint8_t second = ((uint8_t)(str[1] - base)) & 0x7f; // 1 is index
    uint8_t third = ((uint8_t)(str[2] - base)) & 0x7f; // 2 is index
    // 2, 3, 5, 8 is offset.
    return ((0x80 | (first << 2) | (second >> 3)) << 8) | ((second << 5) | third);
}
} // namespace I18n
} // namespace Global
} // namespace OHOS