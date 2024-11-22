/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "notification_impl.h"

#include <cstring>
#include <map>
#include <mutex>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "notification_jni.h"

namespace OHOS {
namespace Notification {
std::shared_ptr<AnsNotification> AnsNotification::GetInstance()
{
    return std::make_shared<NotificationImpl>();
}
void NotificationImpl::PublishNotification(
    const NotificationRequest& request, void* data, NotificationCallback callBack)
{
    if (!NotificationJni::GetInstance().isGranted()) {
        callBack(data, ERR_ANS_NOT_ALLOWED);
        return;
    }
    callBack(data, NotificationJni::GetInstance().publish(request));
}

ErrorCode NotificationImpl::CancelNotification(int32_t id)
{
    return NotificationJni::GetInstance().cancel(id, "");
}
ErrorCode NotificationImpl::CancelAllNotifications()
{
    return NotificationJni::GetInstance().cancelAll();
}
ErrorCode NotificationImpl::SetBadgeNumber(int32_t number)
{
    return NotificationJni::GetInstance().setBadgeNumber(number);
}
void NotificationImpl::RequestEnableNotification(void* data, NotificationCallback callback)
{
    if (!NotificationJni::GetInstance().isHighVersion()) {
        callback(data, NotificationJni::GetInstance().isGranted() ? ERR_ANS_OK : ERR_ANS_NOT_ALLOWED);
        return;
    }
    if (NotificationJni::GetInstance().isGranted()) {
        callback(data, NotificationJni::GetInstance().isGranted() ? ERR_ANS_OK : ERR_ANS_NOT_ALLOWED);
        return;
    }

    OHOS::Plugin::PluginUtilsInner::RunTaskOnPlatform([callback, data]() {
        auto task = [callback, data](const std::vector<std::string> perms, const std::vector<int> result) {
            LOGI("NotificationImpl JSRegisterGrantResult callback");
            std::vector<int> grantResult = result;
            if (grantResult.empty() || grantResult[0] != ERR_ANS_OK) {
                callback(data, ERR_ANS_NOT_ALLOWED);
            } else {
                callback(data, grantResult[0]);
            }
        };
        OHOS::Plugin::PluginUtilsInner::JSRegisterGrantResult(task);
        LOGI("NotificationImpl JSRegisterGrantResult");
    });
    OHOS::Plugin::PluginUtilsInner::RunTaskOnPlatform(
        []() { NotificationJni::GetInstance().RequestEnableNotification(); });
}
void NotificationImpl::IsAllowedNotifySelf(void* data, EnableNotificationCallback callback)
{
    callback(data, ERR_ANS_OK, NotificationJni::GetInstance().isGranted());
}
} // namespace Notification
} // namespace OHOS
