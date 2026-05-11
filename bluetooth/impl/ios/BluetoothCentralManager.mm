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

#import "BluetoothCentralManager.h"
#import "BluetoothUntils.h"

#include "bluetooth_def.h"

#ifndef dispatch_main_async_safe
#define dispatch_main_async_safe(block) \
if (dispatch_queue_get_label(DISPATCH_CURRENT_QUEUE_LABEL) == dispatch_queue_get_label(dispatch_get_main_queue())) { \
    block(); \
} else { \
    dispatch_async(dispatch_get_main_queue(), block); \
}
#endif

@interface BluetoothCentralManager ()

- (void)setCharacteristicBlock:(id)block forKey:(NSString*)strKey;
- (id)characteristicBlockForKey:(NSString*)strKey;
- (void)removeCharacteristicBlockForKey:(NSString*)strKey;
- (void)cacheNotifyBlock:(NotifyCharacterBlock)block enable:(BOOL)enable key:(NSString*)strKey;
- (NotifyCharacterBlock)notifyBlockForKey:(NSString*)strKey;
- (int)requestNotifyStateForPeripheral:(CBPeripheral*)peripheral
                        characteristic:(CBCharacteristic*)characteristic
                                enable:(BOOL)enable
                                   key:(NSString*)strKey
                                 block:(NotifyCharacterBlock)block
                            completion:(NotifyStateCompletionBlock)completion;

@end

@implementation BluetoothCentralManager

static NSString* strYes = @"1";
static NSString* strNo = @"0";
static const int NOTIFY_STATE_TIMEOUT_SECONDS = 5;

static NSString* BuildNotifyCharacteristicKey(NSString* serviceUuid, NSString* characterUuid)
{
    return [NSString stringWithFormat:@"%@%@notifyCharacteristic",
        [BluetoothUntils NormalizeBluetoothUuid:serviceUuid], [BluetoothUntils NormalizeBluetoothUuid:characterUuid]];
}

static NSString* BuildDescriptorOperationKey(
    NSString* serviceUuid, NSString* characterUuid, NSString* descriptorUuid, NSString* operation)
{
    return [NSString stringWithFormat:@"%@%@%@%@",
        [BluetoothUntils NormalizeBluetoothUuid:serviceUuid], [BluetoothUntils NormalizeBluetoothUuid:characterUuid],
        [BluetoothUntils NormalizeBluetoothUuid:descriptorUuid], operation];
}

+ (BluetoothCentralManager*)sharedInstance
{
    static BluetoothCentralManager* bluetoothCentralManager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
      bluetoothCentralManager = [[BluetoothCentralManager alloc] init];
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

- (NSMutableDictionary*)notifyStateBlockDic
{
    if (!_notifyStateBlockDic) {
        _notifyStateBlockDic = [NSMutableDictionary dictionary];
    }
    return _notifyStateBlockDic;
}

- (NSMutableDictionary*)getServicesStateDic
{
    if (!_getServicesStateDic) {
        _getServicesStateDic = [NSMutableDictionary dictionary];
    }
    return _getServicesStateDic;
}

- (NSMutableSet*)notifyCharacteristicSet
{
    if (!_notifyCharacteristicSet) {
        _notifyCharacteristicSet = [NSMutableSet set];
    }
    return _notifyCharacteristicSet;
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
    if (self.centralManager) {
        return bluetoothState;
    }
    return 0;
}

- (CBCentralManager *)centralManager {
    if (!_centralManager) {
        dispatch_main_async_safe(^{
            _centralManager = [[CBCentralManager alloc] initWithDelegate:self queue:nil];
        });
    }
    return _centralManager;
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
        if (![BluetoothUntils IsSameBluetoothUuid:service.UUID.UUIDString right:serviceUUID]) {
            continue;
        }
        for (CBCharacteristic* character in service.characteristics) {
            if ([BluetoothUntils IsSameBluetoothUuid:character.UUID.UUIDString right:charaUUID]) {
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
                     block:(NotifyCharacterBlock)block
                completion:(NotifyStateCompletionBlock)completion
{
    currentPeripheral = [self createGattClientDevice:appId];
    if (currentPeripheral == nil) {
        return BT_ERR_DEVICE_DISCONNECTED;
    }
    currentPeripheral.delegate = self;
    CBCharacteristic* currentCharacter = [self getCurrentCharacteristic:serviceUuid.UUIDString
                                                              charaUUID:characterUuid.UUIDString];
    if (currentPeripheral.state != CBPeripheralStateConnected) {
        return BT_ERR_DEVICE_DISCONNECTED;
    }
    if (currentCharacter == nil) {
        return BT_ERR_GATT_SERVICE_NOT_FOUND;
    }

    NSString* strKey = BuildNotifyCharacteristicKey(serviceUuid.UUIDString, characterUuid.UUIDString);
    BOOL supportNotify = ((currentCharacter.properties & CBCharacteristicPropertyNotify) != 0) ||
        ((currentCharacter.properties & CBCharacteristicPropertyNotifyEncryptionRequired) != 0);
    BOOL supportIndicate = ((currentCharacter.properties & CBCharacteristicPropertyIndicate) != 0) ||
        ((currentCharacter.properties & CBCharacteristicPropertyIndicateEncryptionRequired) != 0);
    if (!supportNotify && !supportIndicate) {
        return BT_ERR_API_NOT_SUPPORT;
    }
    if (currentCharacter.isNotifying == enableNotification) {
        [self cacheNotifyBlock:block enable:enableNotification key:strKey];
        if (completion) {
            completion(BT_NO_ERROR);
        }
        return BT_NO_ERROR;
    }
    return [self requestNotifyStateForPeripheral:currentPeripheral
                                  characteristic:currentCharacter
                                          enable:enableNotification
                                             key:strKey
                                           block:block
                                      completion:completion];
}

- (void)setCharacteristicBlock:(id)block forKey:(NSString*)strKey
{
    if (block == nil || strKey.length == 0) {
        return;
    }
    @synchronized(self) {
        [self.characteristicBlockDic setObject:[block copy] forKey:strKey];
    }
}

- (id)characteristicBlockForKey:(NSString*)strKey
{
    @synchronized(self) {
        return self.characteristicBlockDic[strKey];
    }
}

- (void)removeCharacteristicBlockForKey:(NSString*)strKey
{
    @synchronized(self) {
        [self.characteristicBlockDic removeObjectForKey:strKey];
    }
}

- (void)cacheNotifyBlock:(NotifyCharacterBlock)block enable:(BOOL)enable key:(NSString*)strKey
{
    @synchronized(self) {
        if (enable) {
            if (block) {
                [self.characteristicBlockDic setObject:[block copy] forKey:strKey];
            }
            [self.notifyCharacteristicSet addObject:strKey];
            return;
        }
        [self.characteristicBlockDic removeObjectForKey:strKey];
        [self.notifyCharacteristicSet removeObject:strKey];
    }
}

- (NotifyCharacterBlock)notifyBlockForKey:(NSString*)strKey
{
    @synchronized(self) {
        if (![self.notifyCharacteristicSet containsObject:strKey]) {
            return nil;
        }
        return self.characteristicBlockDic[strKey];
    }
}

- (int)requestNotifyStateForPeripheral:(CBPeripheral*)peripheral
                        characteristic:(CBCharacteristic*)characteristic
                                enable:(BOOL)enable
                                   key:(NSString*)strKey
                                 block:(NotifyCharacterBlock)block
                            completion:(NotifyStateCompletionBlock)completion
{
    __block int notifyRet = BT_ERR_INTERNAL_ERROR;
    BOOL isMainThreadRequest = [NSThread isMainThread];
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    NotifyStateBlock stateBlock = ^(CBCharacteristic* chara, int32_t ret) {
        if (ret == BT_NO_ERROR && chara.isNotifying != enable) {
            notifyRet = BT_ERR_INTERNAL_ERROR;
        } else {
            notifyRet = ret;
        }
        if (notifyRet == BT_NO_ERROR) {
            [self cacheNotifyBlock:block enable:enable key:strKey];
        }
        if (completion && (isMainThreadRequest || notifyRet == BT_NO_ERROR)) {
            completion(notifyRet);
        }
        dispatch_semaphore_signal(semaphore);
    };
    @synchronized(self.notifyStateBlockDic) {
        [self.notifyStateBlockDic setObject:[stateBlock copy] forKey:strKey];
    }
    dispatch_main_async_safe(^{
        [peripheral setNotifyValue:enable forCharacteristic:characteristic];
    });
    if (![NSThread isMainThread]) {
        dispatch_time_t timeout = dispatch_time(DISPATCH_TIME_NOW, NOTIFY_STATE_TIMEOUT_SECONDS * NSEC_PER_SEC);
        if (dispatch_semaphore_wait(semaphore, timeout) != 0) {
            @synchronized(self.notifyStateBlockDic) {
                [self.notifyStateBlockDic removeObjectForKey:strKey];
            }
            return BT_ERR_INTERNAL_ERROR;
        }
    } else {
        // CoreBluetooth reports the real notification result asynchronously on the main queue.
        notifyRet = BT_NO_ERROR;
    }
    return notifyRet;
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
    ReadCharacterBlock readCharacterBlock = (ReadCharacterBlock)[self characteristicBlockForKey:strKey];
    int state = error ? BT_ERR_INTERNAL_ERROR : BT_NO_ERROR;
    if (readCharacterBlock) {
        readCharacterBlock(characteristic, state);
    }

    NSString* notifyKey = BuildNotifyCharacteristicKey(
        characteristic.service.UUID.UUIDString, characteristic.UUID.UUIDString);
    NotifyCharacterBlock notifyCharacterBlock = [self notifyBlockForKey:notifyKey];
    if (notifyCharacterBlock) {
        notifyCharacterBlock(characteristic, state);
    }
}

- (void)peripheral:(CBPeripheral*)peripheral
    didUpdateNotificationStateForCharacteristic:(CBCharacteristic*)characteristic
                                          error:(NSError*)error
{
    NSString* notifyKey = BuildNotifyCharacteristicKey(
        characteristic.service.UUID.UUIDString, characteristic.UUID.UUIDString);
    NotifyStateBlock notifyStateBlock = nil;
    @synchronized(self.notifyStateBlockDic) {
        notifyStateBlock = self.notifyStateBlockDic[notifyKey];
        if (notifyStateBlock) {
            [self.notifyStateBlockDic removeObjectForKey:notifyKey];
        }
    }
    if (!notifyStateBlock) {
        return;
    }
    int state = error ? BT_ERR_INTERNAL_ERROR : BT_NO_ERROR;
    notifyStateBlock(characteristic, state);
}

- (void)peripheral:(CBPeripheral*)peripheral
    didWriteValueForCharacteristic:(CBCharacteristic*)characteristic
                             error:(nullable NSError*)error
{
    NSString* strKey = [NSString stringWithFormat:@"%@writeCharacter", characteristic.UUID.UUIDString];

    WriteCharacterBlock writeCharacteristicValueBlock = (WriteCharacterBlock)[self characteristicBlockForKey:strKey];
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
    ReadDescriptorBlock readDescriptorBlock = (ReadDescriptorBlock)[self characteristicBlockForKey:strKey];
    if (readDescriptorBlock) {
        if (error) {
            readDescriptorBlock(descriptor, BT_ERR_INTERNAL_ERROR);
        } else {
            readDescriptorBlock(descriptor, BT_NO_ERROR);
        }
    }
}

- (void)peripheral:(CBPeripheral*)peripheral didWriteValueForDescriptor:(CBDescriptor*)descriptor error:(NSError*)error
{
    NSString* strKey = BuildDescriptorOperationKey(descriptor.characteristic.service.UUID.UUIDString,
        descriptor.characteristic.UUID.UUIDString, descriptor.UUID.UUIDString, @"writeDescriptor");
    WriteDescriptorBlock writeDescriptorBlock = (WriteDescriptorBlock)[self characteristicBlockForKey:strKey];
    if (!writeDescriptorBlock) {
        return;
    }
    writeDescriptorBlock(descriptor, error ? BT_ERR_INTERNAL_ERROR : BT_NO_ERROR);
    [self removeCharacteristicBlockForKey:strKey];
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
    [self setCharacteristicBlock:dataBlock forKey:strKey];

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
    [self setCharacteristicBlock:block forKey:strKey];
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
    [self setCharacteristicBlock:dataBlock forKey:strKey];
    CBDescriptor* currentDescriptor = nil;
    currentPeripheral = [self createGattClientDevice:appId];
    currentPeripheral.delegate = self;
    CBCharacteristic* currentCharacter = [self getCurrentCharacteristic:serviceUuid.UUIDString
                                                              charaUUID:characterUuid.UUIDString];

    for (CBDescriptor* des in currentCharacter.descriptors) {
        if ([BluetoothUntils IsSameBluetoothUuid:descriptorUuid.UUIDString right:des.UUID.UUIDString]) {
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

- (int)writeDescriptor:(int)appId
           serviceUuid:(CBUUID*)serviceUuid
         characterUuid:(CBUUID*)characterUuid
        descriptorUuid:(CBUUID*)descriptorUuid
                  data:(NSData*)data
                 block:(WriteDescriptorBlock)block
{
    currentPeripheral = [self createGattClientDevice:appId];
    if (currentPeripheral == nil || currentPeripheral.state != CBPeripheralStateConnected || data == nil || !block) {
        return BT_ERR_INTERNAL_ERROR;
    }
    currentPeripheral.delegate = self;
    CBCharacteristic* currentCharacter = [self getCurrentCharacteristic:serviceUuid.UUIDString
                                                              charaUUID:characterUuid.UUIDString];
    if (currentCharacter == nil) {
        return BT_ERR_INTERNAL_ERROR;
    }
    CBDescriptor* currentDescriptor = nil;
    for (CBDescriptor* descriptor in currentCharacter.descriptors) {
        if ([BluetoothUntils IsSameBluetoothUuid:descriptor.UUID.UUIDString right:descriptorUuid.UUIDString]) {
            currentDescriptor = descriptor;
            break;
        }
    }
    if (currentDescriptor == nil) {
        return BT_ERR_INTERNAL_ERROR;
    }
    NSString* strKey = BuildDescriptorOperationKey(serviceUuid.UUIDString,
        characterUuid.UUIDString, descriptorUuid.UUIDString, @"writeDescriptor");
    [self setCharacteristicBlock:block forKey:strKey];
    [currentPeripheral writeValue:data forDescriptor:currentDescriptor];
    return BT_NO_ERROR;
}

@end
