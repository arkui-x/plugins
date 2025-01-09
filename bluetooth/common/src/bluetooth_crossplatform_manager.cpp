/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "bluetooth_crossplatform_manager.h"

#include "bluetooth_profile_manager.h"
#ifdef ANDROID_PLATFORM
#include "bluetooth_impl_utils.h"
#endif

namespace OHOS {
namespace Bluetooth {

BluetoothCrossplatformManager& BluetoothCrossplatformManager::GetInstance()
{
    static BluetoothCrossplatformManager instance;
    return instance;
}

void BluetoothCrossplatformManager::Init()
{
#if defined(ANDROID_PLATFORM)
    BluetoothCtrlJniRegister::AbilityBluetoothCtrlJniInit();
#elif defined(IOS_PLATFORM)
    BluetoothProfileManager::GetInstance();
#endif
}
} // namespace Bluetooth
} // namespace OHOS
