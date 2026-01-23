/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#include "utils.h"

#include <cerrno>
#include <climits>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>

#include "locale_config.h"
#include "localebuilder.h"
#include "log.h"
#include "ohos/init_data.h"
#include "stage_asset_manager.h"

namespace OHOS {
namespace Global {
namespace I18n {
using namespace std;

const size_t HEX_PAIR_LENGTH = 2;
const int32_t HEX_BASE = 16;
const double DOUBLE_EPSILON = 1e-6;

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

void GetAllValidLocalesTag(std::unordered_set<std::string>& allValidLocalesLanguageTag)
{
    static bool init = false;
    if (init) {
        return;
    }
    int32_t validCount = 1;
    const icu::Locale *validLocales = icu::Locale::getAvailableLocales(validCount);
    for (int i = 0; i < validCount; i++) {
        allValidLocalesLanguageTag.insert(validLocales[i].getLanguage());
    }
    init = true;
}

bool IsValidLocaleTag(icu::Locale &locale)
{
    static std::unordered_set<std::string> allValidLocalesLanguageTag;
    GetAllValidLocalesTag(allValidLocalesLanguageTag);
    std::string languageTag = locale.getLanguage();
    if (allValidLocalesLanguageTag.find(languageTag) == allValidLocalesLanguageTag.end()) {
        LOGE("GetTimePeriodName does not support this languageTag: %{public}s", languageTag.c_str());
        return false;
    }
    return true;
}

std::string GetAppDataModuleDir()
{
    #ifdef ANDROID_PLATFORM
        std::string appDataModuleDir =
            OHOS::AbilityRuntime::Platform::StageAssetManager::GetInstance()->GetAppDataModuleDir();
    #else
        std::string appDataModuleDir =
            OHOS::AbilityRuntime::Platform::StageAssetManager::GetInstance()->GetBundleCodeDir();
    #endif
    appDataModuleDir = appDataModuleDir + "/" + "systemres";
    return appDataModuleDir;
}

void I18nSetArkUIXIcuDirectory()
{
    std::string appDataModuleDir = GetAppDataModuleDir();
    LOGI("appDataModuleDir: %{public}s", appDataModuleDir.c_str());
    SetArkuiXIcuDirectory(appDataModuleDir.c_str());
}

void Split(const std::string& src, const std::string& sep, std::unordered_set<std::string>& dest)
{
    dest.clear();
    if (src.empty()) {
        return;
    }
    std::string::size_type begin = 0;
    std::string::size_type end = src.find(sep);
    while (end != string::npos) {
        dest.insert(src.substr(begin, end - begin));
        begin = end + sep.size();
        end = src.find(sep, begin);
    }
    if (begin != src.size()) {
        dest.insert(src.substr(begin));
    }
}

std::string GetAbsoluteFilePath(const std::string& filePath)
{
    if (filePath.empty()) {
        LOGE("utils::GetAbsoluteFilePath: input param filePath is empty");
        return "";
    }
    std::vector<char> resolvedPath(PATH_MAX + 1);
    if (realpath(filePath.c_str(), resolvedPath.data()) == nullptr) {
        LOGE("utils::GetAbsoluteFilePath realpath error, error message: %{public}s.", strerror(errno));
        return "";
    }
    std::ifstream file(resolvedPath.data());
    if (!file.good()) {
        LOGE("utils::GetAbsoluteFilePath file open is not good.");
        return "";
    }
    return std::string(resolvedPath.data());
}

std::string StrReplaceAll(const std::string& str,
    const std::string& target, const std::string& replace)
{
    std::string::size_type pos = 0;
    std::string result = str;
    if (replace.empty() || target.compare(replace) == 0) {
        return result;
    }
    while ((pos = result.find(target)) != std::string::npos) {
        result.replace(pos, target.length(), replace);
    }
    return result;
}

bool Eq(double a, double b)
{
    return fabs(a - b) < DOUBLE_EPSILON;
}

bool Geq(double a, double b)
{
    return a > b || Eq(a, b);
}

bool Leq(double a, double b)
{
    return a < b || Eq(a, b);
}

bool FileExist(const std::string& path)
{
    int isFileExist = access(path.c_str(), F_OK);
    bool resultFileExist = isFileExist == -1 ? false : true;
    return resultFileExist;
}

std::string trim(std::string& s)
{
    if (s.empty()) {
        return s;
    }
    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

std::string GetISO3Language(const std::string& language)
{
    UErrorCode icuStatus = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(language.data(), icuStatus);
    if (U_FAILURE(icuStatus) || !IsValidLocaleTag(locale)) {
        return "";
    }
    return locale.getISO3Language();
}

std::string GetISO3Country(const std::string& country)
{
    UErrorCode icuStatus = U_ZERO_ERROR;
    icu::Locale locale;
    if (LocaleConfig::IsValidRegion(country)) {
        locale = icu::LocaleBuilder().setLanguage("zh").setRegion(country).build(icuStatus);
    } else if (LocaleConfig::IsValidTag(country)) {
        locale = icu::Locale::forLanguageTag(country.data(), icuStatus);
    } else {
        return "";
    }
    if (U_FAILURE(icuStatus) || !IsValidLocaleTag(locale)) {
        return "";
    }
    return locale.getISO3Country();
}

void Merge(const std::vector<std::string>& src, const std::string& sep, std::string& dest)
{
    if (src.size() == 0) {
        dest = "";
        return;
    }
    dest = src[0];
    for (size_t i = 1; i < src.size(); ++i) {
        dest += sep;
        dest += src[i];
    }
}

bool RegexSearchNoExcept(const std::string& str, std::smatch& match, const std::regex& regex)
{
    try {
        return std::regex_search(str, match, regex);
    } catch (const std::regex_error &except) {
        LOGE("RegexSearchNoExcept: regex_error caught %{public}s.", except.what());
        return false;
    }
}

std::string HexToStr(const std::string& hex)
{
    size_t len = HEX_PAIR_LENGTH;
    if (hex.length() % len != 0) {
        return "";
    }

    bool flag = true;
    std::string result;
    for (size_t i = 0; i < hex.length(); i += len) {
        std::string hexStr = hex.substr(i, len);
        if (flag && hexStr.compare("00") == 0) {
            continue;
        }
        flag = false;
        int32_t value = 0;
        try {
            value = std::stoi(hexStr, nullptr, HEX_BASE);
        } catch (const std::invalid_argument &except) {
            LOGE("HexToStr: invalid_argument, hexStr: %{public}s.", hexStr.c_str());
            return "";
        } catch (const std::out_of_range &except) {
            LOGE("HexToStr: out_of_range, hexStr: %{public}s.", hexStr.c_str());
            return "";
        } catch (...) {
            LOGE("HexToStr: unknow error, hexStr: %{public}s.", hexStr.c_str());
            return "";
        }
        result += char(value);
    }
    return result;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS
