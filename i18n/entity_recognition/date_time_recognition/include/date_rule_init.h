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
#ifndef OHOS_GLOBAL_DATE_RULE_INIT_H
#define OHOS_GLOBAL_DATE_RULE_INIT_H

#include <string>
#include <unicode/regex.h>
#include <unordered_map>
#include <vector>

#include "date_time_filter.h"
#include "date_time_rule.h"
#include "matched_date_time_info.h"
#include "rules_engine.h"

namespace OHOS {
namespace Global {
namespace I18n {
class DateRuleInit {
public:
    DateRuleInit(const std::string& locale);
    ~DateRuleInit();
    std::vector<MatchedDateTimeInfo> Detect(icu::UnicodeString& message);
private:
    void Init();
    std::vector<MatchedDateTimeInfo> GetMatches(icu::UnicodeString& message);
    std::vector<MatchedDateTimeInfo> ClearFind(icu::UnicodeString& message);
    std::vector<MatchedDateTimeInfo> PastFind(icu::UnicodeString& message);
    void GetMatchedInfo(std::vector<MatchedDateTimeInfo>& matches, MatchedDateTimeInfo& match,
        icu::UnicodeString& message);

    std::string locale;
    DateTimeRule* dateTimeRule = nullptr;
    DateTimeFilter* filter = nullptr;
    std::vector<RulesEngine> universalAndLocaleRules;
    std::unordered_map<std::string, RulesEngine> subDetectsMap;
    RulesEngine clearRulesEngine;
    RulesEngine pastRulesEngine;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif