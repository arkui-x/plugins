/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#import <CoreLocation/CoreLocation.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import <Foundation/Foundation.h>
#import <objc/runtime.h>
#import <UIKit/UIKit.h>

#include <algorithm>
#include <mutex>
#include <memory>
#include <vector>

#include "location.h"
#include "location_log.h"
#include "location_service_ios.h"
#include "location_napi_event.h"

static BOOL g_errorSubscribed = NO;
static BOOL g_switchSubscribed = NO;
static BOOL g_locationActive = NO;
static BOOL g_countryCodeSubscribed = NO;
static BOOL g_btScanActive = NO;
static NSString* g_lastCountry = nil;
static std::vector<OHOS::sptr<OHOS::Location::CountryCodeCallbackNapi>> g_countryCodeCallbacks;
static std::mutex g_countryCodeMutex;
static std::vector<OHOS::sptr<OHOS::Location::LocationSwitchCallbackNapi>> g_switchCallbacks;
static std::mutex g_switchMutex;
static std::vector<OHOS::sptr<OHOS::Location::LocationErrorCallbackNapi>> g_errorCallbacks;
static std::mutex g_errorMutex;
static std::vector<OHOS::sptr<OHOS::Location::BluetoothScanResultCallbackNapi>> g_btScanCallbacks;
static std::mutex g_btScanMutex;
static std::unique_ptr<OHOS::Location::BluetoothScanResult> g_lastBtScanResult;
static std::vector<OHOS::sptr<OHOS::Location::LocatorCallbackNapi>> g_locatorCallbacks;
static std::mutex g_locatorCallbacksMutex;
static int g_locationTimeIntervalSec = 0;
static int64_t g_lastLocationDispatchMs = 0;
static constexpr int64_t SEC_TO_NANOS = 1000000000LL;
@interface ArkUIBluetoothScanDelegate : NSObject<CBCentralManagerDelegate>
@property(nonatomic, strong) CBCentralManager *central;
- (void)startScanIfPossible;
- (void)stopScan;
@end
static ArkUIBluetoothScanDelegate *g_btDelegate = nil;

static void ArkUIDispatchBluetoothScanResult(const OHOS::Location::BluetoothScanResult &result);

static std::unique_ptr<OHOS::Location::Location> ArkUIBuildLocationFromCL(CLLocation *src)
{
    if (!src) {
        return nullptr;
    }
    auto locationObj = std::make_unique<OHOS::Location::Location>();
    locationObj->SetLatitude(src.coordinate.latitude);
    locationObj->SetLongitude(src.coordinate.longitude);
    locationObj->SetAccuracy((double)src.horizontalAccuracy);
    if (src.verticalAccuracy >= 0) {
        locationObj->SetAltitude(src.altitude);
        locationObj->SetAltitudeAccuracy((double)src.verticalAccuracy);
    }
    if (src.speed >= 0) {
        locationObj->SetSpeed((double)src.speed);
#ifdef __IPHONE_10_0
        if (@available(iOS 10.0, *)) {
            if (src.speedAccuracy >= 0) {
                locationObj->SetSpeedAccuracy((double)src.speedAccuracy);
            }
        }
#endif
    }
    if (src.course >= 0) {
        locationObj->SetDirection((double)src.course);
#ifdef __IPHONE_13_4
        if (@available(iOS 13.4, *)) {
            if (src.courseAccuracy >= 0) {
                locationObj->SetDirectionAccuracy((double)src.courseAccuracy);
            }
        }
#endif
    }
    if (src.floor != nil) {
        locationObj->SetFloorNo((int32_t)src.floor.level);
    }
    long ts = (long)([src.timestamp timeIntervalSince1970] * 1000);
    locationObj->SetTimeStamp(static_cast<int64_t>(ts));
    locationObj->SetIsFromMock(0);
    locationObj->SetLocationSourceType(2);
    NSTimeInterval uptimeSec = [NSProcessInfo processInfo].systemUptime;
    int64_t sinceBootNs = (int64_t)(uptimeSec * SEC_TO_NANOS);
    locationObj->SetTimeSinceBoot(sinceBootNs);
    return locationObj;
}

@interface ArkUILocationServiceDelegate : NSObject<CLLocationManagerDelegate>
@property(nonatomic, strong) CLLocationManager *manager;
@end

@implementation ArkUIBluetoothScanDelegate
- (instancetype)init
{
    self = [super init];
    if (self) {
        _central = [[CBCentralManager alloc] initWithDelegate:self queue:dispatch_get_main_queue()
            options:@{ CBCentralManagerOptionShowPowerAlertKey:@YES }];
    }
    return self;
}

- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
    switch (central.state) {
        case CBManagerStatePoweredOn:
            NSLog(@"Bluetooth central powered on");
            [self startScanIfPossible];
            break;
        case CBManagerStatePoweredOff:
            NSLog(@"Bluetooth central powered off");
            [self stopScan];
            break;
        default:
            NSLog(@"Bluetooth state=%ld", static_cast<long>(central.state));
            break;
    }
}

- (void)startScanIfPossible
{
    if (!_central || _central.state != CBManagerStatePoweredOn) {
        return;
    }
    NSDictionary *options = @{ CBCentralManagerScanOptionAllowDuplicatesKey:@YES };
    [_central scanForPeripheralsWithServices:nil options:options];
}

- (void)stopScan
{
    if (_central) {
        [_central stopScan];
    }
}

- (void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral
    advertisementData:(NSDictionary<NSString *,id> *)advertisementData RSSI:(NSNumber *)rssi
{
    OHOS::Location::BluetoothScanResult result;
    NSString *uuidStr = peripheral.identifier.UUIDString;
    if (uuidStr) {
        result.SetDeviceId([uuidStr UTF8String]);
    }
    NSString *name = peripheral.name;
    if (!name) {
        name = advertisementData[CBAdvertisementDataLocalNameKey];
    } else {
        result.SetDeviceName([name UTF8String]);
    }
    if (rssi) {
        result.SetRssi((int64_t)[rssi integerValue]);
    }
    NSNumber *connectable = advertisementData[CBAdvertisementDataIsConnectable];
    if (connectable) {
        result.SetConnectable([connectable boolValue]);
    }
    NSData *mfg = advertisementData[CBAdvertisementDataManufacturerDataKey];
    if (mfg && mfg.length > 0) {
        std::vector<uint8_t> bytes(
            static_cast<const uint8_t*>(mfg.bytes),
            static_cast<const uint8_t*>(mfg.bytes) + mfg.length);
        result.SetData(bytes);
    }
    g_lastBtScanResult = std::make_unique<OHOS::Location::BluetoothScanResult>(result);
    ArkUIDispatchBluetoothScanResult(result);
}
@end
static ArkUILocationServiceDelegate *g_delegate = nil;

@implementation ArkUILocationServiceDelegate
- (instancetype)init {
    self = [super init];
    if (self) {
        _manager = [[CLLocationManager alloc] init];
        _manager.desiredAccuracy = kCLLocationAccuracyBest;
        _manager.distanceFilter = kCLDistanceFilterNone;
        _manager.delegate = self;
    }
    return self;
}

- (void)locationManager:(CLLocationManager *)manager didUpdateLocations:(NSArray<CLLocation *> *)locations
{
    NSLog(@"didUpdateLocations");
    if (locations.count == 0) {
        return;
    }
    CLLocation *loc = [locations lastObject];
    if (g_locationTimeIntervalSec > 0) {
        long nowMs = (long)([[NSDate date] timeIntervalSince1970] * 1000);
        if (g_lastLocationDispatchMs > 0 && (nowMs - g_lastLocationDispatchMs) < (g_locationTimeIntervalSec * 1000)) {
            return;
        }
        g_lastLocationDispatchMs = nowMs;
    }
    std::vector<OHOS::sptr<OHOS::Location::LocatorCallbackNapi>> copy;
    {
        std::lock_guard<std::mutex> lock(g_locatorCallbacksMutex);
        copy = g_locatorCallbacks;
    }
    if (copy.empty()) {
        return;
    }
    for (auto &host : copy) {
        if (!host) {
            continue;
        }
        auto locationObj = ArkUIBuildLocationFromCL(loc);
        if (!locationObj) {
            continue;
        }
        host->OnLocationReport(locationObj);
    }
}

- (void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error
{
    NSLog(@"didFailWithError: %ld", static_cast<long>(error.code));
    if (g_errorSubscribed) {
        std::vector<OHOS::sptr<OHOS::Location::LocationErrorCallbackNapi>> copy;
        {
            std::lock_guard<std::mutex> lock(g_errorMutex);
            copy = g_errorCallbacks;
        }
        if (!copy.empty()) {
            for (auto &cb : copy) {
                if (cb) {
                    cb->OnErrorReport((int)error.code);
                }
            }
        }
    }
}

- (void)locationManagerDidChangeAuthorization:(CLLocationManager *)manager API_AVAILABLE(ios(14.0))
{
    if (!g_switchSubscribed) {
        return;
    }
    CLAuthorizationStatus s = manager.authorizationStatus;
    int state = (s == kCLAuthorizationStatusDenied || s == kCLAuthorizationStatusRestricted) ? 0 : 1;
    std::vector<OHOS::sptr<OHOS::Location::LocationSwitchCallbackNapi>> copy;
    {
        std::lock_guard<std::mutex> lock(g_switchMutex);
        copy = g_switchCallbacks;
    }
    if (!copy.empty()) {
        for (auto &cb : copy) {
            if (cb) {
                cb->OnSwitchChange(state);
            }
        }
    }
}

- (void)locationManager:(CLLocationManager *)manager didChangeAuthorizationStatus:(CLAuthorizationStatus)status
{
    NSLog(@"didChangeAuthorizationStatus");
    if (!g_switchSubscribed) {
        return;
    }
    NSLog(@"Authorization changed (legacy) status=%d", (int)status);
    int state = (status == kCLAuthorizationStatusDenied || status == kCLAuthorizationStatusRestricted) ? 0 : 1;
    std::vector<OHOS::sptr<OHOS::Location::LocationSwitchCallbackNapi>> copy;
    {
        std::lock_guard<std::mutex> lock(g_switchMutex);
        copy = g_switchCallbacks;
    }
    if (!copy.empty()) {
        for (auto &cb : copy) {
            if (cb) {
                cb->OnSwitchChange(state);
            }
        }
    }
}
@end

void ArkUILocationServiceStart()
{
    if (!g_delegate) {
        g_delegate = [[ArkUILocationServiceDelegate alloc] init];
    }
    if (!g_locationActive) {
        [g_delegate.manager startUpdatingLocation];
        g_locationActive = YES;
    }
}

void ArkUIAddLocatorCallback(void* locatorCallbackHost)
{
    if (!locatorCallbackHost) {
        return;
    }
    auto hostPtrAddr = static_cast<OHOS::sptr<OHOS::Location::LocatorCallbackNapi>*>(locatorCallbackHost);
    if (!hostPtrAddr || !(*hostPtrAddr)) {
        return;
    }
    auto hostCopy = *hostPtrAddr;
    std::lock_guard<std::mutex> lock(g_locatorCallbacksMutex);
    for (auto &existing : g_locatorCallbacks) {
        if (existing.GetRefPtr() == hostCopy.GetRefPtr()) {
            return;
        }
    }
    bool wasEmpty = g_locatorCallbacks.empty();
    g_locatorCallbacks.push_back(hostCopy);
    if (wasEmpty && !g_locationActive && g_delegate) {
        [g_delegate.manager startUpdatingLocation];
        g_locationActive = YES;
    }
}

int32_t ArkUIRequestSingleLocation(void* locatorCallbackHost)
{
    if (!locatorCallbackHost) {
        return 1;
    }
    auto hostPtrAddr = static_cast<OHOS::sptr<OHOS::Location::LocatorCallbackNapi>*>(locatorCallbackHost);
    if (!hostPtrAddr || !(*hostPtrAddr)) {
        return 1;
    }
    auto hostCopy = *hostPtrAddr;
    if (!g_delegate) {
        g_delegate = [[ArkUILocationServiceDelegate alloc] init];
    }
    CLLocation *cached = g_delegate.manager.location;
    [g_delegate.manager startUpdatingLocation];
    g_locationActive = YES;
    if (cached) {
        auto locationObj = ArkUIBuildLocationFromCL(cached);
        hostCopy->OnLocationReport(locationObj);
        hostCopy->SetSingleLocation(locationObj);
        return 0;
    } else {
        return 1;
    }
}

void ArkUIRemoveLocatorCallbackByHost(void* locatorCallbackHost)
{
    if (!locatorCallbackHost) {
        return;
    }
    auto hostPtrAddr = static_cast<OHOS::sptr<OHOS::Location::LocatorCallbackNapi>*>(locatorCallbackHost);
    if (!hostPtrAddr || !(*hostPtrAddr)) {
        return;
    }
    auto target = *hostPtrAddr;
    bool shouldStop = false;
    {
        std::lock_guard<std::mutex> lock(g_locatorCallbacksMutex);
        auto &vec = g_locatorCallbacks;
        vec.erase(std::remove_if(vec.begin(), vec.end(),
            [&](const OHOS::sptr<OHOS::Location::LocatorCallbackNapi> &item) {
            return item && item.GetRefPtr() == target.GetRefPtr();
        }), vec.end());
        shouldStop = vec.empty();
    }
    if (shouldStop && g_delegate && g_locationActive) {
        [g_delegate.manager stopUpdatingLocation];
        g_locationActive = NO;
    }
}

static id g_localeObserver = nil;
static id g_appActiveObserver = nil;

static void ArkUIMaybeDispatchCountryChange()
{
    NSString* newCountry = [[NSLocale currentLocale] objectForKey:NSLocaleCountryCode];
    if (!newCountry) {
        return;
    }
    if (g_lastCountry && [newCountry isEqualToString:g_lastCountry]) {
        return;
    }
    g_lastCountry = [newCountry copy];
    std::string cc([newCountry UTF8String]);
    for (auto &c : cc) {
        c = static_cast<char>(::toupper(static_cast<unsigned char>(c)));
    }
    std::vector<OHOS::sptr<OHOS::Location::CountryCodeCallbackNapi>> copy;
    {
        std::lock_guard<std::mutex> lock(g_countryCodeMutex);
        copy = g_countryCodeCallbacks;
    }
    if (copy.empty()) {
        return;
    }
    auto countryObj = std::make_shared<OHOS::Location::CountryCode>();
    countryObj->SetCountryCodeStr(cc);
    countryObj->SetCountryCodeType(1);
    for (auto &cb : copy) {
        if (cb) {
            cb->OnCountryCodeChange(countryObj);
        }
    }
}

void ArkUIRegisterCountryCodeObserver()
{
    if (g_countryCodeSubscribed) {
        return;
    }
    g_countryCodeSubscribed = YES;
    g_lastCountry = [[[NSLocale currentLocale] objectForKey:NSLocaleCountryCode] copy];
    g_localeObserver = [[NSNotificationCenter defaultCenter]
        addObserverForName:NSCurrentLocaleDidChangeNotification object:nil queue:[NSOperationQueue mainQueue]
        usingBlock:^(NSNotification * _Nonnull note) {
        ArkUIMaybeDispatchCountryChange();
    }];
    g_appActiveObserver = [[NSNotificationCenter defaultCenter]
        addObserverForName:UIApplicationDidBecomeActiveNotification object:nil queue:[NSOperationQueue mainQueue]
        usingBlock:^(NSNotification * _Nonnull note) {
        ArkUIMaybeDispatchCountryChange();
    }];
}

void ArkUIUnregisterCountryCodeObserver()
{
    if (!g_countryCodeSubscribed) {
        return;
    }
    if (g_localeObserver) {
        [[NSNotificationCenter defaultCenter] removeObserver:g_localeObserver];
        g_localeObserver = nil;
    }
    if (g_appActiveObserver) {
        [[NSNotificationCenter defaultCenter] removeObserver:g_appActiveObserver];
        g_appActiveObserver = nil;
    }
    g_countryCodeSubscribed = NO;
}

void ArkUIAddCountryCodeCallback(void* callbackHost)
{
    if (!callbackHost) {
        return;
    }
    auto hostPtrAddr = static_cast<OHOS::sptr<OHOS::Location::CountryCodeCallbackNapi>*>(callbackHost);
    if (!hostPtrAddr || !(*hostPtrAddr)) {
        return;
    }
    auto hostCopy = *hostPtrAddr;
    bool needObserver = false;
    {
        std::lock_guard<std::mutex> lock(g_countryCodeMutex);
        for (auto &existing : g_countryCodeCallbacks) {
            if (existing.GetRefPtr() == hostCopy.GetRefPtr()) {
                return;
            }
        }
        needObserver = g_countryCodeCallbacks.empty();
        g_countryCodeCallbacks.push_back(hostCopy);
    }
    if (needObserver) {
        ArkUIRegisterCountryCodeObserver();
    }
}

void ArkUIRemoveCountryCodeCallbackByHost(void* callbackHost)
{
    if (!callbackHost) {
        return;
    }
    auto hostPtrAddr = static_cast<OHOS::sptr<OHOS::Location::CountryCodeCallbackNapi>*>(callbackHost);
    if (!hostPtrAddr || !(*hostPtrAddr)) {
        return;
    }

    auto target = *hostPtrAddr;
    bool becameEmpty = false;
    {
        std::lock_guard<std::mutex> lock(g_countryCodeMutex);
        auto &vec = g_countryCodeCallbacks;
        vec.erase(std::remove_if(vec.begin(), vec.end(),
            [&](const OHOS::sptr<OHOS::Location::CountryCodeCallbackNapi> &item) {
            return item && item.GetRefPtr() == target.GetRefPtr();
        }), vec.end());
        becameEmpty = vec.empty();
    }
    if (becameEmpty) {
        ArkUIUnregisterCountryCodeObserver();
    }
}

void ArkUIAddLocationErrorCallback(void* callbackHost)
{
    if (!callbackHost) {
        return;
    }
    auto hostPtrAddr = static_cast<OHOS::sptr<OHOS::Location::LocationErrorCallbackNapi>*>(callbackHost);
    if (!hostPtrAddr || !(*hostPtrAddr)) {
        return;
    }
    auto hostCopy = *hostPtrAddr;
    bool needActivate = false;
    {
        std::lock_guard<std::mutex> lock(g_errorMutex);
        for (auto &existing : g_errorCallbacks) {
            if (existing.GetRefPtr() == hostCopy.GetRefPtr()) {
                return;
            }
        }
        needActivate = g_errorCallbacks.empty();
        g_errorCallbacks.push_back(hostCopy);
    }
    if (needActivate) {
        g_errorSubscribed = YES;
    }
}

void ArkUIRemoveLocationErrorCallbackByHost(void* callbackHost)
{
    if (!callbackHost) {
        return;
    }
    auto hostPtrAddr = static_cast<OHOS::sptr<OHOS::Location::LocationErrorCallbackNapi>*>(callbackHost);
    if (!hostPtrAddr || !(*hostPtrAddr)) {
        return;
    }
    auto target = *hostPtrAddr;
    bool emptyNow = false;
    {
        std::lock_guard<std::mutex> lock(g_errorMutex);
        auto &vec = g_errorCallbacks;
        vec.erase(std::remove_if(vec.begin(), vec.end(),
            [&](const OHOS::sptr<OHOS::Location::LocationErrorCallbackNapi> &item) {
            return item && item.GetRefPtr() == target.GetRefPtr();
        }), vec.end());
        emptyNow = vec.empty();
    }
    if (emptyNow) {
        g_errorSubscribed = NO;
    }
}

void ArkUIRegisterSwitchCallback()
{
    g_switchSubscribed = YES;
}

void ArkUIAddSwitchCallback(void* callbackHost)
{
    if (!callbackHost) {
        return;
    }
    auto hostPtrAddr = static_cast<OHOS::sptr<OHOS::Location::LocationSwitchCallbackNapi>*>(callbackHost);
    if (!hostPtrAddr || !(*hostPtrAddr)) {
        return;
    }
    auto hostCopy = *hostPtrAddr;
    bool needActivate = false;
    {
        std::lock_guard<std::mutex> lock(g_switchMutex);
        for (auto &existing : g_switchCallbacks) {
            if (existing.GetRefPtr() == hostCopy.GetRefPtr()) {
                return;
            }
        }
        needActivate = g_switchCallbacks.empty();
        g_switchCallbacks.push_back(hostCopy);
    }
    if (needActivate) {
        g_switchSubscribed = YES;
    }
}

void ArkUIRemoveSwitchCallbackByHost(void* callbackHost)
{
    if (!callbackHost) {
        return;
    }
    auto hostPtrAddr = static_cast<OHOS::sptr<OHOS::Location::LocationSwitchCallbackNapi>*>(callbackHost);
    if (!hostPtrAddr || !(*hostPtrAddr)) {
        return;
    }
    auto target = *hostPtrAddr;
    bool emptyNow = false;
    {
        std::lock_guard<std::mutex> lock(g_switchMutex);
        auto &vec = g_switchCallbacks;
        vec.erase(std::remove_if(vec.begin(), vec.end(),
            [&](const OHOS::sptr<OHOS::Location::LocationSwitchCallbackNapi> &item) {
            return item && item.GetRefPtr() == target.GetRefPtr();
        }), vec.end());
        emptyNow = vec.empty();
    }
    if (emptyNow) {
        g_switchSubscribed = NO;
    }
}

void ArkUIAddBluetoothScanResultCallback(void* callbackHost)
{
    if (!callbackHost) {
        return;
    }
    auto hostPtrAddr = static_cast<OHOS::sptr<OHOS::Location::BluetoothScanResultCallbackNapi>*>(callbackHost);
    if (!hostPtrAddr || !(*hostPtrAddr)) {
        return;
    }
    auto hostCopy = *hostPtrAddr;
    bool needStart = false;
    {
        std::lock_guard<std::mutex> lock(g_btScanMutex);
        for (auto &existing : g_btScanCallbacks) {
            if (existing.GetRefPtr() == hostCopy.GetRefPtr()) {
                return;
            }
        }
        needStart = g_btScanCallbacks.empty();
        g_btScanCallbacks.push_back(hostCopy);
    }
    if (needStart && !g_btScanActive) {
        g_btScanActive = YES;
        if (!g_btDelegate) {
            g_btDelegate = [[ArkUIBluetoothScanDelegate alloc] init];
        }
        [g_btDelegate startScanIfPossible];
    }
    if (g_lastBtScanResult) {
        if (hostPtrAddr && *hostPtrAddr) {
            auto singleCb = *hostPtrAddr;
            auto resPtr = std::make_unique<OHOS::Location::BluetoothScanResult>(*g_lastBtScanResult);
            singleCb->OnBluetoothScanResultChange(resPtr);
        }
    }
}

void ArkUIRemoveBluetoothScanResultCallbackByHost(void* callbackHost)
{
    if (!callbackHost) {
        return;
    }
    auto hostPtrAddr = static_cast<OHOS::sptr<OHOS::Location::BluetoothScanResultCallbackNapi>*>(callbackHost);
    if (!hostPtrAddr || !(*hostPtrAddr)) {
        return;
    }
    auto target = *hostPtrAddr;
    bool emptyNow = false;
    {
        std::lock_guard<std::mutex> lock(g_btScanMutex);
        auto &vec = g_btScanCallbacks;
        vec.erase(std::remove_if(vec.begin(), vec.end(),
            [&](const OHOS::sptr<OHOS::Location::BluetoothScanResultCallbackNapi> &item) {
            return item && item.GetRefPtr() == target.GetRefPtr();
        }), vec.end());
        emptyNow = vec.empty();
    }
    if (emptyNow && g_btScanActive) {
        if (g_btDelegate) {
            [g_btDelegate stopScan];
        }
        g_btScanActive = NO;
    }
}

static void ArkUIDispatchBluetoothScanResult(const OHOS::Location::BluetoothScanResult &result)
{
    std::vector<OHOS::sptr<OHOS::Location::BluetoothScanResultCallbackNapi>> copy;
    {
        std::lock_guard<std::mutex> lock(g_btScanMutex);
        copy = g_btScanCallbacks;
    }
    if (copy.empty()) {
        return;
    }
    for (auto &cb : copy) {
        if (!cb) {
            continue;
        }
        auto resPtr = std::make_unique<OHOS::Location::BluetoothScanResult>(const_cast<OHOS::Location::BluetoothScanResult&>(result));
        cb->OnBluetoothScanResultChange(resPtr);
    }
}

static void ArkUIReserverseConfiguretoDesire(CLLocationAccuracy desired, int scenario, int priority, float maxAccuracy)
{
    switch (priority) {
        case 0x0202:
            desired = kCLLocationAccuracyHundredMeters;
            break;
        case 0x0203:
            desired = kCLLocationAccuracyKilometer;
            break;
        case 0x0201:
            desired = kCLLocationAccuracyBest;
            break;
        default:
            break;
    }
    switch (scenario) {
        case 0x0301:
            desired = kCLLocationAccuracyBestForNavigation;
            break;
        case 0x0304:
            if (desired == kCLLocationAccuracyBest) {
                desired = kCLLocationAccuracyNearestTenMeters;
            }
            break;
        case 0x0305:
            desired = kCLLocationAccuracyThreeKilometers;
            break;
        default:
            break;
    }
    if (maxAccuracy > 0) {
        if (maxAccuracy <= 10) {
            desired = kCLLocationAccuracyNearestTenMeters;
        } else if (maxAccuracy <= 50) {
            desired = kCLLocationAccuracyHundredMeters;
        } else if (maxAccuracy > 1000) {
            desired = kCLLocationAccuracyThreeKilometers;
        }
    }
}

void ArkUIConfigureLocation(int scenario, int priority, int timeInterval,
    double distanceInterval, float maxAccuracy)
{
    if (!g_delegate) {
        g_delegate = [[ArkUILocationServiceDelegate alloc] init];
    }
    CLLocationManager *mgr = g_delegate.manager;
    if (!mgr) {
        return;
    }
    CLLocationAccuracy desired = kCLLocationAccuracyBest;
    ArkUIReserverseConfiguretoDesire(desired, scenario, priority, maxAccuracy);
    mgr.desiredAccuracy = desired;
    if (distanceInterval > 0) {
        mgr.distanceFilter = distanceInterval;
    } else {
        mgr.distanceFilter = kCLDistanceFilterNone;
    }
    if (timeInterval > 0) {
        g_locationTimeIntervalSec = timeInterval;
    } else {
        g_locationTimeIntervalSec = 0;
    }
}
