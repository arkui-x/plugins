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

#include "plugins/ui_appearance/android/java/jni/ui_appearance_impl.h"

#include "plugins/ui_appearance/android/java/jni/ui_appearance_jni.h"

namespace OHOS::Plugin {

std::unique_ptr<UiAppearance> UiAppearance::Create()
{
    return std::make_unique<UiAppearanceImpl>();
}

int32_t UiAppearanceImpl::GetDarkMode()
{
    return UiAppearanceJni::GetDarkMode();
}

double UiAppearanceImpl::GetFontScale()
{
    return UiAppearanceJni::GetFontScale();
}

} // namespace OHOS::Plugin
