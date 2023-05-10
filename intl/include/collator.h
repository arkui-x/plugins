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
#ifndef GLOBAL_i18n_COLLATOR_H
#define GLOBAL_i18n_COLLATOR_H

#include <string>
#include <map>
#include <vector>
#include <set>

#include "locale_info.h"
#include "memory"
#include "unicode/coll.h"
#include "unicode/locid.h"
#include "utypes.h"

namespace OHOS {
namespace Global {
namespace I18n {
typedef enum CompareResult {
    INVALID = -2,
    SMALLER,
    EQUAL,
    GREATER,
} CompareResult;

class Collator {
public:
    Collator(std::vector<std::string> &localeTags, std::map<std::string, std::string> &options);
    ~Collator();
    CompareResult Compare(const std::string &first, const std::string &second);
    void ResolvedOptions(std::map<std::string, std::string> &options);

private:
    std::string localeStr;
    std::string localeMatcher;
    std::string usage;
    std::string sensitivity;
    std::string ignorePunctuation;
    std::string numeric;
    std::string caseFirst;
    std::string collation;

    std::unique_ptr<LocaleInfo> localeInfo = nullptr;
    icu::Locale locale;
    icu::Collator *collatorPtr = nullptr;
    bool createSuccess = false;

    std::set<std::string> GetValidLocales();
    std::string ParseOption(std::map<std::string, std::string> &options, const std::string &key);
    void ParseAllOptions(std::map<std::string, std::string> &options);
    bool IsValidCollation(std::string &collation);
    void SetCollation();
    void SetUsage();
    void SetNumeric();
    void SetCaseFirst();
    void SetSensitivity();
    void SetIgnorePunctuation();
    bool InitCollator();
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif
