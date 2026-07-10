/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
#include "web_cookie_manager_ios.h"

#import <Foundation/Foundation.h>
#import <WebKit/WebKit.h>

#include <atomic>
#include <map>
#include <memory>

#include "AceWebControllerBridge.h"

#define WEBKIT_IOS_VERSION_17_0 170000

using namespace OHOS::NWebError;

namespace OHOS::Plugin {
constexpr NSTimeInterval COOKIE_TASK_TIMEOUT_SECONDS = 2.0;
constexpr NSTimeInterval COOKIE_RUN_LOOP_STEP_SECONDS = 0.01;

namespace {
// URL parsing.
NSURL* CreateCookieUrl(const std::string& url)
{
    NSString* ocUrl = [NSString stringWithCString:url.c_str() encoding:NSUTF8StringEncoding];
    if (ocUrl == nil) {
        return nil;
    }
    NSURL* cookieUrl = [NSURL URLWithString:ocUrl];
    if (cookieUrl == nil || cookieUrl.host == nil) {
        return nil;
    }
    return cookieUrl;
}

// Cookie comparison.
BOOL IsSameCookie(NSHTTPCookie* lhs, NSHTTPCookie* rhs)
{
    if (lhs == nil || rhs == nil) {
        return NO;
    }
    NSString* lhsDomain = lhs.domain == nil ? @"" : [lhs.domain lowercaseString];
    NSString* rhsDomain = rhs.domain == nil ? @"" : [rhs.domain lowercaseString];
    while ([lhsDomain hasPrefix:@"."]) {
        lhsDomain = [lhsDomain substringFromIndex:1];
    }
    while ([rhsDomain hasPrefix:@"."]) {
        rhsDomain = [rhsDomain substringFromIndex:1];
    }
    return [lhs.name isEqualToString:rhs.name] && [lhsDomain isEqualToString:rhsDomain] &&
        [lhs.path isEqualToString:rhs.path];
}

BOOL HasHttpOnlyCookie(NSArray<NSHTTPCookie*>* cookies, NSHTTPCookie* newCookie)
{
    for (NSHTTPCookie* cookie in cookies ?: @[]) {
        if (cookie.isHTTPOnly && IsSameCookie(cookie, newCookie)) {
            return YES;
        }
    }
    return NO;
}

// Cookie data store.
WKWebsiteDataStore* CookieDataStore(bool incognito)
{
    if (incognito) {
        return (__bridge WKWebsiteDataStore*)getCookieDataStoreOC(incognito);
    }
    return [WKWebsiteDataStore defaultDataStore];
}

// Async cookie task synchronization.
BOOL WaitForCookieTask(dispatch_semaphore_t semaphore)
{
    if ([NSThread isMainThread]) {
        NSDate* timeoutDate = [NSDate dateWithTimeIntervalSinceNow:COOKIE_TASK_TIMEOUT_SECONDS];
        while (dispatch_semaphore_wait(semaphore, DISPATCH_TIME_NOW) != 0) {
            if ([timeoutDate timeIntervalSinceNow] <= 0) {
                return NO;
            }
            [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode
                                     beforeDate:[NSDate dateWithTimeIntervalSinceNow:COOKIE_RUN_LOOP_STEP_SECONDS]];
        }
        return YES;
    }
    int64_t timeout = static_cast<int64_t>(COOKIE_TASK_TIMEOUT_SECONDS * NSEC_PER_SEC);
    return dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, timeout)) == 0;
}
} // namespace

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

ErrCode WebCookieManagerIOS::ConfigCookie(const std::string& url, const std::string& value, bool incognito,
    bool includeHttpOnly, int32_t asyncCallbackInfoId)
{
    ErrCode ret = ConfigCookieSync(url, value, incognito, includeHttpOnly);
    if (ret == NO_ERROR) {
        WebCookieManager::OnConfigReceiveValue(true, asyncCallbackInfoId);
    } else {
        WebCookieManager::OnConfigReceiveError(ret, asyncCallbackInfoId);
    }
    return ret;
}

static ErrCode GetExistingIncognitoCookies(NSMutableArray<NSHTTPCookie*>* cookies)
{
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_11_0
    if (@available(iOS 11.0, *)) {
        WKWebsiteDataStore* dataStore = CookieDataStore(true);
        if (dataStore == nil) {
            return CAPABILITY_NOT_SUPPORTED_ERROR;
        }
        __block NSArray<NSHTTPCookie*>* wkCookies = @[];
        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
        [dataStore.httpCookieStore getAllCookies:^(NSArray<NSHTTPCookie*>* fetchedCookies) {
            wkCookies = [fetchedCookies ?: @[] copy];
            dispatch_semaphore_signal(semaphore);
        }];
        if (!WaitForCookieTask(semaphore)) {
            return CAPABILITY_NOT_SUPPORTED_ERROR;
        }
        [cookies addObjectsFromArray:wkCookies ?: @[]];
        return NO_ERROR;
    }
#endif
    return CAPABILITY_NOT_SUPPORTED_ERROR;
}

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_11_0
static ErrCode SetCookieToCookieStore(NSHTTPCookie* cookie, bool incognito) API_AVAILABLE(ios(11.0))
{
    WKWebsiteDataStore* dataStore = CookieDataStore(incognito);
    if (dataStore == nil) {
        return CAPABILITY_NOT_SUPPORTED_ERROR;
    }
    if (cookie == nil) {
        return NO_ERROR;
    }
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    [dataStore.httpCookieStore setCookie:cookie completionHandler:^{
        dispatch_semaphore_signal(semaphore);
    }];
    return WaitForCookieTask(semaphore) ? NO_ERROR : CAPABILITY_NOT_SUPPORTED_ERROR;
}
#endif

static ErrCode ConfigPersistentCookie(NSHTTPCookie* cookie, bool includeHttpOnly)
{
    NSMutableArray<NSHTTPCookie*>* existingCookies = [NSMutableArray array];
    if (!includeHttpOnly) {
        [existingCookies addObjectsFromArray:NSHTTPCookieStorage.sharedHTTPCookieStorage.cookies ?: @[]];
    }
    if (!includeHttpOnly && HasHttpOnlyCookie(existingCookies, cookie)) {
        return NO_ERROR;
    }
    [NSHTTPCookieStorage.sharedHTTPCookieStorage setCookie:cookie];
    return NO_ERROR;
}

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_11_0
static ErrCode ConfigIncognitoCookie(NSHTTPCookie* cookie, bool includeHttpOnly)
{
    if (@available(iOS 11.0, *)) {
        NSMutableArray<NSHTTPCookie*>* existingCookies = [NSMutableArray array];
        if (!includeHttpOnly) {
            ErrCode ret = GetExistingIncognitoCookies(existingCookies);
            if (ret != NO_ERROR) {
                return ret;
            }
        }
        if (!includeHttpOnly && HasHttpOnlyCookie(existingCookies, cookie)) {
            return NO_ERROR;
        }
        return SetCookieToCookieStore(cookie, true);
    }
    return CAPABILITY_NOT_SUPPORTED_ERROR;
}
#endif

// Parses a Set-Cookie header value into NSHTTPCookie objects. Used only by ConfigCookieSync.
static NSArray<NSHTTPCookie*>* ParseCookies(NSURL* cookieUrl, NSString* value)
{
    if (cookieUrl == nil || value == nil || value.length == 0) {
        return @[];
    }
    NSDictionary* headerFields = @{ @"Set-Cookie" : value };
    return [NSHTTPCookie cookiesWithResponseHeaderFields:headerFields forURL:cookieUrl];
}

ErrCode WebCookieManagerIOS::ConfigCookieSync(const std::string& url, const std::string& value, bool incognito,
    bool includeHttpOnly)
{
    NSURL* cookieUrl = CreateCookieUrl(url);
    if (cookieUrl == nil) {
        return INVALID_URL;
    }
    NSString* ocValue = [NSString stringWithCString:value.c_str() encoding:NSUTF8StringEncoding];
    NSArray<NSHTTPCookie*>* cookies = ParseCookies(cookieUrl, ocValue);
    if (cookies.count != 1) {
        return INVALID_COOKIE_VALUE;
    }
    NSHTTPCookie* cookie = cookies.firstObject;
    if (!incognito) {
        return ConfigPersistentCookie(cookie, includeHttpOnly);
    }
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_11_0
    if (@available(iOS 11.0, *)) {
        return ConfigIncognitoCookie(cookie, includeHttpOnly);
    }
#endif
    return CAPABILITY_NOT_SUPPORTED_ERROR;
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

// Filters cookies matching cookieUrl (domain/path/secure/expiry). Used only by GetIncognitoCookiesForUrl.
static NSArray<NSHTTPCookie*>* FilterCookiesForUrl(NSArray<NSHTTPCookie*>* cookies, NSURL* cookieUrl)
{
    NSString* host = cookieUrl.host.lowercaseString ?: @"";
    NSString* requestPath = cookieUrl.path.length > 0 ? cookieUrl.path : @"/";
    BOOL isHttpsRequest = [cookieUrl.scheme.lowercaseString isEqualToString:@"https"];
    NSMutableArray<NSHTTPCookie*>* matchedCookies = [NSMutableArray array];
    for (NSHTTPCookie* cookie in cookies ?: @[]) {
        NSString* domain = cookie.domain.lowercaseString ?: @"";
        while ([domain hasPrefix:@"."]) {
            domain = [domain substringFromIndex:1];
        }
        BOOL domainMatches = [host isEqualToString:domain];
        if (!domainMatches && [cookie.domain hasPrefix:@"."]) {
            NSString* suffix = [NSString stringWithFormat:@".%@", domain];
            domainMatches = [host hasSuffix:suffix];
        }
        NSString* cookiePath = cookie.path.length > 0 ? cookie.path : @"/";
        BOOL pathMatches = [requestPath hasPrefix:cookiePath];
        BOOL secureMatches = !cookie.isSecure || isHttpsRequest;
        BOOL notExpired = cookie.expiresDate == nil || [cookie.expiresDate timeIntervalSinceNow] > 0;
        if (domainMatches && pathMatches && secureMatches && notExpired) {
            [matchedCookies addObject:cookie];
        }
    }
    return matchedCookies;
}

// Helper for FetchCookieSync incognito path.
static ErrCode GetIncognitoCookiesForUrl(NSURL* cookieUrl, NSMutableArray<NSHTTPCookie*>* cookies)
{
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_11_0
    if (@available(iOS 11.0, *)) {
        WKWebsiteDataStore* dataStore = CookieDataStore(true);
        if (dataStore == nil) {
            return CAPABILITY_NOT_SUPPORTED_ERROR;
        }
        __block NSArray<NSHTTPCookie*>* wkCookies = @[];
        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
        [dataStore.httpCookieStore getAllCookies:^(NSArray<NSHTTPCookie*>* allCookies) {
            wkCookies = [allCookies ?: @[] copy];
            dispatch_semaphore_signal(semaphore);
        }];
        if (!WaitForCookieTask(semaphore)) {
            return CAPABILITY_NOT_SUPPORTED_ERROR;
        }
        [cookies addObjectsFromArray:FilterCookiesForUrl(wkCookies, cookieUrl) ?: @[]];
        return NO_ERROR;
    }
#endif
    return CAPABILITY_NOT_SUPPORTED_ERROR;
}

// Formats cookies into a Cookie header string. Used only by FetchCookieSync.
static void SetCookieResult(NSArray<NSHTTPCookie*>* cookies, std::string& result)
{
    NSDictionary<NSString*, NSString*>* headers = [NSHTTPCookie requestHeaderFieldsWithCookies:cookies ?: @[]];
    NSString* cookieValue = headers[@"Cookie"];
    result = cookieValue == nil ? "" : [cookieValue UTF8String];
}

ErrCode WebCookieManagerIOS::FetchCookieSync(const std::string& url, bool incognito, std::string& result)
{
    NSURL* cookieUrl = CreateCookieUrl(url);
    if (cookieUrl == nil) {
        result = "";
        return INVALID_URL;
    }
    NSMutableArray<NSHTTPCookie*>* cookies = [NSMutableArray array];
    if (incognito) {
        ErrCode ret = GetIncognitoCookiesForUrl(cookieUrl, cookies);
        if (ret != NO_ERROR) {
            result = "";
            return ret;
        }
    } else {
        [cookies addObjectsFromArray:[NSHTTPCookieStorage.sharedHTTPCookieStorage cookiesForURL:cookieUrl] ?: @[]];
    }
    SetCookieResult(cookies, result);
    return NO_ERROR;
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

// Selects cookies to delete (all, or session-only). Used by the delete flow below.
static NSMutableArray<NSHTTPCookie*>* BuildCookiesToDelete(NSArray<NSHTTPCookie*>* cookies, bool sessionOnly)
{
    NSMutableArray<NSHTTPCookie*>* cookiesToDelete = [NSMutableArray array];
    for (NSHTTPCookie* cookie in cookies ?: @[]) {
        if (!sessionOnly || cookie.isSessionOnly) {
            [cookiesToDelete addObject:cookie];
        }
    }
    return cookiesToDelete;
}

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_11_0
// Decrements pendingCount; returns YES when it reaches zero. Used only by DeleteCookiesFromCookieList.
static BOOL CompleteCookieTask(const std::shared_ptr<std::atomic<NSUInteger>>& pendingCount)
{
    NSUInteger current = pendingCount->load();
    while (current > 0) {
        if (pendingCount->compare_exchange_weak(current, current - 1)) {
            return current == 1;
        }
    }
    return NO;
}

static void DeleteCookiesFromCookieList(WKHTTPCookieStore* cookieStore, NSArray<NSHTTPCookie*>* cookies,
    bool sessionOnly, dispatch_semaphore_t semaphore) API_AVAILABLE(ios(11.0))
{
    NSMutableArray<NSHTTPCookie*>* cookiesToDelete = BuildCookiesToDelete(cookies, sessionOnly);
    if (cookiesToDelete.count == 0) {
        dispatch_semaphore_signal(semaphore);
        return;
    }
    auto pendingCount = std::make_shared<std::atomic<NSUInteger>>(cookiesToDelete.count);
    for (NSHTTPCookie* cookie in cookiesToDelete) {
        [cookieStore deleteCookie:cookie completionHandler:^{
            if (CompleteCookieTask(pendingCount)) {
                dispatch_semaphore_signal(semaphore);
            }
        }];
    }
}

static ErrCode DeleteCookiesFromCookieStore(bool sessionOnly, bool incognito) API_AVAILABLE(ios(11.0))
{
    WKWebsiteDataStore* dataStore = CookieDataStore(incognito);
    if (dataStore == nil) {
        return CAPABILITY_NOT_SUPPORTED_ERROR;
    }
    WKHTTPCookieStore* cookieStore = dataStore.httpCookieStore;
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    [cookieStore getAllCookies:^(NSArray<NSHTTPCookie*>* cookies) {
        DeleteCookiesFromCookieList(cookieStore, cookies, sessionOnly, semaphore);
    }];
    return WaitForCookieTask(semaphore) ? NO_ERROR : CAPABILITY_NOT_SUPPORTED_ERROR;
}
#endif

static ErrCode DeleteCookiesFromStore(bool sessionOnly, bool incognito)
{
    if (!incognito) {
        NSArray<NSHTTPCookie*>* cookies = [NSHTTPCookieStorage.sharedHTTPCookieStorage.cookies copy];
        for (NSHTTPCookie* cookie in BuildCookiesToDelete(cookies, sessionOnly)) {
            [NSHTTPCookieStorage.sharedHTTPCookieStorage deleteCookie:cookie];
        }
        return NO_ERROR;
    }
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_11_0
    if (@available(iOS 11.0, *)) {
        return DeleteCookiesFromCookieStore(sessionOnly, incognito);
    }
#endif
    return incognito ? CAPABILITY_NOT_SUPPORTED_ERROR : NO_ERROR;
}

ErrCode WebCookieManagerIOS::ClearAllCookiesSync(bool incognito)
{
    return DeleteCookiesFromStore(false, incognito);
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

void WebCookieManagerIOS::ClearSessionCookieSync()
{
    (void)DeleteCookiesFromStore(true, false);
}

ErrCode WebCookieManagerIOS::SaveCookie(int32_t asyncCallbackInfoId)
{
    WebCookieManager::OnCookieManagerReceiveError(CAPABILITY_NOT_SUPPORTED_ERROR, asyncCallbackInfoId);
    return CAPABILITY_NOT_SUPPORTED_ERROR;
}

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= WEBKIT_IOS_VERSION_17_0
static ErrCode GetCookiePolicySync(WKWebsiteDataStore* dataStore, WKCookiePolicy& policy) API_AVAILABLE(ios(17.0))
{
    if (dataStore == nil) {
        return CAPABILITY_NOT_SUPPORTED_ERROR;
    }
    auto currentPolicy = std::make_shared<WKCookiePolicy>(WKCookiePolicyAllow);
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    [dataStore.httpCookieStore getCookiePolicy:^(WKCookiePolicy cookiePolicy) {
        *currentPolicy = cookiePolicy;
        dispatch_semaphore_signal(semaphore);
    }];
    BOOL waitResult = WaitForCookieTask(semaphore);
    if (!waitResult) {
        return CAPABILITY_NOT_SUPPORTED_ERROR;
    }
    policy = *currentPolicy;
    return NO_ERROR;
}

static ErrCode SetCookiePolicySync(WKWebsiteDataStore* dataStore, WKCookiePolicy policy) API_AVAILABLE(ios(17.0))
{
    if (dataStore == nil) {
        return CAPABILITY_NOT_SUPPORTED_ERROR;
    }
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    [dataStore.httpCookieStore setCookiePolicy:policy completionHandler:^{
        dispatch_semaphore_signal(semaphore);
    }];
    return WaitForCookieTask(semaphore) ? NO_ERROR : CAPABILITY_NOT_SUPPORTED_ERROR;
}
#endif

ErrCode WebCookieManagerIOS::PutAcceptCookieEnabled(bool accept)
{
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= WEBKIT_IOS_VERSION_17_0
    if (@available(iOS 17.0, *)) {
        WKCookiePolicy policy = accept ? WKCookiePolicyAllow : WKCookiePolicyDisallow;
        WKWebsiteDataStore* defaultDataStore = [WKWebsiteDataStore defaultDataStore];
        WKWebsiteDataStore* incognitoDataStore = CookieDataStore(true);
        if (defaultDataStore == nil || incognitoDataStore == nil) {
            return CAPABILITY_NOT_SUPPORTED_ERROR;
        }
        WKCookiePolicy previousDefaultPolicy = WKCookiePolicyAllow;
        WKCookiePolicy previousIncognitoPolicy = WKCookiePolicyAllow;
        ErrCode defaultPolicyRet = GetCookiePolicySync(defaultDataStore, previousDefaultPolicy);
        ErrCode incognitoPolicyRet = GetCookiePolicySync(incognitoDataStore, previousIncognitoPolicy);
        if (defaultPolicyRet != NO_ERROR || incognitoPolicyRet != NO_ERROR) {
            return CAPABILITY_NOT_SUPPORTED_ERROR;
        }
        ErrCode defaultRet = SetCookiePolicySync(defaultDataStore, policy);
        if (defaultRet != NO_ERROR) {
            return defaultRet;
        }
        ErrCode incognitoRet = SetCookiePolicySync(incognitoDataStore, policy);
        if (incognitoRet == NO_ERROR) {
            return NO_ERROR;
        }
        (void)SetCookiePolicySync(defaultDataStore, previousDefaultPolicy);
        (void)SetCookiePolicySync(incognitoDataStore, previousIncognitoPolicy);
        return incognitoRet;
    }
#endif
    return CAPABILITY_NOT_SUPPORTED_ERROR;
}

ErrCode WebCookieManagerIOS::PutAcceptThirdPartyCookieEnabled(bool accept)
{
    (void)accept;
    return CAPABILITY_NOT_SUPPORTED_ERROR;
}
} // namespace OHOS::Plugin
