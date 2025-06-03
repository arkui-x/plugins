/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
// cross modify
//
#include "locale_config.h"
#include <map>

namespace OHOS {
namespace Global {
namespace I18n {
using namespace std;
namespace {
const std::map<std::string, std::string> IcuCalendarMap = {
    { "gregorian", "gregory" },
    { "ethiopicAmeteMihret", "ethiopic" },
    { "ethiopicAmeteAlem", "ethioaa" },
    { "islamicCivil", "islamic-civil" },
    { "islamicTabular", "islamic-tbla" },
    { "islamicUmmAlQura", "islamic-umalqura" },
    { "republicOfChina", "roc" },
};
}

unique_ptr<Plugin::INTL> LocaleConfig::plugin = Plugin::INTL::Create();

string LocaleConfig::GetSystemLocale()
{
    if (!plugin) {
        return "";
    }
    return plugin->GetSystemLocale();
}

string LocaleConfig::GetSystemTimezone()
{
    if (!plugin) {
        return "";
    }
    return plugin->GetSystemTimezone();
}

std::string LocaleConfig::GetSystemCalendar()
{
    if (!plugin) {
        return "";
    }
    std::string calendar = plugin->GetSystemCalendar();
    auto iter = IcuCalendarMap.find(calendar);
    if (iter != IcuCalendarMap.end()) {
        calendar = iter->second;
    }
    return calendar;
}

std::string LocaleConfig::GetNumberingSystem()
{
    if (!plugin) {
        return "";
    }
    return plugin->GetNumberingSystem();
}

bool LocaleConfig::Is24HourClock()
{
    if (!plugin) {
        return false;
    }
    return plugin->Is24HourClock();
}

std::string LocaleConfig::GetSystemLocaleWithExtParam()
{
    std::string systemLocale = LocaleConfig::GetSystemLocale();
    bool hasExtParam = false;

    std::string systemNumberingSystem = LocaleConfig::GetNumberingSystem();
    if (!systemNumberingSystem.empty()) {
        systemLocale += "-u-nu-" + systemNumberingSystem;
        hasExtParam = true;
    }
    std::string systemCalendar = LocaleConfig::GetSystemCalendar();
    if (!systemCalendar.empty()) {
        if (!hasExtParam) {
            systemLocale += "-u";
        }
        systemLocale += "-ca-" + systemCalendar;
    }

    return systemLocale;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS
