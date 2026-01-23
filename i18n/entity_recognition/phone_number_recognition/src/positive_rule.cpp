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
#include "positive_rule.h"

#include "log.h"
#include "phonenumbers/phonenumber.h"
#include "phonenumbers/phonenumberutil.h"
#include "phonenumbers/shortnumberinfo.h"

namespace OHOS {
namespace Global {
namespace I18n {
using i18n::phonenumbers::PhoneNumber;
using i18n::phonenumbers::PhoneNumberUtil;
using i18n::phonenumbers::ShortNumberInfo;
const int LIST_SIZE = 2;
PositiveRule::PositiveRule(icu::UnicodeString& regex, std::string& handleType, std::string& insensitive)
{
    this->regex = regex;
    this->handleType = handleType;
    this->insensitive = insensitive;
}

icu::RegexPattern* PositiveRule::GetPattern()
{
    UErrorCode status = U_ZERO_ERROR;
    icu::RegexPattern* pattern;
    if (insensitive == "True") {
        pattern = icu::RegexPattern::compile(this->regex, URegexpFlag::UREGEX_CASE_INSENSITIVE, status);
    } else {
        pattern = icu::RegexPattern::compile(this->regex, 0, status);
    }
    if (U_FAILURE(status)) {
        LOGE("PositiveRule::GetPattern: Compile regex pattern failed.");
        return nullptr;
    }
    return pattern;
}

bool PositiveRule::IsNumberWithOneBracket(icu::UnicodeString& message)
{
    if (message.isEmpty()) {
        return false;
    }
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
    return false;
}

icu::UnicodeString PositiveRule::DealStringWithOneBracket(icu::UnicodeString& message)
{
    if (IsNumberWithOneBracket(message)) {
        return message.tempSubString(1);
    }
    return message;
}

std::vector<MatchedNumberInfo> PositiveRule::Handle(PhoneNumberMatch* match, icu::UnicodeString& message)
{
    if (match == nullptr) {
        return {};
    }
    icu::UnicodeString rawString = match->raw_string().c_str();
    icu::UnicodeString str = DealStringWithOneBracket(rawString);
    icu::RegexPattern* pattern = this->GetPattern();
    if (pattern == nullptr) {
        return {};
    }
    UErrorCode status = U_ZERO_ERROR;
    icu::RegexMatcher* mat1 = pattern->matcher(str, status);
    if (mat1 != nullptr && mat1->find(status)) {
        std::vector<MatchedNumberInfo> infoList = this->HandleInner(match, message);
        delete mat1;
        delete pattern;
        return infoList;
    }
    if (mat1 != nullptr) {
        delete mat1;
    }
    icu::RegexMatcher* mat2 = pattern->matcher(message, status);
    if (mat2 != nullptr && mat2->find(status)) {
        std::vector<MatchedNumberInfo> infoList = this->HandleInner(match, message);
        delete mat2;
        delete pattern;
        return infoList;
    }
    if (mat2 != nullptr) {
        delete mat2;
    }
    delete pattern;
    return {};
}

std::vector<MatchedNumberInfo> PositiveRule::HandleInner(PhoneNumberMatch *possibleNumber, icu::UnicodeString& message)
{
    if (handleType == "Operator") {
        return HandleOperator(possibleNumber, message);
    } else if (handleType == "Blank") {
        return HandleBlank(possibleNumber, message);
    } else if (handleType == "Slant") {
        return HandleSlant(possibleNumber, message);
    } else if (handleType == "StartWithMobile") {
        return HandleStartWithMobile(possibleNumber, message);
    } else if (handleType == "EndWithMobile") {
        return HandleEndWithMobile(possibleNumber, message);
    }
    return HandleDefault(possibleNumber, message);
}

std::vector<MatchedNumberInfo> PositiveRule::HandleDefault(PhoneNumberMatch* possibleNumber,
    icu::UnicodeString& message)
{
    MatchedNumberInfo matcher;
    matcher.SetBegin(0);
    matcher.SetEnd(1);
    icu::UnicodeString content = "";
    matcher.SetContent(content);
    std::vector<MatchedNumberInfo> matchedNumberInfoList;
    matchedNumberInfoList.push_back(matcher);
    return matchedNumberInfoList;
}

std::vector<MatchedNumberInfo> PositiveRule::HandleOperator(PhoneNumberMatch* possibleNumber,
    icu::UnicodeString& message)
{
    std::vector<MatchedNumberInfo> matchedNumberInfoList;
    if (possibleNumber == nullptr) {
        return matchedNumberInfoList;
    }
    MatchedNumberInfo matcher;
    if (possibleNumber->raw_string()[0] == '(' || possibleNumber->raw_string()[0] == '[') {
        matcher.SetBegin(possibleNumber->start() + 1);
    } else {
        matcher.SetBegin(possibleNumber->start());
    }
    matcher.SetEnd(possibleNumber->end());
    matcher.SetContent(message);
    matchedNumberInfoList.push_back(matcher);
    return matchedNumberInfoList;
}

std::vector<MatchedNumberInfo> PositiveRule::HandleBlank(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message)
{
    std::vector<MatchedNumberInfo> matchedNumberInfoList;
    if (possibleNumber == nullptr) {
        return matchedNumberInfoList;
    }
    icu::RegexPattern* pattern = GetPattern();
    if (pattern == nullptr) {
        return matchedNumberInfoList;
    }
    UErrorCode status = U_ZERO_ERROR;
    icu::UnicodeString number = possibleNumber->raw_string().c_str();
    icu::RegexMatcher* matcher = pattern->matcher(number, status);
    if (U_FAILURE(status) || matcher == nullptr) {
        LOGE("PositiveRule::HandleBlank: Pattern match failed.");
        delete pattern;
        return {};
    }
    icu::UnicodeString negativeRegex = "(?<![-\\d])(23{6,7})(?![-\\d])";
    icu::RegexMatcher negativePattern(negativeRegex, 0, status);
    if (U_FAILURE(status)) {
        LOGE("PositiveRule::HandleBlank: Create negative pattern failed.");
        delete matcher;
        delete pattern;
        return {};
    }
    icu::UnicodeString speString = "5201314";
    negativePattern.reset(number);
    if (matcher->find()) {
        if (negativePattern.find() || number == speString) {
            delete matcher;
            delete pattern;
            return matchedNumberInfoList;
        }
        MatchedNumberInfo matchedNumberInfo;
        if (possibleNumber->raw_string()[0] != '(' && possibleNumber->raw_string()[0] != '[') {
            matchedNumberInfo.SetBegin(matcher->start(status) + possibleNumber->start());
        } else {
            matchedNumberInfo.SetBegin(possibleNumber->start());
        }
        matchedNumberInfo.SetEnd(matcher->end(status) + possibleNumber->start());
        matchedNumberInfo.SetContent(number);
        matchedNumberInfoList.push_back(matchedNumberInfo);
    }
    delete matcher;
    delete pattern;
    return matchedNumberInfoList;
}

std::vector<MatchedNumberInfo> PositiveRule::HandleSlant(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message)
{
    std::vector<MatchedNumberInfo> matchedNumberInfoList;
    if (possibleNumber == nullptr) {
        return matchedNumberInfoList;
    }
    icu::RegexPattern* pattern = GetPattern();
    if (pattern == nullptr) {
        return matchedNumberInfoList;
    }
    UErrorCode status = U_ZERO_ERROR;
    icu::UnicodeString number = possibleNumber->raw_string().c_str();
    icu::RegexMatcher* matcher = pattern->matcher(number, status);
    if (U_FAILURE(status) || matcher == nullptr) {
        LOGE("PositiveRule::HandleSlant: Pattern match failed.");
        delete pattern;
        return matchedNumberInfoList;
    }
    if (matcher->find()) {
        int start = matcher->start(status);
        std::vector<MatchedNumberInfo> tempList = GetNumbersWithSlant(number);
        if (tempList.size() == LIST_SIZE && start == 1) {
            start = 0;
        }
        if (tempList.size() > 0) {
            MatchedNumberInfo matchedNumberInfo;
            matchedNumberInfo.SetBegin(tempList[0].GetBegin() + start + possibleNumber->start());
            matchedNumberInfo.SetEnd(tempList[0].GetEnd() + possibleNumber->start());
            icu::UnicodeString contentFirst = tempList[0].GetContent();
            matchedNumberInfo.SetContent(contentFirst);
            matchedNumberInfoList.push_back(matchedNumberInfo);
            if (tempList.size() == LIST_SIZE) {
                MatchedNumberInfo numberInfo;
                numberInfo.SetBegin(tempList[1].GetBegin() + start + possibleNumber->start());
                numberInfo.SetEnd(tempList[1].GetEnd() + possibleNumber->start());
                icu::UnicodeString contentSecond = tempList[1].GetContent();
                numberInfo.SetContent(contentSecond);
                matchedNumberInfoList.push_back(numberInfo);
            }
        }
    }
    delete matcher;
    delete pattern;
    return matchedNumberInfoList;
}

std::vector<MatchedNumberInfo> PositiveRule::HandleStartWithMobile(PhoneNumberMatch* possibleNumber,
    icu::UnicodeString& message)
{
    return HandlePossibleNumberWithPattern(possibleNumber, message, false);
}

std::vector<MatchedNumberInfo> PositiveRule::HandleEndWithMobile(PhoneNumberMatch* possibleNumber,
    icu::UnicodeString& message)
{
    return HandlePossibleNumberWithPattern(possibleNumber, message, true);
}

std::vector<MatchedNumberInfo> PositiveRule::GetNumbersWithSlant(icu::UnicodeString& testStr)
{
    std::vector<MatchedNumberInfo> shortList;
    ShortNumberInfo* shortInfo = new (std::nothrow) ShortNumberInfo();
    if (shortInfo == nullptr) {
        LOGE("ShortNumberInfo construct failed.");
        return shortList;
    }
    std::string numberFisrt = "";
    std::string numberEnd = "";
    int slantIndex = 0;
    for (int i = 0; i < testStr.length(); i++) {
        if (testStr[i] == '/' || testStr[i] == '|') {
            slantIndex = i;
            testStr.tempSubString(0, i).toUTF8String(numberFisrt);
            testStr.tempSubString(i + 1).toUTF8String(numberEnd);
        }
    }
    PhoneNumber phoneNumberFirst;
    PhoneNumber phoneNumberEnd;
    PhoneNumberUtil* pnu = PhoneNumberUtil::GetInstance();
    if (pnu == nullptr) {
        delete shortInfo;
        LOGE("PositiveRule::GetNumbersWithSlant: Get phone number util failed.");
        return shortList;
    }
    pnu->Parse(numberFisrt, "CN", &phoneNumberFirst);
    pnu->Parse(numberEnd, "CN", &phoneNumberEnd);
    if (shortInfo->IsValidShortNumber(phoneNumberFirst)) {
        MatchedNumberInfo matchedNumberInfoFirst;
        matchedNumberInfoFirst.SetBegin(0);
        matchedNumberInfoFirst.SetEnd(slantIndex);
        icu::UnicodeString contentFirst = numberFisrt.c_str();
        matchedNumberInfoFirst.SetContent(contentFirst);
        shortList.push_back(matchedNumberInfoFirst);
    }
    if (shortInfo->IsValidShortNumber(phoneNumberEnd)) {
        MatchedNumberInfo matchedNumberInfoEnd;
        matchedNumberInfoEnd.SetBegin(slantIndex + 1);
        matchedNumberInfoEnd.SetEnd(testStr.length());
        icu::UnicodeString contentEnd = numberEnd.c_str();
        matchedNumberInfoEnd.SetContent(contentEnd);
        shortList.push_back(matchedNumberInfoEnd);
    }
    delete shortInfo;
    return shortList;
}

std::vector<MatchedNumberInfo> PositiveRule::HandlePossibleNumberWithPattern(PhoneNumberMatch* possibleNumber,
    icu::UnicodeString& message, bool isStartsWithNumber)
{
    std::vector<MatchedNumberInfo> matchedList;
    if (possibleNumber == nullptr) {
        return matchedList;
    }
    icu::RegexPattern* pattern = GetPattern();
    if (pattern == nullptr) {
        LOGE("PositiveRule::HandlePossibleNumberWithPattern: RegexPattern is nullptr.");
        return matchedList;
    }
    UErrorCode status = U_ZERO_ERROR;
    icu::RegexMatcher* mat = pattern->matcher(message, status);
    if (U_FAILURE(status) || mat == nullptr) {
        LOGE("PositiveRule::HandlePossibleNumberWithPattern: Pattern match failed.");
        delete pattern;
        return matchedList;
    }
    icu::UnicodeString possible = possibleNumber->raw_string().c_str();
    while (mat->find(status)) {
        int start = mat->start(status);
        int end = mat->end(status);
        icu::UnicodeString matched = message.tempSubString(start, end - start);
        bool isMatch = isStartsWithNumber ? matched.startsWith(possible) : matched.endsWith(possible);
        if (isMatch) {
            MatchedNumberInfo info;
            info.SetBegin(isStartsWithNumber ? start : end - possible.length());
            info.SetEnd(isStartsWithNumber ? (start + possible.length()) : end);
            info.SetContent(possible);
            matchedList.push_back(info);
        }
    }
    delete mat;
    delete pattern;
    return matchedList;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS