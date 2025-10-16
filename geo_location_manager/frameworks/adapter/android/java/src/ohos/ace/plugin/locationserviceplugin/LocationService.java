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

package ohos.ace.plugin.locationserviceplugin;

import android.util.Log;
import android.os.Build;
import android.content.Intent;
import android.app.PendingIntent;
import android.net.wifi.WifiInfo;
import android.provider.Settings;
import android.net.wifi.WifiManager;
import android.telephony.TelephonyManager;
import android.content.Context;
import android.location.Address;
import android.location.Geocoder;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.Manifest;
import android.os.HandlerThread;
import android.os.Looper;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import android.os.Handler;
import android.location.GnssStatus;
import android.location.OnNmeaMessageListener;
import android.app.Activity;
import android.content.pm.PackageManager;

import java.util.concurrent.CountDownLatch;
import java.io.IOException;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.ArrayList;
import java.util.concurrent.ConcurrentHashMap;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Field;

/**
 * LocationService 类提供了与位置服务相关的功能，包括获取当前位置、注册位置变化回调、
 * 注册国家代码回调、注册NMEA消息回调、注册GNSS状态回调、注册蓝牙扫描结果回调等。
 */
public class LocationService {
    private static final String LOG_TAG = "LocationService";
    private static final String ACTION_GEOFENCE = "com.example.geofence";
    private static final String FENCE_ID = "fence_id";
    private final Map<Integer, PendingIntent> geofencePendingIntents = new ConcurrentHashMap<>();
    private volatile boolean geofenceReceiverRegistered = false;
    private BroadcastReceiver geofenceReceiver;
    private Context context;
    private LocationManager locationManager;
    private String currentCountryCode;
    private final Map<Integer, Boolean> countryCodeCallbacks = new ConcurrentHashMap<>();
    private final Map<Integer, Boolean> locationChangeCallbacks = new ConcurrentHashMap<>();
    private boolean isMonitoring = false;
    private HandlerThread locationHandlerThread;
    private final BluetoothLeScanner bluetoothLeScanner;
    private boolean errorListenerRegistered = false;
    private boolean gnssStatusRegistered = false;
    private final Object gnssStatusCbLock = new Object();
    private android.location.GnssStatus.Callback gnssStatusCallback;
    private boolean nmeaRegistered = false;
    private final Object nmeaLock = new Object();
    private android.location.OnNmeaMessageListener nmeaListener;
    private boolean countryCodeRegistered = false;
    private final Object countryLock = new Object();
    private BroadcastReceiver localeReceiver;
    private volatile boolean locatingStarted = false;
    private LocationListener internalLocationListener;
    private HandlerThread locationThread;
    private android.os.Handler locationHandler;
    private int receivedFixCount = 0;
    private final Object geofenceLock = new Object();
    private Location currentLocation;
    private final CountDownLatch latch = new CountDownLatch(1);

    private static final long TIMEOUT = 10000L;
    private volatile boolean pendingGnssStatusRegister = false;
    private Activity mActivity;

    private static final int REQUEST_BLUETOOTH_RELATED_PERM = 102;

    private static final int REQUEST_LOCATION_PERMISSION = 1001;
    private static final int ERR_PROVIDER_OUT_OF_SERVICE = 801;
    private static final int ERR_PROVIDER_TEMP_UNAVAILABLE = 2002;
    private static final int ERR_PROVIDER_DISABLED = 2003;
    private static final int ERR_NO_PERMISSION = 201;
    private static final int ERR_DISABLED = -4;
    private static final int PRIORITY_ACCURACY_HIGH = 0;
    private static final int PRIORITY_ACCURACY_BALANCED = 1;
    private static final int PRIORITY_ACCURACY_LOW = 2;
    private static final int FLAG_MUTABLE = 1 << 25;
    private static final int VERSION_CODES_S = 31;

    public LocationService(Context context) {
        this.context = context;
        this.locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        BluetoothManager bm = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        BluetoothAdapter adapter = (bm == null) ? null : bm.getAdapter();
        bluetoothLeScanner = (adapter == null) ? null : adapter.getBluetoothLeScanner();
        nativeInit();
    }

    /**
     * 请求并检查位置权限。
     * @return 返回是否成功请求位置权限。
     */
    public boolean requestAndCheckLocationPermission() {
        Log.i(LOG_TAG, "Java requestLocationPermission called");
        Activity activity = getMainActivity();
        if (activity == null) {
            Log.e(LOG_TAG, "当前 Java 活动为空");
            return false;
        }

        if (activity.checkSelfPermission(android.Manifest.permission.ACCESS_FINE_LOCATION) ==
                PackageManager.PERMISSION_GRANTED &&
            activity.checkSelfPermission(android.Manifest.permission.ACCESS_COARSE_LOCATION) ==
                PackageManager.PERMISSION_GRANTED &&
            activity.checkSelfPermission(android.Manifest.permission.ACCESS_WIFI_STATE) ==
                PackageManager.PERMISSION_GRANTED) {
            Log.i(LOG_TAG, "All required permissions are granted");
            return true;
        }

        Log.i(LOG_TAG, "Java requestPermissions start");
        activity.requestPermissions(
            new String[]{
                android.Manifest.permission.ACCESS_FINE_LOCATION,
                android.Manifest.permission.ACCESS_COARSE_LOCATION,
                android.Manifest.permission.ACCESS_WIFI_STATE
            },
            REQUEST_LOCATION_PERMISSION
        );
        Log.i(LOG_TAG, "Java requestPermissions finish");
        return false;
    }

    private native void nativeOnBluetoothScanResult(String deviceId,
                                                           String deviceName,
                                                           int rssi,
                                                           byte[] data,
                                                           boolean connectable);
                                    
    private native void nativeOnSwitchStateChanged(int enabled);

    private native void nativeOnGnssStatusChanged(int satNum,
                                                     int[] ids,
                                                     double[] cn0,
                                                     double[] altitudes,
                                                     double[] azimuths,
                                                     double[] carrierFreqs,
                                                     int[] constellationTypes,
                                                     int[] additionalInfo);

    private native void nativeOnNmeaMessage(long timestamp, String sentence);

    private native void nativeOnCountryCodeChanged(String code, int type);

    private native void nativeOnGeofenceEvent(int fenceId);

    private static native void nativeOnNotificationEvent(int fenceId, boolean entering);

    public Location getCurrentLocation() {
        Location lastKnownLocation = getLastKnownLocation();
        if (lastKnownLocation != null) {
            return lastKnownLocation;
        }

        LocationListener listener = new LocationListener() {
            @Override
            public void onLocationChanged(Location location) {
                currentLocation = location;
                latch.countDown();
                stopListener();
            }

            @Override
            public void onStatusChanged(String provider, int status, Bundle extras) {}

            @Override
            public void onProviderEnabled(String provider) {}

            @Override
            public void onProviderDisabled(String provider) {}
        };
        registerListener(listener);

        try {
            latch.await(TIMEOUT, java.util.concurrent.TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        } finally {
            stopListener(listener);
        }

        return currentLocation;
    }

    private Location getLastKnownLocation() {
        Location gpsLocation = null;
        Location networkLocation = null;

        if (locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
            gpsLocation = locationManager.getLastKnownLocation(LocationManager.GPS_PROVIDER);
        }
        if (locationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER)) {
            networkLocation = locationManager.getLastKnownLocation(LocationManager.NETWORK_PROVIDER);
        }

        if (gpsLocation != null && networkLocation != null) {
            return gpsLocation.getTime() > networkLocation.getTime() ? gpsLocation : networkLocation;
        } else {
            return gpsLocation != null ? gpsLocation : networkLocation;
        }
    }

    private void registerListener(LocationListener listener) {
        if (locationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER)) {
            locationManager.requestLocationUpdates(
                    LocationManager.NETWORK_PROVIDER,
                    0, 0,
                    listener);
        }
        if (locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
            locationManager.requestLocationUpdates(
                    LocationManager.GPS_PROVIDER,
                    0, 0,
                    listener);
        }
    }

    private void stopListener(LocationListener listener) {
        if (listener != null) {
            locationManager.removeUpdates(listener);
        }
    }

    private void stopListener() {
        if (locationManager != null && locationListener != null) {
            try {
                locationManager.removeUpdates(locationListener);
            } catch (SecurityException e) {
                Log.e("LocationManager", "SecurityException occurred while removing updates", e);
            }
        }
    }

    /**
     * 根据经纬度地址信息。
     * 
     * @param latitude  纬度
     * @param longitude 经度
     *                  maxItems 返回的最大地址数量
     * @param locale    语言环境
     * @param country   国家
     * @param transId   事务ID
     *                  return 匹配的地址数组
     */
    public Address[] getAddressByCoordinate(double latitude,
                                            double longitude,
                                            int maxItems,
                                            String locale,
                                            String country,
                                            String transId) {
        if (maxItems <= 0) {
            maxItems = 1;
        }
        if (maxItems > 10) {
            maxItems = 10;
        }
        Locale loc;
        if (locale == null || locale.isEmpty()) {
            loc = Locale.getDefault();
        } else {
            // 支持 "zh_CN" / "en-US"
            String lang = locale;
            String region = "";
            if (locale.contains("_")) {
                String[] parts = locale.split("_", 2);
                lang = parts[0];
                region = parts[1];
            } else if (locale.contains("-")) {
                String[] parts = locale.split("-", 2);
                lang = parts[0];
                region = parts[1];
            }
            if (!region.isEmpty()) {
                loc = new Locale(lang, region);
            } else {
                loc = new Locale(lang);
            }
        }
        ArrayList<Address> out = new ArrayList<>();
        try {
            Geocoder geocoder = new Geocoder(context, loc);
            List<Address> list = geocoder.getFromLocation(latitude, longitude, maxItems);
            if (list != null) {
                out.addAll(list);
            }
        } catch (IOException e) {
            Log.e(LOG_TAG, "getAddressByCoordinate geocoder IO error: " + e.getMessage());
        } catch (IllegalArgumentException iae) {
            Log.e(LOG_TAG, "getAddressByCoordinate invalid lat/lon: " + iae.getMessage());
        } catch (Exception e) {
            Log.e(LOG_TAG, "getAddressByCoordinate unexpected: " + e.getMessage());
        }
        return out.toArray(new Address[0]);
    }

    /**
     * 注册国家代码回调函数。
     */
    public void registerCountryCodeCallback() {
        synchronized (countryLock) {
            if (countryCodeRegistered) {
                return;
            }
            final String[] codeHolder = {""};
            final int[] typeHolder = {1};
            typeHolder[0] = resolveCountryCode(codeHolder[0]);
            nativeOnCountryCodeChanged(codeHolder[0], typeHolder[0]);
            if (localeReceiver == null) {
                localeReceiver = new BroadcastReceiver() {
                    @Override
                    public void onReceive(android.content.Context context, android.content.Intent intent) {
                        typeHolder[0] = resolveCountryCode(codeHolder[0]);
                        nativeOnCountryCodeChanged(codeHolder[0], typeHolder[0]);
                    }
                };
            }
            IntentFilter filter = new IntentFilter();
            filter.addAction(Intent.ACTION_LOCALE_CHANGED);
            filter.addAction(TelephonyManager.ACTION_PHONE_STATE_CHANGED);
            context.registerReceiver(localeReceiver, filter);
            countryCodeRegistered = true;
        }
    }

    /**
     * 取消注册国家代码回调。
     */
    public void unregisterCountryCodeCallback() {
        synchronized (countryLock) {
            if (!countryCodeRegistered) {
                return;
            }
            try {
                if (localeReceiver != null) {
                    context.unregisterReceiver(localeReceiver);
                }
            } catch (IllegalArgumentException | SecurityException e) {
                Log.e(LOG_TAG, "unregisterCountryCodeCallback exception: " + e.getMessage());
            }
            countryCodeRegistered = false;
        }
    }

    private int resolveCountryCode(String code) {
        int type = 1;
        TelephonyManager telephonyManager  = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
        if (telephonyManager  != null) {
            String simIso = telephonyManager .getSimCountryIso();
            if (simIso != null && !simIso.isEmpty()) {
                String simTempCode = simIso;
                code = simTempCode;
                type = 2;
            } else {
                String netIso = telephonyManager .getNetworkCountryIso();
                if (netIso != null && !netIso.isEmpty()) {
                    String netTempCode = netIso;
                    code = netTempCode;
                    type = 4;
                }
            }
        }
        String defaultCode = Locale.getDefault().getCountry();
        if (code.isEmpty()) {
            code = defaultCode;
        }
        if (code == null) {
            code = "";
            type = 1;
        }
        return type;
    }

    /**
     * 注册NMEA消息回调
     * 
     * 该方法用于注册NMEA消息的回调，在接收到NMEA消息时进行处理。
     * 
     */
    public void registerNmeaMessageCallback() {
        if (locationManager == null) {
            return;
        }
        if (checkSelfPermission(android.Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            return;
        }
        synchronized (nmeaLock) {
            if (nmeaRegistered) {
                return;
            }
            if (nmeaListener == null) {
                nmeaListener = (message, ts) -> {
                    if (message != null) {
                        nativeOnNmeaMessage(ts, message);
                    }
                };
            }
            boolean ok = locationManager.addNmeaListener(nmeaListener);
            if (ok) {
                nmeaRegistered = true;
            }
        }
    }

    /**
     * 注销NMEA消息回调。
     * 该方法用于取消注册NMEA消息的回调，停止接收NMEA消息。
     */
    public void unregisterNmeaMessageCallback() {
        if (locationManager == null) {
            return;
        }
        synchronized (nmeaLock) {
            if (!nmeaRegistered || nmeaListener == null) {
                return;
            }
            locationManager.removeNmeaListener(nmeaListener);
            nmeaRegistered = false;
        }
    }

    /**
     * 
     * 注册GNSS状态回调。
     * 此方法用于向LocationManager注册GNSS状态监听器。
     */
    public void registerGnssStatusCallback() {
        Log.i(LOG_TAG, "registerGnssStatusCallback called");
        if (locationManager == null) {
            Log.e(LOG_TAG, "registerGnssStatusCallback locationManager null");
            return;
        }
        if (checkSelfPermission(android.Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            Log.e(LOG_TAG, "registerGnssStatusCallback no fine location permission");
            return;
        }
        int fine = checkSelfPermission(android.Manifest.permission.ACCESS_FINE_LOCATION);
        int coarse = checkSelfPermission(android.Manifest.permission.ACCESS_COARSE_LOCATION);
        if (fine != PackageManager.PERMISSION_GRANTED) {
            Log.e(LOG_TAG,
                "registerGnssStatusCallback no fine location permission (fine=" + fine + ", coarse=" + coarse + ")");
            pendingGnssStatusRegister = true;
            return;
        }
        synchronized (gnssStatusCbLock) {
            if (gnssStatusRegistered) {
                Log.i(LOG_TAG, "GNSS status already registered");
                return;
            }
            if (gnssStatusCallback == null) {
                gnssStatusCallback = new android.location.GnssStatus.Callback() {
                    @Override
                    public void onSatelliteStatusChanged(android.location.GnssStatus status) {
                        Log.i(LOG_TAG, "onSatelliteStatusChanged called");
                        if (status == null) {
                            return;
                        }
                        int count = status.getSatelliteCount();
                        Log.i(LOG_TAG, "onSatelliteStatusChanged count=" + count);
                        if (count <= 0) {
                            nativeOnGnssStatusChanged(0,
                                    new int[0], new double[0], new double[0],
                                    new double[0], new double[0], new int[0], new int[0]);
                            return;
                        }
                        int[] ids = new int[count];
                        double[] cn0 = new double[count];
                        double[] elevationsAsAltitudes = new double[count];
                        double[] azimuths = new double[count];
                        double[] freqs = new double[count];
                        int[] constellationTypes = new int[count];
                        int[] additionalInfo = new int[count];
                        for (int i = 0; i < count; i++) {
                            ids[i] = status.getSvid(i);
                            cn0[i] = status.getCn0DbHz(i);
                            elevationsAsAltitudes[i] = status.getElevationDegrees(i); // 以仰角代填 altitude
                            azimuths[i] = status.getAzimuthDegrees(i);
                            constellationTypes[i] = status.getConstellationType(i);
                            additionalInfo[i] = status.usedInFix(i) ? 1 : 0;
                            freqs[i] = status.hasCarrierFrequencyHz(i) ? status.getCarrierFrequencyHz(i) : -1.0;
                        }
                        nativeOnGnssStatusChanged(count,
                                ids, cn0, elevationsAsAltitudes, azimuths, freqs,
                                constellationTypes, additionalInfo);
                        Log.i(LOG_TAG, "onSatelliteStatusChanged processed");
                    }
                };
            }
            try {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                    locationManager.registerGnssStatusCallback(gnssStatusCallback,
                        new android.os.Handler(Looper.getMainLooper()));
                    gnssStatusRegistered = true;
                    pendingGnssStatusRegister = false;
                    Log.i(LOG_TAG, "GNSS status registered success");

                    try {
                        if (locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
                            locationManager.requestLocationUpdates(
                                LocationManager.GPS_PROVIDER,
                                1000L,
                                0f,
                                locationListener);
                            Log.i(LOG_TAG, "Force GPS request to trigger GNSS scan");
                        } else {
                            Log.i(LOG_TAG, "GPS provider disabled, skip force request");
                        }
                    } catch (SecurityException e) {
                        Log.e(LOG_TAG, "Force GPS request failed: " + e.getMessage());
                    }
                } else {
                    Log.e(LOG_TAG, "GNSS status callback not supported on this API (<24)");
                }
            } catch (SecurityException se) {
                Log.e(LOG_TAG, "registerGnssStatusCallback security: " + se.getMessage());
            } catch (IllegalArgumentException | IllegalStateException e) {
                Log.e(LOG_TAG, "registerGnssStatusCallback ex: " + e.getMessage());
            }
        }
    }

    /**
     * 注销GNSS状态回调。
     */
    public void unregisterGnssStatusCallback() {
        Log.i(LOG_TAG, "unregisterGnssStatusCallback called");
        if (locationManager == null) {
            return;
        }
        synchronized (gnssStatusCbLock) {
            if (!gnssStatusRegistered) {
                return;
            }
            try {
                if (gnssStatusCallback != null) {
                    locationManager.unregisterGnssStatusCallback(gnssStatusCallback);
                }
            } catch (IllegalArgumentException | SecurityException e) {
                Log.e(LOG_TAG, "unregisterGnssStatusCallback exception: " + e.getMessage());
            }
            gnssStatusRegistered = false;
        }
    }

    /**
     * 注册开关回调方法，根据位置服务的开启状态调用原生方法更新状态。
     */
    public void registerSwitchCallback() {
    int enabled = isLocationEnabled() ? 1 : 0;
    nativeOnSwitchStateChanged(enabled);
    }

    /**
     * 取消注册开关回调方法。
     */
    public void unregisterSwitchCallback() {
    }

    private boolean isLocationEnabled() {
    android.location.LocationManager lm =
        (android.location.LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
    if (lm == null) {
        return false;
    }
    return lm.isProviderEnabled(android.location.LocationManager.GPS_PROVIDER)
        || lm.isProviderEnabled(android.location.LocationManager.NETWORK_PROVIDER);
    }
    
    private static native void nativeOnLocationChanged(
        double latitude, double longitude, float accuracy,
        double altitude, float speed, float bearing, long timeMs, float uncertaintyOfTimeSinceBoot, String sourceType);

    private static native void nativeOnLocationError(int errorCode);
    
    /**
     * 注册蓝牙扫描结果回调函数。
     *
     * @return 成功返回0，失败返回-1。
     */
    public int registerBluetoothScanResultCallback() {
        if (bluetoothLeScanner == null) {
            Log.e(LOG_TAG, "BLE scanner null");
            return -1;
        }

        if (!checkBluetoothScanPermissions()) {
            Log.e(LOG_TAG, "Missing required permissions for BLE scan");
            requestBluetoothScanPermissions();
            return -1;
        }

        try {
            Log.e(LOG_TAG, "Starting BLE scan");
            bluetoothLeScanner.startScan(scanCallback);
            Log.e(LOG_TAG, "BLE scan started");
            return 0;
        } catch (IllegalArgumentException | IllegalStateException e) {
            Log.e(LOG_TAG, "startScan exception: " + e.getMessage());
            return -1;
        }
    }

    /**
     * 注销蓝牙扫描结果回调函数。
     */
    public void unregisterBluetoothScanResultCallback() {
        if (bluetoothLeScanner == null) {
            return;
        }
        try {
            bluetoothLeScanner.stopScan(scanCallback);
        } catch (IllegalArgumentException | IllegalStateException e) {
            Log.e(LOG_TAG, "stopScan exception: " + e.getMessage());
        }
    }

    private boolean checkBluetoothScanPermissions() {
        if (mActivity == null) {
            Log.e(LOG_TAG, "Activity is null, cannot check permissions");
            return false;
        }

        if (Build.VERSION.SDK_INT >= 31) { // Android 12+
            if (mActivity.checkSelfPermission("Manifest.permission.BLUETOOTH_SCAN") !=
                    PackageManager.PERMISSION_GRANTED) {
                Log.e(LOG_TAG, "BLUETOOTH_SCAN permission not granted");
                return false;
            }
        } else {
            if (mActivity.checkSelfPermission(android.Manifest.permission.ACCESS_FINE_LOCATION) !=
                    PackageManager.PERMISSION_GRANTED) {
                Log.e(LOG_TAG, "ACCESS_FINE_LOCATION permission not granted");
                return false;
            }
        }

        return true;
    }

    private void requestBluetoothScanPermissions() {
        if (mActivity == null) {
            Log.e(LOG_TAG, "Activity is null, cannot request permissions");
            return;
        }

        if (checkBluetoothScanPermissions()) {
            Log.i(LOG_TAG, "Permissions already granted, skip request");
            return;
        }

        String[] permissionsToRequest;
        if (Build.VERSION.SDK_INT >= 31) { // Android 12+
            permissionsToRequest = new String[]{"android.permission.BLUETOOTH_SCAN"};
        } else {
            permissionsToRequest = new String[]{android.Manifest.permission.ACCESS_FINE_LOCATION};
        }

        mActivity.requestPermissions(permissionsToRequest, REQUEST_BLUETOOTH_RELATED_PERM);
    }

    private final ScanCallback scanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            Log.i(LOG_TAG, "onScanResult called");
            if (result == null) {
                return;
            }
            String deviceId = (result.getDevice() != null) ? result.getDevice().getAddress() : "";
            String deviceName = (result.getDevice() != null && result.getDevice().getName() != null)
                ? result.getDevice().getName() : "";
            int rssi = result.getRssi();
            byte[] adv = (result.getScanRecord() != null && result.getScanRecord().getBytes() != null)
                ? result.getScanRecord().getBytes() : new byte[0];
            boolean connectable = true;
            nativeOnBluetoothScanResult(deviceId, deviceName, rssi, adv, connectable);
            Log.i(LOG_TAG, "onScanResult processed");
        }
    };

    private final LocationListener locationListener = new LocationListener() {
        @Override
        public void onLocationChanged(Location location) {
        }

        @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {
        }

        @Override
        public void onProviderEnabled(String provider) {
        }

        @Override
        public void onProviderDisabled(String provider) {
        }
    };

    public int registerLocationChangeCallbackWithConfig(
            int scenario,
            int priority,
            int timeIntervalSec,
            double distanceIntervalMeter,
            float maxAccuracy,
            int fixNumber,
            int timeoutMs,
            boolean needPoi,
            boolean needLocation) {
        if (locationManager == null) {
            return -1;
        }
        if (checkSelfPermission(android.Manifest.permission.ACCESS_FINE_LOCATION)
                != PackageManager.PERMISSION_GRANTED
            || checkSelfPermission(android.Manifest.permission.ACCESS_COARSE_LOCATION)
                != PackageManager.PERMISSION_GRANTED) {
            Log.e(LOG_TAG, "No location permission");
            return -2;
        }

        try {
            String provider = LocationManager.NETWORK_PROVIDER;
            if (priority == 1 && locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
                provider = LocationManager.GPS_PROVIDER;
            }

            long minTimeMs = Math.max(0, timeIntervalSec) * 1000L;
            float minDistance = (float) Math.max(0.0, distanceIntervalMeter);

            locationManager.requestLocationUpdates(provider, minTimeMs, minDistance, locationListener);
            return 0;
        } catch (SecurityException se) {
            Log.e(LOG_TAG, "registerLocationChangeCallbackWithConfig security: " + se.getMessage());
            return -3;
        } catch (IllegalArgumentException | IllegalStateException e) {
            Log.e(LOG_TAG, "registerLocationChangeCallbackWithConfig ex: " + e.getMessage());
            return -4;
        }
    }

    /**
     * 注册位置错误回调。
     * 如果错误监听器已经注册，则记录日志并返回。
     * 如果位置管理器为空记录日志并返回。
     */
    public synchronized void registerLocationErrorCallback() {
        if (errorListenerRegistered) {
            Log.e(LOG_TAG, "Location error listener already registered");
            return;
        }
        if (locationManager == null) {
            nativeOnLocationError(ERR_PROVIDER_DISABLED);
            return;
        }

        if (!hasLocationPermission()) {
            nativeOnLocationError(ERR_NO_PERMISSION);
            return;
        }
        try {
            String provider = chooseProvider();
            if (provider == null) {
                nativeOnLocationError(ERR_PROVIDER_DISABLED);
                return;
            }
            locationManager.requestLocationUpdates(provider, 0L, 0f, statusListener);
            errorListenerRegistered = true;
            Log.i(LOG_TAG, "registerLocationErrorCallback success, provider=" + provider);
        } catch (SecurityException se) {
            nativeOnLocationError(ERR_NO_PERMISSION);
        } catch (Exception e) {
            nativeOnLocationError(ERR_DISABLED);
        }
    }

    public synchronized void unregisterLocationErrorCallback() {
        if (!errorListenerRegistered) {
            return;
        }
        if (locationManager != null) {
            try {
                locationManager.removeUpdates(statusListener);
            } catch (IllegalArgumentException | SecurityException e) {
                Log.e(LOG_TAG, "removeUpdates exception: " + e.getMessage());
            }
        }
        errorListenerRegistered = false;
    }

    private String chooseProvider() {
        if (locationManager == null) {
            return null;
        }
        try {
            if (locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
                return LocationManager.GPS_PROVIDER;
            }
            if (locationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER)) {
                return LocationManager.NETWORK_PROVIDER;
            }
        } catch (IllegalArgumentException | SecurityException e) {
            Log.e(LOG_TAG, "chooseProvider exception: " + e.getMessage());
        }
        return null;
    }

    private boolean hasLocationPermission() {
        return (checkSelfPermission(android.Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED)
            || (checkSelfPermission(android.Manifest.permission.ACCESS_COARSE_LOCATION) == PackageManager.PERMISSION_GRANTED);
    }

    private int checkSelfPermission(String perm) {
        if (context == null) {
            Log.e(LOG_TAG, "checkSelfPermission: context null, treat as denied");
            return PackageManager.PERMISSION_DENIED;
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            return context.checkSelfPermission(perm);
        } else {
            return PackageManager.PERMISSION_GRANTED;
        }
    }

    private final LocationListener statusListener = new LocationListener() {
        @Override
        public void onLocationChanged(Location loc) {
            if (loc == null) {
                return;
            }
            nativeOnLocationChanged(
                loc.getLatitude(),
                loc.getLongitude(),
                loc.hasAccuracy() ? loc.getAccuracy() : 0f,
                loc.hasAltitude() ? loc.getAltitude() : 0.0,
                loc.hasSpeed() ? loc.getSpeed() : 0f,
                loc.hasBearing() ? loc.getBearing() : 0f,
                loc.getTime(),
                loc.getBearingAccuracyDegrees(),
                loc.getProvider());
        }

        @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {
            if (status == android.location.LocationProvider.OUT_OF_SERVICE) {
                nativeOnLocationError(ERR_PROVIDER_OUT_OF_SERVICE);
            } else if (status == android.location.LocationProvider.TEMPORARILY_UNAVAILABLE) {
                nativeOnLocationError(ERR_PROVIDER_TEMP_UNAVAILABLE);
            }
        }

        @Override
        public void onProviderEnabled(String provider) {
        }

        @Override
        public void onProviderDisabled(String provider) {
            nativeOnLocationError(ERR_PROVIDER_DISABLED);
        }
    };

    /**
     * 开始定位
     * @return 返回0表示成功，其他值表示失败
     */
    public int startLocating() {
        Log.i(LOG_TAG, "Java startLocating called");
        if (locatingStarted) {
            return 0;
        }
        if (locationManager == null) {
            Log.e(LOG_TAG, "startLocating: locationManager null");
            return -1;
        }
        if (checkSelfPermission(android.Manifest.permission.ACCESS_FINE_LOCATION) !=
                PackageManager.PERMISSION_GRANTED) {
            Log.e(LOG_TAG, "startLocating: no fine location permission");
            return -1;
        }
        ensureLocationThread();
        createListenerIfNeed();
        try {
            long minTimeMs = 5000L;
            float minDistanceM = 10f;
            String provider = chooseProvider();
            locationManager.requestLocationUpdates(provider, minTimeMs, minDistanceM,
                    internalLocationListener, locationHandler.getLooper());
            locatingStarted = true;
            return 0;
        } catch (SecurityException | IllegalArgumentException | IllegalStateException e) {
            Log.e(LOG_TAG, "startLocating error: " + e.getMessage());
            return -2;
        }
    }

    /**
     * 停止定位功能。
     * @return 返回0表示成功，非0表示失败。
     */
    public int stopLocating() {
        if (!locatingStarted) {
            return 0;
        }
        try {
            if (locationManager != null && internalLocationListener != null) {
                locationManager.removeUpdates(internalLocationListener);
            }
        } catch (SecurityException | IllegalArgumentException e) {
            Log.e(LOG_TAG, "stopLocating removeUpdates warn");
        }
        locatingStarted = false;
        receivedFixCount = 0;
        return 0;
    }

    private String mapPriorityToProvider(int priority) {
        switch (priority) {
            case PRIORITY_ACCURACY_LOW:
                if (locationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER)) {
                    return LocationManager.NETWORK_PROVIDER;
                }
                break;
            case PRIORITY_ACCURACY_BALANCED:
                if (locationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER)) {
                    return LocationManager.NETWORK_PROVIDER;
                }
                if (locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
                    return LocationManager.GPS_PROVIDER;
                }
                break;
            case PRIORITY_ACCURACY_HIGH:
            default:
                if (locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
                    return LocationManager.GPS_PROVIDER;
                }
                if (locationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER)) {
                    return LocationManager.NETWORK_PROVIDER;
                }
        }
        return LocationManager.PASSIVE_PROVIDER;
    }

    private void ensureLocationThread() {
        if (locationThread == null) {
            locationThread = new HandlerThread("LocThread");
            locationThread.start();
            locationHandler = new android.os.Handler(locationThread.getLooper());
        }
    }

    private void createListenerIfNeed() {
        if (internalLocationListener != null) {
            return;
        }
        internalLocationListener = new LocationListener() {
            @Override
            public void onLocationChanged(Location loc) {
                if (loc == null) {
                    return;
                }
                nativeOnLocationChanged(
                    loc.getLatitude(),
                    loc.getLongitude(),
                    loc.hasAccuracy() ? loc.getAccuracy() : 0f,
                    loc.hasAltitude() ? loc.getAltitude() : 0.0,
                    loc.hasSpeed() ? loc.getSpeed() : 0f,
                    loc.hasBearing() ? loc.getBearing() : 0f,
                    loc.getTime(),
                    loc.getBearingAccuracyDegrees(),
                    loc.getProvider());
                if (receivedFixCount >= 0) {
                    receivedFixCount++;
                }
            }
            @Override
            public void onStatusChanged(String provider, int status, Bundle extras) {}
            @Override
            public void onProviderEnabled(String provider) {}
            @Override
            public void onProviderDisabled(String provider) {}
        };
    }

    /**
    * 获取开关状态。
    * 
    * @return 开关状态的整数值。
    */
    public int getSwitchState() {
        Log.i(LOG_TAG, "getSwitchState called");
        try {
            int mode = Settings.Secure.getInt(
                context.getContentResolver(),
                Settings.Secure.LOCATION_MODE
            );
            if (mode == Settings.Secure.LOCATION_MODE_OFF) {
                return 0;
            } else {
                return 1;
            }
            } catch (Settings.SettingNotFoundException e) {
                return 0;
        }
    }

    /**
    * 检查地理转换服务是否可用。
    * @return 如果转换服务可用，返回true；否则返回false。
    */
    public boolean isGeoConvertAvailable() {
        Log.i(LOG_TAG, "isGeoConvertAvailable called");
        return Geocoder.isPresent();
    }
    
    /**
     * 根据位置名称获取地址信息。
     * 
     * @param desc       位置描述
     * @param maxItems   返回的最大地址数量
     * @param localeStr  语言环境
     * @param country    国家
     * @param minLat     最小纬度
     * @param minLon     最小经度
     * @param maxLat     最大纬度
     * @param maxLon     最大经度
     * @param transId    事务ID
     * @return           匹配的地址数组
     */
    public android.location.Address[] getAddressByLocationName(String description,
                                                           int maxItems,
                                                           String localeStr,
                                                           String country,
                                                           double minLat,
                                                           double minLon,
                                                           double maxLat,
                                                           double maxLon,
                                                           String transId) {
        if (maxItems <= 0) {
            maxItems = 1;
        }
        if (maxItems > 10) {
            maxItems = 10;
        }
        java.util.Locale loc;
        if (localeStr == null || localeStr.isEmpty()) {
            loc = java.util.Locale.getDefault();
        } else {
            String lang = localeStr;
            String region = "";
            if (localeStr.contains("_")) {
                String[] p = localeStr.split("_", 2);
                lang = p[0];
                region = p[1];
            } else if (localeStr.contains("-")) {
                String[] p = localeStr.split("-", 2);
                lang = p[0];
                region = p[1];
            }
            loc = region.isEmpty() ? new java.util.Locale(lang) : new java.util.Locale(lang, region);
        }
        java.util.ArrayList<android.location.Address> addresses = new java.util.ArrayList<>();
        try {
            android.location.Geocoder geocoder = new android.location.Geocoder(context, loc);
            java.util.List<android.location.Address> addressList =
                geocoder.getFromLocationName(description, maxItems, minLat, minLon, maxLat, maxLon);
            if (addressList != null) {
                addresses.addAll(addressList);
            }
        } catch (IOException e) {
            Log.e(LOG_TAG, "getAddressByLocationName error: " + e.getMessage());
        }
        return addresses.toArray(new android.location.Address[0]);
    }

    /**
     * 获取ISO国家代码。
     * 
     * @return ISO国家代码的字符串表示。
     */
    public String getIsoCountryCode() {
        Log.i(LOG_TAG, "获取ISO国家代码");
        TelephonyManager tm = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
        String simCountry = null;
        if (tm != null) {
            simCountry = tm.getSimCountryIso();
            if (simCountry != null && !simCountry.isEmpty()) {
                return simCountry.toUpperCase((Locale.ROOT));
            }
        }
        return Locale.getDefault().getCountry().toUpperCase((Locale.ROOT));
    }

    /**
     * 添加一个GNSS地理围栏。
     * @param latitude  围栏中心的纬度。
     * @param longitude 围栏中心的经度。
     * @param radius    围栏的半径（以米为单位）。
     * @param expiration 围栏的过期时间（以毫秒为单位）。
     * @param fenceId   围栏的唯一标识符。
     */
    public void addGnssGeofence(double latitude, double longitude, float radius, long expiration, int fenceId) {
        Log.i(LOG_TAG, "addGnssGeofence called with fenceId: " + fenceId);
        try {
            LocationManager locationManagerInstance = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
            if (locationManagerInstance == null) {
                Log.e(LOG_TAG, "无法获取LocationManager实例");
                return;
            }
            ensureGeofenceReceiverRegistered();

            PendingIntent oldPi = geofencePendingIntents.remove(fenceId);
            if (oldPi != null) {
                try {
                    locationManagerInstance.removeProximityAlert(oldPi);
                } catch (SecurityException | IllegalArgumentException e) {
                    Log.e(LOG_TAG, "remove old proximity alert fail fenceId=" + fenceId);
                }
            }

            Intent geofenceIntent = createGeofenceIntent(fenceId);
            PendingIntent geofencePendingIntent = createPendingIntent(fenceId, geofenceIntent);
            locationManagerInstance.addProximityAlert(latitude, longitude, radius, expiration, geofencePendingIntent);
            geofencePendingIntents.put(fenceId, geofencePendingIntent);
            Log.i(LOG_TAG, "Geofence added successfully: fenceId " + fenceId);
            nativeOnGeofenceEvent(fenceId);
        } catch (SecurityException e) {
            Log.e(LOG_TAG, "Geofence addition failed: permission error", e);
        } catch (Exception e) {
            Log.e(LOG_TAG, "Geofence addition encountered an unexpected error", e);
        }
    }

    private void ensureGeofenceReceiverRegistered() {
        if (geofenceReceiverRegistered) {
            return;
        }
        geofenceReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                if (intent == null || !ACTION_GEOFENCE.equals(intent.getAction())) {
                    return;
                }
                int fenceId = intent.getIntExtra(FENCE_ID, -1);
                if (fenceId <= 0) {
                    Log.e(LOG_TAG, "GeofenceReceiver invalid fenceId");
                    return;
                }
                boolean entering = intent.getBooleanExtra(
                        LocationManager.KEY_PROXIMITY_ENTERING, false);
                Log.i(LOG_TAG, "Geofence event fenceId=" + fenceId + " entering=" + entering);
                nativeOnNotificationEvent(fenceId, entering);
            }
        };
        IntentFilter filter = new IntentFilter(ACTION_GEOFENCE);
        context.registerReceiver(geofenceReceiver, filter);
        geofenceReceiverRegistered = true;
        Log.i(LOG_TAG, "Geofence receiver registered");
    }
    
    /**
     * 移除指定ID的GNSS地理围栏。
     * @param fenceId 要移除的地理栏的ID。
     */
    public void removeGnssGeofence(int fenceId) {
        Log.i(LOG_TAG, "removeGnssGeofence called with fenceId: " + fenceId);
        try {
            LocationManager locationManagerService = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
            if (locationManagerService == null) {
                Log.e(LOG_TAG, "无法获取LocationManager实例");
                return;
            }

            Intent geofenceIntent = createGeofenceIntent(fenceId);
            PendingIntent geofencePendingIntent = createPendingIntent(fenceId, geofenceIntent);

            locationManagerService.removeProximityAlert(geofencePendingIntent);
            Log.i(LOG_TAG, "地理围栏已移除: fenceId = " + fenceId);
        } catch (SecurityException e) {
            Log.e(LOG_TAG, "移除地理围栏失败：权限错误", e);
        } catch (Exception e) {
            Log.e(LOG_TAG, "移除地理围栏时发生意外错误", e);
        }
    }

    private Intent createGeofenceIntent(int fenceId) {
        Intent geofenceIntent = new Intent(ACTION_GEOFENCE);
        geofenceIntent.setPackage(context.getApplicationInfo().packageName);
        geofenceIntent.putExtra(FENCE_ID, fenceId);
        return geofenceIntent;
    }

    private PendingIntent createPendingIntent(int fenceId, Intent broadcastIntent) {
        int flags = PendingIntent.FLAG_UPDATE_CURRENT;
        if (Build.VERSION.SDK_INT >= VERSION_CODES_S) {
            flags |= FLAG_MUTABLE;
        }

        return PendingIntent.getBroadcast(
            context,
            fenceId,
            broadcastIntent,
            flags
        );
    }

    /**
     * 获取当前用于定位的WiFi BSSID。
     * @return 当前WiFi BSSID，如果获取失败则返回 " Denied"。
     */
    public String getCurrentWifiBssidForLocating() {
        Log.i(LOG_TAG, "getCurrentWifiBssidForLocating called");

        if (!checkRequiredPermissions()) {
            Log.e(LOG_TAG, "Missing necessary permissions, unable to obtain WiFi BSSID.");
            return "Permission Denied";
        }

        WifiManager wifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        if (wifiManager == null) {
            Log.e(LOG_TAG, "WiFi Manager is null, cannot obtain WiFi BSSID.");
            return "WiFi Not Supported";
        }

        try {
            if (!wifiManager.isWifiEnabled()) {
                Log.e(LOG_TAG, "WiFi is disabled.");
                return "WiFi Disabled";
            }

            WifiInfo wifiInfo = wifiManager.getConnectionInfo();
            if (wifiInfo == null) {
                Log.e(LOG_TAG, "Failed to get WiFi connection info: WifiInfo is null.");
                return "No WiFi Connection";
            }

            String bssid = wifiInfo.getBSSID();
            if (isInvalidBssid(bssid)) {
                Log.e(LOG_TAG, "Invalid or placeholder BSSID received: " + bssid);
                return "Invalid BSSID";
            }

            Log.i(LOG_TAG, "Successfully obtained WiFi BSSID: " + bssid);
            return bssid;

        } catch (SecurityException e) {
            Log.e(LOG_TAG, "Failed to get WiFi BSSID: Security exception - missing permissions.", e);
            return "Security Exception";
        } catch (Exception e) {
            Log.e(LOG_TAG, "Failed to get WiFi BSSID: Unexpected exception.", e);
            return "Unknown Error";
        }
    }

    private boolean checkRequiredPermissions() {
        boolean hasLocationPerm = context.checkSelfPermission(
                android.Manifest.permission.ACCESS_FINE_LOCATION
        ) == PackageManager.PERMISSION_GRANTED;

        boolean hasWifiPerm = context.checkSelfPermission(
                android.Manifest.permission.ACCESS_WIFI_STATE
        ) == PackageManager.PERMISSION_GRANTED;

        if (!hasLocationPerm) {
            Log.e(LOG_TAG, "Missing required permission: ACCESS_FINE_LOCATION (Fine Location Permission)");
        }
        if (!hasWifiPerm) {
            Log.e(LOG_TAG, "Missing required permission: ACCESS_WIFI_STATE (WiFi State Permission)");
        }

        return hasLocationPerm && hasWifiPerm;
    }

    private boolean isInvalidBssid(String bssid) {
        return bssid == null
                || bssid.trim().isEmpty()
                || "02:00:00:00:00:00".equals(bssid);
    }

    /**
     * 注册开关回调
     @param callback 回调对象
     * @return 返回结果
     */
    public int registerSwitchCallback(Object callback) {
        Log.i(LOG_TAG, "registerSwitchCallback called");
        return 0;
    }

    /**
     * 注册开关回调
     @param callback 回调对象
     * @return 返回结果
     */
    public int unregisterSwitchCallback(Object callback) {
        Log.i(LOG_TAG, "unregisterSwitchCallback called");
        return 0;
    }

    private Activity getMainActivity() {
        try {
            Class activityThreadClass = Class.forName("android.app.ActivityThread");
            Object activityThread = activityThreadClass.getMethod("currentActivityThread").invoke(null);
            Field mActivities = activityThreadClass.getDeclaredField("mActivities");
            mActivities.setAccessible(true);
            Object mActivitiesObj = mActivities.get(activityThread);
            if (mActivitiesObj instanceof Map) {
                Map activitiesMap = (Map) mActivitiesObj;
                for (Object activityClientRecord : activitiesMap.values()) {
                    Class activityClientRecordClass = activityClientRecord.getClass();
                    Field paused = activityClientRecordClass.getDeclaredField("paused");
                    paused.setAccessible(true);
                    if (!paused.getBoolean(activityClientRecord)) {
                        Field activityField = activityClientRecordClass.getDeclaredField("activity");
                        activityField.setAccessible(true);
                        Object activityObj = activityField.get(activityClientRecord);
                        if (activityObj instanceof android.app.Activity) {
                            Activity activityInstance = (Activity) activityObj;
                            return activityInstance;
                        }
                    }
                }
            }
        } catch (ClassNotFoundException ex) {
            Log.e("TAG", "ClassNotFoundException occurred.");
        } catch (InvocationTargetException ex) {
            Log.e("TAG", "InvocationTargetException occurred.");
        } catch (NoSuchMethodException ex) {
            Log.e("TAG", "NoSuchMethodException occurred.");
        } catch (NoSuchFieldException ex) {
            Log.e("TAG", "Exception occurred.");
        } catch (IllegalAccessException ex) {
            Log.e("TAG", "IllegalAccessException occurred.");
        }
        return null;
    }

    /**
     * 初始化本地方法。
     * native void nativeInit();
     */
    protected native void nativeInit();
}
