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
#include "i18n_timezone_addon.h"

#include "character.h"
#include "error_util.h"
#include "log.h"
#include "napi_utils.h"
#include "utils.h"
#include "zone_rules_addon.h"

namespace OHOS {
namespace Global {
namespace I18n {
static thread_local napi_ref* g_timezoneConstructor = nullptr;
const int SIZE_TWO = 2;
const int PARAM_STATUS_DST_ONLY = 2;
const int PARAM_STATUS_LOCALE_DST = 3;

I18nTimeZoneAddon::I18nTimeZoneAddon() {}

I18nTimeZoneAddon::~I18nTimeZoneAddon() {}

void I18nTimeZoneAddon::Destructor(napi_env env, void* nativeObject, void* hint)
{
    if (!nativeObject) {
        return;
    }
    delete reinterpret_cast<I18nTimeZoneAddon *>(nativeObject);
    nativeObject = nullptr;
}

napi_value I18nTimeZoneAddon::GetI18nTimeZone(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok) {
        return nullptr;
    }
    if (!NAPIUtils::CheckNapiIsNull(env, argv[0])) {
        status = napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &argv[0]);
        if (status != napi_ok) {
            LOGE("I18nTimeZoneAddon::GetI18nTimeZone: create string failed.");
            return nullptr;
        }
    }
    return StaticGetTimeZone(env, argv, true);
}

napi_value I18nTimeZoneAddon::I18nTimeZoneConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = SIZE_TWO;
    napi_value argv[SIZE_TWO] = { nullptr };
    napi_value thisVar = nullptr;
    void* data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }

    std::string zoneID;
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    if (argc > 0) {
        status = napi_typeof(env, argv[0], &valueType);
        if (status != napi_ok || valueType != napi_valuetype::napi_string) {
            return nullptr;
        }
        int32_t code = 0;
        zoneID = NAPIUtils::GetString(env, argv[0], code);
        if (code != 0) {
            return nullptr;
        }
    }
    if (argc < FUNC_ARGS_COUNT) {
        return nullptr;
    }
    status = napi_typeof(env, argv[1], &valueType);
    if (status != napi_ok || valueType != napi_valuetype::napi_boolean) {
        return nullptr;
    }
    bool isZoneID = false;
    status = napi_get_value_bool(env, argv[1], &isZoneID);
    if (status != napi_ok) {
        return nullptr;
    }
    std::unique_ptr<I18nTimeZoneAddon> obj = std::make_unique<I18nTimeZoneAddon>();
    if (obj == nullptr) {
        return nullptr;
    }
    status =
        napi_wrap(env, thisVar, reinterpret_cast<void*>(obj.get()), I18nTimeZoneAddon::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        return nullptr;
    }
    obj->timezone_ = I18nTimeZone::CreateInstance(zoneID, isZoneID);
    if (!obj->timezone_) {
        return nullptr;
    }
    obj.release();
    return thisVar;
}

napi_value I18nTimeZoneAddon::InitI18nTimeZone(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getID", GetID),
        DECLARE_NAPI_FUNCTION("getDisplayName", GetTimeZoneDisplayName),
        DECLARE_NAPI_FUNCTION("getRawOffset", GetRawOffset),
        DECLARE_NAPI_FUNCTION("getOffset", GetOffset),
        DECLARE_NAPI_FUNCTION("getZoneRules", GetZoneRules),
    };
    napi_value constructor = nullptr;
    napi_status status = napi_define_class(env, "TimeZone", NAPI_AUTO_LENGTH, I18nTimeZoneConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        LOGE("InitI18nTimeZone: Failed to define class TimeZone at Init");
        return nullptr;
    }

    g_timezoneConstructor = new (std::nothrow) napi_ref;
    if (!g_timezoneConstructor) {
        LOGE("InitI18nTimeZone: Failed to create TimeZone ref at init");
        return nullptr;
    }
    status = napi_create_reference(env, constructor, 1, g_timezoneConstructor);
    if (status != napi_ok) {
        LOGE("InitI18nTimeZone: Failed to create reference g_timezoneConstructor at init");
        return nullptr;
    }
    return exports;
}

napi_value I18nTimeZoneAddon::GetAvailableTimezoneIDs(napi_env env, napi_callback_info info)
{
    std::unordered_set<std::string> timezoneIDs = I18nTimeZone::GetAvailableIDs();
    napi_value result = nullptr;
    napi_status status = napi_create_array_with_length(env, timezoneIDs.size(), &result);
    if (status != napi_ok) {
        LOGE("GetAvailableTimezoneIDs: Failed to create array");
        return nullptr;
    }
    size_t index = 0;
    for (std::unordered_set<std::string>::iterator it = timezoneIDs.begin(); it != timezoneIDs.end(); ++it) {
        napi_value value = nullptr;
        status = napi_create_string_utf8(env, (*it).c_str(), NAPI_AUTO_LENGTH, &value);
        if (status != napi_ok) {
            LOGE("Failed to create string item");
            return nullptr;
        }
        status = napi_set_element(env, result, index, value);
        if (status != napi_ok) {
            LOGE("Failed to set array item");
            return nullptr;
        }
        ++index;
    }
    return result;
}

napi_value I18nTimeZoneAddon::DefaultConstructor(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value I18nTimeZoneAddon::GetID(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value* argv = nullptr;
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    I18nTimeZoneAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&obj));
    if (status != napi_ok || !obj || !obj->timezone_) {
        LOGE("GetID: Get TimeZone object failed");
        return nullptr;
    }
    std::string result = obj->timezone_->GetID();
    napi_value value = nullptr;
    status = napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &value);
    if (status != napi_ok) {
        LOGE("GetID: Create result failed");
        return nullptr;
    }
    return value;
}

napi_value I18nTimeZoneAddon::GetTimeZoneDisplayName(napi_env env, napi_callback_info info)
{
    size_t argc = SIZE_TWO;
    napi_value argv[SIZE_TWO] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }

    I18nTimeZoneAddon* obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&obj));
    if (status != napi_ok || !obj || !obj->timezone_) {
        LOGE("GetTimeZoneDisplayName: Get TimeZone object failed");
        return nullptr;
    }

    std::string locale;
    bool isDST = false;
    int32_t parameterStatus = GetParameter(env, argv, locale, isDST);

    std::string result;
    if (parameterStatus == -1) {
        LOGE("GetTimeZoneDisplayName: Parameter type does not match");
        return nullptr;
    } else if (parameterStatus == 0) {
        result = obj->timezone_->GetDisplayName();
    } else if (parameterStatus == 1) {
        result = obj->timezone_->GetDisplayName(locale);
    } else if (parameterStatus == PARAM_STATUS_DST_ONLY) {
        result = obj->timezone_->GetDisplayName(isDST);
    } else {
        result = obj->timezone_->GetDisplayName(locale, isDST);
    }

    napi_value value = nullptr;
    status = napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &value);
    if (status != napi_ok) {
        LOGE("GetTimeZoneDisplayName: Create result failed");
        return nullptr;
    }
    return value;
}

napi_value I18nTimeZoneAddon::GetRawOffset(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value* argv = nullptr;
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    I18nTimeZoneAddon* obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&obj));
    if (status != napi_ok || !obj || !obj->timezone_) {
        LOGE("GetRawOffset: Get TimeZone object failed");
        return nullptr;
    }
    int32_t result = obj->timezone_->GetRawOffset();
    napi_value value = nullptr;
    status = napi_create_int32(env, result, &value);
    if (status != napi_ok) {
        LOGE("GetRawOffset: Create result failed");
        return nullptr;
    }
    return value;
}

napi_value I18nTimeZoneAddon::GetOffset(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }

    double date = 0;
    if (NAPIUtils::CheckNapiIsNull(env, argv[0])) {
        napi_valuetype valueType = napi_valuetype::napi_undefined;
        status = napi_typeof(env, argv[0], &valueType);
        if (status != napi_ok) {
            return nullptr;
        }
        if (valueType != napi_valuetype::napi_number) {
            LOGE("GetOffset: Invalid parameter type");
            return nullptr;
        }
        status = napi_get_value_double(env, argv[0], &date);
        if (status != napi_ok) {
            LOGE("Get parameter date failed");
            return nullptr;
        }
    } else {
        auto time = std::chrono::system_clock::now();
        auto since_epoch = time.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch);
        date = (double)millis.count();
    }

    I18nTimeZoneAddon* obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&obj));
    if (status != napi_ok || !obj || !obj->timezone_) {
        LOGE("GetOffset: Get TimeZone object failed");
        return nullptr;
    }
    int32_t result = obj->timezone_->GetOffset(date);
    napi_value value = nullptr;
    status = napi_create_int32(env, result, &value);
    if (status != napi_ok) {
        LOGE("GetOffset: Create result failed");
        return nullptr;
    }
    return value;
}

napi_value I18nTimeZoneAddon::GetZoneRules(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        LOGE("GetZoneRules: Get param info failed");
        return nullptr;
    }
    I18nTimeZoneAddon* obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&obj));
    if (status != napi_ok || !obj || !obj->timezone_) {
        LOGE("GetZoneRules: Get TimeZone object failed");
        return nullptr;
    }
    std::string tzid = obj->timezone_->GetID();
    return ZoneRulesAddon::GetZoneRulesObject(env, tzid);
}

napi_value I18nTimeZoneAddon::StaticGetTimeZone(napi_env env, napi_value *argv, bool isZoneID)
{
    napi_value constructor = nullptr;
    if (g_timezoneConstructor == nullptr) {
        LOGE("Failed to create g_timezoneConstructor");
        return nullptr;
    }
    napi_status status = napi_get_reference_value(env, *g_timezoneConstructor, &constructor);
    if (status != napi_ok) {
        LOGE("Failed to create reference at StaticGetTimeZone");
        return nullptr;
    }
    napi_value newArgv[SIZE_TWO] = { 0 };
    newArgv[0] = argv[0];
    status = napi_get_boolean(env, isZoneID, &newArgv[1]);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_value result = nullptr;
    status = napi_new_instance(env, constructor, SIZE_TWO, newArgv, &result);
    if (status != napi_ok) {
        LOGE("StaticGetTimeZone create instance failed");
        return nullptr;
    }
    return result;
}

int32_t I18nTimeZoneAddon::GetParameter(napi_env env, napi_value* argv, std::string& localeStr, bool& isDST)
{
    if (NAPIUtils::CheckNapiIsNull(env, argv[1])) {
        napi_valuetype valueType0 = napi_valuetype::napi_undefined;
        napi_valuetype valueType1 = napi_valuetype::napi_undefined;
        napi_status status = napi_typeof(env, argv[0], &valueType0);  // 0 represents first parameter
        if (status != napi_ok) {
            return -1;
        }
        status = napi_typeof(env, argv[1], &valueType1);
        if (status != napi_ok) {
            return -1;
        }
        bool firstParamFlag = NAPIUtils::CheckNapiIsNull(env, argv[0]);
        if (valueType1 == napi_valuetype::napi_boolean) {
            status = napi_get_value_bool(env, argv[1], &isDST);
            if (status != napi_ok) {
                return -1;
            } else if (!firstParamFlag) {
                return PARAM_STATUS_DST_ONLY;
            }
            if (valueType0 == napi_valuetype::napi_string &&
                 GetStringFromJS(env, argv[0], localeStr)) {
                return PARAM_STATUS_LOCALE_DST;
            }
        }
        return -1;
    }
    return GetFirstParameter(env, argv[0], localeStr, isDST);
}

bool I18nTimeZoneAddon::GetStringFromJS(napi_env env, napi_value argv, std::string& jsString)
{
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv, nullptr, 0, &len);
    if (status != napi_ok) {
        LOGE("Failed to get string length");
        return false;
    }
    std::vector<char> argvBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv, argvBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGE("Failed to get string item");
        return false;
    }
    jsString = argvBuf.data();
    return true;
}

int32_t I18nTimeZoneAddon::GetFirstParameter(napi_env env, napi_value value, std::string& localeStr, bool& isDST)
{
    if (!NAPIUtils::CheckNapiIsNull(env, value)) {
        return 0;
    } else {
        napi_valuetype valueType = napi_valuetype::napi_undefined;
        napi_status status = napi_typeof(env, value, &valueType);
        if (status != napi_ok) {
            return -1;
        }
        if (valueType == napi_valuetype::napi_string) {
            bool valid = GetStringFromJS(env, value, localeStr);
            return !valid ? -1 : 1;
        } else if (valueType == napi_valuetype::napi_boolean) {
            status = napi_get_value_bool(env, value, &isDST);
            return (status != napi_ok) ? -1 : PARAM_STATUS_DST_ONLY;
        }
        return -1;
    }
}
} // namespace I18n
} // namespace Global
} // namespace OHOS