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
        Log.i(TAG, "registerConnectReceiver");
    }

    /**
     * Unregistration of WiFi link status for listening to broadcasts
     */
    public void unRegisterConnectReceiver() {
        if (mWifiConnectReceiver == null) {
            Log.e(TAG, "mWifiConnectReceiver is null");
        }
        context.unregisterReceiver(mWifiConnectReceiver);
        Log.i(TAG, "unRegisterConnectReceiver");
    }

    /**
     * Listening and broadcasting of WiFi link status
     */
    class WifiConnectReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            // The identification (action/intention) of the currently received broadcast
            String action = intent.getAction();
            if (!action.equals(ConnectivityManager.CONNECTIVITY_ACTION)) {
                Log.e(TAG, "WifiConnectReceiver action is not CONNECTIVITY_ACTION");
                return;
            }
            if (mWifiInterface == null) {
                Log.e(TAG, "WifiConnectReceiver mWifiInterface is null");
                return;
            }
            ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
            NetworkInfo info = connectivityManager.getActiveNetworkInfo();
            if (info != null && info.isAvailable()) {
                String name = info.getTypeName();
                if (name.equals(WifiDeviceUtils.NETWORKINFO_TYPE_NAME)) {
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
        Log.i(TAG, "registerSwitchReceiver");
    }

    /**
     * Unregistration broadcast of WiFi switch status
     */
    public void unRegisterSwitchReceiver() {
        if (mWifiSwitchReceiver == null) {
            Log.e(TAG, "mWifiSwitchReceiver is null");
        }
        context.unregisterReceiver(mWifiSwitchReceiver);
        Log.i(TAG, "unRegisterSwitchReceiver");
    }

    /**
     * Monitoring and broadcasting of WiFi switch status
     */
    class WifiSwitchBroadcastReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            if (mWifiInterface == null) {
                Log.e(TAG, "WifiSwitchBroadcastReceiver mWifiInterface is null");
                return;
            }
            int wifiState = intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE, 0);
            Log.e(TAG, "WifiSwitchBroadcastReceiver wifiState: " + wifiState);
            mWifiInterface.wifiSwitchState(wifiState);
        }
    }
}
