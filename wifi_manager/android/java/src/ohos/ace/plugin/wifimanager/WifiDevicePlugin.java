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
import android.text.TextUtils;
import android.util.Log;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;

public class WifiDevicePlugin implements WifiBroadcastInterface{
    private static final String WIFI_STATE_CHANGE = "wifiStateChange";

    private static final String WIFI_CONNECTION_CHANGE = "wifiConnectionChange";
    
    // 0-not active
    private static final long WIFI_SWITCH_NOT_ACTIVE = 0;

    // 1-Active
    private static final long WIFI_SWITCH_ACTIVE = 1;

    // 2-activating
    private static final long WIFI_SWITCH_ACTIVATING = 2;

    // 3-Deactivating
    private static final long WIFI_SWITCH_DEACTIVATING = 3;

    // 0-Disconnected
    private static final long WIFI_CONNECT_DISCONNECTED = 0;
 
    // 1-Connected
    private static final long WIFI_CONNECT_CONNECTED = 1;

    private Context context;

    private WifiDeviceUtils mWifiDeviceUtils;

    private WifiBroadcastReceiver mWifiReceiver;

    /**
     * WifiDevicePlugin
     *
     * @param context context of the application
     */
    public WifiDevicePlugin(Context context) {
        if (context == null) {
            Log.e(TAG, "context is null");
            return;
        }
        this.context = context;
        nativeInit();
        mWifiDeviceUtils = new WifiDeviceUtils(context);
        mWifiReceiver = new WifiBroadcastReceiver(context, this);
    }

    /**
     * WifiDevicePlugin
     *
     * @param context context of the application
     * @param isNativeInit call nativeInit or not
     */
    public WifiDevicePlugin(Context context, boolean isNativeInit) {
        if (context == null) {
            Log.e(TAG, "parameter context is null");
            return;
        }
        this.context = context;
        if (isNativeInit) {
            nativeInit();
        }
        mWifiDeviceUtils = new WifiDeviceUtils(this.context);
        mWifiReceiver = new WifiBroadcastReceiver(this.context, this);
    }

    protected native void nativeInit();

    // Call this method when an event is triggered
    protected native void nativeReceiveCallback(String key, long code);

    public String getLinkedInfo() {
        if (mWifiDeviceUtils == null) {
            mWifiDeviceUtils = new WifiDeviceUtils(context);
        }
        String wifiInfo_json = "";
        if (mWifiDeviceUtils.getWifiActive()) {
            wifiInfo_json = mWifiDeviceUtils.getWifiInfo();
        }
        return wifiInfo_json;
    }

    public boolean isWifiActive() {
        if (mWifiDeviceUtils == null) {
            mWifiDeviceUtils = new WifiDeviceUtils(context);
        }
        return mWifiDeviceUtils.getWifiActive();
    }

    public boolean isConnected() {
        boolean isConnectedWifi = false;
        if (mWifiDeviceUtils == null) {
            mWifiDeviceUtils = new WifiDeviceUtils(context);
        }
        try{
            isConnectedWifi = mWifiDeviceUtils.getIsConnected();
        } catch (Exception exception) {
            Log.e(TAG, "getIsConnected exception");
        }
        return isConnectedWifi;
    }
    
    public void on(String value) {
        try{
            if (mWifiReceiver == null) {
                  mWifiReceiver = new WifiBroadcastReceiver(context, this);
              }
            if (TextUtils.equals(value, WIFI_STATE_CHANGE)) {
                mWifiReceiver.registerSwitchReceiver();
            } else if (TextUtils.equals(value, WIFI_CONNECTION_CHANGE)) {
                mWifiReceiver.registerConnectReceiver();
            } else {
                Log.e(TAG, "on is invalid value: " + value);
            }
        } catch (Exception exception) {
            Log.e(TAG, "on exception");
        }
    }
    
    public void off(String value) {
        try{
            if (mWifiReceiver == null) {
                mWifiReceiver = new WifiBroadcastReceiver(context, this);
            }
            if (TextUtils.equals(value, WIFI_STATE_CHANGE)) {
                mWifiReceiver.unRegisterSwitchReceiver();
            } else if (TextUtils.equals(value, WIFI_CONNECTION_CHANGE)) {
                mWifiReceiver.unRegisterConnectReceiver();
            } else {
                Log.e(TAG, "off is invalid value: " + value);
            }
        } catch (Exception exception) {
            Log.e(TAG, "off exception");
        }
    }

    /**
     * Monitor changes in WiFi switch status
     */
    @Override
    public void wifiSwitchState(int state) {
        switch (state) {
            case WifiBroadcastInterface.WIFI_STATE_DISABLED:
                nativeReceiveCallback(WIFI_STATE_CHANGE, WIFI_SWITCH_NOT_ACTIVE);
                break;
            case WifiBroadcastInterface.WIFI_STATE_DISABLING:
                nativeReceiveCallback(WIFI_STATE_CHANGE, WIFI_SWITCH_DEACTIVATING);
                break;
            case WifiBroadcastInterface.WIFI_STATE_ENABLED:
                nativeReceiveCallback(WIFI_STATE_CHANGE, WIFI_SWITCH_ACTIVE);
                break;
            case WifiBroadcastInterface.WIFI_STATE_ENABLING:
                nativeReceiveCallback(WIFI_STATE_CHANGE, WIFI_SWITCH_ACTIVATING);
                break;
            default:
                Log.e(TAG, "WifiDevicePlugins wifiSwitchState invalid parameter state: " + state);
                break;
        }
    }

    /**
     * Monitor WiFi connection status
     */
    @Override
    public void wifiConnectState(int state) {
        switch (state) {
            case WifiBroadcastInterface.WIFI_STATE_DISCONNECT:
                nativeReceiveCallback(WIFI_CONNECTION_CHANGE, WIFI_CONNECT_DISCONNECTED);
                break;
            case WifiBroadcastInterface.WIFI_STATE_LINKED:
                nativeReceiveCallback(WIFI_CONNECTION_CHANGE, WIFI_CONNECT_CONNECTED);
                break;
            default:
                Log.e(TAG, "WifiDevicePlugins wifiConnectionChange invalid parameter state: " + state);
                break;
        }
    }

}