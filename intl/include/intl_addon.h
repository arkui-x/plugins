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

#ifndef INTL_ADDON_H
#define INTL_ADDON_H

#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "locale_info.h"
#include "date_time_format.h"
#include "number_format.h"
#include "relative_time_format.h"
#include "collator.h"
#include "plural_rules.h"

namespace OHOS {
namespace Global {
namespace I18n {
void GetLocaleTags(napi_env env, napi_value rawLocaleTag, std::vector<std::string> &localeTags);
void GetOptionValue(napi_env env, napi_value options, const std::string &optionName,
    std::map<std::string, std::string> &map);
void GetBoolOptionValue(napi_env env, napi_value options, const std::string &optionName,
    std::map<std::string, std::string> &map);
void GetIntegerOptionValue(napi_env env, napi_value options, const std::string &optionName,
    std::map<std::string, std::string> &map);
void GetDateOptionValues(napi_env env, napi_value options, std::map<std::string, std::string> &map);
void GetNumberOptionValues(napi_env env, napi_value options, std::map<std::string, std::string> &map);
void GetCollatorOptionValue(napi_env env, napi_value options, std::map<std::string, std::string> &map);
void GetPluralRulesOptionValues(napi_env env, napi_value options, std::map<std::string, std::string> &map);
void SetOptionProperties(napi_env env, napi_value &result, std::map<std::string, std::string> &options,
    const std::string &option);
void SetIntegerOptionProperties(napi_env env, napi_value &result,
    std::map<std::string, std::string> &options, const std::string &option);
void SetBooleanOptionProperties(napi_env env, napi_value &result,
    std::map<std::string, std::string> &options, const std::string &option);
void GetRelativeTimeOptionValues(napi_env env, napi_value options, std::map<std::string, std::string> &map);

class IntlAddon {
public:
    static napi_value InitLocale(napi_env env, napi_value exports);
    static napi_value InitDateTimeFormat(napi_env env, napi_value exports);
    static napi_value InitNumberFormat(napi_env env, napi_value exports);
    static napi_value InitCollator(napi_env env, napi_value exports);
    static napi_value InitRelativeTimeFormat(napi_env env, napi_value exports);
    static napi_value InitPluralRules(napi_env env, napi_value exports);
    static void Destructor(napi_env env, void *nativeObject, void *finalize_hint);

    IntlAddon();
    virtual ~IntlAddon();

private:
    static napi_value DateTimeFormatConstructor(napi_env env, napi_callback_info info);
    static napi_value NumberFormatConstructor(napi_env env, napi_callback_info info);
    static napi_value LocaleConstructor(napi_env env, napi_callback_info info);
    static napi_value RelativeTimeFormatConstructor(napi_env env, napi_callback_info info);
    static napi_value GetLanguage(napi_env env, napi_callback_info info);
    static napi_value GetScript(napi_env env, napi_callback_info info);
    static napi_value GetRegion(napi_env env, napi_callback_info info);
    static napi_value GetBaseName(napi_env env, napi_callback_info info);
    static napi_value GetCalendar(napi_env env, napi_callback_info info);
    static napi_value GetCollation(napi_env env, napi_callback_info info);
    static napi_value GetHourCycle(napi_env env, napi_callback_info info);
    static napi_value GetNumberingSystem(napi_env env, napi_callback_info info);
    static napi_value GetNumeric(napi_env env, napi_callback_info info);
    static napi_value GetCaseFirst(napi_env env, napi_callback_info info);
    static napi_value ToString(napi_env env, napi_callback_info info);
    static napi_value Maximize(napi_env env, napi_callback_info info);
    static napi_value Minimize(napi_env env, napi_callback_info info);

    static napi_value FormatDateTime(napi_env env, napi_callback_info info);
    static napi_value FormatDateTimeRange(napi_env env, napi_callback_info info);
    static napi_value GetDateTimeResolvedOptions(napi_env env, napi_callback_info info);

    static napi_value FormatRelativeTime(napi_env env, napi_callback_info info);
    static napi_value FormatToParts(napi_env env, napi_callback_info info);
    static void FillInArrayElement(napi_env env, napi_value &result, napi_status &status,
        const std::vector<std::vector<std::string>> &timeVector);
    static napi_value GetRelativeTimeResolvedOptions(napi_env env, napi_callback_info info);

    static napi_value GetNumberResolvedOptions(napi_env env, napi_callback_info info);
    static napi_value FormatNumber(napi_env env, napi_callback_info info);

    static napi_value CollatorConstructor(napi_env env, napi_callback_info info);
    static napi_value CompareString(napi_env env, napi_callback_info info);
    static napi_value GetCollatorResolvedOptions(napi_env env, napi_callback_info info);

    static napi_value PluralRulesConstructor(napi_env env, napi_callback_info info);
    static napi_value Select(napi_env env, napi_callback_info info);

    static int64_t GetMilliseconds(napi_env env, napi_value *argv, int index);
    bool InitLocaleContext(napi_env env, napi_callback_info info, const std::string localeTag,
        std::map<std::string, std::string> &map);
    bool InitDateTimeFormatContext(napi_env env, napi_callback_info info, std::vector<std::string> localeTags,
        std::map<std::string, std::string> &map);
    bool InitNumberFormatContext(napi_env env, napi_callback_info info, std::vector<std::string> localeTags,
        std::map<std::string, std::string> &map);
    bool InitCollatorContext(napi_env env, napi_callback_info info, std::vector<std::string> localeTags,
        std::map<std::string, std::string> &map);
    bool InitRelativeTimeFormatContext(napi_env env, napi_callback_info info, std::vector<std::string> localeTags,
        std::map<std::string, std::string> &map);
    bool InitPluralRulesContext(napi_env env, napi_callback_info info, std::vector<std::string> localeTags,
        std::map<std::string, std::string> &map);

    static napi_value SetProperty(napi_env env, napi_callback_info info);

    napi_env env_;
    std::unique_ptr<LocaleInfo> locale_ = nullptr;
    std::unique_ptr<DateTimeFormat> datefmt_ = nullptr;
    std::unique_ptr<NumberFormat> numberfmt_ = nullptr;
    std::unique_ptr<RelativeTimeFormat> relativetimefmt_ = nullptr;
    std::unique_ptr<Collator> collator_ = nullptr;
    std::unique_ptr<PluralRules> pluralrules_ = nullptr;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif