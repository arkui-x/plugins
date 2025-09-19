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

#import "BluetoothPeripheralManager.h"

#ifndef dispatch_main_async_safe
#define dispatch_main_async_safe(block) \
if (dispatch_queue_get_label(DISPATCH_CURRENT_QUEUE_LABEL) == dispatch_queue_get_label(dispatch_get_main_queue())) { \
    block(); \
} else { \
    dispatch_async(dispatch_get_main_queue(), block); \
}
#endif

@implementation BluetoothPeripheralManager

typedef NS_ENUM(NSInteger, CharacterSubscribe) {
    Subscribe = 0,
    UnSubscribe,
};

- (NSMutableDictionary*)requestDic
{
    if (_requestDic == nil) {
        _requestDic = [NSMutableDictionary dictionary];
    }
    return _requestDic;
}

- (NSMutableDictionary*)blockDic
{
    if (_blockDic == nil) {
        _blockDic = [NSMutableDictionary dictionary];
    }
    return _blockDic;
}

- (NSMutableDictionary*)servicesDic
{
    if (_servicesDic == nil) {
        _servicesDic = [NSMutableDictionary dictionary];
    }
    return _servicesDic;
}

- (NSMutableDictionary*)centralsDic
{
    if (_centralsDic == nil) {
        _centralsDic = [NSMutableDictionary dictionary];
    }
    return _centralsDic;
}

+ (BluetoothPeripheralManager*)sharedInstance
{
    static BluetoothPeripheralManager* bluetoothPeripheralManager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
      bluetoothPeripheralManager = [[BluetoothPeripheralManager alloc] init];
      bluetoothPeripheralManager.appId = 0;
    });
    return bluetoothPeripheralManager;
}

- (CBPeripheralManager *)peripheralManager {
    if (_peripheralManager == nil) {
        dispatch_main_async_safe(^{
            _peripheralManager = [[CBPeripheralManager alloc] initWithDelegate:self queue:nil options:nil];
        });
    }
    return _peripheralManager;
}

- (void)createPeripheralManager
{
    if (self.peripheralManager == nil) {
        self.peripheralManager = [[CBPeripheralManager alloc] initWithDelegate:self queue:nil options:nil];
    }
}

- (int)getAppId
{
    self.appId += 1;
    return self.appId;
}

- (bool)isBleEnabled
{
    return bluetoothState;
}

- (void)closePeripheral:(NSString*)strAppId
{
    [self removeDic:self.servicesDic appId:strAppId];
    [self removeDic:self.blockDic appId:strAppId];
}

- (void)removeDic:(NSMutableDictionary*)dic appId:(NSString*)strAppId
{
    NSString* strRmKey = [NSString stringWithFormat:@"%@:", strAppId];
    NSMutableArray* arrKeys = [NSMutableArray array];
    for (NSString* strkey in dic.allKeys) {
        if ([strkey containsString:strRmKey]) {
            [arrKeys addObject:strkey];
        }
    }
    for (NSString* strkey in arrKeys) {
        if (dic == self.servicesDic) {
            CBMutableService* service = dic[strkey];
            [self.peripheralManager removeService:service];
        }
        [dic removeObjectForKey:strkey];
    }
}

- (int)startAdvertising:(NSString*)strAppId
          isConnectable:(bool)isConnectable
           servicesUUID:(NSMutableArray*)serviceUUIDs
            serviceData:(NSMutableDictionary*)serviceData
      includeDeviceName:(bool)includeDeviceName
               duration:(int)duration
{
    NSNumber* isConnectableNumber = [NSNumber numberWithBool:isConnectable];

    NSMutableDictionary* advertisementData = [NSMutableDictionary dictionary];

    [advertisementData setObject:isConnectableNumber forKey:CBAdvertisementDataIsConnectable];
    [advertisementData setObject:serviceUUIDs forKey:CBAdvertisementDataServiceUUIDsKey];
    [advertisementData setObject:serviceData forKey:CBAdvertisementDataServiceDataKey];

    if (includeDeviceName) {
        UIDevice* device = [[UIDevice alloc] init];
        [advertisementData setObject:device.name forKey:CBAdvertisementDataLocalNameKey];
    }
    [self.peripheralManager startAdvertising:advertisementData];
    return BT_NO_ERROR;
}

- (int)stopAdvertising:(NSString*)strAppId
{
    [self.peripheralManager stopAdvertising];
    return BT_NO_ERROR;
}

- (int)addService:(CBMutableService*)service appId:(int)appId block:(AddServiceBlock)block
{
    NSString* strAppId = [NSString stringWithFormat:@"%d", appId];
    NSString* strKey = [NSString stringWithFormat:@"%@:%@", strAppId, service.UUID.UUIDString];
    [self.blockDic setObject:block forKey:strKey];
    [self.servicesDic setObject:service forKey:strKey];
    [self.peripheralManager addService:service];
    return BT_NO_ERROR;
}

- (CBMutableService*)getCurrentService:(NSString*)strKey
{
    for (NSString* strServiceKey in self.servicesDic.allKeys) {
        if ([strServiceKey containsString:strKey]) {
            return self.servicesDic[strServiceKey];
        }
    }
    return nil;
}

- (int)removeService:(NSString*)strUUID appId:(int)appId
{
    NSString* strAppId = [NSString stringWithFormat:@"%d", appId];
    NSString* strKey = [NSString stringWithFormat:@"%@:%@", strAppId, strUUID];
    if ([self.servicesDic.allKeys containsObject:strKey]) {
        CBMutableService* service = self.servicesDic[strKey];
        [self.peripheralManager removeService:service];
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

- (int)notifyCharacteristicChanged:(NSString*)strDeviceId
                       serviceUUID:(NSString*)serviceUUID
              notifyCharacteristic:(CBMutableCharacteristic*)notifyCharacteristic
{
    int ret = BT_ERR_INTERNAL_ERROR;
    NSString* strKey = [NSString stringWithFormat:@"%@%@", serviceUUID, notifyCharacteristic.UUID.UUIDString];
    CBMutableService* service = [self getCurrentService:serviceUUID];
    if (service == nil) {
        return ret;
    }
    for (int i = 0; i < service.characteristics.count; i++) {
        CBMutableCharacteristic* character = service.characteristics[i];
        if (![character.UUID.UUIDString isEqualToString:notifyCharacteristic.UUID.UUIDString]) {
            continue;
        }
        NSMutableArray* arrCenral = [NSMutableArray arrayWithArray:self.centralsDic[strKey]];
        if (arrCenral.count == 0 || !character.value) {
            continue;
        }
        [self.peripheralManager updateValue:character.value 
                          forCharacteristic:character 
                        onSubscribedCentrals:arrCenral];
        for (CBCentral* central in arrCenral) {
             [self notifyBlockRequest:central characteristic:character state:Subscribe];
        }
        ret = BT_NO_ERROR;
        break;
    }
    return ret;
}

- (int)sendRespondReadWithDeviceId:(NSString*)deviceId
                       serviceUUID:(NSString*)serviceUUID
                     characterUUID:(NSString*)characterUUID
                              data:(NSData*)data
{
    if (data == nil) {
        return BT_ERR_INTERNAL_ERROR;
    }
    NSString* strKey = [NSString stringWithFormat:@"%@%@%@Read", deviceId, serviceUUID, characterUUID];
    if (![self.requestDic.allKeys containsObject:strKey]) {
        return BT_ERR_INTERNAL_ERROR;
    }
    CBATTRequest* request = self.requestDic[strKey];
    request.value = data;
    [self sendRespond:strKey serviceUUID:serviceUUID characterUUID:characterUUID];
    return BT_NO_ERROR;
}

- (int)sendRespondWriteWithDeviceId:(NSString*)deviceId
                        serviceUUID:(NSString*)serviceUUID
                      characterUUID:(NSString*)characterUUID
{
    NSString* strKey = [NSString stringWithFormat:@"%@%@%@Write", deviceId, serviceUUID, characterUUID];
    if (![self.requestDic.allKeys containsObject:strKey]) {
        return BT_ERR_INTERNAL_ERROR;
    }
    [self sendRespond:strKey serviceUUID:serviceUUID characterUUID:characterUUID];
    return BT_NO_ERROR;
}

- (void)sendRespond:(NSString*)strKey serviceUUID:(NSString*)serviceUUID characterUUID:(NSString*)characterUUID
{
    CBATTError state = CBATTErrorRequestNotSupported;
    CBMutableCharacteristic* character = [self getCurrentCharacteristic:serviceUUID charaUUID:characterUUID];
    if (character) {
        state = CBATTErrorSuccess;
    }
    CBATTRequest* request = self.requestDic[strKey];
    [self.peripheralManager respondToRequest:request withResult:state];

    [self.requestDic removeObjectForKey:strKey];
}

#pragma mark delegate
- (void)peripheralManagerDidUpdateState:(CBPeripheralManager*)peripheral
{
    bluetoothState = false;
    if (peripheral.state == CBManagerStatePoweredOn) {
        bluetoothState = true;
    }
}

- (void)peripheralManagerDidStartAdvertising:(CBPeripheralManager*)peripheral error:(NSError*)error
{
    if (self.startAdvertisingBlock) {
        if (error) {
            self.startAdvertisingBlock(peripheral.isAdvertising, BT_ERR_INTERNAL_ERROR);
        } else {
            self.startAdvertisingBlock(peripheral.isAdvertising, BT_NO_ERROR);
        }
    }
}

- (void)peripheralManager:(CBPeripheralManager*)peripheral didAddService:(CBService*)service error:(NSError*)error
{
    int ret = error ? BT_ERR_INTERNAL_ERROR : BT_NO_ERROR;
    NSString* strKey = [NSString stringWithFormat:@"%@", service.UUID.UUIDString];
    for (NSString* strItem in self.blockDic.allKeys) {
        if ([strItem containsString:strKey]) {
            strKey = strItem;
            break;
        }
    }
    AddServiceBlock addServiceBlock = [self.blockDic objectForKey:strKey];
    if (addServiceBlock) {
        addServiceBlock(ret);
    }
}

- (void)peripheralManager:(CBPeripheralManager*)peripheral
                         central:(CBCentral*)central
    didSubscribeToCharacteristic:(CBCharacteristic*)characteristic
{
    NSString* strKey =
        [NSString stringWithFormat:@"%@%@", characteristic.service.UUID.UUIDString, characteristic.UUID.UUIDString];
    NSMutableArray* arrCentrals = [NSMutableArray arrayWithArray:self.centralsDic[strKey]];
    [arrCentrals addObject:central];
    [self.centralsDic setObject:arrCentrals forKey:strKey];
    [self connectDeviceCallBack:central peripheralState:CBPeripheralStateConnected];
    [self notifyBlockRequest:central characteristic:characteristic state:Subscribe];
}

- (void)peripheralManager:(CBPeripheralManager*)peripheral
                             central:(CBCentral*)central
    didUnsubscribeFromCharacteristic:(CBCharacteristic*)characteristic
{
    [self connectDeviceCallBack:central peripheralState:CBPeripheralStateDisconnected];
    [self notifyBlockRequest:central characteristic:characteristic state:UnSubscribe];

    NSString* strKey =
        [NSString stringWithFormat:@"%@%@", characteristic.service.UUID.UUIDString, characteristic.UUID.UUIDString];

    NSMutableArray* arrCentrals = [NSMutableArray arrayWithArray:self.centralsDic[strKey]];
    NSIndexSet* indexSet =
        [arrCentrals indexesOfObjectsPassingTest:^BOOL(CBCentral* _Nonnull obj, NSUInteger idx, BOOL* _Nonnull stop) {
          return [obj.identifier isEqual:central.identifier];
        }];
    if (arrCentrals.count == 0 || indexSet.count == 0) {
        return;
    }
    [arrCentrals removeObjectsAtIndexes:indexSet];
    [self.centralsDic setObject:arrCentrals forKey:strKey];
}

- (void)peripheralManager:(CBPeripheralManager*)peripheral didReceiveReadRequest:(CBATTRequest*)request
{
    NSString* strKey =
        [NSString stringWithFormat:@"%@%@%@Read", request.central.identifier.UUIDString,
                  request.characteristic.service.UUID.UUIDString, request.characteristic.UUID.UUIDString];
    [self.requestDic setObject:request forKey:strKey];
    [self connectDeviceCallBack:request.central peripheralState:CBPeripheralStateConnected];

    CBMutableCharacteristic* character = [self getCurrentCharacteristic:request.characteristic.service.UUID.UUIDString
                                                              charaUUID:request.characteristic.UUID.UUIDString];
    if (character) {
        request.value = character.value;
        if (self.characterReadBlock) {
            self.characterReadBlock(request.central.identifier.UUIDString, character);
        }
    }
}

- (void)peripheralManager:(CBPeripheralManager*)peripheral didReceiveWriteRequests:(NSArray<CBATTRequest*>*)requests
{
    for (CBATTRequest* request in requests) {
        NSString* strKey =
            [NSString stringWithFormat:@"%@%@%@Write", request.central.identifier.UUIDString,
                      request.characteristic.service.UUID.UUIDString, request.characteristic.UUID.UUIDString];
        [self.requestDic setObject:request forKey:strKey];
        [self connectDeviceCallBack:request.central peripheralState:CBPeripheralStateConnected];

        CBMutableCharacteristic* character =
            [self getCurrentCharacteristic:request.characteristic.service.UUID.UUIDString
                                 charaUUID:request.characteristic.UUID.UUIDString];
        if (character) {
            character.value = request.value;
            [self writeBlockRequest:request characteristic:character];
        }
    }
}

- (void)connectDeviceCallBack:(CBCentral*)central peripheralState:(CBPeripheralState)peripheralState
{
    if (self.connectDeviceBlock) {
        self.connectDeviceBlock(peripheralState, central.identifier.UUIDString);
    }
}

- (void)writeBlockRequest:(CBATTRequest*)request characteristic:(CBMutableCharacteristic*)character
{
    if (self.characterWriteBlock) {
        self.characterWriteBlock(request.central.identifier.UUIDString, character);
    }
}

- (void)notifyBlockRequest:(CBCentral*)central characteristic:(CBCharacteristic*)character state:(int)state
{
    if (self.characterNotifyBlock) {
        self.characterNotifyBlock(central.identifier.UUIDString, character, state);
    }
}

- (CBMutableCharacteristic*)getCurrentCharacteristic:(NSString*)serviceUUID charaUUID:(NSString*)charaUUID
{
    CBMutableService* service = [self getCurrentService:serviceUUID];
    if (service == nil) {
        return nil;
    }

    CBMutableCharacteristic* currentCharacter = nil;

    for (CBMutableCharacteristic* character in service.characteristics) {
        if ([character.UUID.UUIDString isEqualToString:charaUUID]) {
            currentCharacter = character;
            break;
        }
    }

    return currentCharacter;
}

@end
