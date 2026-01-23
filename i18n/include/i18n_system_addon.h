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
#ifndef OHOS_GLOBAL_I18N_SYSTEM_ADDON_H
#define OHOS_GLOBAL_I18N_SYSTEM_ADDON_H

#include <memory>

#include "i18n_timezone.h"
#include "i18n_types.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace Global {
namespace I18n {
class I18nSystemAddon {
public:
    I18nSystemAddon();
    ~I18nSystemAddon();

    static napi_value GetSystemCountriesWithError(napi_env env, napi_callback_info info);
    static napi_value GetPreferredLanguageList(napi_env env, napi_callback_info info);
    static napi_value GetFirstPreferredLanguage(napi_env env, napi_callback_info info);
    static napi_value GetSystemLanguages(napi_env env, napi_callback_info info);
    static napi_value IsSuggestedWithError(napi_env env, napi_callback_info info);
    static napi_value GetUsingLocalDigitAddon(napi_env env, napi_callback_info info);
    static napi_value GetSimplifiedLanguage(napi_env env, napi_callback_info info);
    static napi_value GetTemperatureType(napi_env env, napi_callback_info info);
    static napi_value GetTemperatureName(napi_env env, napi_callback_info info);
    static napi_value GetFirstDayOfWeek(napi_env env, napi_callback_info info);

private:
    static napi_value GetSystemCountriesImpl(napi_env env, napi_callback_info info, bool throwError);
    static napi_value IsSuggestedImpl(napi_env env, napi_callback_info info, bool throwError);
    static bool ParseTemperatureType(napi_env env, napi_callback_info info, TemperatureType& type);
    static const std::string NAME_OF_TEMPERATURE;
    static const std::string TYPE_OF_TEMPERATURE;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif