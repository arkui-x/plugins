/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <set>
#include <string>
#include "i18n_types.h"
#include "memory"
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
    static std::set<std::string> GetAvailableIDs(I18nErrorCode &errorCode);
    static std::set<std::string> GetAvailableZoneCityIDs();
    static std::string GetCityDisplayName(std::string &cityID, std::string &locale);

private:
    static const char *TIMEZONE_KEY;
    static const char *DEFAULT_TIMEZONE;
    static const char *CITY_TIMEZONE_DATA_PATH;
    static const char *DEVICE_CITY_TIMEZONE_DATA_PATH;
    static const char *DEFAULT_LOCALE;
    static const char *CITY_DISPLAYNAME_PATH;
    static const char *DEVICE_CITY_DISPLAYNAME_PATH;
    static const char *SUPPORTED_LOCALES_PATH;
    static const char *TIMEZONE_ROOT_TAG;
    static const char *TIMEZONE_SECOND_ROOT_TAG;
    static const char *CITY_DISPLAYNAME_ROOT_TAG;
    static const char *CITY_DISPLAYNAME_SECOND_ROOT_TAG;
    static const char *ZONEINFO_PATH;
    static const uint32_t ELEMENT_NUM = 2;
    static std::set<std::string> availableIDs;
    static std::set<std::string> supportedLocales;
    static std::set<std::string> availableZoneCityIDs;
    static std::map<std::string, std::string> city2TimeZoneID;
    static constexpr int SYS_PARAM_LEN = 128;
    static bool useDeviceCityDispName;
    icu::TimeZone *timezone = nullptr;

    static bool ReadTimeZoneData(const char *xmlPath);
    static std::string ComputeLocale(std::string &locale);
    icu::TimeZone* GetTimeZone();
    static std::string FindCityDisplayNameFromXml(std::string &cityID, std::string &locale);
    static bool GetSupportedLocales();
    static std::string GetFallBack(std::string &localeStr);
    static void GetTimezoneIDFromZoneInfo(std::set<std::string> &availableIDs, std::string &parentPath,
        std::string &parentName);
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif