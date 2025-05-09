/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "locale_config.h"
#include "ohos/init_data.h"
#include "relative_time_format.h"

namespace OHOS {
namespace Global {
namespace I18n {
std::unordered_map<std::string, UDateRelativeDateTimeFormatterStyle> RelativeTimeFormat::relativeFormatStyle = {
    { "long", UDAT_STYLE_LONG },
    { "short", UDAT_STYLE_SHORT },
    { "narrow", UDAT_STYLE_NARROW }
};

std::unordered_map<std::string, std::string> RelativeTimeFormat::defaultFormatStyle = {
    { "wearable", "narrow" },
    { "liteWearable", "narrow" },
    { "watch", "narrow" }
};

std::unordered_map<std::string, URelativeDateTimeUnit> RelativeTimeFormat::relativeUnits = {
    { "second", UDAT_REL_UNIT_SECOND },
    { "seconds", UDAT_REL_UNIT_SECOND },
    { "minute", UDAT_REL_UNIT_MINUTE },
    { "minutes", UDAT_REL_UNIT_MINUTE },
    { "hour", UDAT_REL_UNIT_HOUR },
    { "hours", UDAT_REL_UNIT_HOUR },
    { "day", UDAT_REL_UNIT_DAY },
    { "days", UDAT_REL_UNIT_DAY },
    { "week", UDAT_REL_UNIT_WEEK },
    { "weeks", UDAT_REL_UNIT_WEEK },
    { "month", UDAT_REL_UNIT_MONTH },
    { "months", UDAT_REL_UNIT_MONTH },
    { "quarter", UDAT_REL_UNIT_QUARTER },
    { "quarters", UDAT_REL_UNIT_QUARTER },
    { "year", UDAT_REL_UNIT_YEAR },
    { "years", UDAT_REL_UNIT_YEAR },
};

RelativeTimeFormat::RelativeTimeFormat(const std::vector<std::string> &localeTags,
    std::map<std::string, std::string> &configs)
{
    SetDefaultStyle();
    UErrorCode status = U_ZERO_ERROR;
    ParseConfigs(configs);
    for (size_t i = 0; i < localeTags.size(); i++) {
        std::string curLocale = localeTags[i];
        locale = icu::Locale::forLanguageTag(icu::StringPiece(curLocale), status);
        if (status != U_ZERO_ERROR) {
            status = U_ZERO_ERROR;
            continue;
        }
        if (LocaleInfo::GetValidLocales().count(locale.getLanguage()) > 0) {
            localeInfo = std::make_unique<LocaleInfo>(curLocale, configs);
            if (!localeInfo->InitSuccess()) {
                continue;
            }
            locale = localeInfo->GetLocale();
            localeBaseName = localeInfo->GetBaseName();
            relativeTimeFormat = std::make_unique<icu::RelativeDateTimeFormatter>(locale, nullptr, style,
                UDISPCTX_CAPITALIZATION_NONE, status);
            if (!U_SUCCESS(status)) {
                status = U_ZERO_ERROR;
                continue;
            }
            createSuccess = true;
            break;
        }
    }
    if (!createSuccess) {
        localeInfo = std::make_unique<LocaleInfo>(LocaleConfig::GetSystemLocaleWithExtParam(), configs);
        if (localeInfo->InitSuccess()) {
            locale = localeInfo->GetLocale();
            localeBaseName = localeInfo->GetBaseName();
            relativeTimeFormat = std::make_unique<icu::RelativeDateTimeFormatter>(locale, nullptr, style,
                UDISPCTX_CAPITALIZATION_NONE, status);
            if (U_SUCCESS(status)) {
                createSuccess = true;
            }
        }
    }
    numberingSystem = localeInfo->GetNumberingSystem();
    if (numberingSystem == "") {
        numberingSystem = "latn";
    }
}

RelativeTimeFormat::~RelativeTimeFormat()
{
}

void RelativeTimeFormat::ParseConfigs(std::map<std::string, std::string> &configs)
{
    if (configs.count("style") > 0) {
        styleString = configs["style"];
    }
    if (relativeFormatStyle.count(styleString) > 0) {
        style = relativeFormatStyle[styleString];
    }
    if (configs.count("numeric") > 0) {
        numeric = configs["numeric"];
    }
}

std::string RelativeTimeFormat::Format(double number, const std::string &unit)
{
    if (!createSuccess || !relativeUnits.count(unit)) {
        return "";
    }
    UErrorCode status = U_ZERO_ERROR;
    icu::UnicodeString formattedTime;
    std::string result;
    if (!strcmp(numeric.c_str(), "always")) {
        formattedTime = relativeTimeFormat->formatNumericToValue(number, relativeUnits[unit], status).toString(status);
    } else {
        formattedTime = relativeTimeFormat->formatToValue(number, relativeUnits[unit], status).toString(status);
    }
    formattedTime.toUTF8String(result);
    return result;
}

void RelativeTimeFormat::InsertInfo(std::vector<std::vector<std::string>> &timeVector,
    const std::string &unit, bool isInteger, const std::string &value)
{
    std::vector<std::string> info;
    if (isInteger) {
        info.push_back("integer");
        info.push_back(value);
        info.push_back(unit);
    } else {
        info.push_back("literal");
        info.push_back(value);
    }
    timeVector.push_back(info);
}

void RelativeTimeFormat::ProcessIntegerField(const std::map<size_t, size_t> &indexMap,
    std::vector<std::vector<std::string>> &timeVector, size_t &startIndex, const std::string &unit,
    const std::string &result)
{
    for (auto iter = indexMap.begin(); iter != indexMap.end(); iter++) {
        if (iter->first > startIndex) {
            InsertInfo(timeVector, unit, true, result.substr(startIndex, iter->first - startIndex));
            InsertInfo(timeVector, unit, true, result.substr(iter->first, iter->second - iter->first));
            startIndex = iter->second;
        }
    }
}

void RelativeTimeFormat::FormatToParts(double number, const std::string &unit,
    std::vector<std::vector<std::string>> &timeVector)
{
    if (!createSuccess || !relativeUnits.count(unit)) {
        return;
    }
    UErrorCode status = U_ZERO_ERROR;
    std::string result;
    icu::FormattedRelativeDateTime fmtRelativeTime;
    if (numeric.empty() || !strcmp(numeric.c_str(), "always")) {
        fmtRelativeTime = relativeTimeFormat->formatNumericToValue(number, relativeUnits[unit], status);
    } else {
        fmtRelativeTime = relativeTimeFormat->formatToValue(number, relativeUnits[unit], status);
    }
    fmtRelativeTime.toString(status).toUTF8String(result);
    icu::ConstrainedFieldPosition constrainedPos;
    constrainedPos.constrainCategory(UFIELD_CATEGORY_NUMBER);
    size_t prevIndex = 0;
    size_t length = result.length();
    std::map<size_t, size_t> indexMap;
    while (fmtRelativeTime.nextPosition(constrainedPos, status)) {
        size_t startIndex = (size_t)constrainedPos.getStart();
        if (constrainedPos.getCategory() == UFIELD_CATEGORY_NUMBER) {
            if (constrainedPos.getField() == UNUM_GROUPING_SEPARATOR_FIELD) {
                indexMap.insert(std::make_pair(startIndex, (size_t)constrainedPos.getLimit()));
                continue;
            }
            if (startIndex > prevIndex) {
                InsertInfo(timeVector, unit, false, result.substr(prevIndex, startIndex - prevIndex));
            }
            if (constrainedPos.getField() == UNUM_INTEGER_FIELD) {
                ProcessIntegerField(indexMap, timeVector, startIndex, unit, result);
            }
            InsertInfo(timeVector, unit, true, result.substr(startIndex,
                (size_t)constrainedPos.getLimit() - startIndex));
            prevIndex = (size_t)constrainedPos.getLimit();
        }
    }
    if (prevIndex < length) {
        InsertInfo(timeVector, unit, false, result.substr(prevIndex, length - prevIndex));
    }
}

void RelativeTimeFormat::GetResolvedOptions(std::map<std::string, std::string> &map)
{
    map.insert(std::make_pair("locale", localeBaseName));
    if (!styleString.empty()) {
        map.insert(std::make_pair("style", styleString));
    }
    if (!numeric.empty()) {
        map.insert(std::make_pair("numeric", numeric));
    }
    if (!numberingSystem.empty()) {
        map.insert(std::make_pair("numberingSystem", numberingSystem));
    }
}

void RelativeTimeFormat::SetDefaultStyle()
{
    auto plugin = Plugin::INTL::Create();
    if (!plugin) {
        return;
    }
    std::string deviceType = plugin->GetDeviceType();
    if (defaultFormatStyle.find(deviceType) != defaultFormatStyle.end()) {
        styleString = defaultFormatStyle[deviceType];
    }
}
} // namespace I18n
} // namespace Global
} // namespace OHOS
