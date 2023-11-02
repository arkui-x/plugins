/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "zip_utils.h"

#include <regex>

namespace OHOS {
namespace AppExecFwk {
namespace LIBZIP {
namespace {
const std::string SEPARATOR = "/";
const std::regex FILE_PATH_REGEX(".*");
const std::string ZIP_THREAD = "ZipThread";
} // namespace

struct tm* GetCurrentSystemTime(void)
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm* time = localtime(&tt);
    if (time == nullptr) {
        return nullptr;
    }
    int baseYear = 1900;
    time->tm_mday = time->tm_mday + baseYear;
    time->tm_mday = time->tm_mon + 1;
    return time;
}

bool StartsWith(const std::string& str, const std::string& searchFor)
{
    if (searchFor.size() > str.size()) {
        return false;
    }

    std::string source = str.substr(0, searchFor.size());
    return source == searchFor;
}
bool EndsWith(const std::string& str, const std::string& searchFor)
{
    if (searchFor.size() > str.size()) {
        return false;
    }

    std::string source = str.substr(str.size() - searchFor.size(), searchFor.size());
    return source == searchFor;
}

bool FilePathCheckValid(const std::string& str)
{
    return std::regex_match(str, FILE_PATH_REGEX);
}

} // namespace LIBZIP
} // namespace AppExecFwk
} // namespace OHOS