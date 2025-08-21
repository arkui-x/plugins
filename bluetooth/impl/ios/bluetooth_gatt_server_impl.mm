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

#include "bluetooth_gatt_server_impl.h"

#import "BluetoothPeripheralManager.h"
#import "BluetoothUntils.h"
#include "bluetooth_gatt_characteristic.h"
#include "bluetooth_log.h"
#include "securec.h"

using Propertie = OHOS::Bluetooth::GattCharacteristic::Propertie;
using namespace OHOS::bluetooth;

const uint16_t MIN_SERVICE_HANDLE = 1;
const uint16_t MAX_SERVICE_HANDLE = 65535;
const uint16_t CHARACTERISTIC_READ = 0;
const uint16_t CHARACTERISTIC_WRITE = 1;

namespace OHOS {
namespace Bluetooth {
BluetoothGattServerImpl::BluetoothGattServerImpl()
{
    appId_ = 0;
}

bool BluetoothGattServerImpl::CalculateAndAssignHandle(const int applicationId, BluetoothGattService& service)
{
    uint16_t count = 1;
    std::pair<uint16_t, uint16_t> handlePair(0, 0);
    count += service.includeServices_.size();
    count += service.characteristics_.size() * 0x2;
    for (auto& ccc : service.characteristics_) {
        count += ccc.descriptors_.size();
    }
    for (auto item = gattServiceHandleMap_[applicationId].begin(); item != gattServiceHandleMap_[applicationId].end();
         item++) {
        auto availableLength = item->second - item->first + 1;
        if (availableLength >= count) {
            handlePair = std::pair<uint16_t, uint16_t>(item->first, item->first + count - 1);
            if (handlePair.second < item->second) {
                gattServiceHandleMap_[applicationId].insert(
                    item, std::pair<uint16_t, uint16_t>(handlePair.second + 1, item->second));
            }
            gattServiceHandleMap_[applicationId].erase(item);
            break;
        }
    }
    service.handle_ = handlePair.first;
    service.endHandle_ = handlePair.second;
    if (handlePair == std::pair<uint16_t, uint16_t>(0, 0)) {
        return false;
    }
    uint32_t currentHandle = handlePair.first + 1;
    for (auto& includeService : service.includeServices_) {
        includeService.handle_ = currentHandle++;
    }
    for (auto& charactor : service.characteristics_) {
        charactor.handle_ = currentHandle++;
        charactor.valueHandle_ = currentHandle++;
        for (auto& descriptor : charactor.descriptors_) {
            descriptor.handle_ = currentHandle++;
        }
        charactor.endHandle_ = currentHandle - 1;
    }
    gattServiceMap_.insert({ service.handle_, service });
    return true;
}

void BluetoothGattServerImpl::ReleaseHandle(const int applicationId, const BluetoothGattService& service)
{
    if (gattServiceHandleMap_[applicationId].empty()) {
        gattServiceHandleMap_[applicationId].emplace_front(service.handle_, service.endHandle_);
        return;
    }
    auto item = gattServiceHandleMap_[applicationId].begin();
    while (item != gattServiceHandleMap_[applicationId].end()) {
        auto currentNode = item;
        auto nextNode = ++item;
        if (currentNode->first > service.endHandle_) {
            if (currentNode->first == service.endHandle_ + 1) {
                gattServiceHandleMap_[applicationId].emplace(currentNode, service.handle_, currentNode->second);
                gattServiceHandleMap_[applicationId].erase(currentNode);
            } else {
                gattServiceHandleMap_[applicationId].emplace(currentNode, service.handle_, service.endHandle_);
            }
            break;
        }
        if (nextNode == gattServiceHandleMap_[applicationId].end()) {
            if (service.handle_ == currentNode->second + 1) {
                gattServiceHandleMap_[applicationId].emplace(currentNode, currentNode->first, service.endHandle_);
                gattServiceHandleMap_[applicationId].erase(currentNode);
            } else {
                gattServiceHandleMap_[applicationId].emplace(nextNode, service.handle_, service.endHandle_);
            }
            break;
        } else {
            if (service.handle_ > nextNode->second) {
                continue;
            }
            if (service.handle_ == currentNode->second + 1 && service.endHandle_ + 1 != nextNode->first) {
                gattServiceHandleMap_[applicationId].emplace(currentNode, currentNode->first, service.endHandle_);
                gattServiceHandleMap_[applicationId].erase(currentNode);
            } else if (service.handle_ != currentNode->second + 1 && service.endHandle_ + 1 == nextNode->first) {
                gattServiceHandleMap_[applicationId].emplace(nextNode, service.handle_, nextNode->second);
                gattServiceHandleMap_[applicationId].erase(nextNode);
            } else if (service.handle_ == currentNode->second + 1 && service.endHandle_ + 1 == nextNode->first) {
                gattServiceHandleMap_[applicationId].emplace(nextNode, currentNode->first, nextNode->second);
                gattServiceHandleMap_[applicationId].erase(currentNode);
                gattServiceHandleMap_[applicationId].erase(nextNode);
            } else {
                gattServiceHandleMap_[applicationId].emplace(nextNode, service.handle_, service.endHandle_);
            }
            break;
        }
    }
}

CBMutableCharacteristic* createCharacter(BluetoothGattCharacteristic& cvc)
{
    CBCharacteristicProperties properties = (CBCharacteristicProperties)cvc.properties_;
    CBAttributePermissions permissions = 0;
    if (cvc.permissions_ & static_cast<int>(GattPermission::READABLE)) {
        permissions |= CBAttributePermissionsReadable;
    }
    if (cvc.permissions_ & static_cast<int>(GattPermission::WRITEABLE)) {
        permissions |= CBAttributePermissionsWriteable;
    }
    NSString* strUUID = [NSString stringWithFormat:@"%s", cvc.uuid_.ToString().c_str()];
    CBUUID* characterUUID = [CBUUID UUIDWithString:strUUID];
    uint8_t* rawData = cvc.value_.get();
    size_t dataSize = cvc.length_;
    NSData* characterData = [NSData dataWithBytes:rawData length:dataSize];
    CBMutableCharacteristic* character = nil;
    if (properties & CBCharacteristicPropertyNotify || properties & CBCharacteristicPropertyIndicate) {
        character = [[CBMutableCharacteristic alloc] initWithType:characterUUID
                                                       properties:properties
                                                            value:nil
                                                      permissions:permissions];
    } else {
        character = [[CBMutableCharacteristic alloc] initWithType:characterUUID
                                                       properties:properties
                                                            value:characterData
                                                      permissions:permissions];
    }
    NSMutableArray* arrDescriptor = [NSMutableArray array];
    for (OHOS::bluetooth::Descriptor descriptor : cvc.descriptors_) {
        NSString* strDid = [NSString stringWithFormat:@"%s", descriptor.uuid_.ToString().c_str()];
        strDid = [BluetoothUntils GetSystemStringUUIDKey:strDid];
        CBUUID* descriptorUUID = [CBUUID UUIDWithString:strDid];
        uint8_t* descriptorValue = descriptor.value_.get();
        size_t descriptorLength = descriptor.length_;
        NSData* descriptorsData = [NSData dataWithBytes:descriptorValue length:descriptorLength];
        id value = [BluetoothUntils GetValueWithData:descriptorsData uuid:strDid];
        CBMutableDescriptor* descriptor_ios = [[CBMutableDescriptor alloc] initWithType:descriptorUUID value:value];
        [arrDescriptor addObject:descriptor_ios];
    }
    character.descriptors = arrDescriptor;
    return character;
}

CBMutableService* CreateServices(BluetoothGattService gattService)
{
    NSString* strServiceUUID = [NSString stringWithFormat:@"%s", gattService.uuid_.ToString().c_str()];
    CBUUID* serviceUUID = [CBUUID UUIDWithString:strServiceUUID];
    CBMutableService* mutableService = [[CBMutableService alloc] initWithType:serviceUUID
                                                                      primary:gattService.isPrimary_];
    NSMutableArray* arrCharacter = [NSMutableArray array];
    for (OHOS::bluetooth::Characteristic cvc : gattService.characteristics_) {
        BluetoothGattCharacteristic gattCharacter(cvc);
        CBMutableCharacteristic* character = createCharacter(gattCharacter);
        [arrCharacter addObject:character];
    }

    [mutableService setCharacteristics:arrCharacter];
    return mutableService;
}

int BluetoothGattServerImpl::AddService(int32_t appId, BluetoothGattService* services)
{
    BluetoothGattService gattService = BluetoothGattService(*services);
    CalculateAndAssignHandle(appId, gattService);
    CBMutableService* service = CreateServices(gattService);
    BluetoothPeripheralManager* peripheralManager = [BluetoothPeripheralManager sharedInstance];
    [peripheralManager addService:service
                            appId:appId
                            block:^(int ret) {
                              if (serviceCallBack_) {
                                  serviceCallBack_->OnAddService(ret, gattService);
                              }
                            }];
    return BT_NO_ERROR;
}

void BluetoothGattServerImpl::ClearServices(int appId) {}

int BluetoothGattServerImpl::RegisterApplication(const sptr<IBluetoothGattServerCallback>& callback)
{
    std::lock_guard<std::mutex> lock(gattServerMutex_);
    serviceCallBack_ = callback;
    BluetoothPeripheralManager* peripheralManager = [BluetoothPeripheralManager sharedInstance];
    peripheralManager.connectDeviceBlock = ^(int state, NSString* _Nonnull deviceId) {
      if (serviceCallBack_) {
          BluetoothGattDevice gattDevice;
          gattDevice.addr_ = OHOS::bluetooth::RawAddress(deviceId.UTF8String);
          gattDevice.transport_ = GATT_TRANSPORT_TYPE_LE;
          switch (state) {
              case CBPeripheralStateConnected:
                  serviceCallBack_->OnConnectionStateChanged(
                      gattDevice, BT_NO_ERROR, static_cast<int>(BTConnectState::CONNECTED),
                      static_cast<int>(GattDisconnectReason::CONN_UNKNOWN));
                  break;
              default:
                  serviceCallBack_->OnConnectionStateChanged(
                      gattDevice, BT_NO_ERROR, static_cast<int>(BTConnectState::DISCONNECTED),
                      static_cast<int>(GattDisconnectReason::CONN_UNKNOWN));
                  break;
          }
      }
    };
    GattCharacteristicReadBlock();
    GattCharacteristicWriteBlock();
    GattCharacteristicNotifyBlock();
    int appId = [[BluetoothPeripheralManager sharedInstance] getAppId];
    gattServiceHandleMap_[appId].emplace_front(std::make_pair(MIN_SERVICE_HANDLE, MAX_SERVICE_HANDLE));
    return appId;
}

int BluetoothGattServerImpl::DeregisterApplication(int appId)
{
    std::lock_guard<std::mutex> lock(gattServerMutex_);
    NSString* strAppId = [NSString stringWithFormat:@"%d", appId];
    [[BluetoothPeripheralManager sharedInstance] closePeripheral:strAppId];
    auto item = gattServiceHandleMap_.find(appId);
    if (item != gattServiceHandleMap_.end()) {
        gattServiceHandleMap_.erase(item);
    }
    return BT_NO_ERROR;
}

int BluetoothGattServerImpl::Connect(int appId, const BluetoothGattDevice& device, bool isDirect)
{
    return BT_NO_ERROR;
}

int BluetoothGattServerImpl::CancelConnection(int appId, const BluetoothGattDevice& device)
{
    return BT_NO_ERROR;
}

CBMutableCharacteristic* GetCurrentCharacteristic(uint16_t currentHandle, BluetoothGattService gattService)
{
    CBMutableCharacteristic* character = nil;
    for (OHOS::bluetooth::Characteristic cvc : gattService.characteristics_) {
        BluetoothGattCharacteristic gattCharacter(cvc);
        if (cvc.handle_ == currentHandle) {
            character = createCharacter(gattCharacter);
            break;
        }
    }
    return character;
}

int BluetoothGattServerImpl::NotifyClient(
    const BluetoothGattDevice& device, BluetoothGattCharacteristic* characteristic, bool needConfirm)
{
    if (characteristic == nullptr) {
        HILOGE("characteristic is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    CBMutableService* service = nil;
    CBMutableCharacteristic* character = nil;
    for (auto item = gattServiceMap_.rbegin(); item != gattServiceMap_.rend(); ++item) {
        uint16_t serviceHandle = item->first;
        if (serviceHandle < characteristic->handle_) {
            service = CreateServices(item->second);
            character = GetCurrentCharacteristic(characteristic->handle_, item->second);
            break;
        }
    }
    if (!service || !character) {
        return BT_ERR_INTERNAL_ERROR;
    }
    NSString* strDeviceId = [NSString stringWithFormat:@"%s", device.addr_.GetAddress().c_str()];
    BluetoothPeripheralManager* bpm = [BluetoothPeripheralManager sharedInstance];
    int ret = [bpm notifyCharacteristicChanged:strDeviceId
                                   serviceUUID:service.UUID.UUIDString
                          notifyCharacteristic:character];
    return ret;
}

int BluetoothGattServerImpl::RemoveService(int32_t appId, const BluetoothGattService& services)
{
    ReleaseHandle(appId, services);
    CBMutableService* service = nil;
    for (auto item = gattServiceMap_.begin(); item != gattServiceMap_.end(); ++item) {
        uint16_t serviceHandle = item->first;
        if (serviceHandle == services.handle_) {
            service = CreateServices(item->second);
            gattServiceMap_.erase(serviceHandle);
            break;
        }
    }
    BluetoothPeripheralManager* peripheralManager = [BluetoothPeripheralManager sharedInstance];
    return [peripheralManager removeService:service.UUID.UUIDString appId:appId];
}

int BluetoothGattServerImpl::RespondCharacteristic(
    const uint16_t type, const BluetoothGattDevice& device, const BluetoothGattCharacteristic& gattCharacter)
{
    uint16_t characterHandle = gattCharacter.handle_;
    CBMutableService* service = nil;
    CBMutableCharacteristic* character = nil;
    for (auto item = gattServiceMap_.rbegin(); item != gattServiceMap_.rend(); ++item) {
        uint16_t serviceHandle = item->first;
        if (serviceHandle < characterHandle) {
            service = CreateServices(item->second);
            character = GetCurrentCharacteristic(characterHandle, item->second);
            break;
        }
    }
    if (!service || !character) {
        HILOGE("service character is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    NSString* strDeviceId = [NSString stringWithFormat:@"%s", device.addr_.GetAddress().c_str()];
    BluetoothPeripheralManager* bpm = [BluetoothPeripheralManager sharedInstance];
    int result;
    if (type == CHARACTERISTIC_READ) {
        uint8_t* rawData = gattCharacter.value_.get();
        size_t dataSize = gattCharacter.length_;
        NSData* characterData = [NSData dataWithBytes:rawData length:dataSize];
        result = [bpm sendRespondReadWithDeviceId:strDeviceId
                                      serviceUUID:service.UUID.UUIDString
                                    characterUUID:character.UUID.UUIDString
                                             data:characterData];
    } else {
        result = [bpm sendRespondWriteWithDeviceId:strDeviceId
                                       serviceUUID:service.UUID.UUIDString
                                     characterUUID:character.UUID.UUIDString];
    }
    return result;
}

int BluetoothGattServerImpl::RespondCharacteristicRead(
    const BluetoothGattDevice& device, BluetoothGattCharacteristic* characteristic, int32_t ret)
{
    BluetoothGattCharacteristic gattCharacteristic = BluetoothGattCharacteristic(*characteristic);
    return RespondCharacteristic(CHARACTERISTIC_READ, device, gattCharacteristic);
}

int BluetoothGattServerImpl::RespondCharacteristicWrite(
    const BluetoothGattDevice& device, const BluetoothGattCharacteristic& characteristic, int32_t ret)
{
    return RespondCharacteristic(CHARACTERISTIC_WRITE, device, characteristic);
}

int BluetoothGattServerImpl::RespondDescriptorRead(
    const BluetoothGattDevice& device, BluetoothGattDescriptor* descriptor, int32_t ret)
{
    return BT_NO_ERROR;
}

int BluetoothGattServerImpl::RespondDescriptorWrite(
    const BluetoothGattDevice& device, const BluetoothGattDescriptor& descriptor, int32_t ret)
{
    return BT_NO_ERROR;
}

int32_t BluetoothGattServerImpl::GetGattServerAppId()
{
    appId_ += 1;
    return appId_;
}

void BluetoothGattServerImpl::GetGattCharacteristic(std::string serviceUUID, std::string charaUUID, int length,
    const uint8_t* bytesValue, BluetoothGattCharacteristic& character)
{
    for (auto item = gattServiceMap_.begin(); item != gattServiceMap_.end(); ++item) {
        BluetoothGattService gattService = item->second;
        if (serviceUUID != gattService.uuid_.ToString()) {
            continue;
        }
        for (OHOS::bluetooth::Characteristic cvc : gattService.characteristics_) {
            if (charaUUID != cvc.uuid_.ToString()) {
                continue;
            }
            character.length_ = cvc.length_;
            character.uuid_ = cvc.uuid_;
            character.handle_ = cvc.handle_;
            character.properties_ = cvc.properties_;
            character.permissions_ = cvc.permissions_;
            character.SetValue(bytesValue, length);
            for (auto& desc : cvc.descriptors_) {
                character.descriptors_.push_back(desc);
            }
            break;
        }
    }
}

void BluetoothGattServerImpl::GattCharacteristicWriteBlock()
{
    BluetoothPeripheralManager* peripheralManager = [BluetoothPeripheralManager sharedInstance];
    peripheralManager.characterWriteBlock =
        ^(NSString* _Nonnull deviceId, CBMutableCharacteristic* _Nonnull characteristic) {
          if (serviceCallBack_ == nullptr) {
              return;
          }
          NSUInteger length = [characteristic.value length];
          const uint8_t* bytesValue = (const uint8_t*)[characteristic.value bytes];
          std::string serviceUuid = characteristic.service.UUID.UUIDString.UTF8String;
          std::string characterUuid = characteristic.UUID.UUIDString.UTF8String;
          BluetoothGattCharacteristic gattCharacteristic;
          GetGattCharacteristic(serviceUuid, characterUuid, length, bytesValue, gattCharacteristic);

          bool needRespones = characteristic.properties & CBCharacteristicPropertyWriteWithoutResponse ? false : true;
          BluetoothGattDevice gattDevice;
          gattDevice.addr_ = OHOS::bluetooth::RawAddress(deviceId.UTF8String);
          gattDevice.transport_ = GATT_TRANSPORT_TYPE_LE;
          serviceCallBack_->OnCharacteristicWriteRequest(gattDevice, gattCharacteristic, needRespones);
        };
}

void BluetoothGattServerImpl::GattCharacteristicReadBlock()
{
    BluetoothPeripheralManager* peripheralManager = [BluetoothPeripheralManager sharedInstance];
    peripheralManager.characterReadBlock =
        ^(NSString* _Nonnull deviceId, CBMutableCharacteristic* _Nonnull characteristic) {
          if (serviceCallBack_ == nullptr) {
              return;
          }
          NSUInteger length = [characteristic.value length];
          const uint8_t* bytesValue = (const uint8_t*)[characteristic.value bytes];
          std::string serviceUuid = characteristic.service.UUID.UUIDString.UTF8String;
          std::string characterUuid = characteristic.UUID.UUIDString.UTF8String;
          BluetoothGattCharacteristic gattCharacteristic;
          GetGattCharacteristic(serviceUuid, characterUuid, length, bytesValue, gattCharacteristic);
          BluetoothGattDevice gattDevice;
          gattDevice.addr_ = OHOS::bluetooth::RawAddress(deviceId.UTF8String);
          gattDevice.transport_ = GATT_TRANSPORT_TYPE_LE;
          serviceCallBack_->OnCharacteristicReadRequest(gattDevice, gattCharacteristic);
        };
}

void BluetoothGattServerImpl::GattCharacteristicNotifyBlock()
{
    BluetoothPeripheralManager* peripheralManager = [BluetoothPeripheralManager sharedInstance];
    peripheralManager.characterNotifyBlock =
        ^(NSString* _Nonnull deviceId, CBCharacteristic* _Nonnull characteristic, int state) {
          if (serviceCallBack_ == nullptr) {
              return;
          }
          NSUInteger length = [characteristic.value length];
          const uint8_t* bytesValue = (const uint8_t*)[characteristic.value bytes];
          std::string serviceUuid = characteristic.service.UUID.UUIDString.UTF8String;
          std::string characterUuid = characteristic.UUID.UUIDString.UTF8String;
          BluetoothGattCharacteristic gattCharacteristic;
          GetGattCharacteristic(serviceUuid, characterUuid, length, bytesValue, gattCharacteristic);

          BluetoothGattDevice gattDevice;
          gattDevice.addr_ = OHOS::bluetooth::RawAddress(deviceId.UTF8String);
          gattDevice.transport_ = GATT_TRANSPORT_TYPE_LE;
          serviceCallBack_->OnNotifyConfirm(gattDevice, gattCharacteristic, BT_NO_ERROR);
        };
}
} // namespace Bluetooth
} // namespace OHOS