/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#ifndef OHOS_GLOBAL_I18N_CALENDAR_H
#define OHOS_GLOBAL_I18N_CALENDAR_H

#include <cmath>
#include <cstdint>

#include "iosfwd"
#include "locid.h"
#include "unicode/calendar.h"
#include "unicode/ucal.h"
#include "utypes.h"

namespace OHOS {
namespace Global {
namespace I18n {
enum CalendarType {
    BUDDHIST,
    CHINESE,
    COPTIC,
    ETHIOPIC,
    HEBREW,
    GREGORY,
    INDIAN,
    ISLAMIC_CIVIL,
    ISLAMIC_TBLA,
    ISLAMIC_UMALQURA,
    JAPANESE,
    PERSIAN,
    UNDEFINED
};

class I18nCalendar {
public:
    explicit I18nCalendar(std::string localeTag);
    I18nCalendar(std::string localeTag, CalendarType type);
    virtual ~I18nCalendar();
    void SetTime(double value);
    void Set(int32_t year, int32_t month, int32_t date);
    void Set(UCalendarDateFields field, int32_t value);
    std::string GetTimeZone(void);
    void SetTimeZone(std::string timezoneId);
    int32_t Get(UCalendarDateFields field) const;
    void Add(UCalendarDateFields field, int32_t amount);
    void SetMinimalDaysInFirstWeek(int32_t value);
    void SetFirstDayOfWeek(int32_t value);
    UDate GetTimeInMillis(void);
    int32_t GetMinimalDaysInFirstWeek(void);
    int32_t GetFirstDayOfWeek(void);
    bool IsWeekend(int64_t date, UErrorCode &status);
    bool IsWeekend(void);
    std::string GetDisplayName(std::string &displayLocaleTag);
    int32_t CompareDays(UDate date);

private:
    icu::Calendar *calendar_;
    void InitCalendar(const icu::Locale &locale, CalendarType type);
    void InitCalendar2(const icu::Locale &locale, CalendarType type, UErrorCode &status);
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif