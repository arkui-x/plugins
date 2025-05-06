/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#ifndef PLUGINS_INTL_PLUGIN_IOS_INTL_PLUGIN_IMPL_H
#define PLUGINS_INTL_PLUGIN_IOS_INTL_PLUGIN_IMPL_H

#include "plugins/intl/INTL.h"

namespace OHOS::Plugin {
class INTLPluginImpl final : public INTL {
public:
    INTLPluginImpl() = default;
    ~INTLPluginImpl() override = default;

    bool Is24HourClock() override;
    std::string GetSystemLocale() override;
    std::string GetSystemTimezone() override;
    std::string GetSystemCalendar() override;
    std::string GetNumberingSystem() override;
    std::string GetDeviceType() override;
};
} // namespace OHOS::Plugin
#endif // PLUGINS_INTL_PLUGIN_IOS_INTL_PLUGIN_IMPL_H