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

#include "plugins/i18n/android/java/jni/i18n_plugin_impl.h"
#include "plugins/i18n/android/java/jni/i18n_plugin_jni.h"

namespace OHOS::Plugin {
std::unique_ptr<I18N> I18N::Create()
{
    return std::make_unique<I18NPluginImpl>();
}

bool I18NPluginImpl::Is24HourClock()
{
    return I18NPluginJni::Is24HourClock();
}
} // namespace OHOS::Plugin
