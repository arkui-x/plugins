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
#ifndef OHOS_GLOBAL_I18N_RELATIVE_TIME_FORMAT_H
#define OHOS_GLOBAL_I18N_RELATIVE_TIME_FORMAT_H

#include <map>
#include <set>
#include <unordered_map>
#include <vector>
#include "unicode/numberformatter.h"
#include "unicode/locid.h"
#include "unicode/numfmt.h"
#include "unicode/unum.h"
#include "unicode/decimfmt.h"
#include "unicode/localebuilder.h"
#include "unicode/numsys.h"
#include "unicode/measfmt.h"
#include "unicode/measunit.h"
#include "unicode/measure.h"
#include "unicode/currunit.h"
#include "unicode/fmtable.h"
#include "unicode/reldatefmt.h"
#include "unicode/uformattedvalue.h"
#include "unicode/ures.h"
#include "unicode/unum.h"
#include "unicode/ulocdata.h"
#include "unicode/ureldatefmt.h"
#include "number_utils.h"
#include "number_utypes.h"
#include "locale_info.h"
#include "measure_data.h"

namespace OHOS {
namespace Global {
namespace I18n {
class RelativeTimeFormat {
public:
    RelativeTimeFormat(const std::vector<std::string> &localeTag, std::map<std::string, std::string> &configs);
    virtual ~RelativeTimeFormat();
    std::string Format(double number, const std::string &unit);
    void FormatToParts(double number, const std::string &unit, std::vector<std::vector<std::string>> &timeVector);
    void GetResolvedOptions(std::map<std::string, std::string> &map);

private:
    icu::Locale locale;
    std::string localeBaseName;
    std::string styleString = "long";
    std::string numeric = "always";
    std::string numberingSystem;
    std::unique_ptr<LocaleInfo> localeInfo;
    std::unique_ptr<icu::RelativeDateTimeFormatter> relativeTimeFormat;
    UDateRelativeDateTimeFormatterStyle style = UDAT_STYLE_LONG;
    bool createSuccess = false;
    static std::unordered_map<std::string, URelativeDateTimeUnit> relativeUnits;
    static std::unordered_map<std::string, UDateRelativeDateTimeFormatterStyle> relativeFormatStyle;
    static std::unordered_map<std::string, std::string> defaultFormatStyle;
    void InsertInfo(std::vector<std::vector<std::string>> &timeVector, const std::string &unit, bool isInteger,
        const std::string &value);
    void ProcessIntegerField(const std::map<size_t, size_t> &indexMap,
        std::vector<std::vector<std::string>> &timeVector, size_t &startIndex, const std::string &unit,
        const std::string &result);
    void ParseConfigs(std::map<std::string, std::string> &configs);
    void SetDefaultStyle();
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif