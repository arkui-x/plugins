/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#import "intl_plugin.h"

@implementation iOSINTLPlugin

+(instancetype)shareinstance{
    static dispatch_once_t onceToken;
    static iOSINTLPlugin *instance = nil;
    dispatch_once(&onceToken, ^{
        instance = [iOSINTLPlugin new];
    });
    return instance;
}

-(bool)is24HourClock{
    NSLocale *curLocale = [NSLocale autoupdatingCurrentLocale];
    NSString *template = @"j";
    NSString *dateFormat = [NSDateFormatter dateFormatFromTemplate:template options:0 locale:curLocale];
    if ([dateFormat containsString:@"a"]) {
        return false;
    } else {
        return true;
    }
}

-(NSString*)getSystemLocale{
    NSString *localeString = [[NSLocale preferredLanguages] firstObject];
    return localeString;
}

-(NSString*)getSystemTimezone{
    return [[NSTimeZone systemTimeZone] name];
}

-(NSString*)getSystemCalendar{
    NSCalendar *currentCalendar = [NSCalendar autoupdatingCurrentCalendar];
    return currentCalendar.calendarIdentifier;
}

-(NSString*)getNumberingSystem{
    NSLocale *currentLocale = [NSLocale autoupdatingCurrentLocale];
    NSString *localeIdentifier = [currentLocale objectForKey:NSLocaleIdentifier];
    NSArray *parts = [localeIdentifier componentsSeparatedByString:@"@"];
    for (NSInteger i = 1; i < parts.count; i++) {
        NSArray *keyValuePairs = [parts[i] componentsSeparatedByString:@";"];
        for (NSString *pair in keyValuePairs) {
            NSArray *keyValue = [pair componentsSeparatedByString:@"="];
            if (keyValue.count == 2 && [keyValue[0] isEqualToString:@"numbers"]) {
                return keyValue[1];
            }
        }
    }
    return @"latn";
}

-(NSString*)getDeviceType{
    NSString *deviceModel = [[UIDevice currentDevice] model];
    NSString *deviceType = @"phones";
    if ([deviceModel isEqualToString: @"iPhone"]) {
        deviceType = @"phones";
    }
    if ([deviceModel isEqualToString: @"iPad"]) {
        deviceType = @"tablet";
    }
    return deviceType;
}

@end
