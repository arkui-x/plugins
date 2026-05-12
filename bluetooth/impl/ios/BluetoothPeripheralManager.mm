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

#import "BluetoothPeripheralManager.h"
#import "BluetoothUntils.h"

#ifndef dispatch_main_async_safe
#define dispatch_main_async_safe(block) \
if (dispatch_queue_get_label(DISPATCH_CURRENT_QUEUE_LABEL) == dispatch_queue_get_label(dispatch_get_main_queue())) { \
    block(); \
} else { \
    dispatch_async(dispatch_get_main_queue(), block); \
}
#endif

@interface BluetoothPeripheralManager ()

@property(nonatomic, strong) NSMutableSet* connectedCentralSet;

- (void)reportCentralConnectedIfNeeded:(CBCentral*)central appId:(NSString*)strAppId;
- (BOOL)hasSubscribedCentral:(CBCentral*)central appId:(NSString*)strAppId;
- (CBMutableService*)getCurrentService:(NSString*)strKey appId:(NSString*)strAppId;
- (CBMutableCharacteristic*)getCurrentCharacteristic:(NSString*)serviceUUID
                                           charaUUID:(NSString*)charaUUID
                                               appId:(NSString*)strAppId;
- (NSString*)appIdForService:(CBService*)service;
- (NSData*)clientConfigurationValueForCharacteristic:(CBCharacteristic*)characteristic subscribed:(BOOL)subscribed;
- (void)descriptorWriteBlockRequest:(CBCentral*)central
                      characteristic:(CBCharacteristic*)character
                               value:(NSData*)value;

@end

@implementation BluetoothPeripheralManager

static const uint8_t CLIENT_CONFIG_DISABLE_VALUE = 0x00;
static const uint8_t CLIENT_CONFIG_NOTIFY_VALUE = 0x01;
static const uint8_t CLIENT_CONFIG_INDICATE_VALUE = 0x02;
static const NSUInteger CLIENT_CONFIG_VALUE_LENGTH = 2;
static NSString* const REQUEST_TYPE_READ = @"Read";
static NSString* const REQUEST_TYPE_WRITE = @"Write";

static NSString* BuildCharacteristicKey(NSString* serviceUuid, NSString* characterUuid)
{
    return [NSString stringWithFormat:@"%@:%@",
        [BluetoothUntils NormalizeBluetoothUuid:serviceUuid], [BluetoothUntils NormalizeBluetoothUuid:characterUuid]];
}

static NSString* BuildAppCharacteristicKey(NSString* strAppId, NSString* serviceUuid, NSString* characterUuid)
{
    NSString* characteristicKey = BuildCharacteristicKey(serviceUuid, characterUuid);
    if (strAppId.length == 0) {
        return characteristicKey;
    }
    return [NSString stringWithFormat:@"%@:%@", strAppId, characteristicKey];
}

static NSString* BuildAppRequestKey(NSString* strAppId, NSString* centralId, NSString* serviceUuid,
    NSString* characterUuid, NSString* requestType)
{
    return [NSString stringWithFormat:@"%@:%@:%@:%@", strAppId, centralId,
        BuildCharacteristicKey(serviceUuid, characterUuid), requestType];
}

static NSString* BuildConnectedCentralKey(NSString* strAppId, NSString* centralId)
{
    if (strAppId.length == 0) {
        return centralId;
    }
    return [NSString stringWithFormat:@"%@:%@", strAppId, centralId];
}

static NSMutableArray* MutableArrayFromDictionary(NSDictionary* dictionary, NSString* key)
{
    NSArray* array = dictionary[key];
    return array ? [NSMutableArray arrayWithArray:array] : [NSMutableArray array];
}

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

- (NSMutableSet*)connectedCentralSet
{
    if (_connectedCentralSet == nil) {
        _connectedCentralSet = [NSMutableSet set];
    }
    return _connectedCentralSet;
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
    [self removeDic:self.centralsDic appId:strAppId];
    [self removeSet:self.connectedCentralSet appId:strAppId];
}

- (void)removeDic:(NSMutableDictionary*)dic appId:(NSString*)strAppId
{
    NSString* strRmKey = [NSString stringWithFormat:@"%@:", strAppId];
    NSMutableArray* arrKeys = [NSMutableArray array];
    for (NSString* strkey in dic.allKeys) {
        if ([strkey hasPrefix:strRmKey]) {
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

- (void)removeSet:(NSMutableSet*)set appId:(NSString*)strAppId
{
    NSString* strRmKey = [NSString stringWithFormat:@"%@:", strAppId];
    NSMutableArray* arrKeys = [NSMutableArray array];
    for (NSString* strkey in set.allObjects) {
        if ([strkey hasPrefix:strRmKey]) {
            [arrKeys addObject:strkey];
        }
    }
    for (NSString* strkey in arrKeys) {
        [set removeObject:strkey];
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
        NSArray<NSString*>* keyComponents = [strServiceKey componentsSeparatedByString:@":"];
        NSString* serviceUuid = keyComponents.count > 1 ? keyComponents.lastObject : strServiceKey;
        if ([BluetoothUntils IsSameBluetoothUuid:serviceUuid right:strKey]) {
            return self.servicesDic[strServiceKey];
        }
    }
    return nil;
}

- (CBMutableService*)getCurrentService:(NSString*)strKey appId:(NSString*)strAppId
{
    NSString* strPrefix = [NSString stringWithFormat:@"%@:", strAppId];
    for (NSString* strServiceKey in self.servicesDic.allKeys) {
        if (![strServiceKey hasPrefix:strPrefix]) {
            continue;
        }
        CBMutableService* currentService = self.servicesDic[strServiceKey];
        if ([BluetoothUntils IsSameBluetoothUuid:currentService.UUID.UUIDString right:strKey]) {
            return currentService;
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
                             appId:(int)appId
                       serviceUUID:(NSString*)serviceUUID
              notifyCharacteristic:(CBMutableCharacteristic*)notifyCharacteristic
{
    int ret = BT_ERR_INTERNAL_ERROR;
    if (strDeviceId.length == 0 || notifyCharacteristic == nil) {
        return ret;
    }
    NSString* strAppId = [NSString stringWithFormat:@"%d", appId];
    CBMutableService* service = [self getCurrentService:serviceUUID appId:strAppId];
    if (service == nil) {
        return BT_ERR_GATT_SERVICE_NOT_FOUND;
    }
    for (NSUInteger i = 0; i < service.characteristics.count; i++) {
        CBMutableCharacteristic* character = (CBMutableCharacteristic*)service.characteristics[i];
        if (![BluetoothUntils IsSameBluetoothUuid:character.UUID.UUIDString
                                            right:notifyCharacteristic.UUID.UUIDString]) {
            continue;
        }
        character.value = notifyCharacteristic.value;
        if (!character.value) {
            return ret;
        }
        NSMutableArray<CBCentral*>* subscribedCentrals = [NSMutableArray array];
        for (CBCentral* subscribedCentral in character.subscribedCentrals) {
            if ([subscribedCentral.identifier.UUIDString isEqualToString:strDeviceId]) {
                [subscribedCentrals addObject:subscribedCentral];
                break;
            }
        }
        if (subscribedCentrals.count == 0) {
            return BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED;
        }
        BOOL updated = [self.peripheralManager updateValue:character.value
                                         forCharacteristic:character
                                      onSubscribedCentrals:subscribedCentrals];
        ret = updated ? BT_NO_ERROR : BT_ERR_OPERATION_BUSY;
        break;
    }
    return ret;
}

- (int)sendRespondReadWithDeviceId:(NSString*)deviceId
                             appId:(int)appId
                       serviceUUID:(NSString*)serviceUUID
                     characterUUID:(NSString*)characterUUID
                              data:(NSData*)data
                            status:(int32_t)status
{
    if (status == BT_NO_ERROR && data == nil) {
        return BT_ERR_INTERNAL_ERROR;
    }
    NSString* strAppId = [NSString stringWithFormat:@"%d", appId];
    NSString* strKey = BuildAppRequestKey(strAppId, deviceId, serviceUUID, characterUUID, REQUEST_TYPE_READ);
    if (![self.requestDic.allKeys containsObject:strKey]) {
        return BT_ERR_INTERNAL_ERROR;
    }
    CBATTRequest* request = self.requestDic[strKey];
    if (status == BT_NO_ERROR) {
        request.value = data;
    }
    [self sendRespond:strKey serviceUUID:serviceUUID characterUUID:characterUUID appId:strAppId status:status];
    return BT_NO_ERROR;
}

- (int)sendRespondWriteWithDeviceId:(NSString*)deviceId
                              appId:(int)appId
                        serviceUUID:(NSString*)serviceUUID
                      characterUUID:(NSString*)characterUUID
                             status:(int32_t)status
{
    NSString* strAppId = [NSString stringWithFormat:@"%d", appId];
    NSString* strKey = BuildAppRequestKey(strAppId, deviceId, serviceUUID, characterUUID, REQUEST_TYPE_WRITE);
    if (![self.requestDic.allKeys containsObject:strKey]) {
        return BT_ERR_INTERNAL_ERROR;
    }
    [self sendRespond:strKey serviceUUID:serviceUUID characterUUID:characterUUID appId:strAppId status:status];
    return BT_NO_ERROR;
}

- (void)sendRespond:(NSString*)strKey
        serviceUUID:(NSString*)serviceUUID
      characterUUID:(NSString*)characterUUID
             appId:(NSString*)strAppId
             status:(int32_t)status
{
    CBATTError state = CBATTErrorRequestNotSupported;
    CBMutableCharacteristic* character = [self getCurrentCharacteristic:serviceUUID
                                                              charaUUID:characterUUID
                                                                  appId:strAppId];
    if (character && status == BT_NO_ERROR) {
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
    NSString* strAppId = [self appIdForService:characteristic.service];
    if (strAppId.length == 0) {
        return;
    }
    NSString* strKey =
        BuildAppCharacteristicKey(strAppId, characteristic.service.UUID.UUIDString, characteristic.UUID.UUIDString);
    NSMutableArray* arrCentrals = MutableArrayFromDictionary(self.centralsDic, strKey);
    NSIndexSet* indexSet =
        [arrCentrals indexesOfObjectsPassingTest:^BOOL(CBCentral* _Nonnull obj, NSUInteger idx, BOOL* _Nonnull stop) {
          return [obj.identifier isEqual:central.identifier];
        }];
    if (indexSet.count > 0) {
        [arrCentrals removeObjectsAtIndexes:indexSet];
        NSString* centralKey = BuildConnectedCentralKey(strAppId, central.identifier.UUIDString);
        [self.connectedCentralSet removeObject:centralKey];
    }
    [arrCentrals addObject:central];
    [self.centralsDic setObject:arrCentrals forKey:strKey];
    [self reportCentralConnectedIfNeeded:central appId:strAppId];
    NSData* value = [self clientConfigurationValueForCharacteristic:characteristic subscribed:YES];
    [self descriptorWriteBlockRequest:central characteristic:characteristic value:value];
}

- (void)peripheralManager:(CBPeripheralManager*)peripheral
                             central:(CBCentral*)central
    didUnsubscribeFromCharacteristic:(CBCharacteristic*)characteristic
{
    NSString* strAppId = [self appIdForService:characteristic.service];
    if (strAppId.length == 0) {
        return;
    }
    NSString* strKey =
        BuildAppCharacteristicKey(strAppId, characteristic.service.UUID.UUIDString, characteristic.UUID.UUIDString);

    NSMutableArray* arrCentrals = MutableArrayFromDictionary(self.centralsDic, strKey);
    NSIndexSet* indexSet =
        [arrCentrals indexesOfObjectsPassingTest:^BOOL(CBCentral* _Nonnull obj, NSUInteger idx, BOOL* _Nonnull stop) {
          return [obj.identifier isEqual:central.identifier];
        }];
    if (arrCentrals.count == 0 || indexSet.count == 0) {
        return;
    }
    [arrCentrals removeObjectsAtIndexes:indexSet];
    [self.centralsDic setObject:arrCentrals forKey:strKey];
    NSData* value = [self clientConfigurationValueForCharacteristic:characteristic subscribed:NO];
    [self descriptorWriteBlockRequest:central characteristic:characteristic value:value];
    if (![self hasSubscribedCentral:central appId:strAppId]) {
        NSString* centralKey = BuildConnectedCentralKey(strAppId, central.identifier.UUIDString);
        [self.connectedCentralSet removeObject:centralKey];
    }
}

- (void)peripheralManager:(CBPeripheralManager*)peripheral didReceiveReadRequest:(CBATTRequest*)request
{
    NSString* strAppId = [self appIdForService:request.characteristic.service];
    if (strAppId.length == 0) {
        return;
    }
    NSString* strKey = BuildAppRequestKey(strAppId, request.central.identifier.UUIDString,
        request.characteristic.service.UUID.UUIDString, request.characteristic.UUID.UUIDString, REQUEST_TYPE_READ);
    [self.requestDic setObject:request forKey:strKey];
    [self reportCentralConnectedIfNeeded:request.central appId:strAppId];

    CBMutableCharacteristic* character = [self getCurrentCharacteristic:request.characteristic.service.UUID.UUIDString
                                                              charaUUID:request.characteristic.UUID.UUIDString
                                                                  appId:strAppId];
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
        NSString* strAppId = [self appIdForService:request.characteristic.service];
        if (strAppId.length == 0) {
            continue;
        }
        NSString* strKey = BuildAppRequestKey(strAppId, request.central.identifier.UUIDString,
            request.characteristic.service.UUID.UUIDString, request.characteristic.UUID.UUIDString, REQUEST_TYPE_WRITE);
        [self.requestDic setObject:request forKey:strKey];
        [self reportCentralConnectedIfNeeded:request.central appId:strAppId];

        CBMutableCharacteristic* character =
            [self getCurrentCharacteristic:request.characteristic.service.UUID.UUIDString
                                 charaUUID:request.characteristic.UUID.UUIDString
                                     appId:strAppId];
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

- (void)reportCentralConnectedIfNeeded:(CBCentral*)central appId:(NSString*)strAppId
{
    if (central == nil) {
        return;
    }
    NSString* centralId = central.identifier.UUIDString;
    NSString* centralKey = BuildConnectedCentralKey(strAppId, centralId);
    if (centralId.length == 0 || self.connectDeviceBlock == nil ||
        [self.connectedCentralSet containsObject:centralKey]) {
        return;
    }
    [self.connectedCentralSet addObject:centralKey];
    [self connectDeviceCallBack:central peripheralState:CBPeripheralStateConnected];
}

- (BOOL)hasSubscribedCentral:(CBCentral*)central appId:(NSString*)strAppId
{
    if (central == nil) {
        return NO;
    }
    NSString* strPrefix = strAppId.length == 0 ? nil : [NSString stringWithFormat:@"%@:", strAppId];
    for (NSString* strKey in self.centralsDic.allKeys) {
        if (strPrefix != nil && ![strKey hasPrefix:strPrefix]) {
            continue;
        }
        NSArray* centrals = self.centralsDic[strKey];
        for (CBCentral* subscribedCentral in centrals) {
            if ([subscribedCentral.identifier isEqual:central.identifier]) {
                return YES;
            }
        }
    }
    return NO;
}

- (NSString*)appIdForService:(CBService*)service
{
    if (service == nil) {
        return nil;
    }
    for (NSString* strServiceKey in self.servicesDic.allKeys) {
        CBService* currentService = self.servicesDic[strServiceKey];
        if (currentService == service) {
            return [strServiceKey componentsSeparatedByString:@":"].firstObject;
        }
    }
    NSString* matchedAppId = nil;
    for (NSString* strServiceKey in self.servicesDic.allKeys) {
        CBService* currentService = self.servicesDic[strServiceKey];
        if ([BluetoothUntils IsSameBluetoothUuid:currentService.UUID.UUIDString right:service.UUID.UUIDString]) {
            if (matchedAppId != nil) {
                NSLog(@"Duplicate service UUID registered for multiple appId.");
                return nil;
            }
            matchedAppId = [strServiceKey componentsSeparatedByString:@":"].firstObject;
        }
    }
    return matchedAppId;
}

- (void)writeBlockRequest:(CBATTRequest*)request characteristic:(CBMutableCharacteristic*)character
{
    if (self.characterWriteBlock) {
        self.characterWriteBlock(request.central.identifier.UUIDString, character);
    }
}

- (NSData*)clientConfigurationValueForCharacteristic:(CBCharacteristic*)characteristic subscribed:(BOOL)subscribed
{
    uint8_t value[CLIENT_CONFIG_VALUE_LENGTH] = { CLIENT_CONFIG_DISABLE_VALUE, CLIENT_CONFIG_DISABLE_VALUE };
    if (!subscribed || characteristic == nil) {
        return [NSData dataWithBytes:value length:CLIENT_CONFIG_VALUE_LENGTH];
    }
    if ((characteristic.properties & CBCharacteristicPropertyNotify) != 0 ||
        (characteristic.properties & CBCharacteristicPropertyNotifyEncryptionRequired) != 0) {
        value[0] = CLIENT_CONFIG_NOTIFY_VALUE;
    } else if ((characteristic.properties & CBCharacteristicPropertyIndicate) != 0 ||
        (characteristic.properties & CBCharacteristicPropertyIndicateEncryptionRequired) != 0) {
        value[0] = CLIENT_CONFIG_INDICATE_VALUE;
    }
    return [NSData dataWithBytes:value length:CLIENT_CONFIG_VALUE_LENGTH];
}

- (void)descriptorWriteBlockRequest:(CBCentral*)central characteristic:(CBCharacteristic*)character value:(NSData*)value
{
    if (self.descriptorWriteBlock == nil) {
        return;
    }
    self.descriptorWriteBlock(central.identifier.UUIDString, character, value);
}

- (void)notifyBlockRequest:(CBCentral*)central characteristic:(CBCharacteristic*)character state:(int)state
{
    if (self.characterNotifyBlock) {
        self.characterNotifyBlock(central.identifier.UUIDString, character, state);
    }
}

- (CBMutableCharacteristic*)getCurrentCharacteristic:(NSString*)serviceUUID
                                          charaUUID:(NSString*)charaUUID
                                              appId:(NSString*)strAppId
{
    CBMutableService* service = [self getCurrentService:serviceUUID appId:strAppId];
    if (service == nil) {
        return nil;
    }

    CBMutableCharacteristic* currentCharacter = nil;

    for (CBMutableCharacteristic* character in service.characteristics) {
        if ([BluetoothUntils IsSameBluetoothUuid:character.UUID.UUIDString right:charaUUID]) {
            currentCharacter = character;
            break;
        }
    }

    return currentCharacter;
}

@end
