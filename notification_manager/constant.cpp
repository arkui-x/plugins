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

#include "constant.h"

namespace OHOS {
namespace NotificationNapi {
using namespace OHOS::Notification;

void SetNamedPropertyByInteger(napi_env env, napi_value dstObj, int32_t objName, const char *propName)
{
    napi_value prop = nullptr;
    if (napi_create_int32(env, objName, &prop) == napi_ok) {
        napi_set_named_property(env, dstObj, propName, prop);
    }
}

napi_value ContentTypeInit(napi_env env, napi_value exports)
{
    LOGD("%{public}s, called", __func__);

    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    SetNamedPropertyByInteger(
        env, obj, (int32_t)ContentType::NOTIFICATION_CONTENT_BASIC_TEXT, "NOTIFICATION_CONTENT_BASIC_TEXT");
    SetNamedPropertyByInteger(
        env, obj, (int32_t)ContentType::NOTIFICATION_CONTENT_LONG_TEXT, "NOTIFICATION_CONTENT_LONG_TEXT");
    SetNamedPropertyByInteger(
        env, obj, (int32_t)ContentType::NOTIFICATION_CONTENT_PICTURE, "NOTIFICATION_CONTENT_PICTURE");
    SetNamedPropertyByInteger(
        env, obj, (int32_t)ContentType::NOTIFICATION_CONTENT_CONVERSATION, "NOTIFICATION_CONTENT_CONVERSATION");
    SetNamedPropertyByInteger(
        env, obj, (int32_t)ContentType::NOTIFICATION_CONTENT_MULTILINE, "NOTIFICATION_CONTENT_MULTILINE");
    SetNamedPropertyByInteger(
        env, obj, (int32_t)ContentType::NOTIFICATION_CONTENT_LOCAL_LIVE_VIEW, "NOTIFICATION_CONTENT_SYSTEM_LIVE_VIEW");
    SetNamedPropertyByInteger(
        env, obj, (int32_t)ContentType::NOTIFICATION_CONTENT_LIVE_VIEW, "NOTIFICATION_CONTENT_LIVE_VIEW");

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("ContentType", obj),
    };

    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    return exports;
}

napi_value ConstantInit(napi_env env, napi_value exports)
{
    ContentTypeInit(env, exports);
    return exports;
}
}  // namespace NotificationNapi
}  // namespace OHOS
