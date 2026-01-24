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
#ifndef OHOS_GLOBAL_MATCHED_NUMBER_INFO_H
#define OHOS_GLOBAL_MATCHED_NUMBER_INFO_H

#include <unicode/regex.h>

namespace OHOS {
namespace Global {
namespace I18n {
class MatchedNumberInfo {
public:
    MatchedNumberInfo() : _begin(-1), _end(-1) {}
    MatchedNumberInfo(int begin, int end, icu::UnicodeString& content) : _begin(begin), _end(end), content(content) {}
    ~MatchedNumberInfo() {}

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

    void SetContent(icu::UnicodeString& content)
    {
        this->content = content;
    }

    icu::UnicodeString GetContent()
    {
        return content;
    }
    
    bool operator<(const MatchedNumberInfo& info) const
    {
        if (_begin < info._begin) {
            return true;
        } else if (_begin == info._begin && _end < info._end) {
            return true;
        } else if (_begin == info._begin && _end == info._end && content < info.content) {
            return true;
        }
        return false;
    }

private:
    int _begin;
    int _end;
    icu::UnicodeString content;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif