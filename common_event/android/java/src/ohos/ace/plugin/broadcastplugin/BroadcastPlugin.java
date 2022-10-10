/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
                e.printStackTrace();
                return false;
            }
        }
        context.sendBroadcast(intent);
        return true;
    }

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
            filter.addAction(action);
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
                    receiveBroadcast(intent.getAction(), jsonObject.toString());
                } catch (JSONException e) {
                    e.printStackTrace();
                }
            }
        };
        context.registerReceiver(broadcastReceiver, filter);
        broadcastReceiverMap.put(key, broadcastReceiver);
        return true;
    }

    private void receiveBroadcast(String action, String json) {
        nativeReceiveBroadcast(action, json);
    }

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

    protected native void nativeInit();
    protected native void nativeReceiveBroadcast(String action, String json);
}
