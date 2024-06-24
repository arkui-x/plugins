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

#include "notification_multiline_content.h"

#include <algorithm>

#include "log.h"

namespace OHOS {
namespace Notification {
const std::vector<std::string>::size_type NotificationMultiLineContent::MAX_LINES {7};

void NotificationMultiLineContent::SetExpandedTitle(const std::string &exTitle)
{
    expandedTitle_ = exTitle;
}

std::string NotificationMultiLineContent::GetExpandedTitle() const
{
    return expandedTitle_;
}

void NotificationMultiLineContent::SetBriefText(const std::string &briefText)
{
    briefText_ = briefText;
}

std::string NotificationMultiLineContent::GetBriefText() const
{
    return briefText_;
}

void NotificationMultiLineContent::AddSingleLine(const std::string &oneLine)
{
    if (allLines_.size() >= NotificationMultiLineContent::MAX_LINES) {
        LOGW("already added seven lines");
        return;
    }

    allLines_.emplace_back(oneLine);
}

std::vector<std::string> NotificationMultiLineContent::GetAllLines() const
{
    return allLines_;
}

bool NotificationMultiLineContent::ToJson(nlohmann::json &jsonObject) const
{
    if (!NotificationBasicContent::ToJson(jsonObject)) {
        LOGE("Cannot convert basicContent to JSON");
        return false;
    }

    jsonObject["expandedTitle"] = GetExpandedTitle();
    jsonObject["briefText"]     = GetBriefText();
    jsonObject["allLines"]      = nlohmann::json(GetAllLines());

    return true;
}

}  // namespace Notification
}  // namespace OHOS
