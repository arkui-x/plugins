/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "plugins/device_info/android/java/jni/device_info_impl.h"

#include "log.h"
#include "inner_api/plugin_utils_inner.h"

#include "plugins/device_info/android/java/jni/device_info_jni.h"

namespace OHOS::Plugin {
std::unique_ptr<DeviceInfo> DeviceInfo::Create()
{
    return std::make_unique<DeviceInfoImpl>();
}

const std::string DeviceInfoImpl::GetDeviceInfo(int id, const std::string &defValue)
{
    return DeviceInfoJni::CallMethod(id, defValue);
}

int DeviceInfoImpl::GetDeviceInfo(int id, int defValue)
{
    return DeviceInfoJni::CallIntMethod(id, defValue);
}
}  // namespace OHOS::Plugin
