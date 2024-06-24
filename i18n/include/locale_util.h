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
#ifndef PLUGINS_I18N_LOCALE_UTIL_H
#define PLUGINS_I18N_LOCALE_UTIL_H

#include <string>
#include "locale_info.h"

namespace OHOS {
namespace Global {
namespace I18n {
class LocaleUtil {
    public:
    static uint16_t EncodeLanguageByLocaleInfo(const LocaleInfo *locale);
    static uint32_t EncodeScriptByLocaleInfo(const LocaleInfo *locale);
    static uint16_t EncodeRegionByLocaleInfo(const LocaleInfo *locale);
    static uint16_t EncodeLanguage(const char *language);
    static uint32_t EncodeScript(const char *script);
    static uint16_t EncodeRegion(const char *region);
    static uint64_t EncodeLocale(const char *language, const char *script, const char *region);
    static bool IsStrEmpty(const char *s);
    static void DecodeScript(uint32_t encodeScript, char *outValue);
    static bool IsAlphaString(const char *s, int32_t len);
    static bool IsNumericString(const char *s, int32_t len);

    private:
    static uint16_t EncodeLanguageOrRegion(const char *str, char base);
    static constexpr uint16_t NULL_LANGUAGE = 0x00;
    static constexpr uint16_t NULL_REGION = 0x00;
    static constexpr uint16_t NULL_SCRIPT = 0x0000;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif // PLUGINS_I18N_LOCALE_UTIL_H