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

#include "notification_request.h"

#include "ans_inner_errors.h"
#include "log.h"
#include "errors.h"
#include <memory>

namespace OHOS {
namespace Notification {

NotificationRequest::NotificationRequest(int32_t notificationId) : notificationId_(notificationId)
{
    deliveryTime_ = GetNowSysTime();
}

NotificationRequest::NotificationRequest(const NotificationRequest &other)
{
    CopyBase(other);
    CopyOther(other);
}

NotificationRequest &NotificationRequest::operator=(const NotificationRequest &other)
{
    CopyBase(other);
    CopyOther(other);

    return *this;
}

NotificationRequest::~NotificationRequest()
{}

bool NotificationRequest::IsInProgress() const
{
    return inProgress_;
}

void NotificationRequest::SetInProgress(bool isOngoing)
{
    inProgress_ = isOngoing;
}

void NotificationRequest::SetBadgeNumber(uint32_t number)
{
    badgeNumber_ = number;
}

uint32_t NotificationRequest::GetBadgeNumber() const
{
    return badgeNumber_;
}

void NotificationRequest::SetNotificationId(int32_t notificationId)
{
    notificationId_ = notificationId;
}

int32_t NotificationRequest::GetNotificationId() const
{
    return notificationId_;
}

void NotificationRequest::SetDeliveryTime(int64_t deliveryTime)
{
    deliveryTime_ = deliveryTime;
}

int64_t NotificationRequest::GetDeliveryTime() const
{
    return deliveryTime_;
}

bool NotificationRequest::IsShowDeliveryTime() const
{
    return (deliveryTime_ != 0) && showDeliveryTime_;
}

void NotificationRequest::SetShowDeliveryTime(bool showDeliveryTime)
{
    showDeliveryTime_ = showDeliveryTime;
}

bool NotificationRequest::IsAlertOneTime() const
{
    return alertOneTime_;
}

void NotificationRequest::SetAlertOneTime(bool isAlertOnce)
{
    alertOneTime_ = isAlertOnce;
}

void NotificationRequest::SetAutoDeletedTime(int64_t deletedTime)
{
    autoDeletedTime_ = deletedTime;
}

int64_t NotificationRequest::GetAutoDeletedTime() const
{
    return autoDeletedTime_;
}

void NotificationRequest::SetContent(const std::shared_ptr<NotificationContent> &content)
{
    notificationContent_ = content;

    if (notificationContent_) {
        notificationContentType_ = notificationContent_->GetContentType();
        return;
    }

    notificationContentType_ = NotificationContent::Type::NONE;
}

const std::shared_ptr<NotificationContent> NotificationRequest::GetContent() const
{
    return notificationContent_;
}

NotificationContent::Type NotificationRequest::GetNotificationType() const
{
    return notificationContentType_;
}

bool NotificationRequest::IsCountdownTimer() const
{
    return isCountdown_;
}

void NotificationRequest::SetCountdownTimer(bool isCountDown)
{
    isCountdown_ = isCountDown;
}

bool NotificationRequest::IsShowStopwatch() const
{
    return showStopwatch_;
}

void NotificationRequest::SetShowStopwatch(bool isShow)
{
    showStopwatch_ = isShow;
}

bool NotificationRequest::IsTapDismissed() const
{
    return tapDismissed_;
}

void NotificationRequest::SetTapDismissed(bool isDismissed)
{
    tapDismissed_ = isDismissed;
}

void NotificationRequest::SetGroupName(const std::string &groupName)
{
    groupName_ = groupName;
}

std::string NotificationRequest::GetGroupName() const
{
    return groupName_;
}

bool NotificationRequest::ToJson(nlohmann::json &jsonObject) const
{
    jsonObject["version"]         = 1;

    jsonObject["id"]              = GetNotificationId();
    jsonObject["deliveryTime"]    = GetDeliveryTime();
    jsonObject["autoDeletedTime"] = GetAutoDeletedTime();
    jsonObject["notificationContentType"] = static_cast<int32_t>(GetNotificationType());

    jsonObject["showDeliveryTime"] = IsShowDeliveryTime();
    jsonObject["groupName"]        = GetGroupName();
    jsonObject["tapDismissed"]     = IsTapDismissed();
    jsonObject["isOngoing"]        = IsInProgress();
    jsonObject["isAlertOnce"]      = IsAlertOneTime();
    jsonObject["isStopwatch"]      = IsShowStopwatch();
    jsonObject["isCountdown"]      = IsCountdownTimer();
    jsonObject["badgeNumber"]      = GetBadgeNumber();
    

    if (!ConvertObjectsToJson(jsonObject)) {
        LOGE("Cannot convert objects to JSON");
        return false;
    }

    return true;
}

bool NotificationRequest::ConvertObjectsToJson(nlohmann::json &jsonObject) const
{
    nlohmann::json contentObj;
    if (GetContent()) {
        if (!NotificationJsonConverter::ConvertToJson(notificationContent_.get(), contentObj)) {
            LOGE("Cannot convert notificationContent to JSON");
            return false;
        }
    }
    jsonObject["content"] = contentObj;
    return true;
}


int64_t NotificationRequest::GetNowSysTime()
{
    std::chrono::time_point<std::chrono::system_clock> nowSys = std::chrono::system_clock::now();
    auto epoch = nowSys.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
    int64_t duration = value.count();
    return duration;
}

void NotificationRequest::CopyBase(const NotificationRequest &other)
{
    this->notificationId_ = other.notificationId_;
    this->badgeNumber_ = other.badgeNumber_;
    this->deliveryTime_ = other.deliveryTime_;
    this->groupName_ = other.groupName_;
    this->autoDeletedTime_ = other.autoDeletedTime_;
    this->notificationContentType_ = other.notificationContentType_;
}

void NotificationRequest::CopyOther(const NotificationRequest &other)
{
    this->showDeliveryTime_ = other.showDeliveryTime_;
    this->tapDismissed_ = other.tapDismissed_;
    this->alertOneTime_ = other.alertOneTime_;
    this->showStopwatch_ = other.showStopwatch_;
    this->isCountdown_ = other.isCountdown_;
    this->inProgress_ = other.inProgress_;
    this->notificationContent_ = other.notificationContent_;
}
}  // namespace Notification
}  // namespace OHOS
