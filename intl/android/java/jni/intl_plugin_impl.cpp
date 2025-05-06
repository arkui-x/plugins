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

#include "plugins/intl/android/java/jni/intl_plugin_impl.h"
#include "plugins/intl/android/java/jni/intl_plugin_jni.h"

#include <string>

namespace OHOS::Plugin {
std::unique_ptr<INTL> INTL::Create()
{
    return std::make_unique<INTLPluginImpl>();
}

bool INTLPluginImpl::Is24HourClock()
{
    return INTLPluginJni::Is24HourClock();
}

std::string INTLPluginImpl::GetSystemLocale()
{
    return INTLPluginJni::GetSystemLocale();
}

std::string INTLPluginImpl::GetSystemTimezone()
{
    return INTLPluginJni::GetSystemTimezone();
}

std::string INTLPluginImpl::GetSystemCalendar()
{
    return INTLPluginJni::GetSystemCalendar();
}

std::string INTLPluginImpl::GetNumberingSystem()
{
    return INTLPluginJni::GetNumberingSystem();
}

std::string INTLPluginImpl::GetDeviceType()
{
    return INTLPluginJni::GetDeviceType();
}
} // namespace OHOS::Plugin
