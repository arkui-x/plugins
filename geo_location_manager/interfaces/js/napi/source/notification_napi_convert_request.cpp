/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "notification_napi.h"
#include "ans_inner_errors.h"
#include "location_log.h"
#include "napi_util.h"
// Minimal inline content parsing (plan B) for BASIC_TEXT without depending on full notification_manager common code.

#include <string>

// Assume BASIC_TEXT content type numeric constant; if integration with full enum later, replace with real value.
// We will attempt to read either 'notificationContentType' or fallback to 'contentType'.


namespace OHOS {
namespace Location {
const int MAX_TITLE_LENGTH = 255;
const int MAX_TEXT_LENGTH = 511;

napi_value NotificationNapi::NapiGetNull(napi_env env)
{
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

napi_value NotificationNapi::GetNotificationId(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "parse id");
    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    int32_t notificationId = 0;
    NAPI_CALL(env, napi_has_named_property(env, value, "id", &hasProperty));
    if (!hasProperty) {
        request.SetNotificationId(0);
        return NotificationNapi::NapiGetNull(env);
    }
    napi_get_named_property(env, value, "id", &result);
    NAPI_CALL(env, napi_typeof(env, result, &valuetype));
    if (valuetype != napi_number) {
        LBSLOGE(NAPI_UTILS, "id type error");
        return nullptr;
    }
    napi_get_value_int32(env, result, &notificationId);
    request.SetNotificationId(notificationId);
    return NotificationNapi::NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationBadgeNumber(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "parse badgeNumber");
    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    int32_t badgeNumber = 0;
    NAPI_CALL(env, napi_has_named_property(env, value, "badgeNumber", &hasProperty));
    if (!hasProperty) {
        return NotificationNapi::NapiGetNull(env);
    }
    napi_get_named_property(env, value, "badgeNumber", &result);
    NAPI_CALL(env, napi_typeof(env, result, &valuetype));
    if (valuetype != napi_number) {
        LBSLOGE(NAPI_UTILS, "badgeNumber type error");
        return nullptr;
    }
    napi_get_value_int32(env, result, &badgeNumber);
    if (badgeNumber < 0) {
        LBSLOGE(NAPI_UTILS, "badgeNumber negative");
        return nullptr;
    }
    request.SetBadgeNumber(badgeNumber);
    return NotificationNapi::NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationDeliveryTime(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "parse deliveryTime");
    napi_value result = nullptr;
    bool hasProperty = false;
    napi_valuetype valuetype = napi_undefined;
    int64_t deliveryTime = 0;
    NAPI_CALL(env, napi_has_named_property(env, value, "deliveryTime", &hasProperty));
    if (!hasProperty) {
        return NotificationNapi::NapiGetNull(env);
    }
    napi_get_named_property(env, value, "deliveryTime", &result);
    NAPI_CALL(env, napi_typeof(env, result, &valuetype));
    if (valuetype != napi_number) {
        LBSLOGE(NAPI_UTILS, "deliveryTime type error");
        return nullptr;
    }
    napi_get_value_int64(env, result, &deliveryTime);
    request.SetDeliveryTime(deliveryTime);
    return NotificationNapi::NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationShowDeliveryTime(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "parse showDeliveryTime");
    napi_value result = nullptr;
    bool hasProperty = false;
    napi_valuetype valuetype = napi_undefined;
    bool showDeliveryTime = false;
    NAPI_CALL(env, napi_has_named_property(env, value, "showDeliveryTime", &hasProperty));
    if (!hasProperty) {
        return NotificationNapi::NapiGetNull(env);
    }
    napi_get_named_property(env, value, "showDeliveryTime", &result);
    NAPI_CALL(env, napi_typeof(env, result, &valuetype));
    if (valuetype != napi_boolean) {
        LBSLOGE(NAPI_UTILS, "showDeliveryTime type error");
        return nullptr;
    }
    napi_get_value_bool(env, result, &showDeliveryTime);
    request.SetShowDeliveryTime(showDeliveryTime);
    return NotificationNapi::NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationIsAlertOnce(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "parse isAlertOnce");
    napi_value result = nullptr;
    bool hasProperty = false;
    napi_valuetype valuetype = napi_undefined;
    bool isAlertOnce = false;
    NAPI_CALL(env, napi_has_named_property(env, value, "isAlertOnce", &hasProperty));
    if (!hasProperty) {
        return NotificationNapi::NapiGetNull(env);
    }
    napi_get_named_property(env, value, "isAlertOnce", &result);
    NAPI_CALL(env, napi_typeof(env, result, &valuetype));
    if (valuetype != napi_boolean) {
        LBSLOGE(NAPI_UTILS, "isAlertOnce type error");
        return nullptr;
    }
    napi_get_value_bool(env, result, &isAlertOnce);
    request.SetAlertOneTime(isAlertOnce);
    return NotificationNapi::NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationAutoDeletedTime(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "parse autoDeletedTime");
    napi_value result = nullptr;
    bool hasProperty = false;
    napi_valuetype valuetype = napi_undefined;
    int64_t autoDeletedTime = 0;
    NAPI_CALL(env, napi_has_named_property(env, value, "autoDeletedTime", &hasProperty));
    if (!hasProperty) {
        return NotificationNapi::NapiGetNull(env);
    }
    napi_get_named_property(env, value, "autoDeletedTime", &result);
    NAPI_CALL(env, napi_typeof(env, result, &valuetype));
    if (valuetype != napi_number) {
        LBSLOGE(NAPI_UTILS, "autoDeletedTime type error");
        return nullptr;
    }
    napi_get_value_int64(env, result, &autoDeletedTime);
    request.SetAutoDeletedTime(autoDeletedTime);
    return NotificationNapi::NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationGroupName(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "parse groupName");
    napi_value result = nullptr;
    bool hasProperty = false;
    napi_valuetype valuetype = napi_undefined;
    size_t strLen = 0;
    NAPI_CALL(env, napi_has_named_property(env, value, "groupName", &hasProperty));
    if (!hasProperty) {
        return NotificationNapi::NapiGetNull(env);
    }
    napi_get_named_property(env, value, "groupName", &result);
    NAPI_CALL(env, napi_typeof(env, result, &valuetype));
    if (valuetype != napi_string) {
        LBSLOGE(NAPI_UTILS, "groupName type error");
        return nullptr;
    }
    char str[STR_MAX_SIZE] = {0};
    NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
    request.SetGroupName(str);
    return NotificationNapi::NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationIsStopwatch(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "parse isStopwatch");
    napi_value result = nullptr;
    bool hasProperty = false;
    napi_valuetype valuetype = napi_undefined;
    bool isStopwatch = false;
    NAPI_CALL(env, napi_has_named_property(env, value, "isStopwatch", &hasProperty));
    if (!hasProperty) {
        return NotificationNapi::NapiGetNull(env);
    }
    napi_get_named_property(env, value, "isStopwatch", &result);
    NAPI_CALL(env, napi_typeof(env, result, &valuetype));
    if (valuetype != napi_boolean) {
        LBSLOGE(NAPI_UTILS, "isStopwatch type error");
        return nullptr;
    }
    napi_get_value_bool(env, result, &isStopwatch);
    request.SetShowStopwatch(isStopwatch);
    return NotificationNapi::NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationIsCountDown(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "parse isCountDown");
    napi_value result = nullptr;
    bool hasProperty = false;
    napi_valuetype valuetype = napi_undefined;
    bool isCountDown = false;
    NAPI_CALL(env, napi_has_named_property(env, value, "isCountDown", &hasProperty));
    if (!hasProperty) {
        return NotificationNapi::NapiGetNull(env);
    }
    napi_get_named_property(env, value, "isCountDown", &result);
    NAPI_CALL(env, napi_typeof(env, result, &valuetype));
    if (valuetype != napi_boolean) {
        LBSLOGE(NAPI_UTILS, "isCountDown type error");
        return nullptr;
    }
    napi_get_value_bool(env, result, &isCountDown);
    request.SetCountdownTimer(isCountDown);
    return NotificationNapi::NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationtapDismissed(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "parse tapDismissed");
    napi_value result = nullptr;
    bool hasProperty = false;
    napi_valuetype valuetype = napi_undefined;
    bool tapDismissed = true;
    NAPI_CALL(env, napi_has_named_property(env, value, "tapDismissed", &hasProperty));
    if (!hasProperty) {
        return NotificationNapi::NapiGetNull(env);
    }
    napi_get_named_property(env, value, "tapDismissed", &result);
    NAPI_CALL(env, napi_typeof(env, result, &valuetype));
    if (valuetype != napi_boolean) {
        LBSLOGE(NAPI_UTILS, "tapDismissed type error");
        return nullptr;
    }
    napi_get_value_bool(env, result, &tapDismissed);
    request.SetTapDismissed(tapDismissed);
    return NotificationNapi::NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationIsOngoing(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "parse isOngoing");
    napi_value result = nullptr;
    bool hasProperty = false;
    napi_valuetype valuetype = napi_undefined;
    bool isOngoing = false;
    NAPI_CALL(env, napi_has_named_property(env, value, "isOngoing", &hasProperty));
    if (!hasProperty) {
        return NotificationNapi::NapiGetNull(env);
    }
    napi_get_named_property(env, value, "isOngoing", &result);
    NAPI_CALL(env, napi_typeof(env, result, &valuetype));
    if (valuetype != napi_boolean) {
        LBSLOGE(NAPI_UTILS, "isOngoing type error");
        return nullptr;
    }
    napi_get_value_bool(env, result, &isOngoing);
    request.SetInProgress(isOngoing);
    return NotificationNapi::NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationContentType(const napi_env &env, const napi_value &result, int32_t &type)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_value contentResult = nullptr;
    napi_valuetype valuetype = napi_undefined;
    bool hasNotificationContentType = false;
    bool hasContentType = false;

    NAPI_CALL(env, napi_has_named_property(env, result, "notificationContentType", &hasNotificationContentType));
    if (hasNotificationContentType) {
        napi_get_named_property(env, result, "notificationContentType", &contentResult);
        NAPI_CALL(env, napi_typeof(env, contentResult, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int32(env, contentResult, &type);

        return NapiGetNull(env);
    }

    NAPI_CALL(env, napi_has_named_property(env, result, "contentType", &hasContentType));
    if (hasContentType) {
        napi_get_named_property(env, result, "contentType", &contentResult);
        NAPI_CALL(env, napi_typeof(env, contentResult, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int32(env, contentResult, &type);

        return NapiGetNull(env);
    } else {
        LBSLOGE(NAPI_UTILS, "Property contentType expected.");
        return nullptr;
    }
}

static bool getStr(const char *key, size_t maxLen, std::string &out, napi_value normal, napi_env env)
{
    napi_value v = nullptr;
    bool h = false;
    napi_valuetype t;
    NAPI_CALL_BASE(env, napi_has_named_property(env, normal, key, &h), false);
    if (!h) {
        return false;
    }
    napi_get_named_property(env, normal, key, &v);
    napi_typeof(env, v, &t);
    if (t != napi_string) {
        return false;
    }
    std::vector<char> buf(maxLen + 1, '\0');
    size_t len = 0;
    napi_get_value_string_utf8(env, v, buf.data(), maxLen, &len);
    if (len == 0) {
        return false;
    }
    out.assign(buf.data(), len);
    return true;
}

napi_value NotificationNapi::GetNotificationContent(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    bool hasContent = false;
    napi_value content = nullptr;
    NAPI_CALL(env, napi_has_named_property(env, value, "content", &hasContent));
    if (!hasContent) {
        LBSLOGE(NAPI_UTILS, "content missing");
        return nullptr;
    }
    napi_get_named_property(env, value, "content", &content);
    napi_valuetype vt;
    napi_typeof(env, content, &vt);
    if (vt != napi_object) {
        LBSLOGE(NAPI_UTILS, "content not object");
        return nullptr;
    }
    bool has = false;
    napi_value normal = nullptr;
    NAPI_CALL(env, napi_has_named_property(env, content, "normal", &has));
    if (!has) {
        LBSLOGE(NAPI_UTILS, "normal missing");
        return nullptr;
    }
    napi_get_named_property(env, content, "normal", &normal);
    napi_typeof(env, normal, &vt);
    if (vt != napi_object) {
        LBSLOGE(NAPI_UTILS, "normal not object");
        return nullptr;
    }
    std::string title;
    std::string text;
    if (!getStr("title", MAX_TITLE_LENGTH, title, normal, env) ||
        !getStr("text", MAX_TEXT_LENGTH, text, normal, env)) {
        return nullptr;
    }
    auto normalContent = std::make_shared<NotificationNormalContent>();
    if (!normalContent) {
        return nullptr;
    }
    normalContent->SetTitle(title);
    normalContent->SetText(text);
    request.SetContent(std::make_shared<NotificationContent>(normalContent));
    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationRequest(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter (trimmed)");
    if (!GetNotificationId(env, value, request)) {
        return nullptr;
    }
    if (!GetNotificationBadgeNumber(env, value, request)) {
        return nullptr;
    }
    if (!GetNotificationDeliveryTime(env, value, request)) {
        return nullptr;
    }
    if (!GetNotificationShowDeliveryTime(env, value, request)) {
        return nullptr;
    }
    if (!GetNotificationIsAlertOnce(env, value, request)) {
        return nullptr;
    }
    if (!GetNotificationAutoDeletedTime(env, value, request)) {
        return nullptr;
    }
    if (!GetNotificationGroupName(env, value, request)) {
        return nullptr;
    }
    if (!GetNotificationIsStopwatch(env, value, request)) {
        return nullptr;
    }
    if (!GetNotificationIsCountDown(env, value, request)) {
        return nullptr;
    }
    if (!GetNotificationtapDismissed(env, value, request)) {
        return nullptr;
    }
    if (!GetNotificationIsOngoing(env, value, request)) {
        return nullptr;
    }
    if (!GetNotificationContent(env, value, request)) {
        return nullptr;
    }
    return NotificationNapi::NapiGetNull(env);
}
}
}
