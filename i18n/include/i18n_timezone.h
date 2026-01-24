/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except", "in compliance with the License.
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
#ifndef OHOS_GLOBAL_I18N_TIMEZONE_H
#define OHOS_GLOBAL_I18N_TIMEZONE_H

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "i18n_types.h"
#include "unicode/timezone.h"

namespace OHOS {
namespace Global {
namespace I18n {
class I18nTimeZone {
public:
    I18nTimeZone(std::string &id, bool isZoneID);
    ~I18nTimeZone();
    int32_t GetOffset(double date);
    int32_t GetRawOffset();
    std::string GetID();
    std::string GetDisplayName();
    std::string GetDisplayName(bool isDST);
    std::string GetDisplayName(std::string localeStr);
    std::string GetDisplayName(std::string localeStr, bool isDST);
    static std::unique_ptr<I18nTimeZone> CreateInstance(std::string &id, bool isZoneID);
    static std::unordered_set<std::string> GetAvailableIDs();

private:
    static const char *DEFAULT_TIMEZONE;
    static std::unordered_set<std::string> availableIDs;
    icu::TimeZone *timezone = nullptr;

    icu::TimeZone* GetTimeZone();
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif