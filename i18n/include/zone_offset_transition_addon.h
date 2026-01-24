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

#ifndef OHOS_GLOBAL_ZONE_OFFSET_TRANSITION_ADDON_H
#define OHOS_GLOBAL_ZONE_OFFSET_TRANSITION_ADDON_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "zone_offset_transition.h"

namespace OHOS {
namespace Global {
namespace I18n {
class ZoneOffsetTransitionAddon {
public:
    ZoneOffsetTransitionAddon();
    ~ZoneOffsetTransitionAddon();
    static void Destructor(napi_env env, void* nativeObject, void* hint);
    static napi_value InitZoneOffsetTransition(napi_env env, napi_value exports);
    static napi_value GetZoneOffsetTransObject(napi_env env, ZoneOffsetTransition* zoneOffsetTrans);
    static napi_value GetMilliseconds(napi_env env, napi_callback_info info);
    static napi_value GetOffsetAfter(napi_env env, napi_callback_info info);
    static napi_value GetOffsetBefore(napi_env env, napi_callback_info info);

private:
    static napi_value I18nZoneOffsetTransitionConstructor(napi_env env, napi_callback_info info);

    static const size_t CONSTRUCTOR_ARGS_COUNT = 3;
    std::unique_ptr<ZoneOffsetTransition> zoneOffsetTransition_ = nullptr;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif