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
#include "zone_rules_addon.h"

#include "character.h"
#include "error_util.h"
#include "log.h"
#include "napi_utils.h"
#include "zone_offset_transition_addon.h"

namespace OHOS {
namespace Global {
namespace I18n {
static thread_local napi_ref g_zoneRulesConstructor = nullptr;

ZoneRulesAddon::ZoneRulesAddon() {}

ZoneRulesAddon::~ZoneRulesAddon() {}

void ZoneRulesAddon::Destructor(napi_env env, void* nativeObject, void* hint)
{
    if (!nativeObject) {
        return;
    }
    delete reinterpret_cast<ZoneRulesAddon*>(nativeObject);
    nativeObject = nullptr;
}

napi_value ZoneRulesAddon::InitI18nZoneRules(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("nextTransition", NextTransition),
    };

    napi_value constructor = nullptr;
    napi_status status = napi_define_class(env, "ZoneRules", NAPI_AUTO_LENGTH, ZoneRulesConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        LOGE("InitI18nZoneRules: Define class failed when InitI18nZoneRules.");
        return nullptr;
    }
    status = napi_set_named_property(env, exports, "ZoneRules", constructor);
    if (status != napi_ok) {
        LOGE("InitI18nZoneRules: Set property ZoneRules failed.");
        return nullptr;
    }
    status = napi_create_reference(env, constructor, 1, &g_zoneRulesConstructor);
    if (status != napi_ok) {
        LOGE("InitI18nZoneRules: Failed to create reference at init.");
        return nullptr;
    }
    return exports;
}

napi_value ZoneRulesAddon::ZoneRulesConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (status != napi_ok) {
        LOGE("ZoneRulesConstructor: Get parameter info failed");
        return nullptr;
    }
    std::string tzId;
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    if (argc < 1) {
        LOGE("ZoneRulesConstructor: Param count less then required");
        return nullptr;
    }
    status = napi_typeof(env, argv[0], &valueType);
    if (status != napi_ok) {
        LOGE("ZoneRulesConstructor: Get param type failed.");
        return nullptr;
    }
    if (valueType != napi_valuetype::napi_string) {
        LOGE("ZoneRulesConstructor: Param type error.");
        return nullptr;
    }
    int32_t code = 0;
    tzId = NAPIUtils::GetString(env, argv[0], code);
    if (code != 0) {
        return nullptr;
    }
    ZoneRulesAddon* obj = new (std::nothrow) ZoneRulesAddon();
    if (obj == nullptr) {
        LOGE("ZoneRulesConstructor: Create ZoneRulesAddon failed.");
        return nullptr;
    }
    status =
        napi_wrap(env, thisVar, reinterpret_cast<void *>(obj), ZoneRulesAddon::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        delete obj;
        LOGE("ZoneRulesConstructor: warp ZoneRulesAddon failed.");
        return nullptr;
    }
    obj->zoneRules_ = std::make_unique<ZoneRules>(tzId);
    if (!obj->zoneRules_) {
        delete obj;
        LOGE("ZoneRulesConstructor: Param type error.");
        return nullptr;
    }
    return thisVar;
}

napi_value ZoneRulesAddon::NextTransition(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (status != napi_ok) {
        LOGE("ZoneRulesAddon::NextTransition: Get parameter info failed");
        return nullptr;
    }
    ZoneRulesAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->zoneRules_) {
        LOGE("ZoneRulesAddon::NextTransition, Get ZoneRulesAddon object failed");
        return nullptr;
    }
    std::unique_ptr<ZoneOffsetTransition> zoneOffsetTrans;
    if (argc < 1) {
        zoneOffsetTrans = obj->zoneRules_->NextTransition();
    } else {
        int32_t errorCode = 0;
        double paramDate = GetFirstParameter(env, argv[0], errorCode);
        if (errorCode != 0) {
            return nullptr;
        }
        zoneOffsetTrans = obj->zoneRules_->NextTransition(paramDate);
    }
    return ZoneOffsetTransitionAddon::GetZoneOffsetTransObject(env, zoneOffsetTrans.get());
}

double ZoneRulesAddon::GetFirstParameter(napi_env env, napi_value argValue, int32_t& errorCode)
{
    if (!NAPIUtils::CheckNapiIsNull(env, argValue)) {
        LOGE("ZoneRulesAddon::GetFirstParameter, parameter is null.");
        errorCode = -1;
        return 0;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_status status = napi_typeof(env, argValue, &valueType);
    if (status != napi_ok) {
        LOGE("ZoneRulesAddon::GetFirstParameter, get parameter type failed.");
        errorCode = -1;
        return 0;
    }
    if (valueType != napi_valuetype::napi_number) {
        LOGE("ZoneRulesAddon::GetFirstParameter, parameter type error.");
        errorCode = -1;
        return 0;
    }
    double date = 0;
    status = napi_get_value_double(env, argValue, &date);
    if (status != napi_ok) {
        LOGE("ZoneRulesAddon::GetFirstParameter, Get parameter date failed");
        errorCode = -1;
        return 0;
    }
    return date;
}

napi_value ZoneRulesAddon::GetZoneRulesObject(napi_env env, std::string tzId)
{
    if (g_zoneRulesConstructor == nullptr) {
        LOGE("ZoneRulesAddon::GetZoneRulesObject: g_zoneRulesConstructor is nullptr");
        return nullptr;
    }
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value param = nullptr;
    napi_status status = napi_create_string_latin1(env, tzId.c_str(), NAPI_AUTO_LENGTH, &param);
    if (status != napi_ok) {
        LOGE("ErrorUtil::NapiThrow: create string failed");
        return nullptr;
    }
    argv[0] = param;
    napi_value constructor = nullptr;
    status = napi_get_reference_value(env, g_zoneRulesConstructor, &constructor);
    if (status != napi_ok || constructor == nullptr) {
        LOGE("Failed to create reference of normalizer Constructor");
        return nullptr;
    }
    napi_value result = nullptr;
    status = napi_new_instance(env, constructor, argc, argv, &result);
    if (status != napi_ok || result == nullptr) {
        LOGE("create normalizer instance failed");
        return nullptr;
    }
    return result;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS