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
#ifndef OHOS_GLOBAL_RULES_ENGINE_H
#define OHOS_GLOBAL_RULES_ENGINE_H

#include <string>
#include <unicode/regex.h>
#include <unordered_map>
#include <vector>

#include "date_time_rule.h"
#include "matched_date_time_info.h"

namespace OHOS {
namespace Global {
namespace I18n {
struct RulesSet {
public:
    RulesSet (std::unordered_map<std::string, std::string> rulesMap,
        std::unordered_map<std::string, std::string> subRules,
        std::unordered_map<std::string, std::string> param,
        std::unordered_map<std::string, std::string> paramBackup)
    {
        this->rulesMap = rulesMap;
        this->subRules = subRules;
        this->param = param;
        this->paramBackup = paramBackup;
    }
    std::unordered_map<std::string, std::string> rulesMap;
    std::unordered_map<std::string, std::string> subRules;
    std::unordered_map<std::string, std::string> param;
    std::unordered_map<std::string, std::string> paramBackup;
};

class RulesEngine {
public:
    RulesEngine();
    RulesEngine(DateTimeRule* dateTimeRule, RulesSet& rulesSet);
    virtual ~RulesEngine();
    std::vector<MatchedDateTimeInfo> Match(icu::UnicodeString& message);

protected:
    virtual bool IsRegexPatternInvalid(icu::RegexPattern* pattern);
    virtual bool IsRegexMatcherInvalid(icu::RegexMatcher* matcher);
    
private:
    void Init();
    std::string InitOptRules(std::string& rule);
    std::string InitSubRules(std::string& rule);
    bool InitRules(std::string& rulesValue);
    DateTimeRule* dateTimeRule = nullptr;
    std::unordered_map<std::string, std::string> rulesMap;
    std::unordered_map<std::string, std::string> subRules;
    std::unordered_map<std::string, std::string> param;
    std::unordered_map<std::string, std::string> paramBackup;
    std::unordered_map<std::string, icu::UnicodeString> patterns;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif