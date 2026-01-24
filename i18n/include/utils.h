/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except", "in compliance with the License.
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
#ifndef OHOS_GLOBAL_I18N_UTILS_H
#define OHOS_GLOBAL_I18N_UTILS_H

#include <regex>
#include <string>
#include <unicode/locid.h>
#include <unordered_set>
#include <vector>

namespace OHOS {
namespace Global {
namespace I18n {
void Split(const std::string &src, const std::string &sep, std::vector<std::string> &dest);
int32_t ConvertString2Int(const std::string &numberStr, int32_t& status);
void GetAllValidLocalesTag(std::unordered_set<std::string>& allValidLocalesLanguageTag);
bool IsValidLocaleTag(icu::Locale &locale);
void Split(const std::string& src, const std::string& sep, std::unordered_set<std::string>& dest);
std::string GetAbsoluteFilePath(const std::string& filePath);
std::string StrReplaceAll(const std::string& str,
    const std::string& target, const std::string& replace);
bool Eq(double a, double b);
bool Geq(double a, double b);
bool Leq(double a, double b);
bool FileExist(const std::string& path);
std::string trim(std::string& s);
std::string GetISO3Language(const std::string& language);
std::string GetISO3Country(const std::string& country);
void Merge(const std::vector<std::string>& src, const std::string& sep, std::string& dest);
bool RegexSearchNoExcept(const std::string& str, std::smatch& match, const std::regex& regex);
std::string HexToStr(const std::string& hex);
std::string GetAppDataModuleDir();
void I18nSetArkUIXIcuDirectory();
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif