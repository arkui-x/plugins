/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
import android.os.BatteryManager;
import android.os.Bundle;
import android.text.TextUtils;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import ohos.ace.adapter.ALog;

import org.json.JSONException;
import org.json.JSONObject;

/**
 * BroadcastPlugin
 *
 * @since 1
 */
public class BroadcastPlugin {
    private static final String LOG_TAG = "BroadcastPlugin";
    private static final String JSON_BUNDLE_NAME = "bundleName";
    private static final String JSON_CODE = "code";
    private static final String JSON_DATA = "data";
    private static final String JSON_PARAMETERS = "parameters";
    private static final String EXTRA_BUNDLE_NAME = "common_event_bundle_name";
    private static final String EXTRA_CODE = "common_event_code";
    private static final String EXTRA_DATA = "common_event_data";
    private static final String EXTRA_PARAMETERS = "common_event_parameters";

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
            ALog.e(LOG_TAG, "context is null");
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
            ALog.e(LOG_TAG, "context is null");
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
     * @param json json of the broadcast, supports only four reserved fields:
     *             "bundleName" (String), "code" (Number),
     *             "data" (String), "parameters" (JSONObject).
     *             Any other top-level keys are ignored.
     *             Use "parameters" to pass custom key-value pairs.
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
        if (!TextUtils.isEmpty(json) && !appendPublishPayload(intent, json)) {
            return false;
        }
        context.sendBroadcast(intent);
        return true;
    }

    // Only handles the four reserved fields: bundleName, code, data, parameters.
    // Other top-level keys in the JSON are intentionally ignored.
    private boolean appendPublishPayload(Intent intent, String json) {
        try {
            JSONObject payload = new JSONObject(json);
            String bundleName = payload.optString(JSON_BUNDLE_NAME, "");
            setTargetBundle(intent, bundleName);
            putBundleNameExtra(intent, bundleName);
            return putCodeExtra(intent, payload)
                && putDataExtra(intent, payload)
                && putParametersExtra(intent, payload);
        } catch (JSONException e) {
            ALog.e(LOG_TAG, "Send broadcast failed, JSONException.");
            return false;
        }
    }

    private void setTargetBundle(Intent intent, String bundleName) {
        if (!TextUtils.isEmpty(bundleName)) {
            Intent result = intent.setPackage(bundleName);
            if (result == null) {
                ALog.w(LOG_TAG, "Failed to set target bundle, check if bundleName is valid: " + bundleName);
            }
            ALog.d(LOG_TAG, "Broadcast targeted to bundle: " + bundleName);
        }
    }

    private void putBundleNameExtra(Intent intent, String bundleName) {
        if (!TextUtils.isEmpty(bundleName)) {
            intent.putExtra(EXTRA_BUNDLE_NAME, bundleName);
        }
    }

    private boolean putCodeExtra(Intent intent, JSONObject payload) throws JSONException {
        if (!payload.has(JSON_CODE)) {
            return true;
        }
        Object value = payload.get(JSON_CODE);
        if (!(value instanceof Number)) {
            ALog.e(LOG_TAG, "code is not a number");
            return false;
        }
        intent.putExtra(EXTRA_CODE, ((Number) value).intValue());
        return true;
    }

    private boolean putDataExtra(Intent intent, JSONObject payload) throws JSONException {
        if (!payload.has(JSON_DATA)) {
            return true;
        }
        Object value = payload.get(JSON_DATA);
        if (!(value instanceof String)) {
            ALog.e(LOG_TAG, "data is not a string");
            return false;
        }
        intent.putExtra(EXTRA_DATA, (String) value);
        return true;
    }

    private boolean putParametersExtra(Intent intent, JSONObject payload) throws JSONException {
        if (!payload.has(JSON_PARAMETERS)) {
            return true;
        }
        Object value = payload.get(JSON_PARAMETERS);
        if (!(value instanceof JSONObject)) {
            ALog.e(LOG_TAG, "parameters is not a JSONObject");
            return false;
        }
        intent.putExtra(EXTRA_PARAMETERS, value.toString());
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
                    receiveBroadcast(findReceiverKey(this), intent, buildReceivePayload(intent));
                } catch (JSONException e) {
                    ALog.e(LOG_TAG, "Receive broadcast failed, JSONException.");
                    e.printStackTrace();
                }
            }
        };
        context.registerReceiver(broadcastReceiver, filter);
        broadcastReceiverMap.put(key, broadcastReceiver);
        return true;
    }

    private void receiveBroadcast(String key, Intent intent, String json) {
        ALog.i(LOG_TAG, " receiveBroadcast " + key);
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

    private String findReceiverKey(BroadcastReceiver receiver) {
        for (Map.Entry<String, BroadcastReceiver> entry : broadcastReceiverMap.entrySet()) {
            if (entry.getValue() == receiver) {
                return entry.getKey();
            }
        }
        return "";
    }

    private String buildReceivePayload(Intent intent) throws JSONException {
        JSONObject payload = new JSONObject();
        Bundle bundle = intent.getExtras();
        JSONObject parameters = readParameters(bundle);
        appendBundleValues(bundle, payload, parameters);
        setBundleNameIfNeeded(payload, bundle, intent);
        return finalizePayload(payload, parameters);
    }

    private void setBundleNameIfNeeded(JSONObject payload, Bundle bundle, Intent intent) throws JSONException {
        if (payload.has(JSON_BUNDLE_NAME)) {
            return;
        }
        String bundleName = resolveBundleName(bundle, intent);
        if (!TextUtils.isEmpty(bundleName)) {
            payload.put(JSON_BUNDLE_NAME, bundleName);
        }
    }

    private String resolveBundleName(Bundle bundle, Intent intent) {
        if (bundle != null) {
            Object value = bundle.get(EXTRA_BUNDLE_NAME);
            if (value != null) {
                return String.valueOf(value);
            }
        }
        return intent.getPackage();
    }

    private JSONObject readParameters(Bundle bundle) throws JSONException {
        JSONObject parameters = new JSONObject();
        if (bundle == null) {
            return parameters;
        }
        Object value = bundle.get(EXTRA_PARAMETERS);
        if (value == null) {
            return parameters;
        }
        if (!(value instanceof String)) {
            return parameters;
        }
        JSONObject jsonObject = new JSONObject(String.valueOf(value));
        Iterator<String> keys = jsonObject.keys();
        while (keys.hasNext()) {
            String name = keys.next();
            parameters.put(name, jsonObject.get(name));
        }
        return parameters;
    }

    private void appendBundleValues(Bundle bundle, JSONObject payload, JSONObject parameters) throws JSONException {
        if (bundle == null || bundle.size() == 0) {
            return;
        }
        for (String extraKey : bundle.keySet()) {
            Object value = bundle.get(extraKey);
            if (appendReservedExtra(extraKey, value, payload)) {
                continue;
            }
            if (EXTRA_PARAMETERS.equals(extraKey) || parameters.has(extraKey)) {
                continue;
            }
            putJsonValue(parameters, extraKey, value);
        }
    }

    private boolean appendReservedExtra(String extraKey, Object value, JSONObject payload) throws JSONException {
        if (EXTRA_BUNDLE_NAME.equals(extraKey)) {
            payload.put(JSON_BUNDLE_NAME, value == null ? "" : String.valueOf(value));
            return true;
        }
        if (EXTRA_CODE.equals(extraKey)) {
            payload.put(JSON_CODE, value instanceof Number ? ((Number) value).intValue() : 0);
            return true;
        }
        if (EXTRA_DATA.equals(extraKey)) {
            payload.put(JSON_DATA, value == null ? "" : String.valueOf(value));
            return true;
        }
        return false;
    }

    private String finalizePayload(JSONObject payload, JSONObject parameters) throws JSONException {
        if (!payload.has(JSON_CODE)) {
            payload.put(JSON_CODE, 0);
        }
        if (!payload.has(JSON_DATA)) {
            payload.put(JSON_DATA, "");
        }
        payload.put(JSON_PARAMETERS, parameters);
        return payload.toString();
    }

    private void putJsonValue(JSONObject jsonObject, String key, Object value) throws JSONException {
        if (value == null) {
            jsonObject.put(key, JSONObject.NULL);
            return;
        }
        Object wrappedValue = JSONObject.wrap(value);
        jsonObject.put(key, wrappedValue == null ? String.valueOf(value) : wrappedValue);
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
