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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.util.Log;

/**
 * 监听wifi的开关状态与网络连接状态的广播
 */

public class WifiBroadcastReceiver {
    private static final String LOG_TAG = "WifiBroadcastReceiver";

    private Context mContext;

    private WifiConnectReceiver mWifiConnectReceiver;

    private WifiSwitchBroadcastReceiver mWifiSwitchReceiver;

    private WifiBroadcastInterface mWifiInterface;

    public WifiBroadcastReceiver(Context mContext, WifiBroadcastInterface mWifiInterface) {
        this.mContext = mContext;
        this.mWifiInterface = mWifiInterface;
    }


    /**
     * wifi链接状态注册监听广播
     */
    public void registerConnectReceiver() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
        mWifiConnectReceiver = new WifiConnectReceiver();
        mContext.registerReceiver(mWifiConnectReceiver, filter);
        Log.i(LOG_TAG, "registerConnectReceiver");
    }

    /**
     * wifi链接状态取消注册监听广播
     */
    public void unRegisterConnectReceiver() {
        if (mWifiConnectReceiver != null) {
            mContext.unregisterReceiver(mWifiConnectReceiver);
            Log.i(LOG_TAG, "unRegisterConnectReceiver");
        }
    }

    /**
     * wifi链接状态的监听广播
     */
    class WifiConnectReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            // 当前接受到的广播的标识(行动/意图)
            String action = intent.getAction();
            if (!action.equals(ConnectivityManager.CONNECTIVITY_ACTION)) {
                return;
            }
            ConnectivityManager connectivityManager = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
            NetworkInfo info = connectivityManager.getActiveNetworkInfo();
            if (info != null && info.isAvailable()) {
                String name = info.getTypeName();
                if (mWifiInterface == null) {
                    return;
                }
                if (name.equals("WIFI")) {
                    mWifiInterface.wifiConnectState(WifiBroadcastInterface.WIFI_STATE_LINK);
                } else {
                    mWifiInterface.wifiConnectState(WifiBroadcastInterface.WIFI_STATE_DISCONNECT);
                }
            } else {
                if (mWifiInterface != null) {
                    mWifiInterface.wifiConnectState(WifiBroadcastInterface.WIFI_STATE_DISCONNECT);
                }
            }
        }
    }

    /**
     * wifi开关状态的注册广播
     */
    public void registerSwitchReceiver() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
        mWifiSwitchReceiver = new WifiSwitchBroadcastReceiver();
        mContext.registerReceiver(mWifiSwitchReceiver, filter);
        Log.i(LOG_TAG, "registerSwitchReceiver");
    }

    /**
     * wifi开关状态的取消注册广播
     */
    public void unRegisterSwitchReceiver() {
        if (mWifiSwitchReceiver != null) {
            mContext.unregisterReceiver(mWifiSwitchReceiver);
            Log.i(LOG_TAG, "unRegisterSwitchReceiver");
        }
    }

    /**
     * wifi开关状态的监听广播
     */
    class WifiSwitchBroadcastReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            if (mWifiInterface == null) {
                return;
            }
            int wifiState = intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE, 0);
            switch (wifiState) {
                case WifiManager.WIFI_STATE_DISABLED:
                    mWifiInterface.wifiSwitchState(WifiBroadcastInterface.WIFI_STATE_DISABLED);
                    break;
                case WifiManager.WIFI_STATE_DISABLING:
                    mWifiInterface.wifiSwitchState(WifiBroadcastInterface.WIFI_STATE_DISABLING);
                    break;
                case WifiManager.WIFI_STATE_ENABLED:
                    mWifiInterface.wifiSwitchState(WifiBroadcastInterface.WIFI_STATE_ENABLED);
                    break;
                case WifiManager.WIFI_STATE_ENABLING:
                    mWifiInterface.wifiSwitchState(WifiBroadcastInterface.WIFI_STATE_ENABLING);
                    break;
                case WifiManager.WIFI_STATE_UNKNOWN:
                    mWifiInterface.wifiSwitchState(WifiBroadcastInterface.WIFI_STATE_UNKNOWN);
                    break;
                default:
                    Log.e(LOG_TAG, "WifiBroadcastReceiver wifiState invalid parameter");
                    break;
            }
        }
    }
}
