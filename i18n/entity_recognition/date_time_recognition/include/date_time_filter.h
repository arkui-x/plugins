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
#ifndef OHOS_GLOBAL_DATE_TIME_FILTER_H
#define OHOS_GLOBAL_DATE_TIME_FILTER_H

#include <string>
#include <unicode/regex.h>
#include <vector>

#include "date_time_rule.h"
#include "matched_date_time_info.h"
#include "rules_engine.h"

namespace OHOS {
namespace Global {
namespace I18n {
class DateTimeFilter {
public:
    DateTimeFilter(const std::string& locale, DateTimeRule* dateTimeRule);
    ~DateTimeFilter();
    std::vector<MatchedDateTimeInfo> Filter(icu::UnicodeString& content, std::vector<MatchedDateTimeInfo>& matches,
        std::vector<MatchedDateTimeInfo>& clearMatches, std::vector<MatchedDateTimeInfo>& pastMatches);
    enum FilterType {
        TYPE_NULL = -1,
        TYPE_DATETIME = 0,
        TYPE_DATE = 1,
        TYPE_TIME = 2,
        TYPE_TIME_PERIOD = 3,
        TYPE_PERIOD = 4,
        TYPE_TODAY = 5,
        TYPE_WEEK = 6
    };
    enum DateCombine {
        NOT_COMBINE = 0,
        TWO_COMBINE = 1,
        ALL_COMBINE = 2,
    };
private:
    int GetType(std::string& name);
    std::vector<MatchedDateTimeInfo> FilterOverlay(std::vector<MatchedDateTimeInfo>& matches);
    std::vector<MatchedDateTimeInfo> FilterDatePeriod(icu::UnicodeString& content,
        std::vector<MatchedDateTimeInfo>& matches);
    std::vector<MatchedDateTimeInfo> FilterByRules(icu::UnicodeString& content,
        std::vector<MatchedDateTimeInfo>& matches, std::vector<MatchedDateTimeInfo>& clears);
    std::vector<MatchedDateTimeInfo> FilterByPast(icu::UnicodeString& content,
        std::vector<MatchedDateTimeInfo>& matches, std::vector<MatchedDateTimeInfo>& pasts);
    std::vector<MatchedDateTimeInfo> FilterOverlayFirst(std::vector<MatchedDateTimeInfo>& matches);
    std::vector<MatchedDateTimeInfo> FilterOverlaySecond(std::vector<MatchedDateTimeInfo>& matches);
    std::vector<MatchedDateTimeInfo> FilterDate(icu::UnicodeString& content,
        std::vector<MatchedDateTimeInfo>& matches);
    std::vector<MatchedDateTimeInfo> FilterDateTime(icu::UnicodeString& content,
        std::vector<MatchedDateTimeInfo>& matches);
    std::vector<MatchedDateTimeInfo> FilterPeriod(icu::UnicodeString& content,
        std::vector<MatchedDateTimeInfo>& matches);
    std::vector<MatchedDateTimeInfo> FilterDateTimePunc(icu::UnicodeString& content,
        std::vector<MatchedDateTimeInfo>& matches);
    bool DealBrackets(icu::UnicodeString& content, std::vector<MatchedDateTimeInfo>& matches,
        int matchIndex, MatchedDateTimeInfo& lastMatch,
        MatchedDateTimeInfo& currentMatch);
    int NestDealDate(icu::UnicodeString& content, MatchedDateTimeInfo& current,
        std::vector<MatchedDateTimeInfo>& matches, int preType);
    void DealMatchE(icu::UnicodeString& content, MatchedDateTimeInfo& nextMatch, MatchedDateTimeInfo& match);
    int GetResult(icu::UnicodeString& content, MatchedDateTimeInfo& current, MatchedDateTimeInfo& nextMatch,
        std::vector<MatchedDateTimeInfo>& matches);

    DateTimeRule* dateTimeRule;
    std::string locale;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif