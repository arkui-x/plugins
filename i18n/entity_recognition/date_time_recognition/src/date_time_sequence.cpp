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
#include "date_time_sequence.h"

#include <regex>
#include "log.h"
#include "unicode/datefmt.h"
#include "unicode/locid.h"
#include "unicode/smpdtfmt.h"
#include "unicode/dtptngen.h"

namespace OHOS {
namespace Global {
namespace I18n {
const int ARRAY_LENGTH_THREE = 3;
const int ARRAY_LENGTH_FOUR = 4;
const int ORDER_MINUS_SIZE = 2;
const int MAX_ARRAY_SIZE = 6;
const int ORDER_TWO = 2;
const int ORDER_THREE = 3;

const std::unordered_map<std::string, std::string> DateTimeSequence::DATE_ORDER_MAP = {
    { "ug", "L-d-y" },
    { "ar", "y-L-d" },
    { "fa", "y-L-d" },
    { "ur", "y-L-d" },
    { "iw", "y-L-d" },
    { "he", "y-L-d" },
};

std::string DateTimeSequence::GetDateOrder(const std::string& locale)
{
    UErrorCode icuStatus = U_ZERO_ERROR;
    icu::Locale localeObj = icu::Locale::forLanguageTag(locale.data(), icuStatus);
    if (U_FAILURE(icuStatus)) {
        LOGE("Failed to create locale for GetDateOrder");
        return "";
    }
    const char* language = localeObj.getBaseName();
    if (language == nullptr) {
        LOGE("%{public}s getBaseName is null.", locale.c_str());
        return "";
    }
    std::string languageTag = language;
    if (DATE_ORDER_MAP.find(languageTag) != DATE_ORDER_MAP.end()) {
        std::string dateOrder = DATE_ORDER_MAP.find(languageTag)->second;
        return dateOrder;
    }
    icu::SimpleDateFormat* formatter = static_cast<icu::SimpleDateFormat*>
        (icu::DateFormat::createDateInstance(icu::DateFormat::EStyle::kDefault, localeObj));
    if (U_FAILURE(icuStatus) || formatter == nullptr) {
        LOGE("Failed to create SimpleDateFormat");
        return "";
    }
    std::string tempValue;
    icu::UnicodeString unistr;
    formatter->toPattern(unistr);
    unistr.toUTF8String<std::string>(tempValue);
    std::string value = ModifyOrder(tempValue);
    delete formatter;
    return value;
}

std::string DateTimeSequence::ModifyOrder(std::string& pattern)
{
    int order[ARRAY_LENGTH_THREE] = { 0 };
    int lengths[ARRAY_LENGTH_FOUR] = { 0 };
    bool flag = true;
    for (size_t i = 0; i < pattern.length(); ++i) {
        char ch = pattern[i];
        if (flag && std::isalpha(ch)) {
            ProcessNormal(ch, order, ARRAY_LENGTH_THREE, lengths, ARRAY_LENGTH_FOUR);
        } else if (ch == '\'') {
            flag = !flag;
        }
    }
    std::unordered_map<char, int> pattern2index = {
        { 'y', 1 },
        { 'L', 2 },
        { 'd', 3 },
    };
    std::string ret;
    for (int i = 0; i < ARRAY_LENGTH_THREE; ++i) {
        auto it = pattern2index.find(order[i]);
        if (it == pattern2index.end()) {
            continue;
        }
        int index = it->second;
        if ((lengths[index] > 0) && (lengths[index] <= MAX_ARRAY_SIZE)) {
            ret.append(lengths[index], order[i]);
        }
        if (i < ORDER_MINUS_SIZE) {
            ret.append(1, '-');
        }
    }
    return ret;
}

void DateTimeSequence::ProcessNormal(char ch, int* order, size_t orderSize, int* lengths, size_t lengsSize)
{
    char adjust;
    int index = -1;
    if (ch == 'd') {
        adjust = 'd';
        index = ORDER_THREE;
    } else if ((ch == 'L') || (ch == 'M')) {
        adjust = 'L';
        index = ORDER_TWO;
    } else if (ch == 'y') {
        adjust = 'y';
        index = 1;
    } else {
        return;
    }
    if ((index < 0) || (index >= static_cast<int>(lengsSize))) {
        return;
    }
    if (lengths[index] == 0) {
        if (lengths[0] >= ORDER_THREE) {
            return;
        }
        order[lengths[0]] = static_cast<int>(adjust);
        ++lengths[0];
        lengths[index] = 1;
    } else {
        ++lengths[index];
    }
}
} // namespace I18n
} // namespace Global
} // namespace OHOS