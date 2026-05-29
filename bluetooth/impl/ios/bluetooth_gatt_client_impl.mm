/*
 * Copyright (C) 2024-2026 Huawei Device Co., Ltd.
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

#include <memory>

#import "BluetoothCentralManager.h"
#import "BluetoothUntils.h"
#include "bluetooth_log.h"
#include "securec.h"

using namespace OHOS::bluetooth;

const uint16_t MIN_CLIENT_HANDLE = 1;
const uint16_t MAX_CLIENT_HANDLE = 65535;

namespace OHOS {
namespace Bluetooth {
static constexpr int64_t DESCRIPTOR_WRITE_CALLBACK_DELAY_MS = 10;
static constexpr size_t CLIENT_CONFIG_VALUE_SIZE = 2;
static const uint8_t DISABLE_NOTIFY_VALUE[] = {0, 0};
static const uint8_t ENABLE_NOTIFY_VALUE[] = {1, 0};
static const char CLIENT_CHARACTERISTIC_CONFIG_UUID[] = "00002902-0000-1000-8000-00805F9B34FB";

static bool IsClientCharacteristicConfigurationDescriptor(const Uuid& uuid)
{
    NSString* strUuid = [NSString stringWithFormat:@"%s", uuid.ToString().c_str()];
    NSString* systemUuid = [BluetoothUntils GetSystemStringUUIDKey:strUuid];
    return [systemUuid isEqualToString:CBUUIDClientCharacteristicConfigurationString];
}

static bool HasClientCharacteristicConfigurationDescriptor(const Characteristic& characteristic)
{
    for (const auto& descriptor : characteristic.descriptors_) {
        if (IsClientCharacteristicConfigurationDescriptor(descriptor.uuid_)) {
            return true;
        }
    }
    return false;
}

static bool HasNotifyOrIndicateProperty(CBCharacteristicProperties properties)
{
    return (properties & (CBCharacteristicPropertyNotify | CBCharacteristicPropertyIndicate |
        CBCharacteristicPropertyNotifyEncryptionRequired | CBCharacteristicPropertyIndicateEncryptionRequired)) != 0;
}

static void AddClientCharacteristicConfigurationDescriptor(Characteristic& cvc, CBCharacteristic* characteristic)
{
    if (HasClientCharacteristicConfigurationDescriptor(cvc) ||
        !HasNotifyOrIndicateProperty(characteristic.properties)) {
        return;
    }
    Descriptor descriptor(Uuid::ConvertFromString(CLIENT_CHARACTERISTIC_CONFIG_UUID),
        static_cast<int>(GattPermission::READABLE) | static_cast<int>(GattPermission::WRITEABLE));
    const uint8_t* value = characteristic.isNotifying ? ENABLE_NOTIFY_VALUE : DISABLE_NOTIFY_VALUE;
    descriptor.value_ = std::make_unique<uint8_t[]>(CLIENT_CONFIG_VALUE_SIZE);
    descriptor.length_ = CLIENT_CONFIG_VALUE_SIZE;
    if (memcpy_s(descriptor.value_.get(), descriptor.length_, value, descriptor.length_) != EOK) {
        HILOGE("Descriptor SetValue Failure");
    }
    cvc.descriptors_.push_back(std::move(descriptor));
}

static void DispatchDescriptorWriteCallback(OHOS::sptr<IBluetoothGattClientCallback> callback, int32_t ret,
    const std::shared_ptr<BluetoothGattDescriptor>& descriptor)
{
    if (callback == nullptr || descriptor == nullptr) {
        return;
    }
    OHOS::sptr<IBluetoothGattClientCallback> callbackHolder = callback;
    std::shared_ptr<BluetoothGattDescriptor> descriptorHolder = descriptor;
    dispatch_after(
        dispatch_time(DISPATCH_TIME_NOW, DESCRIPTOR_WRITE_CALLBACK_DELAY_MS * NSEC_PER_MSEC),
        dispatch_get_main_queue(), ^{
            callbackHolder->OnDescriptorWrite(ret, *descriptorHolder);
        });
}

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
    for (const auto& descriptor : cvc.descriptors_) {
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
        pendingNotifyDescriptorHandleMap_.erase(appId);
        pendingNotifyDescriptorMap_.erase(appId);
        pendingNotifyResultMap_.erase(appId);
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

int BluetoothGattClientImpl::ReadCharacteristicByUuid(int32_t appId, const std::string& uuid,
    int32_t  startHandle, int32_t  endHandle)
{
    HILOGI("NOT SUPPORT NOW");
    return BT_NO_ERROR;
}

int BluetoothGattClientImpl::WriteCharacteristic(
    int32_t appId, BluetoothGattCharacteristic* characteristic, bool withoutRespond, bool isWithContext)
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
    BluetoothGattRspContext rspContext;
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
                        callback->OnCharacteristicWrite(ret, gattCharacteristic, rspContext);
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
    BluetoothGattRspContext rspContext;
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
                        callback->OnCharacteristicWrite(ret, gattCharacteristic, rspContext);
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
    for (const auto& cvc : service.characteristics_) {
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
    if (descriptor == nullptr) {
        return BT_ERR_INTERNAL_ERROR;
    }
    auto item = gattDescriptorMap_.find(descriptor->handle_);
    if (item == gattDescriptorMap_.end()) {
        return BT_ERR_INTERNAL_ERROR;
    }
    BluetoothGattDescriptor gattDes = BluetoothGattDescriptor(item->second);
    bool isClientConfigDescriptor = IsClientCharacteristicConfigurationDescriptor(gattDes.uuid_);
    if (isClientConfigDescriptor) {
        return HandleClientConfigDescriptorWrite(appId, descriptor);
    }
    return WriteDescriptorToPeripheral(appId, descriptor, gattDes);
}

bool BluetoothGattClientImpl::TakePendingNotifyDescriptor(int32_t appId, uint16_t descriptorHandle)
{
    std::lock_guard<std::mutex> lock(gattClientMutex_);
    auto pendingItem = pendingNotifyDescriptorHandleMap_.find(appId);
    if (pendingItem == pendingNotifyDescriptorHandleMap_.end()) {
        return false;
    }
    bool isPendingNotifyDescriptor = pendingItem->second.erase(descriptorHandle) > 0;
    if (pendingItem->second.empty()) {
        pendingNotifyDescriptorHandleMap_.erase(pendingItem);
    }
    return isPendingNotifyDescriptor;
}

bool BluetoothGattClientImpl::TakePendingNotifyResult(int32_t appId, uint16_t descriptorHandle, int32_t& notifyRet)
{
    auto resultItem = pendingNotifyResultMap_.find(appId);
    if (resultItem == pendingNotifyResultMap_.end()) {
        return false;
    }
    auto descriptorItem = resultItem->second.find(descriptorHandle);
    if (descriptorItem == resultItem->second.end()) {
        return false;
    }
    notifyRet = descriptorItem->second;
    resultItem->second.erase(descriptorItem);
    if (resultItem->second.empty()) {
        pendingNotifyResultMap_.erase(resultItem);
    }
    return true;
}

int BluetoothGattClientImpl::HandleClientConfigDescriptorWrite(int32_t appId, BluetoothGattDescriptor* descriptor)
{
    if (!TakePendingNotifyDescriptor(appId, descriptor->handle_)) {
        HILOGI("iOS does not support explicit CCCD write; use setNotifyValue subscription path.");
        return BT_ERR_API_NOT_SUPPORT;
    }
    OHOS::sptr<IBluetoothGattClientCallback> callback = GetCallBack(appId);
    if (callback == nullptr) {
        std::lock_guard<std::mutex> lock(gattClientMutex_);
        pendingNotifyDescriptorMap_.erase(appId);
        pendingNotifyResultMap_.erase(appId);
        return BT_ERR_INTERNAL_ERROR;
    }
    auto descriptorCallback = std::make_shared<BluetoothGattDescriptor>(*descriptor);
    int32_t notifyRet = BT_ERR_INTERNAL_ERROR;
    bool hasNotifyRet = false;
    {
        std::lock_guard<std::mutex> lock(gattClientMutex_);
        hasNotifyRet = TakePendingNotifyResult(appId, descriptor->handle_, notifyRet);
        if (!hasNotifyRet) {
            auto& descriptorMap = pendingNotifyDescriptorMap_[appId];
            descriptorMap.erase(descriptor->handle_);
            descriptorMap.emplace(descriptor->handle_, *descriptorCallback);
        }
    }
    if (hasNotifyRet) {
        DispatchDescriptorWriteCallback(callback, notifyRet, descriptorCallback);
    }
    return BT_NO_ERROR;
}

int BluetoothGattClientImpl::WriteDescriptorToPeripheral(
    int32_t appId, BluetoothGattDescriptor* descriptor, const BluetoothGattDescriptor& gattDes)
{
    BluetoothGattService service = GetServiceWithCharacterHandle(descriptor->handle_);
    if (service.endHandle_ <= 0) {
        return BT_ERR_INTERNAL_ERROR;
    }
    NSString* strServiceUuid = [NSString stringWithFormat:@"%s", service.uuid_.ToString().c_str()];
    NSString* strCharaUuid = nil;
    for (const auto& cvc : service.characteristics_) {
        if (cvc.handle_ < descriptor->handle_ && cvc.endHandle_ > descriptor->handle_) {
            strCharaUuid = [NSString stringWithFormat:@"%s", cvc.uuid_.ToString().c_str()];
            break;
        }
    }
    NSString* strDesUuid = [NSString stringWithFormat:@"%s", gattDes.uuid_.ToString().c_str()];
    CBUUID* serviceUuid = [CBUUID UUIDWithString:strServiceUuid];
    CBUUID* charaUuid = [CBUUID UUIDWithString:strCharaUuid];
    CBUUID* desUuid = [CBUUID UUIDWithString:strDesUuid];
    if (!serviceUuid || !charaUuid || !desUuid || descriptor->value_ == nullptr || descriptor->length_ == 0) {
        return BT_ERR_INTERNAL_ERROR;
    }
    NSData* data = [NSData dataWithBytes:descriptor->value_.get() length:descriptor->length_];
    auto descriptorCallback = std::make_shared<BluetoothGattDescriptor>(*descriptor);
    return [[BluetoothCentralManager sharedInstance] writeDescriptor:appId
                                                         serviceUuid:serviceUuid
                                                       characterUuid:charaUuid
                                                      descriptorUuid:desUuid
                                                                data:data
                                                               block:^(CBDescriptor* des, int32_t ret) {
                                                                 OHOS::sptr<IBluetoothGattClientCallback> callback =
                                                                     GetCallBack(appId);
                                                                 if (callback == nullptr) {
                                                                     return;
                                                                 }
                                                                 callback->OnDescriptorWrite(ret, *descriptorCallback);
                                                               }];
}

int BluetoothGattClientImpl::RequestExchangeMtu(int32_t appId, int32_t mtu)
{
    return BT_NO_ERROR;
}

void BluetoothGattClientImpl::GetAllDevice(std::vector<BluetoothGattDevice>& device) {}

void BluetoothGattClientImpl::CompleteNotifyDescriptorWrite(int32_t appId, uint16_t descriptorHandle, int32_t ret)
{
    OHOS::sptr<IBluetoothGattClientCallback> callback = nullptr;
    std::shared_ptr<BluetoothGattDescriptor> descriptorCallback = nullptr;
    bool hasDescriptor = false;
    {
        std::lock_guard<std::mutex> lock(gattClientMutex_);
        auto callbackItem = gattClientCallBackMap_.find(appId);
        if (callbackItem == gattClientCallBackMap_.end()) {
            pendingNotifyDescriptorMap_.erase(appId);
            pendingNotifyResultMap_.erase(appId);
            return;
        }
        callback = callbackItem->second;
        if (callback == nullptr) {
            pendingNotifyDescriptorMap_.erase(appId);
            pendingNotifyResultMap_.erase(appId);
            return;
        }
        auto descriptorMapItem = pendingNotifyDescriptorMap_.find(appId);
        if (descriptorMapItem != pendingNotifyDescriptorMap_.end()) {
            auto descriptorItem = descriptorMapItem->second.find(descriptorHandle);
            if (descriptorItem != descriptorMapItem->second.end()) {
                descriptorCallback = std::make_shared<BluetoothGattDescriptor>(descriptorItem->second);
                hasDescriptor = true;
                descriptorMapItem->second.erase(descriptorItem);
                if (descriptorMapItem->second.empty()) {
                    pendingNotifyDescriptorMap_.erase(descriptorMapItem);
                }
            }
        }
        if (!hasDescriptor) {
            pendingNotifyResultMap_[appId][descriptorHandle] = ret;
        }
    }
    if (callback != nullptr && descriptorCallback != nullptr) {
        DispatchDescriptorWriteCallback(callback, ret, descriptorCallback);
    }
}

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
    CBCharacteristicProperties unsupportedProperties =
        CBCharacteristicPropertyNotifyEncryptionRequired | CBCharacteristicPropertyIndicateEncryptionRequired;
    cvc.properties_ = characteristic.properties & ~unsupportedProperties;
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
    AddClientCharacteristicConfigurationDescriptor(cvc, characteristic);
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

uint16_t BluetoothGattClientImpl::GetNotifyDescriptorHandle(
    const BluetoothGattService& service, uint16_t characterHandle)
{
    for (const auto& cvc : service.characteristics_) {
        if (cvc.handle_ != characterHandle) {
            continue;
        }
        for (const auto& desc : cvc.descriptors_) {
            if (IsClientCharacteristicConfigurationDescriptor(desc.uuid_)) {
                return desc.handle_;
            }
        }
        break;
    }
    return 0;
}

void BluetoothGattClientImpl::InsertPendingNotifyDescriptor(int32_t appId, uint16_t descriptorHandle)
{
    std::lock_guard<std::mutex> lock(gattClientMutex_);
    pendingNotifyDescriptorHandleMap_[appId].insert(descriptorHandle);
}

void BluetoothGattClientImpl::RemovePendingNotifyDescriptor(int32_t appId, uint16_t descriptorHandle)
{
    std::lock_guard<std::mutex> lock(gattClientMutex_);
    auto pendingItem = pendingNotifyDescriptorHandleMap_.find(appId);
    if (pendingItem == pendingNotifyDescriptorHandleMap_.end()) {
        return;
    }
    pendingItem->second.erase(descriptorHandle);
    if (pendingItem->second.empty()) {
        pendingNotifyDescriptorHandleMap_.erase(pendingItem);
    }
}

void BluetoothGattClientImpl::NotifyCharacteristicChanged(int32_t appId, const std::string& serviceUuid,
    const std::string& characterUuid, int length, const uint8_t* bytesValue)
{
    BluetoothGattCharacteristic gattCharacteristic;
    GetGattCharacteristic(serviceUuid, characterUuid, length, bytesValue, gattCharacteristic);
    OHOS::sptr<IBluetoothGattClientCallback> callback = GetCallBack(appId);
    if (callback == nullptr) {
        return;
    }
    callback->OnCharacteristicChanged(gattCharacteristic);
}

int BluetoothGattClientImpl::RequestNotification(int32_t appId, uint16_t characterHandle, bool enable)
{
    BluetoothGattService service = GetServiceWithCharacterHandle(characterHandle);
    if (service.endHandle_ <= 0) {
        HILOGE("service is null");
        return BT_ERR_INTERNAL_ERROR;
    }

    NSString* strServiceUuid = [NSString stringWithFormat:@"%s", service.uuid_.ToString().c_str()];
    CBUUID* serviceUuid = [CBUUID UUIDWithString:strServiceUuid];
    CBMutableCharacteristic* chara = GetClientCurrentCharacteristic(characterHandle, service);
    if (!serviceUuid || !chara) {
        HILOGE("UUID or characteristic is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    uint16_t notifyDescriptorHandle = GetNotifyDescriptorHandle(service, characterHandle);
    if (notifyDescriptorHandle == 0) {
        HILOGE("client characteristic configuration descriptor is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    InsertPendingNotifyDescriptor(appId, notifyDescriptorHandle);

    int ret = [[BluetoothCentralManager sharedInstance]
        RequestNotification:appId
                serviceUuid:serviceUuid
              characterUuid:chara.UUID
         enableNotification:enable
                      block:^(CBCharacteristic* chara, int32_t ret) {
                        if (ret != BT_NO_ERROR || chara == nil) {
                            return;
                        }
                        NSUInteger length = [chara.value length];
                        const uint8_t* bytesValue = (const uint8_t*)[chara.value bytes];
                        std::string serviceUuid = chara.service.UUID.UUIDString.UTF8String;
                        std::string characterUuid = chara.UUID.UUIDString.UTF8String;
                        NotifyCharacteristicChanged(appId, serviceUuid, characterUuid, length, bytesValue);
                      }
                 completion:^(int32_t ret) {
                   CompleteNotifyDescriptorWrite(appId, notifyDescriptorHandle, ret);
                 }];
    if (ret != BT_NO_ERROR) {
        RemovePendingNotifyDescriptor(appId, notifyDescriptorHandle);
    }
    return ret;
}

int BluetoothGattClientImpl::GetConnectedState(const std::string &deviceId, int &state)
{
    return BT_NO_ERROR;
}

int BluetoothGattClientImpl::SetPhy(int32_t appId, int32_t txPhy, int32_t rxPhy, int32_t phyOptions)
{
    return BT_NO_ERROR;
}

int BluetoothGattClientImpl::ReadPhy(int32_t appId)
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
                static_cast<int>(GattDisconnectReason::CONN_UNKNOWN), "");
            break;
        case CBPeripheralStateConnected:
            callback->OnConnectionStateChanged(ret, static_cast<int>(BTConnectState::CONNECTED),
                static_cast<int>(GattDisconnectReason::CONN_UNKNOWN), "");
            break;
        case CBPeripheralStateDisconnecting:
            callback->OnConnectionStateChanged(ret, static_cast<int>(BTConnectState::DISCONNECTING),
                static_cast<int>(GattDisconnectReason::CONN_UNKNOWN), "");
            break;
        default:
            callback->OnConnectionStateChanged(ret, static_cast<int>(BTConnectState::DISCONNECTED),
                static_cast<int>(GattDisconnectReason::CONN_UNKNOWN), "");
            break;
    }
}
} // namespace Bluetooth
} // namespace OHOS
