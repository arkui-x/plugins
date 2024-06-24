/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "common.h"
#include "ans_inner_errors.h"
#include "log.h"
#include "js_native_api.h"
#include "js_native_api_types.h"
#include "napi_common_util.h"

namespace OHOS {
namespace NotificationNapi {
std::mutex Common::mutex_;

Common::Common()
{}

Common::~Common()
{}

napi_value Common::GetNotificationContentType(const napi_env &env, const napi_value &result, int32_t &type)
{
    LOGD("enter");

    napi_value contentResult = nullptr;
    napi_valuetype valuetype = napi_undefined;
    bool hasNotificationContentType = false;
    bool hasContentType = false;

    NAPI_CALL(env, napi_has_named_property(env, result, "notificationContentType", &hasNotificationContentType));
    if (hasNotificationContentType) {
        napi_get_named_property(env, result, "notificationContentType", &contentResult);
        NAPI_CALL(env, napi_typeof(env, contentResult, &valuetype));
        if (valuetype != napi_number) {
            LOGE("Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int32(env, contentResult, &type);

        return NapiGetNull(env);
    } else {
        LOGE("Property notificationContentType expected.");
    }

    NAPI_CALL(env, napi_has_named_property(env, result, "contentType", &hasContentType));
    if (hasContentType) {
        napi_get_named_property(env, result, "contentType", &contentResult);
        NAPI_CALL(env, napi_typeof(env, contentResult, &valuetype));
        if (valuetype != napi_number) {
            LOGE("Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int32(env, contentResult, &type);

        return NapiGetNull(env);
    } else {
        LOGE("Property contentType expected.");
        return nullptr;
    }
}
}  // namespace NotificationNapi
}  // namespace OHOS
