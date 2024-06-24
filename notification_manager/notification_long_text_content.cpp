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

#include "notification_long_text_content.h"

#include <string>                            // for basic_string, operator+
#include <algorithm>                         // for min

#include "log.h"
#include "notification_basic_content.h"      // for NotificationBasicContent

namespace OHOS {
namespace Notification {
const std::size_t NotificationLongTextContent::MAX_LONGTEXT_LENGTH {1024};

NotificationLongTextContent::NotificationLongTextContent(const std::string &longText)
{
    SetLongText(longText);
}

void NotificationLongTextContent::SetExpandedTitle(const std::string &exTitle)
{
    expandedTitle_ = exTitle;
}

std::string NotificationLongTextContent::GetExpandedTitle() const
{
    return expandedTitle_;
}

void NotificationLongTextContent::SetBriefText(const std::string &briefText)
{
    briefText_ = briefText;
}

std::string NotificationLongTextContent::GetBriefText() const
{
    return briefText_;
}

void NotificationLongTextContent::SetLongText(const std::string &longText)
{
    if (longText.empty()) {
        longText_.clear();
        return;
    }

    auto length = std::min(NotificationLongTextContent::MAX_LONGTEXT_LENGTH, longText.length());
    longText_.assign(longText.begin(), longText.begin() + length);
}

std::string NotificationLongTextContent::GetLongText() const
{
    return longText_;
}
bool NotificationLongTextContent::ToJson(nlohmann::json &jsonObject) const
{
    if (!NotificationBasicContent::ToJson(jsonObject)) {
        LOGE("Cannot convert basicContent to JSON");
        return false;
    }

    jsonObject["longText"]      = GetLongText();
    jsonObject["expandedTitle"] = GetExpandedTitle();
    jsonObject["briefText"]     = GetBriefText();

    return true;
}
}  // namespace Notification
}  // namespace OHOS
