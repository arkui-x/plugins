/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef GLOBAL_I18N_PLURAL_RULES_H
#define GLOBAL_I18N_PLURAL_RULES_H

#include <vector>
#include <string>
#include <map>
#include <set>

#include "locale_info.h"
#include "memory"
#include "unicode/locid.h"
#include "unicode/numberformatter.h"
#include "unicode/plurrule.h"

namespace OHOS {
namespace Global {
namespace I18n {
class PluralRules {
public:
    PluralRules(std::vector<std::string> &localeTags, std::map<std::string, std::string> &options);
    ~PluralRules();
    std::string Select(double number);

private:
    std::string localeStr;
    std::unique_ptr<LocaleInfo> localeInfo = nullptr;
    icu::Locale locale;
    icu::PluralRules *pluralRules = nullptr;
    icu::number::LocalizedNumberFormatter numberFormatter;

    std::string localeMatcher;
    std::string type;
    int minInteger;
    int minFraction;
    int maxFraction;
    int minSignificant;
    int maxSignificant;
    bool createSuccess = false;

    std::set<std::string> GetValidLocales();
    std::string ParseOption(std::map<std::string, std::string> &options, const std::string &key);
    void ParseAllOptions(std::map<std::string, std::string> &options);
    void InitPluralRules(std::vector<std::string> &localeTags, std::map<std::string, std::string> &options);
    void InitNumberFormatter();
    int GetValidInteger(std::string &integerStr, int minValue, int maxValue, int defaultValue);
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif
