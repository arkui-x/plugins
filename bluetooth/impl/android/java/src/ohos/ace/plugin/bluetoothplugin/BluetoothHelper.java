/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

package ohos.ace.plugin.bluetoothplugin;

import ohos.ace.plugin.bluetoothplugin.BluetoothErrorCode;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattServer;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.le.AdvertiseData;
import android.bluetooth.le.AdvertiseSettings;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.os.ParcelUuid;
import android.util.Log;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.HashMap;
import java.util.List;
import java.util.UUID;
import java.util.Vector;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

public class BluetoothHelper {
    private static final String LOG_TAG = "BluetoothHelper";
    private static final int INVALID_LENGTH = 0;
    private static final int CHARACTERISTIC_PROPERTIE_BROADCAST = 0x01;
    private static final int CHARACTERISTIC_PROPERTIE_READ = 0x02;
    private static final int CHARACTERISTIC_PROPERTIE_WRITE = 0x08;
    private static final int CHARACTERISTIC_PROPERTIE_NOTIFY = 0x10;
    private static final int CHARACTERISTIC_PROPERTIE_INDICATE = 0x20;
    private static final int CHARACTERISTIC_PROPERTIE_EXTENDED_PROPERTIES = 0x80;
    private static final String UUID_CHARACTERISTIC_EXTENDED_PROPERTIES = "00002900-0000-1000-8000-00805F9B34FB";
    private static final String UUID_CHARACTERISTIC_USER_DESCRIPTION = "00002901-0000-1000-8000-00805F9B34FB";
    private static final String UUID_CLIENT_CHARACTERISTIC_CONFIGURATION = "00002902-0000-1000-8000-00805F9B34FB";
    private static final String UUID_SERVER_CHARACTERISTIC_CONFIGURATION = "00002903-0000-1000-8000-00805F9B34FB";
    private static final String UUID_CHARACTERISTIC_FROMAT = "00002904-0000-1000-8000-00805F9B34FB";
    private static final String UUID_CHARACTERISTIC_AGGREGATE_FROMAT = "00002905-0000-1000-8000-00805F9B34FB";
    private static final int ANDROID_PERMISSION_READ = 1;
    private static final int ANDROID_PERMISSION_WRITE = 2;
    private static final int PERMISSION_READ = 1;
    private static final int PERMISSION_WRITE = 16;
    private static final int MAX_PERMISSIONS = 16;
    private static final int MIN_PERMISSIONS = 0;
    private static final int MAX_PROPERTIE = 255;
    private static final int MIN_PROPERTIE = 0;

    public static <T> T getValueFromJson(JSONObject jsonObject, String key, Class<T> valueType) {
        if (jsonObject == null) {
            Log.e(LOG_TAG, "jsonObject is null");
            return null;
        }
        try {
            if (jsonObject.has(key)) {
                Object value = jsonObject.get(key);
                if (valueType.isInstance(value)) {
                    return valueType.cast(value);
                } else if (valueType == Integer.class && value instanceof Number) {
                    return valueType.cast(((Number) value).intValue());
                } else if (valueType == Long.class && value instanceof Number) {
                    return valueType.cast(((Number) value).longValue());
                } else if (valueType == Double.class && value instanceof Number) {
                    return valueType.cast(((Number) value).doubleValue());
                } else if (valueType == Boolean.class && value instanceof Boolean) {
                    return valueType.cast(value);
                } else if (valueType == JSONArray.class && value instanceof JSONArray) {
                    return valueType.cast(value);
                } else if (valueType == String.class && value instanceof String) {
                    return valueType.cast(value);
                } else {
                    Log.e(LOG_TAG, "The given ValueType : " + key + " does not exist");
                    return null;
                }
            } else {
                Log.e(LOG_TAG, "JsonData do not has " + key);
                return null;
            }
        } catch (JSONException e) {
            Log.e(LOG_TAG, "CreateScanFilters failed; err is ", e);
            return null;
        }
    }

    public static byte[] convertJsonArrayToByteArray(JSONObject jsonObject, String key) {
        try {
            JSONArray dataJson = getValueFromJson(jsonObject, key, JSONArray.class);
            if (dataJson == null) {
                Log.e(LOG_TAG, key + " dataJson is null");
                return null;
            }
            byte[] data = new byte[dataJson.length()];
            for (int i = 0; i < dataJson.length(); i++) {
                data[i] = (byte) dataJson.getInt(i);
            }
            return data;
        } catch (JSONException | IllegalArgumentException e) {
            Log.e(LOG_TAG, "convertJsonArrayToByteArray failed; err is ", e);
            return null;
        }
    }

    public static ScanFilter getBluetoothScanFilter(String stringFilter) {
        String tempData = null;
        ScanFilter.Builder filterBuilder = new ScanFilter.Builder();
        try {
            JSONObject jsonObject = new JSONObject(stringFilter);
            String deviceName = getValueFromJson(jsonObject, "name", String.class);
            if (deviceName != null && !(deviceName.trim().isEmpty())) {
                filterBuilder.setDeviceName(deviceName);
            }
            String deviceAddress = getValueFromJson(jsonObject, "deviceId", String.class);
            if (deviceAddress != null && !(deviceAddress.trim().isEmpty())) {
                filterBuilder.setDeviceAddress(deviceAddress);
            }
            tempData = getValueFromJson(jsonObject, "serviceUuid", String.class);
            ParcelUuid serviceUuid = (tempData == null) ? null : ParcelUuid.fromString(tempData);
            tempData = getValueFromJson(jsonObject, "serviceUuidMask", String.class);
            ParcelUuid uuidMask = (tempData == null) ? null : ParcelUuid.fromString(tempData);
            if (serviceUuid != null || uuidMask != null) {
                filterBuilder.setServiceUuid(serviceUuid, uuidMask);
            }
            tempData = getValueFromJson(jsonObject, "serviceSolicitationUuid", String.class);
            ParcelUuid serviceSolicitationUuid = (tempData == null) ? null : ParcelUuid.fromString(tempData);
            tempData = getValueFromJson(jsonObject, "serviceSolicitationUuidMask", String.class);
            ParcelUuid serviceSolicitationUuidMask = (tempData == null) ? null : ParcelUuid.fromString(tempData);
            if (serviceSolicitationUuid != null || serviceSolicitationUuidMask != null) {
                Class<?> builderClass = Class.forName("android.bluetooth.le.ScanFilter$Builder");
                Method setServiceSolicitationUuid = builderClass.getMethod("setServiceSolicitationUuid",
                    ParcelUuid.class, ParcelUuid.class);
                setServiceSolicitationUuid.invoke(filterBuilder, serviceSolicitationUuid,
                    serviceSolicitationUuidMask);
            }
            int manufacturerId = getValueFromJson(jsonObject, "manufactureId", Integer.class);
            byte[] manufacturerData = convertJsonArrayToByteArray(jsonObject, "manufactureData");
            byte[] manufacturerDataMask = convertJsonArrayToByteArray(jsonObject, "manufactureDataMask");
            if (manufacturerId != 0 && (manufacturerData != null || manufacturerDataMask != null)) {
                filterBuilder.setManufacturerData(manufacturerId, manufacturerData, manufacturerDataMask);
            } else if (manufacturerId == 0 && (manufacturerData != null || manufacturerDataMask != null)) {
                return null;
            }
            byte[] serviceData = convertJsonArrayToByteArray(jsonObject, "serviceData");
            byte[] serviceDataMask = convertJsonArrayToByteArray(jsonObject, "serviceDataMask");
            if (serviceUuid != null || serviceData != null || serviceDataMask != null) {
                filterBuilder.setServiceData(serviceUuid, serviceData, serviceDataMask);
            }
            ScanFilter filter = filterBuilder.build();
            return filter;
        } catch (JSONException | NoSuchMethodException | IllegalAccessException | IllegalArgumentException |
            InvocationTargetException | SecurityException | ClassNotFoundException e) {
            Log.e(LOG_TAG, "createScanFilters failed; err is " + e.getClass().getSimpleName(), e);
            return null;
        }
    }

    public static List<ScanFilter> getBluetoothScanFilters(String[] stringFilters) {
        List<ScanFilter> filters = new ArrayList<>();
        for (String stringFilter : stringFilters) {
            ScanFilter filter = getBluetoothScanFilter(stringFilter);
            if (filter == null) {
                return null;
            }
            filters.add(filter);
        }
        return filters;
    }

    public static ScanSettings getBluetoothLeScanSettings(String stringSettings) {
        try {
            JSONObject jsonObject = new JSONObject(stringSettings);
            long reportDelayMillis = getValueFromJson(jsonObject, "reportDelayMillis", Long.class);
            int scanMode = getValueFromJson(jsonObject, "scanMode", Integer.class);
            boolean legacy = getValueFromJson(jsonObject, "legacy", Boolean.class);
            int phy = getValueFromJson(jsonObject, "phy", Integer.class);
            int callbackType = getValueFromJson(jsonObject, "callbackType", Integer.class);
            if (reportDelayMillis < 0 || 
               (phy != BluetoothDevice.PHY_LE_1M && phy != ScanSettings.PHY_LE_ALL_SUPPORTED) ||
               (scanMode < ScanSettings.SCAN_MODE_OPPORTUNISTIC || scanMode > ScanSettings.SCAN_MODE_LOW_LATENCY) ||
               (callbackType != ScanSettings.CALLBACK_TYPE_ALL_MATCHES &&
                callbackType != ScanSettings.CALLBACK_TYPE_FIRST_MATCH &&
                callbackType != ScanSettings.CALLBACK_TYPE_MATCH_LOST)) {
                return null;
            }

            ScanSettings settings = new ScanSettings.Builder()
                .setReportDelay(reportDelayMillis)
                .setScanMode(scanMode)
                .setLegacy(legacy)
                .setPhy(phy)
                .setCallbackType(callbackType)
                .build();
            return settings;
        } catch (JSONException | IllegalArgumentException e) {
            Log.e(LOG_TAG, "getBluetoothLeScanSettings failed; err is ", e);
            return null;
        }
    }

    public static String encapsulationBluetoothLeScanResults(ScanResult result) {
        try {
            String addr = result.getDevice().getAddress();
            if (addr == null || addr.isEmpty()) {
                addr = "";
            }
            int rssi = result.getRssi();
            boolean connectable = result.isConnectable();
            String name = result.getDevice().getName();
            if (name == null || name.isEmpty()) {
                name = "";
            }
            byte[] payload = result.getScanRecord().getBytes();
            JSONArray array = new JSONArray();
            for (int i = 0; i < payload.length; i++) {
                array.put(payload[i]);
            }

            JSONObject jsonObject = new JSONObject();
            jsonObject.put("addr", addr);
            jsonObject.put("rssi", rssi);
            jsonObject.put("connectable", connectable);
            jsonObject.put("name", name);
            jsonObject.put("payload", array);
            return jsonObject.toString();
        } catch (JSONException | IllegalArgumentException e) {
            Log.e(LOG_TAG, "encapsulationBluetoothLeScanResults failed; err is ", e);
            return "";
        }
    }

    public static String convertVectorToJString(Vector<String> vector) {
        JSONArray jsonArray = new JSONArray(vector);
        return jsonArray.toString();
    }

    public static AdvertiseSettings getBluetoothAdvertiseSettings(String advertiseStrings) {
        try {
            JSONObject jsonObject = new JSONObject(advertiseStrings);
            boolean connectable = getValueFromJson(jsonObject, "connectable", Boolean.class);
            AdvertiseSettings settings = new AdvertiseSettings.Builder()
                .setConnectable(connectable)
                .build();
            return settings;
        } catch (JSONException | IllegalArgumentException e) {
            Log.e(LOG_TAG, "getBluetoothAdvertiseSettings failed; err is ", e);
            return null;
        }
    }

    public static AdvertiseData getBluetoothAdvertiseData(String advDataStrings) {
        try {
            AdvertiseData.Builder dataBuilder = new AdvertiseData.Builder();
            JSONObject jsonObject = new JSONObject(advDataStrings);
            JSONArray serviceUuidArray = getValueFromJson(jsonObject, "serviceUuids", JSONArray.class);
            if (serviceUuidArray != null) {
                for (int i = 0; i < serviceUuidArray.length(); ++i) {
                    UUID uuid = UUID.fromString(serviceUuidArray.getString(i));
                    dataBuilder.addServiceUuid(new ParcelUuid(uuid));
                }
            }
            JSONArray manufactureDataArray = getValueFromJson(jsonObject, "manufactureData", JSONArray.class);
            if (manufactureDataArray != null) {
                for(int i = 0; i < manufactureDataArray.length(); ++i) {
                    int manufactureId = getValueFromJson(
                        manufactureDataArray.getJSONObject(i), "manufactureId", Integer.class);
                    byte[] manufactureValue = {};
                    if (manufactureDataArray.getJSONObject(i).has("manufactureValue")) {
                        manufactureValue = convertJsonArrayToByteArray(
                            manufactureDataArray.getJSONObject(i), "manufactureValue");
                    }
                    dataBuilder.addManufacturerData(manufactureId, manufactureValue);
                }
            }
            JSONArray ServiceDataArray = getValueFromJson(jsonObject, "serviceData", JSONArray.class);
            if (ServiceDataArray != null) {
                for(int i = 0; i < ServiceDataArray.length(); ++i) {
                    String serviceUuid = getValueFromJson(
                        ServiceDataArray.getJSONObject(i), "serviceUuid", String.class);
                    byte[] serviceValue = {};
                    if (ServiceDataArray.getJSONObject(i).has("serviceValue")) {
                        serviceValue = convertJsonArrayToByteArray(
                            ServiceDataArray.getJSONObject(i), "serviceValue");
                    }
                    UUID uuid = UUID.fromString(serviceUuid);
                    dataBuilder.addServiceData(new ParcelUuid(uuid), serviceValue);
                }
            }
            boolean includeDeviceName = getValueFromJson(jsonObject, "includeDeviceName", Boolean.class);
            dataBuilder.setIncludeDeviceName(includeDeviceName);
            return dataBuilder.build();
        } catch (JSONException | IllegalArgumentException e) {
            Log.e(LOG_TAG, "getBluetoothAdvertiseData failed, try-catch err is " + e);
            return null;
        }
    }

    public static boolean hasScanResponseData(String scanResponseStrings) {
        try {
            JSONObject jsonObject = new JSONObject(scanResponseStrings);
            JSONArray serviceUuidArray = getValueFromJson(jsonObject, "serviceUuids", JSONArray.class);
            JSONArray manufactureDataArray = getValueFromJson(jsonObject, "manufactureData", JSONArray.class);
            JSONArray ServiceDataArray = getValueFromJson(jsonObject, "serviceData", JSONArray.class);
            if (serviceUuidArray == null && manufactureDataArray == null && ServiceDataArray == null) {
                return false;
            }
            return true;
        } catch (JSONException e) {
            return false;
        }
    }

    public static BluetoothGattCharacteristic getBluetoothGattCharacteristic(String characteristicString,
        String serviceUuid, BluetoothGatt gatt) {
        try {
            JSONObject jsonObject = new JSONObject(characteristicString);
            String characteristicUuidString = getValueFromJson(jsonObject, "characteristicUuid", String.class);
            UUID characteristicUuid = UUID.fromString(characteristicUuidString);
            UUID uuid = UUID.fromString(serviceUuid);
            BluetoothGattService bluetoothGattService = gatt.getService(uuid);
            BluetoothGattCharacteristic bluetoothGattCharacteristic;
            return (bluetoothGattService == null ||
                   (bluetoothGattCharacteristic = bluetoothGattService.getCharacteristic(characteristicUuid)) == null)
                    ? null
                    : bluetoothGattCharacteristic;
        } catch (JSONException | IllegalArgumentException e) {
            Log.e(LOG_TAG, "getBluetoothGattCharacteristic failed, try-catch err is " + e);
            return null;
        }
    }

    public static byte[] getCharacteristicValue(String characteristicString) {
        try {
            byte[] characteristicValue = {};
            JSONObject jsonObject = new JSONObject(characteristicString);
            if (jsonObject.has("characteristicValue")) {
                characteristicValue = convertJsonArrayToByteArray(jsonObject, "characteristicValue");
            }
            return characteristicValue;
        } catch (JSONException e) {
            Log.e(LOG_TAG, "getCharacteristicValue failed, try-catch err is " + e);
            return null;
        }
    }

    public static String convertGattServiceToJSONString(BluetoothGattService service) {
        try {
            if (service == null) {
                Log.e(LOG_TAG, "BluetoothGattService Object to be converted is null");
                return "";
            }
            JSONObject jsonObject = new JSONObject();
            String uuid = service.getUuid().toString();
            int type = service.getType();

            jsonObject.put("uuid", uuid);
            jsonObject.put("type", type);
            return jsonObject.toString();
        } catch (JSONException e) {
            Log.e(LOG_TAG, "Json data encapsulation error; error is : ", e);
            return "";
        }
    }

    public static String convertBluetoothDeviceToJString(BluetoothDevice device) {
        if (device == null) {
            Log.e(LOG_TAG, "Device Object to be converted is null");
            return "";
        }
        try {
            String addr = device.getAddress();
            int transport = BluetoothDevice.TRANSPORT_LE;
            JSONObject jsonObject = new JSONObject();
            jsonObject.put("addr", addr);
            jsonObject.put("transport", transport);
            return jsonObject.toString();
        } catch (JSONException e) {
            Log.e(LOG_TAG, "Json data encapsulation error; error is : ", e);
            return "";
        }
    }

    public static String convertCharacteristicToJSONString(BluetoothGattCharacteristic characteristic) {
        try {
            if (characteristic == null) {
                Log.e(LOG_TAG, "Characteristic Object to be converted is null");
                return "";
            }
            JSONObject jsonObject = new JSONObject();
            String uuid = characteristic.getUuid().toString();
            int properties = characteristic.getProperties();
            int permissions = characteristic.getPermissions();
            byte[] value = {};
            int length = INVALID_LENGTH;
            if (characteristic.getValue() != null) {
                value = characteristic.getValue();
                length = value.length;
            }
            JSONArray array = new JSONArray();
            for (int i = 0; i < value.length; i++) {
                array.put(value[i]);
            }

            jsonObject.put("uuid", uuid);
            jsonObject.put("properties", properties);
            jsonObject.put("permissions", permissions);
            jsonObject.put("value", array);
            jsonObject.put("length", length);
            return jsonObject.toString();
        } catch (JSONException e) {
            Log.e(LOG_TAG, "Json data encapsulation error; error is : ", e);
            return "";
        }
    }

    public static String convertDescriptorToJSONString(BluetoothGattDescriptor descriptor) {
        try {
            if (descriptor == null) {
                Log.e(LOG_TAG, "descriptor Object to be converted is null.");
                return "";
            }
            JSONObject jsonObject = new JSONObject();
            UUID uuid = descriptor.getUuid();
            int permissions = descriptor.getPermissions();
            byte[] value = {};
            int length = INVALID_LENGTH;
            if (descriptor.getValue() != null) {
                value = descriptor.getValue();
                length = value.length;
            } else {
                Log.e(LOG_TAG, "descriptor value is null.");
            }
            JSONArray array = new JSONArray();
            for (int i = 0; i < value.length; i++) {
                array.put(value[i]);
            }

            jsonObject.put("uuid", uuid.toString());
            jsonObject.put("permissions", permissions);
            jsonObject.put("value", array);
            jsonObject.put("length", length);
            return jsonObject.toString();
        } catch (JSONException e) {
            Log.e(LOG_TAG, "Json data encapsulation error; error is : ", e);
            return "";
        }
    }

    public static String convertCharacteristicToJString(BluetoothGattCharacteristic characteristic, int handle) {
        try {
            if (characteristic == null) {
                Log.e(LOG_TAG, "Characteristic Object to be converted is null");
                return "";
            }
            JSONObject jsonObject = new JSONObject();
            String uuid = characteristic.getUuid().toString();
            byte[] value = {};
            int length = INVALID_LENGTH;
            if (characteristic.getValue() != null) {
                value = characteristic.getValue();
                length = value.length;
            }
            String valueString = new String(value, StandardCharsets.UTF_8);
            jsonObject.put("uuid", uuid);
            jsonObject.put("handle", handle);
            jsonObject.put("value", valueString);
            jsonObject.put("length", length);
            return jsonObject.toString();
        } catch (JSONException e) {
            Log.e(LOG_TAG, "Json data encapsulation error; error is : ", e);
            return "";
        }
    }

    public static String convertDescriptorToJSONString(BluetoothGattDescriptor descriptor, int handle) {
        try {
            if (descriptor == null) {
                Log.e(LOG_TAG, "Descriptor Object to be converted is null");
                return "";
            }
            JSONObject jsonObject = new JSONObject();
            String uuid = descriptor.getUuid().toString();
            int permissions = descriptor.getPermissions();
            byte[] value = {};
            int length = INVALID_LENGTH;
            if (descriptor.getValue() != null) {
                value = descriptor.getValue();
                length = value.length;
            }
            String valueString = new String(value, StandardCharsets.UTF_8);
            jsonObject.put("uuid", uuid);
            jsonObject.put("handle", handle);
            jsonObject.put("permissions", permissions);
            jsonObject.put("value", valueString);
            jsonObject.put("length", length);
            return jsonObject.toString();
        } catch (JSONException e) {
            Log.e(LOG_TAG, "convert descriptor To JSONString failed; err is ", e);
            return "";
        }
    }

    public static int addServiceToCustomGattServer(String jsonString, CustomGattServer structure) {
        try {
            JSONObject jsonObject = new JSONObject(jsonString);
            String serviceUuid = getValueFromJson(jsonObject, "serviceUuid", String.class);
            boolean isPrimary = getValueFromJson(jsonObject, "isPrimary", Boolean.class);
            int handle = getValueFromJson(jsonObject, "handle", Integer.class);
            int endHandle = getValueFromJson(jsonObject, "endHandle", Integer.class);
            JSONArray characterArray = getValueFromJson(jsonObject, "characteristics", JSONArray.class);
            UUID uuid = UUID.fromString(serviceUuid);
            if (!checkServiceJsonIsLegality(structure, serviceUuid, characterArray)) {
                Log.e(LOG_TAG, "addServiceToCustomGattServer failed; service Json is not legally");
                return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
            }
            BluetoothGattService service = new BluetoothGattService(uuid,
                (isPrimary ? BluetoothGattService.SERVICE_TYPE_PRIMARY : BluetoothGattService.SERVICE_TYPE_SECONDARY));
            int errCode = BluetoothErrorCode.BT_NO_ERROR.getId();
            if (service == null) {
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
            } else {
                HashMap<Integer, BluetoothGattCharacteristic> characteristicMap =
                getCharacteristicMap(characterArray, structure);
                if (characteristicMap == null) {
                    errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
                } else {
                    for (HashMap.Entry<Integer, BluetoothGattCharacteristic> entry : characteristicMap.entrySet()) {
                        Integer characteristicHandle = entry.getKey();
                        BluetoothGattCharacteristic characteristic = entry.getValue();
                        boolean success = service.addCharacteristic(characteristic);
                        if (success) {
                            structure.addBluetoothGattCharacter(characteristicHandle, characteristic);
                        }
                    }
                    structure.addBluetoothGattService(handle, service);
                    structure.addBluetoothHandleInfo(handle, endHandle);
                }
            }
            return errCode;
        } catch (JSONException | IllegalArgumentException e) {
            Log.e(LOG_TAG, "addServiceToCustomGattServer failed; err is ", e);
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        }
    }

    public static boolean checkServiceJsonIsLegality(CustomGattServer structure, String serviceUuid,
        JSONArray characterArray) {
        BluetoothGattServer bluetoothGattServer = structure.getBluetoothGattServer();
        if (bluetoothGattServer == null || structure.serviceIsExist(serviceUuid) ||
            !checkCharacterJsonIsLegality(characterArray)) {
            Log.e(LOG_TAG, "checkServiceJsonIsLegality failed, bluetoothGattServer is null or service is exist");
            return false;
        }
        return true;
    }

    public static boolean checkCharacterJsonIsLegality(JSONArray characterArray) {
        if (characterArray == null) {
            return true;
        }
        boolean res = true;
        try {
            for (int i = 0; i < characterArray.length(); ++i) {
                if (!characterArray.getJSONObject(i).has("characteristicValue")) {
                    res = false;
                    break;
                }
                int properties = getValueFromJson(characterArray.getJSONObject(i), "properties", Integer.class);
                int permissions = convertPermission(
                    getValueFromJson(characterArray.getJSONObject(i), "permissions", Integer.class));
                JSONArray descriptorArray =
                    getValueFromJson(characterArray.getJSONObject(i), "descriptors", JSONArray.class);
                if (properties > MAX_PROPERTIE || properties < MIN_PROPERTIE || permissions > MAX_PERMISSIONS ||
                    permissions < MIN_PERMISSIONS) {
                    res = false;
                    break;
                }
                if (((properties & CHARACTERISTIC_PROPERTIE_READ) != 0 &&
                     (permissions & ANDROID_PERMISSION_READ) == 0) ||
                    ((properties & CHARACTERISTIC_PROPERTIE_WRITE) != 0 &&
                     (permissions & ANDROID_PERMISSION_WRITE) == 0)) {
                    res = false;
                    break;
                } else if (!checkDescriptorJsonIsLegality(properties, descriptorArray)) {
                    res = false;
                    break;
                }
            }
            return res;
        } catch (JSONException e) {
            Log.e(LOG_TAG, "checkCharacterJsonIsLegality error; error is : ", e);
            return false;
        }
    }

    public static boolean checkDescriptorJsonIsLegality(int properties, JSONArray descriptorArray) {
        if (descriptorArray == null) {
            return true;
        }
        boolean res = true;
        try {
            for (int i = 0; i < descriptorArray.length(); ++i) {
                if (!descriptorArray.getJSONObject(i).has("descriptorValue")) {
                    res = false;
                    break;
                }
                List<String> descriptorUuidList = getDescriptorUuidList(descriptorArray);
                int permissions = convertPermission(
                    getValueFromJson(descriptorArray.getJSONObject(i), "permissions", Integer.class));
                if (permissions > MAX_PERMISSIONS || permissions < MIN_PERMISSIONS) {
                    res = false;
                    break;
                }
                if ((properties & CHARACTERISTIC_PROPERTIE_BROADCAST) != 0 &&
                    Collections.frequency(descriptorUuidList, UUID_SERVER_CHARACTERISTIC_CONFIGURATION) != 1) {
                    res = false;
                    break;
                }
                if (((properties & CHARACTERISTIC_PROPERTIE_NOTIFY) != 0 ||
                    (properties & CHARACTERISTIC_PROPERTIE_INDICATE) != 0) &&
                    Collections.frequency(descriptorUuidList, UUID_CLIENT_CHARACTERISTIC_CONFIGURATION) != 1) {
                    res = false;
                    break;
                }
                if ((properties & CHARACTERISTIC_PROPERTIE_EXTENDED_PROPERTIES) != 0 &&
                    Collections.frequency(descriptorUuidList, UUID_CHARACTERISTIC_EXTENDED_PROPERTIES) != 1) {
                    res = false;
                    break;
                }
                if (Collections.frequency(descriptorUuidList, UUID_CHARACTERISTIC_USER_DESCRIPTION) > 1) {
                    res = false;
                    break;
                }
                if ((Collections.frequency(descriptorUuidList, UUID_CHARACTERISTIC_FROMAT) > 1) &&
                    (Collections.frequency(descriptorUuidList, UUID_CHARACTERISTIC_AGGREGATE_FROMAT) != 1)) {
                    res = false;
                    break;
                }
            }
            return res;
        } catch (JSONException | NullPointerException e) {
            Log.e(LOG_TAG, "checkDescriptorJsonIsLegality error; error is : ", e);
            return false;
        }
    }

    public static int convertPermission(int permission) {
        int res = 0;
        if ((permission & PERMISSION_READ) != 0) {
            res |= ANDROID_PERMISSION_READ;
        }
        if ((permission & PERMISSION_WRITE) != 0) {
            res |= ANDROID_PERMISSION_WRITE;
        }
        return res;
    }

    public static HashMap<Integer, BluetoothGattCharacteristic> getCharacteristicMap(
        JSONArray characterArray, CustomGattServer structure) {
        HashMap<Integer, BluetoothGattCharacteristic> characterMap = new HashMap<>();
        if (characterArray == null || characterMap == null) {
            return characterMap;
        }
        try {
            for (int i = 0; i < characterArray.length(); ++i) {
                String characterUuidString = getValueFromJson(
                    characterArray.getJSONObject(i), "characteristicUuid", String.class);
                byte[] characteristicValue = {};
                if (characterArray.getJSONObject(i).has("characteristicValue")) {
                    characteristicValue = convertJsonArrayToByteArray(
                        characterArray.getJSONObject(i), "characteristicValue");
                }
                int characteristicHandle = getValueFromJson(
                    characterArray.getJSONObject(i), "characterHandle", Integer.class);
                int characteristicPermissions = getValueFromJson(
                    characterArray.getJSONObject(i), "permissions", Integer.class);
                JSONArray descriptorArray = getValueFromJson(
                    characterArray.getJSONObject(i), "descriptors", JSONArray.class);
                int properties = getValueFromJson(characterArray.getJSONObject(i), "properties", Integer.class);
                UUID characterUuid = UUID.fromString(characterUuidString);
                BluetoothGattCharacteristic characteristic =
                    new BluetoothGattCharacteristic(characterUuid, properties, characteristicPermissions);
                if (characteristic == null) {
                    continue;
                }
                characteristic.setValue(characteristicValue);
                HashMap<Integer, BluetoothGattDescriptor> descriptorMap;
                if (descriptorArray != null && (descriptorMap = getDescriptorMap(descriptorArray)) != null) {
                    for (HashMap.Entry<Integer, BluetoothGattDescriptor> entry : descriptorMap.entrySet()) {
                        Integer desHandle = entry.getKey();
                        BluetoothGattDescriptor descriptor = entry.getValue();
                        boolean success = characteristic.addDescriptor(descriptor);
                        if (success) {
                            structure.addBluetoothGattDescriptor(desHandle, descriptor);
                        }
                    }
                }
                characterMap.put(characteristicHandle, characteristic);
            }
            return characterMap;
        } catch (JSONException | IllegalArgumentException e) {
            Log.e(LOG_TAG, "getCharacteristicMap failed; err is ", e);
            return new HashMap<Integer, BluetoothGattCharacteristic>();
        }
    }

    public static HashMap<Integer, BluetoothGattDescriptor> getDescriptorMap(JSONArray descriptorArray) {
        HashMap<Integer, BluetoothGattDescriptor> descriptorMap = new HashMap<>();
        if (descriptorArray == null || descriptorMap == null) {
            return descriptorMap;
        }
        try {
            for (int index = 0; index < descriptorArray.length(); ++index) {
                String descriptorUuidString = getValueFromJson(
                    descriptorArray.getJSONObject(index), "descriptorUuid", String.class);
                byte[] descriptorValue = {};
                if (descriptorArray.getJSONObject(index).has("descriptorValue")) {
                    descriptorValue = convertJsonArrayToByteArray(
                        descriptorArray.getJSONObject(index), "descriptorValue");
                }
                int descriptorHandle = getValueFromJson(
                    descriptorArray.getJSONObject(index), "desHandle", Integer.class);
                int descriptorPermissions = getValueFromJson(
                    descriptorArray.getJSONObject(index), "permissions", Integer.class);
                UUID descriptorUuid = UUID.fromString(descriptorUuidString);
                BluetoothGattDescriptor descriptor =
                    new BluetoothGattDescriptor(descriptorUuid, descriptorPermissions);
                if (descriptor == null) {
                    continue;
                }
                descriptor.setValue(descriptorValue);
                descriptorMap.put(descriptorHandle, descriptor);
            }
            return descriptorMap;
        } catch (JSONException | IllegalArgumentException e) {
            Log.e(LOG_TAG, "getDescriptorMap failed; err is ", e);
            return new HashMap<Integer, BluetoothGattDescriptor>();
        }
    }

    public static List<String> getDescriptorUuidList(JSONArray descriptorArray) {
        List<String> descriptorUuidList = new ArrayList<>();
        if (descriptorArray == null || descriptorUuidList == null) {
            return descriptorUuidList;
        }
        try {
            for (int index = 0; index < descriptorArray.length(); ++index) {
                String descriptorUuidString = getValueFromJson(
                    descriptorArray.getJSONObject(index), "descriptorUuid", String.class);
                descriptorUuidList.add(descriptorUuidString);
            }
            return descriptorUuidList;
        } catch (JSONException e) {
            Log.e(LOG_TAG, "getDescriptorUuidList failed; err is ", e);
            return new ArrayList<String>();
        }
    }

}
