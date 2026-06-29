/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PLUGINS_UI_APPEARANCE_IOS_UI_APPEARANCE_IMPL_H
#define PLUGINS_UI_APPEARANCE_IOS_UI_APPEARANCE_IMPL_H

#include "plugins/ui_appearance/ui_appearance.h"

namespace OHOS::Plugin {

class UiAppearanceImpl final : public UiAppearance {
public:
    UiAppearanceImpl() = default;
    ~UiAppearanceImpl() override = default;

    int32_t GetDarkMode() override;
    double GetFontScale() override;
};

} // namespace OHOS::Plugin

#endif // PLUGINS_UI_APPEARANCE_IOS_UI_APPEARANCE_IMPL_H
