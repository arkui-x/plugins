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

#include "ans_convert_enum.h"
#include "log.h"

namespace OHOS {
namespace NotificationNapi {
bool AnsEnumUtil::ContentTypeJSToC(const ContentType &inType, NotificationContent::Type &outType)
{
    switch (inType) {
        case ContentType::NOTIFICATION_CONTENT_BASIC_TEXT:
            outType = NotificationContent::Type::BASIC_TEXT;
            break;
        case ContentType::NOTIFICATION_CONTENT_LONG_TEXT:
            outType = NotificationContent::Type::LONG_TEXT;
            break;
        case ContentType::NOTIFICATION_CONTENT_MULTILINE:
            outType = NotificationContent::Type::MULTILINE;
            break;
        case ContentType::NOTIFICATION_CONTENT_PICTURE:
            outType = NotificationContent::Type::PICTURE;
            break;
        case ContentType::NOTIFICATION_CONTENT_CONVERSATION:
            outType = NotificationContent::Type::CONVERSATION;
            break;
        case ContentType::NOTIFICATION_CONTENT_LOCAL_LIVE_VIEW:
            outType = NotificationContent::Type::LOCAL_LIVE_VIEW;
            break;
        case ContentType::NOTIFICATION_CONTENT_LIVE_VIEW:
            outType = NotificationContent::Type::LIVE_VIEW;
            break;
        default:
            LOGE("ContentType %{public}d is an invalid value", inType);
            return false;
    }
    return true;
}
}
}
