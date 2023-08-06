/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "inner_api/plugin_utils_inner.h"
#include "plugins/display/android/java/jni/display_receiver.h"

namespace OHOS::Plugin {
static napi_value WrapDisplayInfo(napi_env env, const DisplayInfo& displayInfo)
{
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(displayInfo.id), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "id", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, displayInfo.name.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "name", proValue));

    NAPI_CALL(env, napi_get_boolean(env, displayInfo.alive, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "alive", proValue));

    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(displayInfo.state), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "state", proValue));

    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(displayInfo.refreshRate), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "refreshRate", proValue));

    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(displayInfo.rotation), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "rotation", proValue));

    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(displayInfo.width), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "width", proValue));

    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(displayInfo.height), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "height", proValue));

    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(displayInfo.densityDPI), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "densityDPI", proValue));

    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(displayInfo.densityPixels), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "densityPixels", proValue));

    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(displayInfo.scaledDensity), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "scaledDensity", proValue));

    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(displayInfo.xDPI), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "xDPI", proValue));

    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(displayInfo.yDPI), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "yDPI", proValue));

    return result;
}
void DisplayReceiver::ReceiveFun(DisplayInfo displayInfo, AsyncCallbackInfo* ptr)
{
    PluginUtilsInner::RunTaskOnJS([displayInfo, ptr]() {
        ptr->jsdata = WrapDisplayInfo(ptr->env, displayInfo);
        if (ptr->callback[0] == nullptr) {
            PluginUtilsNApi::EmitPromiseWork(ptr);
        } else {
            PluginUtilsNApi::EmitAsyncCallbackWork(ptr);
        }
    });
}
} // namespace OHOS::Plugin
