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

import android.content.Context;
import android.text.TextUtils;
import android.util.Log;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;

public class WifiDevicePlugin implements WifiBroadcastInterface{
    private static final String LOG_TAG = "WifiDevicePlugin";
 
    private static final String WIFI_STATE_CHANGE = "wifiStateChange";

    private static final String WIFI_CONNECTION_CHANGE = "wifiConnectionChange";

    private Context context;

    private WifiDeviceUtils mWifiDeviceUtils;

    private WifiBroadcastReceiver mWifiReceiver;

    /**
     * WifiDevicePlugin
     *
     * @param context context of the application
     */
    public WifiDevicePlugin(Context context) {
        if (context != null) {
            this.context = context;
        } else {
            Log.e(LOG_TAG, "context is null");
        }
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
        if (context != null) {
            this.context = context;
        } else {
            Log.e(LOG_TAG, "parameter context is null");
        }
        if (isNativeInit) {
            nativeInit();
        }
        mWifiDeviceUtils = new WifiDeviceUtils(this.context);
        mWifiReceiver = new WifiBroadcastReceiver(this.context, this);
    }

    protected native void nativeInit();

    // 当触发事件时调用此方法
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
            Log.e(LOG_TAG, "isConnected exception");
        }
        return isConnectedWifi;
    }
    
    public void on(String value) {
        try{
            if (TextUtils.equals(value, WIFI_STATE_CHANGE)) { // 查询WLAN是否已使能
              if (mWifiReceiver == null) {
                    mWifiReceiver = new WifiBroadcastReceiver(context, this);
                }
                mWifiReceiver.registerSwitchReceiver();
            } else if (TextUtils.equals(value, WIFI_CONNECTION_CHANGE)) { // 查询WLAN是否已连接
                if (mWifiReceiver == null) {
                    mWifiReceiver = new WifiBroadcastReceiver(context, this);
                }
                mWifiReceiver.registerConnectReceiver();
            } else {
                Log.e(LOG_TAG, "on is invalid");
            }
        } catch (Exception exception) {
            Log.e(LOG_TAG, "on exception");
        }
    }
    
    public void off(String value) {
        try{
            if (TextUtils.equals(value, WIFI_STATE_CHANGE)) {
                if (mWifiReceiver == null) {
                    mWifiReceiver = new WifiBroadcastReceiver(context, this);
                }
                mWifiReceiver.unRegisterSwitchReceiver();
            } else if (TextUtils.equals(value, WIFI_CONNECTION_CHANGE)) {
                if (mWifiReceiver == null) {
                    mWifiReceiver = new WifiBroadcastReceiver(context, this);
                }
                mWifiReceiver.unRegisterConnectReceiver();
            } else {
                Log.e(LOG_TAG, "off is invalid");
            }
        } catch (Exception exception) {
            Log.e(LOG_TAG, "off exception");
        }
    }

    /**
     * 监听wifi开关状态变化
     *
     */
    @Override
    public void wifiSwitchState(int state) {
        switch (state) {
            case WifiBroadcastInterface.WIFI_STATE_DISABLED:
                // 0-未激活
                nativeReceiveCallback(WIFI_STATE_CHANGE, 0);
                break;
            case WifiBroadcastInterface.WIFI_STATE_DISABLING:
                // 3-去激活中
                nativeReceiveCallback(WIFI_STATE_CHANGE, 3);
                break;
            case WifiBroadcastInterface.WIFI_STATE_ENABLED:
                // 1-已激活
                nativeReceiveCallback(WIFI_STATE_CHANGE, 1);
                break;
            case WifiBroadcastInterface.WIFI_STATE_ENABLING:
                // 2-激活中
                nativeReceiveCallback(WIFI_STATE_CHANGE, 2);
                break;
            default:
                Log.e(LOG_TAG, "WifiDevicePlugins wifiSwitchState invalid parameter");
                break;
        }
    }

    /**
     * 监听wifi连接状态
     *
     */
    @Override
    public void wifiConnectState(int state) {
        switch (state) {
            case WifiBroadcastInterface.WIFI_STATE_DISCONNECT:
                // 0-已断开
                nativeReceiveCallback(WIFI_CONNECTION_CHANGE, 0);
                break;
            case WifiBroadcastInterface.WIFI_STATE_LINK:
                // 1-已连接
                nativeReceiveCallback(WIFI_CONNECTION_CHANGE, 1);
                break;
            default:
                Log.e(LOG_TAG, "WifiDevicePlugins wifiConnectionChange invalid parameter");
                break;
        }
    }

}