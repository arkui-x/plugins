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
 * See the License for the specific locale governing permissions and
 * limitations under the License.
 */
#include "date_time_rule.h"

#include "log.h"
#include "utils.h"

namespace OHOS {
namespace Global {
namespace I18n {
const int LEVEL_FIRST = 1;
const int LEVEL_SECOND = 2;
const int LEVEL_THIRD = 3;
const int SIZE_THREE = 3;
const int RULE_LOWER_THRESHOLD_1 = 9999;
const int RULE_UPPER_THRESHOLD_1 = 20000;
const int RULE_LOWER_THRESHOLD_2 = 19999;
const int RULE_UPPER_THRESHOLD_2 = 40000;
const int BASE_LEVEL_LOW = 10;
const int BASE_LEVEL_MID = 20;
const int BASE_LEVEL_HIGH = 30;
const std::string DateTimeRule::XML_COMMON_PATH = "/intl/entity_recognition/etc/datetime/common.xml";

DateTimeRule::DateTimeRule(const std::string& locale)
{
    this->loadMap = {{"sub_rules", &this->subRules},
                     {"universe_rules", &this->universeRules},
                     {"filter_rules", &this->filterRules},
                     {"past_rules", &this->pastRules},
                     {"locale_rules", &this->localesRules},
                     {"delimiter", &this->delimiter},
                     {"param", &this->param},
                     {"default_locale", &this->localeMap},
                     {"isRelDates", &this->relDates}};
    Init(locale);
}

DateTimeRule::~DateTimeRule()
{
    std::unordered_map<std::string, icu::RegexPattern*>::iterator iter;
    for (iter = patternsMap.begin(); iter != patternsMap.end(); ++iter) {
        icu::RegexPattern* pattern = iter->second;
        if (pattern != nullptr) {
            delete pattern;
        }
        pattern = nullptr;
    }
}

void DateTimeRule::Init(const std::string& locale)
{
    std::string appModuleDir = GetAppDataModuleDir();
    InitRules(appModuleDir + XML_COMMON_PATH);
    std::string xmlPath = appModuleDir + "/intl/entity_recognition/etc/datetime/" + locale + ".xml";
    std::string validXmlPath = GetAbsoluteFilePath(xmlPath);
    if (validXmlPath.empty()) {
        this->locale = this->localeMap["locale"];
    } else {
        this->locale = locale;
    }
    xmlPath = appModuleDir + "/intl/entity_recognition/etc/datetime/" + this->locale + ".xml";
    InitRules(xmlPath);
    std::string xmlPathBackup = appModuleDir + "/intl/entity_recognition/etc/datetime/" +
        this->localeMap["backup"] + ".xml";
    InitRuleBackup(xmlPathBackup);
    RuleLevel();
}

std::string DateTimeRule::GetLocale()
{
    return this->locale;
}

void DateTimeRule::RuleLevel()
{
    std::string ruleName = "mark_ShortDateLevel";
    std::string shortDateMark = GetWithoutB(ruleName);
    if (shortDateMark == "ymd") {
        levels["20016"] = LEVEL_FIRST;
        levels["20014"] = LEVEL_THIRD;
        levels["20015"] = LEVEL_SECOND;
    } else if (shortDateMark == "mdy") {
        levels["20016"] = LEVEL_SECOND;
        levels["20014"] = LEVEL_THIRD;
        levels["20015"] = LEVEL_FIRST;
    }
}

void DateTimeRule::InitRules(const std::string& xmlPath)
{
    std::string validXmlPath = GetAbsoluteFilePath(xmlPath);
    if (validXmlPath.empty()) {
        LOGE("DateTimeRule::InitRules: invalid xmlPath: %{public}s.", xmlPath.c_str());
        return;
    }
    xmlKeepBlanksDefault(0);
    xmlDocPtr doc = xmlParseFile(validXmlPath.c_str());
    if (doc == nullptr) {
        return;
    }
    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (root == nullptr) {
        xmlFreeDoc(doc);
        return;
    }
    xmlNodePtr cur = root->xmlChildrenNode;
    while (cur != nullptr) {
        std::string category = reinterpret_cast<const char*>(cur->name);
        if (category == "sub_rules_map") {
            xmlNodePtr value = cur->xmlChildrenNode;
            while (value != nullptr) {
                std::string key = reinterpret_cast<const char*>(value->name);
                key = key.substr(SIZE_THREE);
                xmlNodePtr subValue = value->xmlChildrenNode;
                std::unordered_map<std::string, std::string> tempMap;
                LoadStrToStr(&tempMap, subValue);
                subRulesMap[key] = tempMap;
                value = value->next;
            }
        } else if (this->loadMap.find(category) != this->loadMap.end()) {
            xmlNodePtr valueNext = cur->xmlChildrenNode;
            LoadStrToStr(this->loadMap[category], valueNext);
        } else if (category == "pattern") {
            xmlNodePtr valueNext = cur->xmlChildrenNode;
            LoadStrToPattern(this->patternsMap, valueNext);
        }
        cur = cur->next;
    }
    xmlFreeDoc(doc);
}

void DateTimeRule::InitRuleBackup(std::string& xmlPathBackup)
{
    std::string validXmlPath = GetAbsoluteFilePath(xmlPathBackup);
    if (validXmlPath.empty()) {
        LOGE("DateTimeRule::InitRuleBackup: invalid xmlPathBackup: %{public}s.", xmlPathBackup.c_str());
        return;
    }
    xmlKeepBlanksDefault(0);
    xmlDocPtr doc = xmlParseFile(validXmlPath.c_str());
    if (doc == nullptr) {
        return;
    }
    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (root == nullptr) {
        xmlFreeDoc(doc);
        return;
    }
    xmlNodePtr cur = root->xmlChildrenNode;
    while (cur != nullptr) {
        std::string category = reinterpret_cast<const char*>(cur->name);
        xmlNodePtr valueNext = cur->xmlChildrenNode;
        if (category == "param") {
            LoadStrToStr(&paramBackup, valueNext);
        } else if (category == "locale_rules") {
            LoadStrToStr(&localesRulesBackup, valueNext);
        }
        cur = cur->next;
    }
    xmlFreeDoc(doc);
}

void DateTimeRule::LoadStrToPattern(std::unordered_map<std::string, icu::RegexPattern*>& map, xmlNodePtr cur)
{
    while (cur != nullptr) {
        std::string key = reinterpret_cast<const char*>(cur->name);
        key = key.substr(SIZE_THREE);
        xmlNodePtr value = cur->xmlChildrenNode;
        bool flag = false;
        if (value != nullptr && !xmlStrcmp(value->name, reinterpret_cast<const xmlChar*>("flag"))) {
            xmlChar* typePtr = xmlNodeGetContent(value);
            if (typePtr != nullptr) {
                std::string type = reinterpret_cast<const char*>(typePtr);
                flag = (type == "True") ? true : flag;
                xmlFree(typePtr);
            }
            value = value->next;
        }
        icu::UnicodeString content;
        while (value != nullptr && !xmlStrcmp(value->name, reinterpret_cast<const xmlChar*>("content"))) {
            xmlChar* contentPtr = xmlNodeGetContent(value);
            if (contentPtr != nullptr) {
                icu::UnicodeString tempContent = reinterpret_cast<char*>(contentPtr);
                content += tempContent;
                xmlFree(contentPtr);
            }
            value = value->next;
        }
        UErrorCode status = U_ZERO_ERROR;
        icu::RegexPattern* pattern = nullptr;
        if (flag) {
            pattern = icu::RegexPattern::compile(content, URegexpFlag::UREGEX_CASE_INSENSITIVE, status);
        } else {
            pattern = icu::RegexPattern::compile(content, 0, status);
        }
        map[key] = pattern;
        cur = cur->next;
    }
}

void DateTimeRule::LoadStrToStr(std::unordered_map<std::string, std::string>* map, xmlNodePtr cur)
{
    while (cur != nullptr) {
        std::string key = reinterpret_cast<const char*>(cur->name);
        key = key.substr(SIZE_THREE);
        xmlNodePtr value = cur->xmlChildrenNode;
        if (value != nullptr && !xmlStrcmp(value->name, reinterpret_cast<const xmlChar*>("level"))) {
            xmlChar* levelPtr = xmlNodeGetContent(value);
            if (levelPtr != nullptr) {
                std::string levelStr = reinterpret_cast<const char*>(levelPtr);
                int32_t status = 0;
                int level = ConvertString2Int(levelStr, status);
                levels[key] = level;
                xmlFree(levelPtr);
            } else {
                break;
            }
            value = value->next;
        }
        std::string content;
        while (value != nullptr && !xmlStrcmp(value->name, reinterpret_cast<const xmlChar*>("content"))) {
            xmlChar* contentPtr = xmlNodeGetContent(value);
            if (contentPtr != nullptr) {
                std::string tempContent = reinterpret_cast<const char*>(contentPtr);
                content += tempContent;
                xmlFree(contentPtr);
            }
            value = value->next;
        }
        (*map)[key] = content;
        cur = cur->next;
    }
}

std::string DateTimeRule::Get(std::unordered_map<std::string, std::string>& paramMap, std::string& ruleName)
{
    std::string result = "";
    if (paramMap.empty() || paramMap.find(ruleName) == paramMap.end()) {
        return result;
    }
    result = paramMap[ruleName];
    std::vector<std::string> temps;
    std::string splitStr = "|";
    Split(result, splitStr, temps);
    std::string sb;
    std::string mark = "";
    if (delimiter.find(locale) != delimiter.end()) {
        mark = delimiter[locale];
    } else {
        mark = "\\b";
    }
    for (auto& temp : temps) {
        if (!CompareBeginEnd(temp, "\\b", true)) {
            sb += mark;
        }
        sb += temp;
        if (!CompareBeginEnd(temp, "\\b", false) && !CompareBeginEnd(temp, ".", false)) {
            sb += mark;
        }
        sb += "|";
    }
    result = sb;
    if (CompareBeginEnd(result, "|", false)) {
        result.pop_back();
    }
    return result;
}

bool DateTimeRule::CompareBeginEnd(const std::string src, const std::string target, bool flag)
{
    size_t lengthSrc = src.length();
    size_t lengthTarget = target.length();
    if (lengthSrc < lengthTarget) {
        return false;
    }
    std::string subStr;
    if (flag) {
        subStr = src.substr(0, lengthTarget);
    } else {
        subStr = src.substr(lengthSrc - lengthTarget, lengthTarget);
    }
    return subStr == target;
}


std::string DateTimeRule::GetWithoutB(const std::string& ruleName)
{
    std::string result = "";
    if (param.empty() || param.find(ruleName) == param.end()) {
        return result;
    }
    result = param[ruleName];
    return result;
}

bool DateTimeRule::IsRelDates(icu::UnicodeString& hyphen, std::string& locale)
{
    bool isRel = false;
    if (hyphen.trim().isEmpty()) {
        isRel = true;
    } else if (hyphen.trim() == ',' && relDates.find(locale) != relDates.end() &&
        relDates[locale].find(",") != std::string::npos) {
        isRel = true;
    }
    return isRel;
}

std::string DateTimeRule::trim(const std::string& src)
{
    std::string target = src;
    if (target.empty()) {
        return target;
    }
    target.erase(0, target.find_first_not_of(" "));
    target.erase(target.find_last_not_of(" ") + 1);
    return target;
}

int DateTimeRule::CompareLevel(std::string& key1, std::string& key2)
{
    int result = 0;
    int level1 = GetLevel(key1);
    int level2 = GetLevel(key2);
    if (level1 > level2) {
        result = 1;
    } else if (level1 < level2) {
        result = -1;
    }
    return result;
}

int DateTimeRule::GetLevel(std::string& name)
{
    int baselevel;
    int32_t status = 0;
    int key = ConvertString2Int(name, status);
    if (key > RULE_LOWER_THRESHOLD_1 && key < RULE_UPPER_THRESHOLD_1) {
        baselevel = BASE_LEVEL_LOW;
    } else if (key > RULE_LOWER_THRESHOLD_2 && key < RULE_UPPER_THRESHOLD_2) {
        baselevel = BASE_LEVEL_MID;
    } else {
        baselevel = BASE_LEVEL_HIGH;
    }
    int addLeve = 1;
    if (levels.find(name) != levels.end()) {
        addLeve = levels[name];
    }
    int level = baselevel + addLeve;
    return level;
}

std::unordered_map<std::string, std::string> DateTimeRule::GetUniverseRules()
{
    return universeRules;
}

std::unordered_map<std::string, std::string> DateTimeRule::GetLocalesRules()
{
    return localesRules;
}

std::unordered_map<std::string, std::string> DateTimeRule::GetLocalesRulesBackup()
{
    return localesRulesBackup;
}

std::unordered_map<std::string, std::unordered_map<std::string, std::string>> DateTimeRule::GetSubRulesMap()
{
    return subRulesMap;
}

std::unordered_map<std::string, std::string> DateTimeRule::GetSubRules()
{
    return subRules;
}

std::unordered_map<std::string, std::string> DateTimeRule::GetFilterRules()
{
    return filterRules;
}

std::unordered_map<std::string, std::string> DateTimeRule::GetPastRules()
{
    return pastRules;
}

std::unordered_map<std::string, std::string> DateTimeRule::GetParam()
{
    return param;
}

std::unordered_map<std::string, std::string> DateTimeRule::GetParamBackup()
{
    return paramBackup;
}

icu::RegexPattern* DateTimeRule::GetPatternByKey(const std::string& key)
{
    if (patternsMap.find(key) != patternsMap.end()) {
        return patternsMap[key];
    }
    return nullptr;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS