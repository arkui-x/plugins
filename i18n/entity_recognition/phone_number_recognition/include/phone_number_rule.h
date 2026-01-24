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
#ifndef OHOS_GLOBAL_PHONE_NUMBER_RULE_H
#define OHOS_GLOBAL_PHONE_NUMBER_RULE_H

#include <string>
#include <unicode/regex.h>
#include <vector>

#include "border_rule.h"
#include "code_rule.h"
#include "find_rule.h"
#include "libxml/globals.h"
#include "libxml/tree.h"
#include "libxml/xmlstring.h"
#include "negative_rule.h"
#include "positive_rule.h"
#include "regex_rule.h"

namespace OHOS {
namespace Global {
namespace I18n {
class PhoneNumberRule {
public:
    PhoneNumberRule(std::string& country);
    ~PhoneNumberRule();
    void Init();
    
    std::vector<BorderRule*> GetBorderRules();
    std::vector<CodeRule*> GetCodesRules();
    std::vector<PositiveRule*> GetPositiveRules();
    std::vector<NegativeRule*> GetNegativeRules();
    std::vector<FindRule*> GetFindRules();
    bool isFixed;

private:
    void InitRule(const std::string& xmlPath);
    void SetRules(std::string& category, icu::UnicodeString& content, std::string& valid,
        std::string& handle, std::string& insensitive, std::string& type);
    void ParseXmlNode(xmlNodePtr cur, std::string& category);
    std::string XmlNodePtrToString(xmlNodePtr valuePtr);
    bool IsXmlNodeValueEmpty(const std::string& insensitive, const std::string& type,
        const std::string& valid, const std::string& handle);

    std::vector<NegativeRule*> negativeRules;
    std::vector<PositiveRule*> positiveRules;
    std::vector<BorderRule*> borderRules;
    std::vector<CodeRule*> codesRules;
    std::vector<FindRule*> findRules;
    std::string country;
    std::string xmlPath;
    bool commonExit;
    static const std::string XML_COMMON_PATH;
    static const std::string XML_COMMON_FILE;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif