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
#ifndef OHOS_GLOBAL_MATCHED_DATE_TIME_INFO_H
#define OHOS_GLOBAL_MATCHED_DATE_TIME_INFO_H

#include <string>
#include <unicode/regex.h>
#include "utils.h"

namespace OHOS {
namespace Global {
namespace I18n {
const int TIME_PERIOD_RULE_LOWER_BOUND = 49999;
const int TIME_PERIOD_RULE_UPPER_BOUND = 60000;
class MatchedDateTimeInfo {
public:
    MatchedDateTimeInfo() : _begin(-1), _end(-1), type(0), isTimePeriod(false) {}
    MatchedDateTimeInfo(int begin, int end, std::string& regex)
        : _begin(begin), _end(end), regex(regex), type(0), isTimePeriod(false) {}
    ~MatchedDateTimeInfo() {}

    void SetIsTimePeriod(bool is)
    {
        isTimePeriod = is;
    }

    bool IsTimePeriod()
    {
        if (isTimePeriod) {
            return isTimePeriod;
        }
        if (regex.empty()) {
            return false;
        }
        int32_t status = 0;
        int key = ConvertString2Int(regex, status);
        return key > TIME_PERIOD_RULE_LOWER_BOUND && key < TIME_PERIOD_RULE_UPPER_BOUND;
    }

    void SetBegin(int begin)
    {
        this->_begin = begin;
    }

    int GetBegin()
    {
        return _begin;
    }

    void SetEnd(int end)
    {
        this->_end = end;
    }

    int GetEnd()
    {
        return _end;
    }

    std::string GetRegex()
    {
        return regex;
    }

    int GetType()
    {
        return type;
    }

    void SetType(int type)
    {
        this->type = type;
    }

    bool operator<(const MatchedDateTimeInfo& info) const
    {
        if (_begin < info._begin) {
            return true;
        }
        return false;
    }

private:
    int32_t _begin;
    int32_t _end;
    std::string regex;
    int32_t type;
    bool isTimePeriod;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif