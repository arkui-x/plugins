/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "zone_rules.h"

#include "log.h"
#include <chrono>
#include "unicode/tzrule.h"
#include "unicode/tztrans.h"
#include "unicode/gregocal.h"

namespace OHOS {
namespace Global {
namespace I18n {
ZoneRules::ZoneRules(const std::string &tzid)
{
    this->tzId = tzid;
    InitBasicTimeZone();
}

ZoneRules::~ZoneRules()
{
}

void ZoneRules::InitBasicTimeZone()
{
    if (tzId.empty()) {
        LOGE("ZoneRules::InitBasicTimeZone: param tzid is empty.");
        return;
    }
    icu::UnicodeString unicodeZoneId(tzId.data(), tzId.length());
    icu::TimeZone* icuTz = icu::TimeZone::createTimeZone(unicodeZoneId);
    if (icuTz == nullptr) {
        LOGE("ZoneRules::InitBasicTimeZone: param tzid: %{public}s invalid.", tzId.c_str());
        return;
    }
    icu::BasicTimeZone* btzPtr = static_cast<icu::BasicTimeZone*>(icuTz);
    if (btzPtr == nullptr) {
        LOGE("ZoneRules::InitBasicTimeZone: static_cast icu::TimeZone failed.");
        return;
    }
    this->btz = std::unique_ptr<icu::BasicTimeZone>(btzPtr);
    if (btz == nullptr) {
        LOGE("ZoneRules::InitBasicTimeZone: btz is nullptr.");
    }
}

std::unique_ptr<ZoneOffsetTransition> ZoneRules::NextTransition()
{
    auto time = std::chrono::system_clock::now();
    auto since_epoch = time.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch);
    UDate currentTime = static_cast<double>(millis.count());
    return NextTransition(currentTime);
}

std::unique_ptr<ZoneOffsetTransition> ZoneRules::NextTransition(double date)
{
    if (btz == nullptr) {
        LOGE("ZoneRules::NextTransition: btz is nullptr.");
        return std::make_unique<ZoneOffsetTransition>();
    }
    icu::TimeZoneTransition trans;
    if (!btz->getNextTransition(date, false, trans)) {
        LOGE("ZoneRules::nextTransition: get next transition after %{public}s failed.",
            std::to_string(date).c_str());
        return std::make_unique<ZoneOffsetTransition>();
    }
    const icu::TimeZoneRule* from = trans.getFrom();
    const icu::TimeZoneRule* to = trans.getTo();
    if (from == nullptr || to == nullptr) {
        return std::make_unique<ZoneOffsetTransition>();
    }
    double time = trans.getTime();
    int32_t offsetBefore = from->getRawOffset() + from->getDSTSavings();
    int32_t offsetAfter = to->getRawOffset() + to->getDSTSavings();
    return std::make_unique<ZoneOffsetTransition>(time, offsetBefore, offsetAfter);
}
} // namespace I18n
} // namespace Global
} // namespace OHOS
