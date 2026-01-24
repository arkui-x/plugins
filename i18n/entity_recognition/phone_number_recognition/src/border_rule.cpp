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
#include "border_rule.h"

#include "log.h"
#include "phone_number_matched.h"

namespace OHOS {
namespace Global {
namespace I18n {
const std::string BorderRule::CONTAIN_STR = "CONTAIN";
const std::string BorderRule::CONTAIN_OR_INTERSECT_STR = "CONTAIN_OR_INTERSECT";
const std::string BorderRule::TRUE_STR = "True";
const int TYPE_CONTAIN = 9;
const int TYPE_CONTAIN_OR_INTERSECT = 8;
const int CHARACTERS_SIZE = 10;
BorderRule::BorderRule(icu::UnicodeString& regex, std::string& insensitive, std::string& type)
{
    this->regex = regex;
    if (type.compare(CONTAIN_STR) == 0) {
        this->type = TYPE_CONTAIN;
    } else if (type.compare(CONTAIN_OR_INTERSECT_STR) == 0) {
        this->type = TYPE_CONTAIN_OR_INTERSECT;
    } else {
        this->type = 0;
    }
    this->insensitive = insensitive;
}

int BorderRule::GetType()
{
    return type;
}

icu::RegexPattern* BorderRule::GetPattern()
{
    UErrorCode status = U_ZERO_ERROR;
    icu::RegexPattern* pattern;
    if (insensitive.compare(TRUE_STR) == 0) {
        pattern = icu::RegexPattern::compile(this->regex, URegexpFlag::UREGEX_CASE_INSENSITIVE, status);
    } else {
        pattern = icu::RegexPattern::compile(this->regex, 0, status);
    }
    if (U_FAILURE(status)) {
        LOGE("BorderRule::GetPattern: Compile regex pattern failed.");
        return nullptr;
    }
    return pattern;
}

bool BorderRule::Handle(PhoneNumberMatch* match, icu::UnicodeString& message)
{
    int begin = match->start();
    int end = match->end();
    int beginSubTen = begin - CHARACTERS_SIZE < 0 ? 0 : begin - CHARACTERS_SIZE;
    int endAddTen = end + CHARACTERS_SIZE > message.length() ? message.length() : end + CHARACTERS_SIZE;
    icu::UnicodeString borderStr = message.tempSubString(beginSubTen, endAddTen - beginSubTen);
        
    icu::RegexPattern* pattern = this->GetPattern();
    if (pattern == nullptr) {
        return false;
    }
    UErrorCode status = U_ZERO_ERROR;
    icu::RegexMatcher* mat = pattern->matcher(borderStr, status);
    if (U_FAILURE(status) || mat == nullptr) {
        LOGE("BorderRule::Handle: Pattern matcher failed.");
        delete pattern;
        return false;
    }
    int type = this->GetType();
    while (mat->find()) {
        int borderBegin = mat->start(status) + beginSubTen;
        int borderEnd = mat->end(status) + beginSubTen;
        bool isDel = false;
        if (type == PhoneNumberMatched::CONTAIN && borderBegin <= begin && end <= borderEnd) {
            isDel = true;
        } else if (type == PhoneNumberMatched::CONTAIN_OR_INTERSECT && ((borderBegin <= begin &&
            end <= borderEnd) || (borderBegin < begin && begin < borderEnd && borderEnd < end) ||
            (begin < borderBegin && borderBegin < end && end < borderEnd))) {
            isDel = true;
        }
        if (isDel) {
            delete mat;
            delete pattern;
            return false;
        }
    }
    delete mat;
    delete pattern;
    return true;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS