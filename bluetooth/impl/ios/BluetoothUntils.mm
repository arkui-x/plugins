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

#import "BluetoothUntils.h"

@implementation BluetoothUntils

static NSString* const BLUETOOTH_BASE_UUID_SUFFIX = @"00001000800000805F9B34FB";
static const NSUInteger BLUETOOTH_BASE_UUID_LENGTH = 32;
static const NSUInteger BLUETOOTH_UUID_PREFIX_LENGTH = 8;
static const NSUInteger BLUETOOTH_UUID_SHORT_LENGTH = 4;
static NSString* const BLUETOOTH_UUID_ZERO_GROUP = @"0000";

+ (id)GetValueWithData:(NSData*)data uuid:(NSString*)strUUID
{
    NSArray* arrDataTypeNumberKey = @[
        CBUUIDCharacteristicExtendedPropertiesString, CBUUIDClientCharacteristicConfigurationString,
        CBUUIDServerCharacteristicConfigurationString
    ];
    NSArray* arrDataTypeStringKey = @[
        CBUUIDCharacteristicUserDescriptionString, CBUUIDCharacteristicAggregateFormatString,
        CBUUIDCharacteristicValidRangeString, CBUUIDL2CAPPSMCharacteristicString
    ];
    id retData;
    if ([arrDataTypeNumberKey containsObject:strUUID]) {
        NSString* string = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        NSNumberFormatter* formatter = [[NSNumberFormatter alloc] init];
        [formatter setLocale:[NSLocale localeWithLocaleIdentifier:@"en_US"]];
        [formatter setNumberStyle:NSNumberFormatterDecimalStyle];
        retData = [formatter numberFromString:string];
    } else if ([arrDataTypeStringKey containsObject:strUUID]) {
        retData = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    } else {
        retData = data;
    }
    return retData;
}

+ (NSString*)GetSystemStringUUIDKey:(NSString*)string
{
    NSString* sysUUIDKey = string;
    NSArray* arrKeys = @[
        CBUUIDCharacteristicExtendedPropertiesString, CBUUIDClientCharacteristicConfigurationString,
        CBUUIDServerCharacteristicConfigurationString, CBUUIDCharacteristicUserDescriptionString,
        CBUUIDCharacteristicAggregateFormatString, CBUUIDCharacteristicValidRangeString,
        CBUUIDL2CAPPSMCharacteristicString
    ];
    for (int i = 0; i < arrKeys.count; i++) {
        NSString* sysKey = arrKeys[i];
        sysKey = [NSString stringWithFormat:@"0000%@", sysKey];
        if ([string containsString:sysKey]) {
            sysUUIDKey = arrKeys[i];
            break;
        }
    }
    return sysUUIDKey;
}

+ (NSString*)NormalizeBluetoothUuid:(NSString*)uuid
{
    NSString* upperUuid = [[uuid stringByReplacingOccurrencesOfString:@"-" withString:@""] uppercaseString];
    if ([upperUuid hasSuffix:BLUETOOTH_BASE_UUID_SUFFIX] && upperUuid.length == BLUETOOTH_BASE_UUID_LENGTH) {
        NSString* shortUuid = [upperUuid substringToIndex:BLUETOOTH_UUID_PREFIX_LENGTH];
        while (shortUuid.length > BLUETOOTH_UUID_SHORT_LENGTH && [shortUuid hasSuffix:BLUETOOTH_UUID_ZERO_GROUP]) {
            shortUuid = [shortUuid substringToIndex:shortUuid.length - BLUETOOTH_UUID_SHORT_LENGTH];
        }
        while (shortUuid.length > BLUETOOTH_UUID_SHORT_LENGTH && [shortUuid hasPrefix:BLUETOOTH_UUID_ZERO_GROUP]) {
            shortUuid = [shortUuid substringFromIndex:BLUETOOTH_UUID_SHORT_LENGTH];
        }
        return shortUuid;
    }
    return upperUuid;
}

+ (BOOL)IsSameBluetoothUuid:(NSString*)left right:(NSString*)right
{
    return [[self NormalizeBluetoothUuid:left] isEqualToString:[self NormalizeBluetoothUuid:right]];
}

@end
