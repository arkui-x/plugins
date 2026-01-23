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
#include "phone_number_matched.h"

#include <climits>
#include <set>
#include "log.h"
#include "regex_rule.h"
#include "utils.h"

namespace OHOS {
namespace Global {
namespace I18n {
using i18n::phonenumbers::PhoneNumber;

const int PhoneNumberMatched::CONTAIN = 9;
const int PhoneNumberMatched::CONTAIN_OR_INTERSECT = 8;
const int PHONE_NUMBER_SIZE = 5;
const size_t PENULTIMATE_POSITION_OFFSET = 2;
const size_t RULE_LENGTH = 3;
const size_t POS_SIZE_NUMBER = 2;
const size_t POS_SIZE_END = 2;
const UChar32 PhoneNumberMatched::REPLACE_CHAR = 'A';

PhoneNumberMatched::PhoneNumberMatched(std::string& country)
{
    phoneNumberRule = new (std::nothrow) PhoneNumberRule(country);
    phoneNumberUtil = PhoneNumberUtil::GetInstance();
    shortNumberInfo = new (std::nothrow) ShortNumberInfo();
    this->country = country;
    if (phoneNumberRule != nullptr) {
        phoneNumberRule->Init();
    }
}

PhoneNumberMatched::~PhoneNumberMatched()
{
    if (phoneNumberRule != nullptr) {
        delete phoneNumberRule;
    }
    if (shortNumberInfo != nullptr) {
        delete shortNumberInfo;
    }
}

std::vector<int> PhoneNumberMatched::GetMatchedPhoneNumber(icu::UnicodeString& message)
{
    icu::UnicodeString messageStr = message;
    if (phoneNumberRule != nullptr && !phoneNumberRule->isFixed) {
        return DealWithoutFixed(messageStr, country);
    }
    icu::UnicodeString filteredString = HandleNegativeRule(messageStr);
    std::vector<MatchedNumberInfo> matchedNumberInfoList = GetPossibleNumberInfos(country,
        messageStr, filteredString);
    std::vector<MatchedNumberInfo> shortList = FindShortNumbers(country, filteredString);
    if (shortList.size() != 0) {
        matchedNumberInfoList.insert(matchedNumberInfoList.end(), shortList.begin(), shortList.end());
    }
    matchedNumberInfoList = DeleteRepeatedInfo(matchedNumberInfoList);
    for (auto& matchedNumberInfo : matchedNumberInfoList) {
        DealNumberWithOneBracket(matchedNumberInfo);
    }
    return DealResult(matchedNumberInfoList);
}

std::vector<int> PhoneNumberMatched::DealWithoutFixed(icu::UnicodeString& message, std::string& country)
{
    std::vector<PhoneNumberMatch*> matchList = FindNumbers(country, message);
    std::vector<MatchedNumberInfo> result;
    for (auto& match : matchList) {
        if (match == nullptr) {
            continue;
        }
        MatchedNumberInfo info;
        icu::UnicodeString content = match->raw_string().c_str();
        PhoneNumber phoneNumber = match->number();
        if (phoneNumberUtil->IsValidNumber(phoneNumber)) {
            info.SetBegin(match->start());
            info.SetEnd(match->end());
            info.SetContent(content);
            result.push_back(info);
        }
        delete match;
    }
    std::vector<MatchedNumberInfo> shortResult = FindShortNumbers(country, message);
    if (shortResult.size() != 0) {
        result.insert(result.end(), shortResult.begin(), shortResult.end());
    }
    result = DeleteRepeatedInfo(result);
    for (auto& res: result) {
        DealNumberWithOneBracket(res);
    }
    return DealResult(result);
}

icu::UnicodeString PhoneNumberMatched::HandleNegativeRule(icu::UnicodeString& src)
{
    std::vector<NegativeRule*> rules = phoneNumberRule->GetNegativeRules();
    icu::UnicodeString ret = src;
    for (NegativeRule* rule : rules) {
        if (rule == nullptr) {
            continue;
        }
        ret = rule->Handle(ret);
    }
    return ret;
}

void PhoneNumberMatched::ReplaceSpecifiedPos(icu::UnicodeString& chs, int start, int end)
{
    if (start >= end) {
        return;
    }
    int len = chs.length();
    for (int i = 0; i < len; i++) {
        if (i >= start && i < end) {
            chs.replace(i, 1, PhoneNumberMatched::REPLACE_CHAR);
        }
    }
}

std::vector<MatchedNumberInfo> PhoneNumberMatched::FindShortNumbers(std::string& country,
    icu::UnicodeString& message)
{
    std::vector<MatchedNumberInfo> matchedNumberInfoList;
    UErrorCode status = U_ZERO_ERROR;
    size_t pos = GetFindRulesInside().size();
    if (pos == 0) {
        LOGE("PhoneNumberRule.findRules is empty.");
        return matchedNumberInfoList;
    }
    FindRule* shortRegexRule = GetFindRulesInside()[pos - 1];
    icu::RegexPattern* shortPattern = shortRegexRule->GetPattern();
    if (shortPattern == nullptr) {
        LOGE("shortPattern getPattern failed.");
        return matchedNumberInfoList;
    }
    icu::RegexMatcher* shortMatch = shortPattern->matcher(message, status);
    if (U_FAILURE(status) || shortMatch == nullptr) {
        delete shortPattern;
        LOGE("shortPattern matcher failed.");
        return matchedNumberInfoList;
    }
    while (shortMatch->find(status)) {
        icu::UnicodeString numberToParse = shortMatch->group(status);
        std::string stringParse;
        numberToParse.toUTF8String(stringParse);
        PhoneNumber phoneNumber;
        PhoneNumberUtil::ErrorType errorType =
            phoneNumberUtil->ParseAndKeepRawInput(stringParse, country, &phoneNumber);
        if (errorType != PhoneNumberUtil::NO_PARSING_ERROR) {
            LOGE("PhoneNumberRule: failed to call the ParseAndKeepRawInput.");
            continue;
        }
        if (shortNumberInfo != nullptr &&
                shortNumberInfo->IsPossibleShortNumberForRegion(phoneNumber, country)) {
            MatchedNumberInfo matcher;
            matcher.SetBegin(shortMatch->start(status));
            matcher.SetEnd(shortMatch->end(status));
            icu::UnicodeString stringShort = shortMatch->group(status);
            matcher.SetContent(stringShort);
            matchedNumberInfoList.push_back(matcher);
        }
    }
    delete shortMatch;
    delete shortPattern;
    return matchedNumberInfoList;
}

std::vector<FindRule*> PhoneNumberMatched::GetFindRulesInside()
{
    if (phoneNumberRule != nullptr) {
        return phoneNumberRule->GetFindRules();
    }
    return {};
}

std::vector<BorderRule*> PhoneNumberMatched::GetBorderRulesInside()
{
    if (phoneNumberRule != nullptr) {
        return phoneNumberRule->GetBorderRules();
    }
    return {};
}

std::vector<CodeRule*> PhoneNumberMatched::GetCodesRulesInside()
{
    if (phoneNumberRule != nullptr) {
        return phoneNumberRule->GetCodesRules();
    }
    return {};
}

std::vector<PositiveRule*> PhoneNumberMatched::GetPositiveRulesInside()
{
    if (phoneNumberRule != nullptr) {
        return phoneNumberRule->GetPositiveRules();
    }
    return {};
}

bool PhoneNumberMatched::AddPhoneNumber(std::string& number, int start, std::vector<PhoneNumberMatch*>& matchList,
    std::string& country)
{
    PhoneNumber phoneNumber;
    icu::UnicodeString uNumber = number.c_str();
    if (RegexRule::CountDigits(uNumber) < PHONE_NUMBER_SIZE) {
        return false;
    }
    PhoneNumberUtil::ErrorType parseStatus = phoneNumberUtil->Parse(number, country, &phoneNumber);
    if (parseStatus != PhoneNumberUtil::NO_PARSING_ERROR) {
        return false;
    }
    UChar32 space = ' ';
    UChar32 slash = '/';
    if ((uNumber.indexOf(space) == -1 && uNumber.indexOf(slash) == -1) ||
        phoneNumberUtil->IsValidNumber(phoneNumber)) {
        PhoneNumberMatch* match = new PhoneNumberMatch(start, number, phoneNumber);
        matchList.push_back(match);
        return true;
    }
    return false;
}

std::vector<PhoneNumberMatch*> PhoneNumberMatched::FindNumbers(std::string& country,
    icu::UnicodeString& filteredString)
{
    std::vector<PhoneNumberMatch*> matchList;
    size_t pos = GetFindRulesInside().size();
    if (pos < PENULTIMATE_POSITION_OFFSET) {
        LOGE("numberRegexRule size less then 2.");
        return matchList;
    }
    FindRule* numberRegexRule = GetFindRulesInside()[pos - PENULTIMATE_POSITION_OFFSET];
    icu::RegexPattern* numberPattern = numberRegexRule->GetPattern();
    if (numberPattern == nullptr) {
        LOGE("numberRegexRule getPattern failed.");
        return matchList;
    }
    UErrorCode status = U_ZERO_ERROR;
    icu::RegexMatcher* numberMatcher = numberPattern->matcher(filteredString, status);
    if (numberMatcher == nullptr) {
        LOGE("numberPattern matcher failed.");
        delete numberPattern;
        return matchList;
    }
    FindNumbersFromInputString(matchList, numberMatcher, country, filteredString);
    delete numberMatcher;
    delete numberPattern;
    return matchList;
}

void PhoneNumberMatched::FindNumbersFromInputString(std::vector<PhoneNumberMatch*> &matchList,
    icu::RegexMatcher* numberMatcher, std::string& country, icu::UnicodeString& filteredString)
{
    UErrorCode status = U_ZERO_ERROR;
    while (numberMatcher->find(status)) {
        int32_t start = numberMatcher->start(status);
        int32_t end = numberMatcher->end(status);
        icu::UnicodeString uNumber = filteredString.tempSubString(start, end - start);
        std::string number;
        uNumber.toUTF8String(number);
        if (!AddPhoneNumber(number, start, matchList, country)) {
            int searchStart = 0;
            UChar32 space = ' ';
            UChar32 slash = '/';
            if (uNumber.indexOf(space, searchStart) == -1 && uNumber.indexOf(slash, searchStart) == -1) {
                continue;
            }
            while (uNumber.indexOf(space, searchStart) != -1 || uNumber.indexOf(slash, searchStart) != -1) {
                int phoneStart = searchStart;
                int indexSpace = uNumber.indexOf(space, searchStart);
                int indexSlash = uNumber.indexOf(slash, searchStart);
                int phoneEnd =
                    (indexSpace == -1 || (indexSlash != -1 && indexSlash < indexSpace)) ? indexSlash : indexSpace;
                searchStart = phoneEnd + 1;
                std::string tempNumber = number.substr(phoneStart, phoneEnd - phoneStart);
                AddPhoneNumber(tempNumber, phoneStart + start, matchList, country);
            }
            std::string lastStr = number.substr(searchStart);
            AddPhoneNumber(lastStr, searchStart + start, matchList, country);
        }
    }
}

bool PhoneNumberMatched::HandleWithShortAndShort(std::vector<MatchedNumberInfo>& result, std::string& country,
    MatchedNumberInfo& info, std::pair<int, int>& pos, icu::UnicodeString& filteredString)
{
    bool flag = false;
    if (GetFindRulesInside().size() == RULE_LENGTH) {
        FindRule* regexRule = GetFindRulesInside()[0];
        if (regexRule == nullptr) {
            LOGE("PhoneNumberMatched::HandleWithShortAndShort: Get FindRule failed.");
            return false;
        }
        icu::RegexPattern* pattern = regexRule->GetPattern();
        if (pattern == nullptr) {
            LOGE("regexRule getPattern failed.");
            return flag;
        }
        icu::UnicodeString str = "";
        if (info.GetContent()[0] == '(' || info.GetContent()[0] == '[') {
            str = info.GetContent().tempSubString(1);
        } else {
            str = info.GetContent();
        }
        UErrorCode status = U_ZERO_ERROR;
        icu::RegexMatcher* matcher = pattern->matcher(str, status);
        if (U_FAILURE(status) || matcher == nullptr) {
            delete pattern;
            LOGE("pattern matcher failed.");
            return flag;
        }
        if (!matcher->find(status) && U_SUCCESS(status)) {
            result.push_back(info);
            ReplaceSpecifiedPos(filteredString, pos.first, pos.second);
            flag = true;
        }
        delete matcher;
        delete pattern;
    } else {
        result.push_back(info);
        ReplaceSpecifiedPos(filteredString, pos.first, pos.second);
    }
    return flag;
}

std::vector<MatchedNumberInfo> PhoneNumberMatched::GetPossibleNumberInfos(std::string& country,
    icu::UnicodeString& src, icu::UnicodeString& filteredString)
{
    std::vector<MatchedNumberInfo> result;
    std::vector<PhoneNumberMatch*> matchList = FindNumbers(country, filteredString);
    for (auto match : matchList) {
        if (!HandleBorderRule(match, filteredString)) {
            continue;
        }
        PhoneNumberMatch* delMatch = HandleCodesRule(match, src);
        if (delMatch == nullptr) {
            continue;
        }
        icu::UnicodeString content = delMatch->raw_string().c_str();
        int contentStart = delMatch->start();
        int contentEnd = delMatch->end();
        std::pair<int, int> pos{contentStart, contentEnd};
        if (phoneNumberUtil->IsValidNumber(delMatch->number())) {
            MatchedNumberInfo info;
            info.SetBegin(contentStart);
            info.SetEnd(contentEnd);
            info.SetContent(content);
            bool flag = HandleWithShortAndShort(result, country, info, pos, filteredString);
            if (flag) {
                continue;
            }
        }
        std::vector<MatchedNumberInfo> posList = HandlePositiveRule(delMatch, filteredString);
        if (posList.size() != 0) {
            for (auto& matchInfo : posList) {
                ReplaceSpecifiedPos(filteredString, matchInfo.GetBegin(), matchInfo.GetEnd());
            }
            result.insert(result.end(), posList.begin(), posList.end());
        }
        delete match;
    }
    return result;
}

std::vector<MatchedNumberInfo> PhoneNumberMatched::DeleteRepeatedInfo(std::vector<MatchedNumberInfo>& list)
{
    std::set<MatchedNumberInfo> set;
    std::vector<MatchedNumberInfo> ret;
    for (auto info : list) {
        if (set.find(info) == set.end()) {
            ret.push_back(info);
        }
        set.insert(info);
    }
    return ret;
}

void PhoneNumberMatched::DealNumberWithOneBracket(MatchedNumberInfo& info)
{
    icu::UnicodeString message = info.GetContent();
    if (IsNumberWithOneBracket(message)) {
        info.SetBegin(info.GetBegin() + 1);
        icu::UnicodeString content = info.GetContent().tempSubString(1);
        info.SetContent(content);
    }
}

bool PhoneNumberMatched::IsNumberWithOneBracket(icu::UnicodeString& message)
{
    if (message != "") {
        int numLeft = 0;
        int numRight = 0;
        int len = message.length();
        for (int i = 0; i < len; i++) {
            if (message[i] == '(' || message[i] == '[') {
                numLeft++;
            }
            if (message[i] == ')' || message[i] == ']') {
                numRight++;
            }
        }
        if (numLeft > numRight && (message[0] == '(' || message[0] == '[')) {
            return true;
        }
    }
    return false;
}

std::vector<int> PhoneNumberMatched::DealResult(std::vector<MatchedNumberInfo>& matchedNumberInfoList)
{
    std::vector<int> result;
    size_t length = matchedNumberInfoList.size();
    if (length == 0) {
        result.push_back(0);
    } else {
        result.resize(POS_SIZE_NUMBER * length + 1);
        result[0] = static_cast<int>(length);
        for (size_t i = 0; i < length; i++) {
            result[POS_SIZE_NUMBER * i + 1] = matchedNumberInfoList[i].GetBegin();
            result[POS_SIZE_NUMBER * i + POS_SIZE_END] = matchedNumberInfoList[i].GetEnd();
        }
    }
    return result;
}

bool PhoneNumberMatched::HandleBorderRule(PhoneNumberMatch* match, icu::UnicodeString& message)
{
    if (match == nullptr) {
        return false;
    }
    std::vector<BorderRule*> rules = GetBorderRulesInside();
    if (rules.size() == 0) {
        return true;
    }
    for (BorderRule* rule : rules) {
        if (rule == nullptr) {
            return false;
        }
        if (!rule->Handle(match, message)) {
            return false;
        }
    }
    return true;
}

PhoneNumberMatch* PhoneNumberMatched::HandleCodesRule(PhoneNumberMatch* phoneNumberMatch, icu::UnicodeString& message)
{
    PhoneNumberMatch* match = phoneNumberMatch;
    std::vector<CodeRule*> rules = GetCodesRulesInside();
    if (rules.size() == 0) {
        return nullptr;
    }
    for (CodeRule* rule : rules) {
        if (rule == nullptr) {
            continue;
        }
        match = rule->Handle(match, message);
    }
    return match;
}

std::vector<MatchedNumberInfo> PhoneNumberMatched::HandlePositiveRule(PhoneNumberMatch* match,
    icu::UnicodeString& message)
{
    std::vector<MatchedNumberInfo> infoList;
    std::vector<PositiveRule*> rules = GetPositiveRulesInside();
    for (PositiveRule* rule : rules) {
        if (rule == nullptr) {
            continue;
        }
        infoList = rule->Handle(match, message);
        if (infoList.size() != 0) {
            break;
        }
    }
    return infoList;
}

icu::UnicodeString PhoneNumberMatched::DealStringWithOneBracket(icu::UnicodeString& message)
{
    if (IsNumberWithOneBracket(message)) {
        return message.tempSubString(1);
    }
    return message;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS