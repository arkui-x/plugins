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

#ifndef OHOS_GLOBAL_ZONE_RULES_ADDON_H
#define OHOS_GLOBAL_ZONE_RULES_ADDON_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "zone_rules.h"

namespace OHOS {
namespace Global {
namespace I18n {
class ZoneRulesAddon {
public:
    ZoneRulesAddon();
    ~ZoneRulesAddon();
    static void Destructor(napi_env env, void* nativeObject, void* hint);
    static napi_value InitI18nZoneRules(napi_env env, napi_value exports);
    static napi_value GetZoneRulesObject(napi_env env, std::string tzId);
    static napi_value NextTransition(napi_env env, napi_callback_info info);

private:
    static napi_value ZoneRulesConstructor(napi_env env, napi_callback_info info);
    static double GetFirstParameter(napi_env env, napi_value argValue, int32_t& errorCode);

    std::unique_ptr<ZoneRules> zoneRules_ = nullptr;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif