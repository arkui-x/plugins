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

#ifndef LOCATION_SERVICE_H
#define LOCATION_SERVICE_H

#include <jni.h>
#include <sys/stat.h>

#include "bluetooth_scan_result_callback_napi.h"
#include "country_code_callback_napi.h"
#include "country_code.h"
#include "geofence_request.h"
#include "geo_convert_callback_host.h"
#include "gnss_status_callback_napi.h"
#include "ilocator_callback.h"
#include "iremote_object.h"
#include "location.h"
#include "location_error_callback_napi.h"
#include "locator_callback_napi.h"
#include "location_switch_callback_napi.h"
#include "nmea_message_callback_napi.h"
#include "request_config.h"

namespace OHOS {
namespace Plugin {
class LocationServiceJni final {
public:
    LocationServiceJni() = delete;
    ~LocationServiceJni() = delete;
    static bool Register(void* env);
    static void NativeInit(JNIEnv* env, jobject jobj);

    static Location::LocationErrCode GetSwitchState(int32_t& state);
    static Location::LocationErrCode StartLocating(
        std::unique_ptr<Location::RequestConfig>& requestConfig,
        sptr<Location::LocatorCallbackNapi>& locatorCallbackHost);
    static Location::LocationErrCode StopLocating(sptr<Location::LocatorCallbackNapi>& locatorCallbackHost);
    static Location::LocationErrCode IsGeoConvertAvailable(bool& isAvailable);
    static Location::LocationErrCode GetAddressByCoordinate(std::unique_ptr<Location::ReverseGeoCodeRequest>& data,
        std::list<std::shared_ptr<Location::GeoAddress>>& replyList);
    static std::shared_ptr<Location::CountryCode> GetIsoCountryCode();
    static Location::LocationErrCode AddGnssGeofence(Location::GeofenceRequest& request,
        sptr<Location::LocationGnssGeofenceCallbackNapi>& callback);
    static Location::LocationErrCode RemoveGnssGeofence(int32_t fenceId,
        sptr<Location::LocationGnssGeofenceCallbackNapi>& callback);
    static Location::LocationErrCode GetCurrentWifiBssidForLocating(std::string& bssid);
    static Location::LocationErrCode RegisterSwitchCallback(
        sptr<Location::LocationSwitchCallbackNapi>& switchCallbackHost);
    static Location::LocationErrCode UnregisterSwitchCallback(
        sptr<Location::LocationSwitchCallbackNapi>& switchCallbackHost);
    static Location::LocationErrCode RegisterCountryCodeCallback(
        sptr<Location::CountryCodeCallbackNapi>& callbackHost);
    static Location::LocationErrCode UnregisterCountryCodeCallback(
        sptr<Location::CountryCodeCallbackNapi>& callbackHost);
    static Location::LocationErrCode RegisterGnssStatusCallback(sptr<Location::GnssStatusCallbackNapi>& cb);
    static Location::LocationErrCode UnregisterGnssStatusCallback(sptr<Location::GnssStatusCallbackNapi>& callback);
    static Location::LocationErrCode RegisterNmeaMessageCallback(sptr<Location::NmeaMessageCallbackNapi>& cb);
    static Location::LocationErrCode UnregisterNmeaMessageCallback(sptr<Location::NmeaMessageCallbackNapi>& cb);
    static Location::LocationErrCode SubscribeLocationError(
        sptr<Location::LocationErrorCallbackNapi>& locationErrorCallbackHost);
    static Location::LocationErrCode SubscribeBluetoothScanResultChange(
        sptr<Location::BluetoothScanResultCallbackNapi>& callback);
    static Location::LocationErrCode UnsubscribeBluetoothScanResultChange(
        sptr<Location::BluetoothScanResultCallbackNapi>& callback);
    static Location::LocationErrCode UnsubscribeLocationError(
        sptr<Location::LocationErrorCallbackNapi>& locationErrorCallbackHost);
    static Location::LocationErrCode GetAddressByLocationName(std::unique_ptr<Location::GeoCodeRequest>& request,
        std::list<std::shared_ptr<Location::GeoAddress>>& replyList);

    //called by java
    static void NativeOnSwitchStateChanged(JNIEnv *env, jobject, jint enabled);
    static void NativeOnBluetoothScanResult(JNIEnv *env, jobject,
        jstring jDeviceId, jstring jDeviceName, jint jRssi, jbyteArray jData, jboolean jConnectable);
    static void NativeOnLocationError(JNIEnv *env, jobject, jint errorCode);
    static void NativeOnLocationChanged(JNIEnv *env, jobject,
        jdouble latitude, jdouble longitude, jfloat accuracy,
        jdouble altitude, jfloat speed, jfloat bearing,
        jlong timeMs, jfloat uncertaintyOfTimeSinceBoot, jstring sourceType);
    static void NativeOnGnssStatusChanged(JNIEnv *env, jobject,
        jint satellitesNumber,
        jintArray jIds,
        jdoubleArray jCn0,
        jdoubleArray jAltitudes,
        jdoubleArray jAzimuths,
        jdoubleArray jCarrierFreqs,
        jintArray jConstellationTypes,
        jintArray jAdditionalInfo);
    static void NativeOnNmeaMessage(JNIEnv *env, jobject, jlong timestamp, jstring nmea);
    static void NativeOnCountryCodeChanged(JNIEnv *env, jobject, jstring code, jint type);
    static void OnGetAddressByLocationNameResult(JNIEnv* env, jobject /*jobj*/,
        jlong cbKey,
        jobjectArray addressLines,
        jdoubleArray latitudes,
        jdoubleArray longitudes,
        jobjectArray countryCodes,
        jobjectArray countryNames,
        jobjectArray adminAreas,
        jobjectArray localities,
        jobjectArray subLocalities,
        jobjectArray thoroughfares,
        jobjectArray postalCodes);
    static void NativeOnGeofenceEvent(JNIEnv *env, jobject, jint fenceId);
    static void NativeOnNotificationEvent(JNIEnv *env, jobject, jint fenceId, jboolean entering);
private:
    static bool hasInit_;
};

} // namespace Plugin
} // namespace OHOS

#endif // LOCATOR_JNI_H
