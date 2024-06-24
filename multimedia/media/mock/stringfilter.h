/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef HISYSEVENT_STRING_FILTER_H
#define HISYSEVENT_STRING_FILTER_H

#include <string>

namespace OHOS {
namespace HiviewDFX {
class StringFilter {
public:
    StringFilter();
    ~StringFilter() {}
    // Transform special char to escaped form ("lookup table" method)
    std::string EscapeToRaw(const std::string &text);
    // Check lexical ("finite state machine" method)
    bool IsValidName(const std::string &text, unsigned int maxSize);
    static StringFilter& GetInstance();

private:
    static constexpr int CHAR_RANGE = 128;
    static constexpr int MAP_STR_LEN = 3;

    static constexpr int STATE_NUM = 2;
    static constexpr int STATE_BEGIN = 0;
    static constexpr int STATE_RUN = 1;
    static constexpr int STATE_STOP = -1;

    static char charTab_[CHAR_RANGE][MAP_STR_LEN];
    static int statTab_[STATE_NUM][CHAR_RANGE];
    static StringFilter filter_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HISYSEVENT_STRING_FILTER_H
