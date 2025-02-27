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
#include "plural_rules.h"

#include <stringpiece.h>

#include "algorithm"
#include "locale_config.h"
#include "locid.h"
#include "log.h"
#include "plural_rules.h"
#include "map"
#include "set"
#include "string"
#include "unicode/unistr.h"
#include "unicode/upluralrules.h"
#include "utility"
#include "utils.h"
#include "utypes.h"
#include "vector"

namespace OHOS {
namespace Global {
namespace I18n {
std::string PluralRules::ParseOption(std::map<std::string, std::string> &options, const std::string &key)
{
    std::map<std::string, std::string>::iterator it = options.find(key);
    if (it != options.end()) {
        return it->second;
    } else {
        return "";
    }
}

int PluralRules::GetValidInteger(std::string &integerStr, int minValue, int maxValue, int defaultValue)
{
    int status = 0;
    int validInteger = ConvertString2Int(integerStr, status);
    if (status < 0) {
        validInteger = defaultValue;
    }
    if (validInteger < minValue) {
        validInteger = minValue;
    }
    if (validInteger > maxValue) {
        validInteger = maxValue;
    }
    return validInteger;
}

void PluralRules::ParseAllOptions(std::map<std::string, std::string> &options)
{
    localeMatcher = ParseOption(options, "localeMatcher");
    localeMatcher = (localeMatcher == "") ? "best fit" : localeMatcher;
    type = ParseOption(options, "type");
    type = (type == "") ? "cardinal" : type;
    std::string minIntegerStr = ParseOption(options, "minimumIntegerDigits");
    // 1 is minValue and defaultValue, 21 is maxValue
    minInteger = GetValidInteger(minIntegerStr, 1, 21, 1);

    minFraction = 0;
    maxFraction = 0;
    std::string minFractionStr = ParseOption(options, "minimumFractionDigits");
    std::string maxFractionStr = ParseOption(options, "maximumFractionDigits");
    std::string minSignificantStr = ParseOption(options, "minimumSignificantDigits");
    std::string maxSignificantStr = ParseOption(options, "maximumSignificantDigits");
    if (minSignificantStr != "" || maxSignificantStr != "") {
        // 1 is minValue and defaultValue, 21 is maxValue
        minSignificant = GetValidInteger(minSignificantStr, 1, 21, 1);
        // 1 is minValue, 21 is maxValue and defaultValue
        maxSignificant = GetValidInteger(maxSignificantStr, 1, 21, 21);
    } else {
        minSignificant = 0;
        maxSignificant = 0;

        if (minFractionStr != "" || maxFractionStr != "") {
            // 0 is minValue and defaultValue, 20 is maxValue
            minFraction = GetValidInteger(minFractionStr, 0, 20, 0);
            int maxFractionDefault = std::max(3, minFraction);  // 3 is the default value of minFraction
            int maxFractionMin = std::max(1, minFraction);  // 1 is the min value of minFraction
            // 21 is max value
            maxFraction = GetValidInteger(maxFractionStr, maxFractionMin, 21, maxFractionDefault);
        } else {
            minFraction = 0;  // 0 is the default value of minFraction.
            maxFraction = 3;  // 3 is the default value of maxFraction
        }
    }
}

void PluralRules::InitPluralRules(std::vector<std::string> &localeTags,
    std::map<std::string, std::string> &options)
{
    UPluralType uPluralType = (type == "cardinal") ? UPLURAL_TYPE_CARDINAL : UPLURAL_TYPE_ORDINAL;
    UErrorCode status = UErrorCode::U_ZERO_ERROR;
    localeTags.push_back(LocaleConfig::GetSystemLocale());
    for (size_t i = 0; i < localeTags.size(); i++) {
        std::string curLocale = localeTags[i];
        locale = icu::Locale::forLanguageTag(icu::StringPiece(curLocale), status);
        if (status != U_ZERO_ERROR) {
            status = U_ZERO_ERROR;
            continue;
        }
        if (LocaleInfo::GetValidLocales().count(locale.getLanguage()) > 0) {
            localeInfo = std::make_unique<LocaleInfo>(curLocale, options);
            if (!localeInfo->InitSuccess()) {
                continue;
            }
            locale = localeInfo->GetLocale();
            localeStr = localeInfo->GetBaseName();
            pluralRules = icu::PluralRules::forLocale(locale, uPluralType, status);
            if (status != UErrorCode::U_ZERO_ERROR || !pluralRules) {
                continue;
            }
            createSuccess = true;
            break;
        }
    }
    if (status != UErrorCode::U_ZERO_ERROR || !pluralRules) {
        LOGE("PluralRules object created failed");
        return;
    }
}

void PluralRules::InitNumberFormatter()
{
    numberFormatter = icu::number::NumberFormatter::withLocale(locale).roundingMode(UNUM_ROUND_HALFUP);
    if (minInteger > 1) {
        numberFormatter = numberFormatter.integerWidth(icu::number::IntegerWidth::zeroFillTo(minInteger));
    }

    if (minSignificant >= 0) {
        if (minSignificant > 0) {
            icu::number::Precision precision = icu::number::Precision::minMaxSignificantDigits(minSignificant,
                maxSignificant);
            numberFormatter = numberFormatter.precision(precision);
        } else {
            icu::number::Precision precision = icu::number::Precision::minMaxFraction(minFraction, maxFraction);
            numberFormatter = numberFormatter.precision(precision);
        }
    }
}

PluralRules::PluralRules(std::vector<std::string> &localeTags, std::map<std::string, std::string> &options)
{
    ParseAllOptions(options);
    InitPluralRules(localeTags, options);
    InitNumberFormatter();
}

PluralRules::~PluralRules()
{
    if (!pluralRules) {
        delete pluralRules;
        pluralRules = nullptr;
    }
}

std::string PluralRules::Select(double number)
{
    if (!createSuccess || pluralRules == nullptr) {
        return "other";
    }
    UErrorCode status = UErrorCode::U_ZERO_ERROR;
    icu::number::FormattedNumber formattedNumber = numberFormatter.formatDouble(number, status);
    if (status != UErrorCode::U_ZERO_ERROR) {
        status = UErrorCode::U_ZERO_ERROR;
        formattedNumber = numberFormatter.formatDouble(number, status);
    }
    icu::UnicodeString unicodeString = pluralRules->select(formattedNumber, status);
    std::string result;
    unicodeString.toUTF8String(result);
    return result;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS