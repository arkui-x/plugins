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

#ifndef OHOS_BLUETOOTH_STANDARD_BLUETOOTHCENTRALMANAGER_H
#define OHOS_BLUETOOTH_STANDARD_BLUETOOTHCENTRALMANAGER_H

#import <CoreBluetooth/CoreBluetooth.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include "bluetooth_errorcode.h"

using namespace OHOS::Bluetooth;

typedef void (^ScanDataBlock)(CBPeripheral* peripheral, NSDictionary<NSString*, id>* advertisementData, NSNumber* RSSI);
typedef void (^ConnectStateBlock)(int32_t ret, int32_t state);
typedef void (^ReadCharacterBlock)(CBCharacteristic* chara, int32_t ret);
typedef void (^ReadDescriptorBlock)(CBDescriptor* des, int32_t ret);
typedef void (^WriteCharacterBlock)(CBCharacteristic* chara, int32_t ret);
typedef void (^ServicesDiscoveredBlock)(int32_t ret);
typedef void (^UpdataBleStateBlock)(int ret);

@interface BluetoothCentralManager : NSObject <CBCentralManagerDelegate, CBPeripheralDelegate> {
    CBPeripheral* currentPeripheral;
    int bluetoothState;
}

@property(nonatomic, strong) CBCentralManager* centralManager;
@property(nonatomic, strong) NSMutableDictionary* diviceIdDic;
@property(nonatomic, strong) NSMutableArray* arrAutoConnectDivice;
@property(nonatomic, strong) NSMutableDictionary* servicesDic;
@property(nonatomic, strong) NSMutableDictionary* characteristicBlockDic;
@property(nonatomic, strong) NSMutableDictionary* getServicesStateDic;

@property(nonatomic, copy) ScanDataBlock scanDataBlock;
@property(nonatomic, copy) ConnectStateBlock connectStateBlock;
@property(nonatomic, copy) ServicesDiscoveredBlock servicesDiscoveredBlock;
@property(nonatomic, copy) UpdataBleStateBlock updataBleStateBlock;

+ (BluetoothCentralManager*)sharedInstance;

- (NSString*)getLocalName;
- (CBPeripheral*)getDevice:(NSString*)address;
- (int)startBLEScanWithId:(NSMutableArray<CBUUID*>*)arrUUID;
- (void)stopBLEScan;
- (void)registerDeviceId:(NSString*)strDeviceId withAppId:(int32_t)appId;
- (void)closeClient:(int32_t)appId;
- (int)connect:(int32_t)appId autoConnect:(bool)autoConnect;
- (int)disConnect:(int32_t)appId;
- (int)discoverServices:(int)appId;
- (NSArray<CBService*>*)getServices:(int)appId;
- (int)getReadCharacteristic:(int)appId
                 serviceUuid:(CBUUID*)serviceUuid
               characterUuid:(CBUUID*)characterUuid
                        data:(ReadCharacterBlock)dataBlock;
- (int)writeCharacteristic:(int)appId
               serviceUuid:(CBUUID*)serviceUuid
             characterUuid:(CBUUID*)characterUuid
                      data:(NSData*)data
             isOutResponse:(bool)isOutResponse
                  isSigned:(bool)isSigned
                     block:(WriteCharacterBlock)block;
- (int)RequestNotification:(int)appId
               serviceUuid:(CBUUID*)serviceUuid
             characterUuid:(CBUUID*)characterUuid
        enableNotification:(bool)enableNotification;
- (int)getReadDescriptor:(int)appId
             serviceUuid:(CBUUID*)serviceUuid
           characterUuid:(CBUUID*)characterUuid
          descriptorUuid:(CBUUID*)descriptorUuid
                    data:(ReadDescriptorBlock)dataBlock;
- (bool)isBleEnabled;
- (int)getBleState;

@end

#endif // OHOS_BLUETOOTH_STANDARD_BLUETOOTHCENTRALMANAGER_H
