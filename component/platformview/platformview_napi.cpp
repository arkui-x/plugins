/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <optional>

#include <vector>

#include "base/log/log.h"
#include "platformview_model_ng.h"
#include "platformview_napi.h"
#include "plugins/interfaces/native/inner_api/plugin_utils_napi.h"

#include "core/components_ng/base/view_stack_processor.h"

extern const char _binary_platformview_js_start[];
extern const char _binary_platformview_abc_start[];
#if !defined(IOS_PLATFORM)
extern const char _binary_platformview_js_end[];
extern const char _binary_platformview_abc_end[];
#else
extern const char* _binary_platformview_js_end;
extern const char* _binary_platformview_abc_end;
#endif

using namespace OHOS::Ace;
std::unique_ptr<NG::PlatformViewModelNG> NG::PlatformViewModelNG::instance_ = nullptr;
std::mutex NG::PlatformViewModelNG::mutex_;

namespace {
std::string GetStringFromNapiValue(napi_env env, napi_value value, const std::string& defaultValue)
{
    if (value == nullptr) {
        return defaultValue;
    }

    napi_valuetype actualType;
    if (napi_typeof(env, value, &actualType) != napi_ok) {
        return defaultValue;
    }
    if (actualType == napi_undefined || actualType == napi_null) {
        return defaultValue;
    }

    napi_valuetype valueType;
    if (napi_typeof(env, value, &valueType) != napi_ok) {
        return defaultValue;
    }
    if (valueType != napi_number && valueType != napi_string) {
        return defaultValue;
    }

    napi_value coerced;
    if (napi_coerce_to_string(env, value, &coerced) != napi_ok) {
        return defaultValue;
    }
    return OHOS::Plugin::PluginUtilsNApi::GetStringFromValueUtf8(env, coerced);
}

bool TryGetStringProperty(napi_env env, napi_value obj, const char* name, std::string& out)
{
    bool hasProp = false;
    if (napi_has_named_property(env, obj, name, &hasProp) != napi_ok || !hasProp) {
        return false;
    }
    napi_value value = nullptr;
    if (napi_get_named_property(env, obj, name, &value) != napi_ok) {
        return false;
    }
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, value, &valueType) != napi_ok || valueType != napi_string) {
        return false;
    }
    out = OHOS::Plugin::PluginUtilsNApi::GetStringFromValueUtf8(env, value);
    return true;
}

bool TryGetInt32Property(napi_env env, napi_value obj, const char* name, int32_t& out)
{
    bool hasProp = false;
    if (napi_has_named_property(env, obj, name, &hasProp) != napi_ok || !hasProp) {
        return false;
    }
    napi_value value = nullptr;
    if (napi_get_named_property(env, obj, name, &value) != napi_ok) {
        return false;
    }
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, value, &valueType) != napi_ok || valueType != napi_number) {
        return false;
    }
    return napi_get_value_int32(env, value, &out) == napi_ok;
}
} // namespace

NG::PlatformViewModelNG* NG::PlatformViewModelNG::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
            instance_.reset(new NG::PlatformViewModelNG());
        }
    }
    return instance_.get();
}
namespace OHOS::Plugin::PlatformView {
namespace {
constexpr int32_t DEFAULT_PLATFORM_VIEW_TYPE = 0;
}

napi_value JsCreate(napi_env env, napi_callback_info info)
{
    size_t argc = PluginUtilsNApi::MAX_ARG_NUM;
    napi_value argv[PluginUtilsNApi::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= PluginUtilsNApi::ARG_NUM_1, "Wrong number of arguments");

    std::string id;
    std::optional<std::string> data = std::nullopt;
    int32_t type = DEFAULT_PLATFORM_VIEW_TYPE;

    napi_value firstArg = argv[PluginUtilsNApi::ARG_NUM_0];

    napi_valuetype firstType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, firstArg, &firstType));

    bool hasIdProperty = false;
    if (firstType == napi_object) {
        NAPI_CALL(env, napi_has_named_property(env, firstArg, "id", &hasIdProperty));
    }
    if (firstType == napi_object && hasIdProperty) {
        NAPI_ASSERT(env, TryGetStringProperty(env, firstArg, "id", id), "Property 'id' must be a string");
        std::string dataValue;
        if (TryGetStringProperty(env, firstArg, "data", dataValue)) {
            data = dataValue;
        }
        int32_t t = DEFAULT_PLATFORM_VIEW_TYPE;
        if (TryGetInt32Property(env, firstArg, "type", t)) {
            type = t;
        }
    } else {
        NAPI_ASSERT(env, firstType == napi_string, "First argument must be a string or object with string 'id'");
        id = PluginUtilsNApi::GetStringFromValueUtf8(env, firstArg);
        if (argc >= PluginUtilsNApi::ARG_NUM_2) {
            auto napiData = argv[PluginUtilsNApi::ARG_NUM_1];
            napi_valuetype paramType;
            napi_typeof(env, napiData, &paramType);
            if (paramType == napi_string) {
                data = PluginUtilsNApi::GetStringFromValueUtf8(env, napiData);
            }
        }
    }
    NG::PlatformViewModelNG::GetInstance()->Create(id, type, data);
    return PluginUtilsNApi::CreateNull(env);
}

napi_value JsScale(napi_env env, napi_callback_info info)
{
    size_t argc = PluginUtilsNApi::MAX_ARG_NUM;
    napi_value argv[PluginUtilsNApi::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= PluginUtilsNApi::ARG_NUM_1, "Wrong number of arguments");
    napi_value x = PluginUtilsNApi::GetNamedProperty(argv[PluginUtilsNApi::ARG_NUM_0], "x", env);
    napi_value y = PluginUtilsNApi::GetNamedProperty(argv[PluginUtilsNApi::ARG_NUM_0], "y", env);
    napi_value z = PluginUtilsNApi::GetNamedProperty(argv[PluginUtilsNApi::ARG_NUM_0], "z", env);
    napi_value centerX = PluginUtilsNApi::GetNamedProperty(argv[PluginUtilsNApi::ARG_NUM_0], "centerX", env);
    napi_value centerY = PluginUtilsNApi::GetNamedProperty(argv[PluginUtilsNApi::ARG_NUM_0], "centerY", env);
    auto isUndefined = [env](napi_value value) {
        return PluginUtilsNApi::IsMatchType(value, napi_undefined, env) == TYPE_CHECK_SUCCESS;
    };
    float fx = isUndefined(x) ? 1.0f : static_cast<float>(PluginUtilsNApi::GetDouble(env, x));
    float fy = isUndefined(y) ? 1.0f : static_cast<float>(PluginUtilsNApi::GetDouble(env, y));
    float fz = isUndefined(z) ? 1.0f : static_cast<float>(PluginUtilsNApi::GetDouble(env, z));

    std::string centerXStr = GetStringFromNapiValue(env, centerX, "0");
    std::string centerYStr = GetStringFromNapiValue(env, centerY, "0");
    NG::PlatformViewModelNG::GetInstance()->SetScale(fx, fy, fz, centerXStr, centerYStr);
    return PluginUtilsNApi::CreateNull(env);
}

napi_value JsRotate(napi_env env, napi_callback_info info)
{
    size_t argc = PluginUtilsNApi::MAX_ARG_NUM;
    napi_value argv[PluginUtilsNApi::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= PluginUtilsNApi::ARG_NUM_1, "Wrong number of arguments");
    napi_value x = PluginUtilsNApi::GetNamedProperty(argv[PluginUtilsNApi::ARG_NUM_0], "x", env);
    napi_value y = PluginUtilsNApi::GetNamedProperty(argv[PluginUtilsNApi::ARG_NUM_0], "y", env);
    napi_value z = PluginUtilsNApi::GetNamedProperty(argv[PluginUtilsNApi::ARG_NUM_0], "z", env);
    napi_value angle = PluginUtilsNApi::GetNamedProperty(argv[PluginUtilsNApi::ARG_NUM_0], "angle", env);
    napi_value centerX = PluginUtilsNApi::GetNamedProperty(argv[PluginUtilsNApi::ARG_NUM_0], "centerX", env);
    napi_value centerY = PluginUtilsNApi::GetNamedProperty(argv[PluginUtilsNApi::ARG_NUM_0], "centerY", env);
    napi_value centerZ = PluginUtilsNApi::GetNamedProperty(argv[PluginUtilsNApi::ARG_NUM_0], "centerZ", env);
    napi_value perspective = PluginUtilsNApi::GetNamedProperty(argv[PluginUtilsNApi::ARG_NUM_0], "perspective", env);
    float fx = static_cast<float>(PluginUtilsNApi::GetDouble(env, x));
    float fy = static_cast<float>(PluginUtilsNApi::GetDouble(env, y));
    float fz = static_cast<float>(PluginUtilsNApi::GetDouble(env, z));

    std::string fangleStr = GetStringFromNapiValue(env, angle, "");
    std::string centerXStr = GetStringFromNapiValue(env, centerX, "50%");
    std::string centerYStr = GetStringFromNapiValue(env, centerY, "50%");
    std::string centerZStr = GetStringFromNapiValue(env, centerZ, "0");
    std::string perspectiveStr = GetStringFromNapiValue(env, perspective, "");
    NG::PlatformViewModelNG::GetInstance()->SetRotation(fx, fy, fz, fangleStr, centerXStr,
        centerYStr, centerZStr, perspectiveStr);
    return PluginUtilsNApi::CreateNull(env);
}

napi_value JsTranslate(napi_env env, napi_callback_info info)
{
    size_t argc = PluginUtilsNApi::MAX_ARG_NUM;
    napi_value argv[PluginUtilsNApi::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= PluginUtilsNApi::ARG_NUM_1, "Wrong number of arguments");
    napi_value x = PluginUtilsNApi::GetNamedProperty(argv[PluginUtilsNApi::ARG_NUM_0], "x", env);
    napi_value y = PluginUtilsNApi::GetNamedProperty(argv[PluginUtilsNApi::ARG_NUM_0], "y", env);
    napi_value z = PluginUtilsNApi::GetNamedProperty(argv[PluginUtilsNApi::ARG_NUM_0], "z", env);
    std::string fx = GetStringFromNapiValue(env, x, "0");
    std::string fy = GetStringFromNapiValue(env, y, "0");
    std::string fz = GetStringFromNapiValue(env, z, "0");
    NG::PlatformViewModelNG::GetInstance()->SetTranslate(fx, fy, fz);
    return PluginUtilsNApi::CreateNull(env);
}

napi_value JsTransform(napi_env env, napi_callback_info info)
{
    size_t argc = PluginUtilsNApi::MAX_ARG_NUM;
    napi_value argv[PluginUtilsNApi::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc == PluginUtilsNApi::ARG_NUM_1, "Wrong number of arguments");

    napi_value matrix4x4 = PluginUtilsNApi::GetNamedProperty(argv[PluginUtilsNApi::ARG_NUM_0], "matrix4x4", env);
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, matrix4x4, &valueType) != napi_ok || valueType != napi_object) {
        return PluginUtilsNApi::CreateNull(env);
    }

    bool isArray = false;
    if (napi_is_array(env, matrix4x4, &isArray) != napi_ok || !isArray) {
        return PluginUtilsNApi::CreateNull(env);
    }

    uint32_t length = 0;
    NAPI_CALL(env, napi_get_array_length(env, matrix4x4, &length));
    constexpr uint32_t MATRIX4_LEN = 16;
    if (length != MATRIX4_LEN) {
        return PluginUtilsNApi::CreateNull(env);
    }

    std::vector<float> matrix;
    matrix.reserve(MATRIX4_LEN);
    for (uint32_t i = 0; i < MATRIX4_LEN; i++) {
        napi_value item = nullptr;
        NAPI_CALL(env, napi_get_element(env, matrix4x4, i, &item));
        napi_valuetype valueType;
        if (napi_typeof(env, item, &valueType) != napi_ok || valueType != napi_number) {
            return PluginUtilsNApi::CreateNull(env);
        }
        matrix.push_back(static_cast<float>(PluginUtilsNApi::GetDouble(env, item)));
    }

    NG::PlatformViewModelNG::GetInstance()->SetTransformMatrix(matrix);
    return PluginUtilsNApi::CreateNull(env);
}

napi_value ExportPlatformView(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("create", JsCreate),
        DECLARE_NAPI_FUNCTION("scale", JsScale),
        DECLARE_NAPI_FUNCTION("rotate", JsRotate),
        DECLARE_NAPI_FUNCTION("translate", JsTranslate),
        DECLARE_NAPI_FUNCTION("transform", JsTransform),
        DECLARE_NAPI_FUNCTION("transform3D", JsTransform),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}
} // namespace OHOS::Plugin::PlatformView

extern "C" __attribute__((visibility("default"))) void NAPI_platformview_GetJSCode(const char** buf, int* bufLen)
{
    if (buf != nullptr) {
        *buf = _binary_platformview_js_start;
    }

    if (bufLen != nullptr) {
        *bufLen = _binary_platformview_js_end - _binary_platformview_js_start;
    }
}

// platformview JS register
extern "C" __attribute__((visibility("default"))) void NAPI_platformview_GetABCCode(const char** buf, int* buflen)
{
    if (buf != nullptr) {
        *buf = _binary_platformview_abc_start;
    }
    if (buflen != nullptr) {
        *buflen = _binary_platformview_abc_end - _binary_platformview_abc_start;
    }
}

static napi_module_with_js platfromViewModule  = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = OHOS::Plugin::PlatformView::ExportPlatformView,
    .nm_modname = "platformview",
    .nm_priv = ((void*)0),
    .nm_get_abc_code = NAPI_platformview_GetABCCode,
    .nm_get_js_code = NAPI_platformview_GetJSCode,
};

extern "C" __attribute__((constructor)) void RegisterModulePlatformView()
{
    napi_module_with_js_register(&platfromViewModule);
}