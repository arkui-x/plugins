/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#import <WebKit/WebKit.h>
#include "web_cookie_manager_ios.h"
#import <Foundation/Foundation.h>
using namespace OHOS::NWebError;

namespace OHOS::Plugin {
bool SetCookie(const std::string& url, std::map<std::string, std::string> cookieAttr,
    std::map<std::string, std::string> cookieValue)
{
    NSString* ocUrl = [NSString stringWithCString:url.c_str() encoding:NSUTF8StringEncoding];
    NSString* urlHost = [[NSURL URLWithString:ocUrl] host];
    if (ocUrl == nil || urlHost == nil) {
        return false;
    }
    for (auto iter = cookieValue.begin(); iter != cookieValue.end(); iter++) {
        NSString* key = (0 == iter->first.length()) ? (@"") : (@(iter->first.c_str()));
        NSString* value = (0 == iter->second.length()) ? (@"") : (@(iter->second.c_str()));
        NSString* path = [NSString stringWithCString:cookieAttr["path"].c_str() encoding:NSUTF8StringEncoding];
        NSString* domain = [NSString stringWithCString:cookieAttr["domain"].c_str() encoding:NSUTF8StringEncoding];
        NSString* expiresDate =
            [NSString stringWithCString:cookieAttr["expires"].c_str() encoding:NSUTF8StringEncoding];
        NSString* maxAge = [NSString stringWithCString:cookieAttr["max-age"].c_str() encoding:NSUTF8StringEncoding];
        NSString* sameSite = [NSString stringWithCString:cookieAttr["samesite"].c_str() encoding:NSUTF8StringEncoding];
        NSString* isSecure = [NSString stringWithCString:cookieAttr["secure"].c_str() encoding:NSUTF8StringEncoding];
        NSMutableDictionary* cookieProperties = [NSMutableDictionary dictionary];
        if (key == nil || value == nil || key.length == 0 || value.length == 0) {
            break;
        }
        [cookieProperties setObject:key forKey:NSHTTPCookieName];
        [cookieProperties setObject:value forKey:NSHTTPCookieValue];
        if (path != nil && path.length != 0) {
            [cookieProperties setObject:path forKey:NSHTTPCookiePath];
        } else {
            [cookieProperties setObject:@"/" forKey:NSHTTPCookiePath];
        }
        if (domain != nil && domain.length != 0) {
            [cookieProperties setObject:domain forKey:NSHTTPCookieDomain];
        } else {
            [cookieProperties setObject:urlHost forKey:NSHTTPCookieDomain];
        }
        if (expiresDate != nil && expiresDate.length != 0) {
            [cookieProperties setObject:expiresDate forKey:NSHTTPCookieExpires];
        }
        if (maxAge != nil && maxAge.length != 0) {
            [cookieProperties setObject:maxAge forKey:NSHTTPCookieMaximumAge];
        }
        if (isSecure != nil && [isSecure boolValue]) {
            [cookieProperties setObject:@"TRUE" forKey:NSHTTPCookieSecure];
        }
        if (sameSite != nil && sameSite.length != 0) {
            [cookieProperties setObject:sameSite forKey:NSHTTPCookieSameSitePolicy];
        }
        NSHTTPCookie* cookie = [NSHTTPCookie cookieWithProperties:cookieProperties];
        [NSHTTPCookieStorage.sharedHTTPCookieStorage setCookie:cookie];
    }
    return true;
}

ErrCode WebCookieManagerIOS::ConfigCookie(const std::string& url, const std::string& value, int32_t asyncCallbackInfoId)
{
    NSArray* cookieAttribute = @[
        @"domain", @"path", @"expires", @"max-age", @"size", @"httponly", @"secure", @"samesite", @"partitioned",
        @"priority"
    ];
    NSString* ocUrl = [NSString stringWithCString:url.c_str() encoding:NSUTF8StringEncoding];
    NSString* ocString = [NSString stringWithCString:value.c_str() encoding:NSUTF8StringEncoding];
    NSArray* array = [ocString componentsSeparatedByString:@";"];
    std::map<std::string, std::string> cookieValueDict;
    std::map<std::string, std::string> cookieAttrDict;
    for (NSString* key_value in array) {
        NSRange range = [key_value rangeOfString:@"="];
        if (range.location != NSNotFound) {
            NSArray* keyValueArray = [key_value componentsSeparatedByString:@"="];
            NSString* cookieName = [keyValueArray objectAtIndex:0];
            NSString* cookieValue = [keyValueArray objectAtIndex:1];
            BOOL contains = [cookieAttribute containsObject:[cookieName lowercaseString]];
            if (contains) {
                cookieAttrDict.insert(
                    std::map<std::string, std::string>::value_type([cookieName UTF8String], [cookieValue UTF8String]));
            } else {
                cookieValueDict.insert(
                    std::map<std::string, std::string>::value_type([cookieName UTF8String], [cookieValue UTF8String]));
            }
        }
    }
    WebCookieManager::OnConfigReceiveValue(SetCookie(url, cookieAttrDict, cookieValueDict), asyncCallbackInfoId);
    return NO_ERROR;
}

void WebCookieManagerIOS::FetchCookie(const std::string& url, int32_t asyncCallbackInfoId)
{
    NSString* ocUrl = [NSString stringWithCString:url.c_str() encoding:NSUTF8StringEncoding];
    NSString* urlHost = [[NSURL URLWithString:ocUrl] host];
    NSString* cookieValue = @"";
    NSArray* cookies = NSHTTPCookieStorage.sharedHTTPCookieStorage.cookies;
    for (int i = 0; i < cookies.count; i++) {
        NSHTTPCookie* cookie = cookies[i];
        if ([urlHost hasSuffix:cookie.domain] ||
            [[NSString stringWithFormat:@".\\%@", urlHost] hasSuffix:cookie.domain]) {
            NSString* nameValue = [NSString stringWithFormat:@"%@=%@", cookie.name, cookie.value];
            if (cookieValue.length > 0) {
                cookieValue = [NSString stringWithFormat:@"%@;%@", cookieValue, nameValue];
            } else {
                cookieValue = nameValue;
            }
        }
    }

    WebCookieManager::OnFetchReceiveValue([cookieValue UTF8String], asyncCallbackInfoId);
}

void WebCookieManagerIOS::ClearAllCookies(int32_t asyncCallbackInfoId)
{
    NSHTTPCookieStorage* sharedHTTPCookieStorage = NSHTTPCookieStorage.sharedHTTPCookieStorage;
    NSArray* cookies = sharedHTTPCookieStorage.cookies;
    for (int i = 0; i < (int)cookies.count; i++) {
        [sharedHTTPCookieStorage deleteCookie:cookies[i]];
    }
    WebCookieManager::OnClearReceiveValue(asyncCallbackInfoId);
}

bool WebCookieManagerIOS::ExistCookie(bool incognito)
{
    NSHTTPCookieStorage* sharedHTTPCookieStorage = NSHTTPCookieStorage.sharedHTTPCookieStorage;
    NSArray* cookies = sharedHTTPCookieStorage.cookies;
    return cookies && [cookies count] > 0;
}

void WebCookieManagerIOS::ClearSessionCookie(int32_t asyncCallbackInfoId)
{
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_11_0
    if (@available(iOS 11.0, *)) {
        WKWebsiteDataStore *defaultDataStore = [WKWebsiteDataStore defaultDataStore];
        WKHTTPCookieStore *wkHTTPCookieStorage = [defaultDataStore httpCookieStore];
        [wkHTTPCookieStorage getAllCookies:^(NSArray<NSHTTPCookie *> * _Nonnull cookies) {
            for (NSHTTPCookie *cookie in cookies) {
                if ([cookie isSessionOnly]) {
                    [wkHTTPCookieStorage deleteCookie:cookie completionHandler:nil];
                }
            }
        }];
    }
#endif
    NSHTTPCookieStorage *sharedHTTPCookieStorage = [NSHTTPCookieStorage sharedHTTPCookieStorage];
    NSArray<NSHTTPCookie *> * cookies = [sharedHTTPCookieStorage.cookies copy];
    for (NSHTTPCookie *cookie in cookies) {
        if ([cookie isSessionOnly]) {
            [sharedHTTPCookieStorage deleteCookie:cookie];
        }
    }
    WebCookieManager::OnClearReceiveValue(asyncCallbackInfoId);
}
} // namespace OHOS::Plugin
