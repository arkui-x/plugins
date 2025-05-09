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
#ifndef OHOS_GLOBAL_I18N_LOCALE_CONFIG_H
#define OHOS_GLOBAL_I18N_LOCALE_CONFIG_H

#include <string>

#include "plugins/intl/INTL.h"

namespace OHOS {
namespace Global {
namespace I18n {
class LocaleConfig {
public:
    LocaleConfig() = default;
    virtual ~LocaleConfig() = default;
    static std::string GetSystemLocale();
    static std::string GetSystemTimezone();
    static std::string GetSystemCalendar();
    static std::string GetNumberingSystem();
    static bool Is24HourClock();
    static std::string GetSystemLocaleWithExtParam();
private:
    static std::unique_ptr<Plugin::INTL> plugin;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif
