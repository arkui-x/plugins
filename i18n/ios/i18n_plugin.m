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

- (NSArray<NSString *> *)getSystemLanguages
{
    NSArray<NSString *> *allIdentifiers = [NSLocale availableLocaleIdentifiers];
    NSMutableArray *systemLanguages = [NSMutableArray array];
    
    for (NSString *code in allIdentifiers) {
        NSString *languageTag;
        NSLocale *locale = [[NSLocale alloc] initWithLocaleIdentifier:code];
        if (locale.languageCode && locale.languageCode.length != 0) {
            languageTag = locale.languageCode;
        }
        
        if (locale.scriptCode && locale.scriptCode.length != 0) {
            languageTag = [languageTag stringByAppendingFormat:@"-%@", locale.scriptCode];
        }
        if (!languageTag || languageTag.length == 0) {
            continue;
        }
        [systemLanguages addObject:languageTag];
    }
    systemLanguages = [[[NSOrderedSet orderedSetWithArray:systemLanguages] array] mutableCopy];
    
    return systemLanguages;
}

- (NSArray<NSString *> *)getSystemCountries:(NSString *)languageCode
{
    NSArray<NSString *> *allIdentifiers = [NSLocale availableLocaleIdentifiers];
    NSMutableArray<NSString *> *allCountryRegions = [NSMutableArray array];
    
    for (NSString *code in allIdentifiers) {
        NSLocale *locale = [[NSLocale alloc] initWithLocaleIdentifier:code];
        if (!locale.languageCode || locale.languageCode.length == 0) {
            continue;
        }
        
        if (!languageCode || languageCode.length == 0) {
            [allCountryRegions addObject:locale.languageCode];
            continue;
        }
        
        if ([languageCode isEqualToString:locale.languageCode]) {
            if (!locale.countryCode || locale.countryCode.length == 0) {
                continue;
            }
            [allCountryRegions addObject:[locale.countryCode uppercaseString]];
        }
    }
    allCountryRegions = [[[NSOrderedSet orderedSetWithArray:allCountryRegions] array] mutableCopy];
    
    return allCountryRegions;
}

- (NSArray<NSString *> *)getAvailableIDs
{
    return [NSTimeZone knownTimeZoneNames];
}

- (NSArray<NSString *> *)getPreferredLanguages
{
    return [NSLocale preferredLanguages];
}

- (BOOL)isSuggested:(NSString *)langCode regionCode:(NSString *)regionCode
{
    if (!langCode || langCode.length == 0) {
        return NO;
    }
    if (regionCode == nil || regionCode.length == 0) {
        NSLocale *systemLocale = [NSLocale currentLocale];
        NSString *country = [systemLocale objectForKey:NSLocaleCountryCode];
        if (country == nil || country.length == 0) {
            return NO;
        }
        regionCode = country;
    }
    for (NSString *identifier in [NSLocale availableLocaleIdentifiers]) {
        NSLocale *locale = [NSLocale localeWithLocaleIdentifier:identifier];
        if (locale == nil) {
            continue;
        }
        NSString *country = [locale objectForKey:NSLocaleCountryCode];
        NSString *lang = [locale objectForKey:NSLocaleLanguageCode];
        if (country.length == 0 || lang.length == 0) {
            continue;
        }
        if ([country isEqualToString:regionCode] && [lang isEqualToString:langCode]) {
            return YES;
        }
    }
    return NO;
}

- (NSString *)getNumberingSystem
{
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

- (BOOL)isUsingLocalDigit
{
    NSDictionary *languageToNumberSystem = @{
            @"ar": @"arab",
            @"as": @"beng",
            @"bn": @"beng",
            @"fa": @"arabext",
            @"mr": @"deva",
            @"my": @"mymr",
            @"ne": @"deva",
            @"ur": @"latn",
        };
    
    NSString *sysLangs = [[NSLocale preferredLanguages] firstObject];
    NSArray<NSString *> *langCode = [sysLangs componentsSeparatedByString:@"-"];
    if (langCode.count < 1) {
        return NO;
    }
    NSString *currentLang = [langCode firstObject];
    NSString *localSystem = languageToNumberSystem[currentLang];
    if (!localSystem) {
        return NO;
    }
    
    NSString *numberSystem = [self getNumberingSystem];
    return [localSystem isEqualToString:numberSystem];
}
@end
