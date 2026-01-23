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
#ifndef OHOS_GLOBAL_PHONE_NUMBER_MATCHED_H
#define OHOS_GLOBAL_PHONE_NUMBER_MATCHED_H

#include <string>
#include <unicode/regex.h>
#include <unordered_set>
#include <vector>
#include "matched_number_info.h"
#include "phone_number_rule.h"
#include "phonenumbers/phonenumbermatch.h"
#include "phonenumbers/phonenumberutil.h"
#include "phonenumbers/shortnumberinfo.h"

namespace OHOS {
namespace Global {
namespace I18n {
using i18n::phonenumbers::PhoneNumberUtil;
using i18n::phonenumbers::PhoneNumberMatch;
using i18n::phonenumbers::ShortNumberInfo;
class PhoneNumberMatched {
public:
    PhoneNumberMatched(std::string& country);
    ~PhoneNumberMatched();
    std::vector<int> GetMatchedPhoneNumber(icu::UnicodeString& message);
    static const int CONTAIN;
    static const int CONTAIN_OR_INTERSECT;

private:
    bool AddPhoneNumber(std::string& number, int start, std::vector<PhoneNumberMatch*>& matchList,
        std::string& country);
    std::vector<MatchedNumberInfo> DeleteRepeatedInfo(std::vector<MatchedNumberInfo>& list);
    std::vector<int> DealResult(std::vector<MatchedNumberInfo>& matchedNumberInfoList);
    std::vector<int> DealWithoutFixed(icu::UnicodeString& message, std::string& country);
    bool HandleWithShortAndShort(std::vector<MatchedNumberInfo>& result, std::string& country,
        MatchedNumberInfo& info, std::pair<int, int>& pos, icu::UnicodeString& filteredString);
    std::vector<MatchedNumberInfo> GetPossibleNumberInfos(std::string& country,
        icu::UnicodeString& src, icu::UnicodeString& filteredString);
    std::vector<PhoneNumberMatch*> FindNumbers(std::string& country, icu::UnicodeString& filteredString);
    std::vector<MatchedNumberInfo> FindShortNumbers(std::string& country, icu::UnicodeString& message);
    PhoneNumberMatch* HandleCodesRule(PhoneNumberMatch* phoneNumberMatch, icu::UnicodeString& message);
    bool IsNumberWithOneBracket(icu::UnicodeString& message);
    void DealNumberWithOneBracket(MatchedNumberInfo& info);
    icu::UnicodeString DealStringWithOneBracket(icu::UnicodeString& message);
    icu::UnicodeString HandleNegativeRule(icu::UnicodeString& src);
    std::vector<MatchedNumberInfo> HandlePositiveRule(PhoneNumberMatch* match, icu::UnicodeString& message);
    bool HandleBorderRule(PhoneNumberMatch* match, icu::UnicodeString& message);
    void ReplaceSpecifiedPos(icu::UnicodeString& chs, int start, int end);
    void FindNumbersFromInputString(std::vector<PhoneNumberMatch*>& matchList,
        icu::RegexMatcher* numberMatcher, std::string& country, icu::UnicodeString& filteredString);
    std::vector<FindRule*> GetFindRulesInside();
    std::vector<BorderRule*> GetBorderRulesInside();
    std::vector<CodeRule*> GetCodesRulesInside();
    std::vector<PositiveRule*> GetPositiveRulesInside();

    static const UChar32 REPLACE_CHAR;
    PhoneNumberRule* phoneNumberRule;
    PhoneNumberUtil* phoneNumberUtil;
    ShortNumberInfo* shortNumberInfo;
    std::string country;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif