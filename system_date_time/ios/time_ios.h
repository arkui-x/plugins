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
#ifndef PLUGINS_SYSTEM_DATE_TIME_IOS_TIME_IOS_H
#define PLUGINS_SYSTEM_DATE_TIME_IOS_TIME_IOS_H

#include <string>
#include <stdint.h>

namespace OHOS::Time {
class TimeIOS final {
public:
    TimeIOS() = delete;
    ~TimeIOS() = delete;
    static int32_t GetTimeZone(std::string& timezoneId);
};
} // namespace OHOS::Time
#endif