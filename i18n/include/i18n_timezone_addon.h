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
#ifndef I18N_TIMEZONE_ADDON_H
#define I18N_TIMEZONE_ADDON_H

#include "i18n_timezone.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace Global {
namespace I18n {
class I18nTimeZoneAddon {
public:
    I18nTimeZoneAddon();
    ~I18nTimeZoneAddon();
    static void Destructor(napi_env env, void* nativeObject, void* hint);
    static napi_value DefaultConstructor(napi_env env, napi_callback_info info);
    static napi_value GetI18nTimeZone(napi_env env, napi_callback_info info);
    static napi_value GetZoneRules(napi_env env, napi_callback_info info);
    static napi_value GetAvailableTimezoneIDs(napi_env env, napi_callback_info info);
    static napi_value InitI18nTimeZone(napi_env env, napi_value exports);

private:
    static napi_value I18nTimeZoneConstructor(napi_env env, napi_callback_info info);

    static napi_value GetID(napi_env env, napi_callback_info info);
    static napi_value GetTimeZoneDisplayName(napi_env env, napi_callback_info info);
    static napi_value GetRawOffset(napi_env env, napi_callback_info info);
    static napi_value GetOffset(napi_env env, napi_callback_info info);

    static napi_value StaticGetTimeZone(napi_env, napi_value* argv, bool isZoneID);
    static int32_t GetParameter(napi_env env, napi_value* argv, std::string& localeStr, bool& isDST);
    static bool GetStringFromJS(napi_env env, napi_value argv, std::string& jsString);
    static int32_t GetFirstParameter(napi_env env, napi_value value, std::string& localeStr, bool& isDST);
    std::unique_ptr<I18nTimeZone> timezone_ = nullptr;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif