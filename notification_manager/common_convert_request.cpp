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
napi_value Common::GetNotificationRequestByNumber(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LOGD("enter");
    // id?: number
    if (GetNotificationId(env, value, request) == nullptr) {
        return nullptr;
    }
    // deliveryTime?: number
    if (GetNotificationDeliveryTime(env, value, request) == nullptr) {
        return nullptr;
    }
    // autoDeletedTime?: number
    if (GetNotificationAutoDeletedTime(env, value, request) == nullptr) {
        return nullptr;
    }
    // badgeNumber?: number
    if (GetNotificationBadgeNumber(env, value, request) == nullptr) {
        return nullptr;
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationRequestByString(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LOGD("enter");
    // groupName?: string
    if (GetNotificationGroupName(env, value, request) == nullptr) {
        return nullptr;
    }
    return NapiGetNull(env);
}

napi_value Common::GetNotificationRequestByBool(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LOGD("enter");
    // isOngoing?: boolean
    if (GetNotificationIsOngoing(env, value, request) == nullptr) {
        return nullptr;
    }
    // tapDismissed?: boolean
    if (GetNotificationtapDismissed(env, value, request) == nullptr) {
        return nullptr;
    }
    // isAlertOnce?: boolean
    if (GetNotificationIsAlertOnce(env, value, request) == nullptr) {
        return nullptr;
    }
    // isStopwatch?: boolean
    if (GetNotificationIsStopwatch(env, value, request) == nullptr) {
        return nullptr;
    }
    // isCountDown?: boolean
    if (GetNotificationIsCountDown(env, value, request) == nullptr) {
        return nullptr;
    }
    // showDeliveryTime?: boolean
    if (GetNotificationShowDeliveryTime(env, value, request) == nullptr) {
        return nullptr;
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationRequestByCustom(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LOGD("enter");
    // content: NotificationContent
    if (GetNotificationContent(env, value, request) == nullptr) {
        return nullptr;
    }
    return NapiGetNull(env);
}

napi_value Common::GetNotificationRequest(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LOGD("enter");
    if (!GetNotificationRequestByNumber(env, value, request)) {
        return nullptr;
    }
    if (!GetNotificationRequestByString(env, value, request)) {
        return nullptr;
    }
    if (!GetNotificationRequestByBool(env, value, request)) {
        return nullptr;
    }
    if (!GetNotificationRequestByCustom(env, value, request)) {
        return nullptr;
    }
    return NapiGetNull(env);
}

napi_value Common::GetNotificationId(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    int32_t notificationId = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "id", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "id", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_number) {
            LOGE("Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int32(env, result, &notificationId);
        request.SetNotificationId(notificationId);
        LOGD("notificationId = %{public}d", notificationId);
    } else {
        LOGD("default notificationId = 0");
        request.SetNotificationId(0);
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationIsOngoing(const napi_env& env, const napi_value& value, NotificationRequest& request)
{
    LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool isOngoing = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "isOngoing", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "isOngoing", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            LOGE("Wrong argument type. Bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, result, &isOngoing);
        request.SetInProgress(isOngoing);
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationDeliveryTime(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    int64_t deliveryTime = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "deliveryTime", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "deliveryTime", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_number) {
            LOGE("Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int64(env, result, &deliveryTime);
        request.SetDeliveryTime(deliveryTime);
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationtapDismissed(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool tapDismissed = true;

    NAPI_CALL(env, napi_has_named_property(env, value, "tapDismissed", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "tapDismissed", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            LOGE("Wrong argument type. Bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, result, &tapDismissed);
        request.SetTapDismissed(tapDismissed);
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationGroupName(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    size_t strLen = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "groupName", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "groupName", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_string) {
            LOGE("Wrong argument type. String expected.");
            return nullptr;
        }
        char str[STR_MAX_SIZE] = {0};
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        request.SetGroupName(str);
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationAutoDeletedTime(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    int64_t autoDeletedTime = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "autoDeletedTime", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "autoDeletedTime", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_number) {
            LOGE("Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int64(env, result, &autoDeletedTime);
        request.SetAutoDeletedTime(autoDeletedTime);
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationIsAlertOnce(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool isAlertOnce = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "isAlertOnce", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "isAlertOnce", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            LOGE("Wrong argument type. Bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, result, &isAlertOnce);
        request.SetAlertOneTime(isAlertOnce);
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationIsStopwatch(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool isStopwatch = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "isStopwatch", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "isStopwatch", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            LOGE("Wrong argument type. Bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, result, &isStopwatch);
        request.SetShowStopwatch(isStopwatch);
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationIsCountDown(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool isCountDown = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "isCountDown", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "isCountDown", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            LOGE("Wrong argument type. Bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, result, &isCountDown);
        request.SetCountdownTimer(isCountDown);
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationShowDeliveryTime(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool showDeliveryTime = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "showDeliveryTime", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "showDeliveryTime", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            LOGE("Wrong argument type. Bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, result, &showDeliveryTime);
        request.SetShowDeliveryTime(showDeliveryTime);
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationBadgeNumber(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    int32_t badgeNumber = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "badgeNumber", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "badgeNumber", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_number) {
            LOGE("Wrong argument type. Number expected.");
            return nullptr;
        }

        napi_get_value_int32(env, result, &badgeNumber);
        if (badgeNumber < 0) {
            LOGE("Wrong badge number.");
            return nullptr;
        }

        request.SetBadgeNumber(badgeNumber);
    }

    return NapiGetNull(env);
}
}
}
