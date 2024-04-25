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

#ifndef BASE_NOTIFICATION_DISTRIBUTED_NOTIFICATION_SERVICE_INTERFACES_INNER_API_NOTIFICATION_MULTILINE_CONTENT_H
#define BASE_NOTIFICATION_DISTRIBUTED_NOTIFICATION_SERVICE_INTERFACES_INNER_API_NOTIFICATION_MULTILINE_CONTENT_H

#include "notification_basic_content.h"
#include <string>
#include <vector>

namespace OHOS {
namespace Notification {
class NotificationMultiLineContent : public NotificationBasicContent {
public:
    NotificationMultiLineContent() = default;

    ~NotificationMultiLineContent() = default;

    /**
     * @brief Sets the title to be displayed when this multi-line notification is expanded.
     * After this title is set, the title set by setTitle(string) will be displayed only
     * when this notification is in the collapsed state.
     *
     * @param exTitle Indicates the title to be displayed when this notification is expanded.
     */
    void SetExpandedTitle(const std::string &exTitle);

    /**
     * @brief Obtains the title that will be displayed for this multi-line notification when it is expanded.
     *
     * @return Returns the title to be displayed when this notification is expanded.
     */
    std::string GetExpandedTitle() const;

    /**
     * @brief Sets the brief text to be included in a multi-line notification.
     * The brief text is a summary of this multi-line notification and is displayed in the first line of
     * the notification. Similar to setAdditionalText(string), the font of the brief text is also
     * smaller than the notification text set by calling setText(string).
     * The positions where the brief text and additional text will display may conflict.
     * If both texts are set, only the additional text will be displayed.
     *
     * @param briefText Indicates the brief text to be included.
     */
    void SetBriefText(const std::string &briefText);

    /**
     * @brief Obtains the brief text that has been set by calling setBriefText(string) for this multi-line notification.
     *
     * @return Returns the brief text of this notification.
     */
    std::string GetBriefText() const;

    /**
     * @brief Adds a single line of text to this notification.
     * You can call this method up to seven times to add seven lines to a notification.
     *
     * @param oneLine Indicates the single line of text to be included.
     */
    void AddSingleLine(const std::string &oneLine);

    /**
     * @brief Obtains the list of lines included in this multi-line notification.
     *
     * @return Returns the list of lines included in this notification.
     */
    std::vector<std::string> GetAllLines() const;
	
	/**
     * @brief Converts a NotificationMultiLineContent object into a Json.
     *
     * @param jsonObject Indicates the Json object.
     * @return Returns true if succeed; returns false otherwise.
     */
    virtual bool ToJson(nlohmann::json &jsonObject) const override;

private:
    /**
     * the maximum size of vector is 7.
     */
    static const std::vector<std::string>::size_type MAX_LINES;

private:
    std::string expandedTitle_ {};
    std::string briefText_ {};
    std::vector<std::string> allLines_ {};
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_DISTRIBUTED_NOTIFICATION_SERVICE_INTERFACES_INNER_API_NOTIFICATION_MULTILINE_CONTENT_H