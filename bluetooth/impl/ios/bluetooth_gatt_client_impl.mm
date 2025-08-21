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

#include "bluetooth_gatt_client_impl.h"

#import "BluetoothCentralManager.h"
#import "BluetoothUntils.h"
#include "bluetooth_log.h"
#include "securec.h"

using namespace OHOS::bluetooth;

const uint16_t MIN_CLIENT_HANDLE = 1;
const uint16_t MAX_CLIENT_HANDLE = 65535;

namespace OHOS {
namespace Bluetooth {
BluetoothGattService BluetoothGattClientImpl::GetServiceWithCharacterHandle(uint16_t characterHandle)
{
    for (auto item = gattServiceMap_.rbegin(); item != gattServiceMap_.rend(); ++item) {
        BluetoothGattService service = BluetoothGattService(item->second);
        if (service.handle_ < characterHandle && service.endHandle_ > characterHandle) {
            return service;
        }
    }
    return BluetoothGattService();
}

CBMutableCharacteristic* CreateClientCharacter(BluetoothGattCharacteristic& cvc)
{
    CBCharacteristicProperties properties = (CBCharacteristicProperties)cvc.properties_;
    NSString* strUUID = [NSString stringWithFormat:@"%s", cvc.uuid_.ToString().c_str()];
    CBUUID* characterUUID = [CBUUID UUIDWithString:strUUID];
    uint8_t* rawData = cvc.value_.get();
    size_t dataSize = cvc.length_;
    NSData* characterData = [NSData dataWithBytes:rawData length:dataSize];
    CBMutableCharacteristic* character = nil;
    CBAttributePermissions permissions = 0;
    if (cvc.permissions_ & static_cast<int>(GattPermission::READABLE)) {
        permissions |= CBAttributePermissionsReadable;
    }
    if (cvc.permissions_ & static_cast<int>(GattPermission::WRITEABLE)) {
        permissions |= CBAttributePermissionsWriteable;
    }
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

CBMutableCharacteristic* GetClientCurrentCharacteristic(uint16_t currentHandle, BluetoothGattService gattService)
{
    CBMutableCharacteristic* character = nil;
    for (OHOS::bluetooth::Characteristic cvc : gattService.characteristics_) {
        BluetoothGattCharacteristic gattCharacter(cvc);
        if (cvc.handle_ == currentHandle) {
            character = CreateClientCharacter(gattCharacter);
            break;
        }
    }
    return character;
}

BluetoothGattClientImpl::BluetoothGattClientImpl()
{
    appId_ = 0;
}

int BluetoothGattClientImpl::RegisterApplication(
    const sptr<IBluetoothGattClientCallback>& callback, const BluetoothRawAddress& addr, int32_t transport)
{
    return BT_NO_ERROR;
}

int BluetoothGattClientImpl::RegisterApplication(
    const sptr<IBluetoothGattClientCallback>& callback, const BluetoothRawAddress& addr, int32_t transport, int& appId)
{
    if (callback == nullptr || addr.GetAddress().empty()) {
        return BT_ERR_INTERNAL_ERROR;
    }
    std::lock_guard<std::mutex> lock(gattClientMutex_);
    BluetoothCentralManager* centralManager = [BluetoothCentralManager sharedInstance];
    appId = GetGattClientAppId();
    NSString* strDeviceId = [NSString stringWithFormat:@"%s", addr.GetAddress().c_str()];
    [centralManager registerDeviceId:strDeviceId withAppId:appId];
    gattClientHandleMap_.insert({ appId, std::make_pair(MIN_CLIENT_HANDLE, MAX_CLIENT_HANDLE) });
    gattClientCallBackMap_.insert({ appId, callback });
    auto gattClientCallback = gattClientCallBackMap_.find(appId);
    if (gattClientCallback != gattClientCallBackMap_.end()) {
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

int BluetoothGattClientImpl::DeregisterApplication(int32_t appId)
{
    std::lock_guard<std::mutex> lock(gattClientMutex_);
    [[BluetoothCentralManager sharedInstance] closeClient:appId];
    auto gattClientCallback = gattClientCallBackMap_.find(appId);
    if (gattClientCallback != gattClientCallBackMap_.end()) {
        gattClientCallBackMap_.erase(gattClientCallback);
        return BT_NO_ERROR;
    } else {
        return BT_ERR_INTERNAL_ERROR;
    }
}

int BluetoothGattClientImpl::Connect(int32_t appId, bool autoConnect)
{
    BluetoothCentralManager* centralManager = [BluetoothCentralManager sharedInstance];
    [centralManager connect:appId autoConnect:autoConnect];
    centralManager.connectStateBlock = ^(int ret, int state) {
      ChangeConnectState(appId, ret, state);
    };
    return BT_NO_ERROR;
}

int BluetoothGattClientImpl::Disconnect(int32_t appId)
{
    BluetoothCentralManager* centralManager = [BluetoothCentralManager sharedInstance];
    [centralManager disConnect:appId];
    centralManager.connectStateBlock = ^(int ret, int state) {
      ChangeConnectState(appId, ret, state);
    };
    return BT_NO_ERROR;
}

int BluetoothGattClientImpl::DiscoveryServices(int32_t appId)
{
    BluetoothCentralManager* centralManager = [BluetoothCentralManager sharedInstance];
    centralManager.servicesDiscoveredBlock = ^(int32_t ret) {
      OHOS::sptr<IBluetoothGattClientCallback> callback = GetCallBack(appId);
      if (callback == nullptr) {
          return;
      }
      callback->OnServicesDiscovered(ret);
    };
    return [centralManager discoverServices:appId];
}

void BluetoothGattClientImpl::GetGattCharacteristic(std::string serviceUUID, std::string charaUUID, int length,
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
            character.uuid_ = cvc.uuid_;
            character.handle_ = cvc.handle_;
            character.properties_ = cvc.properties_;
            character.permissions_ = cvc.permissions_;
            character.length_ = cvc.length_;
            character.SetValue(bytesValue, length);
            for (auto& desc : cvc.descriptors_) {
                character.descriptors_.push_back(desc);
            }
            break;
        }
    }
}

int BluetoothGattClientImpl::ReadCharacteristic(int32_t appId, const BluetoothGattCharacteristic& characteristic)
{
    BluetoothGattService service = GetServiceWithCharacterHandle(characteristic.handle_);
    if (service.endHandle_ <= 0) {
        return BT_ERR_INTERNAL_ERROR;
    }
    NSString* strServiceUuid = [NSString stringWithFormat:@"%s", service.uuid_.ToString().c_str()];
    CBUUID* serviceUuid = [CBUUID UUIDWithString:strServiceUuid];
    CBMutableCharacteristic* chara = GetClientCurrentCharacteristic(characteristic.handle_, service);
    int ret = [[BluetoothCentralManager sharedInstance]
        getReadCharacteristic:appId
                  serviceUuid:serviceUuid
                characterUuid:chara.UUID
                         data:^(CBCharacteristic* chara, int32_t ret) {
                           NSUInteger length = [chara.value length];
                           const uint8_t* bytesValue = (const uint8_t*)[chara.value bytes];
                           std::string serviceUuid = chara.service.UUID.UUIDString.UTF8String;
                           std::string characterUuid = chara.UUID.UUIDString.UTF8String;
                           BluetoothGattCharacteristic gattCharacteristic;
                           GetGattCharacteristic(serviceUuid, characterUuid, length, bytesValue, gattCharacteristic);
                           OHOS::sptr<IBluetoothGattClientCallback> callback = GetCallBack(appId);
                           if (callback == nullptr || ret != BT_NO_ERROR) {
                               return;
                           }
                           callback->OnCharacteristicRead(GattStatus::GATT_SUCCESS, gattCharacteristic);
                         }];
    return BT_NO_ERROR;
}

int BluetoothGattClientImpl::WriteCharacteristic(
    int32_t appId, BluetoothGattCharacteristic* characteristic, bool withoutRespond)
{
    BluetoothGattCharacteristic gattCharacteristic = BluetoothGattCharacteristic(*characteristic);
    uint8_t* rawData = gattCharacteristic.value_.get();
    size_t dataSize = gattCharacteristic.length_;
    NSData* nsData = [NSData dataWithBytes:rawData length:dataSize];
    BluetoothGattService service = GetServiceWithCharacterHandle(gattCharacteristic.handle_);
    if (service.endHandle_ <= 0) {
        return BT_ERR_INTERNAL_ERROR;
    }
    NSString* strServiceUuid = [NSString stringWithFormat:@"%s", service.uuid_.ToString().c_str()];
    CBUUID* serviceUuid = [CBUUID UUIDWithString:strServiceUuid];
    CBMutableCharacteristic* chara = GetClientCurrentCharacteristic(gattCharacteristic.handle_, service);

    BluetoothCentralManager* centralManager = [BluetoothCentralManager sharedInstance];
    int ret = [centralManager
        writeCharacteristic:appId
                serviceUuid:serviceUuid
              characterUuid:chara.UUID
                       data:nsData
              isOutResponse:withoutRespond
                   isSigned:false
                      block:^(CBCharacteristic* chara, int32_t ret) {
                        NSUInteger length = [chara.value length];
                        const uint8_t* bytesValue = (const uint8_t*)[chara.value bytes];
                        std::string serviceUuid = chara.service.UUID.UUIDString.UTF8String;
                        std::string characterUuid = chara.UUID.UUIDString.UTF8String;
                        BluetoothGattCharacteristic gattCharacteristic;
                        GetGattCharacteristic(serviceUuid, characterUuid, length, bytesValue, gattCharacteristic);
                        OHOS::sptr<IBluetoothGattClientCallback> callback = GetCallBack(appId);
                        if (callback == nullptr) {
                            return;
                        }
                        callback->OnCharacteristicWrite(ret, gattCharacteristic);
                      }];
    return ret;
}

int BluetoothGattClientImpl::SignedWriteCharacteristic(int32_t appId, BluetoothGattCharacteristic* characteristic)
{
    BluetoothGattCharacteristic gattCharacteristic = BluetoothGattCharacteristic(*characteristic);
    uint8_t* rawData = gattCharacteristic.value_.get();
    size_t dataSize = gattCharacteristic.length_;
    NSData* nsData = [NSData dataWithBytes:rawData length:dataSize];

    BluetoothGattService service = GetServiceWithCharacterHandle(gattCharacteristic.handle_);
    if (service.endHandle_ <= 0) {
        return BT_ERR_INTERNAL_ERROR;
    }
    NSString* strServiceUuid = [NSString stringWithFormat:@"%s", service.uuid_.ToString().c_str()];
    CBUUID* serviceUuid = [CBUUID UUIDWithString:strServiceUuid];
    CBMutableCharacteristic* chara = GetClientCurrentCharacteristic(gattCharacteristic.handle_, service);

    BluetoothCentralManager* centralManager = [BluetoothCentralManager sharedInstance];
    int ret = [centralManager
        writeCharacteristic:appId
                serviceUuid:serviceUuid
              characterUuid:chara.UUID
                       data:nsData
              isOutResponse:true
                   isSigned:true
                      block:^(CBCharacteristic* chara, int32_t ret) {
                        NSUInteger length = [chara.value length];
                        const uint8_t* bytesValue = (const uint8_t*)[chara.value bytes];
                        std::string serviceUuid = chara.service.UUID.UUIDString.UTF8String;
                        std::string characterUuid = chara.UUID.UUIDString.UTF8String;
                        BluetoothGattCharacteristic gattCharacteristic;
                        GetGattCharacteristic(serviceUuid, characterUuid, length, bytesValue, gattCharacteristic);
                        OHOS::sptr<IBluetoothGattClientCallback> callback = GetCallBack(appId);
                        if (callback == nullptr) {
                            return;
                        }
                        callback->OnCharacteristicWrite(ret, gattCharacteristic);
                      }];
    return ret;
}

void CreateClientDescriptor(CBDescriptor* des, BluetoothGattDescriptor& gattDes)
{
    gattDes.uuid_ = Uuid::ConvertFromString(des.UUID.UUIDString.UTF8String);
    NSData* data = [NSData data];
    if ([des.value isKindOfClass:[NSNumber class]]) {
        data = [NSKeyedArchiver archivedDataWithRootObject:des.value];
    } else if ([des.value isKindOfClass:[NSString class]]) {
        data = [des.value dataUsingEncoding:NSUTF8StringEncoding];
    } else if ([des.value isKindOfClass:[NSData class]]) {
        data = des.value;
    } else {
        return;
    }
    NSUInteger length = [data length];
    const uint8_t* bytesValue = (const uint8_t*)[data bytes];
    gattDes.length_ = length;
    if (memcpy_s(gattDes.value_.get(), length, bytesValue, length) != EOK) {
        HILOGE("Descriptor SetValue Failure");
    }
}

int BluetoothGattClientImpl::ReadDescriptor(int32_t appId, const BluetoothGattDescriptor& descriptor)
{
    BluetoothGattService service = GetServiceWithCharacterHandle(descriptor.handle_);
    if (service.endHandle_ <= 0) {
        return BT_ERR_INTERNAL_ERROR;
    }
    NSString* strServiceUuid = [NSString stringWithFormat:@"%s", service.uuid_.ToString().c_str()];
    NSString* strCharaUuid;
    for (OHOS::bluetooth::Characteristic cvc : service.characteristics_) {
        BluetoothGattCharacteristic gattCharacter(cvc);
        if (cvc.handle_ < descriptor.handle_ && cvc.endHandle_ > descriptor.handle_) {
            strCharaUuid = [NSString stringWithFormat:@"%s", cvc.uuid_.ToString().c_str()];
            break;
        }
    }
    auto item = gattDescriptorMap_.find(descriptor.handle_);
    if (item == gattDescriptorMap_.end()) {
        return BT_ERR_INTERNAL_ERROR;
    }
    BluetoothGattDescriptor gattDes = BluetoothGattDescriptor(item->second);
    NSString* strDesUuid = [NSString stringWithFormat:@"%s", gattDes.uuid_.ToString().c_str()];

    CBUUID* serviceUuid = [CBUUID UUIDWithString:strServiceUuid];
    CBUUID* charaUuid = [CBUUID UUIDWithString:strCharaUuid];
    CBUUID* desUuid = [CBUUID UUIDWithString:strDesUuid];
    if (!serviceUuid || !charaUuid || !desUuid) {
        HILOGE("UUID is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    return [[BluetoothCentralManager sharedInstance] getReadDescriptor:appId
                                                           serviceUuid:serviceUuid
                                                         characterUuid:charaUuid
                                                        descriptorUuid:desUuid
                                                                  data:^(CBDescriptor* des, int32_t ret) {
                                                                    BluetoothGattDescriptor desCallback;
                                                                    CreateClientDescriptor(des, desCallback);
                                                                    OHOS::sptr<IBluetoothGattClientCallback> callback =
                                                                        GetCallBack(appId);
                                                                    if (callback == nullptr) {
                                                                        return;
                                                                    }
                                                                    callback->OnDescriptorRead(ret, desCallback);
                                                                  }];
}

int BluetoothGattClientImpl::WriteDescriptor(int32_t appId, BluetoothGattDescriptor* descriptor)
{
    return BT_NO_ERROR;
}

int BluetoothGattClientImpl::RequestExchangeMtu(int32_t appId, int32_t mtu)
{
    return BT_NO_ERROR;
}

void BluetoothGattClientImpl::GetAllDevice(std::vector<BluetoothGattDevice>& device) {}

int BluetoothGattClientImpl::RequestConnectionPriority(int32_t appId, int32_t connPriority)
{
    return BT_NO_ERROR;
}

bool BluetoothGattClientImpl::CalculateAndAssignHandle(const int32_t applicationId, BluetoothGattService& service)
{
    uint16_t count = 1;
    std::pair<uint16_t, uint16_t> handlePair(0, 0);
    count += service.includeServices_.size();
    count += service.characteristics_.size() * 0x2;
    for (auto& ccc : service.characteristics_) {
        count += ccc.descriptors_.size();
    }
    auto item = gattClientHandleMap_[applicationId];
    auto availableLength = item.second - item.first + 1;
    if (availableLength >= count) {
        handlePair = std::pair<uint16_t, uint16_t>(item.first, item.first + count - 1);
        if (handlePair.second < item.second) {
            gattClientHandleMap_[applicationId] = std::pair<uint16_t, uint16_t>(handlePair.second + 1, item.second);
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
            BluetoothGattDescriptor gattDescriptor = BluetoothGattDescriptor(descriptor);
            gattDescriptorMap_.insert(std::make_pair(descriptor.handle_, gattDescriptor));
        }
        charactor.endHandle_ = currentHandle - 1;
    }
    gattServiceMap_.insert(std::make_pair(service.handle_, service));
    return true;
}

void createGattCharacter(Characteristic& cvc, CBCharacteristic* characteristic)
{
    cvc.uuid_ = Uuid::ConvertFromString(characteristic.UUID.UUIDString.UTF8String);
    if (characteristic.properties != CBCharacteristicPropertyNotifyEncryptionRequired &&
        characteristic.properties != CBCharacteristicPropertyIndicateEncryptionRequired) {
        cvc.properties_ = characteristic.properties;
    }
    if (characteristic.value) {
        NSUInteger length = [characteristic.value length];
        const uint8_t* bytesValue = (const uint8_t*)[characteristic.value bytes];
        cvc.SetValue(bytesValue, length);
    }
    for (CBDescriptor* descriptor_ios in characteristic.descriptors) {
        Descriptor descriptor;
        NSString* strDesUuid = descriptor_ios.UUID.UUIDString;
        if (strDesUuid.length == 4) {
            strDesUuid = [NSString stringWithFormat:@"0000%@-0000-1000-8000-00805F9B34FB", strDesUuid];
        }
        descriptor.uuid_ = Uuid::ConvertFromString(strDesUuid.UTF8String);
        if (descriptor_ios.value) {
            NSData* data = [NSData data];
            if ([descriptor_ios.value isKindOfClass:[NSNumber class]]) {
                data = [NSKeyedArchiver archivedDataWithRootObject:descriptor_ios.value];
            } else if ([descriptor_ios.value isKindOfClass:[NSString class]]) {
                data = [descriptor_ios.value dataUsingEncoding:NSUTF8StringEncoding];
            } else if ([descriptor_ios.value isKindOfClass:[NSData class]]) {
                data = descriptor_ios.value;
            } else {
                break;
            }
            NSUInteger length = [data length];
            const uint8_t* bytesValue = (const uint8_t*)[data bytes];
            descriptor.length_ = length;
            if (memcpy_s(descriptor.value_.get(), length, bytesValue, length) != EOK) {
                HILOGE("Descriptor SetValue Failure");
            }
        }
        cvc.descriptors_.push_back(descriptor);
    }
}

int BluetoothGattClientImpl::GetServices(int32_t appId, std::vector<BluetoothGattService>& service)
{
    NSArray<CBService*>* arrServices = [[BluetoothCentralManager sharedInstance] getServices:appId];

    for (CBService* service_ios in arrServices) {
        BluetoothGattService svc;
        svc.isPrimary_ = service_ios.isPrimary;
        svc.uuid_ = Uuid::ConvertFromString(service_ios.UUID.UUIDString.UTF8String);
        for (CBCharacteristic* characteristic in service_ios.characteristics) {
            Characteristic cvc;
            createGattCharacter(cvc, characteristic);
            svc.characteristics_.push_back(cvc);
        }
        CalculateAndAssignHandle(appId, svc);
        service.push_back(svc);
    }
    return BT_NO_ERROR;
}

int BluetoothGattClientImpl::RequestFastestConn(const BluetoothRawAddress& addr)
{
    return BT_NO_ERROR;
}

int BluetoothGattClientImpl::ReadRemoteRssiValue(int32_t appId)
{
    return BT_NO_ERROR;
}

int BluetoothGattClientImpl::RequestNotification(int32_t appId, uint16_t characterHandle, bool enable)
{
    return BT_NO_ERROR;
}

int BluetoothGattClientImpl::GetGattClientAppId()
{
    appId_ += 1;
    return appId_;
}

OHOS::sptr<IBluetoothGattClientCallback> BluetoothGattClientImpl::GetCallBack(const int32_t appId)
{
    auto gattClientCallback = gattClientCallBackMap_.find(appId);
    if (gattClientCallback != gattClientCallBackMap_.end()) {
        OHOS::sptr<OHOS::Bluetooth::IBluetoothGattClientCallback>& callback = gattClientCallBackMap_[appId];
        return callback;
    }
    return nullptr;
}

void BluetoothGattClientImpl::ChangeConnectState(int32_t appId, int ret, int state)
{
    OHOS::sptr<IBluetoothGattClientCallback> callback = GetCallBack(appId);
    if (callback == nullptr) {
        return;
    }
    switch (state) {
        case CBPeripheralStateConnecting:
            callback->OnConnectionStateChanged(ret, static_cast<int>(BTConnectState::CONNECTING),
                static_cast<int>(GattDisconnectReason::CONN_UNKNOWN));
            break;
        case CBPeripheralStateConnected:
            callback->OnConnectionStateChanged(ret, static_cast<int>(BTConnectState::CONNECTED),
                static_cast<int>(GattDisconnectReason::CONN_UNKNOWN));
            break;
        case CBPeripheralStateDisconnecting:
            callback->OnConnectionStateChanged(ret, static_cast<int>(BTConnectState::DISCONNECTING),
                static_cast<int>(GattDisconnectReason::CONN_UNKNOWN));
            break;
        default:
            callback->OnConnectionStateChanged(ret, static_cast<int>(BTConnectState::DISCONNECTED),
                static_cast<int>(GattDisconnectReason::CONN_UNKNOWN));
            break;
    }
}
} // namespace Bluetooth
} // namespace OHOS