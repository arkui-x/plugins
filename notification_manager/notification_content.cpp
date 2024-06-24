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

#include "notification_content.h"
#include "log.h"

namespace OHOS {
namespace Notification {
NotificationContent::NotificationContent(const std::shared_ptr<NotificationNormalContent> &normalContent)
{
    if (!normalContent) {
        LOGE("NotificationNormalContent can not be null");
        return;
    }

    contentType_ = NotificationContent::Type::BASIC_TEXT;
    content_ = normalContent;
}

NotificationContent::NotificationContent(const std::shared_ptr<NotificationLongTextContent> &longTextContent)
{
    if (!longTextContent) {
        LOGE("NotificationLongTextContent can not be null");
        return;
    }

    contentType_ = NotificationContent::Type::LONG_TEXT;
    content_ = longTextContent;
}

NotificationContent::NotificationContent(const std::shared_ptr<NotificationMultiLineContent> &multiLineContent)
{
    if (!multiLineContent) {
        LOGE("NotificationMultiLineContent can not be null");
        return;
    }

    contentType_ = NotificationContent::Type::MULTILINE;
    content_ = multiLineContent;
}

NotificationContent::~NotificationContent()
{}

NotificationContent::Type NotificationContent::GetContentType() const
{
    return contentType_;
}

std::shared_ptr<NotificationBasicContent> NotificationContent::GetNotificationContent() const
{
    return content_;
}

bool NotificationContent::ToJson(nlohmann::json &jsonObject) const
{
    jsonObject["contentType"] = static_cast<int32_t>(GetContentType());
    jsonObject["notificationContentType"] = static_cast<int32_t>(GetContentType());

    if (!content_) {
        LOGE("Invalid content. Cannot convert to JSON.");
        return false;
    }

    nlohmann::json contentObj;
    if (!NotificationJsonConverter::ConvertToJson(GetNotificationContent().get(), contentObj)) {
        LOGE("Cannot convert content to JSON");
        return false;
    }
    jsonObject["content"] = contentObj;

    return true;
}
}  // namespace Notification
}  // namespace OHOS
