/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

package ohos.ace.plugin.webviewplugin.webstorage;

import android.content.Context;
import android.webkit.ValueCallback;
import android.webkit.WebStorage;
import java.util.Map;
import java.util.ArrayList;
import org.json.JSONObject;

/**
 * The WebStoragePlugin class provides a functionality for handling web storage plugins.
 * This class is mainly used for processing file path conversion and storage management.
 *
 * @since 2025-01-10
 */
public class WebStoragePlugin {
    private static final String LOG_TAG = "WebStoragePlugin";
    private static final String FILE = "file:///";
    private static final String REPLACEMENT = "resource://rawfile/";

    /**
     * Construct a WebStoragePlugin object to initialize local resources.
     *
     * @param context The context is used to create the context for the WebStoragePlugin.
     */
    public WebStoragePlugin(Context context) {
        nativeInit();
    }

    /**
     * Get the quota for the specified source.
     *
     * @param origin The source to be queried.
     * @param asyncCallbackInfoId Asynchronous callback information ID.
     */
    public void getOriginQuota(String origin, long asyncCallbackInfoId) {
        WebStorage.getInstance().getQuotaForOrigin(origin, new ValueCallback<Long>() {
            @Override
            public void onReceiveValue(Long value) {
                onReceiveStorageValue(value, asyncCallbackInfoId);
            }
        });
    }

    /**
     * Retrieve the storage usage of the specified source.
     *
     * @param origin The identification string of the source.
     * @param asyncCallbackInfoId The identifier for asynchronous callback information.
     */
    public void getOriginUsage(String origin, long asyncCallbackInfoId) {
        WebStorage.getInstance().getUsageForOrigin(origin, new ValueCallback<Long>() {
            @Override
            public void onReceiveValue(Long value) {
                onReceiveStorageUsageValue(value, asyncCallbackInfoId);
            }
        });
    }

    /**
     * Retrieve storage source information.
     *
     * @param asyncCallbackInfoId Asynchronous callback information ID.
     */
    public void getOrigins(long asyncCallbackInfoId) {
        WebStorage.getInstance().getOrigins(new ValueCallback<Map>() {
            @Override
            public void onReceiveValue(Map value) {
                ArrayList<JSONObject> keyValueList = new ArrayList<>();
                for (Object key : value.keySet()) {
                    if (value.get(key) instanceof WebStorage.Origin) {
                        WebStorage.Origin originValue = (WebStorage.Origin) value.get(key);
                        String origin = originValue.getOrigin();
                        long quota = originValue.getQuota();
                        long usage = originValue.getUsage();
                        if (FILE.equals(origin)) {
                            origin = REPLACEMENT;
                        }
                        try {
                            JSONObject employee = new JSONObject();
                            employee.put("origin", origin);
                            employee.put("quota", Long.toString(quota));
                            employee.put("usage", Long.toString(usage));
                            keyValueList.add(employee);
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                }
                String currentValue = keyValueList.toString();
                onReceiveStorageValue(keyValueList, asyncCallbackInfoId);
            }
        });
    }

    /**
     * Delete All Data.
     */
    public void deleteAllData() {
        WebStorage.getInstance().deleteAllData();
    }

    /**
     * Delete data from the specified source.
     *
     * @param origin The source to be queried.
     */
    public void deleteOrigin(String origin) {
        WebStorage.getInstance().deleteOrigin(origin);
    }

    /**
     * Initialize local methods.
     */
    protected native void nativeInit();

    /**
     * The callback method for receiving stored values.
     *
     * @param value stored value
     * @param asyncCallbackInfoId Asynchronous callback information ID
     */
    protected native static void onReceiveStorageValue(long value, long asyncCallbackInfoId);

    /**
     * The callback method for receiving storage usage values.
     *
     * @param value Storage usage value
     * @param asyncCallbackInfoId Asynchronous callback information ID
     */
    protected native static void onReceiveStorageUsageValue(long value, long asyncCallbackInfoId);

    /**
     * The callback method for receiving stored values.
     *
     * @param value Storage value list
     * @param asyncCallbackInfoId Asynchronous callback information ID
     */
    protected native static void onReceiveStorageValue(ArrayList<JSONObject> value, long asyncCallbackInfoId);
}