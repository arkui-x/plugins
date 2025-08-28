/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "time_service_client.h"

#include <cinttypes>
#include <mutex>

#ifdef ANDROID_PLATFORM
#include "android/java/jni/time_jni.h"
#endif
#ifdef IOS_PLATFORM
#include "ios/time_ios.h"
#endif
#include "time_hilog.h"

namespace OHOS {
namespace MiscServices {
namespace {
static const int MILLI_TO_SEC = 1000LL;
static const int NANO_TO_SEC = 1000000000LL;
constexpr int32_t NANO_TO_MILLI = NANO_TO_SEC / MILLI_TO_SEC;
} // namespace

std::mutex TimeServiceClient::instanceLock_;
sptr<TimeServiceClient> TimeServiceClient::instance_;

TimeServiceClient::TimeServiceListener::TimeServiceListener() {}

void TimeServiceClient::TimeServiceListener::OnAddSystemAbility(int32_t saId, const std::string& deviceId) {}

void TimeServiceClient::TimeServiceListener::OnRemoveSystemAbility(int32_t asId, const std::string& deviceId) {}

TimeServiceClient::TimeServiceClient()
{
    listener_ = nullptr;
}

TimeServiceClient::~TimeServiceClient() {}

sptr<TimeServiceClient> TimeServiceClient::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (instance_ == nullptr) {
            instance_ = new TimeServiceClient;
        }
    }
    return instance_;
}

bool TimeServiceClient::SubscribeSA(sptr<ISystemAbilityManager> systemAbilityManager)
{
    return true;
}

bool TimeServiceClient::ConnectService()
{
    return true;
}

bool TimeServiceClient::GetTimeByClockId(clockid_t clockId, struct timespec& tv)
{
    return true;
}

bool TimeServiceClient::SetTime(int64_t time)
{
    return true;
}

bool TimeServiceClient::SetTime(int64_t milliseconds, int32_t& code)
{
    return true;
}

int32_t TimeServiceClient::SetTimeV9(int64_t time)
{
    return 0;
}

bool TimeServiceClient::SetTimeZone(const std::string& timezoneId)
{
    return true;
}

bool TimeServiceClient::SetTimeZone(const std::string& timezoneId, int32_t& code)
{
    return true;
}

int32_t TimeServiceClient::SetTimeZoneV9(const std::string& timezoneId)
{
    return 0;
}

uint64_t TimeServiceClient::CreateTimer(std::shared_ptr<ITimerInfo> timerOptions)
{
    return 0;
}

int32_t TimeServiceClient::CreateTimerV9(std::shared_ptr<ITimerInfo> timerOptions, uint64_t& timerId)
{
    return 0;
}

bool TimeServiceClient::StartTimer(uint64_t timerId, uint64_t triggerTime)
{
    return true;
}

int32_t TimeServiceClient::StartTimerV9(uint64_t timerId, uint64_t triggerTime)
{
    return 0;
}

bool TimeServiceClient::StopTimer(uint64_t timerId)
{
    return true;
}

int32_t TimeServiceClient::StopTimerV9(uint64_t timerId)
{
    return 0;
}

bool TimeServiceClient::DestroyTimer(uint64_t timerId)
{
    return true;
}

int32_t TimeServiceClient::DestroyTimerV9(uint64_t timerId)
{
    return 0;
}

bool TimeServiceClient::DestroyTimerAsync(uint64_t timerId)
{
    return true;
}

int32_t TimeServiceClient::DestroyTimerAsyncV9(uint64_t timerId)
{
    return 0;
}

std::string TimeServiceClient::GetTimeZone()
{
    return "";
}

int32_t TimeServiceClient::GetTimeZone(std::string& timezoneId)
{
#if defined(ANDROID_PLATFORM)
    return Time::TimeJni::GetTimeZone(timezoneId);
#elif defined(IOS_PLATFORM)
    return Time::TimeIOS::GetTimeZone(timezoneId);
#else
    timezoneId = "Asia/Shanghai";
    return 0;
#endif
}

int64_t TimeServiceClient::GetWallTimeMs()
{
    return 0;
}

int32_t TimeServiceClient::GetWallTimeMs(int64_t& time)
{
    return 0;
}

int64_t TimeServiceClient::GetWallTimeNs()
{
    return 0;
}

int32_t TimeServiceClient::GetWallTimeNs(int64_t& time)
{
    return 0;
}

int64_t TimeServiceClient::GetBootTimeMs()
{
    return 0;
}

int32_t TimeServiceClient::GetBootTimeMs(int64_t& time)
{
    return 0;
}

int64_t TimeServiceClient::GetBootTimeNs()
{
    return 0;
}

int32_t TimeServiceClient::GetBootTimeNs(int64_t& time)
{
    return 0;
}

int64_t TimeServiceClient::GetMonotonicTimeMs()
{
    return 0;
}

int32_t TimeServiceClient::GetMonotonicTimeMs(int64_t& time)
{
    return 0;
}

int64_t TimeServiceClient::GetMonotonicTimeNs()
{
    return 0;
}

int32_t TimeServiceClient::GetMonotonicTimeNs(int64_t& time)
{
    return 0;
}

int64_t TimeServiceClient::GetThreadTimeMs()
{
    return 0;
}

int32_t TimeServiceClient::GetThreadTimeMs(int64_t& time)
{
    return 0;
}

int64_t TimeServiceClient::GetThreadTimeNs()
{
    return 0;
}

int32_t TimeServiceClient::GetThreadTimeNs(int64_t& time)
{
    return 0;
}

bool TimeServiceClient::ProxyTimer(int32_t uid, std::set<int> pidList, bool isProxy, bool needRetrigger)
{
    return true;
}

int32_t TimeServiceClient::AdjustTimer(bool isAdjust, uint32_t interval, uint32_t delta)
{
    return 0;
}

int32_t TimeServiceClient::SetTimerExemption(const std::unordered_set<std::string>& nameArr, bool isExemption)
{
    return 0;
}

bool TimeServiceClient::ResetAllProxy()
{
    return true;
}

int32_t TimeServiceClient::GetNtpTimeMs(int64_t& time)
{
    return 0;
}

int32_t TimeServiceClient::GetRealTimeMs(int64_t& time)
{
    return 0;
}

sptr<ITimeService> TimeServiceClient::GetProxy()
{
    return nullptr;
}

void TimeServiceClient::SetProxy(sptr<ITimeService> proxy) {}

void TimeServiceClient::ClearProxy() {}

int32_t TimeServiceClient::SetAutoTime(bool autoTime)
{
    return 0;
}
} // namespace MiscServices
} // namespace OHOS