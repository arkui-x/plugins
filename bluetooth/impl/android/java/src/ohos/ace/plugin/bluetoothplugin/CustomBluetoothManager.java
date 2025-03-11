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

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattServer;
import android.bluetooth.BluetoothGattServerCallback;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.AdvertiseCallback;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.HashMap;

public class CustomBluetoothManager {
    private static final String LOG_TAG = "CustomBluetoothManager";
    private static final int NOT_FIND = 0;
    private static final Object INSTANCE_LOCK = new Object();
    private static volatile CustomBluetoothManager INSTANCE = null;

    private Context context_ = null;
    private BluetoothManager bluetoothManager_ = null;
    private BluetoothAdapter bluetoothAdapter_ = null;
    private BluetoothLeScanner bluetoothLeScanner_ = null;
    private CustomBluetoothScanCallback bluetoothScanCallback_ = new CustomBluetoothScanCallback();
    private HashMap<Integer, Integer> bluetoothAdvertisingStatusMap_ = new HashMap<>();
    private HashMap<Integer, AdvertiseCallback> bluetoothAdvertiseCallbackMap_ = new HashMap<>();
    private HashMap<Integer, CustomGattServer> bluetoothGattServerMap_ = new HashMap<>();
    private HashMap<Integer, BluetoothGattClient> bluetoothGattClientMap_ = new HashMap<Integer, BluetoothGattClient>();
    private HashMap<String, BluetoothDevice> bluetoothDeviceMap_ = new HashMap<String, BluetoothDevice>();
    private Lock bluetoothAdvertiserMapLock_ = new ReentrantLock();
    private Lock bluetoothDeviceMapLock_ = new ReentrantLock();
    private Lock bluetoothGattClientMapLock_ = new ReentrantLock();
    private Lock bluetoothGattServerMapLock_ = new ReentrantLock();

    private CustomBluetoothManager(Context context) {
        if (context == null) {
            Log.e(LOG_TAG, "Constructor failure, context is null");
            return;
        }
        try {
            context_ = context;
            bluetoothManager_ = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
            if (bluetoothManager_ == null) {
                Log.e(LOG_TAG, "Constructor failure, bluetoothManager_ is null");
            }
        } catch (IllegalStateException e) {
            Log.e(LOG_TAG, "Constructor failure, IllegalStateException err is " + e);
        }
    }

    /**
     * Get CustomBluetoothManager object.
     *
     * @return The CustomBluetoothManager object.
     */
    public static CustomBluetoothManager getInstance(Context context) {
        if (INSTANCE != null) {
            return INSTANCE;
        }

        synchronized (INSTANCE_LOCK) {
            if (INSTANCE != null) {
                return INSTANCE;
            }

            INSTANCE = new CustomBluetoothManager(context);
            return INSTANCE;
        }
    }

    public Context getContext() {
        return context_;
    }

    public BluetoothAdapter getBluetoothAdapter() {
        if (bluetoothAdapter_ != null) {
            return bluetoothAdapter_;
        } else {
            if (bluetoothManager_ != null) {
                bluetoothAdapter_ = bluetoothManager_.getAdapter();
                return bluetoothAdapter_;
            } else {
                Log.e(LOG_TAG, "getBluetoothAdapter failed, bluetoothManager is null");
                return null;
            }
        }
    }

    public BluetoothLeScanner getBluetoothLeScanner() {
        if (bluetoothLeScanner_ != null) {
            return bluetoothLeScanner_;
        } else {
            if (bluetoothAdapter_ != null && bluetoothAdapter_.isEnabled()) {
                bluetoothLeScanner_ = bluetoothAdapter_.getBluetoothLeScanner();
                return bluetoothLeScanner_;
            } else {
                Log.e(LOG_TAG, "getBluetoothLeScanner failed, bluetoothAdapter is error");
                return null;
            }
        }
    }

    public boolean bluetoothAdapterIsEnabled() {
        if (bluetoothAdapter_ == null) {
            return false;
        } else {
            return bluetoothAdapter_.isEnabled();
        }
    }

    public void addAdvertiseCallback(int advHandle, AdvertiseCallback advertiseCallback) {
        bluetoothAdvertiserMapLock_.lock();
        if (bluetoothAdvertiseCallbackMap_ != null) {
            bluetoothAdvertiseCallbackMap_.put(advHandle, advertiseCallback);
        }
        bluetoothAdvertiserMapLock_.unlock();
    }

    public AdvertiseCallback getAdvertiseCallback(int advHandle) {
        return (bluetoothAdvertiseCallbackMap_ != null) ? bluetoothAdvertiseCallbackMap_.get(advHandle) : null;
    }

    public void removeAdvertiseCallback(int advHandle) {
        bluetoothAdvertiserMapLock_.lock();
        if (bluetoothAdvertiseCallbackMap_ != null) {
            bluetoothAdvertiseCallbackMap_.remove(advHandle);
        }
        bluetoothAdvertiserMapLock_.unlock();
    }

    public void setAdvertisingStatus(int advHandle, int status) {
        bluetoothAdvertiserMapLock_.lock();
        if (bluetoothAdvertisingStatusMap_ != null) {
            bluetoothAdvertisingStatusMap_.put(advHandle, status);
        }
        bluetoothAdvertiserMapLock_.unlock();
    }

    public int getAdvertisingStatus(int advHandle) {
        return (bluetoothAdvertisingStatusMap_ != null && bluetoothAdvertisingStatusMap_.containsKey(advHandle))
                ? bluetoothAdvertisingStatusMap_.get(advHandle)
                : NOT_FIND;
    }

    public void removeAdvertisingStatus(int advHandle) {
        bluetoothAdvertiserMapLock_.lock();
        if (bluetoothAdvertisingStatusMap_ != null) {
            bluetoothAdvertisingStatusMap_.remove(advHandle);
        }
        bluetoothAdvertiserMapLock_.unlock();
    }

    public int registerBluetoothGattServer(int appId, Context context) {
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        if (bluetoothManager_ == null) {
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        }
        bluetoothGattServerMapLock_.lock();
        try {
            CustomGattServerCallback gattServerCallback = new CustomGattServerCallback();
            if (gattServerCallback == null) {
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            } else {
                gattServerCallback.setApplicationId(appId);
                BluetoothGattServer bluetoothGattServer = bluetoothManager_.openGattServer(context, gattServerCallback);
                CustomGattServer customGattServer;
                if (bluetoothGattServer == null || bluetoothGattServerMap_ == null ||
                   (customGattServer = new CustomGattServer(bluetoothGattServer)) == null) {
                    errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
                } else {
                    bluetoothGattServerMap_.put(appId, customGattServer);
                }
            }
            return errCode.getId();
        } catch (IllegalArgumentException e) {
            Log.e(LOG_TAG, "registerBluetoothGattServer failure, IllegalArgumentException err is " + e);
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        } finally {
            bluetoothGattServerMapLock_.unlock();
        }
    }

    public BluetoothGattServer getGattServer(int appId) {
        CustomGattServer customGattServer;
        return (bluetoothGattServerMap_ != null && (customGattServer = bluetoothGattServerMap_.get(appId)) != null)
                ? customGattServer.getBluetoothGattServer()
                : null;
    }

    public CustomGattServer getCustomGattServer(int appId) {
        return (bluetoothGattServerMap_ != null) ? bluetoothGattServerMap_.get(appId) : null;
    }

    public void removeBluetoothGattService(int appId, int handle) {
        CustomGattServer customGattServer;
        bluetoothGattServerMapLock_.lock();
        if (bluetoothGattServerMap_ != null && (customGattServer = bluetoothGattServerMap_.get(appId)) != null) {
            customGattServer.removeBluetoothGattService(handle);
        }
        bluetoothGattServerMapLock_.unlock();
    }

    public BluetoothGattService getBluetoothGattService(int appId, int handle) {
        CustomGattServer customGattServer;
        return (bluetoothGattServerMap_ != null && (customGattServer = bluetoothGattServerMap_.get(appId)) != null)
                ? customGattServer.getBluetoothGattService(handle)
                : null;
    }

    public void gattServerClose(int appId) {
        bluetoothGattServerMapLock_.lock();
        if (bluetoothGattServerMap_ != null) {
            bluetoothGattServerMap_.remove(appId);
        }
        bluetoothGattServerMapLock_.unlock();
    }

    public BluetoothGattCharacteristic getBluetoothGattCharacteristic(int appId, int characterHandle) {
        CustomGattServer customGattServer;
        return (bluetoothGattServerMap_ != null && (customGattServer = bluetoothGattServerMap_.get(appId)) != null)
                ? customGattServer.getBluetoothGattCharacter(characterHandle)
                : null;
    }

    public boolean addBluetoothGattClient(int appId, BluetoothGattClient bluetoothGattClient) {
        this.bluetoothGattClientMapLock_.lock();
        try {
            if (bluetoothGattClientMap_ != null && bluetoothGattClientMap_.containsKey(appId)) {
                Log.e(LOG_TAG, "The BluetoothGattClient Already exists, appId is " + String.valueOf(appId));
                return false;
            } else {
                bluetoothGattClientMap_.put(appId, bluetoothGattClient);
                Log.i(LOG_TAG, "The BluetoothGattClient add successfully, appId is " + String.valueOf(appId));
                return true;
            }
        } finally {
            this.bluetoothGattClientMapLock_.unlock();
        }
    }

    public BluetoothGattClient findBluetoothGattClient(int appId) {
        if (bluetoothGattClientMap_ != null) {
            return bluetoothGattClientMap_.get(appId);
        } else {
            return null;
        }
    }

    public void deleteBluetoothGattClient(int appId) {
        this.bluetoothGattClientMapLock_.lock();
        try {
            if (bluetoothGattClientMap_ != null && bluetoothGattClientMap_.containsKey(appId)) {
                bluetoothGattClientMap_.remove(appId);
            } else {
                Log.e(LOG_TAG, "delete BluetoothGattClient failed, appId is " + String.valueOf(appId));
            }
        } finally {
            this.bluetoothGattClientMapLock_.unlock();
        }
    }

    public BluetoothDevice findBluetoothDevice(String address) {
        this.bluetoothDeviceMapLock_.lock();
        try {
            if (bluetoothDeviceMap_.containsKey(address)) {
                return bluetoothDeviceMap_.get(address);
            } else {
                return null;
            }
        } finally {
            this.bluetoothDeviceMapLock_.unlock();
        }
    }

    public void addBluetoothDevice(String address, BluetoothDevice bluetoothDevice) {
        this.bluetoothDeviceMapLock_.lock();
        try {
            if (bluetoothDeviceMap_ != null) {
                bluetoothDeviceMap_.put(address, bluetoothDevice);
            } else {
                Log.e(LOG_TAG, "add BluetoothDevice failed.");
            }
        } finally {
            this.bluetoothDeviceMapLock_.unlock();
        }
    }

    public void deleteBluetoothDevice() {
        this.bluetoothDeviceMapLock_.lock();
        try {
            if (bluetoothDeviceMap_ != null && !bluetoothDeviceMap_.isEmpty()) {
                bluetoothDeviceMap_.clear();
            } else {
                Log.e(LOG_TAG, "delete BluetoothDevice failed.");
            }
        } finally {
            this.bluetoothDeviceMapLock_.unlock();
        }
    }

    public ScanCallback getScanCallback(int scannerId) {
        if (bluetoothScanCallback_ != null && bluetoothScanCallback_.checkScannerId(scannerId)) {
            bluetoothScanCallback_.setScannerId(scannerId);
        }
        return bluetoothScanCallback_;
    }

    public class CustomBluetoothScanCallback extends ScanCallback {
        private int scannerId_ = 0;

        public void setScannerId(int id) {
            scannerId_ = id;
        }

        public boolean checkScannerId(int id) {
            return (scannerId_ == id) ? false : true;
        }

        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            super.onScanResult(callbackType, result);
            BluetoothDevice bluetoothDevice = result.getDevice();
            if (bluetoothDevice.getName() != null) {
                if (!bluetoothDevice.getName().isEmpty() && !bluetoothDevice.getName().equalsIgnoreCase("null")) {
                    addBluetoothDevice(bluetoothDevice.getAddress(), bluetoothDevice);
                }
            }
            BluetoothPlugin.nativeOnScanResult(
                BluetoothHelper.encapsulationBluetoothLeScanResults(result), scannerId_);
        }

        @Override
        public void onScanFailed(int errorCode) {
            super.onScanFailed(errorCode);
            Log.e(LOG_TAG, "StartScan callback onScanFailed; errorCode is " + errorCode);
        }
    }

    public class CustomGattServerCallback extends BluetoothGattServerCallback {
        private int applicationId_ = 0;

        public void setApplicationId(int appId) {
            applicationId_ = appId;
        }

        @Override
        public void onMtuChanged(BluetoothDevice device, int mtu) {
            super.onMtuChanged(device, mtu);
            String deviceData = BluetoothHelper.convertBluetoothDeviceToJString(device);
            Intent intent = new Intent(BluetoothPlugin.BLE_SERVER_MTU_CHANGE);
            intent.putExtra("applicationId", applicationId_);
            intent.putExtra("device", deviceData);
            intent.putExtra("mtu", mtu);
            BluetoothPlugin.sendBleBroadcast(intent);
        }

        @Override
        public void onConnectionStateChange(BluetoothDevice device, int status, int newState) {
            super.onConnectionStateChange(device, status, newState);
            String deviceData = BluetoothHelper.convertBluetoothDeviceToJString(device);
            Intent intent = new Intent(BluetoothPlugin.BLE_SERVER_CONNECTION_STATE_CHANGE);
            intent.putExtra("applicationId", applicationId_);
            intent.putExtra("device", deviceData);
            intent.putExtra("state", newState);
            BluetoothPlugin.sendBleBroadcast(intent);
        }

        @Override
        public void onCharacteristicReadRequest(BluetoothDevice device, int requestId, int offset,
                BluetoothGattCharacteristic characteristic) {
            super.onCharacteristicReadRequest(device, requestId, offset, characteristic);
            if (bluetoothGattServerMap_ != null) {
                String deviceData = BluetoothHelper.convertBluetoothDeviceToJString(device);
                String serviceUuid = characteristic.getService().getUuid().toString();
                String characteristicUuid = characteristic.getUuid().toString();
                int characteristicHandle = bluetoothGattServerMap_.get(applicationId_).getCharacteristicHandleByUuid(
                    serviceUuid, characteristicUuid);
                String characteristicData = BluetoothHelper.convertCharacteristicToJString(
                    characteristic, characteristicHandle);
                bluetoothGattServerMap_.get(applicationId_).setRequestId(requestId);
                Intent intent = new Intent(BluetoothPlugin.BLE_SERVER_READ_CHARACTERISTIC);
                intent.putExtra("applicationId", applicationId_);
                intent.putExtra("device", deviceData);
                intent.putExtra("characteristic", characteristicData);
                BluetoothPlugin.sendBleBroadcast(intent);
            }
        }

        @Override
        public void onCharacteristicWriteRequest(BluetoothDevice device, int requestId,
                BluetoothGattCharacteristic characteristic, boolean preparedWrite, boolean responseNeeded,
                int offset, byte[] value) {
            super.onCharacteristicWriteRequest(device, requestId, characteristic, preparedWrite, responseNeeded,
                offset, value);
            if (bluetoothGattServerMap_ != null) {
                bluetoothGattServerMap_.get(applicationId_).setRequestId(requestId);
                String deviceData = BluetoothHelper.convertBluetoothDeviceToJString(device);
                String serviceUuid = characteristic.getService().getUuid().toString();
                String characteristicUuid = characteristic.getUuid().toString();
                int characteristicHandle = bluetoothGattServerMap_.get(applicationId_).getCharacteristicHandleByUuid(
                    serviceUuid, characteristicUuid);
                String characteristicData = BluetoothHelper.convertCharacteristicToJString(characteristic,
                    characteristicHandle);
                Intent intent = new Intent(BluetoothPlugin.BLE_SERVER_WRITE_CHARACTERISTIC);
                intent.putExtra("applicationId", applicationId_);
                intent.putExtra("device", deviceData);
                intent.putExtra("responseNeeded", responseNeeded ? 1 : 0);
                intent.putExtra("characteristic", characteristicData);
                BluetoothPlugin.sendBleBroadcast(intent);
            }
        }

        @Override
        public void onDescriptorReadRequest(BluetoothDevice device, int requestId,
                int offset, BluetoothGattDescriptor descriptor) {
            super.onDescriptorReadRequest(device, requestId, offset, descriptor);
            if (bluetoothGattServerMap_ != null) {
                String deviceData = BluetoothHelper.convertBluetoothDeviceToJString(device);
                String serviceUuid = descriptor.getCharacteristic().getService().getUuid().toString();
                String descriptorUuid = descriptor.getUuid().toString();
                int descriptorHandle = bluetoothGattServerMap_.get(applicationId_).getDescriptorHandleByUuid(
                    serviceUuid, descriptorUuid);
                String descriptorData = BluetoothHelper.convertDescriptorToJSONString(descriptor, descriptorHandle);
                bluetoothGattServerMap_.get(applicationId_).setRequestId(requestId);
                Intent intent = new Intent(BluetoothPlugin.BLE_SERVER_READ_DESCRIPTOR);
                intent.putExtra("applicationId", applicationId_);
                intent.putExtra("device", deviceData);
                intent.putExtra("descriptor", descriptorData);
                BluetoothPlugin.sendBleBroadcast(intent);
            }
        }

        @Override
        public void onDescriptorWriteRequest(BluetoothDevice device, int requestId,
                BluetoothGattDescriptor descriptor, boolean preparedWrite, boolean responseNeeded, int offset,
                byte[] value) {
            super.onDescriptorWriteRequest(device, requestId, descriptor, preparedWrite,
                responseNeeded, offset, value);
            if (bluetoothGattServerMap_ != null) {
                bluetoothGattServerMap_.get(applicationId_).setRequestId(requestId);
                String deviceData = BluetoothHelper.convertBluetoothDeviceToJString(device);
                String serviceUuid = descriptor.getCharacteristic().getService().getUuid().toString();
                String descriptorUuid = descriptor.getUuid().toString();
                int descriptorHandle = bluetoothGattServerMap_.get(applicationId_).getDescriptorHandleByUuid(
                    serviceUuid, descriptorUuid);
                String descriptorData = BluetoothHelper.convertDescriptorToJSONString(descriptor, descriptorHandle);
                Intent intent = new Intent(BluetoothPlugin.BLE_SERVER_WRITE_DESCRIPTOR);
                intent.putExtra("applicationId", applicationId_);
                intent.putExtra("device", deviceData);
                intent.putExtra("descriptor", descriptorData);
                BluetoothPlugin.sendBleBroadcast(intent);
            }
        }
    }
}
