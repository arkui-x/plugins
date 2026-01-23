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
#ifndef OHOS_GLOBAL_I18N_POSITIVE_RULE_H
#define OHOS_GLOBAL_I18N_POSITIVE_RULE_H

#include <unicode/regex.h>
#include "matched_number_info.h"
#include "phonenumbers/phonenumbermatch.h"

namespace OHOS {
namespace Global {
namespace I18n {
using i18n::phonenumbers::PhoneNumberMatch;
class PositiveRule {
public:
    PositiveRule(icu::UnicodeString& regex, std::string& HandleType, std::string& insensitive);
    icu::RegexPattern* GetPattern();
    std::vector<MatchedNumberInfo> HandleInner(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    std::vector<MatchedNumberInfo> Handle(PhoneNumberMatch* match, icu::UnicodeString& message);

private:
    static std::vector<MatchedNumberInfo> GetNumbersWithSlant(icu::UnicodeString& testStr);
    std::vector<MatchedNumberInfo> HandlePossibleNumberWithPattern(PhoneNumberMatch* possibleNumber,
        icu::UnicodeString& message, bool isStartsWithNumber);
    
    std::vector<MatchedNumberInfo> HandleDefault(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    std::vector<MatchedNumberInfo> HandleOperator(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    std::vector<MatchedNumberInfo> HandleBlank(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    std::vector<MatchedNumberInfo> HandleSlant(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    std::vector<MatchedNumberInfo> HandleStartWithMobile(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    std::vector<MatchedNumberInfo> HandleEndWithMobile(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);

    icu::UnicodeString DealStringWithOneBracket(icu::UnicodeString& message);
    bool IsNumberWithOneBracket(icu::UnicodeString& message);
    icu::UnicodeString regex;
    std::string handleType;
    std::string insensitive;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif