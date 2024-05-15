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

#include "notification_basic_content.h"
#include "log.h"

namespace OHOS {
namespace Notification {
NotificationBasicContent::~NotificationBasicContent()
{}

void NotificationBasicContent::SetText(const std::string &text)
{
    text_ = text;
}

std::string NotificationBasicContent::GetText() const
{
    return text_;
}

void NotificationBasicContent::SetTitle(const std::string &title)
{
    title_ = title;
}

std::string NotificationBasicContent::GetTitle() const
{
    return title_;
}
bool NotificationBasicContent::ToJson(nlohmann::json &jsonObject) const
{
    jsonObject["text"] = GetText();
    jsonObject["title"] = GetTitle();
    return true;
}
}  // namespace Notification
}  // namespace OHOS
