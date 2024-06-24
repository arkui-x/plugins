/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#ifndef OHOS_GLOBAL_I18N_DATE_TIME_FORMAT_H
#define OHOS_GLOBAL_I18N_DATE_TIME_FORMAT_H

#include <map>
#include <vector>
#include <climits>
#include <set>
#include <unordered_map>
#include "locale_info.h"
#include "unicode/datefmt.h"
#include "unicode/dtptngen.h"
#include "unicode/localebuilder.h"
#include "unicode/locid.h"
#include "unicode/smpdtfmt.h"
#include "unicode/timezone.h"
#include "unicode/calendar.h"
#include "unicode/numsys.h"
#include "unicode/dtitvfmt.h"

namespace OHOS {
namespace Global {
namespace I18n {
enum class DefaultStyle {
    LONG,
    DEFAULT,
    SHORT,
};
class DateTimeFormat {
public:
    DateTimeFormat(const std::vector<std::string> &localeTags, std::map<std::string, std::string> &configs);
    virtual ~DateTimeFormat();
    std::string Format(int64_t milliseconds);
    std::string FormatRange(int64_t fromMilliseconds, int64_t toMilliseconds);
    void GetResolvedOptions(std::map<std::string, std::string> &map);
    std::string GetDateStyle() const;
    std::string GetTimeStyle() const;
    std::string GetHourCycle() const;
    std::string GetTimeZone() const;
    std::string GetTimeZoneName() const;
    std::string GetNumberingSystem() const;
    std::string GetHour12() const;
    std::string GetWeekday() const;
    std::string GetEra() const;
    std::string GetYear() const;
    std::string GetMonth() const;
    std::string GetDay() const;
    std::string GetHour() const;
    std::string GetMinute() const;
    std::string GetSecond() const;
    static std::unique_ptr<DateTimeFormat> CreateInstance(const std::vector<std::string> &localeTags,
                                                          std::map<std::string, std::string> &configs);

private:
    std::string localeTag;
    std::string dateStyle;
    std::string timeStyle;
    std::string hourCycle;
    std::string timeZone;
    std::string numberingSystem;
    std::string hour12;
    std::string weekday;
    std::string era;
    std::string year;
    std::string month;
    std::string day;
    std::string hour;
    std::string minute;
    std::string second;
    std::string timeZoneName;
    std::string dayPeriod;
    std::string localeMatcher;
    std::string formatMatcher;
    icu::DateFormat *dateFormat = nullptr;
    icu::DateIntervalFormat *dateIntvFormat = nullptr;
    icu::Calendar *calendar = nullptr;
    LocaleInfo *localeInfo = nullptr;
    icu::Locale locale;
    icu::UnicodeString pattern;
    char16_t yearChar = 'Y';
    char16_t monthChar = 'M';
    char16_t dayChar = 'd';
    char16_t hourChar = 'h';
    char16_t minuteChar = 'm';
    char16_t secondChar = 's';
    char16_t timeZoneChar = 'z';
    char16_t weekdayChar = 'E';
    char16_t eraChar = 'G';
    char16_t amPmChar = 'a';
    std::string hourTwoDigitString = "HH";
    std::string hourNumericString = "H";
    bool createSuccess = false;
    DefaultStyle defaultStyle;
    static const int32_t NUMERIC_LENGTH = 1;
    static const int32_t TWO_DIGIT_LENGTH = 2;
    static const int32_t SHORT_LENGTH = 3;
    static const int32_t LONG_LENGTH = 4;
    static const int32_t NARROW_LENGTH = 5;
    static const size_t YEAR_INDEX = 0;
    static const size_t MONTH_INDEX = 1;
    static const size_t DAY_INDEX = 2;
    static const size_t HOUR_INDEX = 3;
    static const size_t MINUTE_INDEX = 4;
    static const size_t SECOND_INDEX = 5;
    static const int32_t SHORT_ERA_LENGTH = 1;
    static const int32_t LONG_ERA_LENGTH = 4;
    static const int HALF_HOUR = 30;
    static const int HOURS_OF_A_DAY = 24;
    static bool icuInitialized;
    static const char *TIMEZONE_KEY;
    static const char *DEFAULT_TIMEZONE;
    static constexpr int SYS_PARAM_LEN = 128;
    static bool Init();
    static std::map<std::string, icu::DateFormat::EStyle> dateTimeStyle;
    static std::unordered_map<std::string, DefaultStyle> DeviceToStyle;
    static std::unordered_map<DefaultStyle, icu::DateFormat::EStyle> DefaultDTStyle;
    bool InitWithLocale(const std::string &curLocale, std::map<std::string, std::string> &configs);
    bool InitWithDefaultLocale(std::map<std::string, std::string> &configs);
    void ParseConfigsPartOne(std::map<std::string, std::string> &configs);
    void ParseConfigsPartTwo(std::map<std::string, std::string> &configs);
    void AddOptions(std::string option, char16_t optionChar);
    void ComputeSkeleton();
    void ComputePattern();
    void ComputePartOfPattern(std::string option, char16_t character, std::string twoDigitChar,
        std::string numericChar);
    void ComputeHourCycleChars();
    void ComputeTimeZoneOfPattern(
        std::string option, char16_t character, std::string longChar, std::string shortChar);
    void ComputeWeekdayOfPattern(
        std::string option, char16_t character, std::string longChar, std::string shortChar, std::string narrowChar);
    void ComputeEraOfPattern(
        std::string option, char16_t character, std::string longChar, std::string shortChar, std::string narrowChar);
    void InitDateFormatWithoutConfigs(UErrorCode &status);
    void InitDateFormat(UErrorCode &status);
    void GetAdditionalResolvedOptions(std::map<std::string, std::string> &map);
    void FixPatternPartOne();
    void FixPatternPartTwo();
    void RemoveAmPmChar();
    int64_t GetArrayValue(int64_t *dateArray, size_t index, size_t size);
    bool CheckInitSuccess();
    void FreeDateTimeFormat();
    void SetDayPeriod();
    std::string GetSystemTimezone();
    DefaultStyle GetDefaultStyle();
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif