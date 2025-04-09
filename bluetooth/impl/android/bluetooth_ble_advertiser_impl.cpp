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

#include "bluetooth_ble_advertiser_impl.h"

#include "bluetooth_impl_utils.h"
#include "bluetooth_jni.h"
#include "bluetooth_log.h"

using namespace OHOS::bluetooth;
namespace OHOS {
namespace Bluetooth {
const int MAX_ADV_HANDLE_NUM = 255;

BluetoothBleAdvertiserImpl::BluetoothBleAdvertiserImpl() {}

BluetoothBleAdvertiserImpl::~BluetoothBleAdvertiserImpl() {}

void BluetoothBleAdvertiserImpl::RegisterBleAdvertiserCallback(const sptr<IBluetoothBleAdvertiseCallback>& callback)
{
    std::lock_guard<std::mutex> lock(advertiserMutex_);
    callbackImpl_ = callback;
}

void BluetoothBleAdvertiserImpl::DeregisterBleAdvertiserCallback(const sptr<IBluetoothBleAdvertiseCallback>& callback)
{
    std::lock_guard<std::mutex> lock(advertiserMutex_);
    callbackImpl_ = nullptr;
}

int BluetoothBleAdvertiserImpl::StartAdvertising(const BluetoothBleAdvertiserSettings& settings,
    const BluetoothBleAdvertiserData& advData, const BluetoothBleAdvertiserData& scanResponse, int32_t advHandle,
    uint16_t duration, bool isRawData)
{
    int ret = BluetoothJni::StartAdvertising(settings, advData, scanResponse, advHandle, duration, isRawData);
    std::lock_guard<std::mutex> lock(advertiserMutex_);
    if (ret != BT_NO_ERROR && callbackImpl_ != nullptr) {
        bleAdvertiseHandleList_.erase(remove(bleAdvertiseHandleList_.begin(), bleAdvertiseHandleList_.end(), advHandle),
            bleAdvertiseHandleList_.end());
        callbackImpl_->OnStartResultEvent(
            BluetoothBleAdvertiserState::INTERNAL, advHandle, bluetooth::BLE_ADV_START_FAILED_OP_CODE);
    }
    return ret;
}

int BluetoothBleAdvertiserImpl::StopAdvertising(int32_t advHandle)
{
    int ret = BluetoothJni::StopAdvertising(advHandle);
    std::lock_guard<std::mutex> lock(advertiserMutex_);
    if (ret == BT_NO_ERROR && callbackImpl_ != nullptr) {
        bleAdvertiseHandleList_.erase(remove(bleAdvertiseHandleList_.begin(), bleAdvertiseHandleList_.end(), advHandle),
            bleAdvertiseHandleList_.end());
        callbackImpl_->OnStopResultEvent(BT_NO_ERROR, advHandle);
    }
    return ret;
}

int32_t BluetoothBleAdvertiserImpl::GetAdvertiserHandle(int32_t& advHandle,
    const sptr<IBluetoothBleAdvertiseCallback> &callback)
{
    std::lock_guard<std::mutex> lock(advertiserMutex_);
    for (uint8_t handle = 0; handle < MAX_ADV_HANDLE_NUM; ++handle) {
        std::vector<uint8_t>::iterator ret =
            std::find(bleAdvertiseHandleList_.begin(), bleAdvertiseHandleList_.end(), handle);
        if (ret == bleAdvertiseHandleList_.end()) {
            advHandle = handle;
            bleAdvertiseHandleList_.push_back(advHandle);
            return BT_NO_ERROR;
        }
    }
    advHandle = MAX_ADV_HANDLE_NUM;
    return BT_ERR_INTERNAL_ERROR;
}

void BluetoothBleAdvertiserImpl::OnStartResultEvent(int32_t result, int32_t advHandle, int32_t opCode)
{
    std::lock_guard<std::mutex> lock(advertiserMutex_);
    if (callbackImpl_ == nullptr) {
        HILOGE("callbackImpl is null");
        return;
    }
    callbackImpl_->OnStartResultEvent(result, advHandle, opCode);
}

void BluetoothBleAdvertiserImpl::OnStopResultEvent(int32_t result, int32_t advHandle)
{
    std::lock_guard<std::mutex> lock(advertiserMutex_);
    if (callbackImpl_ == nullptr) {
        HILOGE("callbackImpl is null");
        return;
    }
    callbackImpl_->OnStopResultEvent(result, advHandle);
}

void BluetoothBleAdvertiserImpl::Close(int32_t advHandle)
{
    HILOGE("Not Supported");
}

void BluetoothBleAdvertiserImpl::SetAdvertisingData(
    const BluetoothBleAdvertiserData& advData, const BluetoothBleAdvertiserData& scanResponse, int32_t advHandle)
{
    HILOGE("Not Supported");
}

int BluetoothBleAdvertiserImpl::EnableAdvertising(uint8_t advHandle, uint16_t duration)
{
    HILOGE("Not Supported");
    return BT_NO_ERROR;
}

int BluetoothBleAdvertiserImpl::DisableAdvertising(uint8_t advHandle)
{
    HILOGE("Not Supported");
    return BT_NO_ERROR;
}

int BluetoothBleAdvertiserImpl::ChangeAdvertisingParams(
    uint8_t advHandle, const BluetoothBleAdvertiserSettings& settings)
{
    HILOGE("Not Supported");
    return BT_NO_ERROR;
}
} // namespace Bluetooth
} // namespace OHOS
