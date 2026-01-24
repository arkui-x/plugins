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
#ifndef GLOBAL_I18N_TIMEZONE_ZONE_OFFSET_TRANSITION_H
#define GLOBAL_I18N_TIMEZONE_ZONE_OFFSET_TRANSITION_H

#include <cstdint>

namespace OHOS {
namespace Global {
namespace I18n {
class ZoneOffsetTransition {
public:
    ZoneOffsetTransition();
    ZoneOffsetTransition(double time, int32_t offsetBefore, int32_t offsetAfter);
    ~ZoneOffsetTransition();
    double GetMilliseconds();
    int32_t GetOffsetAfter();
    int32_t GetOffsetBefore();

private:
    double time = 0.0;
    int32_t offsetBefore = 0;
    int32_t offsetAfter = 0;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif
