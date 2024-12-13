/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "key_event.h"
#include "napi/native_node_api.h"
#include "plugins/interfaces/native/inner_api/plugin_utils_napi.h"

namespace OHOS::Plugin {

napi_value Export(napi_env env, napi_value exports)
{
    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_NAPI_STATIC_PROPERTY("KEYCODE_SPACE", PluginUtilsNApi::GetNapiInt32(MMI::KeyEvent::KEYCODE_SPACE, env)),
        DECLARE_NAPI_STATIC_PROPERTY("KEYCODE_ENTER", PluginUtilsNApi::GetNapiInt32(MMI::KeyEvent::KEYCODE_ENTER, env)),
        DECLARE_NAPI_STATIC_PROPERTY(
            "KEYCODE_NUMPAD_ENTER", PluginUtilsNApi::GetNapiInt32(MMI::KeyEvent::KEYCODE_NUMPAD_ENTER, env)),
    };

    PluginUtilsNApi::DefineClass(env, exports, properties, "KeyCode");
    return exports;
}

static napi_module MmiKeyCodeModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Export,
    .nm_modname = "multimodalInput.keyCode",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void MmiKeyCodeRegisterModule(void)
{
    napi_module_register(&MmiKeyCodeModule);
}
} // PLUGINS_JS_KEY_CODE