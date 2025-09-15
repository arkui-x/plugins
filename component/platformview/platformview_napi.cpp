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

#include "platformview_napi.h"

#include "platformview_model_ng.h"

#include "base/log/log.h"
#include "plugins/interfaces/native/inner_api/plugin_utils_napi.h"

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
napi_value JsCreate(napi_env env, napi_callback_info info)
{
    size_t argc = PluginUtilsNApi::MAX_ARG_NUM;
    napi_value argv[PluginUtilsNApi::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= PluginUtilsNApi::ARG_NUM_1, "Wrong number of arguments");
    std::string id = PluginUtilsNApi::GetStringFromValueUtf8(env, argv[PluginUtilsNApi::ARG_NUM_0]);
    std::optional<std::string> data = std::nullopt;
    auto napiData = argv[PluginUtilsNApi::ARG_NUM_1];
    napi_valuetype paramType;
    napi_typeof(env, napiData, &paramType);
    if (argc == PluginUtilsNApi::ARG_NUM_2 && paramType == napi_string) {
        data = PluginUtilsNApi::GetStringFromValueUtf8(env, napiData);
    }
    NG::PlatformViewModelNG::GetInstance()->Create(id, data);
    return PluginUtilsNApi::CreateNull(env);
}

napi_value ExportPlatformView(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("create", JsCreate),
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