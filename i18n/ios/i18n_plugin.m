/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#import "i18n_plugin.h"

@implementation iOSI18NPlugin

+(instancetype)shareinstance{
    static dispatch_once_t onceToken;
    static iOSI18NPlugin *instance = nil;
    dispatch_once(&onceToken, ^{
        instance = [iOSI18NPlugin new];
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

-(NSString*)getSystemLanguage{
    NSString *language = [[NSLocale preferredLanguages] firstObject];
    return language;
}

-(NSString*)getSystemRegion{
    NSString *region = [[NSLocale currentLocale] objectForKey: NSLocaleCountryCode];
    return region;
}

-(NSString*)getSystemTimezone{
    return [[NSTimeZone systemTimeZone] name];
}

-(NSString*)getAppPreferredLanguage{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSString *customLanguages = [defaults objectForKey:@"ArkuiXApplePreferredLanguages"];

    NSString *languageCode;
    if (customLanguages && customLanguages.length != 0) {
        languageCode = customLanguages;
    } else {
        languageCode = [self getSystemLocale];
    }

    return languageCode;
}

- (void)setAppPreferredLanguage:(NSString *)languageCode {
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    [defaults setObject:languageCode forKey:@"ArkuiXApplePreferredLanguages"];
    [defaults synchronize];
}
@end
