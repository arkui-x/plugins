/**
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

import java.util.HashMap;
import java.util.Map;
import java.io.Serializable;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.util.Log;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.UUID;

public class BluetoothGattClient {
    private static final String LOG_TAG = "BluetoothGattClient";
    private static final int OH_BT_STATUS_SUCCESS = 0;
    private static final int OH_BT_STATUS_FAIL = 1;
    private static final int WRITE_SUCCESS = 0;
    private static final int VERSION_TIRAMISU = 33;

    private BluetoothGatt bluetoothGatt_ = null;
    private int appId_ = 0;

    private BluetoothGattCallback gattCallback_ = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            super.onConnectionStateChange(gatt, status, newState);
            Log.i(LOG_TAG, "onConnectionStateChange status is " + status + ", newState is " + newState);
            if (status != 0 && gatt != null) {
                gatt.close();
            }
            Intent intent = new Intent(BluetoothPlugin.BLE_CLIENT_CONNECTION_STATE_CHANGE);
            intent.putExtra("appId", appId_);
            intent.putExtra("status", status);
            intent.putExtra("newState", newState);
            BluetoothPlugin.sendBleBroadcast(intent);
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            super.onServicesDiscovered(gatt, status);
            Log.i(LOG_TAG, "onServicesDiscovered enter.");
            Intent intent = new Intent(BluetoothPlugin.BLE_CLIENT_SERVICES_DISCOVERED);
            intent.putExtra("appId", appId_);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                for (BluetoothGattService service : gatt.getServices()) {
                    String serviceJSONString = BluetoothHelper.convertGattServiceToJSONString(service);
                    intent.putExtra("serviceJSONString", serviceJSONString);
                    HashMap<String, String[]> clientCharacteristicMap = new HashMap<>();
                    String serviceUuid = service.getUuid().toString();
                    List<String> characterStrings = new ArrayList<>();

                    for (BluetoothGattCharacteristic characteristic : service.getCharacteristics()) {
                        String characterJSONString = BluetoothHelper.convertCharacteristicToJSONString(characteristic);
                        characterStrings.add(characterJSONString);
                        intent.putStringArrayListExtra ("characterStrings", (ArrayList<String>)characterStrings);
                        List<String> descriptorStrings = new ArrayList<>();
                        for (BluetoothGattDescriptor descriptor : characteristic.getDescriptors()) {
                            String descriptorResult = BluetoothHelper.convertDescriptorToJSONString(descriptor);
                            descriptorStrings.add(descriptorResult);
                        }
                        clientCharacteristicMap.put(
                            characteristic.getUuid().toString(), descriptorStrings.toArray(new String[] {}));
                        intent.putExtra("clientCharacteristicMap", (Serializable)clientCharacteristicMap);
                    }
                    BluetoothPlugin.sendBleBroadcast(intent);
                }
            }
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            super.onCharacteristicRead(gatt, characteristic, status);
            Intent intent = new Intent(BluetoothPlugin.BLE_CLIENT_READ_CHARACTERISTIC);
            intent.putExtra("appId", appId_);
            String result = BluetoothHelper.convertCharacteristicToJSONString(characteristic);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                intent.putExtra("status", OH_BT_STATUS_SUCCESS);
            } else {
                intent.putExtra("status", OH_BT_STATUS_FAIL);
            }
            intent.putExtra("result", result);
            BluetoothPlugin.sendBleBroadcast(intent);
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            super.onCharacteristicWrite(gatt, characteristic, status);
            Intent intent = new Intent(BluetoothPlugin.BLE_CLIENT_WRITE_CHARACTERISTIC);
            intent.putExtra("appId", appId_);
            String result = BluetoothHelper.convertCharacteristicToJSONString(characteristic);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                intent.putExtra("status", OH_BT_STATUS_SUCCESS);
            } else {
                intent.putExtra("status", OH_BT_STATUS_FAIL);
            }
            intent.putExtra("result", result);
            BluetoothPlugin.sendBleBroadcast(intent);
        }

        @Override
        public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            super.onDescriptorRead(gatt, descriptor, status);
            Log.i(LOG_TAG, "onDescriptorRead enter.");
            Intent intent = new Intent(BluetoothPlugin.BLE_CLIENT_READ_DESCRIPTOR);
            intent.putExtra("appId", appId_);
            String result = BluetoothHelper.convertDescriptorToJSONString(descriptor);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                intent.putExtra("status", OH_BT_STATUS_SUCCESS);
            } else {
                intent.putExtra("status", OH_BT_STATUS_FAIL);
            }
            intent.putExtra("result", result);
            BluetoothPlugin.sendBleBroadcast(intent);
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            super.onDescriptorWrite(gatt, descriptor, status);
            Log.i(LOG_TAG, "onDescriptorWrite enter.");
            Intent intent = new Intent(BluetoothPlugin.BLE_CLIENT_WRITE_DESCRIPTOR);
            intent.putExtra("appId", appId_);
            String result = BluetoothHelper.convertDescriptorToJSONString(descriptor);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                intent.putExtra("status", OH_BT_STATUS_SUCCESS);
            } else {
                intent.putExtra("status", OH_BT_STATUS_FAIL);
            }
            intent.putExtra("result", result);
            BluetoothPlugin.sendBleBroadcast(intent);
        }

        @Override
        public void onMtuChanged(BluetoothGatt gatt, int mtu, int status) {
            super.onMtuChanged(gatt, mtu, status);
            Intent intent = new Intent(BluetoothPlugin.BLE_CLIENT_SET_MTU);
            intent.putExtra("appId", appId_);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.i(LOG_TAG, "MTU updated to: " + mtu);
                intent.putExtra("mtu", mtu);
                intent.putExtra("status", OH_BT_STATUS_SUCCESS);
            } else {
                Log.e(LOG_TAG, "MTU update failed: " + mtu);
                intent.putExtra("mtu", mtu);
                intent.putExtra("status", OH_BT_STATUS_FAIL);
            }
            BluetoothPlugin.sendBleBroadcast(intent);
        }
    };

    public BluetoothGattClient(int appId) {
        this.appId_ = appId;
    }

    public int connect(Context context, BluetoothDevice device) {
        int errorCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        if (context == null) {
            Log.e(LOG_TAG, "connect failed, context is null");
            return errorCode;
        }
        if (device == null) {
            Log.e(LOG_TAG, "connect failed, device is null");
            return errorCode;
        }
        try{
            bluetoothGatt_ = device.connectGatt(context, false, gattCallback_, BluetoothDevice.TRANSPORT_LE);
            if (bluetoothGatt_ == null) {
                Log.e(LOG_TAG, "connect failed, bluetoothGatt is null");
                errorCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
            } else {
                errorCode = BluetoothErrorCode.BT_NO_ERROR.getId();
            }
        } catch (IllegalArgumentException e) {
            Log.e(LOG_TAG, "connectGatt failed, error is " + e);
            errorCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        }
        return errorCode;
    }

    public int discoverServices() {
        if (bluetoothGatt_ != null) {
            if (bluetoothGatt_.discoverServices()) {
                return BluetoothErrorCode.BT_NO_ERROR.getId();
            } else {
                Log.e(LOG_TAG, "native discoverServices result is false");
                return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
            }
        } else {
            Log.e(LOG_TAG, "discoverServices failed, bluetoothGatt is null");
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        }
    }

    public int disconnect() {
        if (bluetoothGatt_ != null) {
            bluetoothGatt_.disconnect();
            return BluetoothErrorCode.BT_NO_ERROR.getId();
        } else {
            Log.e(LOG_TAG, "disconnect failed, bluetoothGatt is null");
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        }
    }

    public int close() {
        if (bluetoothGatt_ != null) {
            bluetoothGatt_.close();
            Log.e(LOG_TAG, "close " + BluetoothErrorCode.BT_NO_ERROR.getId());
            return BluetoothErrorCode.BT_NO_ERROR.getId();
        } else {
            Log.e(LOG_TAG, "close failed, bluetoothGatt is null");
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        }
    }

    public int requestExchangeMtu(int mtu) {
        if (bluetoothGatt_ == null) {
            Log.e(LOG_TAG, "requestExchangeMtu failed, bluetoothGatt_ is null");
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        } else {
            if (bluetoothGatt_.requestMtu(mtu)) {
                return BluetoothErrorCode.BT_NO_ERROR.getId();
            } else {
                Log.e(LOG_TAG, "native requestMtu failed.");
                return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
            }
        }
    }

    private BluetoothGattCharacteristic getCharacterForServices(String sUuidString, String cUuidString) {
        try {
            if (sUuidString != null && cUuidString != null) {
                UUID sUuid = UUID.fromString(sUuidString);
                BluetoothGattService service = bluetoothGatt_.getService(sUuid);
                if (service != null) {
                    UUID cUuid = UUID.fromString(cUuidString);
                    BluetoothGattCharacteristic characteristic = service.getCharacteristic(cUuid);
                    return characteristic;
                } else {
                    Log.e(LOG_TAG, "getCharacterForServices failed, the service is null.");
                }
            } else {
                Log.e(LOG_TAG, "getCharacterForServices failed, sUuidString or cUuidString is null.");
            }
        } catch (IllegalArgumentException e) {
            Log.e(LOG_TAG, "UUID fromString failed for getCharacterForServices, error is " + e);
        }
        return null;
    }

    public int readCharacter(String sUuidString, String cUuidString) {
        BluetoothGattCharacteristic characteristic = getCharacterForServices(sUuidString, cUuidString);
        if (characteristic == null) {
            Log.e(LOG_TAG, "readCharacter failed, characteristic is null.");
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        }
        if (bluetoothGatt_.readCharacteristic(characteristic)) {
            return BluetoothErrorCode.BT_NO_ERROR.getId();
        } else {
            Log.e(LOG_TAG, "native readCharacteristic failed.");
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        }
    }

    public int writeCharacter(String sUuidString, String cUuidString, byte[] value, int writeType) {
        BluetoothGattCharacteristic characteristic = getCharacterForServices(sUuidString, cUuidString);
        int errorCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        if (characteristic == null) {
            return errorCode;
        }
        if (Build.VERSION.SDK_INT >= VERSION_TIRAMISU) {
            try {
                Method writeCharacteristicMethod = BluetoothGatt.class.getMethod("writeCharacteristic",
                    BluetoothGattCharacteristic.class, byte[].class, int.class);
                errorCode = (int) writeCharacteristicMethod.invoke(bluetoothGatt_, characteristic, value, writeType);
                if (errorCode == WRITE_SUCCESS) {
                    errorCode = BluetoothErrorCode.BT_NO_ERROR.getId();
                } else {
                    Log.e(LOG_TAG, "native writeCharacter failed.");
                    errorCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
                }
            } catch (NoSuchMethodException e) {
                Log.e(LOG_TAG, "writeCharacter failed, NoSuchMethodException.");
                errorCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
            } catch (IllegalAccessException e) {
                Log.e(LOG_TAG, "writeCharacter failed, IllegalAccessException.");
                errorCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
            } catch (InvocationTargetException e) {
                Log.e(LOG_TAG, "writeCharacter failed, InvocationTargetException.");
                errorCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
            }
        } else {
            characteristic.setValue(value);
            characteristic.setWriteType(writeType);
            if (bluetoothGatt_.writeCharacteristic(characteristic)) {
                errorCode = BluetoothErrorCode.BT_NO_ERROR.getId();
            } else {
                Log.e(LOG_TAG, "native readCharacteristic failed.");
                errorCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
            }
        }
        return errorCode;
    }

    public int readDescriptor(String sUuidString, String cUuidString, String dUuidString) {
        BluetoothGattDescriptor descriptor = getDescriptorForServices(sUuidString, cUuidString, dUuidString);
        if (descriptor == null) {
            Log.e(LOG_TAG, "readDescriptor failed, descriptor is null.");
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        }
        if (bluetoothGatt_.readDescriptor(descriptor)) {
            return BluetoothErrorCode.BT_NO_ERROR.getId();
        } else {
            Log.e(LOG_TAG, "native readDescriptor failed.");
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        }
    }

    private BluetoothGattDescriptor getDescriptorForServices(
        String sUuidString, String cUuidString, String dUuidString) {
        if (sUuidString == null || cUuidString == null || dUuidString == null) {
            Log.e(LOG_TAG, "getDescriptorForServices failed, sUuidString or cUuidString or dUuidString is null.");
            return null;
        }
        try {
            UUID sUuid = UUID.fromString(sUuidString);
            BluetoothGattService service = bluetoothGatt_.getService(sUuid);
            if (service == null) {
                Log.e(LOG_TAG, "getDescriptorForServices failed, the service is null.");
                return null;
            }

            UUID cUuid = UUID.fromString(cUuidString);
            BluetoothGattCharacteristic characteristic = service.getCharacteristic(cUuid);
            if (characteristic == null) {
                Log.e(LOG_TAG, "getDescriptorForServices failed, the characteristic is null.");
                return null;
            }

            UUID dUuid = UUID.fromString(dUuidString);
            BluetoothGattDescriptor descriptor = characteristic.getDescriptor(dUuid);
            return descriptor;
        } catch (IllegalArgumentException e) {
            Log.e(LOG_TAG, "UUID fromString failed for getDescriptorForServices, error is " + e);
            return null;
        }
    }

    public int writeDescriptor(String sUuidString, String cUuidString, String dUuidString, byte[] value) {
        BluetoothGattDescriptor descriptor =  getDescriptorForServices(sUuidString, cUuidString, dUuidString);
        int errorCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        if (descriptor == null) {
            return errorCode;
        }
        if (Build.VERSION.SDK_INT >= VERSION_TIRAMISU) {
            try {
                Method writeDescriptorMethod = 
                    BluetoothGatt.class.getMethod("writeDescriptor", BluetoothGattDescriptor.class, byte[].class);
                errorCode = (int) writeDescriptorMethod.invoke(bluetoothGatt_, descriptor, value);
                if (errorCode == WRITE_SUCCESS) {
                    errorCode = BluetoothErrorCode.BT_NO_ERROR.getId();
                } else {
                    Log.e(LOG_TAG, "native writeDescriptor failed.");
                    errorCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
                }
            } catch (NoSuchMethodException e) {
                Log.e(LOG_TAG, "writeDescriptor failed, NoSuchMethodException.");
                errorCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
            } catch (IllegalAccessException e) {
                Log.e(LOG_TAG, "writeDescriptor failed, IllegalAccessException.");
                errorCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
            } catch (InvocationTargetException e) {
                Log.e(LOG_TAG, "writeDescriptor failed, InvocationTargetException.");
                errorCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
            }
        } else {
            descriptor.setValue(value);
            if (bluetoothGatt_.writeDescriptor(descriptor)) {
                errorCode = BluetoothErrorCode.BT_NO_ERROR.getId();
            } else {
                Log.e(LOG_TAG, "native writeDescriptor failed.");
                errorCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
            }
        }
        return errorCode;
    }
}
