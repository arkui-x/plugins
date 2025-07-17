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

#include "bluetooth_ble_centralmanager_impl.h"

#import "BluetoothCentralManager.h"
#include "bluetooth_log.h"

using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {
BluetoothBleCentralManagerImpl::BluetoothBleCentralManagerImpl()
{
    [BluetoothCentralManager sharedInstance];
    scannerId_ = 0;
}

BluetoothBleCentralManagerImpl::~BluetoothBleCentralManagerImpl() {}

void BluetoothBleCentralManagerImpl::RegisterBleCentralManagerCallback(
    int32_t& scannerId, bool enableRandomAddrMode, const sptr<IBluetoothBleCentralManagerCallback>& callback)
{
    if (callback == nullptr) {
        HILOGE("RegisterBleCentralManagerCallback callback is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(scanMutex_);
    scannerId = getCallBackScannerId();
    profileIdScanMap_.insert({ scannerId, callback });
}

void BluetoothBleCentralManagerImpl::DeregisterBleCentralManagerCallback(
    int32_t scannerId, const sptr<IBluetoothBleCentralManagerCallback>& callback)
{
    std::lock_guard<std::mutex> lock(scanMutex_);
    auto mapCallback = profileIdScanMap_.find(scannerId);
    if (mapCallback != profileIdScanMap_.end()) {
        profileIdScanMap_.erase(mapCallback);
    }
}

std::string changeTypeWithData(NSDictionary* dic)
{
    NSMutableString* resultString = [NSMutableString stringWithFormat:@"{"];
    BOOL isFirstEntry = YES;
    for (NSString* key in dic.allKeys) {
        if (!isFirstEntry) {
            [resultString appendString:@","];
        } else {
            isFirstEntry = NO;
        }

        [resultString appendFormat:@"%@:%@", key, dic[key]];
    }
    [resultString appendString:@"}"];
    return resultString.UTF8String;
}

void onSetScanResults(BluetoothBleScanResult& bleScanResult_, CBPeripheral* peripheral,
    NSDictionary<NSString*, id>* advertisementData, NSNumber* RSSI)
{
    if (peripheral.name.length) {
        bleScanResult_.SetName(peripheral.name.UTF8String);
    }
    if (RSSI.integerValue >= INT8_MIN && RSSI.integerValue <= INT8_MAX) {
        bleScanResult_.SetRssi((int8_t)RSSI.integerValue);
    }
    if (advertisementData) {
        bleScanResult_.SetPayload(changeTypeWithData(advertisementData));
        bleScanResult_.SetConnectable([[advertisementData objectForKey:CBAdvertisementDataIsConnectable] boolValue]);
    }
    bleScanResult_.SetPeripheralDevice(OHOS::bluetooth::RawAddress(peripheral.identifier.UUIDString.UTF8String));
}

bool isConditionFilters(NSString* string, NSString* strFilter)
{
    if ([string containsString:strFilter] || [strFilter containsString:@"null"] || strFilter.length == 0) {
        return true;
    }
    return false;
}

int BluetoothBleCentralManagerImpl::StartScan(int32_t scannerId, const BluetoothBleScanSettings& settings,
    const std::vector<BluetoothBleScanFilter>& filters, bool isNewApi)
{
    NSMutableArray<CBUUID*>* arrUuid = [NSMutableArray array];
    NSMutableArray* arrFilter = [NSMutableArray array];
    for (BluetoothBleScanFilter item : filters) {
        NSString* strName = [NSString stringWithFormat:@"%s", item.GetName().c_str()];
        NSString* strDeviceId = [NSString stringWithFormat:@"%s", item.GetDeviceId().c_str()];
        NSString* strSSU = [NSString stringWithFormat:@"%s", item.GetServiceSolicitationUuid().ToString().c_str()];
        strSSU = item.GetServiceSolicitationUuid().ToString() == BLUETOOTH_UUID_BASE_UUID ? @"" : strSSU;
        NSDictionary* dic = @ { @"name" : strName, @"deviceId" : strDeviceId, @"ssu" : strSSU };
        [arrFilter addObject:dic];
        std::string serviceUUID = item.GetServiceUuid().ToString();
        if (serviceUUID == BLUETOOTH_UUID_BASE_UUID || serviceUUID.empty()) {
            continue;
        }
        NSString* strUuid = [NSString stringWithFormat:@"%s", serviceUUID.c_str()];
        [arrUuid addObject:[CBUUID UUIDWithString:strUuid]];
    }
    BluetoothCentralManager* centralManager = [BluetoothCentralManager sharedInstance];
    centralManager.scanDataBlock =
        ^(CBPeripheral* peripheral, NSDictionary<NSString*, id>* advertisementData, NSNumber* RSSI) {
          sptr<IBluetoothBleCentralManagerCallback>& callback = profileIdScanMap_[scannerId];
          if (callback == nullptr) {
              return;
          }
          BluetoothBleScanResult bleSR;
          onSetScanResults(bleSR, peripheral, advertisementData, RSSI);
          if (arrFilter.count == 0) {
              callback->OnScanCallback(bleSR, OHOS::bluetooth::BLE_SCAN_CALLBACK_TYPE_ALL_MATCH);
              return;
          }
          NSString* strAdvDataDeviceUuid = [NSString stringWithFormat:@"%@", peripheral.identifier.UUIDString];
          NSString* strAdvDataName =
              [NSString stringWithFormat:@"%@", [advertisementData objectForKey:CBAdvertisementDataLocalNameKey]];
          NSString* strAdvDataSD =
              [NSString stringWithFormat:@"%@", [advertisementData objectForKey:CBAdvertisementDataServiceDataKey]];
          NSString* strAdvDataSSU = [NSString
              stringWithFormat:@"%@", [advertisementData objectForKey:CBAdvertisementDataSolicitedServiceUUIDsKey]];
          for (NSDictionary* dic in arrFilter) {
              if (isConditionFilters(peripheral.name, dic[@"name"]) &&
                  isConditionFilters(strAdvDataDeviceUuid, dic[@"deviceId"]) &&
                  isConditionFilters(strAdvDataSSU, dic[@"ssu"])) {
                  callback->OnScanCallback(bleSR, OHOS::bluetooth::BLE_SCAN_CALLBACK_TYPE_ALL_MATCH);
                  break;
              }
          }
        };
    return [centralManager startBLEScanWithId:arrUuid];
}

int BluetoothBleCentralManagerImpl::StopScan(int32_t scannerId)
{
    [[BluetoothCentralManager sharedInstance] stopBLEScan];
    return BT_NO_ERROR;
}

void BluetoothBleCentralManagerImpl::RemoveScanFilter(int32_t scannerId) {}

bool BluetoothBleCentralManagerImpl::FreezeByRss(std::set<int> pidSet, bool isProxy)
{
    return BT_NO_ERROR;
}

bool BluetoothBleCentralManagerImpl::ResetAllProxy()
{
    return BT_NO_ERROR;
}

int BluetoothBleCentralManagerImpl::SetLpDeviceAdvParam(
    int duration, int maxExtAdvEvents, int window, int interval, int advHandle)
{
    return BT_NO_ERROR;
}

int BluetoothBleCentralManagerImpl::SetScanReportChannelToLpDevice(int32_t scannerId, bool enable)
{
    return BT_NO_ERROR;
}

int BluetoothBleCentralManagerImpl::EnableSyncDataToLpDevice()
{
    return BT_NO_ERROR;
}

int BluetoothBleCentralManagerImpl::DisableSyncDataToLpDevice()
{
    return BT_NO_ERROR;
}

int BluetoothBleCentralManagerImpl::SendParamsToLpDevice(const std::vector<uint8_t>& dataValue, int32_t type)
{
    return BT_NO_ERROR;
}

bool BluetoothBleCentralManagerImpl::IsLpDeviceAvailable()
{
    return BT_NO_ERROR;
}

int BluetoothBleCentralManagerImpl::SetLpDeviceParam(const BluetoothLpDeviceParamSet& paramSet)
{
    return BT_NO_ERROR;
}

int BluetoothBleCentralManagerImpl::RemoveLpDeviceParam(const bluetooth::Uuid& uuid)
{
    return BT_NO_ERROR;
}

int BluetoothBleCentralManagerImpl::ChangeScanParams(int32_t scannerId, const BluetoothBleScanSettings& settings,
    const std::vector<BluetoothBleScanFilter>& filters, uint32_t filterAction)
{
    return BT_NO_ERROR;
}

int BluetoothBleCentralManagerImpl::IsValidScannerId(int32_t scannerId, bool &isValid)
{
    return BT_NO_ERROR;
}

int BluetoothBleCentralManagerImpl::getCallBackScannerId()
{
    scannerId_ += 1;
    return scannerId_;
}
} // namespace Bluetooth
} // namespace OHOS
