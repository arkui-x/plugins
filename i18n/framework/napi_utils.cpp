/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "napi_utils.h"

#include "error_util.h"
#include "intl_addon.h"
#include "log.h"

namespace OHOS {
namespace Global {
namespace I18n {
static const std::string EMPTY_STRING = "";

std::string NAPIUtils::GetString(napi_env &env, napi_value &value, int32_t &code)
{
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, value, nullptr, 0, &len);
    if (status != napi_ok) {
        LOGE("Get string failed");
        code = 1;
        return EMPTY_STRING;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, value, buf.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGE("Create string failed");
        code = 1;
        return EMPTY_STRING;
    }
    return buf.data();
}

bool NAPIUtils::GetStringArrayFromJsParam(napi_env env, napi_value &jsArray, const std::string& valueName,
    std::vector<std::string> &strArray)
{
    if (jsArray == nullptr) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, valueName, "", true);
        return false;
    }
    bool isArray = false;
    napi_status status = napi_is_array(env, jsArray, &isArray);
    if (status != napi_ok) {
        return false;
    } else if (!isArray) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, valueName, "an Array", true);
        return false;
    }
    uint32_t arrayLength = 0;
    napi_get_array_length(env, jsArray, &arrayLength);
    napi_value element = nullptr;
    int32_t code = 0;
    for (uint32_t i = 0; i < arrayLength; ++i) {
        napi_get_element(env, jsArray, i, &element);
        std::string str = GetString(env, element, code);
        if (code != 0) {
            napi_throw_type_error(env, nullptr, "GetStringArrayFromJsParam: Failed to obtain the parameter.");
            return false;
        }
        strArray.push_back(str);
    }
    return true;
}

napi_value NAPIUtils::CreateString(napi_env env, const std::string &str)
{
    napi_value result;
    napi_status status = napi_create_string_utf8(env, str.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("create string js variable failed.");
        return nullptr;
    }
    return result;
}

void NAPIUtils::VerifyType(napi_env env, const std::string& valueName, const std::string& type,
    napi_value argv)
{
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv, &valueType);
    if (type == "string" && valueType != napi_valuetype::napi_string) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, valueName, type, true);
    } else if (type == "number" && valueType != napi_valuetype::napi_number) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, valueName, type, true);
    }
}

napi_value NAPIUtils::CreateNumber(napi_env env, const int32_t& num)
{
    napi_value result;
    napi_status status = napi_create_int32(env, num, &result);
    if (status != napi_ok) {
        LOGE("create number js variable failed.");
        return nullptr;
    }
    return result;
}

bool NAPIUtils::CheckNapiIsNull(napi_env env, napi_value value)
{
    if (value != nullptr) {
        napi_valuetype valueType = napi_valuetype::napi_undefined;
        napi_status status = napi_typeof(env, value, &valueType);
        if (status != napi_ok) {
            return false;
        }
        if (valueType != napi_valuetype::napi_undefined && valueType != napi_valuetype::napi_null) {
            return true;
        }
    }
    return false;
}

double NAPIUtils::GetDouble(napi_env env, napi_value value, int32_t& code)
{
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_status status = napi_typeof(env, value, &valueType);
    if (status != napi_ok) {
        LOGE("NAPIUtils::GetDouble Get napi_value type failed");
        code = 1;
        return 0;
    }
    if (valueType != napi_valuetype::napi_number) {
        LOGE("NAPIUtils::GetDouble check napi_type_number failed");
        code = 1;
        return 0;
    }
    double result = 0;
    status = napi_get_value_double(env, value, &result);
    if (status != napi_ok) {
        LOGE("NAPIUtils::GetDouble Get double failed");
        code = 1;
        return 0;
    }
    return result;
}

int32_t NAPIUtils::GetInt(napi_env env, napi_value value, int32_t& code)
{
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_status status = napi_typeof(env, value, &valueType);
    if (status != napi_ok) {
        LOGE("NAPIUtils::GetInt: Get napi_value type failed");
        code = 1;
        return 0;
    }
    if (valueType != napi_valuetype::napi_number) {
        LOGE("NAPIUtils::GetInt: check napi_type_number failed");
        code = 1;
        return 0;
    }
    int32_t result = 0;
    status = napi_get_value_int32(env, value, &result);
    if (status != napi_ok) {
        LOGE("NAPIUtils::GetInt: get int value failed");
        code = 1;
        return 0;
    }
    return result;
}

napi_status NAPIUtils::SetEnumValue(napi_env env, napi_value enumObj, const std::string& enumName,
    int32_t enumVal)
{
    napi_value name = nullptr;
    napi_status status = napi_create_string_utf8(env, enumName.c_str(), NAPI_AUTO_LENGTH, &name);
    if (status != napi_ok) {
        LOGE("VariableConvertor::SetEnumValue: create string %{public}s failed.", enumName.c_str());
        return status;
    }
    napi_value value = nullptr;
    status = napi_create_int32(env, enumVal, &value);
    if (status != napi_ok) {
        LOGE("VariableConvertor::SetEnumValue: create int32 %{public}d failed.", enumVal);
        return status;
    }
    status = napi_set_property(env, enumObj, name, value);
    if (status != napi_ok) {
        LOGE("VariableConvertor::SetEnumValue: set property %{public}s failed.", enumName.c_str());
        return status;
    }
    status = napi_set_property(env, enumObj, value, name);
    if (status != napi_ok) {
        LOGE("VariableConvertor::SetEnumValue: set property %{public}d failed.", enumVal);
        return status;
    }
    return napi_ok;
}

napi_value NAPIUtils::GetBuiltinsLocaleConstructor(napi_env env)
{
    napi_value global = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok || global == nullptr) {
        LOGE("NAPIUtils::GetBuiltinsLocaleConstructor: Get global failed.");
        return nullptr;
    }

    napi_value intl = nullptr;
    status = napi_get_named_property(env, global, "Intl", &intl);
    if (status != napi_ok || intl == nullptr) {
        LOGE("NAPIUtils::GetBuiltinsLocaleConstructor: Load intl failed.");
        return nullptr;
    }

    napi_value localeConstructor = nullptr;
    status = napi_get_named_property(env, intl, "Locale", &localeConstructor);
    if (status != napi_ok || localeConstructor == nullptr) {
        LOGE("NAPIUtils::GetBuiltinsLocaleConstructor: Get locale constructor failed.");
        return nullptr;
    }
    return localeConstructor;
}

napi_value NAPIUtils::CreateBuiltinsLocaleObject(napi_env env, const std::string& locale)
{
    napi_value param = nullptr;
    napi_status status = napi_create_string_utf8(env, locale.c_str(), NAPI_AUTO_LENGTH, &param);
    if (status != napi_ok || param == nullptr) {
        LOGE("NAPIUtils::CreateBuiltinsLocaleObject: Create string failed.");
        return nullptr;
    }

    napi_value localeConstructor = NAPIUtils::GetBuiltinsLocaleConstructor(env);
    if (localeConstructor == nullptr) {
        LOGE("NAPIUtils::CreateBuiltinsLocaleObject: Get constructor failed.");
        return nullptr;
    }

    size_t argc = 1;
    napi_value argv[1] = { param };
    napi_value builtinsLocale = nullptr;
    status = napi_new_instance(env, localeConstructor, argc, argv, &builtinsLocale);
    if (status != napi_ok || builtinsLocale == nullptr) {
        LOGE("NAPIUtils::CreateBuiltinsLocaleObject: Create locale failed.");
        return nullptr;
    }
    return builtinsLocale;
}

bool NAPIUtils::IsBuiltinsLocale(napi_env env, napi_value locale)
{
    napi_value localeConstructor = NAPIUtils::GetBuiltinsLocaleConstructor(env);
    if (localeConstructor == nullptr) {
        LOGE("NAPIUtils::IsBuiltinsLocale: Get constructor failed.");
        return false;
    }

    bool isBuiltinsLocale = false;
    napi_status status = napi_instanceof(env, locale, localeConstructor, &isBuiltinsLocale);
    if (status != napi_ok) {
        LOGE("NAPIUtils::IsBuiltinsLocale: Get instance of locale failed.");
        return false;
    }
    return isBuiltinsLocale;
}

LocaleType NAPIUtils::GetLocaleType(napi_env env, napi_value locale)
{
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_status status = napi_typeof(env, locale, &valueType);
    if (status != napi_ok) {
        LOGE("NAPIUtils::GetLocaleType: Failed to get type of intlLocale.");
        return LocaleType::INVALID;
    } else if (valueType != napi_valuetype::napi_object) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, true);
        return LocaleType::INVALID;
    }
    bool isBuiltinsLocale = NAPIUtils::IsBuiltinsLocale(env, locale);
    if (isBuiltinsLocale) {
        return LocaleType::BUILTINS_LOCALE;
    }
    bool isLocaleInfo = IntlAddon::IsLocaleInfo(env, locale);
    if (isLocaleInfo) {
        return LocaleType::LOCALE_INFO;
    }
    ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, true);
    return LocaleType::INVALID;
}

std::shared_ptr<LocaleInfo> NAPIUtils::ParseLocaleInfo(napi_env env, napi_value localeInfo)
{
    IntlAddon* intlAddon = nullptr;
    napi_status status = napi_unwrap(env, localeInfo, reinterpret_cast<void **>(&intlAddon));
    if (status != napi_ok) {
        LOGE("NAPIUtils::ParseLocaleInfo: Failed to unwrap localeInfo.");
        return nullptr;
    } else if (!intlAddon) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, "locale", "intl.Locale", true);
        return nullptr;
    } else if (!intlAddon->GetLocaleInfo()) {
        ErrorUtil::NapiThrow(env, I18N_NOT_VALID, "locale", "a valid intl.Locale", true);
        return nullptr;
    }
    return intlAddon->GetLocaleInfo();
}

std::string NAPIUtils::ParseBuiltinsLocale(napi_env env, napi_value builtinsLocale)
{
    napi_value func = nullptr;
    napi_status status = napi_get_named_property(env, builtinsLocale, "toString", &func);
    if (status != napi_ok || func == nullptr) {
        LOGE("NAPIUtils::ParseBuiltinsLocale: Get function toString failed.");
        return "";
    }
    napi_value locale = nullptr;
    status = napi_call_function(env, builtinsLocale, func, 0, nullptr, &locale);
    if (status != napi_ok || locale == nullptr) {
        LOGE("NAPIUtils::ParseBuiltinsLocale: Call function failed.");
        return "";
    }
    int32_t code = 0;
    std::string localeTag = NAPIUtils::GetString(env, locale, code);
    if (code != 0) {
        LOGE("NAPIUtils::ParseBuiltinsLocale: Get string failed.");
        return "";
    }
    return localeTag;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS