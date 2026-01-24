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

#ifndef OHOS_GLOBAL_HOLIDAY_MANAGER_H
#define OHOS_GLOBAL_HOLIDAY_MANAGER_H

#include <map>
#include <string>
#include <vector>

namespace OHOS {
namespace Global {
namespace I18n {
struct HolidayLocalName {
    // Holiday locale name language id
    std::string language = "";

    // Holiday local name
    std::string name = "";
};

struct HolidayInfoItem {
    // Holiday base name
    std::string baseName;

    // Holiday start year
    int32_t year = 0;

    // Holiday start month
    int32_t month = 0;

    // Holiday start day
    int32_t day = 0;

    // Holiday local name array
    std::vector<HolidayLocalName> localNames;
};

class HolidayManager {
public:
    HolidayManager(const char* path);
    ~HolidayManager();
    bool IsHoliday();
    bool IsHoliday(int32_t year, int32_t month, int32_t day);
    std::vector<HolidayInfoItem> GetHolidayInfoItemArray(int32_t year);
    std::vector<HolidayInfoItem> GetHolidayInfoItemArray();
    void SetHolidayData(std::map<std::string, std::vector<HolidayInfoItem>> holidayDataMap);

private:
    std::vector<HolidayInfoItem> ReadHolidayFile(const std::string& path);
    void ParseFileLine(const std::string& line, HolidayInfoItem* holidayItem);
    std::string GetLanguageFromPath(const char* path);
    static std::string& Trim(std::string& str);
    std::string Format(int32_t year, int32_t month, int32_t day);
    std::map<std::string, std::vector<HolidayInfoItem>> holidayItemMap;
    static const char* ITEM_BEGIN_TAG;
    static const char* ITEM_END_TAG;
    static const char* ITEM_DTSTART_TAG;
    static const char* ITEM_DTEND_TAG;
    static const char* ITEM_SUMMARY_TAG;
    static const char* ITEM_RESOURCES_TAG;
    static const int32_t MONTH_GREATER_ONE = 1;
    static const int32_t YEAR_START = 1900;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif