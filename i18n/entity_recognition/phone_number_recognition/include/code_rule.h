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
#ifndef OHOS_GLOBAL_I18N_CODE_RULE_H
#define OHOS_GLOBAL_I18N_CODE_RULE_H

#include <unicode/regex.h>
#include "matched_number_info.h"
#include "phonenumbers/phonenumbermatch.h"

namespace OHOS {
namespace Global {
namespace I18n {
using i18n::phonenumbers::PhoneNumberMatch;
class CodeRule {
public:
    CodeRule(std::string& IsValidType);
    PhoneNumberMatch* IsValid(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    PhoneNumberMatch* Handle(PhoneNumberMatch* phoneNumberMatch, icu::UnicodeString& message);

private:
    static int CountDigits(icu::UnicodeString& str);
    void StartWithBrackets(icu::UnicodeString& phone);
    bool NumberValid(icu::UnicodeString& number);
    bool PrefixValid(icu::UnicodeString& number, int length);
    PhoneNumberMatch* IsValidEnd(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    PhoneNumberMatch* IsValidStart(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    PhoneNumberMatch* IsValidDefault(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    PhoneNumberMatch* IsValidPreSuf(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    PhoneNumberMatch* IsValidCode(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    PhoneNumberMatch* IsValidRawstr(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message);
    std::string isValidType;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif