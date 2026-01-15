/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
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

package ohos.ace.plugin.clipboard;

import android.content.ClipDescription;
import android.content.Intent;
import android.os.Bundle;
import android.os.PersistableBundle;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.Set;

/**
 * Helper utilities to convert between the project's typed JSON parameter format and
 * Android `Bundle`/`Intent`/`ClipDescription` extras.
 *
 * Supported parameter types (type code):
 * - 1  : boolean
 * - 5  : int
 * - 9  : double
 * - 10 : string
 * - 101: wantParams (nested params array)
 * - 102: array (primitive arrays: boolean/int/long/double/string)
 *
 * @since 1
 */
public class WantParamsUtils {
    private static final String LOG_TAG = "WantParamsUtils";
    private static final String KEY_TYPE = "type";
    private static final String KEY_VALUE = "value";
    private static final String KEY_VALUE_JSON = "value_json";
    private static final int TYPE_BOOLEAN = 1;
    private static final int TYPE_INT = 5;
    private static final int TYPE_DOUBLE = 9;
    private static final int TYPE_STRING = 10;
    private static final int TYPE_WANT_PARAMS = 101;
    private static final int TYPE_ARRAY = 102;

    /**
     * Convert the typed JSON string (with top-level "params" array) into an Android
     * {@link Bundle} suitable for use as Intent extras or ClipDescription extras.
     *
     * @param json typed JSON string, or null
     * @return a {@link Bundle} representing the parameters, or an empty Bundle on parse error
     */
    public static Bundle jsonToExtras(String json) {
        if (json == null) {
            return new Bundle();
        }
        try {
            JSONObject root = new JSONObject(json);
            JSONArray params = root.optJSONArray("params");
            if (params == null) {
                return new Bundle();
            } else {
                return buildBundleFromParamsArray(params);
            }
        } catch (JSONException e) {
            Log.e(LOG_TAG, "jsonToExtras: JSON parse error");
            return new Bundle();
        }
    }

    /**
     * Convert an extras {@link Bundle} (produced by {@link #jsonToExtras}) back to the
     * typed JSON string format (with top-level "params" array).
     *
     * @param extras Bundle to convert
     * @return typed JSON string or empty string on error
     */
    public static String extrasToJson(Bundle extras) {
        if (extras == null) {
            return "{}";
        }
        try {
            JSONObject root = new JSONObject();
            JSONArray params = buildParamsArrayFromBundle(extras);
            root.put("params", params);
            return root.toString();
        } catch (JSONException e) {
            Log.e(LOG_TAG, "extrasToJson: JSON parse error");
            return "{}";
        }
    }

    private static String intentToJsonString(Intent intent) {
        if (intent == null) {
            Log.e(LOG_TAG, "intentToJsonString: intent is null");
            return "{}";
        }
        try {
            JSONObject root = new JSONObject();
            if (intent.getComponent() != null) {
                root.put("bundleName", intent.getComponent().getPackageName());
                root.put("abilityName", intent.getComponent().getClassName());
            }

            if (intent.hasExtra("ArkUIXModuleNameCustom")) {
                root.put("moduleName", intent.getStringExtra("ArkUIXModuleNameCustom"));
                intent.removeExtra("ArkUIXModuleNameCustom");
            }
            if (intent.getType() != null) {
                root.put("type", intent.getType());
            }
            Bundle extras = intent.getExtras();
            if (extras != null && !extras.isEmpty()) {
                JSONArray params = buildParamsArrayFromBundle(extras);
                root.put("params", params);
            }

            return root.toString();
        } catch (JSONException e) {
            Log.e(LOG_TAG, "intentToJsonString: JSON parse error");
            return "{}";
        }
    }

    /**
     * Put typed JSON parameters into the given {@link Intent} using the appropriate
     * {@code putExtra} overloads. For {@code wantParams} (type=101) a nested
     * {@link Bundle} is created and stored with {@code putExtra(name, Bundle)}.
     *
     * @param intent target intent
     * @param json typed JSON string
     *
     * @return true if successful, false otherwise
     */
    public static Boolean putJsonToIntent(Intent intent, String json) {
        if (intent == null || json == null) {
            Log.e(LOG_TAG, "putJsonToIntent: intent or json is null");
            return false;
        }
        try {
            JSONObject root = new JSONObject(json);
            String bundleName = root.optString("bundleName", null);
            String abilityName = root.optString("abilityName", null);
            String moduleName = root.optString("moduleName", null);
            if (bundleName != null && abilityName != null) {
                intent.setClassName(bundleName, abilityName);
            }
            if (moduleName != null) {
                intent.putExtra("ArkUIXModuleNameCustom", moduleName);
            }
            String typeFlag = root.optString("type", null);
            if (typeFlag != null) {
                intent.setType(typeFlag);
            }
            JSONArray params = root.optJSONArray("params");
            if (params == null) {
                return false;
            }
            for (int i = 0; i < params.length(); i++) {
                JSONObject p = params.getJSONObject(i);
                String key = p.optString("key", null);
                if (key == null) {
                    continue;
                }
                int type = p.getInt("type");
                handlePutParamToIntent(intent, key, type, p);
            }
        } catch (JSONException ignored) {
            Log.w(LOG_TAG, "putJsonToIntent: JSON parse error");
            return false;
        }
        return true;
    }

    private static void handlePutParamToIntent(Intent intent, String key, int type, JSONObject p)
            throws JSONException {
        if (type == TYPE_BOOLEAN) {
            intent.putExtra(key, p.getBoolean("value"));
            return;
        }
        if (type == TYPE_INT) {
            intent.putExtra(key, p.getInt("value"));
            return;
        }
        if (type == TYPE_DOUBLE) {
            intent.putExtra(key, p.getDouble("value"));
            return;
        }
        if (type == TYPE_STRING) {
            intent.putExtra(key, p.getString("value"));
            return;
        }
        if (type == TYPE_WANT_PARAMS) {
            JSONArray inner = p.optJSONArray("value");
            Bundle innerBundle = (inner == null) ? new Bundle() : buildBundleFromParamsArray(inner);
            intent.putExtra(key, innerBundle);
            return;
        }
        if (type == TYPE_ARRAY) {
            JSONArray arrValue = p.optJSONArray("value");
            if (arrValue == null) {
                return;
            }
            handlePutArrayToIntent(intent, key, arrValue);
            return;
        }
        Log.w(LOG_TAG, "handlePutParamToIntent: Unknown param type");
        String defaultValue = p.optString("value", "");
        intent.putExtra(key, defaultValue);
    }

    private static void handlePutArrayToIntent(Intent intent, String key, JSONArray arrValue) throws JSONException {
        Object primitiveArray = jsonArrayToPrimitiveArray(arrValue);
        if (primitiveArray instanceof boolean[]) {
            intent.putExtra(key, (boolean[]) primitiveArray);
            return;
        }
        if (primitiveArray instanceof int[]) {
            intent.putExtra(key, (int[]) primitiveArray);
            return;
        }
        if (primitiveArray instanceof long[]) {
            intent.putExtra(key, (long[]) primitiveArray);
            return;
        }
        if (primitiveArray instanceof double[]) {
            intent.putExtra(key, (double[]) primitiveArray);
            return;
        }
        if (primitiveArray instanceof String[]) {
            intent.putExtra(key, (String[]) primitiveArray);
            return;
        }
        intent.putExtra(key, arrValue.toString());
    }

    /**
     * Read typed JSON from an {@link Intent}'s extras by converting the extras
     * {@link Bundle} back to the typed JSON string.
     *
     * @param intent source intent
     * @return typed JSON string or null
     */
    public static String getJsonFromIntent(Intent intent) {
        if (intent == null) {
            Log.e(LOG_TAG, "getJsonFromIntent: intent is null");
            return "{}";
        }
        return intentToJsonString(intent);
    }

    /**
     * Attach the provided {@link Bundle} as extras to a {@link ClipDescription}.
     * Some platform versions may not support modifying ClipDescription extras;
     * callers should handle compatibility if necessary.
     *
     * @param desc target ClipDescription
     * @param extras extras to attach
     */
    public static void setExtrasToClipDescription(ClipDescription desc, Bundle extras) {
        if (desc == null || extras == null) {
            Log.e(LOG_TAG, "setExtrasToClipDescription: desc or extras is null");
            return;
        }
        try {
            PersistableBundle pb = bundleToPersistableBundle(extras);
            desc.setExtras(pb);
        } catch (NoSuchMethodError ignored) {
            // platform may not allow modifying ClipDescription extras; caller must handle it
            Log.w(LOG_TAG, "setExtrasToClipDescription: platform does not support setting ClipDescription extras");
        }
    }

    /**
     * Convert the extras attached to a {@link ClipDescription} back into a typed
     * JSON string.
     *
     * @param desc source ClipDescription
     * @return typed JSON string or empty string on error
     */
    public static String getJsonFromClipDescription(ClipDescription desc) {
        if (desc == null) {
            Log.e(LOG_TAG, "getJsonFromClipDescription: desc is null");
            return "{}";
        }
        PersistableBundle pb = desc.getExtras();
        Bundle b = persistableBundleToBundle(pb);
        return extrasToJson(b);
    }

    private static PersistableBundle bundleToPersistableBundle(Bundle bundle) {
        PersistableBundle result = new PersistableBundle();
        if (bundle == null) {
            Log.e(LOG_TAG, "bundleToPersistableBundle: bundle is null");
            return result;
        }
        Set<String> keys = bundle.keySet();
        for (String key : keys) {
            Object value = bundle.get(key);
            if (value instanceof Bundle) {
                result.putPersistableBundle(key, bundleToPersistableBundle((Bundle) value));
            } else if (value instanceof Boolean) {
                result.putBoolean(key, (Boolean) value);
            } else if (value instanceof Integer) {
                result.putInt(key, (Integer) value);
            } else if (value instanceof Long) {
                result.putLong(key, (Long) value);
            } else if (value instanceof Double) {
                result.putDouble(key, (Double) value);
            } else if (value instanceof String) {
                result.putString(key, (String) value);
            } else if (value instanceof boolean[]) {
                result.putBooleanArray(key, (boolean[]) value);
            } else if (value instanceof int[]) {
                result.putIntArray(key, (int[]) value);
            } else if (value instanceof long[]) {
                result.putLongArray(key, (long[]) value);
            } else if (value instanceof double[]) {
                result.putDoubleArray(key, (double[]) value);
            } else if (value instanceof String[]) {
                result.putStringArray(key, (String[]) value);
            } else {
                // fallback to string representation
                result.putString(key, String.valueOf(value));
            }
        }
        return result;
    }

    private static Bundle persistableBundleToBundle(PersistableBundle pb) {
        Bundle result = new Bundle();
        if (pb == null) {
            Log.e(LOG_TAG, "persistableBundleToBundle: pb is null");
            return result;
        }
        Set<String> keys = pb.keySet();
        for (String key : keys) {
            Object value = pb.get(key);
            if (value instanceof PersistableBundle) {
                result.putBundle(key, persistableBundleToBundle((PersistableBundle) value));
            } else if (value instanceof Boolean) {
                result.putBoolean(key, (Boolean) value);
            } else if (value instanceof Integer) {
                result.putInt(key, (Integer) value);
            } else if (value instanceof Long) {
                result.putLong(key, (Long) value);
            } else if (value instanceof Double) {
                result.putDouble(key, (Double) value);
            } else if (value instanceof String) {
                result.putString(key, (String) value);
            } else if (value instanceof boolean[]) {
                result.putBooleanArray(key, (boolean[]) value);
            } else if (value instanceof int[]) {
                result.putIntArray(key, (int[]) value);
            } else if (value instanceof long[]) {
                result.putLongArray(key, (long[]) value);
            } else if (value instanceof double[]) {
                result.putDoubleArray(key, (double[]) value);
            } else if (value instanceof String[]) {
                result.putStringArray(key, (String[]) value);
            } else {
                result.putString(key, String.valueOf(value));
            }
        }
        return result;
    }

    private static Bundle buildBundleFromParamsArray(JSONArray arr) throws JSONException {
        Bundle result = new Bundle();
        for (int i = 0; i < arr.length(); i++) {
            JSONObject p = arr.getJSONObject(i);
            putParamIntoBundle(result, p);
        }
        return result;
    }

    private static void putParamIntoBundle(Bundle result, JSONObject paramObj) throws JSONException {
        String key = paramObj.optString("key", null);
        if (key == null) {
            Log.e(LOG_TAG, "putParamIntoBundle: key is null");
            return;
        }
        int type = paramObj.getInt("type");
        if (type == TYPE_BOOLEAN) {
            handleBooleanParam(result, key, paramObj);
            return;
        }
        if (type == TYPE_INT) {
            handleIntParam(result, key, paramObj);
            return;
        }
        if (type == TYPE_DOUBLE) {
            handleDoubleParam(result, key, paramObj);
            return;
        }
        if (type == TYPE_STRING) {
            handleStringParam(result, key, paramObj);
            return;
        }
        if (type == TYPE_WANT_PARAMS) {
            handleWantParamsParam(result, key, paramObj);
            return;
        }
        if (type == TYPE_ARRAY) {
            handleArrayParam(result, key, paramObj);
            return;
        }
        handleDefaultParam(result, key, paramObj);
    }

    private static void handleBooleanParam(Bundle result, String key, JSONObject paramObj) throws JSONException {
        result.putBoolean(key, paramObj.getBoolean("value"));
    }

    private static void handleIntParam(Bundle result, String key, JSONObject paramObj) throws JSONException {
        Object valueObj = paramObj.opt("value");
        if (valueObj instanceof Number) {
            Number numeric = (Number) valueObj;
            long longValue = numeric.longValue();
            if (longValue < Integer.MIN_VALUE || longValue > Integer.MAX_VALUE) {
                result.putLong(key, longValue);
            } else {
                result.putInt(key, numeric.intValue());
            }
            return;
        }
        String text = paramObj.optString("value", "0");
        try {
            long longValue = Long.parseLong(text);
            if (longValue < Integer.MIN_VALUE || longValue > Integer.MAX_VALUE) {
                result.putLong(key, longValue);
            } else {
                result.putInt(key, (int) longValue);
            }
        } catch (NumberFormatException ex) {
            Log.w(LOG_TAG, "handleIntParam: NumberFormatException for key " + key);
            result.putInt(key, 0);
        }
    }

    private static void handleDoubleParam(Bundle result, String key, JSONObject paramObj) throws JSONException {
        result.putDouble(key, paramObj.getDouble("value"));
    }

    private static void handleStringParam(Bundle result, String key, JSONObject paramObj) throws JSONException {
        result.putString(key, paramObj.getString("value"));
    }

    private static void handleWantParamsParam(Bundle result, String key, JSONObject paramObj) throws JSONException {
        JSONArray inner = paramObj.optJSONArray("value");
        if (inner != null) {
            Bundle innerBundle = buildBundleFromParamsArray(inner);
            result.putBundle(key, innerBundle);
        } else {
            result.putBundle(key, new Bundle());
        }
    }

    private static void handleArrayParam(Bundle result, String key, JSONObject paramObj) throws JSONException {
        JSONArray arrValue = paramObj.optJSONArray("value");
        if (arrValue == null) {
            return;
        }
        Object primitiveArray = jsonArrayToPrimitiveArray(arrValue);
        if (primitiveArray instanceof boolean[]) {
            result.putBooleanArray(key, (boolean[]) primitiveArray);
            return;
        }
        if (primitiveArray instanceof int[]) {
            result.putIntArray(key, (int[]) primitiveArray);
            return;
        }
        if (primitiveArray instanceof long[]) {
            result.putLongArray(key, (long[]) primitiveArray);
            return;
        }
        if (primitiveArray instanceof double[]) {
            result.putDoubleArray(key, (double[]) primitiveArray);
            return;
        }
        if (primitiveArray instanceof String[]) {
            result.putStringArray(key, (String[]) primitiveArray);
            return;
        }
        result.putString(key, arrValue.toString());
    }

    private static void handleDefaultParam(Bundle result, String key, JSONObject p) throws JSONException {
        result.putString(key, p.optString("value", ""));
    }

    private static JSONArray buildParamsArrayFromBundle(Bundle bundle) throws JSONException {
        JSONArray params = new JSONArray();
        Set<String> keys = bundle.keySet();
        for (String key : keys) {
            Object valueObj = bundle.get(key);
            params.put(paramFromEntry(key, valueObj));
        }
        return params;
    }

    private static JSONObject paramFromEntry(String key, Object valueObj) throws JSONException {
        JSONObject paramJson = new JSONObject();
        paramJson.put("key", key);
        if (valueObj instanceof Bundle) {
            paramJson.put("type", TYPE_WANT_PARAMS);
            paramJson.put("value", buildParamsArrayFromBundle((Bundle) valueObj));
            return paramJson;
        }
        if (valueObj instanceof Boolean) {
            paramJson.put("type", TYPE_BOOLEAN);
            paramJson.put("value", (Boolean) valueObj);
            return paramJson;
        }
        if (valueObj instanceof Number) {
            return paramFromNumber(key, (Number) valueObj, paramJson);
        }
        if (valueObj instanceof String) {
            paramJson.put("type", TYPE_STRING);
            paramJson.put("value", (String) valueObj);
            return paramJson;
        }
        // arrays handled by helper
        if (valueObj instanceof boolean[]) {
            paramJson.put("type", TYPE_ARRAY);
            paramJson.put("value", arrayToJson((boolean[]) valueObj));
            return paramJson;
        }
        if (valueObj instanceof int[]) {
            paramJson.put("type", TYPE_ARRAY);
            paramJson.put("value", arrayToJson((int[]) valueObj));
            return paramJson;
        }
        if (valueObj instanceof long[]) {
            paramJson.put("type", TYPE_ARRAY);
            paramJson.put("value", arrayToJson((long[]) valueObj));
            return paramJson;
        }
        if (valueObj instanceof double[]) {
            paramJson.put("type", TYPE_ARRAY);
            paramJson.put("value", arrayToJson((double[]) valueObj));
            return paramJson;
        }
        if (valueObj instanceof String[]) {
            paramJson.put("type", TYPE_ARRAY);
            paramJson.put("value", arrayToJson((String[]) valueObj));
            return paramJson;
        }
        paramJson.put("type", TYPE_STRING);
        paramJson.put("value", valueObj == null ? JSONObject.NULL : valueObj.toString());
        return paramJson;
    }

    private static JSONObject paramFromNumber(String key, Number numeric, JSONObject paramJson) throws JSONException {
        // numeric may be Integer, Long, Double, Float, etc.
        if (numeric instanceof Integer) {
            paramJson.put("type", TYPE_INT);
            paramJson.put("value", numeric.intValue());
            return paramJson;
        }
        if (numeric instanceof Long) {
            long longVal = numeric.longValue();
            paramJson.put("type", TYPE_INT);
            paramJson.put("value", longVal >= Integer.MIN_VALUE && longVal <= Integer.MAX_VALUE ?
                (int) longVal : longVal);
            return paramJson;
        }
        if (numeric instanceof Double || numeric instanceof Float) {
            paramJson.put("type", TYPE_DOUBLE);
            paramJson.put("value", numeric.doubleValue());
            return paramJson;
        }
        // fallback for other Number implementations
        double dv = numeric.doubleValue();
        if (Double.isFinite(dv) && dv == Math.rint(dv) && dv >= Integer.MIN_VALUE && dv <= Integer.MAX_VALUE) {
            paramJson.put("type", TYPE_INT);
            paramJson.put("value", (int) dv);
            return paramJson;
        }
        paramJson.put("type", TYPE_DOUBLE);
        paramJson.put("value", dv);
        return paramJson;
    }

    private static JSONArray arrayToJson(boolean[] a) throws JSONException {
        JSONArray arr = new JSONArray();
        for (boolean isVal : a) {
            arr.put(isVal);
        }
        return arr;
    }

    private static JSONArray arrayToJson(int[] a) throws JSONException {
        JSONArray arr = new JSONArray();
        for (int v : a) {
            arr.put(v);
        }
        return arr;
    }

    private static JSONArray arrayToJson(long[] a) throws JSONException {
        JSONArray arr = new JSONArray();
        for (long v : a) {
            arr.put(v);
        }
        return arr;
    }

    private static JSONArray arrayToJson(double[] a) throws JSONException {
        JSONArray arr = new JSONArray();
        for (double v : a) {
            arr.put(v);
        }
        return arr;
    }

    private static JSONArray arrayToJson(String[] a) throws JSONException {
        JSONArray arr = new JSONArray();
        for (String s : a) {
            arr.put(s);
        }
        return arr;
    }

    private static Object jsonArrayToPrimitiveArray(JSONArray arr) throws JSONException {
        ArrayFlags flags = detectArrayFlags(arr);
        return buildPrimitiveArrayFromFlags(arr, flags);
    }

    private static class ArrayFlags {
        boolean hasString;
        boolean hasBoolean;
        boolean hasNumber;
        boolean hasDecimal;
        boolean hasLong;
        int length;
    }

    private static ArrayFlags detectArrayFlags(JSONArray arr) throws JSONException {
        ArrayFlags flags = new ArrayFlags();
        flags.length = arr.length();
        for (int i = 0; i < flags.length; i++) {
            Object item = arr.get(i);
            if (item instanceof String) {
                flags.hasString = true;
            } else if (item instanceof Boolean) {
                flags.hasBoolean = true;
            } else if (item instanceof Number) {
                flags.hasNumber = true;
                double doubleVal = ((Number) item).doubleValue();
                long longVal = ((Number) item).longValue();
                if (Double.compare(doubleVal, (double) longVal) != 0) {
                    flags.hasDecimal = true;
                }
                if (longVal < Integer.MIN_VALUE || longVal > Integer.MAX_VALUE) {
                    flags.hasLong = true;
                }
            } else {
                Log.e(LOG_TAG, "detectArrayFlags: unsupported array item type: " +
                    ((item == null) ? "null" : item.getClass().getName()));
            }
        }
        return flags;
    }

    private static Object buildPrimitiveArrayFromFlags(JSONArray arr, ArrayFlags flags) throws JSONException {
        int length = flags.length;
        if (length == 0) {
            return new String[0];
        }
        if (flags.hasString) {
            return buildStringArray(arr, length);
        }
        if (flags.hasBoolean && !flags.hasNumber && !flags.hasString) {
            return buildBooleanArray(arr, length);
        }
        if (flags.hasNumber) {
            if (flags.hasDecimal) {
                return buildDoubleArray(arr, length);
            }
            if (flags.hasLong) {
                return buildLongArray(arr, length);
            }
            return buildIntArray(arr, length);
        }
        return buildFallbackStringArray(arr, length);
    }

    private static String[] buildStringArray(JSONArray arr, int length) throws JSONException {
        String[] result = new String[length];
        for (int i = 0; i < length; i++) {
            result[i] = String.valueOf(arr.get(i));
        }
        return result;
    }

    private static boolean[] buildBooleanArray(JSONArray arr, int length) throws JSONException {
        boolean[] result = new boolean[length];
        for (int i = 0; i < length; i++) {
            result[i] = arr.getBoolean(i);
        }
        return result;
    }

    private static double[] buildDoubleArray(JSONArray arr, int length) throws JSONException {
        double[] result = new double[length];
        for (int i = 0; i < length; i++) {
            Object value = arr.get(i);
            if (value instanceof Number) {
                result[i] = ((Number) value).doubleValue();
            } else {
                result[i] = Double.parseDouble(String.valueOf(value));
            }
        }
        return result;
    }

    private static long[] buildLongArray(JSONArray arr, int length) throws JSONException {
        long[] result = new long[length];
        for (int i = 0; i < length; i++) {
            Object value = arr.get(i);
            if (value instanceof Number) {
                result[i] = ((Number) value).longValue();
            } else {
                result[i] = Long.parseLong(String.valueOf(value));
            }
        }
        return result;
    }

    private static int[] buildIntArray(JSONArray arr, int length) throws JSONException {
        int[] result = new int[length];
        for (int i = 0; i < length; i++) {
            Object value = arr.get(i);
            if (value instanceof Number) {
                result[i] = ((Number) value).intValue();
            } else {
                result[i] = Integer.parseInt(String.valueOf(value));
            }
        }
        return result;
    }

    private static String[] buildFallbackStringArray(JSONArray arr, int length) throws JSONException {
        String[] result = new String[length];
        for (int i = 0; i < length; i++) {
            result[i] = String.valueOf(arr.get(i));
        }
        return result;
    }
}
