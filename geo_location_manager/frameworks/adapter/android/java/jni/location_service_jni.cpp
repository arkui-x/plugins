/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <cstdio>
#include <fstream>
#include <iostream>
#include <jni.h>
#include <memory>
#include <string>

#include "bluetooth_scan_result_callback_napi.h"
#include "constant_definition.h"
#include "country_code.h"
#include "inner_api/plugin_utils_inner.h"
#include "i_country_code_callback.h"
#include "location_gnss_geofence_callback_napi.h"
#include "location_service_jni.h"
#include "location_napi_event.h"
#include "location_crossplatform_service_impl.h"
#include "location_log.h"
#include "plugin_utils.h"
#include "refbase.h"
#if !defined(PLUGIN_INTERFACE_NATIVE_LOG_H)
#define LogLevel GEOLOC_PLUGIN_LOGLEVEL_REMAED__
#define GEOLOC_PLUGIN_LOGLEVEL_RENAMED
#endif
#include "notification_request.h"
#include "notification_helper.h"
#ifdef GEOLOC_PLUGIN_LOGLEVEL_RENAMED
#undef LogLevel
#endif

namespace OHOS::Plugin {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LOCATION_SERVICE_JNI = { LOG_CORE, LOCATION_LOG_DOMAIN,
    "LocationServiceJni" };
const char LOCATION_CLASS_NAME[] = "android/location/Address";

constexpr double MIN_LATITUDE = -90.0;
constexpr double MAX_LATITUDE = 90.0;
constexpr double MIN_LONGITUDE = -180.0;
constexpr double MAX_LONGITUDE = 180.0;
constexpr double MAX_GEOFENCE_RADIUS = 1000000.0;
constexpr double MIN_GEOFENCE_RADIUS = 0.0;
constexpr int64_t MILL_TO_NANOS = 1000000LL;
static const int32_t FENCE_MAX_ID = 20;
static int32_t g_fenceIdCounter = 0;
static std::mutex g_fenceIdMutex;
static bool g_fenceIdLoaded = false;
static const std::string COUNTER_FILE = "fence_id_counter.txt";
static std::map<int32_t, sptr<OHOS::Location::LocationGnssGeofenceCallbackNapi>> g_geofenceCallbackMap;
static std::mutex g_geofenceCbMutex;
static std::map<int32_t, std::vector<std::shared_ptr<Notification::NotificationRequest>>>
    g_geofenceNotifications;
static std::mutex g_geofenceNotiMutex;
static std::vector<sptr<Location::LocatorCallbackNapi>> g_locatorCallbacks;
static std::mutex g_locatorCallbacksMutex;
static std::vector<sptr<Location::LocationSwitchCallbackNapi>> g_switchCallbacks;
static std::mutex g_switchCallbacksMutex;
static std::vector<sptr<Location::LocationErrorCallbackNapi>> g_locationErrorCallbacks;
static std::mutex g_locationErrorCallbacksMutex;
static std::vector<sptr<Location::GnssStatusCallbackNapi>> g_gnssStatusCallbacks;
static std::mutex g_gnssStatusCallbacksMutex;
static std::vector<sptr<Location::NmeaMessageCallbackNapi>> g_nmeaCallbacks;
static std::mutex g_nmeaCallbacksMutex;
static std::vector<sptr<Location::BluetoothScanResultCallbackNapi>> g_btScanCallbacks;
static std::mutex g_btScanCallbacksMutex;
static std::vector<sptr<Location::CountryCodeCallbackNapi>> g_countryCodeCallbacks;
static std::mutex g_countryCodeCallbacksMutex;

template<typename T>
std::vector<sptr<T>> SnapshotCallbacks(std::vector<sptr<T>> &vec, std::mutex &mtx)
{
    std::lock_guard<std::mutex> lock(mtx);
    return vec;
}

const char LOCATION_SERVICE_CLASS_NAME[] = "ohos/ace/plugin/locationserviceplugin/LocationService";
static const JNINativeMethod g_methods[] = {
    {"nativeInit", "()V",
        reinterpret_cast<void *>(LocationServiceJni::NativeInit)},
    {"nativeOnBluetoothScanResult", "(Ljava/lang/String;Ljava/lang/String;I[BZ)V",
        reinterpret_cast<void *>(LocationServiceJni::NativeOnBluetoothScanResult)},
    {"nativeOnLocationError", "(I)V",
        reinterpret_cast<void *>(LocationServiceJni::NativeOnLocationError)},
    {"nativeOnLocationChanged", "(DDFDFFJFLjava/lang/String;)V",
        reinterpret_cast<void *>(LocationServiceJni::NativeOnLocationChanged)},
    {"nativeOnSwitchStateChanged", "(I)V",
        reinterpret_cast<void *>(LocationServiceJni::NativeOnSwitchStateChanged)},
    {"nativeOnGnssStatusChanged", "(I[I[D[D[D[D[I[I)V",
        reinterpret_cast<void *>(LocationServiceJni::NativeOnGnssStatusChanged)},
    {"nativeOnNmeaMessage", "(JLjava/lang/String;)V",
        reinterpret_cast<void *>(LocationServiceJni::NativeOnNmeaMessage)},
    {"nativeOnCountryCodeChanged", "(Ljava/lang/String;I)V",
        reinterpret_cast<void *>(LocationServiceJni::NativeOnCountryCodeChanged)},
    {"nativeOnGeofenceEvent", "(I)V",
        reinterpret_cast<void *>(LocationServiceJni::NativeOnGeofenceEvent)},
    {"nativeOnNotificationEvent", "(IZ)V",
        reinterpret_cast<void *>(LocationServiceJni::NativeOnNotificationEvent)},
};

struct {
    jmethodID getSwitchState;
    jmethodID startLocating;
    jmethodID stopLocating;
    jmethodID isGeoConvertAvailable;
    jmethodID getAddressByCoordinate;
    jmethodID getAddressByLocationName;
    jmethodID getIsoCountryCode;
    jmethodID addGnssGeofence;
    jmethodID removeGnssGeofence;
    jmethodID getCurrentWifiBssidForLocating;
    jmethodID registerSwitchCallback;
    jmethodID unregisterSwitchCallback;
    jmethodID registerCountryCodeCallback;
    jmethodID unregisterCountryCodeCallback;
    jmethodID registerBluetoothScanResultCallback;
    jmethodID unregisterBluetoothScanResultCallback;
    jmethodID registerLocationErrorCallback;
    jmethodID unregisterLocationErrorCallback;
    jmethodID registerLocationChangeCallbackWithConfig;
    jmethodID registerGnssStatusCallback;
    jmethodID unregisterGnssStatusCallback;
    jmethodID registerNmeaMessageCallback;
    jmethodID unregisterNmeaMessageCallback;
    jmethodID getCurrentLocation;
    jmethodID requestAndCheckLocationPermission;
    jobject globalRef;
} g_locationservicepluginClass;
} //namespace
bool LocationServiceJni::hasInit_ = false;

bool LocationServiceJni::Register(void *env)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::Register");
    if (hasInit_) {
        return hasInit_;
    }
    auto *jniEnv = static_cast<JNIEnv*>(env);
    if (!jniEnv) {
        LBSLOGE(LOCATION_SERVICE_JNI, "LocationServiceJni::Register, jniEnv is null");
        return false;
    }
    jclass cls = jniEnv->FindClass(LOCATION_SERVICE_CLASS_NAME);
    if (!cls) {
        LBSLOGE(LOCATION_SERVICE_JNI, "LocationServiceJni::Register, cls is null");
        return false;
    }
    bool ret = jniEnv->RegisterNatives(
        cls, g_methods, sizeof(g_methods) / sizeof(g_methods[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LBSLOGE(LOCATION_SERVICE_JNI, "LocationServiceJni::Register failed");
        return false;
    }
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::Register success");
    return true;
}

static inline bool InitGlobalRef(JNIEnv *env, jobject jobj)
{
    if (!env) {
        LBSLOGE(LOCATION_SERVICE_JNI, "InitGlobalRef: env is null");
        return false;
    }
    g_locationservicepluginClass.globalRef = env->NewGlobalRef(jobj);
    if (!g_locationservicepluginClass.globalRef) {
        LBSLOGE(LOCATION_SERVICE_JNI, "InitGlobalRef: failed to create global reference");
        return false;
    }
    return true;
}

static inline jmethodID GetMethodID(JNIEnv *env, jclass cls, const char *name, const char *sig)
{
    if (!env || !cls || !name || !sig) {
        LBSLOGE(LOCATION_SERVICE_JNI, "GetMethodID: invalid parameters");
        return nullptr;
    }
    jmethodID methodID = env->GetMethodID(cls, name, sig);
    if (!methodID) {
        LBSLOGE(LOCATION_SERVICE_JNI, "GetMethodID: failed to get method ID for %s", name);
    }
    return methodID;
}

static inline void RegisterCoreMethodIDs(JNIEnv *env, jclass cls)
{
    g_locationservicepluginClass.getSwitchState = GetMethodID(env, cls, "getSwitchState", "()I");
    g_locationservicepluginClass.startLocating = GetMethodID(env, cls, "startLocating", "()I");
    g_locationservicepluginClass.stopLocating = GetMethodID(env, cls, "stopLocating", "()I");
    g_locationservicepluginClass.isGeoConvertAvailable = GetMethodID(env, cls, "isGeoConvertAvailable", "()Z");
    g_locationservicepluginClass.getAddressByCoordinate = GetMethodID(env, cls, "getAddressByCoordinate",
        "(DDILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)[Landroid/location/Address;");
    g_locationservicepluginClass.getAddressByLocationName = GetMethodID(env, cls, "getAddressByLocationName",
        "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;DDDDLjava/lang/String;)[Landroid/location/Address;");
    g_locationservicepluginClass.getIsoCountryCode =
        GetMethodID(env, cls, "getIsoCountryCode", "()Ljava/lang/String;");
}

static inline void RegisterGeofenceMethodIDs(JNIEnv *env, jclass cls)
{
    g_locationservicepluginClass.addGnssGeofence = GetMethodID(env, cls, "addGnssGeofence", "(DDFJI)V");
    g_locationservicepluginClass.removeGnssGeofence = GetMethodID(env, cls, "removeGnssGeofence", "(I)V");
}

static void RegisterCallbackMethodIDs(JNIEnv *env, jclass cls)
{
    g_locationservicepluginClass.registerSwitchCallback = GetMethodID(env, cls, "registerSwitchCallback", "()V");
    g_locationservicepluginClass.unregisterSwitchCallback = GetMethodID(env, cls, "unregisterSwitchCallback", "()V");
    g_locationservicepluginClass.registerCountryCodeCallback =
        GetMethodID(env, cls, "registerCountryCodeCallback", "()V");
    g_locationservicepluginClass.unregisterCountryCodeCallback =
        GetMethodID(env, cls, "unregisterCountryCodeCallback", "()V");
    g_locationservicepluginClass.registerBluetoothScanResultCallback =
        GetMethodID(env, cls, "registerBluetoothScanResultCallback", "()I");
    g_locationservicepluginClass.unregisterBluetoothScanResultCallback =
        GetMethodID(env, cls, "unregisterBluetoothScanResultCallback", "()V");
    g_locationservicepluginClass.registerLocationErrorCallback =
        GetMethodID(env, cls, "registerLocationErrorCallback", "()V");
    g_locationservicepluginClass.unregisterLocationErrorCallback =
        GetMethodID(env, cls, "unregisterLocationErrorCallback", "()V");
    g_locationservicepluginClass.registerLocationChangeCallbackWithConfig =
        GetMethodID(env, cls, "registerLocationChangeCallbackWithConfig", "(IIIDFIIZZ)I");
    g_locationservicepluginClass.registerGnssStatusCallback =
        GetMethodID(env, cls, "registerGnssStatusCallback", "()V");
    g_locationservicepluginClass.unregisterGnssStatusCallback =
        GetMethodID(env, cls, "unregisterGnssStatusCallback", "()V");
    g_locationservicepluginClass.registerNmeaMessageCallback =
        GetMethodID(env, cls, "registerNmeaMessageCallback", "()V");
    g_locationservicepluginClass.unregisterNmeaMessageCallback =
        GetMethodID(env, cls, "unregisterNmeaMessageCallback", "()V");
}

static inline void RegisterUtilityMethodIDs(JNIEnv *env, jclass cls)
{
    g_locationservicepluginClass.getCurrentWifiBssidForLocating =
        GetMethodID(env, cls, "getCurrentWifiBssidForLocating", "()Ljava/lang/String;");
    g_locationservicepluginClass.getCurrentLocation =
        GetMethodID(env, cls, "getCurrentLocation", "()Landroid/location/Location;");
    g_locationservicepluginClass.requestAndCheckLocationPermission =
        GetMethodID(env, cls, "requestAndCheckLocationPermission", "()Z");
}

static inline void RegisterMethodIDs(JNIEnv *env, jclass cls)
{
    RegisterCoreMethodIDs(env, cls);
    RegisterGeofenceMethodIDs(env, cls);
    RegisterCallbackMethodIDs(env, cls);
    RegisterUtilityMethodIDs(env, cls);
}

void LocationServiceJni::NativeInit(JNIEnv *env, jobject jobj)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::NativeInit");
    if (!env || !jobj) {
        LBSLOGE(LOCATION_SERVICE_JNI, "NativeInit: invalid env or jobj");
        return;
    }
    if (!InitGlobalRef(env, jobj)) {
        return;
    }
    jclass cls = env->GetObjectClass(jobj);
    if (!cls) {
        return;
    }
    RegisterMethodIDs(env, cls);
    env->DeleteLocalRef(cls);
    hasInit_ = true;
}

inline static Location::LocationErrCode CheckLocationPermission(JNIEnv* env)
{
    jboolean permissionResult = env->CallBooleanMethod(
        g_locationservicepluginClass.globalRef, g_locationservicepluginClass.requestAndCheckLocationPermission);
    if (!permissionResult) {
        LBSLOGE(LOCATION_SERVICE_JNI, "CheckLocationPermission: no location permission");
        return Location::LocationErrCode::ERRCODE_PERMISSION_DENIED;
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

Location::LocationErrCode LocationServiceJni::GetSwitchState(int32_t &state)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::GetSwitchState");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LBSLOGE(LOCATION_SERVICE_JNI, "GetSwitchState: failed to get jni env");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto errCode = CheckLocationPermission(env);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errCode;
    }
    if (!g_locationservicepluginClass.globalRef || !g_locationservicepluginClass.getSwitchState) {
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    state = static_cast<int>(env->CallIntMethod(
        g_locationservicepluginClass.globalRef, g_locationservicepluginClass.getSwitchState));
    if (env->ExceptionCheck()) {
        LBSLOGE(LOCATION_SERVICE_JNI, "LocationServiceJni JNI: call GetSwitchState has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

static inline bool CheckAndLogJniEnv(JNIEnv* env)
{
    if (!env) {
        LBSLOGE(LOCATION_SERVICE_JNI, "StartLocating: failed to get jni env");
        return false;
    }
    return true;
}

static inline bool ValidateRequestConfig(const std::unique_ptr<RequestConfig>& requestConfig,
    const sptr<Location::LocatorCallbackNapi>& locatorCallbackHost)
{
    if (!requestConfig || !locatorCallbackHost) {
        LBSLOGE(LOCATION_SERVICE_JNI, "StartLocating invalid params");
        return false;
    }
    return true;
}

static std::unique_ptr<Location::Location> ExtractLocation(JNIEnv* env, jobject res, jclass cls)
{
    jmethodID getLatitude = env->GetMethodID(cls, "getLatitude", "()D");
    if (!getLatitude) {
        return nullptr;
    }
    jmethodID getLongitude = env->GetMethodID(cls, "getLongitude", "()D");
    if (!getLongitude) {
        return nullptr;
    }
    jmethodID getAccuracy = env->GetMethodID(cls, "getAccuracy", "()F");
    if (!getAccuracy) {
        return nullptr;
    }
    jmethodID getAltitude = env->GetMethodID(cls, "getAltitude", "()D");
    if (!getAltitude) {
        return nullptr;
    }
    jmethodID getSpeed = env->GetMethodID(cls, "getSpeed", "()F");
    if (!getSpeed) {
        return nullptr;
    }
    jmethodID getBearing = env->GetMethodID(cls, "getBearing", "()F");
    if (!getBearing) {
        return nullptr;
    }
    jmethodID getTime = env->GetMethodID(cls, "getTime", "()J");
    if (!getTime) {
        return nullptr;
    }

    jlong timeMs = env->CallLongMethod(res, getTime);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }
    auto loc = std::make_unique<Location::Location>();
    loc->SetLatitude(env->CallDoubleMethod(res, getLatitude));
    loc->SetLongitude(env->CallDoubleMethod(res, getLongitude));
    loc->SetAccuracy(env->CallFloatMethod(res, getAccuracy));
    loc->SetAltitude(env->CallDoubleMethod(res, getAltitude));
    loc->SetSpeed(env->CallFloatMethod(res, getSpeed));
    loc->SetDirection(env->CallFloatMethod(res, getBearing));
    loc->SetTimeStamp(static_cast<int64_t>(timeMs));
    return loc;
}

static Location::LocationErrCode HandleSingleFix(JNIEnv* env,
    const std::unique_ptr<RequestConfig>& requestConfig,
    const sptr<Location::LocatorCallbackNapi>& locatorCallbackHost)
{
    if (!g_locationservicepluginClass.globalRef || !g_locationservicepluginClass.getCurrentLocation) {
        LBSLOGE(LOCATION_SERVICE_JNI, "StartLocating: getCurrentLocation method is null");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto res = env->CallObjectMethod(g_locationservicepluginClass.globalRef,
        g_locationservicepluginClass.getCurrentLocation);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }

    if (res) {
        jclass cls = env->GetObjectClass(res);
        if (!cls) {
            LBSLOGE(LOCATION_SERVICE_JNI, "getCurrentLocation JNI exception");
            env->DeleteLocalRef(res);
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }

        auto loc = ExtractLocation(env, res, cls);
        if (!loc) {
            env->DeleteLocalRef(res);
            env->DeleteLocalRef(cls);
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }

        locatorCallbackHost->SetSingleLocation(loc);
        locatorCallbackHost->OnLocationReport(loc);
        LBSLOGE(LOCATION_SERVICE_JNI, "StartLocating single location dispatched");

        env->DeleteLocalRef(res);
        env->DeleteLocalRef(cls);
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

static void RegisterLocatorCallback(JNIEnv* env, const sptr<Location::LocatorCallbackNapi>& locatorCallbackHost)
{
    bool needObserver = false;
    {
        std::lock_guard<std::mutex> lock(g_locatorCallbacksMutex);
        for (auto& existing : g_locatorCallbacks) {
            if (existing.GetRefPtr() == locatorCallbackHost.GetRefPtr()) {
                return;
            }
        }
        needObserver = g_locatorCallbacks.empty();
        g_locatorCallbacks.push_back(sptr<Location::LocatorCallbackNapi>(locatorCallbackHost));
    }
    if (needObserver) {
        if (!g_locationservicepluginClass.globalRef || !g_locationservicepluginClass.startLocating) {
            return;
        }
        env->CallIntMethod(g_locationservicepluginClass.globalRef, g_locationservicepluginClass.startLocating);
    }
}

Location::LocationErrCode RegisterLocationChangeCallback(JNIEnv* env,
    const std::unique_ptr<RequestConfig>& requestConfig)
{
    if (!g_locationservicepluginClass.globalRef ||
        !g_locationservicepluginClass.registerLocationChangeCallbackWithConfig) {
        LBSLOGE(LOCATION_SERVICE_JNI, "StartLocating: registerLocationChangeCallbackWithConfig method is null");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    jint ret = env->CallIntMethod(
        g_locationservicepluginClass.globalRef,
        g_locationservicepluginClass.registerLocationChangeCallbackWithConfig,
        static_cast<jint>(requestConfig->GetScenario()),
        static_cast<jint>(requestConfig->GetPriority()),
        static_cast<jint>(requestConfig->GetTimeInterval()),
        static_cast<jdouble>(requestConfig->GetDistanceInterval()),
        static_cast<jfloat>(requestConfig->GetMaxAccuracy()),
        static_cast<jint>(requestConfig->GetFixNumber()),
        static_cast<jint>(requestConfig->GetTimeOut()),
        static_cast<jboolean>(requestConfig->GetIsNeedPoi()),
        static_cast<jboolean>(requestConfig->GetIsNeedLocation()));

    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    (void)ret;
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

Location::LocationErrCode LocationServiceJni::StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
    sptr<Location::LocatorCallbackNapi>& locatorCallbackHost)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::StartLocating enter");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!CheckAndLogJniEnv(env)) {
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto errCode = CheckLocationPermission(env);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errCode;
    }
    if (!ValidateRequestConfig(requestConfig, locatorCallbackHost)) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    if (requestConfig->GetFixNumber() == 1) {
        return HandleSingleFix(env, requestConfig, locatorCallbackHost);
    }

    RegisterLocatorCallback(env, locatorCallbackHost);
    return RegisterLocationChangeCallback(env, requestConfig);
}

static inline std::string JStringToCppString(JNIEnv* env, jstring jstr)
{
    if (!jstr) {
        return "";
    }
    const char* cStr = env->GetStringUTFChars(jstr, nullptr);
    if (!cStr) {
        return "";
    }
    std::string cppStr(cStr);
    env->ReleaseStringUTFChars(jstr, cStr);
    return cppStr;
}

enum class LocationSourceType {
    GNSS = 1,
    NETWORK = 2,
    INDOOR = 3,
    RTK = 4
};

static LocationSourceType ConvertSourceType(const std::string& sourceTypeCpp)
{
    if (sourceTypeCpp == "GNSS") {
        return LocationSourceType::GNSS;
    } else if (sourceTypeCpp == "NETWORK") {
        return LocationSourceType::NETWORK;
    } else if (sourceTypeCpp == "INDOOR") {
        return LocationSourceType::INDOOR;
    } else if (sourceTypeCpp == "RTK") {
        return LocationSourceType::RTK;
    } else {
        LBSLOGE(LOCATION_SERVICE_JNI, "Unknown source type: %s", sourceTypeCpp.c_str());
        return LocationSourceType::GNSS; // 默认返回 GNSS
    }
}

void LocationServiceJni::NativeOnLocationChanged(JNIEnv *env, jobject,
    jdouble latitude, jdouble longitude, jfloat accuracy,
    jdouble altitude, jfloat speed, jfloat bearing, jlong timeMs, jfloat uncertaintyOfTimeSinceBoot, jstring sourceType)
{
    auto list = SnapshotCallbacks<Location::LocatorCallbackNapi>(
        g_locatorCallbacks, g_locatorCallbacksMutex);
    if (list.empty()) {
        LBSLOGE(LOCATION_SERVICE_JNI, "NativeOnLocationChanged no listeners");
        return;
    }
    auto sourceTypeCpp = JStringToCppString(env, sourceType);
    LocationSourceType locationSourceType = ConvertSourceType(sourceTypeCpp);
    auto base = std::make_unique<Location::Location>();
    base->SetLatitude((double)latitude);
    base->SetLongitude((double)longitude);
    base->SetAccuracy((float)accuracy);
    base->SetAltitude((double)altitude);
    base->SetSpeed((float)speed);
    base->SetDirection((float)bearing);
    base->SetTimeStamp((int64_t)timeMs);
    base->SetUncertaintyOfTimeSinceBoot((int64_t)uncertaintyOfTimeSinceBoot);
    base->SetLocationSourceType(static_cast<int>(locationSourceType));

    for (auto &cb : list) {
        if (!cb) {
            continue;
        }
        auto copyLoc = std::make_unique<Location::Location>(*base);
        cb->SetSingleLocation(copyLoc);
        cb->OnLocationReport(copyLoc);
    }
}

Location::LocationErrCode LocationServiceJni::StopLocating(sptr<Location::LocatorCallbackNapi>& locatorCallbackHost)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::StopLocating precise");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env || !g_locationservicepluginClass.globalRef ||
        !g_locationservicepluginClass.stopLocating) {
        LBSLOGE(LOCATION_SERVICE_JNI, "StopLocating env/globalRef/method null");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto errCode = CheckLocationPermission(env);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errCode;
    }

    if (!locatorCallbackHost) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }

    bool emptyNow = false;
    {
        std::lock_guard<std::mutex> lock(g_locatorCallbacksMutex);
        auto& vec = g_locatorCallbacks;
        vec.erase(std::remove_if(vec.begin(), vec.end(),
            [&](const sptr<Location::LocatorCallbackNapi>& item) {
                return item && item.GetRefPtr() == locatorCallbackHost.GetRefPtr();
            }),
            vec.end());
        emptyNow = vec.empty();
    }
    if (emptyNow) {
        if (!env || !g_locationservicepluginClass.globalRef ||
            !g_locationservicepluginClass.stopLocating) {
            LBSLOGE(LOCATION_SERVICE_JNI, "StopLocating env/globalRef/method null");
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
        env->CallIntMethod(g_locationservicepluginClass.globalRef,
            g_locationservicepluginClass.stopLocating);
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

Location::LocationErrCode LocationServiceJni::IsGeoConvertAvailable(bool& isAvailable)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::IsGeoConvertAvailable");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LBSLOGE(LOCATION_SERVICE_JNI, "IsGeoConvertAvailable: failed to get jni env");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (!g_locationservicepluginClass.globalRef || !g_locationservicepluginClass.isGeoConvertAvailable) {
        LBSLOGE(LOCATION_SERVICE_JNI, "IsGeoConvertAvailable: globalRef or method ID is null");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    isAvailable = env->CallBooleanMethod(
        g_locationservicepluginClass.globalRef, g_locationservicepluginClass.isGeoConvertAvailable);
    if (env->ExceptionCheck()) {
        LBSLOGE(LOCATION_SERVICE_JNI, "LocationServiceJni JNI: call IsGeoConvertAvailable has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

static bool CreateJavaStrings(JNIEnv* env, const std::unique_ptr<Location::ReverseGeoCodeRequest>& data,
    jstring& jLocale, jstring& jCountry, jstring& jTransId)
{
    const char* localeCStr = data->locale_.empty() ? "" : data->locale_.c_str();
    const char* countryCStr = data->country_.empty() ? "" : data->country_.c_str();
    const char* transIdCStr = data->transId_.empty() ? "" : data->transId_.c_str();
    jLocale = env->NewStringUTF(localeCStr);
    jCountry = env->NewStringUTF(countryCStr);
    jTransId = env->NewStringUTF(transIdCStr);
    if (!jLocale || !jCountry || !jTransId) {
        LBSLOGE(LOCATION_SERVICE_JNI, "Failed to create Java strings");
        if (jLocale) env->DeleteLocalRef(jLocale);
        if (jCountry) env->DeleteLocalRef(jCountry);
        if (jTransId) env->DeleteLocalRef(jTransId);
        return false;
    }
    return true;
}

static inline void ReleaseJavaStrings(JNIEnv* env, jstring jLocale, jstring jCountry, jstring jTransId)
{
    if (jLocale) env->DeleteLocalRef(jLocale);
    if (jCountry) env->DeleteLocalRef(jCountry);
    if (jTransId) env->DeleteLocalRef(jTransId);
}

static inline std::string JStringToCpp(JNIEnv* env, jobject jobjStr)
{
    if (!jobjStr) {
        return "";
    }
    const char* c = env->GetStringUTFChars((jstring)jobjStr, nullptr);
    if (!c) {
        return "";
    }
    std::string out(c);
    env->ReleaseStringUTFChars((jstring)jobjStr, c);
    return out;
}

static void ParseAddress(JNIEnv* env, jobject jAddr, jclass clsAddress,
    const std::unique_ptr<Location::ReverseGeoCodeRequest>& data,
    std::list<std::shared_ptr<Location::GeoAddress>>& replyList)
{
    auto geo = std::make_shared<Location::GeoAddress>();
    geo->latitude_ = env->CallDoubleMethod(jAddr, env->GetMethodID(clsAddress, "getLatitude", "()D"));
    geo->longitude_ = env->CallDoubleMethod(jAddr, env->GetMethodID(clsAddress, "getLongitude", "()D"));
    geo->locale_ = data->locale_;

    geo->countryCode_ = JStringToCpp(env, env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getCountryCode", "()Ljava/lang/String;")));
    geo->countryName_ = JStringToCpp(env, env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getCountryName", "()Ljava/lang/String;")));
    geo->placeName_ = JStringToCpp(env, env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getFeatureName", "()Ljava/lang/String;")));
    geo->administrativeArea_ = JStringToCpp(env, env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getAdminArea", "()Ljava/lang/String;")));
    geo->subAdministrativeArea_ = JStringToCpp(env, env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getSubAdminArea", "()Ljava/lang/String;")));
    geo->locality_ = JStringToCpp(env, env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getLocality", "()Ljava/lang/String;")));
    geo->subLocality_ = JStringToCpp(env, env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getSubLocality", "()Ljava/lang/String;")));
    geo->roadName_ = JStringToCpp(env, env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getThoroughfare", "()Ljava/lang/String;")));
    geo->subRoadName_ = JStringToCpp(env, env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getSubThoroughfare", "()Ljava/lang/String;")));
    geo->postalCode_ = JStringToCpp(env, env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getPostalCode", "()Ljava/lang/String;")));
    geo->phoneNumber_ = JStringToCpp(env, env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getPhone", "()Ljava/lang/String;")));
    geo->addressUrl_ = JStringToCpp(env, env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getUrl", "()Ljava/lang/String;")));

    replyList.push_back(geo);
}

static inline bool ValidateCoordinates(double lat, double lon)
{
    if (lat < MIN_LATITUDE || lat > MAX_LATITUDE || lon < MIN_LONGITUDE || lon > MAX_LONGITUDE ||
        (lat == 0.0 && lon == 0.0)) {
        LBSLOGE(LOCATION_SERVICE_JNI, "GetAddressByCoordinate invalid lat/lon");
        return false;
    }
    return true;
}

static jobjectArray GetAddressArray(JNIEnv* env, const std::unique_ptr<Location::ReverseGeoCodeRequest>& data,
    jstring jLocale, jstring jCountry, jstring jTransId)
{
    if (!g_locationservicepluginClass.globalRef || !g_locationservicepluginClass.getAddressByCoordinate) {
        LBSLOGE(LOCATION_SERVICE_JNI, "GetAddressByCoordinate: globalRef or method ID is null");
        return nullptr;
    }
    return static_cast<jobjectArray>(env->CallObjectMethod(
        g_locationservicepluginClass.globalRef,
        g_locationservicepluginClass.getAddressByCoordinate,
        static_cast<jdouble>(data->latitude_),
        static_cast<jdouble>(data->longitude_),
        static_cast<jint>(data->maxItems_),
        jLocale,
        jCountry,
        jTransId));
}

static Location::LocationErrCode ProcessAddressArray(JNIEnv* env, jobjectArray jAddrArray, jclass clsAddress,
    const std::unique_ptr<Location::ReverseGeoCodeRequest>& data,
    std::list<std::shared_ptr<Location::GeoAddress>>& replyList)
{
    jsize length = env->GetArrayLength(jAddrArray);
    if (length <= 0) {
        return Location::LocationErrCode::ERRCODE_REVERSE_GEOCODING_FAIL;
    }

    for (int i = 0; i < length; i++) {
        jobject jAddr = env->GetObjectArrayElement(jAddrArray, i);
        if (!jAddr) {
            continue;
        }
        ParseAddress(env, jAddr, clsAddress, data, replyList);
        env->DeleteLocalRef(jAddr);
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

Location::LocationErrCode LocationServiceJni::GetAddressByCoordinate(
    std::unique_ptr<Location::ReverseGeoCodeRequest>& data,
    std::list<std::shared_ptr<Location::GeoAddress>>& replyList)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::GetAddressByCoordinate");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        return Location::LocationErrCode::ERRCODE_REVERSE_GEOCODING_FAIL;
    }
    if (!data) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    if (!ValidateCoordinates(data->latitude_, data->longitude_)) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }

    jstring jLocale = nullptr;
    jstring jCountry = nullptr;
    jstring jTransId = nullptr;
    if (!CreateJavaStrings(env, data, jLocale, jCountry, jTransId)) {
        return Location::LocationErrCode::ERRCODE_REVERSE_GEOCODING_FAIL;
    }

    jobjectArray jAddrArray = GetAddressArray(env, data, jLocale, jCountry, jTransId);
    if (env->ExceptionCheck() || !jAddrArray) {
        LBSLOGE(LOCATION_SERVICE_JNI, "GetAddressByCoordinate JNI exception or no results");
        env->ExceptionDescribe();
        env->ExceptionClear();
        ReleaseJavaStrings(env, jLocale, jCountry, jTransId);
        return Location::LocationErrCode::ERRCODE_REVERSE_GEOCODING_FAIL;
    }
    jclass clsAddress = env->FindClass(LOCATION_CLASS_NAME);
    if (!clsAddress) {
        LBSLOGE(LOCATION_SERVICE_JNI, "GetAddressByCoordinate cannot find Address class");
        env->DeleteLocalRef(jAddrArray);
        ReleaseJavaStrings(env, jLocale, jCountry, jTransId);
        return Location::LocationErrCode::ERRCODE_REVERSE_GEOCODING_FAIL;
    }

    auto result = ProcessAddressArray(env, jAddrArray, clsAddress, data, replyList);
    if (env->ExceptionCheck()) {
        LBSLOGE(LOCATION_SERVICE_JNI, "GetAddressByCoordinate: Exception in ProcessAddressArray");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteLocalRef(clsAddress);
        env->DeleteLocalRef(jAddrArray);
        ReleaseJavaStrings(env, jLocale, jCountry, jTransId);
        return Location::LocationErrCode::ERRCODE_REVERSE_GEOCODING_FAIL;
    }

    env->DeleteLocalRef(clsAddress);
    env->DeleteLocalRef(jAddrArray);
    ReleaseJavaStrings(env, jLocale, jCountry, jTransId);
    return result;
}

static bool CreateJavaStrings(JNIEnv* env, const std::unique_ptr<Location::GeoCodeRequest>& request,
    jstring& jDesc, jstring& jLocale, jstring& jCountry, jstring& jTransId)
{
    jDesc = env->NewStringUTF(request->description_.c_str());
    if (!jDesc) {
        env->DeleteLocalRef(jDesc);
        return false;
    }
    jLocale = env->NewStringUTF(request->locale_.c_str());
    if (!jLocale) {
        env->DeleteLocalRef(jDesc);
        env->DeleteLocalRef(jLocale);
        return false;
    }
    jCountry = env->NewStringUTF(request->country_.c_str());
    if (!jCountry) {
        env->DeleteLocalRef(jDesc);
        env->DeleteLocalRef(jLocale);
        env->DeleteLocalRef(jCountry);
        return false;
    }
    jTransId = env->NewStringUTF(request->transId_.c_str());
    if (!jTransId) {
        env->DeleteLocalRef(jDesc);
        env->DeleteLocalRef(jLocale);
        env->DeleteLocalRef(jCountry);
        env->DeleteLocalRef(jTransId);
        return false;
    }
    return true;
}

static inline void ReleaseJavaStrings(JNIEnv* env, jstring jDesc, jstring jLocale, jstring jCountry, jstring jTransId)
{
    if (jDesc) env->DeleteLocalRef(jDesc);
    if (jLocale) env->DeleteLocalRef(jLocale);
    if (jCountry) env->DeleteLocalRef(jCountry);
    if (jTransId) env->DeleteLocalRef(jTransId);
}

static void ParseAddress(JNIEnv* env, jobject jAddr, jclass clsAddress,
    const std::unique_ptr<Location::GeoCodeRequest>& request,
    std::list<std::shared_ptr<Location::GeoAddress>>& replyList)
{
    auto geo = std::make_shared<Location::GeoAddress>();
    geo->latitude_ = env->CallDoubleMethod(jAddr, env->GetMethodID(clsAddress, "getLatitude", "()D"));
    geo->longitude_ = env->CallDoubleMethod(jAddr, env->GetMethodID(clsAddress, "getLongitude", "()D"));
    geo->locale_ = request->locale_;

    auto grabStr = [env](jobject js) -> std::string {
        if (!js)
            return "";
        const char* c = env->GetStringUTFChars((jstring)js, nullptr);
        std::string s(c ? c : "");
        if (c)
            env->ReleaseStringUTFChars((jstring)js, c);
        return s;
    };

    geo->placeName_ = grabStr(env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getFeatureName", "()Ljava/lang/String;")));
    geo->countryCode_ = grabStr(env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getCountryCode", "()Ljava/lang/String;")));
    geo->countryName_ = grabStr(env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getCountryName", "()Ljava/lang/String;")));
    geo->locality_ = grabStr(env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getLocality", "()Ljava/lang/String;")));
    geo->administrativeArea_ = grabStr(env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getAdminArea", "()Ljava/lang/String;")));
    geo->subAdministrativeArea_ = grabStr(env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getSubAdminArea", "()Ljava/lang/String;")));
    geo->subLocality_ = grabStr(env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getSubLocality", "()Ljava/lang/String;")));
    geo->roadName_ = grabStr(env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getThoroughfare", "()Ljava/lang/String;")));
    geo->subRoadName_ = grabStr(env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getSubThoroughfare", "()Ljava/lang/String;")));
    geo->postalCode_ = grabStr(env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getPostalCode", "()Ljava/lang/String;")));
    geo->phoneNumber_ = grabStr(env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getPhone", "()Ljava/lang/String;")));
    geo->addressUrl_ = grabStr(env->CallObjectMethod(
        jAddr, env->GetMethodID(clsAddress, "getUrl", "()Ljava/lang/String;")));

    replyList.push_back(geo);
}

static Location::LocationErrCode ProcessAddressArray(JNIEnv* env, jobjectArray jAddrArray, jclass clsAddress,
    const std::unique_ptr<Location::GeoCodeRequest>& request,
    std::list<std::shared_ptr<Location::GeoAddress>>& replyList, jint maxItems)
{
    jsize len = env->GetArrayLength(jAddrArray);
    int outCount = std::min<int>(len, maxItems);

    for (int i = 0; i < outCount; i++) {
        jobject jAddr = env->GetObjectArrayElement(jAddrArray, i);
        if (!jAddr) {
            continue;
        }
        ParseAddress(env, jAddr, clsAddress, request, replyList);
        env->DeleteLocalRef(jAddr);

        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            break;
        }
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

static inline void HandleJniExceptionAndRelease(JNIEnv* env,
    jstring jDesc, jstring jLocale, jstring jCountry, jstring jTransId)
{
    LBSLOGE(LOCATION_SERVICE_JNI, "GetAddressByLocationName(sync) JNI exception or no results");
    env->ExceptionDescribe();
    env->ExceptionClear();
    ReleaseJavaStrings(env, jDesc, jLocale, jCountry, jTransId);
}

static Location::LocationErrCode PrepareAndProcessAddress(JNIEnv* env,
    std::unique_ptr<Location::GeoCodeRequest>& request,
    std::list<std::shared_ptr<Location::GeoAddress>>& replyList,
    jstring jDesc, jstring jLocale, jstring jCountry, jstring jTransId)
{
    if (!g_locationservicepluginClass.globalRef || !g_locationservicepluginClass.getAddressByLocationName) {
        LBSLOGE(LOCATION_SERVICE_JNI, "GetAddressByLocationName: globalRef or method ID is null");
        ReleaseJavaStrings(env, jDesc, jLocale, jCountry, jTransId);
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    jobjectArray jAddrArray = (jobjectArray)env->CallObjectMethod(
        g_locationservicepluginClass.globalRef, g_locationservicepluginClass.getAddressByLocationName, jDesc,
        (jint)request->maxItems_, jLocale, jCountry, (jdouble)request->minLatitude_, (jdouble)request->minLongitude_,
        (jdouble)request->maxLatitude_, (jdouble)request->maxLongitude_, jTransId);
    if (env->ExceptionCheck() || !jAddrArray) {
        HandleJniExceptionAndRelease(env, jDesc, jLocale, jCountry, jTransId);
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    jclass clsAddress = env->FindClass(LOCATION_CLASS_NAME);
    if (!clsAddress) {
        LBSLOGE(LOCATION_SERVICE_JNI, "GetAddressByLocationName(sync) Address class not found");
        env->DeleteLocalRef(jAddrArray);
        ReleaseJavaStrings(env, jDesc, jLocale, jCountry, jTransId);
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }

    auto errCode = ProcessAddressArray(env, jAddrArray, clsAddress, request, replyList, request->maxItems_);
    env->DeleteLocalRef(clsAddress);
    env->DeleteLocalRef(jAddrArray);
    ReleaseJavaStrings(env, jDesc, jLocale, jCountry, jTransId);
    return errCode;
}

Location::LocationErrCode LocationServiceJni::GetAddressByLocationName(
    std::unique_ptr<Location::GeoCodeRequest>& request,
    std::list<std::shared_ptr<Location::GeoAddress>>& replyList)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::GetAddressByLocationName(sync) enter");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LBSLOGE(LOCATION_SERVICE_JNI, "GetAddressByLocationName(sync): failed to get jni env");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto errCode = CheckLocationPermission(env);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errCode;
    }
    if (!request) {
        LBSLOGE(LOCATION_SERVICE_JNI, "GetAddressByLocationName(sync) request null");
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }

    jstring jDesc = nullptr;
    jstring jLocale = nullptr;
    jstring jCountry = nullptr;
    jstring jTransId = nullptr;
    if (!CreateJavaStrings(env, request, jDesc, jLocale, jCountry, jTransId)) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }

    errCode = PrepareAndProcessAddress(env, request, replyList, jDesc, jLocale, jCountry, jTransId);

    LBSLOGI(LOCATION_SERVICE_JNI, "GetAddressByLocationName(sync) done size=%zu", replyList.size());
    return errCode;
}

std::shared_ptr<Location::CountryCode> LocationServiceJni::GetIsoCountryCode()
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::GetIsoCountryCode");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LBSLOGE(LOCATION_SERVICE_JNI, "GetIsoCountryCode: failed to get jni env");
        return nullptr;
    }
    if (!g_locationservicepluginClass.globalRef || !g_locationservicepluginClass.getIsoCountryCode) {
        LBSLOGE(LOCATION_SERVICE_JNI, "GetIsoCountryCode: globalRef or method ID is null");
        return nullptr;
    }
    auto countryCodeFromJava = env->CallObjectMethod(
        g_locationservicepluginClass.globalRef, g_locationservicepluginClass.getIsoCountryCode);
    if (env->ExceptionCheck()) {
        LBSLOGE(LOCATION_SERVICE_JNI, "LocationServiceJni JNI: call GetIsoCountryCode has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }
    auto countryCodestr = static_cast<jstring>(countryCodeFromJava);
    const char *nativeString = env->GetStringUTFChars(countryCodestr, nullptr);
    std::string cppString(nativeString);
    env->ReleaseStringUTFChars(countryCodestr, nativeString);
    auto ret = std::make_shared<Location::CountryCode>();
    ret->SetCountryCodeStr(cppString);
    return ret;
}

static int32_t GenerateFenceId()
{
    std::lock_guard<std::mutex> lock(g_fenceIdMutex);

    // Lazy load persisted counter
    if (!g_fenceIdLoaded) {
        std::ifstream inputFile(COUNTER_FILE);
        if (inputFile.is_open()) {
            int64_t persistedValue;
            inputFile >> persistedValue;
            if (persistedValue >= 0 && persistedValue <= FENCE_MAX_ID) {
                g_fenceIdCounter = static_cast<int32_t>(persistedValue);
            }
            inputFile.close();
        }
        g_fenceIdLoaded = true;
    }

    if (g_fenceIdCounter >= FENCE_MAX_ID) {
        g_fenceIdCounter = 0; // wrap
    }
    ++g_fenceIdCounter;

    // Persist new value (best-effort; failure is non-fatal)
    std::ofstream outputFile(COUNTER_FILE);
    if (outputFile.is_open()) {
        outputFile << g_fenceIdCounter;
        outputFile.close();
    }

    return g_fenceIdCounter;
}

static int64_t ComputeRemainingExpirationMs(GeofenceRequest& request)
{
    int64_t abs = request.GetRequestExpirationTime();
    if (abs <= 0) {
        return 0;
    }
    int64_t now = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    int64_t remain = abs - now;
    if (remain <= 0) {
        return 1;
    }
    int64_t ms = remain / MILL_TO_NANOS;
    return ms == 0 ? 1 : ms;
}

static inline void StoreFenceNotifications(int32_t fenceId,
    const std::vector<std::shared_ptr<Notification::NotificationRequest>>& list)
{
#ifdef NOTIFICATION_ENABLE
    if (list.empty()) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_geofenceNotiMutex);
    g_geofenceNotifications[fenceId] = list;
    LBSLOGI(LOCATION_SERVICE_JNI, "StoreFenceNotifications fenceId=%d size=%zu",
        fenceId, list.size());
#endif
}

static inline std::vector<std::shared_ptr<Notification::NotificationRequest>> GetFenceNotifications(int32_t fenceId)
{
#ifdef NOTIFICATION_ENABLE
    std::lock_guard<std::mutex> lock(g_geofenceNotiMutex);
    auto it = g_geofenceNotifications.find(fenceId);
    if (it == g_geofenceNotifications.end()) {
        return {};
    }
    return it->second;
#else
    (void)fenceId;
    return {};
#endif
}

inline void RemoveFenceNotifications(int32_t fenceId)
{
#ifdef NOTIFICATION_ENABLE
    std::lock_guard<std::mutex> lock(g_geofenceNotiMutex);
    g_geofenceNotifications.erase(fenceId);
    LBSLOGI(LOCATION_SERVICE_JNI, "RemoveFenceNotifications fenceId=%d", fenceId);
#else
    (void)fenceId;
#endif
}

#ifdef NOTIFICATION_ENABLE
static inline void PublishFenceNotificationNow(const std::shared_ptr<OHOS::Notification::NotificationRequest>& reqPtr)
{
    if (!reqPtr) {
        return;
    }
    OHOS::Notification::NotificationHelper::PublishNotification(
        *reqPtr,
        nullptr,
        [](void*, int32_t code) {
            LBSLOGI(LOCATION_SERVICE_JNI, "PublishFenceNotificationNow result=%d", code);
        });
}
#endif

inline bool ValidateGeofenceParams(Location::GeoFence& g)
{
    if (g.latitude < MIN_LATITUDE || g.latitude > MAX_LATITUDE ||
        g.longitude < MIN_LONGITUDE || g.longitude > MAX_LONGITUDE) {
        LBSLOGE(LOCATION_SERVICE_JNI, "AddGnssGeofence invalid lat/lon");
        return false;
    }
    if (g.radius <= MIN_GEOFENCE_RADIUS  || g.radius > MAX_GEOFENCE_RADIUS) {
        LBSLOGE(LOCATION_SERVICE_JNI, "AddGnssGeofence invalid radius=%f", g.radius);
        return false;
    }
    return true;
}

static inline void StampAbsoluteExpirationIfNeeded(GeofenceRequest& request)
{
    auto g = request.GetGeofence();
    if (g.expiration <= 0) {
        return;
    }
    int64_t nowNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    int64_t absNs = nowNs + static_cast<int64_t>(g.expiration) * MILL_TO_NANOS;
    request.SetRequestExpirationTime(absNs);
}

static inline void ReportInvalidGeofenceParams(
    sptr<Location::LocationGnssGeofenceCallbackNapi>& callback)
{
    callback->OnReportOperationResult(0,
        (int)Location::GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_ADD,
        (int)Location::GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_PARAMS_INVALID);
}

static bool CallAddGnssGeofence(JNIEnv* env, const Location::GeoFence& geofence,
    int64_t expirationMs, int32_t fenceId)
{
    if (!env || !g_locationservicepluginClass.globalRef ||
        !g_locationservicepluginClass.addGnssGeofence) {
        LBSLOGE(LOCATION_SERVICE_JNI, "AddGnssGeofence env/globalRef/method null");
        return false;
    }
    env->CallVoidMethod(
        g_locationservicepluginClass.globalRef,
        g_locationservicepluginClass.addGnssGeofence,
        geofence.latitude,
        geofence.longitude,
        (jfloat)geofence.radius,
        (jlong)expirationMs,
        (jint)fenceId);

    if (env->ExceptionCheck()) {
        LBSLOGE(LOCATION_SERVICE_JNI, "AddGnssGeofence JNI exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return true;
}

static void ReportGeofenceAddFailure(JNIEnv* env,
    sptr<Location::LocationGnssGeofenceCallbackNapi>& callback, int32_t fenceId)
{
    callback->OnReportOperationResult(fenceId,
        (int)Location::GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_ADD,
        (int)Location::GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_UNKNOWN);
}

static void RegisterGeofenceCallback(int32_t fenceId,
    sptr<Location::LocationGnssGeofenceCallbackNapi>& callback)
{
    std::lock_guard<std::mutex> lock(g_geofenceCbMutex);
    g_geofenceCallbackMap[fenceId] = callback;
}

static void HandleNotifications(GeofenceRequest& request, int32_t fenceId)
{
#ifdef NOTIFICATION_ENABLE
    auto notiList = request.GetNotificationRequestList();
    if (!notiList.empty()) {
        StoreFenceNotifications(fenceId, notiList);
    }
#endif
}

static Location::LocationErrCode AddGnssGeofenceInternal(JNIEnv* env, GeofenceRequest& request,
    sptr<Location::LocationGnssGeofenceCallbackNapi>& callback)
{
    auto geofence = request.GetGeofence();
    if (!ValidateGeofenceParams(geofence)) {
        ReportInvalidGeofenceParams(callback);
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }

    auto fenceId = GenerateFenceId();
    request.SetFenceId(fenceId);
    StampAbsoluteExpirationIfNeeded(request);
    int64_t expirationMs = ComputeRemainingExpirationMs(request);
    if (!CallAddGnssGeofence(env, geofence, expirationMs, fenceId)) {
        ReportGeofenceAddFailure(env, callback, fenceId);
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }

    RegisterGeofenceCallback(fenceId, callback);
    HandleNotifications(request, fenceId);
    callback->OnReportOperationResult(fenceId,
        (int)Location::GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_ADD,
        (int)Location::GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_SUCCESS);

    LBSLOGI(LOCATION_SERVICE_JNI, "AddGnssGeofence success fenceId=%d expMs=%lld", fenceId,
        (long long)expirationMs);
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

Location::LocationErrCode LocationServiceJni::AddGnssGeofence(GeofenceRequest& request,
    sptr<Location::LocationGnssGeofenceCallbackNapi>& callback)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::AddGnssGeofence");

    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LBSLOGE(LOCATION_SERVICE_JNI, "AddGnssGeofence: failed to get jni env");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }

    auto errCode = CheckLocationPermission(env);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errCode;
    }

    if (!callback) {
        LBSLOGE(LOCATION_SERVICE_JNI, "AddGnssGeofence callback null");
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }

    if (!g_locationservicepluginClass.globalRef || !g_locationservicepluginClass.addGnssGeofence) {
        LBSLOGE(LOCATION_SERVICE_JNI, "AddGnssGeofence env/globalRef/method null");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }

    return AddGnssGeofenceInternal(env, request, callback);
}

void LocationServiceJni::NativeOnGeofenceEvent(JNIEnv *env, jobject,
    jint fenceId)
{
    sptr<Location::LocationGnssGeofenceCallbackNapi> cb;
    {
        std::lock_guard<std::mutex> lock(g_geofenceCbMutex);
        auto it = g_geofenceCallbackMap.find((int)fenceId);
        if (it != g_geofenceCallbackMap.end()) {
            cb = it->second;
        }
    }
    if (!cb) {
        LBSLOGE(LOCATION_SERVICE_JNI, "NativeOnGeofenceEvent no callback fenceId=%d", (int)fenceId);
        return;
    }
    cb->OnReportOperationResult((int)fenceId,
        (int)Location::GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_ADD,
        (int)Location::GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_SUCCESS);
}

static inline sptr<OHOS::Location::LocationGnssGeofenceCallbackNapi> GetGeofenceCallback(int32_t fenceId)
{
    std::lock_guard<std::mutex> lock(g_geofenceCbMutex);
    auto it = g_geofenceCallbackMap.find(fenceId);
    if (it != g_geofenceCallbackMap.end()) {
        return it->second;
    }
    return nullptr;
}

void LocationServiceJni::NativeOnNotificationEvent(JNIEnv *env, jobject,
    jint fenceId, jboolean entering)
{
    auto cb = GetGeofenceCallback(fenceId);
    if (cb) {
        Location::GeofenceTransition transition;
        transition.fenceId = fenceId;
        transition.event = OHOS::Location::GEOFENCE_TRANSITION_EVENT_ENTER;
        transition.beaconFence = nullptr;
        cb->OnTransitionStatusChange(transition);
    }
#ifdef NOTIFICATION_ENABLE
        auto list = GetFenceNotifications(fenceId);
        if (!list.empty()) {
            LBSLOGI(LOCATION_SERVICE_JNI, "NativeOnNotificationEvent fenceId=%d entering=%d size=%zu",
                fenceId, (int)entering, list.size());
            for (auto &n : list) {
                PublishFenceNotificationNow(n);
            }
        }
#endif
}

Location::LocationErrCode LocationServiceJni::RemoveGnssGeofence(int32_t fenceId,
    sptr<Location::LocationGnssGeofenceCallbackNapi>& callback)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::RemoveGnssGeofence fenceId=%d", fenceId);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto errCode = CheckLocationPermission(env);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errCode;
    }
    if (fenceId <= 0 || !callback) {
        if (callback) {
            callback->OnReportOperationResult(fenceId,
                (int)Location::GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_DELETE,
                (int)Location::GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_PARAMS_INVALID);
        }
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    if (!g_locationservicepluginClass.globalRef || !g_locationservicepluginClass.removeGnssGeofence) {
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (!GetGeofenceCallback(fenceId)) {
        callback->OnReportOperationResult(fenceId,
            (int)Location::GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_DELETE,
            (int)Location::GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_PARAMS_INVALID);
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    env->CallVoidMethod(
        g_locationservicepluginClass.globalRef,
        g_locationservicepluginClass.removeGnssGeofence,
        (jint)fenceId);

    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        callback->OnReportOperationResult(fenceId,
            (int)Location::GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_DELETE,
            (int)Location::GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_UNKNOWN);
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }

    {
        std::lock_guard<std::mutex> lock(g_geofenceCbMutex);
        g_geofenceCallbackMap.erase(fenceId);
    }
    RemoveFenceNotifications(fenceId);

    callback->OnReportOperationResult(fenceId,
        (int)Location::GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_DELETE,
        (int)Location::GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_SUCCESS);

    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

Location::LocationErrCode LocationServiceJni::GetCurrentWifiBssidForLocating(std::string &bssid)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::GetCurrentWifiBssidForLocating");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LBSLOGE(LOCATION_SERVICE_JNI, "GetCurrentWifiBssidForLocating: failed to get jni env");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto errCode = CheckLocationPermission(env);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errCode;
    }
    if (!g_locationservicepluginClass.globalRef || !g_locationservicepluginClass.getCurrentWifiBssidForLocating) {
        LBSLOGE(LOCATION_SERVICE_JNI, "GetCurrentWifiBssidForLocating: globalRef or method ID is null");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto result = env->CallObjectMethod(
        g_locationservicepluginClass.globalRef, g_locationservicepluginClass.getCurrentWifiBssidForLocating);
    if (env->ExceptionCheck()) {
        LBSLOGE(LOCATION_SERVICE_JNI, "LocationServiceJni JNI: call GetCurrentWifiBssidForLocating has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Location::LocationErrCode::ERRCODE_PERMISSION_DENIED;
    }
    if (result == nullptr) {
        LBSLOGE(LOCATION_SERVICE_JNI, "LocationServiceJni::result is null");
        bssid = "";
        return Location::LocationErrCode::ERRCODE_PERMISSION_DENIED;
    }
    auto resultStr = static_cast<jstring>(result);
    const char *nativeString = env->GetStringUTFChars(resultStr, nullptr);
    std::string cppString(nativeString);
    env->ReleaseStringUTFChars(resultStr, nativeString);
    bssid = cppString;
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

Location::LocationErrCode LocationServiceJni::RegisterSwitchCallback(
    sptr<Location::LocationSwitchCallbackNapi>& switchCallbackHost)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::RegisterSwitchCallback");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LBSLOGE(LOCATION_SERVICE_JNI, "RegisterSwitchCallback: failed to get jni env");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto errCode = CheckLocationPermission(env);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errCode;
    }

    if (!switchCallbackHost) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    bool needObserver = false;
    {
        std::lock_guard<std::mutex> lock(g_switchCallbacksMutex);
        for (auto &existing : g_switchCallbacks) {
            if (existing.GetRefPtr() == switchCallbackHost.GetRefPtr()) {
                return Location::LocationErrCode::ERRCODE_SUCCESS;
            }
        }
        needObserver = g_switchCallbacks.empty();
        g_switchCallbacks.push_back(sptr<Location::LocationSwitchCallbackNapi>(switchCallbackHost));
    }
    if (needObserver) {
        if (!g_locationservicepluginClass.globalRef ||
            !g_locationservicepluginClass.registerSwitchCallback) {
            LBSLOGE(LOCATION_SERVICE_JNI, "RegisterSwitchCallback: globalRef or method ID is null");
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
        env->CallVoidMethod(
            g_locationservicepluginClass.globalRef, g_locationservicepluginClass.registerSwitchCallback);
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

Location::LocationErrCode LocationServiceJni::UnregisterSwitchCallback(
    sptr<Location::LocationSwitchCallbackNapi>& switchCallbackHost)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::UnregisterSwitchCallback");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LBSLOGE(LOCATION_SERVICE_JNI, "UnregisterSwitchCallback: failed to get jni env");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto errCode = CheckLocationPermission(env);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errCode;
    }
    if (!switchCallbackHost) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    bool emptyNow = false;
    {
        std::lock_guard<std::mutex> lock(g_switchCallbacksMutex);
        auto& vec = g_switchCallbacks;
        vec.erase(std::remove_if(vec.begin(), vec.end(),
            [&](const sptr<Location::LocationSwitchCallbackNapi>& item) {
                return item && item.GetRefPtr() == switchCallbackHost.GetRefPtr();
            }),
            vec.end());
        emptyNow = vec.empty();
    }
    if (emptyNow) {
        if (!g_locationservicepluginClass.globalRef ||
            !g_locationservicepluginClass.unregisterSwitchCallback) {
            LBSLOGE(LOCATION_SERVICE_JNI, "UnregisterSwitchCallback: globalRef or method ID is null");
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
        env->CallVoidMethod(
            g_locationservicepluginClass.globalRef, g_locationservicepluginClass.unregisterSwitchCallback);
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

void LocationServiceJni::NativeOnSwitchStateChanged(JNIEnv *env, jobject, jint enabled)
{
    auto list = SnapshotCallbacks<Location::LocationSwitchCallbackNapi>(
        g_switchCallbacks, g_switchCallbacksMutex);
    for (auto &cb : list) {
        if (cb) {
            cb->OnSwitchChange((int)enabled);
        }
    }
}

Location::LocationErrCode LocationServiceJni::RegisterCountryCodeCallback(
    sptr<Location::CountryCodeCallbackNapi>& callbackHost)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::RegisterCountryCodeCallback");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LBSLOGE(LOCATION_SERVICE_JNI, "RegisterCountryCodeCallback: failed to get jni env");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto errCode = CheckLocationPermission(env);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errCode;
    }

    if (!callbackHost) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    bool needObserver = false;
    {
        std::lock_guard<std::mutex> lock(g_countryCodeCallbacksMutex);
        for (auto &existing : g_countryCodeCallbacks) {
            if (existing.GetRefPtr() == callbackHost.GetRefPtr()) {
                return Location::LocationErrCode::ERRCODE_SUCCESS;
            }
        }
        needObserver = g_countryCodeCallbacks.empty();
        g_countryCodeCallbacks.push_back(sptr<Location::CountryCodeCallbackNapi>(callbackHost));
    }
    if (needObserver) {
        if (!g_locationservicepluginClass.globalRef || !g_locationservicepluginClass.registerCountryCodeCallback) {
            LBSLOGE(LOCATION_SERVICE_JNI, "RegisterCountryCodeCallback: globalRef or method ID is null");
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
        auto code = GetIsoCountryCode();
        callbackHost->OnCountryCodeChange(code);
        env->CallVoidMethod(
            g_locationservicepluginClass.globalRef, g_locationservicepluginClass.registerCountryCodeCallback);
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

Location::LocationErrCode LocationServiceJni::UnregisterCountryCodeCallback(
    sptr<Location::CountryCodeCallbackNapi>& callbackHost)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::UnregisterCountryCodeCallback");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LBSLOGE(LOCATION_SERVICE_JNI, "UnregisterCountryCodeCallback: failed to get jni env");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto errCode = CheckLocationPermission(env);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errCode;
    }
    if (!callbackHost) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    bool becameEmpty = false;
    {
        std::lock_guard<std::mutex> lock(g_countryCodeCallbacksMutex);
        auto& vec = g_countryCodeCallbacks;
        vec.erase(std::remove_if(vec.begin(), vec.end(),
            [&](const sptr<Location::CountryCodeCallbackNapi>& item) {
                return item && item.GetRefPtr() == callbackHost.GetRefPtr();
            }),
            vec.end());
        becameEmpty = vec.empty();
    }
    if (becameEmpty) {
        if (!g_locationservicepluginClass.globalRef ||
            !g_locationservicepluginClass.unregisterCountryCodeCallback) {
            LBSLOGE(LOCATION_SERVICE_JNI, "UnregisterCountryCodeCallback: globalRef or method ID is null");
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
        env->CallVoidMethod(
            g_locationservicepluginClass.globalRef, g_locationservicepluginClass.unregisterCountryCodeCallback);
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

void LocationServiceJni::NativeOnCountryCodeChanged(JNIEnv *env, jobject, jstring jCode, jint type)
{
    auto list = SnapshotCallbacks<Location::CountryCodeCallbackNapi>(
        g_countryCodeCallbacks, g_countryCodeCallbacksMutex);
    if (list.empty()) {
        return;
    }

    std::string code;
    if (jCode) {
        const char* c = env->GetStringUTFChars(jCode, nullptr);
        if (c) {
            code.assign(c);
            env->ReleaseStringUTFChars(jCode, c);
        }
    }
    for (auto &cb : list) {
        if (!cb) {
            continue;
        }
        auto country = std::make_shared<Location::CountryCode>();
        country->SetCountryCodeStr(code);
        country->SetCountryCodeType((int)type);
        cb->OnCountryCodeChange(country);
    }
}

Location::LocationErrCode LocationServiceJni::RegisterGnssStatusCallback(sptr<Location::GnssStatusCallbackNapi>& cb)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::RegisterGnssStatusCallback");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LBSLOGE(LOCATION_SERVICE_JNI, "RegisterGnssStatusCallback: failed to get jni env");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto errCode = CheckLocationPermission(env);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errCode;
    }
    if (!cb) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    bool needObserver = false;
    {
        std::lock_guard<std::mutex> lock(g_gnssStatusCallbacksMutex);
        for (auto &existing : g_gnssStatusCallbacks) {
            if (existing.GetRefPtr() == cb.GetRefPtr()) {
                return Location::LocationErrCode::ERRCODE_SUCCESS;
            }
        }
        needObserver = g_gnssStatusCallbacks.empty();
        g_gnssStatusCallbacks.push_back(sptr<Location::GnssStatusCallbackNapi>(cb));
    }
    if (needObserver) {
        if (!g_locationservicepluginClass.globalRef ||
            !g_locationservicepluginClass.registerGnssStatusCallback) {
            LBSLOGE(LOCATION_SERVICE_JNI, "RegisterGnssStatusCallback: globalRef or method ID is null");
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
        env->CallVoidMethod(g_locationservicepluginClass.globalRef,
            g_locationservicepluginClass.registerGnssStatusCallback);
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

Location::LocationErrCode LocationServiceJni::UnregisterGnssStatusCallback(
    sptr<Location::GnssStatusCallbackNapi>& callback)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::UnregisterGnssStatusCallback");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LBSLOGE(LOCATION_SERVICE_JNI, "UnregisterGnssStatusCallback: failed to get jni env");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto errCode = CheckLocationPermission(env);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errCode;
    }

    if (!callback) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    bool emptyNow = false;
    {
        std::lock_guard<std::mutex> lock(g_gnssStatusCallbacksMutex);
        auto& vec = g_gnssStatusCallbacks;
        vec.erase(std::remove_if(vec.begin(), vec.end(),
            [&](const sptr<Location::GnssStatusCallbackNapi>& item) {
                return item && item.GetRefPtr() == callback.GetRefPtr();
            }),
            vec.end());
        emptyNow = vec.empty();
    }
    if (emptyNow) {
        if (!g_locationservicepluginClass.globalRef ||
            !g_locationservicepluginClass.unregisterGnssStatusCallback) {
            LBSLOGE(LOCATION_SERVICE_JNI, "UnregisterGnssStatusCallback: globalRef or method ID is null");
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
        env->CallVoidMethod(g_locationservicepluginClass.globalRef,
            g_locationservicepluginClass.unregisterGnssStatusCallback);
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

Location::LocationErrCode LocationServiceJni::RegisterNmeaMessageCallback(sptr<Location::NmeaMessageCallbackNapi> &cb)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::RegisterNmeaMessageCallback");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LBSLOGE(LOCATION_SERVICE_JNI, "RegisterNmeaMessageCallback: failed to get jni env");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto errCode = CheckLocationPermission(env);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errCode;
    }

    if (!cb) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    bool needObserver = false;
    {
        std::lock_guard<std::mutex> lock(g_nmeaCallbacksMutex);
        for (auto &existing : g_nmeaCallbacks) {
            if (existing.GetRefPtr() == cb.GetRefPtr()) {
                return Location::LocationErrCode::ERRCODE_SUCCESS;
            }
        }
        needObserver = g_nmeaCallbacks.empty();
        g_nmeaCallbacks.push_back(sptr<Location::NmeaMessageCallbackNapi>(cb));
    }
    if (needObserver) {
        if (!g_locationservicepluginClass.globalRef ||
            !g_locationservicepluginClass.registerNmeaMessageCallback) {
            LBSLOGE(LOCATION_SERVICE_JNI, "RegisterNmeaMessageCallback: globalRef or method ID is null");
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
        env->CallVoidMethod(g_locationservicepluginClass.globalRef,
            g_locationservicepluginClass.registerNmeaMessageCallback);
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

Location::LocationErrCode LocationServiceJni::UnregisterNmeaMessageCallback(sptr<Location::NmeaMessageCallbackNapi>& cb)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::UnregisterNmeaMessageCallback");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LBSLOGE(LOCATION_SERVICE_JNI, "UnregisterNmeaMessageCallback: failed to get jni env");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto errCode = CheckLocationPermission(env);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errCode;
    }
    if (!cb) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    bool emptyNow = false;
    {
        std::lock_guard<std::mutex> lock(g_nmeaCallbacksMutex);
        auto& vec = g_nmeaCallbacks;
        vec.erase(std::remove_if(vec.begin(), vec.end(),
            [&](const sptr<Location::NmeaMessageCallbackNapi>& item) {
                return item && item.GetRefPtr() == cb.GetRefPtr();
            }),
            vec.end());
        emptyNow = vec.empty();
    }
    if (emptyNow) {
        if (!g_locationservicepluginClass.globalRef ||
            !g_locationservicepluginClass.unregisterNmeaMessageCallback) {
            LBSLOGE(LOCATION_SERVICE_JNI, "UnregisterNmeaMessageCallback: globalRef or method ID is null");
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
        env->CallVoidMethod(g_locationservicepluginClass.globalRef,
            g_locationservicepluginClass.unregisterNmeaMessageCallback);
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

void LocationServiceJni::NativeOnNmeaMessage(JNIEnv *env, jobject, jlong timestamp, jstring jSentence)
{
    auto list = SnapshotCallbacks<Location::NmeaMessageCallbackNapi>(
        g_nmeaCallbacks, g_nmeaCallbacksMutex);
    if (list.empty()) {
        return;
    }
    std::string sentence;
    if (jSentence) {
        const char *c = env->GetStringUTFChars(jSentence, nullptr);
        if (c) {
            sentence.assign(c);
            env->ReleaseStringUTFChars(jSentence, c);
        }
    }
    for (auto &cb : list) {
        if (cb) {
            cb->OnMessageChange((int64_t)timestamp, sentence);
        }
    }
}

static inline void FillIntArray(JNIEnv* env, jintArray arr,
    const std::function<void(std::vector<int>&)>& setter)
{
    if (!arr) {
        return;
    }
    jsize len = env->GetArrayLength(arr);
    if (len <= 0) {
        return;
    }
    std::vector<int> vec(len);
    env->GetIntArrayRegion(arr, 0, len, vec.data());
    setter(vec);
}

static inline void FillDoubleArray(JNIEnv* env, jdoubleArray arr,
    const std::function<void(std::vector<double>&)>& setter)
{
    if (!arr) {
        return;
    }
    jsize len = env->GetArrayLength(arr);
    if (len <= 0) {
        return;
    }
    std::vector<double> vec(len);
    env->GetDoubleArrayRegion(arr, 0, len, vec.data());
    setter(vec);
}

void LocationServiceJni::NativeOnGnssStatusChanged(JNIEnv *env, jobject,
    jint satellitesNumber,
    jintArray jIds,
    jdoubleArray jCn0,
    jdoubleArray jAltitudes,
    jdoubleArray jAzimuths,
    jdoubleArray jCarrierFreqs,
    jintArray jConstellationTypes,
    jintArray jAdditionalInfo)
{
    auto list = SnapshotCallbacks<Location::GnssStatusCallbackNapi>(
        g_gnssStatusCallbacks, g_gnssStatusCallbacksMutex);
    if (list.empty()) {
        return;
    }
    for (auto &cb : list) {
        if (!cb) {
            continue;
        }
        auto status = std::make_unique<Location::SatelliteStatus>();
        status->SetSatellitesNumber((int)satellitesNumber);

        FillIntArray(env, jIds, [&](std::vector<int>& v) {
            status->SetSatelliteIds(v);
        });
        FillDoubleArray(env, jCn0, [&](std::vector<double>& v) {
            status->SetCarrierToNoiseDensitys(v);
        });
        FillDoubleArray(env, jAltitudes, [&](std::vector<double>& v) {
            status->SetAltitudes(v);
        });
        FillDoubleArray(env, jAzimuths, [&](std::vector<double>& v) {
            status->SetAzimuths(v);
        });
        FillDoubleArray(env, jCarrierFreqs, [&](std::vector<double>& v) {
            status->SetCarrierFrequencies(v);
        });
        FillIntArray(env, jConstellationTypes, [&](std::vector<int>& v) {
            status->SetConstellationTypes(v);
        });
        FillIntArray(env, jAdditionalInfo, [&](std::vector<int>& v) {
            status->SetSatelliteAdditionalInfoList(v);
        });

        cb->Send(status);
    }
}

Location::LocationErrCode LocationServiceJni::SubscribeLocationError(
    sptr<Location::LocationErrorCallbackNapi>& locationErrorCallbackHost)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::SubscribeLocationError");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LBSLOGE(LOCATION_SERVICE_JNI, "SubscribeLocationError: failed to get jni env");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto errCode = CheckLocationPermission(env);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errCode;
    }

    if (!locationErrorCallbackHost) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    bool needObserver = false;
    {
        std::lock_guard<std::mutex> lock(g_locationErrorCallbacksMutex);
        for (auto &existing : g_locationErrorCallbacks) {
            if (existing.GetRefPtr() == locationErrorCallbackHost.GetRefPtr()) {
                return Location::LocationErrCode::ERRCODE_SUCCESS;
            }
            needObserver = g_locationErrorCallbacks.empty();
            g_locationErrorCallbacks.push_back(sptr<Location::LocationErrorCallbackNapi>(locationErrorCallbackHost));
        }
    }
    if (needObserver) {
        if (!g_locationservicepluginClass.globalRef ||
            !g_locationservicepluginClass.registerLocationErrorCallback) {
            LBSLOGE(LOCATION_SERVICE_JNI, "SubscribeLocationError: globalRef or method ID is null");
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
        env->CallVoidMethod(g_locationservicepluginClass.globalRef,
            g_locationservicepluginClass.registerLocationErrorCallback);
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

Location::LocationErrCode LocationServiceJni::UnsubscribeLocationError(
    sptr<Location::LocationErrorCallbackNapi>& locationErrorCallbackHost)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::UnsubscribeLocationError");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LBSLOGE(LOCATION_SERVICE_JNI, "UnsubscribeLocationError: failed to get jni env");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto errCode = CheckLocationPermission(env);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errCode;
    }

    if (!locationErrorCallbackHost) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    bool emptyNow = false;
    {
        std::lock_guard<std::mutex> lock(g_locationErrorCallbacksMutex);
        auto& vec = g_locationErrorCallbacks;
        vec.erase(std::remove_if(vec.begin(), vec.end(),
            [&](const sptr<Location::LocationErrorCallbackNapi>& item) {
                return item && item.GetRefPtr() == locationErrorCallbackHost.GetRefPtr();
            }),
            vec.end());
        emptyNow = vec.empty();
    }
    if (emptyNow) {
        if (!g_locationservicepluginClass.globalRef ||
            !g_locationservicepluginClass.unregisterLocationErrorCallback) {
            LBSLOGE(LOCATION_SERVICE_JNI, "UnsubscribeLocationError: globalRef or method ID is null");
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
        env->CallVoidMethod(
            g_locationservicepluginClass.globalRef, g_locationservicepluginClass.unregisterLocationErrorCallback);
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

void LocationServiceJni::NativeOnLocationError(
    JNIEnv *env, jobject, jint errorCode)
{
    auto list = SnapshotCallbacks<Location::LocationErrorCallbackNapi>(
        g_locationErrorCallbacks, g_locationErrorCallbacksMutex);
    if (list.empty()) {
        return;
    }
    for (auto &cb : list) {
        if (cb) cb->OnErrorReport((int)errorCode);
    }
}

Location::LocationErrCode LocationServiceJni::SubscribeBluetoothScanResultChange(
    sptr<Location::BluetoothScanResultCallbackNapi>& callback)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::SubscribeBluetoothScanResultChange");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LBSLOGE(LOCATION_SERVICE_JNI, "SubscribeBluetoothScanResultChange: failed to get jni env");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto errCode = CheckLocationPermission(env);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errCode;
    }

    if (!callback) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    bool needObserver = false;
    {
        std::lock_guard<std::mutex> lock(g_btScanCallbacksMutex);
        for (auto &existing : g_btScanCallbacks) {
            if (existing.GetRefPtr() == callback.GetRefPtr()) {
                return Location::LocationErrCode::ERRCODE_SUCCESS;
            }
        }
        needObserver = g_btScanCallbacks.empty();
        g_btScanCallbacks.push_back(sptr<Location::BluetoothScanResultCallbackNapi>(callback));
    }
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::SubscribeBluetoothScanResultChange before call Java");
    LBSLOGI(LOCATION_SERVICE_JNI,
        "LocationServiceJni::SubscribeBluetoothScanResultChange needObserver=%d", needObserver ? 1 : 0);
    if (needObserver) {
        if (!g_locationservicepluginClass.globalRef ||
            !g_locationservicepluginClass.registerBluetoothScanResultCallback) {
            LBSLOGE(LOCATION_SERVICE_JNI, "SubscribeBluetoothScanResultChange: globalRef or method ID is null");
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
        env->CallIntMethod(g_locationservicepluginClass.globalRef,
            g_locationservicepluginClass.registerBluetoothScanResultCallback);
    }
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::SubscribeBluetoothScanResultChange after call Java");
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

Location::LocationErrCode LocationServiceJni::UnsubscribeBluetoothScanResultChange(
    sptr<Location::BluetoothScanResultCallbackNapi>& callback)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::UnsubscribeBluetoothScanResultChange");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LBSLOGE(LOCATION_SERVICE_JNI, "UnsubscribeBluetoothScanResultChange: failed to get jni env");
        return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto errCode = CheckLocationPermission(env);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errCode;
    }
    if (!callback) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    bool emptyNow = false;
    {
        std::lock_guard<std::mutex> lock(g_btScanCallbacksMutex);
        auto& vec = g_btScanCallbacks;
        vec.erase(std::remove_if(vec.begin(), vec.end(),
            [&](const sptr<Location::BluetoothScanResultCallbackNapi>& item) {
                return item && item.GetRefPtr() == callback.GetRefPtr();
            }),
            vec.end());
        emptyNow = vec.empty();
    }
    if (emptyNow) {
        if (!g_locationservicepluginClass.globalRef ||
            !g_locationservicepluginClass.unregisterBluetoothScanResultCallback) {
            LBSLOGE(LOCATION_SERVICE_JNI, "UnsubscribeBluetoothScanResultChange: globalRef or method ID is null");
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
        env->CallVoidMethod(
            g_locationservicepluginClass.globalRef,
            g_locationservicepluginClass.unregisterBluetoothScanResultCallback);
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return Location::LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE;
        }
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

void LocationServiceJni::NativeOnBluetoothScanResult(JNIEnv *env, jobject,
    jstring jDeviceId, jstring jDeviceName, jint jRssi, jbyteArray jData, jboolean jConnectable)
{
    LBSLOGI(LOCATION_SERVICE_JNI, "LocationServiceJni::NativeOnBluetoothScanResult");
    auto list = SnapshotCallbacks<Location::BluetoothScanResultCallbackNapi>(
        g_btScanCallbacks, g_btScanCallbacksMutex);
    if (list.empty()) {
        return;
    }

    std::string devId;
    std::string devName;
    if (jDeviceId) {
        const char *c = env->GetStringUTFChars(jDeviceId, nullptr);
        if (c) {
            devId = c;
            env->ReleaseStringUTFChars(jDeviceId, c);
        }
    }
    if (jDeviceName) {
        const char *c = env->GetStringUTFChars(jDeviceName, nullptr);
        if (c) {
            devName = c;
            env->ReleaseStringUTFChars(jDeviceName, c);
        }
    }
    std::vector<uint8_t> dataVec;
    if (jData) {
        jsize len = env->GetArrayLength(jData);
        if (len > 0) {
            dataVec.resize(len);
            env->GetByteArrayRegion(jData, 0, len, reinterpret_cast<jbyte*>(dataVec.data()));
        }
    }

    for (auto &cb : list) {
        if (!cb) { continue; }
        auto result = std::make_unique<Location::BluetoothScanResult>();
        result->SetDeviceId(devId);
        result->SetDeviceName(devName);
        result->SetRssi((int64_t)jRssi);
        result->SetConnectable(jConnectable == JNI_TRUE);
        result->SetData(dataVec);
        cb->OnBluetoothScanResultChange(result);
    }
}

} // namespace OHOS
