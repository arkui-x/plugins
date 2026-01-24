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
#ifndef OHOS_GLOBAL_I18N_HOLIDAY_MANAGER_H
#define OHOS_GLOBAL_I18N_HOLIDAY_MANAGER_H

#include <vector>

#include "holiday_manager.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace Global {
namespace I18n {
class HolidayManagerAddon {
public:
    HolidayManagerAddon();
    ~HolidayManagerAddon();
    static void Destructor(napi_env env, void* nativeObject, void* hint);
    static napi_value InitHolidayManager(napi_env env, napi_value exports);

private:
    static napi_value HolidayManagerConstructor(napi_env env, napi_callback_info info);
    bool InitHolidayManagerContext(napi_env env, napi_callback_info info, const char* path);

    // process js function call
    static napi_value IsHoliday(napi_env env, napi_callback_info info);
    static napi_value GetHolidayInfoItemArray(napi_env env, napi_callback_info info);
    static int32_t ValidateParamNumber(napi_env& env, napi_value& argv);
    static std::vector<int> ValidateParamDate(napi_env& env, napi_value& argv);
    static napi_value CreateHolidayItem(napi_env env, const HolidayInfoItem& holidayItem);
    static napi_value HolidayLocalNameItem(napi_env env, const std::vector<HolidayLocalName> localNames);
    static napi_value GetHolidayInfoItemResult(napi_env env, std::vector<HolidayInfoItem> itemList);
    static int GetDateValue(napi_env env, napi_value value, const std::string method);

    static const int32_t MONTH_INCREASE_ONE = 1;

    napi_env env_;
    std::unique_ptr<HolidayManager> holidayManager_ = nullptr;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif