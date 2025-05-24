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

#include "plugins/i18n/I18N.h"

namespace OHOS {
namespace Global {
namespace I18n {
class LocaleConfig {
public:
    LocaleConfig() = default;
    virtual ~LocaleConfig() = default;
    static std::string GetSystemLocale();
    static std::string GetSystemLanguage();
    static std::string GetSystemRegion();
    static std::string GetSystemTimezone();
    static bool Is24HourClock();
    static bool IsRTL(const std::string &locale);
    static std::string GetDisplayLanguage(const std::string &language, const std::string &displayLocale,
        bool sentenceCase);
    static std::string GetDisplayRegion(const std::string &region, const std::string &displayLocale,
        bool sentenceCase);
    static bool IsValidRegion(const std::string &region);
    static bool IsValidLanguage(const std::string &language);
    static bool IsValidTag(const std::string &tag);
    static std::string GetAppPreferredLanguage();
    static void SetAppPreferredLanguage(const std::string &language);

private:
    static constexpr uint32_t LANGUAGE_LEN = 2;
    static std::unique_ptr<Plugin::I18N> plugin;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif
