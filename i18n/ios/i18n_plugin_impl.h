/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef PLUGINS_I18N_PLUGIN_IOS_I18N_PLUGIN_IMPL_H
#define PLUGINS_I18N_PLUGIN_IOS_I18N_PLUGIN_IMPL_H

#include "plugins/i18n/i18n.h"

namespace OHOS::Plugin {

class I18NPluginImpl final : public I18N {
public:
    I18NPluginImpl() = default;
    ~I18NPluginImpl() override = default;

    bool Is24HourClock() override;
};

} // namespace OHOS::Plugin

#endif // PLUGINS_I18N_PLUGIN_IOS_I18N_PLUGIN_IMPL_H