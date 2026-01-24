/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#ifndef OHOS_GLOBAL_REGEX_RULE_H
#define OHOS_GLOBAL_REGEX_RULE_H

#include <string>
#include <unicode/regex.h>
#include <vector>
#include "matched_number_info.h"
#include "phonenumbers/phonenumbermatch.h"

namespace OHOS {
namespace Global {
namespace I18n {
using i18n::phonenumbers::PhoneNumberMatch;
class RegexRule {
public:
    RegexRule(icu::UnicodeString& regex, std::string& IsValidType, std::string& HandleType,
        std::string& insensitive, std::string& type);
    ~RegexRule();

    int GetType();
    icu::RegexPattern* GetPattern();

    std::vector<MatchedNumberInfo> Handle(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    PhoneNumberMatch* IsValid(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    static int CountDigits(icu::UnicodeString& str);

private:
    void StartWithBrackets(icu::UnicodeString& phone);
    static std::vector<MatchedNumberInfo> GetNumbersWithSlant(icu::UnicodeString& testStr);
    std::vector<MatchedNumberInfo> HandlePossibleNumberWithPattern(PhoneNumberMatch* possibleNumber,
        icu::UnicodeString& message, bool isStartsWithNumber);
    bool NumberValid(icu::UnicodeString& number);
    bool PrefixValid(icu::UnicodeString& number, int length);

    PhoneNumberMatch* IsValidEnd(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    PhoneNumberMatch* IsValidStart(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    PhoneNumberMatch* IsValidDefault(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    PhoneNumberMatch* IsValidPreSuf(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    PhoneNumberMatch* IsValidCode(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    PhoneNumberMatch* IsValidRawstr(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);

    std::vector<MatchedNumberInfo> HandleDefault(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    std::vector<MatchedNumberInfo> HandleOperator(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    std::vector<MatchedNumberInfo> HandleBlank(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    std::vector<MatchedNumberInfo> HandleSlant(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    std::vector<MatchedNumberInfo> HandleStartWithMobile(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    std::vector<MatchedNumberInfo> HandleEndWithMobile(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);

    int type;
    icu::UnicodeString regex;
    std::string isValidType;
    std::string handleType;
    std::string insensitive;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif