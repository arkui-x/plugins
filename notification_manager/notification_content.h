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

#ifndef BASE_NOTIFICATION_DISTRIBUTED_NOTIFICATION_SERVICE_INTERFACES_INNER_API_NOTIFICATION_CONTENT_H
#define BASE_NOTIFICATION_DISTRIBUTED_NOTIFICATION_SERVICE_INTERFACES_INNER_API_NOTIFICATION_CONTENT_H

#include "notification_basic_content.h"
#include "notification_long_text_content.h"
#include "notification_multiline_content.h"
#include "notification_normal_content.h"

namespace OHOS {
namespace Notification {
class NotificationContent : public NotificationJsonConvertionBase {
public:
    enum class Type {
        /**
         * invalid type
         */
        NONE,
        /**
         * Indicates basic notifications. Such notifications are created using NotificationNormalContent.
         */
        BASIC_TEXT,
        /**
         * Indicates notifications that include a conversation among multiple users.
         * Such notifications are created using NotificationConversationalContent.
         */
        CONVERSATION,
        /**
         * Indicates notifications that include long text.
         * Such notifications are created using NotificationLongTextContent.
         */
        LONG_TEXT,
        /**
         * Indicates notifications that include media playback sessions.
         * Such notifications are created using NotificationMediaContent.
         */
        MEDIA,
        /**
         * Indicates notifications that include multiple independent lines of text.
         * Such notifications are created using NotificationMultiLineContent.
         */
        MULTILINE,
        /**
         * Indicates notifications that include a picture.
         * Such notifications are created using NotificationPictureContent.
         */
        PICTURE,
        /**
         * Indicates notifications that include local live view.
         * Such notifications are created using NotificationLocalLiveViewContent.
         */
        LOCAL_LIVE_VIEW,
		/**
         * Indicates notifications that include a live view.
         * Such notifications are created using NotificationLiveViewContent.
         */
        LIVE_VIEW
    };

    /**
     * @brief A constructor used to create a NotificationNormalContent instance (obtained by calling
     * GetNotificationContent()) and set the content type to NotificationContent::Type::BASIC_TEXT (obtained by calling
     * GetContentType()).
     *
     * @param normalContent Indicates the NotificationNormalContent object.
     */
    explicit NotificationContent(const std::shared_ptr<NotificationNormalContent> &normalContent);

    /**
     * @brief A constructor used to create a NotificationLongTextContent instance (obtained by calling
     * GetNotificationContent()) and set the content type to NotificationContent::Type::LONG_TEXT (obtained by calling
     * GetContentType()).
     *
     * @param longTextContent Indicates the NotificationLongTextContent object.
     */
    explicit NotificationContent(const std::shared_ptr<NotificationLongTextContent> &longTextContent);

    /**
     * @brief A constructor used to create a NotificationMultiLineContent instance (obtained by calling
     * GetNotificationContent()) and set the content type to NotificationContent::Type::MULTILINE (obtained by calling
     * GetContentType()).
     *
     * @param multiLineContent Indicates the NotificationMultiLineContent object.
     */
    explicit NotificationContent(const std::shared_ptr<NotificationMultiLineContent> &multiLineContent);

    // /**
    //  * @brief A constructor used to create a NotificationLiveViewContent instance (obtained by calling
    //  * GetNotificationContent()) and set the content type to NotificationContent::Type::LIVE_VIEW (obtained by calling
    //  * GetContentType()).
    //  *
    //  * @param liveViewContent Indicates the NotificationMediaContent object.
    //  */
    // explicit NotificationContent(const std::shared_ptr<NotificationLiveViewContent> &liveViewContent);
    virtual ~NotificationContent();

    /**
     * @brief Obtains the type value of the notification content.
     *
     * @return Returns the type value of the current content, which can be
     * NotificationContent::Type::BASIC_TEXT,
     * NotificationContent::Type::LONG_TEXT,
     * NotificationContent::Type::PICTURE,
     * NotificationContent::Type::CONVERSATION,
     * NotificationContent::Type::MULTILINE,
     * NotificationContent::Type::MEDIA,
     * NotificationContent::Type::LIVE_VIEW, or
     * NotificationContent::Type::LOCAL_LIVE_VIEW
     */
    NotificationContent::Type GetContentType() const;

    /**
     * @brief Obtains the object matching the current notification content.
     *
     * @return Returns the content object, which can be NotificationLongTextContent,
     * NotificationNormalContent,
     * NotificationPictureContent,
     * NotificationMultiLineContent, or
     * NotificationMediaContent.
     */
    std::shared_ptr<NotificationBasicContent> GetNotificationContent() const;

    /**
     * @brief Converts a NotificationContent object into a Json.
     *
     * @param jsonObject Indicates the Json object.
     * @return Returns true if succeed; returns false otherwise.
     */
    bool ToJson(nlohmann::json &jsonObject) const override;

private:
    NotificationContent() = default;

private:
    NotificationContent::Type contentType_ {NotificationContent::Type::NONE};
    std::shared_ptr<NotificationBasicContent> content_ {};
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_DISTRIBUTED_NOTIFICATION_SERVICE_INTERFACES_INNER_API_NOTIFICATION_CONTENT_H
