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

#import "BluetoothPeripheralManager.h"
#include "bluetooth_log.h"

using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {
BluetoothBleAdvertiserImpl::BluetoothBleAdvertiserImpl()
{
    [BluetoothPeripheralManager sharedInstance];
}

BluetoothBleAdvertiserImpl::~BluetoothBleAdvertiserImpl() {}

void BluetoothBleAdvertiserImpl::RegisterBleAdvertiserCallback(const sptr<IBluetoothBleAdvertiseCallback>& callback)
{
    std::lock_guard<std::mutex> lock(advertiserMutex_);
    advertiseCallBack_ = callback;
}

void BluetoothBleAdvertiserImpl::DeregisterBleAdvertiserCallback(const sptr<IBluetoothBleAdvertiseCallback>& callback)
{
    std::lock_guard<std::mutex> lock(advertiserMutex_);
    advertiseCallBack_ = nullptr;
}

NSMutableArray* GetServiceUUIDs(std::vector<Uuid> scanResponseUUIDS)
{
    NSMutableArray* arrServiceUuids = [NSMutableArray array];
    for (Uuid& uuid : scanResponseUUIDS) {
        NSString* strUuid = [NSString stringWithFormat:@"%s", uuid.ToString().c_str()];
        CBUUID* cbUuid = [CBUUID UUIDWithString:strUuid];
        [arrServiceUuids addObject:cbUuid];
    }
    return arrServiceUuids;
}

NSMutableDictionary* GetServiceData(std::map<Uuid, std::string> serviceData)
{
    NSMutableDictionary* serviceDataDic = [NSMutableDictionary dictionary];
    for (auto& mapData : serviceData) {
        const Uuid& uuid = mapData.first;
        std::string strValue = mapData.second;
        NSData* data = [NSData data];
        if (strValue.length()) {
            data = [NSData dataWithBytes:strValue.c_str() length:strValue.length()];
        }
        NSString* strUuid = [NSString stringWithFormat:@"%s", uuid.ToString().c_str()];
        [serviceDataDic setObject:data forKey:strUuid];
    }
    return serviceDataDic;
}

int BluetoothBleAdvertiserImpl::StartAdvertising(const BluetoothBleAdvertiserSettings& settings,
    const BluetoothBleAdvertiserData& advData, const BluetoothBleAdvertiserData& scanResponse, int32_t advHandle,
    uint16_t duration, bool isRawData)
{
    NSMutableArray* arrServiceUuids = GetServiceUUIDs(advData.GetServiceUuids());
    NSMutableDictionary* serviceDataDic = GetServiceData(advData.GetServiceData());

    bool includeDeviceName = advData.GetIncludeDeviceName();
    bool isConnectable = settings.IsConnectable();

    NSString* strAppId = [NSString stringWithFormat:@"%d", advHandle];
    BluetoothPeripheralManager* peripheralManager = [BluetoothPeripheralManager sharedInstance];
    int ret = [peripheralManager startAdvertising:strAppId
                                    isConnectable:isConnectable
                                     servicesUUID:arrServiceUuids
                                      serviceData:serviceDataDic
                                includeDeviceName:includeDeviceName
                                         duration:duration];

    peripheralManager.startAdvertisingBlock = ^(bool isAdvertising, int state) {
      if (advertiseCallBack_) {
          int bleCode = isAdvertising ? BLE_ADV_DEFAULT_OP_CODE : BLE_ADV_START_FAILED_OP_CODE;
          advertiseCallBack_->OnStartResultEvent(state, advHandle, bleCode);
      }
    };
    return ret;
}

int BluetoothBleAdvertiserImpl::EnableAdvertising(uint8_t advHandle, uint16_t duration)
{
    return BT_NO_ERROR;
}

int BluetoothBleAdvertiserImpl::DisableAdvertising(uint8_t advHandle)
{
    return BT_NO_ERROR;
}

int BluetoothBleAdvertiserImpl::ChangeAdvertisingParams(
    uint8_t advHandle, const BluetoothBleAdvertiserSettings& settings)
{
    return BT_NO_ERROR;
}

int BluetoothBleAdvertiserImpl::StopAdvertising(int32_t advHandle)
{
    NSString* strAppId = [NSString stringWithFormat:@"%d", advHandle];
    BluetoothPeripheralManager* peripheralManager = [BluetoothPeripheralManager sharedInstance];
    int ret = [peripheralManager stopAdvertising:strAppId];
    if (advertiseCallBack_) {
        advertiseCallBack_->OnStopResultEvent(BT_NO_ERROR, advHandle);
    }
    return ret;
}

void BluetoothBleAdvertiserImpl::Close(int32_t advHandle) {}

int32_t BluetoothBleAdvertiserImpl::GetAdvertiserHandle(int32_t& advHandle,
    const sptr<IBluetoothBleAdvertiseCallback> &callback)
{
    advHandle = [[BluetoothPeripheralManager sharedInstance] getAppId];
    return BT_NO_ERROR;
}

void BluetoothBleAdvertiserImpl::SetAdvertisingData(const BluetoothBleAdvertiserData& advData,
    const BluetoothBleAdvertiserData& scanResponse, int32_t advHandle) {}
} // namespace Bluetooth
} // namespace OHOS