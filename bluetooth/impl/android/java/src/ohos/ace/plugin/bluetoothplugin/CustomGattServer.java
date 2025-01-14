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

import ohos.ace.plugin.bluetoothplugin.BluetoothHelper;

import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattServer;
import android.bluetooth.BluetoothGattService;
import android.util.Log;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.HashMap;

public class CustomGattServer {
    private static final String LOG_TAG = "CustomGattServer";
    private static final int INVALID_HANDLE = -1;
    private static final int INTEGER_SIZE = 2;
    private static final int SUB_INDEX = 4;
    private static final int SUB_INDEX_END = 8;
    private static final String GATT_SERVICE_DIS = "180A";
    private static final String GATT_SERVICE_GATTS = "1800";
    private static final String GATT_SERVICE_GAP = "1801";

    private BluetoothGattServer gattServer_ = null;
    private int requestId_;
    private HashMap<Integer, Integer> handleInfoMap_ = new HashMap<>();
    private HashMap<Integer, BluetoothGattService> bluetoothGattServiceMap_ = new HashMap<>();
    private HashMap<Integer, BluetoothGattCharacteristic> BluetoothGattCharacteristicMap_ = new HashMap<>();
    private HashMap<Integer, BluetoothGattDescriptor> BluetoothGattDescriptorMap_ = new HashMap<>();
    private Lock bluetoothServerMapLock_ = new ReentrantLock();

    public CustomGattServer(BluetoothGattServer gattServer) {
        gattServer_ = gattServer;
    }

    public BluetoothGattServer getBluetoothGattServer() {
        return gattServer_;
    }

    public void addBluetoothGattService(int handle, BluetoothGattService bluetoothGattService) {
        bluetoothServerMapLock_.lock();
        if (bluetoothGattServiceMap_ != null) {
            bluetoothGattServiceMap_.put(handle, bluetoothGattService);
        }
        bluetoothServerMapLock_.unlock();
    }

    public void removeBluetoothGattService(int handle) {
        bluetoothServerMapLock_.lock();
        if (bluetoothGattServiceMap_ != null && handleInfoMap_ != null &&
            BluetoothGattCharacteristicMap_ != null && BluetoothGattDescriptorMap_ != null) {
            bluetoothGattServiceMap_.remove(handle);
            if (handleInfoMap_.containsKey(handle)) {
                for (int i = handle; i <= handleInfoMap_.get(handle); ++i) {
                    BluetoothGattCharacteristicMap_.remove(i);
                    BluetoothGattDescriptorMap_.remove(i);
                }
                handleInfoMap_.remove(handle);
            }
        }
        bluetoothServerMapLock_.unlock();
    }

    public BluetoothGattService getBluetoothGattService(int handle) {
        return (bluetoothGattServiceMap_ != null) ? bluetoothGattServiceMap_.get(handle) : null;
    }

    public void addBluetoothGattCharacter(int handle, BluetoothGattCharacteristic BluetoothGattCharacteristic) {
        bluetoothServerMapLock_.lock();
        if (BluetoothGattCharacteristicMap_ != null) {
            BluetoothGattCharacteristicMap_.put(handle, BluetoothGattCharacteristic);
        }
        bluetoothServerMapLock_.unlock();
    }

    public BluetoothGattCharacteristic getBluetoothGattCharacter(int handle) {
        return (BluetoothGattCharacteristicMap_ != null) ? BluetoothGattCharacteristicMap_.get(handle) : null;
    }

    public void addBluetoothGattDescriptor(int handle, BluetoothGattDescriptor descriptor) {
        bluetoothServerMapLock_.lock();
        if (BluetoothGattDescriptorMap_ != null) {
            BluetoothGattDescriptorMap_.put(handle, descriptor);
        }
        bluetoothServerMapLock_.unlock();
    }

    public BluetoothGattDescriptor getBluetoothGattDescriptor(int handle) {
        return (BluetoothGattDescriptorMap_ != null) ? BluetoothGattDescriptorMap_.get(handle) : null;
    }

    public void addBluetoothHandleInfo(int startHandle, int endHandle) {
        bluetoothServerMapLock_.lock();
        if (handleInfoMap_ != null) {
            handleInfoMap_.put(startHandle, endHandle);
        }
        bluetoothServerMapLock_.unlock();
    }

    public boolean serviceIsExist(String uuid) {
        if (bluetoothGattServiceMap_ != null) {
            for (HashMap.Entry<Integer, BluetoothGattService> entry : bluetoothGattServiceMap_.entrySet()) {
                String inputUuid = uuid.substring(SUB_INDEX, SUB_INDEX_END);
                String curUuid = entry.getValue().getUuid().toString().substring(SUB_INDEX, SUB_INDEX_END);
                if ((inputUuid.equalsIgnoreCase(GATT_SERVICE_DIS) && curUuid.equalsIgnoreCase(GATT_SERVICE_DIS)) ||
                    (inputUuid.equals(GATT_SERVICE_GATTS) && curUuid.equals(GATT_SERVICE_GATTS)) ||
                    (inputUuid.equals(GATT_SERVICE_GAP) && curUuid.equals(GATT_SERVICE_GAP))) {
                    return true;
                }
            }
        }
        return false;
    }

    public Integer[] getServiceHandleScopeByUuid(String uuid) {
        Integer[] result = new Integer[INTEGER_SIZE];
        if (bluetoothGattServiceMap_ != null) {
            for (HashMap.Entry<Integer, BluetoothGattService> entry : bluetoothGattServiceMap_.entrySet()) {
                if (entry.getValue().getUuid().toString().equalsIgnoreCase(uuid)) {
                    result[0] = entry.getKey();
                    result[1] = handleInfoMap_.get(result[0]);
                    Log.i(LOG_TAG, "HandleScope: " + result[0] + " -- " + result[1]);
                    return result;
                }
            }
        }
        return null;
    }

    public int getCharacteristicHandleByUuid(String serviceUuid, String characteristicUuid) {
        int startHandle = getServiceHandleScopeByUuid(serviceUuid)[0];
        int endHandle = getServiceHandleScopeByUuid(serviceUuid)[1];
        if (BluetoothGattCharacteristicMap_ != null) {
            for (int i = startHandle; i <= endHandle; i++) {
                if (BluetoothGattCharacteristicMap_.containsKey(i) &&
                    BluetoothGattCharacteristicMap_.get(i).getUuid().toString().equalsIgnoreCase(characteristicUuid)) {
                    Log.i(LOG_TAG, "Characteristic handle is " + i);
                    return i;
                }
            }
        }
        return INVALID_HANDLE;
    }

    public int getDescriptorHandleByUuid(String serviceUuid, String descriptorUuid) {
        int startHandle = getServiceHandleScopeByUuid(serviceUuid)[0];
        int endHandle = getServiceHandleScopeByUuid(serviceUuid)[1];
        if (BluetoothGattDescriptorMap_ != null) {
            for (int i = startHandle; i <= endHandle; i++) {
                if (BluetoothGattDescriptorMap_.containsKey(i) && 
                    BluetoothGattDescriptorMap_.get(i).getUuid().toString().equalsIgnoreCase(descriptorUuid)) {
                    Log.i(LOG_TAG, "Descriptor handle is " + i);
                    return i;
                }
            }
        }
        return INVALID_HANDLE;
    }

    public void setRequestId(int requestId) {
        requestId_ = requestId;
    }

    public int getRequestId() {
        return requestId_;
    }
}
