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

#include "init_module.h"

#include "constant.h"
#include "js_runtime_utils.h"
#include "log.h"
#include "napi/native_api.h"
#include "napi_cancel.h"
#include "napi_display_badge.h"
#include "napi_enable_notification.h"
#include "napi_publish.h"
#ifdef ANDROID_PLATFORM
#include "plugin_utils.h"
#include "android/java/jni/notification_jni.h"
#endif

namespace OHOS {
namespace NotificationNapi {
using namespace OHOS::Notification;

EXTERN_C_START

napi_value NotificationManagerInit(napi_env env, napi_value exports)
{
    LOGD("NotificationManagerInit start");

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("publish", NapiPublish),
        DECLARE_NAPI_FUNCTION("cancel", NapiCancel),
        DECLARE_NAPI_FUNCTION("cancelAll", NapiCancelAll),
        DECLARE_NAPI_FUNCTION("isNotificationEnabled", NapiIsNotificationEnabled),
        DECLARE_NAPI_FUNCTION("setBadgeNumber", NapiSetBadgeNumber),
        DECLARE_NAPI_FUNCTION("requestEnableNotification", NapiRequestEnableNotification),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    return exports;
}

/*
 * Module export function
 */
static napi_value Init(napi_env env, napi_value exports)
{
    /*
     * Propertise define
     */
    NotificationManagerInit(env, exports);

    ConstantInit(env, exports);

    return exports;
}
#ifdef ANDROID_PLATFORM
static void NotificationManagerPluginJniRegister()
{
    const char className[] = "ohos.ace.plugin.notificationmanager.NotificationPlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&OHOS::Notification::NotificationJni::Register, className);
}
#endif

/*
 * Module register function
 */
__attribute__((constructor)) void RegisterModule(void)
{
    LOGD("RegisterModule notification");
#ifdef ANDROID_PLATFORM
    NotificationManagerPluginJniRegister();
#endif
    napi_module_register(&_module_manager);
}
EXTERN_C_END
} // namespace NotificationNapi
} // namespace OHOS
