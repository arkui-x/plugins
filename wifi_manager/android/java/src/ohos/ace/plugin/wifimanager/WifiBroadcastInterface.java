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

/**
 * WifiBroadcastInterface Interface definition for WifiManager Android Plugin.
 *
 * @since 2024-06-24
 */
public interface WifiBroadcastInterface {
    /**
     * Wifi connect state callback.
     */
    String TAG = "WifiManagerAndroid";

    /**
     * 0-Disconnected
     */
    int WIFI_STATE_DISCONNECT = 0;

    /**
     * 1-Connected
     */
    int WIFI_STATE_LINKED = 1;

    /**
     * 2-Connecting
     */
    int WIFI_STATE_ENABLING = 2;

    /**
     * 3-Enabled
     */
    int WIFI_STATE_ENABLED = 3;

    /**
     * 4-Disabling
     */
    int WIFI_STATE_DISABLING = 0;

    /**
     * 5-Disabled
     */
    int WIFI_STATE_DISABLED = 1;

    /**
     * Wifi connect state callback.
     *
     * @param state 0-Disconnected 1-Connected 2-Connecting 3-Enabled 4-Disabling 5-Disabled
     */
    void wifiConnectState(int state);

    /**
     * Wifi switch state callback.
     *
     * @param state 0-off 1-on
     */
    void wifiSwitchState(int state);
}