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

#import "BluetoothUntils.h"

@implementation BluetoothUntils

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

@end