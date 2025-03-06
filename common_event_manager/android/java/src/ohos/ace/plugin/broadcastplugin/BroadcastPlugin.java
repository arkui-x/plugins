/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

package ohos.ace.plugin.broadcastplugin;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.BatteryManager;
import android.text.TextUtils;
import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

/**
 * BroadcastPlugin
 *
 * @since 1
 */
public class BroadcastPlugin {
    private static final String LOG_TAG = "BroadcastPlugin";

    private static final int BATTERY_LOW_VALUE = 20;

    private static final int BATTERY_FULL_VALUE = 100;

    private static boolean BATTERY_LOW_FLAG = false;

    private static boolean BATTERY_NORMAL_FLAG = false;

    private Context context;

    private Map<String, BroadcastReceiver> broadcastReceiverMap;

    /**
     * BroadcastPlugin
     *
     * @param context context of the application
     */
    public BroadcastPlugin(Context context) {
        if (context != null) {
            this.context = context;
        } else {
            Log.e(LOG_TAG, "context is null");
        }
        this.broadcastReceiverMap = new HashMap<>();
        nativeInit();
    }

    /**
     * BroadcastPlugin
     *
     * @param context context of the application
     * @param isNativeInit call nativeInit or not
     */
    public BroadcastPlugin(Context context, boolean isNativeInit) {
        if (context != null) {
            this.context = context;
        } else {
            Log.e(LOG_TAG, "context is null");
        }
        this.broadcastReceiverMap = new HashMap<>();
        if (isNativeInit) {
            nativeInit();
        }
    }

    /**
     * sendBroadcast send broadcast
     *
     * @param action action of the broadcast
     * @param json json of the broadcast
     * @return true if send broadcast success, false if send broadcast failed
     */
    public boolean sendBroadcast(String action, String json) {
        if (context == null) {
            return false;
        }
        if (TextUtils.isEmpty(action)) {
            return false;
        }
        Intent intent = new Intent(action);
        if (!TextUtils.isEmpty(json)) {
            try {
                JSONObject jsonObject = new JSONObject(json);
                for (Iterator<String> iter = jsonObject.keys(); iter.hasNext(); ) {
                    String key = iter.next();
                    String value = (String) jsonObject.get(key);
                    intent.putExtra(key, value);
                }
            } catch (JSONException e) {
                Log.e(LOG_TAG, "Send broadcast failed, JSONException.");
                e.printStackTrace();
                return false;
            }
        }
        context.sendBroadcast(intent);
        return true;
    }

    /**
     * registerBroadcast register broadcast
     *
     * @param key key of the broadcast
     * @param actions actions of the broadcast
     * @return true if register broadcast success, false if register broadcast failed
     */
    public boolean registerBroadcast(String key, String[] actions) {
        if (context == null) {
            return false;
        }
        if (TextUtils.isEmpty(key)) {
            return false;
        }
        if (actions == null || actions.length == 0) {
            return false;
        }
        BroadcastReceiver broadcastReceiver;
        IntentFilter filter = new IntentFilter();
        for (String action : actions) {
            if (Intent.ACTION_BATTERY_LOW.equals(action)
                || Intent.ACTION_BATTERY_OKAY.equals(action)) {
                filter.addAction(Intent.ACTION_BATTERY_CHANGED);
            } else {
                filter.addAction(action);
            }
        }
        broadcastReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                try {
                    JSONObject jsonObject = new JSONObject();
                    Bundle bundle = intent.getExtras();
                    if (bundle != null && bundle.size() > 0) {
                        for (String key : bundle.keySet()) {
                            Object value = bundle.get(key);
                            jsonObject.put(key, value + "");
                        }
                    }
                    String key = "";
                    for (Map.Entry<String, BroadcastReceiver> entry : broadcastReceiverMap.entrySet()) {
                        if (entry.getValue() == this) {
                            key = entry.getKey();
                            break;
                        }
                    }
                    receiveBroadcast(key, intent, jsonObject.toString());
                } catch (JSONException e) {
                    Log.e(LOG_TAG, "Receive broadcast failed, JSONException.");
                    e.printStackTrace();
                }
            }
        };
        context.registerReceiver(broadcastReceiver, filter);
        broadcastReceiverMap.put(key, broadcastReceiver);
        return true;
    }

    private void receiveBroadcast(String key, Intent intent, String json) {
        Log.i(LOG_TAG, " receiveBroadcast " + key);
        if (Intent.ACTION_BATTERY_CHANGED.equals(intent.getAction())) {
            int level = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, 0);
            int scale = intent.getIntExtra(BatteryManager.EXTRA_SCALE, BATTERY_FULL_VALUE);
            int batteryPercent = level * BATTERY_FULL_VALUE / scale;
            if (batteryPercent <= BATTERY_LOW_VALUE
                && key.contains("usual.event.BATTERY_LOW")
                && !BATTERY_LOW_FLAG) {
                    nativeReceiveBroadcast(key, Intent.ACTION_BATTERY_LOW, json);
                    BATTERY_LOW_FLAG = true;
                    BATTERY_NORMAL_FLAG = false;
            }
            if (batteryPercent > BATTERY_LOW_VALUE
                && key.contains("usual.event.BATTERY_OKAY")
                && !BATTERY_NORMAL_FLAG) {
                    nativeReceiveBroadcast(key, Intent.ACTION_BATTERY_OKAY, json);
                    BATTERY_NORMAL_FLAG = true;
                    BATTERY_LOW_FLAG = false;
            }
        } else {
            nativeReceiveBroadcast(key, intent.getAction(), json);
        }
    }

    /**
     * nativeDestroy is used to destroy the native environment
     *
     * @param key the key of broadcast
     * @return boolean whether the broadcast is unregistered successfully
     */
    public boolean unRegisterBroadcast(String key) {
        if (context == null) {
            return false;
        }
        if (TextUtils.isEmpty(key)) {
            return false;
        }
        if (broadcastReceiverMap != null && broadcastReceiverMap.size() > 0) {
            for (String mapKey : broadcastReceiverMap.keySet()) {
                if (mapKey.equals(key)) {
                    BroadcastReceiver broadcastReceiver = broadcastReceiverMap.get(mapKey);
                    context.unregisterReceiver(broadcastReceiver);
                    broadcastReceiver = null;
                    break;
                }
            }
        }
        return true;
    }

    /**
     * nativeInit is used to initialize the native environment
     */
    protected native void nativeInit();

    /**
     * nativeReceiveBroadcast is used to receive broadcast
     *
     * @param key the key of broadcast
     * @param action the action of broadcast
     * @param json the json of broadcast
     */
    protected native void nativeReceiveBroadcast(String key, String action, String json);
}
