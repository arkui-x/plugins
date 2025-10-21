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
 * The LocationService class provides functions related to location services, including obtaining the current location,
 * registering location change callbacks, registering country code callbacks, registering NMEA message callbacks,
 * registering GNSS status callbacks, registering Bluetooth scan result callbacks, etc.
 *
 * @since 2025-10
 */
public class LocationService {
    private static final String LOG_TAG = "LocationService";
    private static final String ACTION_GEOFENCE = "com.example.geofence";
    private static final String FENCE_ID = "fence_id";
    private static final long TIMEOUT = 10000L;
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
    private static final int SUCCESS = 0;
    private static final int FAIL = -1;

    private volatile boolean geofenceReceiverRegistered = false;
    private volatile boolean locatingStarted = false;
    private volatile boolean pendingGnssStatusRegister = false;

    private BroadcastReceiver geofenceReceiver;
    private Context context;
    private LocationManager locationManager;
    private String currentCountryCode;
    private final Map<Integer, PendingIntent> geofencePendingIntents = new ConcurrentHashMap<>();
    private final Map<Integer, Boolean> countryCodeCallbacks = new ConcurrentHashMap<>();
    private final Map<Integer, Boolean> locationChangeCallbacks = new ConcurrentHashMap<>();
    private boolean isMonitoring = false;
    private HandlerThread locationHandlerThread;
    private BluetoothLeScanner bluetoothLeScanner;
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
    private LocationListener internalLocationListener;
    private HandlerThread locationThread;
    private android.os.Handler locationHandler;
    private int receivedFixCount = 0;
    private final Object geofenceLock = new Object();

    private Location currentLocation;

    private final CountDownLatch latch = new CountDownLatch(1);

    private Activity mActivity;

    private static class CountryResult {
        int type;
        String code;

        CountryResult(int type, String code) {
            this.type = type;
            this.code = code;
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

    public LocationService(Context context) {
        this.context = context;
        init();
    }

    /**
     * Requests and checks location permissions.
     *
     * @return Returns whether the location permission request was successful.
     */
    public boolean requestAndCheckLocationPermission() {
        Log.i(LOG_TAG, "Java requestLocationPermission called");
        Activity activity = getMainActivity();
        if (activity == null) {
            Log.e(LOG_TAG, "Java activity is null");
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
        if (this.locationManager == null) {
            this.locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        }
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
            Log.i(LOG_TAG, "latch await interrupted, no upper logic to handle", e);
        } finally {
            stopListener(listener);
        }

        return currentLocation;
    }

    private Location getLastKnownLocation() {
        Location gpsLocation = null;
        Location networkLocation = null;
        if (this.locationManager == null) {
            this.locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        }
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
        if (this.locationManager == null) {
            this.locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        }
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
        if (this.locationManager == null) {
            this.locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        }
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
     * Obtains address information based on latitude and longitude.
     *
     * @param latitude  Latitude
     * @param longitude Longitude
     * @param maxItems  Maximum number of addresses to return
     * @param locale    Language locale
     * @param country   Country
     * @param transId   Transaction ID
     * @return          Array of matching addresses
     */
    public Address[] getAddressByCoordinate(double latitude,
                                            double longitude,
                                            int maxItems,
                                            String locale,
                                            String country,
                                            String transId) {
        if (this.locationManager == null) {
            this.locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        }
        Locale loc;
        if (locale == null || locale.isEmpty()) {
            loc = Locale.getDefault();
        } else {
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
     * Registers the country code callback function.
     */
    public void registerCountryCodeCallback() {
        synchronized (countryLock) {
            handleCountryCodeRegistration();
        }
    }

    /**
     * Handles all registration logic within the synchronized block (split into this single sub-method only)
     */
    private void handleCountryCodeRegistration() {
        if (countryCodeRegistered) {
            return;
        }

        CountryResult res = resolveCountryCode();
        nativeOnCountryCodeChanged(res.code, res.type);

        if (localeReceiver == null) {
            localeReceiver = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                    CountryResult res = resolveCountryCode();
                    nativeOnCountryCodeChanged(res.code, res.type);
                }
            };
        }

        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_LOCALE_CHANGED);
        filter.addAction(TelephonyManager.ACTION_PHONE_STATE_CHANGED);
        context.registerReceiver(localeReceiver, filter);
        countryCodeRegistered = true;
    }

    /**
     * Unregisters the country code callback.
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

    private CountryResult resolveCountryCode() {
        String code = "";
        int type = 1;
        TelephonyManager telephonyManager = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
        if (telephonyManager != null) {
            String simIso = telephonyManager.getSimCountryIso();
            if (simIso != null && !simIso.isEmpty()) {
                String tempCode = simIso;
                code = tempCode;
                type = 2;
                return new CountryResult(type, code);
            } else {
                String netIso = telephonyManager.getNetworkCountryIso();
                if (netIso != null && !netIso.isEmpty()) {
                    String tempCode = netIso;
                    code = tempCode;
                    type = 4;
                    return new CountryResult(type, code);
                }
            }
        }
        String defaultCode = Locale.getDefault().getCountry();
        if (code.isEmpty()) {
            String tempCode = defaultCode;
            code = tempCode;
        }
        if (code == null) {
            String tempCode = "";
            code = tempCode;
            type = 1;
        }
        return new CountryResult(type, code);
    }

    /**
     * Register NMEA message callback
     *
     * This method is used to register a callback for NMEA messages, which will process the messages when
     * they are received.
     *
     */
    public void registerNmeaMessageCallback() {
        if (this.locationManager == null) {
            this.locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        }
        if (checkSelfPermission(android.Manifest.permission.ACCESS_FINE_LOCATION) !=
            PackageManager.PERMISSION_GRANTED) {
            return;
        }

        synchronized (nmeaLock) {
            doRegisterNmeaListener();
        }
    }

    /**
     * The core registration logic within the synchronized block, extracted as a sub-method to reduce nesting depth
     */
    private void doRegisterNmeaListener() {
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

    /**
     * Unregister NMEA message callback.
     * This method is used to unregister the callback for NMEA messages and stop receiving NMEA messages.
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
     * Register GNSS status callback.
     * This method is used to register a GNSS status listener with the LocationManager.
     */
    public void registerGnssStatusCallback() {
        Log.i(LOG_TAG, "registerGnssStatusCallback called");
        if (!checkRegisterPreconditions()) {
            return;
        }

        synchronized (gnssStatusCbLock) {
            if (gnssStatusRegistered) {
                Log.i(LOG_TAG, "GNSS status already registered");
                return;
            }
            initGnssStatusCallback();
            registerGnssCallbackWithSdkCheck();
        }
    }

    /**
     * Check the prerequisites for GNSS registration (locationManager, permissions)
     *
     * @return true: Prerequisites are met; false: Prerequisites are not met
     */
    private boolean checkRegisterPreconditions() {
        if (locationManager == null) {
            Log.e(LOG_TAG, "registerGnssStatusCallback locationManager null");
            return false;
        }

        int finePerm = checkSelfPermission(android.Manifest.permission.ACCESS_FINE_LOCATION);
        int coarsePerm = checkSelfPermission(android.Manifest.permission.ACCESS_COARSE_LOCATION);
        if (finePerm != PackageManager.PERMISSION_GRANTED || coarsePerm != PackageManager.PERMISSION_GRANTED) {
            pendingGnssStatusRegister = true;
            return false;
        }

        return true;
    }

    /**
     * Initialize the GNSS status callback (create only when the callback is null)
     */
    private void initGnssStatusCallback() {
        if (gnssStatusCallback != null) {
            return;
        }

        gnssStatusCallback = new android.location.GnssStatus.Callback() {
            @Override
            public void onSatelliteStatusChanged(android.location.GnssStatus status) {
                Log.i(LOG_TAG, "onSatelliteStatusChanged called");
                if (status == null) {
                    return;
                }

                int satelliteCount = status.getSatelliteCount();
                Log.i(LOG_TAG, "onSatelliteStatusChanged count=" + satelliteCount);
                if (satelliteCount <= 0) {
                    nativeOnGnssStatusChanged(0, new int[0], new double[0], new double[0],
                        new double[0], new double[0], new int[0], new int[0]);
                    return;
                }

                int[] ids = new int[satelliteCount];
                double[] cn0 = new double[satelliteCount];
                double[] elevations = new double[satelliteCount];
                double[] azimuths = new double[satelliteCount];
                double[] freqs = new double[satelliteCount];
                int[] constellations = new int[satelliteCount];
                int[] usedInFix = new int[satelliteCount];
                for (int i = 0; i < satelliteCount; i++) {
                    ids[i] = status.getSvid(i);
                    cn0[i] = status.getCn0DbHz(i);
                    elevations[i] = status.getElevationDegrees(i);
                    azimuths[i] = status.getAzimuthDegrees(i);
                    constellations[i] = status.getConstellationType(i);
                    usedInFix[i] = status.usedInFix(i) ? 1 : 0;
                    freqs[i] = status.hasCarrierFrequencyHz(i) ? status.getCarrierFrequencyHz(i) : -1.0;
                }

                nativeOnGnssStatusChanged(satelliteCount, ids, cn0, elevations, azimuths,
                    freqs, constellations, usedInFix);
                Log.i(LOG_TAG, "onSatelliteStatusChanged processed");
            }
        };
    }

    /**
     * Register GNSS callback by SDK version (only supports API 24 and above)
     */
    private void registerGnssCallbackWithSdkCheck() {
        if (this.locationManager == null) {
            this.locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        }
        try {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                locationManager.registerGnssStatusCallback(gnssStatusCallback,
                    new android.os.Handler(Looper.getMainLooper()));
                gnssStatusRegistered = true;
                pendingGnssStatusRegister = false;
                Log.i(LOG_TAG, "GNSS status registered success");

                forceGpsLocationUpdate();
            } else {
                Log.e(LOG_TAG, "GNSS status callback not supported on this API (<24)");
            }
        } catch (SecurityException se) {
            Log.e(LOG_TAG, "registerGnssStatusCallback security: " + se.getMessage());
        } catch (IllegalArgumentException | IllegalStateException e) {
            Log.e(LOG_TAG, "registerGnssStatusCallback ex: " + e.getMessage());
        }
    }

    /**
     * Force request GPS location updates (trigger GNSS scanning)
     */
    private void forceGpsLocationUpdate() {
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
    }

    /**
     * Unregister GNSS status callback.
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
     * Register switch callback method, call native method to update status based on location service state.
     */
    public void registerSwitchCallback() {
        int enabled = isLocationEnabled() ? 1 : 0;
        nativeOnSwitchStateChanged(enabled);
    }

    /**
     * Unregister switch callback method.
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
     * Register Bluetooth scan result callback function.
     *
     * @return Success returns 0, failure returns -1.
     */
    public int registerBluetoothScanResultCallback() {
        BluetoothManager bm = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        BluetoothAdapter adapter = (bm == null) ? null : bm.getAdapter();
        bluetoothLeScanner = (adapter == null) ? null : adapter.getBluetoothLeScanner();
        if (bluetoothLeScanner == null) {
            Log.e(LOG_TAG, "BLE scanner null");
            return FAIL;
        }

        try {
            Log.e(LOG_TAG, "Starting BLE scan");
            bluetoothLeScanner.startScan(scanCallback);
            Log.e(LOG_TAG, "BLE scan started");
            return SUCCESS;
        } catch (IllegalArgumentException | IllegalStateException e) {
            Log.e(LOG_TAG, "startScan exception: " + e.getMessage());
            return FAIL;
        } catch (SecurityException e) {
            Log.e(LOG_TAG, "startScan missing permission: " + e.getMessage());
            return FAIL;
        }
    }

    /**
     * Unregister Bluetooth scan result callback function.
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

    /**
     * Register location change callback function with specified configuration parameters.
     *
     * @param scenario            Scene type
     * @param priority            Priority
     * @param timeIntervalSec     Time interval (seconds)
     * @param distanceIntervalMeter Distance interval (meters)
     * @param maxAccuracy         Maximum accuracy
     * @param fixNumber           Fix number
     * @param timeoutMs          Timeout (milliseconds)
     * @param needPoi            Whether to need POI
     * @param needLocation       Whether to need location
     * @return                   Success returns SUCCESS, failure returns negative error code
     */
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
        if (this.locationManager == null) {
            this.locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        }

        try {
            String provider = LocationManager.NETWORK_PROVIDER;
            if (priority == 1 && locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
                provider = LocationManager.GPS_PROVIDER;
            }

            long minTimeMs = Math.max(0, timeIntervalSec) * 1000L;
            float minDistance = (float) Math.max(0.0, distanceIntervalMeter);

            locationManager.requestLocationUpdates(provider, minTimeMs, minDistance, locationListener);
            return SUCCESS;
        } catch (SecurityException se) {
            Log.e(LOG_TAG, "registerLocationChangeCallbackWithConfig security: " + se.getMessage());
            return FAIL;
        } catch (IllegalArgumentException | IllegalStateException e) {
            Log.e(LOG_TAG, "registerLocationChangeCallbackWithConfig ex: " + e.getMessage());
            return FAIL;
        }
    }

    /**
     * Register location error callback.
     * If the error listener is already registered, log and return.
     * If the location manager is null, log and return.
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

    /**
     * Unregister location callback.
     * If the error listener is not registered, return directly.
     * If the location manager is not null, try to unregister the listener.
     */
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
        if (this.locationManager == null) {
            this.locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
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
        return (checkSelfPermission(android.Manifest.permission.ACCESS_FINE_LOCATION)
                == PackageManager.PERMISSION_GRANTED)
            || (checkSelfPermission(android.Manifest.permission.ACCESS_COARSE_LOCATION)
                == PackageManager.PERMISSION_GRANTED);
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

    /**
     * Start positioning
     *
     * @return Returns SUCCESS for success, other values for failure
     */
    public int startLocating() {
        Log.i(LOG_TAG, "Java startLocating called");
        if (locatingStarted) {
            return SUCCESS;
        }
        if (this.locationManager == null) {
            this.locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
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
            return SUCCESS;
        } catch (SecurityException | IllegalArgumentException | IllegalStateException e) {
            Log.e(LOG_TAG, "startLocating error: " + e.getMessage());
            return FAIL;
        }
    }

    /**
     * Stop positioning.
     *
     * @return Returns SUCCESS for success, other values for failure.
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
        if (this.locationManager == null) {
            this.locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        }
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
     * Obtain the switch status.
     *
     * @return Returns the integer value of the switch state.
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
     * Check if the geographic conversion service is available.
     *
     * @return Returns true if the conversion service is available; otherwise, returns false.
     */
    public boolean isGeoConvertAvailable() {
        Log.i(LOG_TAG, "isGeoConvertAvailable called");
        return Geocoder.isPresent();
    }

    /**
     * Obtain address information based on location name.
     *
     * @param description Location description
     * @param maxItems    Maximum number of addresses to return
     * @param localeStr   Language locale string
     * @param country     Country
     * @param minLat      Minimum latitude
     * @param minLon      Minimum longitude
     * @param maxLat      Maximum latitude
     * @param maxLon      Maximum longitude
     * @param transId     Transaction ID
     * @return            Array of matching addresses
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
     * Get the ISO country code.
     *
     * @return The string representation of the ISO country code.
     */
    public String getIsoCountryCode() {
        Log.i(LOG_TAG, "Get the ISO country code");
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
     * Add a GNSS geofence.
     *
     * @param latitude  Latitude of the geofence center.
     * @param longitude Longitude of the geofence center.
     * @param radius    Radius of the geofence (in meters).
     * @param expiration Expiration time of the geofence (in milliseconds).
     * @param fenceId   Unique identifier of the geofence.
     */
    public void addGnssGeofence(double latitude, double longitude, float radius, long expiration, int fenceId) {
        Log.i(LOG_TAG, "addGnssGeofence called with fenceId: " + fenceId);
        try {
            LocationManager locationManagerInstance =
                (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
            if (locationManagerInstance == null) {
                Log.e(LOG_TAG, "Get locationManager failed");
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
     * Remove the GNSS geofence with the specified ID.
     *
     * @param fenceId The ID of the geofence to be removed.
     */
    public void removeGnssGeofence(int fenceId) {
        Log.i(LOG_TAG, "removeGnssGeofence called with fenceId: " + fenceId);
        try {
            LocationManager locationManagerService =
                (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
            if (locationManagerService == null) {
                Log.e(LOG_TAG, "Failed to obtain the LocationManager instance");
                return;
            }

            Intent geofenceIntent = createGeofenceIntent(fenceId);
            PendingIntent geofencePendingIntent = createPendingIntent(fenceId, geofenceIntent);

            locationManagerService.removeProximityAlert(geofencePendingIntent);
            Log.i(LOG_TAG, "Geofence has been removed: fenceId = " + fenceId);
        } catch (SecurityException e) {
            Log.e(LOG_TAG, "Failed to remove geofence: Permission error", e);
        } catch (Exception e) {
            Log.e(LOG_TAG, "An unexpected error occurred while removing the geofence", e);
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
     * Get the WiFi BSSID currently used for positioning.
     *
     * @return The current WiFi BSSID; returns "Denied" if the acquisition fails.
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

    private Activity getMainActivity() {
        try {
            Class<?> activityThreadClass = Class.forName("android.app.ActivityThread");
            Object activityThread = activityThreadClass.getMethod("currentActivityThread").invoke(null);

            Field mActivitiesField = activityThreadClass.getDeclaredField("mActivities");
            mActivitiesField.setAccessible(true);
            Object mActivitiesObj = mActivitiesField.get(activityThread);

            if (!(mActivitiesObj instanceof Map)) {
                return null;
            }
            Map<?, ?> activitiesMap = (Map<?, ?>) mActivitiesObj;

            for (Object activityClientRecord : activitiesMap.values()) {
                Activity activity = getNonPausedActivity(activityClientRecord);
                if (activity != null) {
                    return activity;
                }
            }
        } catch (ClassNotFoundException ex) {
            Log.e("TAG", "ClassNotFoundException occurred: " + ex.getMessage());
        } catch (InvocationTargetException ex) {
            Log.e("TAG", "InvocationTargetException occurred: " + ex.getMessage());
        } catch (NoSuchMethodException ex) {
            Log.e("TAG", "NoSuchMethodException occurred: " + ex.getMessage());
        } catch (NoSuchFieldException ex) {
            Log.e("TAG", "NoSuchFieldException occurred: " + ex.getMessage());
        } catch (IllegalAccessException ex) {
            Log.e("TAG", "IllegalAccessException occurred: " + ex.getMessage());
        }
        return null;
    }

    /**
     * Extract the Activity in non-paused state from ActivityClientRecord
     *
     * @param activityClientRecord The ActivityClientRecord object
     * @return The Activity object in non-paused state; returns null if none exists
     * @throws NoSuchFieldException If the field does not exist
     * @throws IllegalAccessException If the field cannot be accessed
     */
    private Activity getNonPausedActivity(Object activityClientRecord)
            throws NoSuchFieldException, IllegalAccessException {
        Class<?> recordClass = activityClientRecord.getClass();
        Field pausedField = recordClass.getDeclaredField("paused");
        pausedField.setAccessible(true);
        if (pausedField.getBoolean(activityClientRecord)) {
            return null;
        }

        Field activityField = recordClass.getDeclaredField("activity");
        activityField.setAccessible(true);
        Object activityObj = activityField.get(activityClientRecord);

        return (activityObj instanceof Activity) ? (Activity) activityObj : null;
    }

    /**
     * Initialize native methods.
     * native void nativeInit();
     */
    protected native void nativeInit();

    private void init() {
        nativeInit();
    }
}
