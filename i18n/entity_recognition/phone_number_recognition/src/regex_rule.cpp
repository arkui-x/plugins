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
#include "regex_rule.h"

#include "log.h"
#include "phonenumbers/phonenumber.h"
#include "phonenumbers/phonenumberutil.h"
#include "phonenumbers/shortnumberinfo.h"

namespace OHOS {
namespace Global {
namespace I18n {
using i18n::phonenumbers::PhoneNumberMatch;
using i18n::phonenumbers::PhoneNumber;
using i18n::phonenumbers::PhoneNumberUtil;
using i18n::phonenumbers::ShortNumberInfo;

const int RULE_TYPE_CONTAIN = 9;
const int RULE_TYPE_CONTAIN_OR_INTERSECT = 8;
const int ALPHA_CHECK_LIMIT = 2;
const int THIRD_CHAR_INDEX = 2;
const int PREFIX_LENGTH_THREE = 3;
const int PREFIX_LENGTH_FIVE = 5;
const int DIGIT_COUNT_MAX_PREFIX_ONE = 11;
const int DIGIT_COUNT_MAX_PREFIX_ZERO = 12;
const int DIGIT_COUNT_EXPECTED_PREFIX_THREE = 10;
const int DIGIT_COUNT_MIN_GENERIC = 9;
const int DIGIT_COUNT_SHORT = 8;
const int DIGIT_COUNT_MIN_VALID = 4;
const int EXTRA_DIGIT_COUNT_TWO = 2;
const size_t TEMP_LIST_SIZE_TWO = 2;

RegexRule::RegexRule(icu::UnicodeString& regex, std::string& isValidType, std::string& handleType,
    std::string& insensitive, std::string& type)
{
    this->regex = regex;
    if (type == "CONTAIN") {
        this->type = RULE_TYPE_CONTAIN;
    } else if (type == "CONTAIN_OR_INTERSECT") {
        this->type = RULE_TYPE_CONTAIN_OR_INTERSECT;
    } else {
        this->type = 0;
    }
    this->isValidType = isValidType;
    this->handleType = handleType;
    this->insensitive = insensitive;
}

RegexRule::~RegexRule()
{
}

int RegexRule::CountDigits(icu::UnicodeString& str)
{
    int count = 0;
    int len = str.length();
    for (int i = 0; i < len; i++) {
        if (u_isdigit(str[i])) {
            count++;
        }
    }
    return count;
}

int RegexRule::GetType()
{
    return type;
}

icu::RegexPattern* RegexRule::GetPattern()
{
    UErrorCode status = U_ZERO_ERROR;
    icu::RegexPattern* pattern = nullptr;
    if (insensitive == "True") {
        pattern = icu::RegexPattern::compile(this->regex, URegexpFlag::UREGEX_CASE_INSENSITIVE, status);
    } else {
        pattern = icu::RegexPattern::compile(this->regex, 0, status);
    }
    if (U_FAILURE(status)) {
        LOGE("RegexRule::GetPattern: Compile regex pattern failed.");
        return nullptr;
    }
    return pattern;
}

PhoneNumberMatch* RegexRule::IsValid(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message)
{
    if (isValidType == "PreSuf") {
        return IsValidPreSuf(possibleNumber, message);
    } else if (isValidType == "Code") {
        return IsValidCode(possibleNumber, message);
    } else if (isValidType == "Rawstr") {
        return IsValidRawstr(possibleNumber, message);
    }
    return IsValidDefault(possibleNumber, message);
}

PhoneNumberMatch* RegexRule::IsValidPreSuf(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message)
{
    if (possibleNumber != nullptr) {
        if (possibleNumber->start() - 1 >= 0) {
            return IsValidStart(possibleNumber, message);
        }
        if (possibleNumber->end() <= message.length() - 1) {
            return IsValidEnd(possibleNumber, message);
        }
    }
    return possibleNumber;
}

PhoneNumberMatch* RegexRule::IsValidEnd(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message)
{
    if (possibleNumber == nullptr) {
        return possibleNumber;
    }
    icu::UnicodeString after = message.tempSubString(possibleNumber->end());
    bool isTwo = true;
    int len = after.length();
    for (int i = 0; i < len; i++) {
        UChar32 afterChar = after[i];
        if (i == 0 && !u_isUUppercase(afterChar)) {
            isTwo = false;
            break;
        }
        if (i < ALPHA_CHECK_LIMIT && u_isUAlphabetic(afterChar)) {
            if (u_isUUppercase(afterChar)) {
                continue;
            } else {
                isTwo = false;
                break;
            }
        }
        if (i == 1 || i == THIRD_CHAR_INDEX) {
            if (afterChar == '-' || afterChar == '\'') {
                isTwo = false;
                break;
            } else if (u_isdigit(afterChar) || u_isspace(afterChar)) {
                break;
            } else if (!u_isUAlphabetic(afterChar)) {
                break;
            } else {
                isTwo = false;
                break;
            }
        }
    }
    return isTwo ? nullptr : possibleNumber;
}

PhoneNumberMatch* RegexRule::IsValidStart(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message)
{
    if (possibleNumber == nullptr) {
        return possibleNumber;
    }
    icu::UnicodeString before = message.tempSubString(0, possibleNumber->start());
    bool isTwo = true;
    int len = before.length();
    for (int i = 0; i < len; i++) {
        char beforeChar = before[len - 1 - i];
        if (i == 0 && !u_isUUppercase(beforeChar)) {
            isTwo = false;
            break;
        }
        if (i < ALPHA_CHECK_LIMIT && u_isUAlphabetic(beforeChar)) {
            if (u_isUUppercase(beforeChar)) {
                continue;
            } else {
                isTwo = false;
                break;
            }
        }
        if (beforeChar == '-' || beforeChar == '\'') {
            isTwo = false;
            break;
        } else if (u_isdigit(beforeChar) || u_isspace(beforeChar)) {
            break;
        } else if (!u_isUAlphabetic(beforeChar)) {
            break;
        } else {
            isTwo = false;
            break;
        }
    }
    return isTwo ? nullptr : possibleNumber;
}

PhoneNumberMatch* RegexRule::IsValidDefault(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message)
{
    return possibleNumber;
}

bool RegexRule::PrefixValid(icu::UnicodeString& number, int length)
{
    icu::UnicodeString preNumber = number.tempSubString(0, length);
    if (length == 1) {
        if (number[0] == '0' || number[0] == '1' || number[0] == '+') {
            return true;
        }
    } else if (length == PREFIX_LENGTH_THREE) {
        if (preNumber == "400" || preNumber == "800") {
            return true;
        }
    } else if (length == PREFIX_LENGTH_FIVE) {
        if (preNumber == "11808" || preNumber == "17909" || preNumber == "12593" ||
            preNumber == "17951" || preNumber == "17911") {
            return true;
        }
    }
    return false;
}

bool RegexRule::NumberValid(icu::UnicodeString& number)
{
    int lengthOne = 1;
    int lengthThree = PREFIX_LENGTH_THREE;
    if (number[0] == '1' && CountDigits(number) > DIGIT_COUNT_MAX_PREFIX_ONE) {
        int lengthFive = PREFIX_LENGTH_FIVE;
        if (!PrefixValid(number, lengthFive)) {
            return false;
        }
    } else if (number[0] == '0' && CountDigits(number) > DIGIT_COUNT_MAX_PREFIX_ZERO && number[1] != '0') {
        return false;
    } else if (PrefixValid(number, lengthThree) && CountDigits(number) != DIGIT_COUNT_EXPECTED_PREFIX_THREE) {
        return false;
    } else if (!PrefixValid(number, lengthOne) && !PrefixValid(number, lengthThree) &&
        CountDigits(number) >= DIGIT_COUNT_MIN_GENERIC) {
        if (number.trim()[0] != '9' && number.trim()[0] != '1') {
            return false;
        }
    } else if (CountDigits(number) <= DIGIT_COUNT_MIN_VALID) {
        return false;
    }
    return true;
}

PhoneNumberMatch* RegexRule::IsValidCode(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message)
{
    if (possibleNumber == nullptr) {
        return possibleNumber;
    }
    icu::UnicodeString number = possibleNumber->raw_string().c_str();
    int32_t ind = number.trim().indexOf(";ext=");
    if (ind != -1) {
        number = number.trim().tempSubString(0, ind);
    }
    if (number[0] == '(' || number[0] == '[') {
        StartWithBrackets(number);
    }
    if (!NumberValid(number)) {
        return nullptr;
    }
    return possibleNumber;
}

PhoneNumberMatch* RegexRule::IsValidRawstr(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message)
{
    icu::UnicodeString number = possibleNumber->raw_string().c_str();
    int32_t ind = number.trim().indexOf(";ext=");
    if (ind != -1) {
        number = number.trim().tempSubString(0, ind);
    }
    if (number[0] == '(' || number[0] == '[') {
        number = number.tempSubString(1);
    }
    if ((number[0] != '0' && CountDigits(number) == DIGIT_COUNT_SHORT) ||
        CountDigits(number) <= DIGIT_COUNT_MIN_VALID) {
        return nullptr;
    }
    return possibleNumber;
}

std::vector<MatchedNumberInfo> RegexRule::Handle(PhoneNumberMatch *possibleNumber, icu::UnicodeString& message)
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

std::vector<MatchedNumberInfo> RegexRule::HandleDefault(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message)
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

std::vector<MatchedNumberInfo> RegexRule::HandleOperator(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message)
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

std::vector<MatchedNumberInfo> RegexRule::HandleBlank(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message)
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
        LOGE("RegexRule::HandleBlank: Pattern match failed.");
        delete pattern;
        return matchedNumberInfoList;
    }
    icu::UnicodeString negativeRegex = "(?<![-\\d])(23{6,7})(?![-\\d])";
    icu::RegexMatcher negativePattern(negativeRegex, 0, status);
    if (U_FAILURE(status)) {
        delete matcher;
        delete pattern;
        return matchedNumberInfoList;
    }
    negativePattern.reset(number);
    if (matcher->find()) {
        icu::UnicodeString speString = "5201314";
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

std::vector<MatchedNumberInfo> RegexRule::HandleSlant(PhoneNumberMatch* possibleNumber, icu::UnicodeString& message)
{
    std::vector<MatchedNumberInfo> matchedNumberInfoList;
    if (possibleNumber == nullptr) {
        return matchedNumberInfoList;
    }
    icu::RegexPattern* pattern = GetPattern();
    if (pattern == nullptr) {
        LOGE("RegexRule::HandleSlant: pattern is nullptr.");
        return matchedNumberInfoList;
    }
    UErrorCode status = U_ZERO_ERROR;
    icu::UnicodeString number = possibleNumber->raw_string().c_str();
    icu::RegexMatcher* matcher = pattern->matcher(number, status);
    if (U_FAILURE(status) || matcher == nullptr) {
        LOGE("RegexRule::HandleSlant: Pattern match failed.");
        delete pattern;
        return matchedNumberInfoList;
    }
    if (matcher->find()) {
        int start = matcher->start(status);
        std::vector<MatchedNumberInfo> tempList = GetNumbersWithSlant(number);
        if (tempList.size() == TEMP_LIST_SIZE_TWO && start == 1) {
            start = 0;
        }
        if (tempList.size() > 0) {
            MatchedNumberInfo matchedNumberInfo;
            matchedNumberInfo.SetBegin(tempList[0].GetBegin() + start + possibleNumber->start());
            matchedNumberInfo.SetEnd(tempList[0].GetEnd() + possibleNumber->start());
            icu::UnicodeString contentFirst = tempList[0].GetContent();
            matchedNumberInfo.SetContent(contentFirst);
            matchedNumberInfoList.push_back(matchedNumberInfo);
            if (tempList.size() == TEMP_LIST_SIZE_TWO) {
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

std::vector<MatchedNumberInfo> RegexRule::HandleStartWithMobile(PhoneNumberMatch* possibleNumber,
    icu::UnicodeString& message)
{
    return HandlePossibleNumberWithPattern(possibleNumber, message, false);
}

std::vector<MatchedNumberInfo> RegexRule::HandleEndWithMobile(PhoneNumberMatch* possibleNumber,
    icu::UnicodeString& message)
{
    return HandlePossibleNumberWithPattern(possibleNumber, message, true);
}

// Handle phone number starting with '(' or '['
void RegexRule::StartWithBrackets(icu::UnicodeString& number)
{
    icu::UnicodeString right = "";
    if (number[0] == '(') {
        right = ')';
    }
    if (number[0] == '[') {
        right = ']';
    }
    int neind = number.indexOf(right);
    if (neind != -1) {
        icu::UnicodeString phoneStr = number.tempSubString(0, neind);
        int phoneLength = CountDigits(phoneStr);
        icu::UnicodeString extraStr = number.tempSubString(neind);
        int extra = CountDigits(extraStr);
        if ((phoneLength > DIGIT_COUNT_MIN_VALID) && (extra == 1 || extra == EXTRA_DIGIT_COUNT_TWO)) {
            number = number.tempSubString(1, neind - 1);
        } else {
            number = number.tempSubString(1);
        }
    } else {
        number = number.tempSubString(1);
    }
}

// Identify short number separated by '/'
std::vector<MatchedNumberInfo> RegexRule::GetNumbersWithSlant(icu::UnicodeString& testStr)
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
    PhoneNumberUtil* pnu = PhoneNumberUtil::GetInstance();
    if (pnu == nullptr) {
        delete shortInfo;
        LOGE("RegexRule::GetNumbersWithSlant: Get phone number util failed.");
        return shortList;
    }
    PhoneNumber phoneNumberFirst;
    PhoneNumber phoneNumberEnd;
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

std::vector<MatchedNumberInfo> RegexRule::HandlePossibleNumberWithPattern(PhoneNumberMatch* possibleNumber,
    icu::UnicodeString& message, bool isStartsWithNumber)
{
    std::vector<MatchedNumberInfo> matchedList;
    if (possibleNumber == nullptr) {
        return matchedList;
    }
    icu::RegexPattern* pattern = GetPattern();
    if (pattern == nullptr) {
        LOGE("RegexPattern is nullptr.");
        return matchedList;
    }
    UErrorCode status = U_ZERO_ERROR;
    icu::RegexMatcher* mat = pattern->matcher(message, status);
    if (U_FAILURE(status) || mat == nullptr) {
        LOGE("RegexRule::HandlePossibleNumberWithPattern: Pattern match failed.");
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