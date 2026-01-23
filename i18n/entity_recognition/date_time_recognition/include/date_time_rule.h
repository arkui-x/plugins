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
#ifndef OHOS_GLOBAL_DATE_TIME_RULE_H
#define OHOS_GLOBAL_DATE_TIME_RULE_H

#include <cctype>
#include <filesystem>
#include <string>
#include <unicode/regex.h>
#include <unordered_map>
#include <vector>

#include "libxml/globals.h"
#include "libxml/tree.h"
#include "libxml/xmlstring.h"

namespace OHOS {
namespace Global {
namespace I18n {
class DateTimeRule {
public:
    DateTimeRule(const std::string& locale);
    ~DateTimeRule();

    std::unordered_map<std::string, std::string> GetUniverseRules();
    std::unordered_map<std::string, std::string> GetLocalesRules();
    std::unordered_map<std::string, std::string> GetLocalesRulesBackup();
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> GetSubRulesMap();
    std::unordered_map<std::string, std::string> GetSubRules();
    std::unordered_map<std::string, std::string> GetFilterRules();
    std::unordered_map<std::string, std::string> GetPastRules();
    std::unordered_map<std::string, std::string> GetParam();
    std::unordered_map<std::string, std::string> GetParamBackup();
    int CompareLevel(std::string& key1, std::string& key2);
    std::string Get(std::unordered_map<std::string, std::string>& param, std::string& ruleName);
    std::string GetLocale();
    bool IsRelDates(icu::UnicodeString& hyphen, std::string& locale);
    static std::string trim(const std::string& src);
    static bool CompareBeginEnd(const std::string src, const std::string target, bool flag);
    icu::RegexPattern* GetPatternByKey(const std::string& key);

private:
    void Init(const std::string& locale);
    void RuleLevel();
    void InitRules(const std::string& xmlPath);
    void InitRuleBackup(std::string& xmlPathBackup);
    void LoadStrToStr(std::unordered_map<std::string, std::string>* map, xmlNodePtr cur);
    void LoadStrToPattern(std::unordered_map<std::string, icu::RegexPattern*>& map, xmlNodePtr cur);
    int GetLevel(std::string& name);
    std::string GetWithoutB(const std::string& ruleName);
    std::unordered_map<std::string, std::string> universeRules;
    std::unordered_map<std::string, std::string> localesRules;
    std::unordered_map<std::string, std::string> localesRulesBackup;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> subRulesMap;
    std::unordered_map<std::string, std::string> subRules;
    std::unordered_map<std::string, std::string> filterRules;
    std::unordered_map<std::string, std::string> pastRules;
    std::unordered_map<std::string, int> levels;
    std::unordered_map<std::string, std::string> delimiter;
    std::unordered_map<std::string, std::string> relDates;
    std::unordered_map<std::string, std::string> param;
    std::unordered_map<std::string, std::string> paramBackup;
    std::unordered_map<std::string, std::string> localeMap;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>*> loadMap;
    std::unordered_map<std::string, icu::RegexPattern*> patternsMap;
    std::string locale;
    static const std::string XML_COMMON_PATH;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif