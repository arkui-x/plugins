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
#include "negative_rule.h"
#include "log.h"

namespace OHOS {
namespace Global {
namespace I18n {
NegativeRule::NegativeRule(icu::UnicodeString& regex, std::string& insensitive)
{
    this->regex = regex;
    this->insensitive = insensitive;
}

icu::RegexPattern* NegativeRule::GetPattern()
{
    UErrorCode status = U_ZERO_ERROR;
    icu::RegexPattern* pattern;
    if (this->insensitive == "True") {
        pattern = icu::RegexPattern::compile(this->regex, URegexpFlag::UREGEX_CASE_INSENSITIVE, status);
    } else {
        pattern = icu::RegexPattern::compile(this->regex, 0, status);
    }
    if (U_FAILURE(status)) {
        LOGE("NegativeRule::GetPattern: Compile regex pattern failed.");
        return nullptr;
    }
    return pattern;
}

void NegativeRule::ReplaceSpecifiedPos(icu::UnicodeString& chs, int start, int end)
{
    if (start < end) {
        int len = chs.length();
        for (int i = 0; i < len; i++) {
            if (i >= start && i < end) {
                chs.replace(i, 1, 'A');
            }
        }
    }
}

icu::UnicodeString NegativeRule::Handle(icu::UnicodeString& src)
{
    icu::UnicodeString ret = src;
    icu::RegexPattern* pattern = GetPattern();
    if (pattern == nullptr) {
        LOGE("NegativeRule::Handle: pattern is nullptr.");
        return ret;
    }
    UErrorCode status = U_ZERO_ERROR;
    icu::RegexMatcher* matcher = pattern->matcher(ret, status);
    while (matcher != nullptr && matcher->find(status)) {
        int start = matcher->start(status);
        int end = matcher->end(status);
        ReplaceSpecifiedPos(ret, start, end);
    }
    if (matcher != nullptr) {
        delete matcher;
    }
    delete pattern;
    return ret;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS