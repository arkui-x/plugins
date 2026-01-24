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
#include "zone_offset_transition.h"

#include "locale_config.h"
#include "log.h"
#include "zone_rules.h"

namespace OHOS {
namespace Global {
namespace I18n {
ZoneOffsetTransition::ZoneOffsetTransition(double time, int32_t offsetBefore, int32_t offsetAfter)
    : time(time), offsetBefore(offsetBefore), offsetAfter(offsetAfter)
{}

ZoneOffsetTransition::ZoneOffsetTransition()
{}

ZoneOffsetTransition::~ZoneOffsetTransition()
{}

double ZoneOffsetTransition::GetMilliseconds()
{
    return time;
}

int32_t ZoneOffsetTransition::GetOffsetAfter()
{
    return offsetAfter;
}

int32_t ZoneOffsetTransition::GetOffsetBefore()
{
    return offsetBefore;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS
