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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.util.Log;
import ohos.ace.plugin.wifimanager.WifiBroadcastReceiver.WifiConnectReceiver;

/**
 * Monitor the broadcast of the on/off status and network connection status of WiFi
 *
 * @since 2024-06-24
 */
public class WifiBroadcastReceiver {
    private Context context;

    private WifiConnectReceiver mWifiConnectReceiver;

    private WifiSwitchBroadcastReceiver mWifiSwitchReceiver;

    private WifiBroadcastInterface mWifiInterface;

    public WifiBroadcastReceiver(Context context, WifiBroadcastInterface mWifiInterface) {
        this.context = context;
        this.mWifiInterface = mWifiInterface;
    }


    /**
     * WiFi link status registration listening broadcast
     */
    public void registerConnectReceiver() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
        mWifiConnectReceiver = new WifiConnectReceiver();
        context.registerReceiver(mWifiConnectReceiver, filter);
    }

    /**
     * Unregistration of WiFi link status for listening to broadcasts
     */
    public void unRegisterConnectReceiver() {
        if (mWifiConnectReceiver == null) {
            Log.e(TAG, "mWifiConnectReceiver is null");
        }
        context.unregisterReceiver(mWifiConnectReceiver);
    }

    /**
     * Listening and broadcasting of WiFi link status
     */
    class WifiConnectReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent safeIntent) {
            // The identification (action/intention) of the currently received broadcast
            if (mWifiInterface == null || context == null || safeIntent == null) {
                Log.e(TAG, "WifiConnectReceiver onReceive parameter is null");
                return;
            }
            String action = safeIntent.getAction();
            if (!ConnectivityManager.CONNECTIVITY_ACTION.equals(action)) {
                Log.e(TAG, "WifiConnectReceiver action is not CONNECTIVITY_ACTION");
                return;
            }
            ConnectivityManager connectivityManager =
                (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
            if (connectivityManager == null) {
                Log.e(TAG, "WifiConnectReceiver connectivityManager is null");
                return;
            }
            NetworkInfo info = connectivityManager.getActiveNetworkInfo();
            if (info != null && info.isAvailable()) {
                String typeName = info.getTypeName();
                if (WifiDeviceUtils.NETWORKINFO_TYPE_NAME.equals(typeName)) {
                    mWifiInterface.wifiConnectState(WifiBroadcastInterface.WIFI_STATE_LINKED);
                } else {
                    mWifiInterface.wifiConnectState(WifiBroadcastInterface.WIFI_STATE_DISCONNECT);
                }
            } else {
                mWifiInterface.wifiConnectState(WifiBroadcastInterface.WIFI_STATE_DISCONNECT);
            }
        }
    }

    /**
     * Registration broadcast of WiFi switch status
     */
    public void registerSwitchReceiver() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
        mWifiSwitchReceiver = new WifiSwitchBroadcastReceiver();
        context.registerReceiver(mWifiSwitchReceiver, filter);
    }

    /**
     * Unregistration broadcast of WiFi switch status
     */
    public void unRegisterSwitchReceiver() {
        if (mWifiSwitchReceiver == null) {
            Log.e(TAG, "mWifiSwitchReceiver is null");
        }
        context.unregisterReceiver(mWifiSwitchReceiver);
    }

    /**
     * Monitoring and broadcasting of WiFi switch status
     */
    class WifiSwitchBroadcastReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent safeIntent) {
            if (mWifiInterface == null || context == null || safeIntent == null) {
                Log.e(TAG, "WifiSwitchBroadcastReceiver onReceive parameter is null");
                return;
            }
            mWifiInterface.wifiSwitchState(safeIntent.getIntExtra(WifiManager.EXTRA_WIFI_STATE, 0));
        }
    }
}
