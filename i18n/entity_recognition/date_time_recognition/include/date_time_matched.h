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
#ifndef OHOS_GLOBAL_DATE_TIME_MATCHED_H
#define OHOS_GLOBAL_DATE_TIME_MATCHED_H

#include <filesystem>
#include <string>
#include <vector>
#include "date_rule_init.h"

namespace OHOS {
namespace Global {
namespace I18n {
class DateTimeMatched {
public:
    DateTimeMatched(std::string& locale);
    ~DateTimeMatched();
    std::vector<int> GetMatchedDateTime(icu::UnicodeString& message);

private:
    std::unique_ptr<DateRuleInit> dateRuleInit;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif