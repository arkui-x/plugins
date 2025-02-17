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

#include "intl_addon.h"

#include <vector>
#include <set>

#include "icu_data.h"
#include "log.h"
#include "node_api.h"
#include "plugin_utils.h"

#ifdef ANDROID_PLATFORM
#include "plugins/intl/android/java/jni/intl_plugin_jni.h"
#endif

namespace OHOS {
namespace Global {
namespace I18n {
static thread_local napi_ref *g_constructor = nullptr;

IntlAddon::IntlAddon() : env_(nullptr) {}

IntlAddon::~IntlAddon()
{
}

void IntlAddon::Destructor(napi_env env, void *nativeObject, void *hint)
{
    if (!nativeObject) {
        return;
    }
    reinterpret_cast<IntlAddon *>(nativeObject)->~IntlAddon();
}

napi_value IntlAddon::SetProperty(napi_env env, napi_callback_info info)
{
    // do nothing but provided as an input parameter for DECLARE_NAPI_GETTER_SETTER;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

napi_value IntlAddon::InitLocale(napi_env env, napi_value exports)
{
    napi_status status = napi_ok;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_GETTER_SETTER("language", GetLanguage, SetProperty),
        DECLARE_NAPI_GETTER_SETTER("baseName", GetBaseName, SetProperty),
        DECLARE_NAPI_GETTER_SETTER("region", GetRegion, SetProperty),
        DECLARE_NAPI_GETTER_SETTER("script", GetScript, SetProperty),
        DECLARE_NAPI_GETTER_SETTER("calendar", GetCalendar, SetProperty),
        DECLARE_NAPI_GETTER_SETTER("collation", GetCollation, SetProperty),
        DECLARE_NAPI_GETTER_SETTER("hourCycle", GetHourCycle, SetProperty),
        DECLARE_NAPI_GETTER_SETTER("numberingSystem", GetNumberingSystem, SetProperty),
        DECLARE_NAPI_GETTER_SETTER("numeric", GetNumeric, SetProperty),
        DECLARE_NAPI_GETTER_SETTER("caseFirst", GetCaseFirst, SetProperty),
        DECLARE_NAPI_FUNCTION("toString", ToString),
        DECLARE_NAPI_FUNCTION("minimize", Minimize),
        DECLARE_NAPI_FUNCTION("maximize", Maximize),
    };

    napi_value constructor = nullptr;
    status = napi_define_class(env, "Locale", NAPI_AUTO_LENGTH, LocaleConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        LOGE("Define class failed when InitLocale");
        return nullptr;
    }

    status = napi_set_named_property(env, exports, "Locale", constructor);
    if (status != napi_ok) {
        LOGE("Set property failed when InitLocale");
        return nullptr;
    }
    g_constructor = new (std::nothrow) napi_ref;
    if (!g_constructor) {
        LOGE("Failed to create ref at init");
        return nullptr;
    }
    status = napi_create_reference(env, constructor, 1, g_constructor);
    if (status != napi_ok) {
        LOGE("Failed to create reference at init");
        return nullptr;
    }
    return exports;
}

napi_value IntlAddon::InitDateTimeFormat(napi_env env, napi_value exports)
{
    napi_status status = napi_ok;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("format", FormatDateTime),
        DECLARE_NAPI_FUNCTION("formatRange", FormatDateTimeRange),
        DECLARE_NAPI_FUNCTION("resolvedOptions", GetDateTimeResolvedOptions)
    };

    napi_value constructor = nullptr;
    status = napi_define_class(env, "DateTimeFormat", NAPI_AUTO_LENGTH, DateTimeFormatConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        LOGE("Define class failed when InitDateTimeFormat");
        return nullptr;
    }

    status = napi_set_named_property(env, exports, "DateTimeFormat", constructor);
    if (status != napi_ok) {
        LOGE("Set property failed when InitDateTimeFormat");
        return nullptr;
    }
    return exports;
}

napi_value IntlAddon::InitRelativeTimeFormat(napi_env env, napi_value exports)
{
    napi_status status = napi_ok;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("format", FormatRelativeTime),
        DECLARE_NAPI_FUNCTION("formatToParts", FormatToParts),
        DECLARE_NAPI_FUNCTION("resolvedOptions", GetRelativeTimeResolvedOptions)
    };

    napi_value constructor = nullptr;
    status = napi_define_class(env, "RelativeTimeFormat", NAPI_AUTO_LENGTH, RelativeTimeFormatConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        LOGE("Define class failed when InitRelativeTimeFormat");
        return nullptr;
    }

    status = napi_set_named_property(env, exports, "RelativeTimeFormat", constructor);
    if (status != napi_ok) {
        LOGE("Set property failed when InitRelativeTimeFormat");
        return nullptr;
    }
    return exports;
}

napi_value IntlAddon::InitNumberFormat(napi_env env, napi_value exports)
{
    napi_status status = napi_ok;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("format", FormatNumber),
        DECLARE_NAPI_FUNCTION("resolvedOptions", GetNumberResolvedOptions)
    };

    napi_value constructor = nullptr;
    status = napi_define_class(env, "NumberFormat", NAPI_AUTO_LENGTH, NumberFormatConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        LOGE("Define class failed when InitNumberFormat");
        return nullptr;
    }

    status = napi_set_named_property(env, exports, "NumberFormat", constructor);
    if (status != napi_ok) {
        LOGE("Set property failed when InitNumberFormat");
        return nullptr;
    }
    return exports;
}

void GetOptionValue(napi_env env, napi_value options, const std::string &optionName,
    std::map<std::string, std::string> &map)
{
    napi_value optionValue = nullptr;
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, options, &type);
    if (status != napi_ok && type != napi_object) {
        LOGE("Get option failed, option is not an object");
        return;
    }
    bool hasProperty = false;
    napi_status propStatus = napi_has_named_property(env, options, optionName.c_str(), &hasProperty);
    if (propStatus == napi_ok && hasProperty) {
        status = napi_get_named_property(env, options, optionName.c_str(), &optionValue);
        if (status == napi_ok) {
            size_t len = 0;
            napi_get_value_string_utf8(env, optionValue, nullptr, 0, &len);
            std::vector<char> optionBuf(len + 1);
            status = napi_get_value_string_utf8(env, optionValue, optionBuf.data(), len + 1, &len);
            if (status != napi_ok) {
                return;
            }
            map.insert(make_pair(optionName, optionBuf.data()));
        }
    }
}

void GetIntegerOptionValue(napi_env env, napi_value options, const std::string &optionName,
    std::map<std::string, std::string> &map)
{
    napi_value optionValue = nullptr;
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, options, &type);
    if (status != napi_ok && type != napi_object) {
        LOGE("Set option failed, option is not an object");
        return;
    }
    bool hasProperty = false;
    napi_status propStatus = napi_has_named_property(env, options, optionName.c_str(), &hasProperty);
    if (propStatus == napi_ok && hasProperty) {
        status = napi_get_named_property(env, options, optionName.c_str(), &optionValue);
        if (status == napi_ok) {
            int64_t integerValue = -1;
            status = napi_get_value_int64(env, optionValue, &integerValue);
            if (status == napi_ok) {
                map.insert(make_pair(optionName, std::to_string(integerValue)));
            }
        }
    }
}

void GetBoolOptionValue(napi_env env, napi_value options, const std::string &optionName,
    std::map<std::string, std::string> &map)
{
    napi_value optionValue = nullptr;
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, options, &type);
    if (status != napi_ok && type != napi_object) {
        LOGE("Set option failed, option is not an object");
        return;
    }
    bool hasProperty = false;
    napi_status propStatus = napi_has_named_property(env, options, optionName.c_str(), &hasProperty);
    if (propStatus == napi_ok && hasProperty) {
        status = napi_get_named_property(env, options, optionName.c_str(), &optionValue);
        if (status == napi_ok) {
            bool boolValue = false;
            napi_get_value_bool(env, optionValue, &boolValue);
            std::string value = boolValue ? "true" : "false";
            map.insert(make_pair(optionName, value));
        }
    }
}

void GetDateOptionValues(napi_env env, napi_value options, std::map<std::string, std::string> &map)
{
    GetOptionValue(env, options, "calendar", map);
    GetOptionValue(env, options, "dateStyle", map);
    GetOptionValue(env, options, "timeStyle", map);
    GetOptionValue(env, options, "hourCycle", map);
    GetOptionValue(env, options, "timeZone", map);
    GetOptionValue(env, options, "timeZoneName", map);
    GetOptionValue(env, options, "numberingSystem", map);
    GetBoolOptionValue(env, options, "hour12", map);
    GetOptionValue(env, options, "weekday", map);
    GetOptionValue(env, options, "era", map);
    GetOptionValue(env, options, "year", map);
    GetOptionValue(env, options, "month", map);
    GetOptionValue(env, options, "day", map);
    GetOptionValue(env, options, "hour", map);
    GetOptionValue(env, options, "minute", map);
    GetOptionValue(env, options, "second", map);
    GetOptionValue(env, options, "localeMatcher", map);
    GetOptionValue(env, options, "formatMatcher", map);
    GetOptionValue(env, options, "dayPeriod", map);
}

void GetRelativeTimeOptionValues(napi_env env, napi_value options, std::map<std::string, std::string> &map)
{
    GetOptionValue(env, options, "localeMatcher", map);
    GetOptionValue(env, options, "numeric", map);
    GetOptionValue(env, options, "style", map);
}

std::string GetLocaleTag(napi_env env, napi_value argv)
{
    std::string localeTag = "";
    std::vector<char> buf;
    if (argv != nullptr) {
        napi_valuetype valueType = napi_valuetype::napi_undefined;
        napi_typeof(env, argv, &valueType);
        if (valueType != napi_valuetype::napi_string) {
            return "";
        }
        size_t len = 0;
        napi_status status = napi_get_value_string_utf8(env, argv, nullptr, 0, &len);
        if (status != napi_ok) {
            LOGE("Get locale tag length failed");
            return "";
        }
        buf.resize(len + 1);
        status = napi_get_value_string_utf8(env, argv, buf.data(), len + 1, &len);
        if (status != napi_ok) {
            LOGE("Get locale tag failed");
            return "";
        }
        localeTag = buf.data();
    } else {
        localeTag = "";
    }
    return localeTag;
}

napi_value IntlAddon::LocaleConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    std::string localeTag = GetLocaleTag(env, argv[0]);

    std::map<std::string, std::string> map = {};
    if (argc > 1) {
        GetOptionValue(env, argv[1], "calendar", map);
        GetOptionValue(env, argv[1], "collation", map);
        GetOptionValue(env, argv[1], "hourCycle", map);
        GetOptionValue(env, argv[1], "numberingSystem", map);
        GetBoolOptionValue(env, argv[1], "numeric", map);
        GetOptionValue(env, argv[1], "caseFirst", map);
    }
    std::unique_ptr<IntlAddon> obj = nullptr;
    obj = std::make_unique<IntlAddon>();
    status =
        napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()), IntlAddon::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("Wrap IntlAddon failed");
        return nullptr;
    }
    if (!obj->InitLocaleContext(env, info, localeTag, map)) {
        return nullptr;
    }
    obj.release();
    return thisVar;
}

bool IntlAddon::InitLocaleContext(napi_env env, napi_callback_info info, const std::string localeTag,
    std::map<std::string, std::string> &map)
{
    napi_value global = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok) {
        LOGE("Get global failed");
        return false;
    }
    env_ = env;
    locale_ = std::make_unique<LocaleInfo>(localeTag, map);

    return locale_ != nullptr;
}

void GetLocaleTags(napi_env env, napi_value rawLocaleTag, std::vector<std::string> &localeTags)
{
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, rawLocaleTag, nullptr, 0, &len);
    if (status != napi_ok) {
        LOGE("Get locale tag length failed");
        return;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, rawLocaleTag, buf.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGE("Get locale tag failed");
        return;
    }
    localeTags.push_back(buf.data());
}

napi_value IntlAddon::DateTimeFormatConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    std::vector<std::string> localeTags;
    if (argc > 0) {
        napi_valuetype valueType = napi_valuetype::napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        bool isArray = false;
        napi_is_array(env, argv[0], &isArray);
        if (valueType == napi_valuetype::napi_string) {
            GetLocaleTags(env, argv[0], localeTags);
        } else if (isArray) {
            uint32_t arrayLength = 0;
            napi_get_array_length(env, argv[0], &arrayLength);
            napi_value element = nullptr;
            for (uint32_t i = 0; i < arrayLength; i++) {
                napi_get_element(env, argv[0], i, &element);
                GetLocaleTags(env, element, localeTags);
            }
        }
    }
    std::map<std::string, std::string> map = {};
    if (argc > 1) {
        GetDateOptionValues(env, argv[1], map);
    }
    std::unique_ptr<IntlAddon> obj = nullptr;
    obj = std::make_unique<IntlAddon>();
    status =
        napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()), IntlAddon::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("Wrap IntlAddon failed");
        return nullptr;
    }
    if (!obj->InitDateTimeFormatContext(env, info, localeTags, map)) {
        LOGE("Init DateTimeFormat failed");
        return nullptr;
    }
    obj.release();
    return thisVar;
}

bool IntlAddon::InitDateTimeFormatContext(napi_env env, napi_callback_info info, std::vector<std::string> localeTags,
    std::map<std::string, std::string> &map)
{
    napi_value global = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok) {
        LOGE("Get global failed");
        return false;
    }
    env_ = env;
    datefmt_ = DateTimeFormat::CreateInstance(localeTags, map);

    return datefmt_ != nullptr;
}

napi_value IntlAddon::RelativeTimeFormatConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    std::vector<std::string> localeTags;
    if (argc > 0) {
        napi_valuetype valueType = napi_valuetype::napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        bool isArray = false;
        napi_is_array(env, argv[0], &isArray);
        if (valueType == napi_valuetype::napi_string) {
            GetLocaleTags(env, argv[0], localeTags);
        } else if (isArray) {
            uint32_t arrayLength = 0;
            napi_get_array_length(env, argv[0], &arrayLength);
            napi_value element = nullptr;
            for (uint32_t i = 0; i < arrayLength; i++) {
                napi_get_element(env, argv[0], i, &element);
                GetLocaleTags(env, element, localeTags);
            }
        }
    }
    std::map<std::string, std::string> map = {};
    if (argc > 1) {
        GetRelativeTimeOptionValues(env, argv[1], map);
    }
    std::unique_ptr<IntlAddon> obj = nullptr;
    obj = std::make_unique<IntlAddon>();
    status =
        napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()), IntlAddon::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("Wrap IntlAddon failed");
        return nullptr;
    }
    if (!obj->InitRelativeTimeFormatContext(env, info, localeTags, map)) {
        LOGE("Init RelativeTimeFormat failed");
        return nullptr;
    }
    obj.release();
    return thisVar;
}

bool IntlAddon::InitRelativeTimeFormatContext(napi_env env, napi_callback_info info,
    std::vector<std::string> localeTags, std::map<std::string, std::string> &map)
{
    env_ = env;
    relativetimefmt_ = std::make_unique<RelativeTimeFormat>(localeTags, map);

    return relativetimefmt_ != nullptr;
}

napi_value IntlAddon::FormatDateTime(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    
    int64_t milliseconds = GetMilliseconds(env, argv, 0);
    if (milliseconds == -1) {
        return nullptr;
    }
    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->datefmt_) {
        LOGE("Get DateTimeFormat object failed");
        return nullptr;
    }
    std::string value = obj->datefmt_->Format(milliseconds);
    napi_value result = nullptr;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Create format string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::FormatDateTimeRange(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (argc < 2) {
        LOGE("Parameter wrong");
        return nullptr;
    }
    int64_t firstMilliseconds = GetMilliseconds(env, argv, 0);
    int64_t secondMilliseconds = GetMilliseconds(env, argv, 1);
    if (firstMilliseconds == -1 || secondMilliseconds == -1) {
        return nullptr;
    }
    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->datefmt_) {
        LOGE("Get DateTimeFormat object failed");
        return nullptr;
    }
    std::string value = obj->datefmt_->FormatRange(firstMilliseconds, secondMilliseconds);
    napi_value result = nullptr;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Create format string failed");
        return nullptr;
    }
    return result;
}

void GetNumberOptionValues(napi_env env, napi_value options, std::map<std::string, std::string> &map)
{
    GetOptionValue(env, options, "currency", map);
    GetOptionValue(env, options, "currencySign", map);
    GetOptionValue(env, options, "currencyDisplay", map);
    GetOptionValue(env, options, "unit", map);
    GetOptionValue(env, options, "unitDisplay", map);
    GetOptionValue(env, options, "compactDisplay", map);
    GetOptionValue(env, options, "signDisplay", map);
    GetOptionValue(env, options, "localeMatcher", map);
    GetOptionValue(env, options, "style", map);
    GetOptionValue(env, options, "numberingSystem", map);
    GetOptionValue(env, options, "notation", map);
    GetOptionValue(env, options, "unitUsage", map);
    GetBoolOptionValue(env, options, "useGrouping", map);
    GetIntegerOptionValue(env, options, "minimumIntegerDigits", map);
    GetIntegerOptionValue(env, options, "minimumFractionDigits", map);
    GetIntegerOptionValue(env, options, "maximumFractionDigits", map);
    GetIntegerOptionValue(env, options, "minimumSignificantDigits", map);
    GetIntegerOptionValue(env, options, "maximumSignificantDigits", map);
}

napi_value IntlAddon::NumberFormatConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    std::vector<std::string> localeTags;
    if (argc > 0) {
        napi_valuetype valueType = napi_valuetype::napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        bool isArray = false;
        napi_is_array(env, argv[0], &isArray);

        if (valueType == napi_valuetype::napi_string) {
            GetLocaleTags(env, argv[0], localeTags);
        } else if (isArray) {
            uint32_t arrayLength = 0;
            napi_get_array_length(env, argv[0], &arrayLength);
            napi_value element = nullptr;
            for (uint32_t i = 0; i < arrayLength; i++) {
                napi_get_element(env, argv[0], i, &element);
                GetLocaleTags(env, element, localeTags);
            }
        }
    }
    std::map<std::string, std::string> map = {};
    if (argc > 1) {
        GetNumberOptionValues(env, argv[1], map);
    }
    std::unique_ptr<IntlAddon> obj = nullptr;
    obj = std::make_unique<IntlAddon>();
    status =
        napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()), IntlAddon::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("Wrap IntlAddon failed");
        return nullptr;
    }
    if (!obj->InitNumberFormatContext(env, info, localeTags, map)) {
        LOGE("Init NumberFormat failed");
        return nullptr;
    }
    obj.release();
    return thisVar;
}

bool IntlAddon::InitNumberFormatContext(napi_env env, napi_callback_info info, std::vector<std::string> localeTags,
    std::map<std::string, std::string> &map)
{
    napi_value global = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok) {
        LOGE("Get global failed");
        return false;
    }
    env_ = env;
    numberfmt_ = std::make_unique<NumberFormat>(localeTags, map);

    return numberfmt_ != nullptr;
}

int64_t IntlAddon::GetMilliseconds(napi_env env, napi_value *argv, int index)
{
    napi_value funcGetDateInfo = nullptr;
    napi_status status = napi_get_named_property(env, argv[index], "getTime", &funcGetDateInfo);
    if (status != napi_ok) {
        LOGE("Get Milliseconds property failed");
        return -1;
    }
    napi_value ret_value = nullptr;
    status = napi_call_function(env, argv[index], funcGetDateInfo, 0, nullptr, &ret_value);
    if (status != napi_ok) {
        LOGE("Get Milliseconds function failed");
        return -1;
    }
    int64_t milliseconds = 0;
    status = napi_get_value_int64(env, ret_value, &milliseconds);
    if (status != napi_ok) {
        LOGE("Get Milliseconds failed");
        return -1;
    }
    return milliseconds;
}

napi_value IntlAddon::GetLanguage(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv[0];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->locale_) {
        LOGE("Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->GetLanguage();

    napi_value result = nullptr;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Create language string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::GetScript(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv[0];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->locale_) {
        LOGE("Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->GetScript();

    napi_value result = nullptr;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Create script string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::GetRegion(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv[0];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->locale_) {
        LOGE("Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->GetRegion();

    napi_value result = nullptr;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Create region string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::GetBaseName(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv[0];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->locale_) {
        LOGE("Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->GetBaseName();

    napi_value result = nullptr;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Create base name string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::GetCalendar(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv[0];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->locale_) {
        LOGE("Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->GetCalendar();

    napi_value result = nullptr;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Create base name string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::GetCollation(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv[0];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->locale_) {
        LOGE("Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->GetCollation();

    napi_value result = nullptr;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Create base name string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::GetHourCycle(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv[0];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->locale_) {
        LOGE("Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->GetHourCycle();

    napi_value result = nullptr;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Create base name string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::GetNumberingSystem(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv[0];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->locale_) {
        LOGE("Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->GetNumberingSystem();

    napi_value result = nullptr;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Create base name string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::GetNumeric(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv[0];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->locale_) {
        LOGE("Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->GetNumeric();
    bool optionBoolValue = (value == "true");
    napi_value result = nullptr;
    status = napi_get_boolean(env, optionBoolValue, &result);
    if (status != napi_ok) {
        LOGE("Create numeric boolean value failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::GetCaseFirst(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv[0];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->locale_) {
        LOGE("Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->GetCaseFirst();
    napi_value result = nullptr;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Create caseFirst string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::ToString(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv[0];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->locale_) {
        LOGE("Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->ToString();

    napi_value result = nullptr;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Create language string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::Maximize(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv[0];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->locale_) {
        LOGE("Get Locale object failed");
        return nullptr;
    }
    std::string localeTag = obj->locale_->Maximize();

    napi_value constructor = nullptr;
    status = napi_get_reference_value(env, *g_constructor, &constructor);
    if (status != napi_ok) {
        LOGE("Get locale constructor reference failed");
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value arg = nullptr;
    status = napi_create_string_utf8(env, localeTag.c_str(), NAPI_AUTO_LENGTH, &arg);
    if (status != napi_ok) {
        LOGE("Create localeTag string failed");
        return nullptr;
    }
    status = napi_new_instance(env, constructor, 1, &arg, &result);
    if (status != napi_ok) {
        LOGE("Create new locale instance failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::Minimize(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv[0];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->locale_) {
        LOGE("Get Locale object failed");
        return nullptr;
    }
    std::string localeTag = obj->locale_->Minimize();

    napi_value constructor = nullptr;
    status = napi_get_reference_value(env, *g_constructor, &constructor);
    if (status != napi_ok) {
        LOGE("Get locale constructor reference failed");
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value arg = nullptr;
    status = napi_create_string_utf8(env, localeTag.c_str(), NAPI_AUTO_LENGTH, &arg);
    if (status != napi_ok) {
        LOGE("Create localeTag string failed");
        return nullptr;
    }
    status = napi_new_instance(env, constructor, 1, &arg, &result);
    if (status != napi_ok) {
        LOGE("Create new locale instance failed");
        return nullptr;
    }
    return result;
}

void SetOptionProperties(napi_env env, napi_value &result, std::map<std::string, std::string> &options,
    const std::string &option)
{
    if (options.count(option) > 0) {
        std::string optionValue = options[option];
        napi_value optionJsValue = nullptr;
        napi_create_string_utf8(env, optionValue.c_str(), NAPI_AUTO_LENGTH, &optionJsValue);
        napi_set_named_property(env, result, option.c_str(), optionJsValue);
    } else {
        napi_value undefined = nullptr;
        napi_get_undefined(env, &undefined);
        napi_set_named_property(env, result, option.c_str(), undefined);
    }
}

void SetIntegerOptionProperties(napi_env env, napi_value &result, std::map<std::string, std::string> &options,
    const std::string &option)
{
    if (options.count(option) > 0) {
        std::string optionValue = options[option];
        napi_value optionJsValue = nullptr;
        int64_t integerValue = std::stoi(optionValue);
        napi_create_int64(env, integerValue, &optionJsValue);
        napi_set_named_property(env, result, option.c_str(), optionJsValue);
    } else {
        napi_value undefined = nullptr;
        napi_get_undefined(env, &undefined);
        napi_set_named_property(env, result, option.c_str(), undefined);
    }
}

void SetBooleanOptionProperties(napi_env env, napi_value &result, std::map<std::string, std::string> &options,
    const std::string &option)
{
    if (options.count(option) > 0) {
        std::string optionValue = options[option];
        bool optionBoolValue = (optionValue == "true");
        napi_value optionJsValue = nullptr;
        napi_get_boolean(env, optionBoolValue, &optionJsValue);
        napi_set_named_property(env, result, option.c_str(), optionJsValue);
    } else {
        napi_value undefined = nullptr;
        napi_get_undefined(env, &undefined);
        napi_set_named_property(env, result, option.c_str(), undefined);
    }
}

napi_value IntlAddon::GetRelativeTimeResolvedOptions(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv[0];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->relativetimefmt_) {
        LOGE("Get RelativeTimeFormat object failed");
        return nullptr;
    }
    napi_value result = nullptr;
    napi_create_object(env, &result);
    std::map<std::string, std::string> options = {};
    obj->relativetimefmt_->GetResolvedOptions(options);
    SetOptionProperties(env, result, options, "locale");
    SetOptionProperties(env, result, options, "style");
    SetOptionProperties(env, result, options, "numeric");
    SetOptionProperties(env, result, options, "numberingSystem");
    return result;
}

napi_value IntlAddon::GetDateTimeResolvedOptions(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv[0];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->datefmt_) {
        LOGE("Get DateTimeFormat object failed");
        return nullptr;
    }
    napi_value result = nullptr;
    napi_create_object(env, &result);
    std::map<std::string, std::string> options = {};
    obj->datefmt_->GetResolvedOptions(options);
    SetOptionProperties(env, result, options, "locale");
    SetOptionProperties(env, result, options, "calendar");
    SetOptionProperties(env, result, options, "dateStyle");
    SetOptionProperties(env, result, options, "timeStyle");
    SetOptionProperties(env, result, options, "hourCycle");
    SetOptionProperties(env, result, options, "timeZone");
    SetOptionProperties(env, result, options, "timeZoneName");
    SetOptionProperties(env, result, options, "numberingSystem");
    SetBooleanOptionProperties(env, result, options, "hour12");
    SetOptionProperties(env, result, options, "weekday");
    SetOptionProperties(env, result, options, "era");
    SetOptionProperties(env, result, options, "year");
    SetOptionProperties(env, result, options, "month");
    SetOptionProperties(env, result, options, "day");
    SetOptionProperties(env, result, options, "hour");
    SetOptionProperties(env, result, options, "minute");
    SetOptionProperties(env, result, options, "second");
    SetOptionProperties(env, result, options, "dayPeriod");
    SetOptionProperties(env, result, options, "localeMatcher");
    SetOptionProperties(env, result, options, "formatMatcher");
    return result;
}

napi_value IntlAddon::GetNumberResolvedOptions(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv[0];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->numberfmt_) {
        LOGE("Get NumberFormat object failed");
        return nullptr;
    }
    napi_value result = nullptr;
    napi_create_object(env, &result);
    std::map<std::string, std::string> options = {};
    obj->numberfmt_->GetResolvedOptions(options);
    SetOptionProperties(env, result, options, "locale");
    SetOptionProperties(env, result, options, "currency");
    SetOptionProperties(env, result, options, "currencySign");
    SetOptionProperties(env, result, options, "currencyDisplay");
    SetOptionProperties(env, result, options, "unit");
    SetOptionProperties(env, result, options, "unitDisplay");
    SetOptionProperties(env, result, options, "signDisplay");
    SetOptionProperties(env, result, options, "compactDisplay");
    SetOptionProperties(env, result, options, "notation");
    SetOptionProperties(env, result, options, "style");
    SetOptionProperties(env, result, options, "numberingSystem");
    SetOptionProperties(env, result, options, "unitUsage");
    SetBooleanOptionProperties(env, result, options, "useGrouping");
    SetIntegerOptionProperties(env, result, options, "minimumIntegerDigits");
    SetIntegerOptionProperties(env, result, options, "minimumFractionDigits");
    SetIntegerOptionProperties(env, result, options, "maximumFractionDigits");
    SetIntegerOptionProperties(env, result, options, "minimumSignificantDigits");
    SetIntegerOptionProperties(env, result, options, "maximumSignificantDigits");
    SetOptionProperties(env, result, options, "localeMatcher");
    return result;
}

napi_value IntlAddon::FormatNumber(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    double number = 0;
    napi_get_value_double(env, argv[0], &number);
    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->numberfmt_) {
        LOGE("Get NumberFormat object failed");
        return nullptr;
    }
    std::string value = obj->numberfmt_->Format(number);
    napi_value result = nullptr;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Create format string failed");
        return nullptr;
    }
    return result;
}

void GetCollatorLocaleMatcher(napi_env env, napi_value options, std::map<std::string, std::string> &map)
{
    GetOptionValue(env, options, "localeMatcher", map);
    auto it = map.find("localeMatcher");
    if (it != map.end()) {
        std::string localeMatcher = it->second;
        if (localeMatcher != "lookup" && localeMatcher != "best fit") {
            LOGE("invalid localeMatcher");
            return;
        }
    } else {
        map.insert(std::make_pair("localeMatcher", "best fit"));
    }
}

void GetCollatorUsage(napi_env env, napi_value options, std::map<std::string, std::string> &map)
{
    GetOptionValue(env, options, "usage", map);
    auto it = map.find("usage");
    if (it != map.end()) {
        std::string usage = it->second;
        if (usage != "sort" && usage != "search") {
            LOGE("invalid usage");
            return;
        }
    } else {
        map.insert(std::make_pair("usage", "sort"));
    }
}

void GetCollatorSensitivity(napi_env env, napi_value options, std::map<std::string, std::string> &map)
{
    GetOptionValue(env, options, "sensitivity", map);
    auto it = map.find("sensitivity");
    if (it != map.end()) {
        std::string sensitivity = it->second;
        if (sensitivity != "base" && sensitivity != "accent" && sensitivity != "case" && sensitivity != "variant") {
            LOGE("invalid sensitivity");
            return;
        }
    } else {
        map.insert(std::make_pair("sensitivity", "variant"));
    }
}

void GetCollatorIgnorePunctuation(napi_env env, napi_value options, std::map<std::string, std::string> &map)
{
    GetBoolOptionValue(env, options, "ignorePunctuation", map);
    auto it = map.find("ignorePunctuation");
    if (it != map.end()) {
        std::string ignorePunctuation = it->second;
        if (ignorePunctuation != "true" && ignorePunctuation != "false") {
            LOGE("invalid ignorePunctuation");
            return;
        }
    } else {
        map.insert(std::make_pair("ignorePunctuation", "false"));
    }
}

void GetCollatorNumeric(napi_env env, napi_value options, std::map<std::string, std::string> &map)
{
    GetBoolOptionValue(env, options, "numeric", map);
    auto it = map.find("numeric");
    if (it != map.end()) {
        std::string numeric = it->second;
        if (numeric != "true" && numeric != "false") {
            LOGE("invalid numeric");
            return;
        }
    }
}

void GetCollatorCaseFirst(napi_env env, napi_value options, std::map<std::string, std::string> &map)
{
    GetOptionValue(env, options, "caseFirst", map);
    auto it = map.find("caseFirst");
    if (it != map.end()) {
        std::string caseFirst = it->second;
        if (caseFirst != "upper" && caseFirst != "lower" && caseFirst != "false") {
            LOGE("invalid caseFirst");
            return;
        }
    }
}

void GetCollatorCollation(napi_env env, napi_value options, std::map<std::string, std::string> &map)
{
    GetOptionValue(env, options, "collation", map);
    auto it = map.find("collation");
    if (it != map.end()) {
        std::string collation = it->second;
        std::set<std::string> validCollation;
        validCollation.insert("big5han");
        validCollation.insert("compat");
        validCollation.insert("dict");
        validCollation.insert("direct");
        validCollation.insert("ducet");
        validCollation.insert("eor");
        validCollation.insert("gb2312");
        validCollation.insert("phonebk");
        validCollation.insert("phonetic");
        validCollation.insert("pinyin");
        validCollation.insert("reformed");
        validCollation.insert("searchjl");
        validCollation.insert("stroke");
        validCollation.insert("trad");
        validCollation.insert("unihan");
        validCollation.insert("zhuyin");
        if (validCollation.find(collation) == validCollation.end()) {
            map["collation"] = "default";
        }
    }
}

void GetCollatorOptionValue(napi_env env, napi_value options, std::map<std::string, std::string> &map)
{
    GetCollatorLocaleMatcher(env, options, map);
    GetCollatorUsage(env, options, map);
    GetCollatorSensitivity(env, options, map);
    GetCollatorIgnorePunctuation(env, options, map);
    GetCollatorNumeric(env, options, map);
    GetCollatorCaseFirst(env, options, map);
    GetCollatorCollation(env, options, map);
}

napi_value IntlAddon::InitCollator(napi_env env, napi_value exports)
{
    napi_status status = napi_ok;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("compare", CompareString),
        DECLARE_NAPI_FUNCTION("resolvedOptions", GetCollatorResolvedOptions)
    };

    napi_value constructor;
    status = napi_define_class(env, "Collator", NAPI_AUTO_LENGTH, CollatorConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        LOGE("Define class failed when InitCollator");
        return nullptr;
    }

    status = napi_set_named_property(env, exports, "Collator", constructor);
    if (status != napi_ok) {
        LOGE("Set property failed when InitCollator");
        return nullptr;
    }
    return exports;
}

napi_value IntlAddon::CollatorConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    std::vector<std::string> localeTags;
    if (argc > 0) {
        napi_valuetype valueType = napi_valuetype::napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        bool isArray = false;
        napi_is_array(env, argv[0], &isArray);
        if (valueType == napi_valuetype::napi_string) {
            GetLocaleTags(env, argv[0], localeTags);
        } else if (isArray) {
            uint32_t arrayLength = 0;
            napi_get_array_length(env, argv[0], &arrayLength);
            napi_value element = nullptr;
            for (uint32_t i = 0; i < arrayLength; i++) {
                napi_get_element(env, argv[0], i, &element);
                GetLocaleTags(env, element, localeTags);
            }
        }
    }
    std::map<std::string, std::string> map = {};
    if (argc > 1) {
        GetCollatorOptionValue(env, argv[1], map);
    }
    std::unique_ptr<IntlAddon> obj = nullptr;
    obj = std::make_unique<IntlAddon>();
    status =
        napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()), IntlAddon::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("Wrap IntlAddon failed");
        return nullptr;
    }
    if (!obj->InitCollatorContext(env, info, localeTags, map)) {
        LOGE("Init DateTimeFormat failed");
        return nullptr;
    }
    obj.release();
    return thisVar;
}

bool IntlAddon::InitCollatorContext(napi_env env, napi_callback_info info, std::vector<std::string> localeTags,
    std::map<std::string, std::string> &map)
{
    napi_value global = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok) {
        LOGE("Get global failed");
        return false;
    }
    env_ = env;
    collator_ = std::make_unique<Collator>(localeTags, map);

    return collator_ != nullptr;
}

bool GetStringParameter(napi_env env, napi_value value, std::vector<char> &buf)
{
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return false;
    }
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, value, nullptr, 0, &len);
    if (status != napi_ok) {
        LOGE("Get first length failed");
        return false;
    }
    buf.resize(len + 1);
    status = napi_get_value_string_utf8(env, value, buf.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGE("Get first failed");
        return false;
    }

    return true;
}

napi_value IntlAddon::FormatRelativeTime(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_status status;
    double number;
    status = napi_get_value_double(env, argv[0], &number);
    if (status != napi_ok) {
        LOGE("Get number failed");
        return nullptr;
    }
    std::vector<char> unit;
    if (!GetStringParameter(env, argv[1], unit)) {
        return nullptr;
    }
    IntlAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->relativetimefmt_) {
        LOGE("Get RelativeTimeFormat object failed");
        return nullptr;
    }
    std::string value = obj->relativetimefmt_->Format(number, unit.data());
    napi_value result = nullptr;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Create format string failed");
        return nullptr;
    }
    return result;
}

void IntlAddon::FillInArrayElement(napi_env env, napi_value &result, napi_status &status,
    const std::vector<std::vector<std::string>> &timeVector)
{
    for (size_t i = 0; i < timeVector.size(); i++) {
        napi_value value = nullptr;
        status = napi_create_string_utf8(env, timeVector[i][1].c_str(), NAPI_AUTO_LENGTH, &value);
        if (status != napi_ok) {
            LOGE("Failed to create string item.");
            return;
        }
        napi_value type = nullptr;
        status = napi_create_string_utf8(env, timeVector[i][0].c_str(), NAPI_AUTO_LENGTH, &type);
        if (status != napi_ok) {
            LOGE("Failed to create string item.");
            return;
        }
        napi_value unit = nullptr;
        size_t unitIndex = 2;
        if (timeVector[i].size() > unitIndex) {
            status = napi_create_string_utf8(env, timeVector[i][unitIndex].c_str(), NAPI_AUTO_LENGTH, &unit);
            if (status != napi_ok) {
                LOGE("Failed to create string item.");
                return;
            }
        } else {
            napi_get_undefined(env, &unit);
        }
        napi_value formatInfo;
        status = napi_create_object(env, &formatInfo);
        if (status != napi_ok) {
            LOGE("Failed to create format info object.");
            return;
        }
        napi_set_named_property(env, formatInfo, "type", type);
        napi_set_named_property(env, formatInfo, "value", value);
        napi_set_named_property(env, formatInfo, "unit", unit);
        status = napi_set_element(env, result, i, formatInfo);
        if (status != napi_ok) {
            LOGE("Failed to set array item");
            return;
        }
    }
}

napi_value IntlAddon::FormatToParts(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    double number = 0;
    napi_get_value_double(env, argv[0], &number);
    std::vector<char> unit;
    if (!GetStringParameter(env, argv[1], unit)) {
        return nullptr;
    }
    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->relativetimefmt_) {
        LOGE("Get RelativeTimeFormat object failed");
        return nullptr;
    }
    std::vector<std::vector<std::string>> timeVector;
    obj->relativetimefmt_->FormatToParts(number, unit.data(), timeVector);
    napi_value result = nullptr;
    status = napi_create_array_with_length(env, timeVector.size(), &result);
    if (status != napi_ok) {
        LOGE("Failed to create array");
        return nullptr;
    }
    FillInArrayElement(env, result, status, timeVector);
    return result;
}

napi_value IntlAddon::CompareString(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    std::vector<char> first;
    if (!GetStringParameter(env, argv[0], first)) {
        return nullptr;
    }

    std::vector<char> second;
    if (!GetStringParameter(env, argv[1], second)) {
        return nullptr;
    }

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->collator_) {
        LOGE("Get Collator object failed");
        return nullptr;
    }

    CompareResult compareResult = obj->collator_->Compare(first.data(), second.data());
    napi_value result = nullptr;
    status = napi_create_int32(env, compareResult, &result);
    if (status != napi_ok) {
        LOGE("Create compare result failed");
        return nullptr;
    }

    return result;
}

napi_value IntlAddon::GetCollatorResolvedOptions(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv[0];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->collator_) {
        LOGE("Get Collator object failed");
        return nullptr;
    }
    napi_value result = nullptr;
    napi_create_object(env, &result);
    std::map<std::string, std::string> options = {};
    obj->collator_->ResolvedOptions(options);
    SetOptionProperties(env, result, options, "localeMatcher");
    SetOptionProperties(env, result, options, "locale");
    SetOptionProperties(env, result, options, "usage");
    SetOptionProperties(env, result, options, "sensitivity");
    SetBooleanOptionProperties(env, result, options, "ignorePunctuation");
    SetBooleanOptionProperties(env, result, options, "numeric");
    SetOptionProperties(env, result, options, "caseFirst");
    SetOptionProperties(env, result, options, "collation");
    return result;
}

void GetPluralRulesType(napi_env env, napi_value options, std::map<std::string, std::string> &map)
{
    GetOptionValue(env, options, "type", map);
    auto it = map.find("type");
    if (it != map.end()) {
        std::string type = it->second;
        if (type != "cardinal" && type != "ordinal") {
            LOGE("invalid type");
            return;
        }
    } else {
        map.insert(std::make_pair("type", "cardinal"));
    }
}

void GetPluralRulesInteger(napi_env env, napi_value options, std::map<std::string, std::string> &map)
{
    GetIntegerOptionValue(env, options, "minimumIntegerDigits", map);
    auto it = map.find("minimumIntegerDigits");
    if (it != map.end()) {
        std::string minimumIntegerDigits = it->second;
        int n = std::stoi(minimumIntegerDigits);
        if (n < 1 || n > 21) {  // the valid range of minimumIntegerDigits is [1, 21]
            LOGE("invalid minimumIntegerDigits");
            return;
        }
    } else {
        map.insert(std::make_pair("minimumIntegerDigits", std::to_string(1)));
    }
}

void GetPluralRulesFractions(napi_env env, napi_value options, std::map<std::string, std::string> &map)
{
    GetIntegerOptionValue(env, options, "minimumFractionDigits", map);
    auto it = map.find("minimumFractionDigits");
    if (it != map.end()) {
        std::string minimumFractionDigits = it->second;
        int n = std::stoi(minimumFractionDigits);
        if (n < 0 || n > 20) {  // the valid range of minimumFractionDigits is [0, 20]
            LOGE("invalid minimumFractionDigits");
            return;
        }
    }

    GetIntegerOptionValue(env, options, "maximumFractionDigits", map);
    it = map.find("maximumFractionDigits");
    if (it != map.end()) {
        std::string maximumFractionDigits = it->second;
        int n = std::stoi(maximumFractionDigits);
        if (n < 0 || n > 20) {  // the valid range of maximumFractionDigits is [0, 20]
            LOGE("invalid maximumFractionDigits");
            return;
        }
    }
}

void GetPluralRulesSignificant(napi_env env, napi_value options, std::map<std::string, std::string> &map)
{
    int minSignificant = -1;
    GetIntegerOptionValue(env, options, "minimumSignificantDigits", map);
    auto it = map.find("minimumSignificantDigits");
    if (it != map.end()) {
        std::string minSignificantStr = it->second;
        int minSignificantInt = std::stoi(minSignificantStr);
        // the valid range of minSignificantInt is [1, 21]
        if (minSignificantInt < 1 || minSignificantInt > 21) {
            LOGE("invalid minimumSignificantDigits");
            return;
        }
        minSignificant = minSignificantInt;
    } else {
        minSignificant = 1;
    }

    GetIntegerOptionValue(env, options, "maximumSignificantDigits", map);
    it = map.find("maximumSignificantDigits");
    if (it != map.end()) {
        std::string maxSignificantStr = it->second;
        int maxSignificant = std::stoi(maxSignificantStr);
        // the valid range of minSignificant is [minSignificant, 21]
        if (maxSignificant < minSignificant || maxSignificant > 21) {
            LOGE("invalid maximumSignificantDigits");
            return;
        }
    }
}

void GetPluralRulesOptionValues(napi_env env, napi_value options, std::map<std::string, std::string> &map)
{
    GetCollatorLocaleMatcher(env, options, map);
    GetPluralRulesType(env, options, map);
    GetPluralRulesInteger(env, options, map);
    GetPluralRulesFractions(env, options, map);
    GetPluralRulesSignificant(env, options, map);
}

napi_value IntlAddon::InitPluralRules(napi_env env, napi_value exports)
{
    napi_status status = napi_ok;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("select", Select)
    };

    napi_value constructor = nullptr;
    status = napi_define_class(env, "PluralRules", NAPI_AUTO_LENGTH, PluralRulesConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        LOGE("Define class failed when InitPluralRules");
        return nullptr;
    }

    status = napi_set_named_property(env, exports, "PluralRules", constructor);
    if (status != napi_ok) {
        LOGE("Set property failed when InitPluralRules");
        return nullptr;
    }
    return exports;
}

napi_value IntlAddon::PluralRulesConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    std::vector<std::string> localeTags;
    if (argc > 0) {
        napi_typeof(env, argv[0], &valueType);
        bool isArray = false;
        napi_is_array(env, argv[0], &isArray);
        if (valueType == napi_valuetype::napi_string) {
            GetLocaleTags(env, argv[0], localeTags);
        } else if (isArray) {
            uint32_t arrayLength = 0;
            napi_get_array_length(env, argv[0], &arrayLength);
            napi_value element = nullptr;
            for (uint32_t i = 0; i < arrayLength; i++) {
                napi_get_element(env, argv[0], i, &element);
                GetLocaleTags(env, element, localeTags);
            }
        }
    }
    std::map<std::string, std::string> map = {};
    if (argc > 1) {
        GetPluralRulesOptionValues(env, argv[1], map);
    }
    std::unique_ptr<IntlAddon> obj = nullptr;
    obj = std::make_unique<IntlAddon>();
    status =
        napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()), IntlAddon::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("Wrap IntlAddon failed");
        return nullptr;
    }
    if (!obj->InitPluralRulesContext(env, info, localeTags, map)) {
        LOGE("Init DateTimeFormat failed");
        return nullptr;
    }
    obj.release();
    return thisVar;
}

bool IntlAddon::InitPluralRulesContext(napi_env env, napi_callback_info info, std::vector<std::string> localeTags,
    std::map<std::string, std::string> &map)
{
    napi_value global = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok) {
        LOGE("Get global failed");
        return false;
    }
    env_ = env;
    pluralrules_ = std::make_unique<PluralRules>(localeTags, map);

    return pluralrules_ != nullptr;
}

napi_value IntlAddon::Select(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }

    double number = 0;
    napi_status status = napi_get_value_double(env, argv[0], &number);
    if (status != napi_ok) {
        LOGE("Get number failed");
        return nullptr;
    }

    IntlAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->pluralrules_) {
        LOGE("Get PluralRules object failed");
        return nullptr;
    }

    std::string res = obj->pluralrules_->Select(number);
    napi_value result = nullptr;
    status = napi_create_string_utf8(env, res.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("get select result failed");
        return nullptr;
    }
    return result;
}

napi_value Init(napi_env env, napi_value exports)
{
    napi_value val = IntlAddon::InitLocale(env, exports);
    val = IntlAddon::InitDateTimeFormat(env, val);
    val = IntlAddon::InitNumberFormat(env, val);
    val = IntlAddon::InitCollator(env, val);
    val = IntlAddon::InitRelativeTimeFormat(env, val);
    return IntlAddon::InitPluralRules(env, val);
}

static napi_module g_intlModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "intl",
    .nm_priv = nullptr,
    .reserved = { 0 }
};

#ifdef ANDROID_PLATFORM
static void INTLPluginJniRegister()
{
    const char className[] = "ohos.ace.plugin.intlplugin.INTLPlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&Plugin::INTLPluginJni::Register, className);
}
#endif

extern "C" __attribute__((constructor)) void INTLPluginRegister()
{
#ifdef ANDROID_PLATFORM
    INTLPluginJniRegister();
#else
    InitIcuData();
#endif
    napi_module_register(&g_intlModule);
}
} // namespace I18n
} // namespace Global
} // namespace OHOS
