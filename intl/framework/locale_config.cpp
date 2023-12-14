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
// cross modify
//
#include "locale_config.h"

namespace OHOS {
namespace Global {
namespace I18n {
using namespace std;

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

bool LocaleConfig::Is24HourClock()
{
    if (!plugin) {
        return false;
    }
    return plugin->Is24HourClock();
}
} // namespace I18n
} // namespace Global
} // namespace OHOS
