/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <stdexcept>
#include <string>
#include <vector>
#include "utils.h"

namespace OHOS {
namespace Global {
namespace I18n {
using namespace std;
void Split(const string &src, const string &sep, vector<string> &dest)
{
    if (src == "") {
        return;
    }
    string::size_type begin = 0;
    string::size_type end = src.find(sep);
    while (end != string::npos) {
        dest.push_back(src.substr(begin, end - begin));
        begin = end + sep.size();
        end = src.find(sep, begin);
    }
    if (begin != src.size()) {
        dest.push_back(src.substr(begin));
    }
}

int32_t ConvertString2Int(const string &numberStr, int32_t& status)
{
    if (numberStr.empty()) {
        status = -1;
        return -1;
    }
    try {
        return std::stoi(numberStr);
    } catch(const std::invalid_argument& except) {
        status = -1;
        return -1;
    } catch (const std::out_of_range& except) {
        status = -1;
        return -1;
    }
}
} // namespace I18n
} // namespace Global
} // namespace OHOS