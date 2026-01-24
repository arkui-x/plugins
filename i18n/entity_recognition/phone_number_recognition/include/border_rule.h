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
#ifndef OHOS_GLOBAL_I18N_BORDER_RULE_H
#define OHOS_GLOBAL_I18N_BORDER_RULE_H

#include <unicode/regex.h>
#include "phonenumbers/phonenumbermatch.h"

namespace OHOS {
namespace Global {
namespace I18n {
using i18n::phonenumbers::PhoneNumberMatch;

class BorderRule {
public:
    BorderRule(icu::UnicodeString& regex, std::string& insensitive, std::string& type);
    int GetType();
    icu::RegexPattern* GetPattern();
    bool Handle(PhoneNumberMatch* match, icu::UnicodeString& message);

private:
    int type;
    icu::UnicodeString regex;
    std::string insensitive;
    static const std::string CONTAIN_STR;
    static const std::string CONTAIN_OR_INTERSECT_STR;
    static const std::string TRUE_STR;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif