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
#include "date_time_matched.h"
#include "log.h"

namespace OHOS {
namespace Global {
namespace I18n {
const size_t POS_DATA_TIME = 2;
const size_t POS_END = 2;
DateTimeMatched::DateTimeMatched(std::string& locale)
{
    dateRuleInit = std::make_unique<DateRuleInit>(locale);
    if (dateRuleInit == nullptr) {
        LOGE("DateRuleInit construct failed.");
    }
}

DateTimeMatched::~DateTimeMatched()
{
}

std::vector<int> DateTimeMatched::GetMatchedDateTime(icu::UnicodeString& message)
{
    icu::UnicodeString messageStr = message;
    std::vector<int> result {0};
    if (this->dateRuleInit == nullptr) {
        LOGE("GetMatchedDateTime failed because this->dateRuleInit is nullptr.");
        return result;
    }
    std::vector<MatchedDateTimeInfo> matches = this->dateRuleInit->Detect(messageStr);
    size_t length = matches.size();
    if (length > 0) {
        size_t posBegin = 1;
        result.resize(POS_DATA_TIME * length + 1);
        result[0] = static_cast<int>(length);
        for (size_t i = 0; i < length; i++) {
            result[i * POS_DATA_TIME + posBegin] = matches[i].GetBegin();
            result[i * POS_DATA_TIME + POS_END] = matches[i].GetEnd();
        }
    }
    return result;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS