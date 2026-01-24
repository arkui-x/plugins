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
#include "i18n_system_addon.h"

#include "error_util.h"
#include "locale_config.h"
#include "locale_info.h"
#include "log.h"
#include "napi_utils.h"
#include "utils.h"

namespace OHOS {
namespace Global {
namespace I18n {
const std::string I18nSystemAddon::NAME_OF_TEMPERATURE = "type";
const std::string I18nSystemAddon::TYPE_OF_TEMPERATURE = "TemperatureType";
const int SIZE_TWO = 2;

I18nSystemAddon::I18nSystemAddon() {}

I18nSystemAddon::~I18nSystemAddon() {}

napi_value I18nSystemAddon::GetSystemLanguages(napi_env env, napi_callback_info info)
{
    std::unordered_set<std::string> systemLanguages = LocaleConfig::GetSystemLanguages();
    napi_value result = nullptr;
    napi_status status = napi_create_array_with_length(env, systemLanguages.size(), &result);
    if (status != napi_ok) {
        LOGE("GetSystemLanguages: Failed to create array");
        return NAPIUtils::CreateString(env, "");
    }
    size_t pos = 0;
    for (const auto& language : systemLanguages) {
        napi_value value = nullptr;
        status = napi_create_string_utf8(env, language.c_str(), NAPI_AUTO_LENGTH, &value);
        if (status != napi_ok) {
            LOGE("GetSystemLanguages: Failed to create string item");
            return NAPIUtils::CreateString(env, "");
        }
        status = napi_set_element(env, result, pos, value);
        if (status != napi_ok) {
            LOGE("GetSystemLanguages: Failed to set array item");
            return NAPIUtils::CreateString(env, "");
        }
        pos++;
    }
    return result;
}

napi_value I18nSystemAddon::GetSystemCountriesWithError(napi_env env, napi_callback_info info)
{
    return I18nSystemAddon::GetSystemCountriesImpl(env, info, true);
}

napi_value I18nSystemAddon::IsSuggestedWithError(napi_env env, napi_callback_info info)
{
    return I18nSystemAddon::IsSuggestedImpl(env, info, true);
}

napi_value I18nSystemAddon::GetPreferredLanguageList(napi_env env, napi_callback_info info)
{
    std::vector<std::string> languageList = LocaleConfig::GetPreferredLanguages();
    napi_value result = nullptr;
    napi_status status = napi_create_array_with_length(env, languageList.size(), &result);
    if (status != napi_ok) {
        LOGE("getPreferrdLanguageList: create array failed");
        return NAPIUtils::CreateString(env, "");
    }

    int pos = 0;
    for (auto& language : languageList) {
        napi_value value = nullptr;
        status = napi_create_string_utf8(env, language.c_str(), NAPI_AUTO_LENGTH, &value);
        if (status != napi_ok) {
            LOGE("getPreferrdLanguageList: create string failed");
            return NAPIUtils::CreateString(env, "");
        }
        status = napi_set_element(env, result, pos, value);
        if (status != napi_ok) {
            LOGE("GetPreferredLanguageList: set array item failed");
            return NAPIUtils::CreateString(env, "");
        }
        pos++;
    }
    return result;
}

napi_value I18nSystemAddon::GetFirstPreferredLanguage(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    std::string languages = LocaleConfig::GetFirstPreferredLanguage();
    napi_status status = napi_create_string_utf8(env, languages.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("getFirstPreferrdLanguage: create string result failed");
        return NAPIUtils::CreateString(env, "");
    }
    return result;
}

napi_value I18nSystemAddon::GetUsingLocalDigitAddon(napi_env env, napi_callback_info info)
{
    bool res = LocaleConfig::GetUsingLocalDigit();
    napi_value value = nullptr;
    napi_status status = napi_get_boolean(env, res, &value);
    if (status != napi_ok) {
        return NAPIUtils::CreateString(env, "");
    }
    return value;
}

napi_value I18nSystemAddon::GetSystemCountriesImpl(napi_env env, napi_callback_info info, bool throwError)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok) {
        return NAPIUtils::CreateString(env, "");
    } else if (argc < 1) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, throwError);
    }
    NAPIUtils::VerifyType(env, "language", "string", argv[0]);
    int32_t code = 0;
    std::string language = NAPIUtils::GetString(env, argv[0], code);
    if (code != 0) {
        LOGE("GetSystemCountriesImpl: Get argv[0] failed.");
        return NAPIUtils::CreateString(env, "");
    }
    std::unordered_set<std::string> systemCountries = LocaleConfig::GetSystemCountries(language);
    napi_value result = nullptr;
    status = napi_create_array_with_length(env, systemCountries.size(), &result);
    if (status != napi_ok) {
        LOGE("GetSystemCountriesImpl: Failed to create array");
        return NAPIUtils::CreateString(env, "");
    }
    size_t pos = 0;
    for (const auto& country : systemCountries) {
        napi_value value = nullptr;
        status = napi_create_string_utf8(env, country.c_str(), NAPI_AUTO_LENGTH, &value);
        if (status != napi_ok) {
            LOGE("GetSystemCountries: Failed to create string item");
            return NAPIUtils::CreateString(env, "");
        }
        status = napi_set_element(env, result, pos, value);
        if (status != napi_ok) {
            LOGE("GetSystemCountries: Failed to set array item");
            return NAPIUtils::CreateString(env, "");
        }
        pos++;
    }
    return result;
}

napi_value I18nSystemAddon::IsSuggestedImpl(napi_env env, napi_callback_info info, bool throwError)
{
    size_t argc = SIZE_TWO;
    napi_value argv[SIZE_TWO] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok) {
        return NAPIUtils::CreateString(env, "");
    } else if (argc < 1) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, throwError);
        return NAPIUtils::CreateString(env, "");
    }
    NAPIUtils::VerifyType(env, "language", "string", argv[0]);
    int32_t code = 0;
    std::string language = NAPIUtils::GetString(env, argv[0], code);
    if (code != 0) {
        LOGE("IsSuggestedImpl: Failed to get string argv[0].");
        return NAPIUtils::CreateString(env, "");
    }
    bool isSuggested = false;
    if (NAPIUtils::CheckNapiIsNull(env, argv[1])) {
        std::string region = NAPIUtils::GetString(env, argv[1], code);
        if (code != 0) {
            LOGE("IsSuggestedImpl: Failed to get string argv[1].");
            return NAPIUtils::CreateString(env, "");
        }
        isSuggested = LocaleConfig::IsSuggested(language, region);
    } else {
        isSuggested = LocaleConfig::IsSuggested(language);
    }
    napi_value result = nullptr;
    status = napi_get_boolean(env, isSuggested, &result);
    if (status != napi_ok) {
        LOGE("Create case first boolean value failed");
        return NAPIUtils::CreateString(env, "");
    }
    return result;
}

napi_value I18nSystemAddon::GetTemperatureType(napi_env env, napi_callback_info info)
{
    TemperatureType type = LocaleConfig::GetTemperatureType();
    int32_t value = static_cast<int32_t>(type);
    return NAPIUtils::CreateNumber(env, value);
}

napi_value I18nSystemAddon::GetTemperatureName(napi_env env, napi_callback_info info)
{
    TemperatureType type = TemperatureType::CELSIUS;
    if (!ParseTemperatureType(env, info, type)) {
        LOGE("I18nSystemAddon::GetTemperatureName: parse temperature type failed.");
        return NAPIUtils::CreateString(env, "");
    }
    std::string temperatureName = LocaleConfig::GetTemperatureName(type);
    if (temperatureName.empty()) {
        ErrorUtil::NapiThrow(env, I18N_NOT_VALID, true);
        return NAPIUtils::CreateString(env, "");
    }
    return NAPIUtils::CreateString(env, temperatureName);
}

bool I18nSystemAddon::ParseTemperatureType(napi_env env, napi_callback_info info, TemperatureType& type)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("I18nSystemAddon::ParseTemperatureType: get cb info failed.");
        return false;
    }
    if (argc < 1) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, true);
        return false;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    status = napi_typeof(env, argv[0], &valueType);
    if (status != napi_ok) {
        LOGE("I18nSystemAddon::ParseTemperatureType: get value type failed.");
        return false;
    }
    if (valueType != napi_valuetype::napi_number) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, true);
        return false;
    }
    int32_t temperatureType = 0;
    status = napi_get_value_int32(env, argv[0], &temperatureType);
    if (status != napi_ok) {
        LOGE("I18nSystemAddon::ParseTemperatureType: get temperature type failed.");
        return false;
    }
    type = static_cast<TemperatureType>(temperatureType);
    return true;
}

napi_value I18nSystemAddon::GetFirstDayOfWeek(napi_env env, napi_callback_info info)
{
    WeekDay type = LocaleConfig::GetFirstDayOfWeek();
    int32_t value = static_cast<int32_t>(type);
    return NAPIUtils::CreateNumber(env, value);
}

napi_value I18nSystemAddon::GetSimplifiedLanguage(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("GetSimplifiedLanguage get parameter fail");
        return NAPIUtils::CreateString(env, "");
    }

    std::string simplifiedLanguage = "";
    if (NAPIUtils::CheckNapiIsNull(env, argv[0])) {
        napi_valuetype valueType = napi_valuetype::napi_undefined;
        status = napi_typeof(env, argv[0], &valueType);
        if (status != napi_ok) {
            return NAPIUtils::CreateString(env, "");
        }
        if (valueType != napi_valuetype::napi_string) {
            LOGE("GetSimplifiedLanguage Parameter type is not string");
            ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, true);
            return NAPIUtils::CreateString(env, "");
        }
        int code = 0;
        std::string language = NAPIUtils::GetString(env, argv[0], code);
        if (code == 1) {
            LOGE("GetSimplifiedLanguage can't get string from js param");
            return NAPIUtils::CreateString(env, "");
        }
        code = 0;
        simplifiedLanguage = LocaleConfig::GetSimplifiedLanguage(language, code);
        if (code != 0) {
            LOGE("GetSimplifiedLanguage: language is not valid");
            ErrorUtil::NapiThrow(env, I18N_NOT_VALID, true);
            return NAPIUtils::CreateString(env, "");
        }
    } else {
        simplifiedLanguage = LocaleConfig::GetSimplifiedSystemLanguage();
    }

    napi_value result = NAPIUtils::CreateString(env, simplifiedLanguage);
    if (!result) {
        LOGE("GetSimplifiedLanguage: Failed to create string item");
    }
    return result;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS