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
 * See the License for the specific locale governing permissions and
 * limitations under the License.
 */
#include "rules_engine.h"

#include <algorithm>
#include "log.h"
#include "utils.h"

namespace OHOS {
namespace Global {
namespace I18n {
RulesEngine::RulesEngine()
{
}

RulesEngine::RulesEngine(DateTimeRule* dateTimeRule, RulesSet& rulesSet)
{
    this->dateTimeRule = dateTimeRule;
    this->rulesMap = rulesSet.rulesMap;
    this->subRules = rulesSet.subRules;
    this->param = rulesSet.param;
    this->paramBackup = rulesSet.paramBackup;
    Init();
}

RulesEngine::~RulesEngine()
{
}

std::vector<MatchedDateTimeInfo> RulesEngine::Match(icu::UnicodeString& message)
{
    std::vector<MatchedDateTimeInfo> matches;
    std::vector<std::string> keys;
    for (auto& kv : this->patterns) {
        keys.push_back(kv.first);
    }
    std::string res;
    message.toUTF8String(res);
    for (auto& key : keys) {
        UErrorCode status = U_ZERO_ERROR;
        icu::UnicodeString regex = this->patterns[key];
        icu::RegexPattern* pattern = icu::RegexPattern::compile(regex,
            URegexpFlag::UREGEX_CASE_INSENSITIVE, status);
        if (IsRegexPatternInvalid(pattern) || U_FAILURE(status)) {
            LOGE("Match failed because pattern is nullptr.");
            return matches;
        }
        icu::RegexMatcher* matcher = pattern->matcher(message, status);
        if (IsRegexMatcherInvalid(matcher)) {
            LOGE("Match failed because pattern matcher failed.");
            delete pattern;
            return matches;
        }
        while (matcher->find(status)) {
            int begin = matcher->start(status);
            int end = matcher->end(status);
            MatchedDateTimeInfo match(begin, end, key);
            matches.push_back(match);
        }
        delete matcher;
        delete pattern;
    }
    return matches;
}

void RulesEngine::Init()
{
    for (const auto& rule : rulesMap) {
        std::string rulesKey = rule.first;
        std::string rulesValue = rule.second;
        rulesValue = InitSubRules(rulesValue);
        rulesValue = InitOptRules(rulesValue);
        bool isVaild = InitRules(rulesValue);
        icu::UnicodeString regex = rulesValue.c_str();
        if (!regex.trim().isEmpty() && isVaild) {
            this->patterns.insert({rulesKey, regex});
        }
    }
}

bool RulesEngine::InitRules(std::string& rulesValue)
{
    bool isVaild = true;
    if (this->dateTimeRule == nullptr) {
        LOGE("InitRules failed because this->dateTimeRule is nullptr.");
        return false;
    }
    icu::RegexPattern* pattern = dateTimeRule->GetPatternByKey("rules");
    if (IsRegexPatternInvalid(pattern)) {
        LOGE("InitRules failed because pattern is nullptr.");
        return false;
    }
    UErrorCode status = U_ZERO_ERROR;
    if (param.size() != 0 || paramBackup.size() != 0) {
        icu::UnicodeString rules = rulesValue.c_str();
        icu::RegexMatcher* matcher = pattern->matcher(rules, status);
        if (IsRegexMatcherInvalid(matcher)) {
            LOGE("InitRules failed because pattern matcher failed.");
            return false;
        }
        while (matcher->find(status) && isVaild) {
            icu::UnicodeString value = matcher->group(1, status);
            std::string valueStr;
            value.toUTF8String(valueStr);
            std::string paramValue = this->dateTimeRule->Get(param, valueStr);
            std::string paramBackupValue = this->dateTimeRule->Get(paramBackup, valueStr);
            std::string targetStr =
                (paramValue.length() == 0 && paramBackupValue.length() != 0) ? paramBackupValue : paramValue;
            targetStr = (paramValue.length() != 0 && paramBackupValue.length() != 0 &&
                !DateTimeRule::CompareBeginEnd(paramValue, "]", false) &&
                !DateTimeRule::CompareBeginEnd(paramValue, "]\\b", false)) ?
                paramValue + "|" + paramBackupValue : targetStr;
            isVaild = (targetStr.length() == 0) ? false : true;
            std::string replaceStr = "[" + valueStr + "]";
            if (DateTimeRule::trim(paramValue).length() != 0) {
                rulesValue = StrReplaceAll(rulesValue, replaceStr, targetStr);
            }
        }
        delete matcher;
    }
    return isVaild;
}

std::string RulesEngine::InitOptRules(std::string& rule)
{
    std::string rulesValue = rule;
    icu::UnicodeString matchedStr = rule.c_str();
    UErrorCode status = U_ZERO_ERROR;
    if (this->dateTimeRule == nullptr) {
        LOGE("InitOptRules failed because this->dateTimeRule is nullptr.");
        return rulesValue;
    }
    icu::RegexPattern* pattern = dateTimeRule->GetPatternByKey("optrules");
    if (IsRegexPatternInvalid(pattern)) {
        LOGE("InitOptRules failed because pattern is nullptr.");
        return rulesValue;
    }
    if (param.size() != 0 || paramBackup.size() != 0) {
        icu::RegexMatcher* matcher = pattern->matcher(matchedStr, status);
        if (IsRegexMatcherInvalid(matcher)) {
            LOGE("InitOptRules failed because pattern matcher failed.");
            return rulesValue;
        }
        while (matcher->find(status)) {
            icu::UnicodeString key = matcher->group(1, status);
            std::string keyStr;
            key.toUTF8String(keyStr);
            std::string value = "param_" + keyStr;
            int begin = matcher->start(status);
            int end = matcher->end(status);
            std::string paramValue = this->dateTimeRule->Get(param, value);
            std::string paramBackupValue = this->dateTimeRule->Get(paramBackup, value);
            std::string targetStr = paramValue;
            if (paramValue.length() == 0 && paramBackupValue.length() != 0) {
                targetStr = paramBackupValue;
            } else if (DateTimeRule::trim(paramValue).length() != 0 &&
                DateTimeRule::trim(paramBackupValue).length() != 0) {
                targetStr = paramValue + '|' + paramBackupValue;
            }
            if (paramValue.length() == 0 && paramBackupValue.length() == 0) {
                int rIndex = (rule.substr(begin - 1, 1) == "|") ? begin - 1 : begin;
                std::string replaceStr1 = rule.substr(rIndex, end - rIndex);
                rulesValue = StrReplaceAll(rulesValue, replaceStr1, "");
            }
            std::string replaceStr2 = "[paramopt_" + keyStr + "]";
            rulesValue = StrReplaceAll(rulesValue, replaceStr2, targetStr);
        }
        delete matcher;
    }
    return rulesValue;
}

std::string RulesEngine::InitSubRules(std::string& rule)
{
    std::string rulesValue = rule;
    icu::UnicodeString matchedStr = rule.c_str();
    UErrorCode status = U_ZERO_ERROR;
    icu::RegexPattern* pattern = dateTimeRule->GetPatternByKey("subrules");
    if (IsRegexPatternInvalid(pattern)) {
        LOGE("InitSubRules failed because pattern is nullptr.");
        return rulesValue;
    }
    if (subRules.size() != 0) {
        icu::RegexMatcher* matcher = pattern->matcher(matchedStr, status);
        if (IsRegexMatcherInvalid(matcher)) {
            LOGE("InitSubRules failed because pattern matcher failed.");
            return rulesValue;
        }
        while (matcher->find(status)) {
            icu::UnicodeString text = matcher->group(0, status);
            icu::UnicodeString value = matcher->group(1, status);
            std::string valueStr;
            value.toUTF8String(valueStr);
            std::string targetStr = subRules[valueStr];
            std::string replaceStr;
            text.toUTF8String(replaceStr);
            rulesValue = StrReplaceAll(rulesValue, replaceStr, targetStr);
        }
        delete matcher;
    }
    return rulesValue;
}

bool RulesEngine::IsRegexPatternInvalid(icu::RegexPattern* pattern)
{
    return pattern == nullptr;
}

bool RulesEngine::IsRegexMatcherInvalid(icu::RegexMatcher* matcher)
{
    return matcher == nullptr;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS