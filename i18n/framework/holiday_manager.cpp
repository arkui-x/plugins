/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "holiday_manager.h"
#include "log.h"
#include <algorithm>
#include <climits>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include "utils.h"

namespace OHOS {
namespace Global {
namespace I18n {
const char* HolidayManager::ITEM_BEGIN_TAG = "BEGIN:VEVENT";
const char* HolidayManager::ITEM_END_TAG = "END:VEVENT";
const char* HolidayManager::ITEM_DTSTART_TAG = "DTSTART";
const char* HolidayManager::ITEM_DTEND_TAG = "DTEND";
const char* HolidayManager::ITEM_SUMMARY_TAG = "SUMMARY";
const char* HolidayManager::ITEM_RESOURCES_TAG = "RESOURCES";

HolidayManager::HolidayManager(const char* path)
{
    if (path == nullptr) {
        LOGE("HolidayManager::HolidayManager: parameter path is NULL.");
        return;
    }
    std::string absolutePath = GetAbsoluteFilePath(path);
    if (absolutePath.empty()) {
        LOGE("HolidayManager::HolidayManager: holiday file path invalid.");
        return;
    }
    std::vector<HolidayInfoItem> items = ReadHolidayFile(absolutePath);
    for (size_t i = 0; i < items.size(); i++) {
        struct tm tmObj = {.tm_mday = items[i].day, .tm_mon = items[i].month, .tm_year = items[i].year};
        char strDate[10];
        size_t resCode = strftime(strDate, sizeof(strDate), "%Y%m%d", &tmObj);
        if (resCode == 0) {
            LOGE("HolidayManager::HolidayManager: strftime error:%{public}zu.", resCode);
            return;
        }
        std::string startDate(strDate);
        items[i].year += YEAR_START;
        items[i].month += MONTH_GREATER_ONE;
        if (holidayItemMap.find(startDate) != holidayItemMap.end()) {
            std::vector<HolidayInfoItem> *vetor = &(holidayItemMap.find(startDate)->second);
            vetor->push_back(items[i]);
        } else {
            std::vector<HolidayInfoItem> vetor;
            vetor.push_back(items[i]);
            holidayItemMap.insert(std::pair<std::string, std::vector<HolidayInfoItem>>(startDate, vetor));
        }
    }
}

HolidayManager::~HolidayManager()
{
}

void HolidayManager::SetHolidayData(std::map<std::string, std::vector<HolidayInfoItem>> holidayDataMap)
{
    holidayItemMap = holidayDataMap;
}

bool HolidayManager::IsHoliday()
{
    time_t timeStamp = time(NULL);
    struct tm* timObj = localtime(&timeStamp);
    if (timObj == nullptr) {
        return false;
    }
    int32_t year = timObj->tm_year + YEAR_START;
    int32_t month = timObj->tm_mon + MONTH_GREATER_ONE;
    return IsHoliday(year, month, timObj->tm_mday);
}

bool HolidayManager::IsHoliday(int32_t year, int32_t month, int32_t day)
{
    std::string startDate = Format(year, month, day);
    if (holidayItemMap.find(startDate) != holidayItemMap.end()) {
        std::vector<HolidayInfoItem> list = holidayItemMap.find(startDate)->second;
        return list.size() > 0;
    }
    return false;
}

std::string HolidayManager::Format(int32_t year, int32_t month, int32_t day)
{
    std::string formated;
    formated += std::to_string(year);
    // Numbers less than 10 are one digit
    formated += month < 10 ? ("0" + std::to_string(month)) : std::to_string(month);
    // Numbers less than 10 are one digit
    formated += day < 10 ? ("0" + std::to_string(day)) : std::to_string(day);
    return formated;
}

std::vector<HolidayInfoItem> HolidayManager::GetHolidayInfoItemArray()
{
    time_t timeStamp = time(NULL);
    struct tm* timObj = localtime(&timeStamp);
    if (timObj == nullptr) {
        std::vector<HolidayInfoItem> emptyList;
        return emptyList;
    }
    int32_t realYear = timObj->tm_year + YEAR_START;
    return GetHolidayInfoItemArray(realYear);
}

std::vector<HolidayInfoItem> HolidayManager::GetHolidayInfoItemArray(int32_t year)
{
    std::vector<HolidayInfoItem> vetor;
    std::string formatedYear = std::to_string(year);
    std::map<std::string, std::vector<HolidayInfoItem>>::iterator iter;
    for (iter = holidayItemMap.begin(); iter != holidayItemMap.end(); ++iter) {
        std::string key = iter->first;
        if (formatedYear == key.substr(0, 4)) { // 4 is the length of full year
            std::vector<HolidayInfoItem> temp = iter->second;
            for (size_t i = 0; i < temp.size(); i++) {
                vetor.push_back(temp[i]);
            }
        }
    }
    return vetor;
}

std::vector<HolidayInfoItem> HolidayManager::ReadHolidayFile(const std::string &path)
{
    std::vector<HolidayInfoItem> items;
    std::ifstream fin;
    fin.open(path.c_str(), std::ios::in);
    std::string line;
    while (getline(fin, line)) {
        line = Trim(line);
        if (line.compare(ITEM_BEGIN_TAG) != 0) {
            continue;
        }
        struct HolidayInfoItem holidayItem;
        while (getline(fin, line)) {
            line = Trim(line);
            ParseFileLine(line, &(holidayItem));
            if (line.compare(ITEM_END_TAG) == 0) {
                items.push_back(holidayItem);
                break;
            }
        }
    }
    fin.close();
    return items;
}

void HolidayManager::ParseFileLine(const std::string &line, HolidayInfoItem *holidayItem)
{
    if (holidayItem == nullptr) {
        return;
    }
    std::regex reg("([A-Z]+)[:;](.+)");
    std::smatch match;
    bool found = RegexSearchNoExcept(line, match, reg);
    if (!found) {
        return;
    }
    std::string tag = match[1].str();
    size_t valuePos = line.find_last_of(":");
    if (valuePos == std::string::npos || valuePos + 1 >= line.length()) {
        return;
    }
    std::string value = line.substr(valuePos + 1, line.length());
    if (tag.compare(ITEM_DTSTART_TAG) == 0) {
        std::string startDate = value.size() >= 8 ? value.substr(0, 8) : ""; // 8 is date formarted string length
        if (startDate.size() == 8) {
            struct tm timeObj;
            strptime(startDate.c_str(), "%Y%m%d", &timeObj);
            holidayItem->year = timeObj.tm_year;
            holidayItem->month = timeObj.tm_mon;
            holidayItem->day = timeObj.tm_mday;
        }
    } else if (tag.compare(ITEM_SUMMARY_TAG) == 0) {
        holidayItem->baseName = value;
    } else if (tag.compare(ITEM_RESOURCES_TAG) == 0) {
        size_t displayNamesPos = line.find_last_of('=');
        if (displayNamesPos == std::string::npos || displayNamesPos + 1 >= line.length()) {
            return;
        }
        std::string displayName = line.substr(displayNamesPos + 1, line.length());
        size_t languagePos = displayName.find_first_of(":");
        if (languagePos == std::string::npos || languagePos + 1 >= displayName.length()) {
            return;
        }
        std::string language = displayName.substr(0, languagePos);
        std::string localName = displayName.substr(languagePos + 1, displayName.length());
        transform(language.begin(), language.end(), language.begin(), ::tolower);
        HolidayLocalName localeName = {language, localName};
        holidayItem->localNames.push_back(localeName);
    }
}

std::string& HolidayManager::Trim(std::string &str)
{
    if (str.empty()) {
        return str;
    }
    size_t endPos = str.find_first_not_of(" \t");
    if (endPos != std::string::npos) {
        str.erase(0, endPos);
    }
    size_t startPos = str.find_last_not_of("\r\n\t");
    if (startPos != std::string::npos && startPos + 1 < str.length()) {
        str.erase(startPos + 1);
    }
    return str;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS
