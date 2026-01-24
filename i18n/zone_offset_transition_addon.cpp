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
#include "zone_offset_transition_addon.h"

#include "error_util.h"
#include "log.h"
#include "napi_utils.h"
#include "zone_offset_transition.h"

namespace OHOS {
namespace Global {
namespace I18n {
static thread_local napi_ref g_zoneOffsetTransConstructor = nullptr;
const int SIZE_THREE = 3;
constexpr size_t PARAM_INDEX_FIRST = 0;
constexpr size_t PARAM_INDEX_SECOND = 1;
constexpr size_t PARAM_INDEX_THIRD = 2;

ZoneOffsetTransitionAddon::ZoneOffsetTransitionAddon() {}

ZoneOffsetTransitionAddon::~ZoneOffsetTransitionAddon() {}

void ZoneOffsetTransitionAddon::Destructor(napi_env env, void* nativeObject, void* hint)
{
    if (!nativeObject) {
        return;
    }
    delete reinterpret_cast<ZoneOffsetTransitionAddon*>(nativeObject);
    nativeObject = nullptr;
}

napi_value ZoneOffsetTransitionAddon::InitZoneOffsetTransition(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getMilliseconds", GetMilliseconds),
        DECLARE_NAPI_FUNCTION("getOffsetAfter", GetOffsetAfter),
        DECLARE_NAPI_FUNCTION("getOffsetBefore", GetOffsetBefore),
    };
    napi_value constructor = nullptr;
    napi_status status = napi_define_class(env, "ZoneOffsetTransition", NAPI_AUTO_LENGTH,
        I18nZoneOffsetTransitionConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        LOGE("InitZoneOffsetTransition: Failed to define class ZoneOffsetTransition at Init");
        return nullptr;
    }
    status = napi_set_named_property(env, exports, "ZoneOffsetTransition", constructor);
    if (status != napi_ok) {
        LOGE("InitZoneOffsetTransition: Set property ZoneOffsetTransition failed.");
        return nullptr;
    }
    status = napi_create_reference(env, constructor, 1, &g_zoneOffsetTransConstructor);
    if (status != napi_ok) {
        LOGE("InitZoneOffsetTransition: Failed to create reference g_timezoneConstructor at init");
        return nullptr;
    }
    return exports;
}

napi_value ZoneOffsetTransitionAddon::I18nZoneOffsetTransitionConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = SIZE_THREE;
    napi_value argv[SIZE_THREE] = { nullptr };
    napi_value thisVar = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (status != napi_ok) {
        LOGE("ZoneOffsetTransitionAddon: Get parameter info failed");
        return nullptr;
    }
    if (argc < CONSTRUCTOR_ARGS_COUNT) {
        LOGE("ZoneOffsetTransitionAddon: Constructor args count less then required");
        return nullptr;
    }
    int32_t code = 0;
    double milliSecond = NAPIUtils::GetDouble(env, argv[PARAM_INDEX_FIRST], code);
    if (code != 0) {
        return nullptr;
    }
    int32_t offsetBefore = NAPIUtils::GetInt(env, argv[PARAM_INDEX_SECOND], code);
    if (code != 0) {
        return nullptr;
    }
    int32_t offsetAfter = NAPIUtils::GetInt(env, argv[PARAM_INDEX_THIRD], code);
    if (code != 0) {
        return nullptr;
    }
    ZoneOffsetTransitionAddon* obj = new (std::nothrow) ZoneOffsetTransitionAddon();
    if (obj == nullptr) {
        LOGE("ZoneOffsetTransitionAddon: Create ZoneOffsetTransitionAddon object failed");
        return nullptr;
    }
    status = napi_wrap(env, thisVar, reinterpret_cast<void *>(obj),
        ZoneOffsetTransitionAddon::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        delete obj;
        LOGE("ZoneOffsetTransitionAddon: Wrap ZoneOffsetTransition failed");
        return nullptr;
    }
    obj->zoneOffsetTransition_ = std::make_unique<ZoneOffsetTransition>(milliSecond, offsetBefore, offsetAfter);
    if (!obj->zoneOffsetTransition_) {
        delete obj;
        LOGE("ZoneOffsetTransitionAddon: Create ZoneOffsetTransition failed");
        return nullptr;
    }
    return thisVar;
}

napi_value ZoneOffsetTransitionAddon::GetZoneOffsetTransObject(napi_env env, ZoneOffsetTransition* zoneOffsetTrans)
{
    if (g_zoneOffsetTransConstructor == nullptr) {
        LOGE("GetZoneOffsetTransObject: g_zoneOffsetTransConstructor is nullptr");
        return nullptr;
    }
    size_t argc = SIZE_THREE;
    napi_value argv[SIZE_THREE] = { nullptr };
    napi_value milliseconds = nullptr;
    napi_status status = napi_create_double(env, zoneOffsetTrans->GetMilliseconds(), &milliseconds);
    if (status != napi_ok || milliseconds == nullptr) {
        LOGE("ErrorUtil::NapiThrow: create napi double failed");
        return nullptr;
    }
    argv[0] = milliseconds;
    napi_value offsetBefore = nullptr;
    status = napi_create_int32(env, zoneOffsetTrans->GetOffsetBefore(), &offsetBefore);
    if (status != napi_ok || offsetBefore == nullptr) {
        LOGE("ErrorUtil::NapiThrow: create int32 failed");
        return nullptr;
    }
    argv[1] = offsetBefore;
    napi_value offsetAfter = nullptr;
    status = napi_create_int32(env, zoneOffsetTrans->GetOffsetAfter(), &offsetAfter);
    if (status != napi_ok || offsetAfter == nullptr) {
        LOGE("ErrorUtil::NapiThrow: create int32 failed");
        return nullptr;
    }
    argv[CONSTRUCTOR_ARGS_COUNT - 1] = offsetAfter;
    napi_value constructor = nullptr;
    status = napi_get_reference_value(env, g_zoneOffsetTransConstructor, &constructor);
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

napi_value ZoneOffsetTransitionAddon::GetMilliseconds(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value* argv = nullptr;
    napi_value thisVar = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (status != napi_ok) {
        LOGE("GetMilliseconds: Get parameter info failed");
        return nullptr;
    }
    ZoneOffsetTransitionAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->zoneOffsetTransition_) {
        LOGE("GetMilliseconds: Get ZoneOffsetTransition object failed");
        return nullptr;
    }
    double time = obj->zoneOffsetTransition_->GetMilliseconds();
    napi_value result = nullptr;
    status = napi_create_double(env, time, &result);
    if (status != napi_ok || result == nullptr) {
        LOGE("GetMilliseconds: create double js variable failed.");
        return nullptr;
    }
    return result;
}

napi_value ZoneOffsetTransitionAddon::GetOffsetAfter(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value* argv = nullptr;
    napi_value thisVar = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (status != napi_ok) {
        LOGE("GetOffsetAfter: Get parameter info failed");
        return nullptr;
    }
    ZoneOffsetTransitionAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->zoneOffsetTransition_) {
        LOGE("GetOffsetAfter: Get ZoneOffsetTransition object failed");
        return nullptr;
    }
    int32_t offsetAfter = obj->zoneOffsetTransition_->GetOffsetAfter();
    return NAPIUtils::CreateNumber(env, offsetAfter);
}

napi_value ZoneOffsetTransitionAddon::GetOffsetBefore(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value* argv = nullptr;
    napi_value thisVar = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (status != napi_ok) {
        LOGE("GetOffsetBefore: Get parameter info failed");
        return nullptr;
    }
    ZoneOffsetTransitionAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&obj));
    if (status != napi_ok || !obj || !obj->zoneOffsetTransition_) {
        LOGE("GetOffsetBefore: Get ZoneOffsetTransition object failed");
        return nullptr;
    }
    int32_t offsetBefore = obj->zoneOffsetTransition_->GetOffsetBefore();
    return NAPIUtils::CreateNumber(env, offsetBefore);
}
} // namespace I18n
} // namespace Global
} // namespace OHOS