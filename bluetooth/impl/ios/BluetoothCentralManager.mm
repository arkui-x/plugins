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

#import "BluetoothCentralManager.h"

#include "bluetooth_def.h"

@implementation BluetoothCentralManager

static NSString* strYes = @"1";
static NSString* strNo = @"0";

+ (BluetoothCentralManager*)sharedInstance
{
    static BluetoothCentralManager* bluetoothCentralManager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
      bluetoothCentralManager = [[BluetoothCentralManager alloc] init];
      bluetoothCentralManager.centralManager = [[CBCentralManager alloc] initWithDelegate:bluetoothCentralManager
                                                                                     queue:nil
                                                                                   options:nil];
    });

    return bluetoothCentralManager;
}

- (NSMutableArray*)arrAutoConnectDivice
{
    if (!_arrAutoConnectDivice) {
        _arrAutoConnectDivice = [NSMutableArray array];
    }
    return _arrAutoConnectDivice;
}

- (NSMutableDictionary*)diviceIdDic
{
    if (!_diviceIdDic) {
        _diviceIdDic = [NSMutableDictionary dictionary];
    }
    return _diviceIdDic;
}

- (NSMutableDictionary*)servicesDic
{
    if (!_servicesDic) {
        _servicesDic = [NSMutableDictionary dictionary];
    }
    return _servicesDic;
}

- (NSMutableDictionary*)characteristicBlockDic
{
    if (!_characteristicBlockDic) {
        _characteristicBlockDic = [NSMutableDictionary dictionary];
    }
    return _characteristicBlockDic;
}

- (NSMutableDictionary*)getServicesStateDic
{
    if (!_getServicesStateDic) {
        _getServicesStateDic = [NSMutableDictionary dictionary];
    }
    return _getServicesStateDic;
}

- (int)startBLEScanWithId:(NSMutableArray<CBUUID*>*)arrUUID
{
    if (self.centralManager.state == CBManagerStatePoweredOn) {
        [self.centralManager scanForPeripheralsWithServices:arrUUID options:nil];
        return BT_NO_ERROR;
    } else {
        return BT_ERR_INTERNAL_ERROR;
    }
}

- (void)stopBLEScan
{
    [self.centralManager stopScan];
}

- (int)getBleState
{
    return bluetoothState;
}

- (bool)isBleEnabled
{
    if (bluetoothState == CBManagerStatePoweredOn) {
        return true;
    }
    return false;
}

- (CBPeripheral*)getDevice:(NSString*)address
{
    NSUUID* uuid = [[NSUUID alloc] initWithUUIDString:address];
    if (!uuid) {
        return nil;
    }
    NSArray<CBPeripheral*>* arrPeripheral = [self.centralManager retrievePeripheralsWithIdentifiers:@[ uuid ]];

    if (arrPeripheral.count == 0) {
        NSLog(@"Peripheral is nullptr");
        return nil;
    }
    CBPeripheral* peripheral = arrPeripheral.firstObject;
    return peripheral;
}

- (NSString*)getLocalName
{
    UIDevice* device = [[UIDevice alloc] init];
    return device.name;
}

#pragma mark - gatt client

- (void)registerDeviceId:(NSString*)strDeviceId withAppId:(int32_t)appId
{
    NSString* strAppId = [NSString stringWithFormat:@"%d", appId];
    [self.diviceIdDic setValue:strDeviceId forKey:strAppId];
}

- (void)closeClient:(int32_t)appId
{
    NSString* strAppId = [NSString stringWithFormat:@"%d", appId];
    currentPeripheral = [self createGattClientDevice:appId];
    if (currentPeripheral == nil) {
        return;
    }
    [self.centralManager cancelPeripheralConnection:currentPeripheral];
    [self.diviceIdDic removeObjectForKey:strAppId];
}

- (CBPeripheral*)createGattClientDevice:(int32_t)appId
{
    NSString* strAppId = [NSString stringWithFormat:@"%d", appId];
    NSString* strDeviceId = [NSString stringWithFormat:@"%@", self.diviceIdDic[strAppId]];
    NSUUID* uuid = [[NSUUID alloc] initWithUUIDString:strDeviceId];
    if (!uuid) {
        return nil;
    }
    NSArray<CBPeripheral*>* arrPeripheral = [self.centralManager retrievePeripheralsWithIdentifiers:@[ uuid ]];
    if (arrPeripheral.count == 0) {
        return nil;
    }
    return arrPeripheral.firstObject;
}

- (int)connect:(int32_t)appId autoConnect:(bool)autoConnect
{
    CBPeripheral* peripheral = [self createGattClientDevice:appId];

    if (peripheral == nil) {
        NSLog(@"Peripheral is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (autoConnect) {
        [self.arrAutoConnectDivice addObject:peripheral];
    }
    peripheral.delegate = self;
    [self.centralManager connectPeripheral:peripheral options:nil];
    currentPeripheral = peripheral;
    return BT_NO_ERROR;
}

- (int)disConnect:(int32_t)appId
{
    currentPeripheral = [self createGattClientDevice:appId];
    if (currentPeripheral == nil) {
        NSLog(@"Peripheral is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    [self.centralManager cancelPeripheralConnection:currentPeripheral];
    return BT_NO_ERROR;
}

- (void)onServicesDiscoveredCallBack:(CBPeripheral*)peripheral
{
    NSString* strCharacterKey = [NSString stringWithFormat:@"%@Character", peripheral.identifier.UUIDString];
    NSString* strIncludeServicesKey =
        [NSString stringWithFormat:@"%@IncludeServices", peripheral.identifier.UUIDString];
    NSString* strDescriptorsKey = [NSString stringWithFormat:@"%@Descriptors", peripheral.identifier.UUIDString];

    NSMutableDictionary* characterDic = self.getServicesStateDic[strCharacterKey];
    NSMutableDictionary* includeServicesDic = self.getServicesStateDic[strIncludeServicesKey];
    NSMutableDictionary* descriptorsDic = self.getServicesStateDic[strDescriptorsKey];

    if ([characterDic.allValues containsObject:strNo] || [includeServicesDic.allValues containsObject:strNo] ||
        [descriptorsDic.allValues containsObject:strNo]) {
        return;
    }
    [self.getServicesStateDic removeObjectForKey:peripheral.identifier.UUIDString];
    if (self.servicesDiscoveredBlock) {
        self.servicesDiscoveredBlock(BT_NO_ERROR);
    }
}

- (NSArray<CBService*>*)getServices:(int)appId
{
    currentPeripheral = [self createGattClientDevice:appId];
    return currentPeripheral.services;
}

- (int)discoverServices:(int)appId
{
    currentPeripheral = [self createGattClientDevice:appId];
    currentPeripheral.delegate = self;
    if (currentPeripheral.state == CBPeripheralStateConnected) {
        [currentPeripheral discoverServices:nil];
        return BT_NO_ERROR;
    } else {
        return BT_ERR_INTERNAL_ERROR;
    }
}

- (CBCharacteristic*)getCurrentCharacteristic:(NSString*)serviceUUID charaUUID:(NSString*)charaUUID
{
    CBCharacteristic* currentCharacter = nil;
    for (CBService* service in currentPeripheral.services) {
        if (![service.UUID.UUIDString isEqualToString:serviceUUID]) {
            continue;
        }
        for (CBCharacteristic* character in service.characteristics) {
            if ([character.UUID.UUIDString isEqualToString:charaUUID]) {
                currentCharacter = character;
                break;
            }
        }
    }
    return currentCharacter;
}

- (int)RequestNotification:(int)appId
               serviceUuid:(CBUUID*)serviceUuid
             characterUuid:(CBUUID*)characterUuid
        enableNotification:(bool)enableNotification
{
    currentPeripheral = [self createGattClientDevice:appId];
    currentPeripheral.delegate = self;
    CBCharacteristic* currentCharacter = [self getCurrentCharacteristic:serviceUuid.UUIDString
                                                              charaUUID:characterUuid.UUIDString];
    if (currentCharacter == nil || currentPeripheral.state != CBPeripheralStateConnected) {
        return BT_ERR_INTERNAL_ERROR;
    }

    if (currentCharacter.properties & CBCharacteristicPropertyNotify ||
        currentCharacter.properties & CBCharacteristicPropertyIndicate ||
        currentCharacter.properties & CBCharacteristicPropertyIndicateEncryptionRequired ||
        currentCharacter.properties & CBCharacteristicPropertyNotifyEncryptionRequired) {
        [currentPeripheral setNotifyValue:enableNotification forCharacteristic:currentCharacter];
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

#pragma mark - delegate
- (void)centralManagerDidUpdateState:(CBCentralManager*)central
{
    bluetoothState = central.state;
    if (self.updataBleStateBlock) {
        self.updataBleStateBlock(bluetoothState);
    }
}

- (void)centralManager:(CBCentralManager*)central
    didDiscoverPeripheral:(CBPeripheral*)peripheral
        advertisementData:(NSDictionary<NSString*, id>*)advertisementData
                     RSSI:(NSNumber*)RSSI
{
    if (self.scanDataBlock) {
        self.scanDataBlock(peripheral, advertisementData, RSSI);
    }
}

- (void)centralManager:(CBCentralManager*)central didConnectPeripheral:(CBPeripheral*)peripheral
{
    if (self.connectStateBlock) {
        self.connectStateBlock(BT_NO_ERROR, peripheral.state);
    }
}

- (void)centralManager:(CBCentralManager*)central
    didFailToConnectPeripheral:(CBPeripheral*)peripheral
                         error:(NSError*)error
{
    if (self.connectStateBlock) {
        self.connectStateBlock(BT_ERR_INTERNAL_ERROR, peripheral.state);
    }
}

- (void)centralManager:(CBCentralManager*)central
    didDisconnectPeripheral:(CBPeripheral*)peripheral
                      error:(NSError*)error
{
    if ([self.arrAutoConnectDivice containsObject:peripheral]) {
        [self.centralManager connectPeripheral:peripheral options:nil];
    }
    if (self.connectStateBlock) {
        self.connectStateBlock(BT_NO_ERROR, peripheral.state);
    }
}

- (void)peripheral:(CBPeripheral*)peripheral didDiscoverServices:(NSError*)error
{
    NSMutableDictionary* characterDic = [NSMutableDictionary dictionary];
    NSMutableDictionary* includeServicesDic = [NSMutableDictionary dictionary];

    for (CBService* service in peripheral.services) {
        [includeServicesDic setValue:strNo forKey:service.UUID.UUIDString];
        [characterDic setValue:strNo forKey:service.UUID.UUIDString];

        [peripheral discoverIncludedServices:nil forService:service];
        [peripheral discoverCharacteristics:nil forService:service];
    }
    NSString* strCharacterKey = [NSString stringWithFormat:@"%@Character", peripheral.identifier.UUIDString];
    NSString* strIncludeServicesKey =
        [NSString stringWithFormat:@"%@IncludeServices", peripheral.identifier.UUIDString];

    [self.getServicesStateDic setValue:characterDic forKey:strCharacterKey];
    [self.getServicesStateDic setValue:includeServicesDic forKey:strIncludeServicesKey];
}

- (void)peripheral:(CBPeripheral*)peripheral
    didDiscoverIncludedServicesForService:(CBService*)service
                                    error:(NSError*)error
{
    NSString* strCharacterKey = [NSString stringWithFormat:@"%@Character", peripheral.identifier.UUIDString];
    NSString* strIncludedServicesKey =
        [NSString stringWithFormat:@"%@IncludeServices", peripheral.identifier.UUIDString];
    NSMutableDictionary* includedServicesDic = self.getServicesStateDic[strIncludedServicesKey];
    NSMutableDictionary* characterDic = self.getServicesStateDic[strCharacterKey];

    [includedServicesDic setValue:strYes forKey:service.UUID.UUIDString];
    for (CBService* includeService in service.includedServices) {
        [includedServicesDic setValue:strNo forKey:includeService.UUID.UUIDString];
        [characterDic setValue:strNo forKey:includeService.UUID.UUIDString];

        [peripheral discoverIncludedServices:nil forService:includeService];
        [peripheral discoverCharacteristics:nil forService:includeService];
    }

    [self onServicesDiscoveredCallBack:peripheral];
}

- (void)peripheral:(CBPeripheral*)peripheral
    didDiscoverCharacteristicsForService:(CBService*)service
                                   error:(NSError*)error
{
    NSString* strCharacterKey = [NSString stringWithFormat:@"%@Character", peripheral.identifier.UUIDString];
    NSMutableDictionary* characterDic = self.getServicesStateDic[strCharacterKey];
    [characterDic setValue:strYes forKey:service.UUID.UUIDString];

    NSString* strDescriptorsKey = [NSString stringWithFormat:@"%@Descriptors", peripheral.identifier.UUIDString];
    NSMutableDictionary* descriptorsDic = [NSMutableDictionary dictionary];
    for (CBCharacteristic* characteristic in service.characteristics) {
        [peripheral discoverDescriptorsForCharacteristic:characteristic];
        [descriptorsDic setValue:strNo forKey:characteristic.UUID.UUIDString];
    }
    [self.getServicesStateDic setValue:descriptorsDic forKey:strDescriptorsKey];

    [self onServicesDiscoveredCallBack:peripheral];
}

- (void)peripheral:(CBPeripheral*)peripheral
    didDiscoverDescriptorsForCharacteristic:(CBCharacteristic*)characteristic
                                      error:(NSError*)error
{
    NSString* strDescriptorsKey = [NSString stringWithFormat:@"%@Descriptors", peripheral.identifier.UUIDString];
    NSMutableDictionary* descriptorsDic = self.getServicesStateDic[strDescriptorsKey];
    [descriptorsDic setValue:strYes forKey:characteristic.UUID.UUIDString];
    [self onServicesDiscoveredCallBack:peripheral];
}

- (void)peripheral:(CBPeripheral*)peripheral
    didUpdateValueForCharacteristic:(CBCharacteristic*)characteristic
                              error:(NSError*)error
{
    NSString* strKey = [NSString stringWithFormat:@"%@readCharacteristic", characteristic.UUID.UUIDString];
    ReadCharacterBlock readCharacterBlock = self.characteristicBlockDic[strKey];
    int state = error ? BT_ERR_INTERNAL_ERROR : BT_NO_ERROR;
    if (readCharacterBlock) {
        readCharacterBlock(characteristic, state);
    }
}

- (void)peripheral:(CBPeripheral*)peripheral
    didWriteValueForCharacteristic:(CBCharacteristic*)characteristic
                             error:(nullable NSError*)error
{
    NSString* strKey = [NSString stringWithFormat:@"%@writeCharacter", characteristic.UUID.UUIDString];

    WriteCharacterBlock writeCharacteristicValueBlock = self.characteristicBlockDic[strKey];
    if (writeCharacteristicValueBlock) {
        if (error) {
            writeCharacteristicValueBlock(characteristic, BT_ERR_INTERNAL_ERROR);
        } else {
            writeCharacteristicValueBlock(characteristic, BT_NO_ERROR);
        }
    }
}

- (void)peripheral:(CBPeripheral*)peripheral didUpdateValueForDescriptor:(CBDescriptor*)descriptor error:(NSError*)error
{
    NSString* strKey = [NSString stringWithFormat:@"%@readDescriptor", descriptor.UUID.UUIDString];
    ReadDescriptorBlock readDescriptorBlock = self.characteristicBlockDic[strKey];
    if (readDescriptorBlock) {
        if (error) {
            readDescriptorBlock(descriptor, BT_ERR_INTERNAL_ERROR);
        } else {
            readDescriptorBlock(descriptor, BT_NO_ERROR);
        }
    }
}

- (int)getReadCharacteristic:(int)appId
                 serviceUuid:(CBUUID*)serviceUuid
               characterUuid:(CBUUID*)characterUuid
                        data:(ReadCharacterBlock)dataBlock
{
    if (!dataBlock) {
        return BT_ERR_INTERNAL_ERROR;
    }
    NSString* strKey = [NSString stringWithFormat:@"%@readCharacteristic", characterUuid.UUIDString];
    [self.characteristicBlockDic setValue:dataBlock forKey:strKey];

    currentPeripheral = [self createGattClientDevice:appId];
    currentPeripheral.delegate = self;
    CBCharacteristic* currentCharacter = [self getCurrentCharacteristic:serviceUuid.UUIDString
                                                              charaUUID:characterUuid.UUIDString];

    if (currentCharacter == nil || currentPeripheral.state != CBPeripheralStateConnected) {
        return BT_ERR_INTERNAL_ERROR;
    }

    if (currentCharacter.properties & CBCharacteristicPropertyRead) {
        [currentPeripheral readValueForCharacteristic:currentCharacter];
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

- (int)WriteCharacterCallBackError:(WriteCharacterBlock)block
{
    if (block) {
        block(nil, BT_ERR_INTERNAL_ERROR);
    }
    return BT_ERR_INTERNAL_ERROR;
}

- (int)writeCharacteristic:(int)appId
               serviceUuid:(CBUUID*)serviceUuid
             characterUuid:(CBUUID*)characterUuid
                      data:(NSData*)data
             isOutResponse:(bool)isOutResponse
                  isSigned:(bool)isSigned
                     block:(WriteCharacterBlock)block
{
    if (currentPeripheral.state != CBPeripheralStateConnected || !block) {
        return BT_ERR_INTERNAL_ERROR;
    }
    NSString* strKey = [NSString stringWithFormat:@"%@writeCharacter", characterUuid.UUIDString];
    [self.characteristicBlockDic setValue:block forKey:strKey];
    currentPeripheral = [self createGattClientDevice:appId];
    currentPeripheral.delegate = self;
    CBCharacteristic* currentCharacter = [self getCurrentCharacteristic:serviceUuid.UUIDString
                                                              charaUUID:characterUuid.UUIDString];
    if (currentCharacter == nil || data == nil) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (isSigned) {
        if (currentCharacter.properties & CBCharacteristicPropertyAuthenticatedSignedWrites) {
            [currentPeripheral writeValue:data
                        forCharacteristic:currentCharacter
                                     type:CBCharacteristicWriteWithoutResponse];
        } else {
            return BT_ERR_INTERNAL_ERROR;
        }
    } else if (isOutResponse) {
        if (currentCharacter.properties & CBCharacteristicPropertyWriteWithoutResponse) {
            [currentPeripheral writeValue:data
                        forCharacteristic:currentCharacter
                                     type:CBCharacteristicWriteWithoutResponse];
        } else {
            return BT_ERR_INTERNAL_ERROR;
        }
    } else {
        if (currentCharacter.properties & CBCharacteristicPropertyWrite) {
            [currentPeripheral writeValue:data
                        forCharacteristic:currentCharacter
                                     type:CBCharacteristicWriteWithResponse];
        } else {
            return BT_ERR_INTERNAL_ERROR;
        }
    }
    return BT_NO_ERROR;
}

- (int)getReadDescriptor:(int)appId
             serviceUuid:(CBUUID*)serviceUuid
           characterUuid:(CBUUID*)characterUuid
          descriptorUuid:(CBUUID*)descriptorUuid
                    data:(ReadDescriptorBlock)dataBlock
{
    if (currentPeripheral.state != CBPeripheralStateConnected || !dataBlock) {
        return BT_ERR_INTERNAL_ERROR;
    }
    NSString* strKey = [NSString stringWithFormat:@"%@readDescriptor", descriptorUuid.UUIDString];
    [self.characteristicBlockDic setValue:dataBlock forKey:strKey];
    CBDescriptor* currentDescriptor = nil;
    currentPeripheral = [self createGattClientDevice:appId];
    currentPeripheral.delegate = self;
    CBCharacteristic* currentCharacter = [self getCurrentCharacteristic:serviceUuid.UUIDString
                                                              charaUUID:characterUuid.UUIDString];

    for (CBDescriptor* des in currentCharacter.descriptors) {
        if ([descriptorUuid.UUIDString isEqualToString:des.UUID.UUIDString]) {
            currentDescriptor = des;
            break;
        }
    }
    if (currentDescriptor == nil) {
        return BT_ERR_INTERNAL_ERROR;
    }
    [currentPeripheral readValueForDescriptor:currentDescriptor];
    return BT_NO_ERROR;
}

@end
