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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_CORE_INCLUDE_ANS_CONVER_ENUM_H
#define BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_CORE_INCLUDE_ANS_CONVER_ENUM_H

#include "notification_content.h"

namespace OHOS {
namespace NotificationNapi {
using namespace OHOS::Notification;

enum class ContentType {
    NOTIFICATION_CONTENT_BASIC_TEXT,
    NOTIFICATION_CONTENT_LONG_TEXT,
    NOTIFICATION_CONTENT_PICTURE,
    NOTIFICATION_CONTENT_CONVERSATION,
    NOTIFICATION_CONTENT_MULTILINE,
    NOTIFICATION_CONTENT_LOCAL_LIVE_VIEW,
    NOTIFICATION_CONTENT_LIVE_VIEW
};

class AnsEnumUtil {
public:
    /**
     * @brief Converts content type from js to native
     *
     * @param inType Indicates a js ContentType object
     * @param outType Indicates a NotificationContent object
     * @return Returns true if success, returns false otherwise
     */
    static bool ContentTypeJSToC(const ContentType &inType, NotificationContent::Type &outType);
};
}
}

#endif
