/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

package ohos.ace.plugin.wifimanager;

import static ohos.ace.plugin.wifimanager.WifiBroadcastInterface.TAG;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.text.TextUtils;
import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;

/**
 * WifiDeviceUtils providing wifi device information.
 *
 * @since 2024-06-24
 */
public class WifiDeviceUtils {
    /**
     * Network type name
     */
    public static final String NETWORKINFO_TYPE_NAME = "WIFI";

    private Context context;

    private WifiManager mWifiManager;

    /**
     * Constructor for WifiDeviceUtils
     *
     * @param context Context
     */
    public WifiDeviceUtils(Context context) {
        if (context == null) {
            Log.e(TAG, "WifiDeviceUtils context is null");
            return;
        }
        this.context = context;
        mWifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
    }

    /**
     * Obtain information on the WIFI link
     *
     * @return WifiInfo
     */
    public String getWifiInfo() {
        if (mWifiManager == null) {
            Log.e(TAG, "WifiDeviceUtils getWifiInfo mWifiManager is null");
            return "";
        }
        WifiInfo wifiInfo = mWifiManager.getConnectionInfo();
        if (wifiInfo == null) {
            Log.e(TAG, "WifiDeviceUtils getWifiInfo wifiInfo is null");
            return "";
        }
        String ssid = wifiInfo.getSSID();
        if (!TextUtils.isEmpty(ssid) && ssid.contains("\"")) {
            ssid = ssid.replace("\"", "");
        }
        String bssid = wifiInfo.getBSSID();
        int networkId = wifiInfo.getNetworkId();
        int rssi = wifiInfo.getRssi();
        int linkSpeed = wifiInfo.getLinkSpeed();
        int frequency = wifiInfo.getFrequency();
        boolean isHidden = wifiInfo.getHiddenSSID();
        JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put("ssid", ssid == null ? "" : ssid);
            jsonObject.put("bssid", bssid == null ? "" : bssid);
            jsonObject.put("networkId", networkId);
            jsonObject.put("rssi", rssi);
            jsonObject.put("linkSpeed", linkSpeed);
            jsonObject.put("frequency", frequency);
            jsonObject.put("isHidden", isHidden);
            return jsonObject.toString();
        } catch (JSONException exception) {
            Log.e(TAG, "WifiDeviceUtils getWifiInfo jsonObject.put JSONException");
        }
        return "";
    }

    /**
     * Check if the WiFi switch is turned on
     *
     * @return true: open, false: close
     */
    public boolean getWifiActive() {
        if (mWifiManager == null) {
            Log.e(TAG, "WifiDeviceUtils mWifiManager is null");
            return false;
        }
        return mWifiManager.getWifiState() == WifiManager.WIFI_STATE_ENABLED;
    }

    /**
     * Get WIFI link status
     *
     * @return true: connected, false: disconnected
     */
    public boolean getIsConnected() {
        if (context == null) {
            Log.e(TAG, "WifiDeviceUtils getIsConnected context is null");
            return false;
        }

        // Get Network Connection Manager
        ConnectivityManager connectivityManager =
            (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        if (connectivityManager == null) {
            Log.e(TAG, "WifiDeviceUtils getIsConnected connectivityManager is null");
            return false;
        }

        // Obtain current network status information
        NetworkInfo info = connectivityManager.getActiveNetworkInfo();
        if (info != null && info.isAvailable()) {
            return NETWORKINFO_TYPE_NAME.equals(info.getTypeName());
        }
        return false;
    }
}
