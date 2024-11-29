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
#ifndef OHOS_BLUETOOTH_STANDARD_BLUETOOTHPERIPHERALMANAGER_H
#define OHOS_BLUETOOTH_STANDARD_BLUETOOTHPERIPHERALMANAGER_H

#import <CoreBluetooth/CoreBluetooth.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include "bluetooth_errorcode.h"

using namespace OHOS::Bluetooth;

typedef void (^StartAdvertisingBlock)(bool isAdvertising, int state);
typedef void (^AddServiceBlock)(int ret);
typedef void (^UpdataConnectDeviceBlock)(int state, NSString* _Nonnull deviceId);
typedef void (^CharacterReadBlock)(NSString* _Nonnull deviceId, CBMutableCharacteristic* _Nullable characteristic);
typedef void (^CharacterWriteBlock)(NSString* _Nonnull deviceId, CBMutableCharacteristic* _Nullable characteristic);
typedef void (^CharacterNotifyBlock)(NSString* _Nonnull deviceId, CBCharacteristic* _Nullable characteristic, int ret);

@interface BluetoothPeripheralManager : NSObject <CBPeripheralManagerDelegate> {
    bool bluetoothState;
}

@property(nonatomic, strong) CBPeripheralManager* peripheralManager;

@property(nonatomic, assign) int appId;
@property(nonatomic, strong) NSMutableDictionary* requestDic;
@property(nonatomic, strong) NSMutableDictionary* servicesDic;
@property(nonatomic, strong) NSMutableDictionary* centralsDic;
@property(nonatomic, strong) NSMutableDictionary* blockDic;
@property(nonatomic, copy) StartAdvertisingBlock startAdvertisingBlock;
@property(nonatomic, copy) UpdataConnectDeviceBlock connectDeviceBlock;
@property(nonatomic, copy) CharacterReadBlock characterReadBlock;
@property(nonatomic, copy) CharacterWriteBlock characterWriteBlock;
@property(nonatomic, copy) CharacterNotifyBlock characterNotifyBlock;

+ (BluetoothPeripheralManager*)sharedInstance;

- (int)getAppId;
- (bool)isBleEnabled;
- (void)createPeripheralManager;
- (void)closePeripheral:(NSString* _Nullable)strAppId;
- (int)startAdvertising:(NSString* _Nullable)strAppId
          isConnectable:(bool)isConnectable
           servicesUUID:(NSMutableArray*)serviceUUIDs
            serviceData:(NSMutableDictionary*)serviceData
      includeDeviceName:(bool)includeDeviceName
               duration:(int)duration;
- (int)stopAdvertising:(NSString* _Nullable)strAppId;
- (void)addService:(CBMutableService* _Nullable)service appId:(int)appId block:(AddServiceBlock)block;
- (int)removeService:(NSString* _Nullable)strUUID appId:(int)appId;
- (int)notifyCharacteristicChanged:(NSString* _Nullable)strDeviceId
                       serviceUUID:(NSString* _Nullable)serviceUUID
              notifyCharacteristic:(CBMutableCharacteristic* _Nullable)notifyCharacteristic;
- (int)sendRespondReadWithDeviceId:(NSString* _Nullable)deviceId
                       serviceUUID:(NSString* _Nullable)serviceUUID
                     characterUUID:(NSString* _Nullable)characterUUID
                              data:(NSData* _Nullable)data;
- (int)sendRespondWriteWithDeviceId:(NSString* _Nullable)deviceId
                        serviceUUID:(NSString* _Nullable)serviceUUID
                      characterUUID:(NSString* _Nullable)characterUUID;

@end

#endif // OHOS_BLUETOOTH_STANDARD_BLUETOOTHPERIPHERALMANAGER_H