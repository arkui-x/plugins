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

#import <Foundation/Foundation.h>
#import <CoreLocation/CoreLocation.h>
#import <SystemConfiguration/CaptiveNetwork.h>
#ifdef NOTIFICATION_ENABLE
#import <UserNotifications/UserNotifications.h>
#endif

#include <cctype>
#include <mutex>
#include <map>
#include <memory>
#include <list>
#include <string>
#include <vector>

#include "country_code.h"
#include "geofence_definition.h"
#include "location.h"
#include "location_crossplatform_service_impl.h"
#include "location_gnss_geofence_callback_napi.h"
#include "location_log.h"
#include "location_service_ios.h"
#if !defined(PLUGIN_INTERFACE_NATIVE_LOG_H)
#define LogLevel GEOLOC_PLUGIN_LOGLEVEL_REMAED__
#define GEOLOC_PLUGIN_LOGLEVEL_RENAMED
#endif
#include "notification_request.h"
#include "notification_helper.h"
#ifdef GEOLOC_PLUGIN_LOGLEVEL_RENAMED
#undef LogLevel
#endif

static std::mutex g_geofenceCbMutex;
static std::map<int32_t, OHOS::sptr<OHOS::Location::LocationGnssGeofenceCallbackNapi>> g_geofenceCallbackMap;
static std::map<int32_t, std::vector<std::shared_ptr<OHOS::Notification::NotificationRequest>>>
    g_geofenceNotifications;
static std::mutex g_geofenceNotiMutex;
static NSArray<CLPlacemark*> *g_lastReversePlacemarkList = nil;
static NSArray<CLPlacemark*> *g_lastForwardPlacemarkList = nil;
static const int32_t FENCE_MAX_ID = 20;
static int32_t g_fenceIdCounter = 0;
static std::mutex g_fenceIdMutex;
static bool g_fenceIdLoaded = false;
static constexpr int64_t MILL_TO_NANOS = 1000000LL;

static inline int64_t NowSinceBootNs()
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}

#ifdef NOTIFICATION_ENABLE
static void RequestNotificationAuthorizationIfNeeded()
{
    if (!@available(iOS 10.0, *)) {
        return;
    }
    auto execBlock = ^{
        UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
        [center getNotificationSettingsWithCompletionHandler:^(UNNotificationSettings * _Nonnull settings) {
            if (!(settings && settings.authorizationStatus == UNAuthorizationStatusNotDetermined)) {
                return;
            }
            UNAuthorizationOptions opts = (UNAuthorizationOptions)(
                UNAuthorizationOptionAlert | UNAuthorizationOptionSound | UNAuthorizationOptionBadge);
            [center requestAuthorizationWithOptions:opts completionHandler:nil];
        }];
    };
    if ([NSThread isMainThread]) {
        execBlock();
        return;
    }
    dispatch_async(dispatch_get_main_queue(), execBlock);
}
#endif

static void RegisterGeofenceCallbackIOS(int32_t fenceId,
    OHOS::sptr<OHOS::Location::LocationGnssGeofenceCallbackNapi> cb)
{
    if (fenceId <= 0 || cb == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_geofenceCbMutex);
    g_geofenceCallbackMap[fenceId] = cb;
}

static OHOS::sptr<OHOS::Location::LocationGnssGeofenceCallbackNapi> GetGeofenceCallbackIOS(int32_t fenceId)
{
    std::lock_guard<std::mutex> lock(g_geofenceCbMutex);
    auto it = g_geofenceCallbackMap.find(fenceId);
    if (it != g_geofenceCallbackMap.end()) {
        return it->second;
    }
    return nullptr;
}

static void UnregisterGeofenceCallbackIOS(int32_t fenceId)
{
    if (fenceId <= 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_geofenceCbMutex);
    g_geofenceCallbackMap.erase(fenceId);
}

static void StoreFenceNotifications(int32_t fenceId,
    const std::vector<std::shared_ptr<OHOS::Notification::NotificationRequest>> &list)
{
    if (fenceId <= 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_geofenceNotiMutex);
    if (list.empty()) {
        g_geofenceNotifications.erase(fenceId);
    } else {
        g_geofenceNotifications[fenceId] = list;
    }
}

static std::vector<std::shared_ptr<OHOS::Notification::NotificationRequest>> GetFenceNotifications(int32_t fenceId)
{
    std::lock_guard<std::mutex> lock(g_geofenceNotiMutex);
    auto it = g_geofenceNotifications.find(fenceId);
    if (it != g_geofenceNotifications.end()) {
        return it->second;
    }
    return {};
}

static void RemoveFenceNotifications(int32_t fenceId)
{
    std::lock_guard<std::mutex> lock(g_geofenceNotiMutex);
    g_geofenceNotifications.erase(fenceId);
}

#ifdef NOTIFICATION_ENABLE
static void PublishFenceNotificationNow(const std::shared_ptr<OHOS::Notification::NotificationRequest>& reqPtr)
{
    if (!reqPtr) {
        return;
    }
    OHOS::Notification::NotificationHelper::PublishNotification(
        *reqPtr,
        nullptr,
        [](void* /*data*/, int32_t code) {
            NSLog(@"ios fence notification publish result=%d", code);
        }
    );
}
#endif

// Helper to load persisted fence id; isolated to keep GenerateFenceIdIOS nesting shallow.
static inline void ArkUILoadFenceIdIfNeeded() {
    if (g_fenceIdLoaded) { return; }
    @autoreleasepool {
        NSNumber *saved = [[NSUserDefaults standardUserDefaults] objectForKey:@"arkui_geo_fence_id_counter"];
        if (saved && [saved isKindOfClass:[NSNumber class]]) {
            int64_t v = [saved longLongValue];
            if (v >= 0 && v <= FENCE_MAX_ID) {
                g_fenceIdCounter = static_cast<int32_t>(v);
            }
        }
    }
    g_fenceIdLoaded = true;
}

static int32_t GenerateFenceIdIOS()
{
    std::lock_guard<std::mutex> lock(g_fenceIdMutex);
    ArkUILoadFenceIdIfNeeded();
    if (g_fenceIdCounter >= FENCE_MAX_ID) { g_fenceIdCounter = 0; }
    ++g_fenceIdCounter;
    @autoreleasepool {
        [[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithLongLong:g_fenceIdCounter]
            forKey:@"arkui_geo_fence_id_counter"];
        [[NSUserDefaults standardUserDefaults] synchronize];
    }
    return g_fenceIdCounter;
}

@interface GeoLocationHelper : NSObject <CLLocationManagerDelegate>
- (instancetype)init;
- (CLLocationManager*)manager;
- (void)ensureAuth;
- (BOOL)addRegionWithFenceId:(int)fenceId latitude:(double)lat longitude:(double)lon radius:(double)radius expirationMs:(int64_t)expirationMs;
- (BOOL)removeRegionWithFenceId:(int)fenceId;
@end

@implementation GeoLocationHelper {
    CLLocationManager* _manager;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _manager = [[CLLocationManager alloc] init];
        _manager.delegate = (id<CLLocationManagerDelegate>)self;
        _manager.desiredAccuracy = kCLLocationAccuracyBest;
    }
    return self;
}

- (CLLocationManager*)manager {
    return _manager;
}

- (void)ensureAuth {
    dispatch_async(dispatch_get_main_queue(), ^{
        CLAuthorizationStatus s;
        if (@available(iOS 14.0, *)) {
            s = _manager.authorizationStatus;
        } else {
            s = [CLLocationManager authorizationStatus];
        }
        if (s == kCLAuthorizationStatusNotDetermined) {
            NSLog(@"No location auth, need request");
        }
    });
}

- (BOOL)addRegionWithFenceId:(int)fenceId latitude:(double)lat longitude:(double)lon radius:(double)radius expirationMs:(int64_t)expirationMs {
    if (!_manager) {
        return NO;
    }
    NSString* identifier = [NSString stringWithFormat:@"arkui_geofence_%d", fenceId];
    for (CLRegion* r in _manager.monitoredRegions) {
        if ([r.identifier isEqualToString:identifier]) {
            return YES;
        }
    }
    CLLocationCoordinate2D center; center.latitude = lat; center.longitude = lon;
    CLLocationDistance effectiveRadius = radius > 100000 ? 100000 : radius;
    CLCircularRegion* region = [[CLCircularRegion alloc] initWithCenter:center radius:effectiveRadius identifier:identifier];
    region.notifyOnEntry = YES;
    region.notifyOnExit = YES;
    [_manager startMonitoringForRegion:region];
    if (expirationMs > 0) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, expirationMs * NSEC_PER_MSEC), dispatch_get_main_queue(), ^{
            [self removeRegionWithFenceId:fenceId];
            RemoveFenceNotifications(fenceId);
        });
    }
    return YES;
}

- (BOOL)removeRegionWithFenceId:(int)fenceId {
    if (!_manager) {
        return NO;
    }
    NSString* identifier = [NSString stringWithFormat:@"arkui_geofence_%d", fenceId];
    CLRegion* target = nil;
    for (CLRegion* r in _manager.monitoredRegions) {
        if ([r.identifier isEqualToString:identifier]) {
            target = r;
            break;
        }
    }
    if (target) {
        [_manager stopMonitoringForRegion:target];
        UnregisterGeofenceCallbackIOS(fenceId);
        return YES;
    }
    return NO;
}

- (void)locationManager:(CLLocationManager *)manager didEnterRegion:(CLRegion *)region {
    NSLog(@"Geofence enter: %s", region.identifier.UTF8String);
    NSString *ident = region.identifier;
    if ([ident hasPrefix:@"arkui_geofence_"]) {
        NSString *numStr = [ident substringFromIndex:[@"arkui_geofence_" length]];
        int fenceId = [numStr intValue];
        auto cb = GetGeofenceCallbackIOS(fenceId);
        if (cb) {
            OHOS::Location::GeofenceTransition transition;
            transition.fenceId = fenceId;
            transition.event = OHOS::Location::GEOFENCE_TRANSITION_EVENT_ENTER;
            transition.beaconFence = nullptr;
            cb->OnTransitionStatusChange(transition);
        }
#ifdef NOTIFICATION_ENABLE
        auto list = GetFenceNotifications(fenceId);
        if (!list.empty()) {
            for (auto &n : list) {
                PublishFenceNotificationNow(n);
            }
        }
#endif
    }
}

- (void)locationManager:(CLLocationManager *)manager didExitRegion:(CLRegion *)region {
    NSLog(@"Geofence exit: %s", region.identifier.UTF8String);
    NSString *ident = region.identifier;
    if ([ident hasPrefix:@"arkui_geofence_"]) {
        NSString *numStr = [ident substringFromIndex:[@"arkui_geofence_" length]];
        int fenceId = [numStr intValue];
        auto cb = GetGeofenceCallbackIOS(fenceId);
        if (cb) {
            OHOS::Location::GeofenceTransition transition;
            transition.fenceId = fenceId;
            transition.event = OHOS::Location::GEOFENCE_TRANSITION_EVENT_EXIT;
            transition.beaconFence = nullptr;
            cb->OnTransitionStatusChange(transition);
        }
#ifdef NOTIFICATION_ENABLE
        auto list = GetFenceNotifications(fenceId);
        if (!list.empty()) {
            for (auto &n : list) {
                PublishFenceNotificationNow(n);
            }
        }
#endif
    }
}
@end

class GeoLocationIosRuntimeWrapper {
public:
    static GeoLocationIosRuntimeWrapper* Instance()
    {
        static GeoLocationIosRuntimeWrapper inst;
        return &inst;
    }

    CLLocationManager* GetManager()
    {
        return [_helper manager];
    }

    void EnsureAuth()
    {
        [_helper ensureAuth];
    }

    bool AddRegion(int fenceId, double lat, double lon, double radius, int64_t expirationMs)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return (bool)[_helper addRegionWithFenceId:fenceId latitude:lat longitude:lon radius:radius expirationMs:expirationMs];
    }

    bool RemoveRegion(int fenceId)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return (bool)[_helper removeRegionWithFenceId:fenceId];
    }

private:
    GeoLocationIosRuntimeWrapper()
    {
        _helper = [[GeoLocationHelper alloc] init];
    }
    ~GeoLocationIosRuntimeWrapper() = default;

    GeoLocationHelper* _helper = nil;
    std::mutex mutex_;
};

using GeoLocationIosRuntime = GeoLocationIosRuntimeWrapper;
using OHOS::Location::LocationErrCode;
namespace OHOS {
namespace Plugin {
std::shared_ptr<LocationCrossplatformService> LocationCrossplatformService::Get()
{
    GeoLocationIosRuntime::Instance()->EnsureAuth();
#ifdef NOTIFICATION_ENABLE
    RequestNotificationAuthorizationIfNeeded();
#endif
    return std::make_shared<LocatorCrossplatformServiceIosImpl>();
}

bool CheckLocationPermission()
{
    CLAuthorizationStatus status = [CLLocationManager authorizationStatus];
    return (status == kCLAuthorizationStatusAuthorizedWhenInUse ||
            status == kCLAuthorizationStatusAuthorizedAlways);
}

LocationErrCode LocatorCrossplatformServiceIosImpl::GetSwitchState(int32_t &state)
{
    NSLog(@"Get switch state");
    CLAuthorizationStatus status = [CLLocationManager authorizationStatus];
    if (status == kCLAuthorizationStatusNotDetermined) {
        state = Location::DEFAULT_SWITCH_STATE;
        return LocationErrCode::ERRCODE_SUCCESS;
    }
    if (status == kCLAuthorizationStatusDenied || status == kCLAuthorizationStatusRestricted) {
        state = Location::DISABLED;
        return LocationErrCode::ERRCODE_PERMISSION_DENIED;
    }
    state = Location::ENABLED;
    return LocationErrCode::ERRCODE_SUCCESS;
}

LocationErrCode LocatorCrossplatformServiceIosImpl::StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
    sptr<Location::LocatorCallbackNapi>& locatorCallbackHost)
{
    NSLog(@"Start locating");
    if (!CheckLocationPermission()) {
        NSLog(@"no permissions");
        return LocationErrCode::ERRCODE_PERMISSION_DENIED;
    }
    if (!locatorCallbackHost) {
        return LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    int32_t state = 0;
    GetSwitchState(state);
    if (state != 1) {
        return LocationErrCode::ERRCODE_SWITCH_OFF;
    }
    if (!requestConfig) {
        ArkUIConfigureLocation(0, 0, 0, 0.0, 0.0f);
        ArkUIAddLocatorCallback(&locatorCallbackHost);
        ArkUILocationServiceStart();
        return LocationErrCode::ERRCODE_SUCCESS;
    }
    int scenario = requestConfig->GetScenario();
    int priority = requestConfig->GetPriority();
    int timeInterval = requestConfig->GetTimeInterval();
    double distanceInterval = requestConfig->GetDistanceInterval();
    float maxAccuracy = requestConfig->GetMaxAccuracy();
    ArkUIConfigureLocation(scenario, priority, timeInterval, distanceInterval, maxAccuracy);
    int fixNumber = requestConfig->GetFixNumber();
    if (fixNumber == 1) {
        int ret = ArkUIRequestSingleLocation(&locatorCallbackHost);
        if (ret != 0) {
            return LocationErrCode::ERRCODE_LOCATING_FAIL;
        }
    } else {
        ArkUIAddLocatorCallback(&locatorCallbackHost);
        ArkUILocationServiceStart();
    }
    return LocationErrCode::ERRCODE_SUCCESS;
}

LocationErrCode LocatorCrossplatformServiceIosImpl::StopLocating(
    sptr<Location::LocatorCallbackNapi>& locatorCallbackHost)
{
    NSLog(@"Stop locating");
    if (!CheckLocationPermission()) {
        NSLog(@"no permissions");
        return LocationErrCode::ERRCODE_PERMISSION_DENIED;
    }
    if (!locatorCallbackHost) {
        return LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    ArkUIRemoveLocatorCallbackByHost(&locatorCallbackHost);
    return LocationErrCode::ERRCODE_SUCCESS;
}

LocationErrCode LocatorCrossplatformServiceIosImpl::IsGeoConvertAvailable(bool &isAvailable)
{
    NSLog(@"Is geo convert available");
    isAvailable = true;
    return LocationErrCode::ERRCODE_SUCCESS;
}

static CLCircularRegion* BuildRegion(const Location::GeoCodeRequest &req)
{
    bool hasBounds = (req.minLatitude_ != 0 || req.minLongitude_ != 0 || req.maxLatitude_ != 0 || req.maxLongitude_ != 0);
    if (!hasBounds) {
        return nil;
    }
    double minLat = std::min(req.minLatitude_, req.maxLatitude_);
    double maxLat = std::max(req.minLatitude_, req.maxLatitude_);
    double minLon = std::min(req.minLongitude_, req.maxLongitude_);
    double maxLon = std::max(req.minLongitude_, req.maxLongitude_);
    if (!(minLat > -90 && maxLat < 90 && minLon > -180 && maxLon < 180 && (maxLat - minLat) > 0 && (maxLon - minLon) > 0)) {
        return nil;
    }
    double midLat = (minLat + maxLat) / 2.0;
    double midLon = (minLon + maxLon) / 2.0;
    double maxSpanDeg = std::max(maxLat - minLat, maxLon - minLon);
    double approxMeters = maxSpanDeg * 111000.0 / 2.0;
    if (approxMeters < 50) {
        approxMeters = 50;
    }
    if (approxMeters > 500000) {
        approxMeters = 500000;
    }
    CLLocationCoordinate2D center; center.latitude = midLat; center.longitude = midLon;
    return [[CLCircularRegion alloc] initWithCenter:center radius:approxMeters identifier:@"geo_req_bounds"];
}

static Location::LocationErrCode WaitGeocode(dispatch_semaphore_t sema, NSString *q)
{
    if (dispatch_semaphore_wait(sema, dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3 * NSEC_PER_SEC))) != 0) {
        return Location::LocationErrCode::ERRCODE_GEOCODING_FAIL;
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

static void FillForwardAddress(Location::GeoAddress &dst, CLPlacemark *pm, const Location::GeoCodeRequest &req)
{
    if (pm.location) {
        dst.latitude_ = pm.location.coordinate.latitude;
        dst.longitude_ = pm.location.coordinate.longitude;
    }
    dst.locale_ = req.locale_.empty() ? [[NSLocale currentLocale].localeIdentifier UTF8String] : req.locale_;
    if (pm.ISOcountryCode) {
        dst.countryCode_ = [pm.ISOcountryCode UTF8String];
    }
    if (pm.country) {
        dst.countryName_ = [pm.country UTF8String];
    }
    if (pm.administrativeArea) {
        dst.administrativeArea_ = [pm.administrativeArea UTF8String];
    }
    if (pm.locality) {
        dst.locality_ = [pm.locality UTF8String];
    }
    if (pm.thoroughfare) {
        dst.roadName_ = [pm.thoroughfare UTF8String];
    }
    if (pm.name) {
        dst.placeName_ = [pm.name UTF8String];
    }
    if (pm.postalCode) {
        dst.postalCode_ = [pm.postalCode UTF8String];
    }
}

static void BuildFormatted(CLPlacemark *pm, Location::GeoAddress &dst)
{
    std::vector<std::string> parts;
    if (pm.name) {
        parts.emplace_back([pm.name UTF8String]);
    }
    if (pm.thoroughfare) {
        parts.emplace_back([pm.thoroughfare UTF8String]);
    }
    if (pm.locality) {
        parts.emplace_back([pm.locality UTF8String]);
    }
    if (pm.administrativeArea) {
        parts.emplace_back([pm.administrativeArea UTF8String]);
    }
    if (pm.country) {
        parts.emplace_back([pm.country UTF8String]);
    }
    std::string formatted;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i) {
            formatted += ", ";
        }
        formatted += parts[i];
    }
    if (!formatted.empty()) {
        dst.descriptions_.insert({0, formatted});
        dst.descriptionsSize_ = 1;
    }
}

Location::LocationErrCode LocatorCrossplatformServiceIosImpl::GetAddressByLocationName(std::unique_ptr<Location::GeoCodeRequest> &request,
    std::list<std::shared_ptr<Location::GeoAddress>>& replyList)
{
    if (!request) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    std::string query = request->description_.empty() ? request->country_ : request->description_;
    if (query.empty()) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    CLGeocoder *geocoder = [[CLGeocoder alloc] init];
    if (!geocoder) {
        return Location::LocationErrCode::ERRCODE_GEOCODING_FAIL;
    }
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);
    __block NSError *err = nil;
    __block NSArray<CLPlacemark*> *placemarksLocal = nil;
    CLCircularRegion *region = BuildRegion(*request);
    NSString *q = [NSString stringWithUTF8String:query.c_str()];
    auto handler = ^(NSArray<CLPlacemark*> * _Nullable p, NSError * _Nullable e) {
        placemarksLocal = p ? [p copy] : @[];
        err = e;
        dispatch_semaphore_signal(sema);
    };
    if (region) {
        [geocoder geocodeAddressString:q inRegion:region completionHandler:handler];
    } else {
        [geocoder geocodeAddressString:q completionHandler:handler];
    }
    auto waitCode = WaitGeocode(sema, q);
    if (waitCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return waitCode;
    }
    if (err) {
        return Location::LocationErrCode::ERRCODE_GEOCODING_FAIL;
    }
    g_lastForwardPlacemarkList = placemarksLocal;
    int32_t maxItems = request->maxItems_ > 0 ? request->maxItems_ : 5;
    int added = 0;
    std::string wantCountry = request->country_;
    for (CLPlacemark *pm in g_lastForwardPlacemarkList) {
        if (added >= maxItems) {
            break;
        }
        auto addr = std::make_shared<Location::GeoAddress>();
        FillForwardAddress(*addr, pm, *request);
        BuildFormatted(pm, *addr);
        replyList.push_back(addr);
        ++added;
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

static bool ValidateLatLon(double lat, double lon)
{
    return !(lat < -90.0 || lat > 90.0 || lon < -180.0 || lon > 180.0);
}

static void FillReverseAddr(Location::GeoAddress &addr, CLPlacemark *pm, double fallbackLat, double fallbackLon, const Location::ReverseGeoCodeRequest &req)
{
    if (pm.location) {
        addr.latitude_ = pm.location.coordinate.latitude;
        addr.longitude_ = pm.location.coordinate.longitude;
    } else {
        addr.latitude_ = fallbackLat;
        addr.longitude_ = fallbackLon;
    }
    addr.locale_ = req.locale_.empty() ? [[NSLocale currentLocale].localeIdentifier UTF8String] : req.locale_;
    if (pm.ISOcountryCode) {
        addr.countryCode_ = [pm.ISOcountryCode UTF8String];
    }
    if (pm.country) {
        addr.countryName_ = [pm.country UTF8String];
    }
    if (pm.administrativeArea) {
        addr.administrativeArea_ = [pm.administrativeArea UTF8String];
    }
    if (pm.subAdministrativeArea) {
        addr.subAdministrativeArea_ = [pm.subAdministrativeArea UTF8String];
    }
    if (pm.locality) {
        addr.locality_ = [pm.locality UTF8String];
    }
    if (pm.subLocality) {
        addr.subLocality_ = [pm.subLocality UTF8String];
    }
    if (pm.thoroughfare) {
        addr.roadName_ = [pm.thoroughfare UTF8String];
    }
    if (pm.subThoroughfare) {
        addr.subRoadName_ = [pm.subThoroughfare UTF8String];
    }
    if (pm.name) {
        addr.placeName_ = [pm.name UTF8String];
    }
    if (pm.postalCode) {
        addr.postalCode_ = [pm.postalCode UTF8String];
    }
}

static void BuildFormattedReverse(CLPlacemark *pm, Location::GeoAddress &addr)
{
    std::vector<std::string> parts;
    if (pm.name) {
        parts.emplace_back([pm.name UTF8String]);
    }
    if (pm.thoroughfare) {
        parts.emplace_back([pm.thoroughfare UTF8String]);
    }
    if (pm.locality) {
        parts.emplace_back([pm.locality UTF8String]);
    }
    if (pm.administrativeArea) {
        parts.emplace_back([pm.administrativeArea UTF8String]);
    }
    if (pm.country) {
        parts.emplace_back([pm.country UTF8String]);
    }
    std::string formatted;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i) {
            formatted += ", ";
        }
        formatted += parts[i];
    }
    if (!formatted.empty()) {
        addr.descriptions_.insert({0, formatted});
        addr.descriptionsSize_ = 1;
    }
}

static bool ReverseCountryMismatch(const std::string &want, const Location::GeoAddress &addr)
{
    if (want.empty()) {
        return false;
    }
    std::string iso = addr.countryCode_;
    std::string nameLower = addr.countryName_;
    std::string wantLower = want;
    for (auto &c: iso) {
        c = (char)tolower((unsigned char)c);
    }
    for (auto &c: nameLower) {
        c = (char)tolower((unsigned char)c);
    }
    for (auto &c: wantLower) {
        c = (char)tolower((unsigned char)c);
    }
    return (iso != wantLower && nameLower.find(wantLower) == std::string::npos);
}

Location::LocationErrCode LocatorCrossplatformServiceIosImpl::GetAddressByCoordinate(std::unique_ptr<Location::ReverseGeoCodeRequest> &request,
    std::list<std::shared_ptr<Location::GeoAddress>>& replyList)
{
    NSLog(@"Get address by coordinate");
    if (!request) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    double lat = request->latitude_;
    double lon = request->longitude_;
    if (!ValidateLatLon(lat, lon)) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    CLGeocoder *geocoder = [[CLGeocoder alloc] init];
    if (!geocoder) {
        return Location::LocationErrCode::ERRCODE_REVERSE_GEOCODING_FAIL;
    }
    CLLocation *loc = [[CLLocation alloc] initWithLatitude:lat longitude:lon];
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);
    __block NSError *err = nil;
    __block NSArray<CLPlacemark*> *placemarksLocal = nil;
    [geocoder reverseGeocodeLocation:loc completionHandler:^(NSArray<CLPlacemark *> * _Nullable p,
        NSError * _Nullable e) {
        placemarksLocal = p ? [p copy] : @[];
        err = e;
        dispatch_semaphore_signal(sema);
    }];
    if (dispatch_semaphore_wait(sema, dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3 * NSEC_PER_SEC))) != 0) {
        return Location::LocationErrCode::ERRCODE_REVERSE_GEOCODING_FAIL;
    }
    if (err) {
        return Location::LocationErrCode::ERRCODE_SUCCESS;
    }
    g_lastReversePlacemarkList = placemarksLocal;
    NSUInteger placemarkCount = g_lastReversePlacemarkList.count;
    int32_t maxItems = request->maxItems_ > 0 ? request->maxItems_ : 1;
    int added = 0;
    std::string wantCountry = request->country_;
    for (NSUInteger i = 0; i < placemarkCount; ++i) {
        if (added >= maxItems) {
            break;
        }
        auto addr = std::make_shared<Location::GeoAddress>();
        FillReverseAddr(*addr, g_lastReversePlacemarkList[i], lat, lon, *request);
        if (ReverseCountryMismatch(wantCountry, *addr)) {
            continue;
        }
        BuildFormattedReverse(g_lastReversePlacemarkList[i], *addr);
        replyList.push_back(addr);
        ++added;
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

std::shared_ptr<Location::CountryCode> LocatorCrossplatformServiceIosImpl::GetIsoCountryCode()
{
    NSLog(@"Get country code");
    auto ret = std::make_shared<Location::CountryCode>();
    NSString *country = [[NSLocale currentLocale] objectForKey:NSLocaleCountryCode];
    if (!country) {
        ret->SetCountryCodeStr("");
        return ret;
    }
    std::string cc([country UTF8String]);
    for (auto &c : cc) {
        c = static_cast<char>(::toupper(static_cast<unsigned char>(c)));
    }
    ret->SetCountryCodeStr(cc);
    return ret;
}

static LocationErrCode AddGnssGeofenceReport(bool ok, int32_t fenceId, GeofenceRequest &request,
    sptr<Location::LocationGnssGeofenceCallbackNapi>& callback)
{
    if (ok) {
        RegisterGeofenceCallbackIOS(fenceId, callback);
#ifdef NOTIFICATION_ENABLE
        auto notiList = request.GetNotificationRequestList();
        if (!notiList.empty()) {
            StoreFenceNotifications(fenceId, notiList);
        }
#endif
        callback->OnReportOperationResult(fenceId, (int)Location::GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_ADD,
            (int)Location::GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_SUCCESS);
        return LocationErrCode::ERRCODE_SUCCESS;
    }
    callback->OnReportOperationResult(fenceId, (int)Location::GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_ADD,
        (int)Location::GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_UNKNOWN);
    return LocationErrCode::ERRCODE_GEOFENCE_FAIL;
}

LocationErrCode LocatorCrossplatformServiceIosImpl::AddGnssGeofence(GeofenceRequest &request,
    sptr<Location::LocationGnssGeofenceCallbackNapi>& callback)
{
    NSLog(@"Add gnss geofence");
    if (!CheckLocationPermission()) {
        NSLog(@"no permissions");
        return LocationErrCode::ERRCODE_PERMISSION_DENIED;
    }
    int32_t state = 0;
    GetSwitchState(state);
    if (state != 1) {
        return LocationErrCode::ERRCODE_SWITCH_OFF;
    }
    if (!callback) {
        return LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    auto geofence = request.GetGeofence();
    int32_t fenceId = GenerateFenceIdIOS();
    request.SetFenceId(fenceId);
    NSLog(@"Add gnss geofence assigned fenceId=%d", fenceId);
    if (geofence.expiration > 0) {
        int64_t nowNs = NowSinceBootNs();
        int64_t absNs = nowNs + (int64_t)geofence.expiration * MILL_TO_NANOS;
        request.SetRequestExpirationTime(absNs);
    }
    int64_t expirationMs = 0;
    int64_t storedAbsNs = request.GetRequestExpirationTime();
    if (storedAbsNs > 0) {
        int64_t remainNs = storedAbsNs - NowSinceBootNs();
        if (remainNs <= 0) {
            expirationMs = 1;
        } else {
            expirationMs = remainNs / MILL_TO_NANOS;
            if (expirationMs == 0) {
                expirationMs = 1;
            }
        }
    }
    auto runtime = GeoLocationIosRuntime::Instance();
    runtime->EnsureAuth();
    CLLocationManager *mgr = runtime->GetManager();
    bool ok = runtime->AddRegion(fenceId, geofence.latitude, geofence.longitude, geofence.radius, expirationMs);
    return AddGnssGeofenceReport(ok, fenceId, request, callback);
}

LocationErrCode LocatorCrossplatformServiceIosImpl::RemoveGnssGeofence(int32_t fenceId,
    sptr<Location::LocationGnssGeofenceCallbackNapi>& callback)
{
    NSLog(@"Remove gnss geofence");
    if (!CheckLocationPermission()) {
        NSLog(@"no permissions");
        return LocationErrCode::ERRCODE_PERMISSION_DENIED;
    }
    if (callback == nullptr || fenceId <= 0) {
        return LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    auto runtime = GeoLocationIosRuntime::Instance();
    bool ok = runtime->RemoveRegion(fenceId);
    if (ok) {
#ifdef NOTIFICATION_ENABLE
        auto list = GetFenceNotifications(fenceId);
        if (!list.empty()) {
            RemoveFenceNotifications(fenceId);
        }
#endif
        callback->OnReportOperationResult(fenceId,
            (int)Location::GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_DELETE,
            (int)Location::GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_SUCCESS);
        return LocationErrCode::ERRCODE_SUCCESS;
    }
    callback->OnReportOperationResult(fenceId,
        (int)Location::GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_DELETE,
        (int)Location::GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_GEOFENCE_ID_UNKNOWN);
    return LocationErrCode::ERRCODE_GEOFENCE_INCORRECT_ID;
}

LocationErrCode LocatorCrossplatformServiceIosImpl::GetCurrentWifiBssidForLocating(std::string &bssid)
{
    bssid.clear();
    NSLog(@"Get current wifi bssid for locating");
    if (!CheckLocationPermission()) {
        NSLog(@"no permissions");
        return LocationErrCode::ERRCODE_PERMISSION_DENIED;
    }
    CFArrayRef interfaces = CNCopySupportedInterfaces();
    if (!interfaces || CFArrayGetCount(interfaces) == 0) {
        if (interfaces) {
            CFRelease(interfaces);
        }
        return LocationErrCode::ERRCODE_WIFI_IS_NOT_CONNECTED;
    }
    bool found = false;
    CFIndex count = CFArrayGetCount(interfaces);
    for (CFIndex i = 0; i < count; ++i) {
        CFStringRef ifname = (CFStringRef)CFArrayGetValueAtIndex(interfaces, i);
        NSString *ifnameStr = (__bridge NSString *)ifname;
        if (![ifnameStr isEqualToString:@"en0"]) {
            continue;
        }
        CFDictionaryRef info = CNCopyCurrentNetworkInfo(ifname);
        if (!info) {
            continue;
        }
        CFStringRef bssidRef = (CFStringRef)CFDictionaryGetValue(info, kCNNetworkInfoKeyBSSID);
        if (bssidRef) {
            char buff[256] = {0};
            if (CFStringGetCString(bssidRef, buff, sizeof(buff), kCFStringEncodingUTF8)) {
                bssid = buff;
                found = true;
            }
        }
        CFRelease(info);
        if (found) {
            break;
        }
    }
    CFRelease(interfaces);

    if (found) {
        return LocationErrCode::ERRCODE_SUCCESS;
    } else {
        return LocationErrCode::ERRCODE_WIFI_IS_NOT_CONNECTED;
    }
}

LocationErrCode LocatorCrossplatformServiceIosImpl::RegisterSwitchCallback(
    sptr<Location::LocationSwitchCallbackNapi>& switchCallbackHost)
{
    NSLog(@"Register switch callback");
    if (!switchCallbackHost) {
        return LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    int32_t state = 0;
    GetSwitchState(state);
    switchCallbackHost->OnSwitchChange(state);
    ArkUIAddSwitchCallback(&switchCallbackHost);
    return LocationErrCode::ERRCODE_SUCCESS;
}

LocationErrCode LocatorCrossplatformServiceIosImpl::UnregisterSwitchCallback(
    sptr<Location::LocationSwitchCallbackNapi>& switchCallbackHost)
{
    NSLog(@"Unregister switchCallback");
    if (!switchCallbackHost) {
        return LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    ArkUIRemoveSwitchCallbackByHost(&switchCallbackHost);
    return LocationErrCode::ERRCODE_SUCCESS;
}

LocationErrCode LocatorCrossplatformServiceIosImpl::RegisterCountryCodeCallback(
    sptr<Location::CountryCodeCallbackNapi>& callbackHost)
{
    NSLog(@"Register country code callback");
    if (!callbackHost) {
        return LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    ArkUIAddCountryCodeCallback(&callbackHost);
    auto current = GetIsoCountryCode();
    if (current) {
        callbackHost->OnCountryCodeChange(current);
    }
    return LocationErrCode::ERRCODE_SUCCESS;
}

LocationErrCode LocatorCrossplatformServiceIosImpl::UnregisterCountryCodeCallback(
    sptr<Location::CountryCodeCallbackNapi>& callbackHost)
{
    NSLog(@"Unregister country code callback");
    if (!callbackHost) {
        return LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    ArkUIRemoveCountryCodeCallbackByHost(&callbackHost);
    return LocationErrCode::ERRCODE_SUCCESS;
}

LocationErrCode LocatorCrossplatformServiceIosImpl::SubscribeLocationError(
    sptr<Location::LocationErrorCallbackNapi>& locationErrorCallbackHost)
{
    NSLog(@"Subscribe location error");
    if (!CheckLocationPermission()) {
        NSLog(@"no permissions");
        return LocationErrCode::ERRCODE_PERMISSION_DENIED;
    }
    if (!locationErrorCallbackHost) {
        return LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    ArkUIAddLocationErrorCallback(&locationErrorCallbackHost);
    return LocationErrCode::ERRCODE_SUCCESS;
}

LocationErrCode LocatorCrossplatformServiceIosImpl::UnSubscribeLocationError(
    sptr<Location::LocationErrorCallbackNapi>& locationErrorCallbackHost)
{
    NSLog(@"UnSubscribe location error");
    if (!CheckLocationPermission()) {
        NSLog(@"no permissions");
        return LocationErrCode::ERRCODE_PERMISSION_DENIED;
    }
    if (!locationErrorCallbackHost) {
        return LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    ArkUIRemoveLocationErrorCallbackByHost(&locationErrorCallbackHost);
    return LocationErrCode::ERRCODE_SUCCESS;
}

LocationErrCode LocatorCrossplatformServiceIosImpl::SubscribeBluetoothScanResultChange(
    sptr<Location::BluetoothScanResultCallbackNapi>& bluetoothScanResultCallbackHost)
{
    NSLog(@"Subscribe bluetooth scan result change");
    if (!CheckLocationPermission()) {
        NSLog(@"no permissions");
        return LocationErrCode::ERRCODE_PERMISSION_DENIED;
    }
    if (!bluetoothScanResultCallbackHost) {
        return LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    int32_t state = 0;
    GetSwitchState(state);
    if (state != 1) {
        return LocationErrCode::ERRCODE_SWITCH_OFF;
    }
    ArkUIAddBluetoothScanResultCallback(&bluetoothScanResultCallbackHost);
    return LocationErrCode::ERRCODE_SUCCESS;
}

LocationErrCode LocatorCrossplatformServiceIosImpl::UnSubscribeBluetoothScanResultChange(
    sptr<Location::BluetoothScanResultCallbackNapi>& bluetoothScanResultCallbackHost)
{
    NSLog(@"Unsubscribe bluetooth scan result change");
    if (!CheckLocationPermission()) {
        NSLog(@"no permissions");
        return LocationErrCode::ERRCODE_PERMISSION_DENIED;
    }
    if (!bluetoothScanResultCallbackHost) {
        return LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    ArkUIRemoveBluetoothScanResultCallbackByHost(&bluetoothScanResultCallbackHost);
    return LocationErrCode::ERRCODE_SUCCESS;
}

LocationErrCode LocatorCrossplatformServiceIosImpl::RegisterGnssStatusCallback(
    sptr<Location::GnssStatusCallbackNapi>& callback)
{
    NSLog(@"not support in ios");
    return LocationErrCode::ERRCODE_NOT_SUPPORTED;
}

LocationErrCode LocatorCrossplatformServiceIosImpl::UnregisterGnssStatusCallback(
    sptr<Location::GnssStatusCallbackNapi>& callback)
{
    NSLog(@"not support in ios");
    return LocationErrCode::ERRCODE_NOT_SUPPORTED;
}

LocationErrCode LocatorCrossplatformServiceIosImpl::RegisterNmeaMessageCallback(
    sptr<Location::NmeaMessageCallbackNapi>& cb)
{
    NSLog(@"not support in ios");
    return LocationErrCode::ERRCODE_NOT_SUPPORTED;
}

LocationErrCode LocatorCrossplatformServiceIosImpl::UnregisterNmeaMessageCallback(
    sptr<Location::NmeaMessageCallbackNapi>& cb)
{
    NSLog(@"not support in ios");
    return LocationErrCode::ERRCODE_NOT_SUPPORTED;
}

LocationErrCode LocatorCrossplatformServiceIosImpl::ClearAllGnssGeofences()
{
    NSLog(@"Clear all gnssGeofences");
    if (!CheckLocationPermission()) {
        NSLog(@"no permissions");
        return LocationErrCode::ERRCODE_PERMISSION_DENIED;
    }
    auto runtime = GeoLocationIosRuntime::Instance();
    CLLocationManager *mgr = runtime->GetManager();
    if (!mgr) {
        return LocationErrCode::ERRCODE_GEOFENCE_FAIL;
    }
    std::vector<int32_t> fenceIds;
    for (CLRegion *r in mgr.monitoredRegions) {
        if ([r.identifier hasPrefix:@"arkui_geofence_"]) {
            NSString *numStr = [r.identifier substringFromIndex:[@"arkui_geofence_" length]];
            int32_t fid = (int32_t)[numStr intValue];
            fenceIds.push_back(fid);
            [mgr stopMonitoringForRegion:r];
        }
    }
    for (auto fid : fenceIds) {
        UnregisterGeofenceCallbackIOS(fid);
#ifdef NOTIFICATION_ENABLE
        auto list = GetFenceNotifications(fid);
        if (!list.empty()) {
            RemoveFenceNotifications(fid);
        }
#endif
    }
    return LocationErrCode::ERRCODE_SUCCESS;
}
} // namespace Plugin
} // namespace OHOS
