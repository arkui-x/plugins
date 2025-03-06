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
import ohos.ace.plugin.bluetoothplugin.BluetoothGattClient;
import ohos.ace.plugin.bluetoothplugin.BluetoothHelper;
import ohos.ace.plugin.bluetoothplugin.CustomBluetoothManager;
import ohos.ace.plugin.bluetoothplugin.CustomGattServer;

import android.bluetooth.BluetoothA2dp;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothClass;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattServer;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.AdvertiseCallback;
import android.bluetooth.le.AdvertiseData;
import android.bluetooth.le.AdvertiseSettings;
import android.bluetooth.le.BluetoothLeAdvertiser;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanSettings;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.ParcelUuid;
import android.util.Log;
import java.util.HashMap;
import java.util.List;
import java.util.Set;
import java.util.Vector;

/**
 * BlueTooth android plugin module
 *
 */
public class BluetoothPlugin {
    private static final String LOG_TAG = "BluetoothPlugin";
    private static final int DEFAULT_OFFSET = 0;
    private static final int API_31 = 31;
    private static final int API_33 = 33;
    private static final Object Profile_Lock = new Object();
    private static CustomBluetoothManager btManager_ = null;
    private static BluetoothA2dp bluetoothA2dp_ = null;

    private BluetoothBroadcastReceive btBroadcastReceive_;
    private BluetoothProfile.ServiceListener btServiceListener_ = null;
    private Context mContext_ = null;

    // BroadcastReceiver handle method
    protected static final String BLE_ADVERTISER_RESULT = "nativeOnStartAdvertisingResult";
    protected static final String BLE_CLIENT_SERVICES_DISCOVERED = "onServicesDiscovered";
    protected static final String BLE_CLIENT_CONNECTION_STATE_CHANGE = "nativeGattClientSetServices";
    protected static final String BLE_CLIENT_READ_CHARACTERISTIC = "nativeOnCharacteristicRead";
    protected static final String BLE_CLIENT_WRITE_CHARACTERISTIC = "nativeOnCharacteristicWrite";
    protected static final String BLE_CLIENT_READ_DESCRIPTOR = "nativeOnDescriptorRead";
    protected static final String BLE_CLIENT_WRITE_DESCRIPTOR = "nativeOnDescriptorWrite";
    protected static final String BLE_CLIENT_SET_MTU = "nativeOnMtuChanged";
    protected static final String BLE_SERVER_MTU_CHANGE = "onMtuChanged";
    protected static final String BLE_SERVER_CONNECTION_STATE_CHANGE = "connectionStateChange";
    protected static final String BLE_SERVER_READ_CHARACTERISTIC = "characteristicRead";
    protected static final String BLE_SERVER_WRITE_CHARACTERISTIC = "characteristicWrite";
    protected static final String BLE_SERVER_READ_DESCRIPTOR = "descriptorRead";
    protected static final String BLE_SERVER_WRITE_DESCRIPTOR = "descriptorWrite";

    // Bluetooth permissions
    private static final String PERMISSION_BLUETOOTH = "android.permission.BLUETOOTH";
    private static final String PERMISSION_BLUETOOTH_ADMIN = "android.permission.BLUETOOTH_ADMIN";
    private static final String PERMISSION_ACCESS_COARSE_LOCATION = "android.permission.ACCESS_COARSE_LOCATION";
    private static final String PERMISSION_ACCESS_FINE_LOCATION = "android.permission.ACCESS_FINE_LOCATION";
    private static final String PERMISSION_BLUETOOTH_ADVERTISE = "android.permission.BLUETOOTH_ADVERTISE";
    private static final String PERMISSION_BLUETOOTH_CONNECT = "android.permission.BLUETOOTH_CONNECT";
    private static final String PERMISSION_BLUETOOTH_SCAN = "android.permission.BLUETOOTH_SCAN";

    /**
     * BluetoothPlugin
     *
     * @param context context of the application
     */
    public BluetoothPlugin(Context context) {
        mContext_ = context;
        btManager_ = CustomBluetoothManager.getInstance(context);
        nativeInit();
        registerBluetoothReceiver(mContext_);
        registerBluetoothServiceListener(mContext_);
    }

    private boolean checkPermissions(String... parameters) {
        if (mContext_ == null) {
            Log.e(LOG_TAG, "checkPermissions mContext_ is null.");
            return false;
        }
        int permissionResult = PackageManager.PERMISSION_DENIED;
        for (String permissions : parameters) {
            permissionResult = mContext_.checkCallingOrSelfPermission(permissions);
            if (permissionResult != PackageManager.PERMISSION_GRANTED) {
                Log.e(LOG_TAG, "Permission check failed, The permissions is " + permissions);
                return false;
            }
        }
        return true;
    }

    public boolean checkGattServerPermission() {
        boolean permissionRes = false;
        if (Build.VERSION.SDK_INT >= API_31) {
            permissionRes = checkPermissions(PERMISSION_BLUETOOTH, PERMISSION_BLUETOOTH_CONNECT);
        } else {
            permissionRes = checkPermissions(PERMISSION_BLUETOOTH, PERMISSION_BLUETOOTH_ADMIN);
        }
        return permissionRes;
    }

    public boolean checkGattClientPermission() {
        if (Build.VERSION.SDK_INT >= API_31) {
            return checkPermissions(PERMISSION_BLUETOOTH_CONNECT);
        } else {
            return checkPermissions(PERMISSION_BLUETOOTH, PERMISSION_BLUETOOTH_ADMIN);
        }
    }

    public int startScan(String stringSettings, String[] stringFilters, int scannerId) {
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        boolean permissionRes = false;
        if (Build.VERSION.SDK_INT >= API_31) {
            permissionRes = checkPermissions(PERMISSION_BLUETOOTH, PERMISSION_BLUETOOTH_SCAN);
        } else {
            boolean locationRes = checkPermissions(PERMISSION_ACCESS_COARSE_LOCATION) ||
                checkPermissions(PERMISSION_ACCESS_FINE_LOCATION);
            permissionRes = locationRes && checkPermissions(PERMISSION_BLUETOOTH, PERMISSION_BLUETOOTH_ADMIN);
        }
        if (btManager_ == null) {
            Log.e(LOG_TAG, "CustomBluetoothManager is null");
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        } else if (!permissionRes) {
            Log.e(LOG_TAG, "Permission check failed");
            return BluetoothErrorCode.BT_ERR_PERMISSION_FAILED.getId();
        }
        try {
            BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
            BluetoothLeScanner scanner = btManager_.getBluetoothLeScanner();
            if (bluetoothAdapter == null) {
                Log.e(LOG_TAG, "startScan failed, The device does not support Bluetooth");
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            } else if (!bluetoothAdapter.isEnabled()) {
                Log.e(LOG_TAG, "startScan failed, User not enabled Bluetooth switch");
                errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
            } else if (scanner == null) {
                Log.e(LOG_TAG, "startScan failed, BluetoothLeScanner is null");
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            } else {
                List<ScanFilter> filters = BluetoothHelper.getBluetoothScanFilters(stringFilters);
                ScanSettings settings = BluetoothHelper.getBluetoothLeScanSettings(stringSettings);
                if (filters == null || settings == null) {
                    Log.e(LOG_TAG, "startScan failed, Invalid Param");
                    errCode = BluetoothErrorCode.BT_ERR_INVALID_PARAM;
                } else {
                    if (filters.isEmpty()) {
                        filters = null;
                    }
                    btManager_.deleteBluetoothDevice();
                    scanner.startScan(filters, settings, btManager_.getScanCallback(scannerId));
                }
            }
        } catch (IllegalArgumentException | IllegalStateException e) {
            Log.e(LOG_TAG, "startScan failed, try-catch err is " + e);
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        } finally {
            return errCode.getId();
        }
    }

    public int stopScan(int scannerId) {
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        boolean permissionRes = false;
        try {
            if (Build.VERSION.SDK_INT >= API_31) {
                permissionRes = checkPermissions(PERMISSION_BLUETOOTH, PERMISSION_BLUETOOTH_SCAN);
            } else {
                boolean locationRes = checkPermissions(PERMISSION_ACCESS_COARSE_LOCATION) ||
                    checkPermissions(PERMISSION_ACCESS_FINE_LOCATION);
                permissionRes = locationRes && checkPermissions(PERMISSION_BLUETOOTH, PERMISSION_BLUETOOTH_ADMIN);
            }
            if (btManager_ == null) {
                Log.e(LOG_TAG, "stopBLEScan failed, CustomBluetoothManager is null");
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            } else if (!permissionRes) {
                Log.e(LOG_TAG, "Permission check failed");
                errCode = BluetoothErrorCode.BT_ERR_PERMISSION_FAILED;
            } else {
                BluetoothLeScanner scanner = btManager_.getBluetoothLeScanner();
                if (scanner == null) {
                    Log.e(LOG_TAG, "stopBLEScan failed, BluetoothLeScanner is null");
                    errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
                } else {
                    scanner.stopScan(btManager_.getScanCallback(scannerId));
                }
            }
            return errCode.getId();
        } catch (IllegalStateException e) {
            Log.e(LOG_TAG, "startScan failed, try-catch err is " + e);
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        }
    }

    public AdvertiseCallback createAdvertisingCallback(int advHandle) {
        AdvertiseCallback advertiseCallback = new AdvertiseCallback() {
            @Override
            public void onStartSuccess(AdvertiseSettings settingsInEffect) {
                super.onStartSuccess(settingsInEffect);
                Intent intent = new Intent(BluetoothPlugin.BLE_ADVERTISER_RESULT);
                intent.putExtra("result", BluetoothErrorCode.BT_NO_ERROR.getId());
                intent.putExtra("advHandle", advHandle);
                intent.putExtra("opCode", AdvOptCode.ADV_DEFAULT_CODE.ordinal());
                BluetoothPlugin.sendBleBroadcast(intent);
            }  
            @Override
            public void onStartFailure(int errorCode) {
                super.onStartFailure(errorCode);
                if (btManager_ != null) {
                    btManager_.removeAdvertisingStatus(advHandle);
                    btManager_.removeAdvertiseCallback(advHandle);
                }
                Intent intent = new Intent(BluetoothPlugin.BLE_ADVERTISER_RESULT);
                intent.putExtra("result", AdvertisingStatus.ADVERTISING_STATUS_INTERNAL.ordinal());
                intent.putExtra("advHandle", advHandle);
                intent.putExtra("opCode", AdvOptCode.ADV_FAILED_CODE.ordinal());
                BluetoothPlugin.sendBleBroadcast(intent);
            }
        };
        return advertiseCallback;
    }

    public void sendAdvertisingBroadcast(int advHandle) {
        Intent intent = new Intent(BluetoothPlugin.BLE_ADVERTISER_RESULT);
        intent.putExtra("result", AdvertisingStatus.ADVERTISING_STATUS_ALREADY_START.ordinal());
        intent.putExtra("advHandle", advHandle);
        intent.putExtra("opCode", AdvOptCode.ADV_DEFAULT_CODE.ordinal());
        BluetoothPlugin.sendBleBroadcast(intent);
    }

    public boolean checkAdvertisingPermission() {
        boolean permissionRes = false;
        if (Build.VERSION.SDK_INT >= API_31) {
            permissionRes = checkPermissions(PERMISSION_BLUETOOTH, PERMISSION_BLUETOOTH_ADVERTISE);
        } else {
            permissionRes = checkPermissions(PERMISSION_BLUETOOTH, PERMISSION_BLUETOOTH_ADMIN);
        }
        return permissionRes;
    }

    public int startAdvertising(String settingsStrings, String advDataStrings, String scanResponseStrings,
        int advHandle, boolean isRawData) {
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        if (btManager_ == null) {
            Log.e(LOG_TAG, "startAdvertising failed, CustomBluetoothManager is null");
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        }
        try {
            BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
            if (bluetoothAdapter == null) {
                Log.e(LOG_TAG, "startAdvertising failed, The device does not support Bluetooth");
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            } else if (!bluetoothAdapter.isEnabled()) {
                Log.e(LOG_TAG, "startAdvertising failed, User not enabled Bluetooth switch");
                errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
            } else if (btManager_.getAdvertisingStatus(advHandle) !=
                AdvertisingStatus.ADVERTISING_STATUS_NOT_START.ordinal()) {
                sendAdvertisingBroadcast(advHandle);
            } else {
                BluetoothLeAdvertiser bluetoothLeAdvertiser = bluetoothAdapter.getBluetoothLeAdvertiser();
                AdvertiseSettings settings = BluetoothHelper.getBluetoothAdvertiseSettings(settingsStrings);
                AdvertiseData advertiseData = BluetoothHelper.getBluetoothAdvertiseData(advDataStrings);
                AdvertiseData scanResponseData = BluetoothHelper.getBluetoothAdvertiseData(scanResponseStrings);
                AdvertiseCallback callback = createAdvertisingCallback(advHandle);
                if (bluetoothLeAdvertiser == null || settings == null || advertiseData == null ||
                    scanResponseData == null || callback == null) {
                    Log.e(LOG_TAG, "startAdvertising failed, Params is null");
                    errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
                } else {
                    btManager_.setAdvertisingStatus(advHandle, AdvertisingStatus.ADVERTISING_STATUS_START.ordinal());
                    btManager_.addAdvertiseCallback(advHandle, callback);
                    if (BluetoothHelper.hasScanResponseData(scanResponseStrings)) {
                        bluetoothLeAdvertiser.startAdvertising(settings, advertiseData, scanResponseData, callback);
                    } else {
                        bluetoothLeAdvertiser.startAdvertising(settings, advertiseData, callback);
                    }
                }
            }
        } catch (IllegalStateException | IllegalArgumentException e) {
            Log.e(LOG_TAG, "startAdvertising failed, try-catch err is " + e);
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        } finally {
            return errCode.getId();
        }
    }

    public int stopAdvertising(int advHandle) {
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        if (btManager_ == null) {
            Log.e(LOG_TAG, "stopAdvertising failed, CustomBluetoothManager is null");
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        }else if (!checkAdvertisingPermission()) {
            Log.e(LOG_TAG, "stopAdvertising failed, Permission check failed");
            return BluetoothErrorCode.BT_ERR_PERMISSION_FAILED.getId();
        }
        try {
            BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
            if (bluetoothAdapter == null) {
                Log.e(LOG_TAG, "stopAdvertising failed, The device does not support Bluetooth");
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            } else if (!bluetoothAdapter.isEnabled()) {
                Log.e(LOG_TAG, "startAdvertising failed, User not enabled Bluetooth switch");
                errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
            } else {
                BluetoothLeAdvertiser bluetoothLeAdvertiser = bluetoothAdapter.getBluetoothLeAdvertiser();
                AdvertiseCallback advertiseCallback = btManager_.getAdvertiseCallback(advHandle);
                if (bluetoothLeAdvertiser == null || advertiseCallback == null) {
                    Log.e(LOG_TAG, "startAdvertising failed, BluetoothLeAdvertiser or AdvertiseCallback is null");
                    errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
                } else {
                    bluetoothLeAdvertiser.stopAdvertising(advertiseCallback); 
                    btManager_.removeAdvertiseCallback(advHandle);
                    btManager_.removeAdvertisingStatus(advHandle);
                }
            }
        } catch (IllegalArgumentException e) {
            Log.e(LOG_TAG, "stopAdvertising failed, try-catch err is " + e);
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        } finally {
            return errCode.getId();
        }
    }

    public int startPair(String address) {
        if (Build.VERSION.SDK_INT < API_31) {
            if (!checkPermissions(PERMISSION_BLUETOOTH_ADMIN)) {
                Log.e(LOG_TAG, "startPair permission failed" + PERMISSION_BLUETOOTH_ADMIN);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        } else {
            if (!checkPermissions(PERMISSION_BLUETOOTH_CONNECT)) {
                Log.e(LOG_TAG, "startPair permission failed" + PERMISSION_BLUETOOTH_CONNECT);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        }

        if (btManager_ == null) {
            Log.e(LOG_TAG, "startPair failed, btManager_ is null");
            return (BluetoothErrorCode.BT_ERR_INTERNAL_ERROR).getId();
        }
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "startPair failed, The device does not support Bluetooth");
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "startPair failed, User not enabled Bluetooth switch");
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
        } else {
            BluetoothDevice device = bluetoothAdapter.getRemoteDevice(address);
            if (device == null) {
                Log.e(LOG_TAG, "startPair failed, BluetoothDevice is null");
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            } else {
                errCode = device.createBond() ? BluetoothErrorCode.BT_NO_ERROR
                        : BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            }
        }
        return errCode.getId();
    }

    public int getBtProfileConnState(int profileId, int[] retCode) {
        if (Build.VERSION.SDK_INT < API_31) {
            if (!checkPermissions(PERMISSION_BLUETOOTH)) {
                Log.e(LOG_TAG, "getBtProfileConnState permission failed" + PERMISSION_BLUETOOTH);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        } else {
            if (!checkPermissions(PERMISSION_BLUETOOTH_CONNECT)) {
                Log.e(LOG_TAG, "getBtProfileConnState permission failed" + PERMISSION_BLUETOOTH_CONNECT);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        }

        if (btManager_ == null) {
            Log.e(LOG_TAG, "getBtProfileConnState failed, btManager_ is null");
            return (BluetoothErrorCode.BT_ERR_INTERNAL_ERROR).getId();
        }
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "getBtProfileConnState failed, The device does not support Bluetooth");
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "getBtProfileConnState failed, User not enabled Bluetooth switch");
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
        } else {
            retCode[0] = bluetoothAdapter.getProfileConnectionState(profileId);
        }
        return errCode.getId();
    }

    public int getBtProfilesConnState(int[] profileId, int[] retCode) {
        if (Build.VERSION.SDK_INT < API_31) {
            if (!checkPermissions(PERMISSION_BLUETOOTH)) {
                Log.e(LOG_TAG, "getBtProfilesConnState permission failed" + PERMISSION_BLUETOOTH);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        } else {
            if (!checkPermissions(PERMISSION_BLUETOOTH_CONNECT)) {
                Log.e(LOG_TAG, "getBtProfilesConnState permission failed" + PERMISSION_BLUETOOTH_CONNECT);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        }

        if (btManager_ == null) {
            Log.e(LOG_TAG, "getBtProfilesConnState failed, btManager_ is null");
            return (BluetoothErrorCode.BT_ERR_INTERNAL_ERROR).getId();
        }
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "getBtProfileConnState failed, The device does not support Bluetooth");
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "getBtProfileConnState failed, User not enabled Bluetooth switch");
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
        } else {
            for (int index = 0; index < profileId.length; index++) {
                retCode[0] = bluetoothAdapter.getProfileConnectionState(profileId[index]);
                if (BluetoothAdapter.STATE_CONNECTED == retCode[0]) {
                    break;
                } else {
                    retCode[0] = BluetoothAdapter.STATE_DISCONNECTED;
                }
            }
        }
        return errCode.getId();
    }

    public int getDeviceName(String address, String[] name) {
        if (Build.VERSION.SDK_INT < API_31) {
            if (!checkPermissions(PERMISSION_BLUETOOTH)) {
                Log.e(LOG_TAG, "getDeviceName permission failed" + PERMISSION_BLUETOOTH);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        } else {
            if (!checkPermissions(PERMISSION_BLUETOOTH_CONNECT)) {
                Log.e(LOG_TAG, "getDeviceName permission failed" + PERMISSION_BLUETOOTH_CONNECT);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        }

        if (btManager_ == null) {
            Log.e(LOG_TAG, "getBtProfilesConnState failed, btManager_ is null");
            return (BluetoothErrorCode.BT_ERR_INTERNAL_ERROR).getId();
        }
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "getDeviceName failed, The device does not support Bluetooth");
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "getDeviceName failed, User not enabled Bluetooth switch");
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
        } else {
            BluetoothDevice device = bluetoothAdapter.getRemoteDevice(address);
            if (device == null) {
                Log.e(LOG_TAG, "getDeviceName failed, BluetoothDevice is null");
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            } else {
                name[0] = device.getName();
                if (name[0] == null) {
                    errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
                }
            }
        }
        return errCode.getId();
    }

    public int getLocalName(String[] name) {
        if (Build.VERSION.SDK_INT < API_31) {
            if (!checkPermissions(PERMISSION_BLUETOOTH)) {
                Log.e(LOG_TAG, "getLocalName permission failed" + PERMISSION_BLUETOOTH);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        } else {
            if (!checkPermissions(PERMISSION_BLUETOOTH_CONNECT)) {
                Log.e(LOG_TAG, "getLocalName permission failed" + PERMISSION_BLUETOOTH_CONNECT);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        }

        if (btManager_ == null) {
            Log.e(LOG_TAG, "getLocalName failed, btManager_ is null");
            return (BluetoothErrorCode.BT_ERR_INTERNAL_ERROR).getId();
        }
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "getLocalName failed, The device does not support Bluetooth");
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "getLocalName failed, User not enabled Bluetooth switch");
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
        } else {
            name[0] = bluetoothAdapter.getName();
            if (name[0] == null) {
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            }
        }
        return errCode.getId();
    }

    public int getPairedDevices(String[] pairedAddr) {
        if (Build.VERSION.SDK_INT < API_31) {
            if (!checkPermissions(PERMISSION_BLUETOOTH)) {
                Log.e(LOG_TAG, "getPairedDevices permission failed" + PERMISSION_BLUETOOTH);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        } else {
            if (!checkPermissions(PERMISSION_BLUETOOTH_CONNECT)) {
                Log.e(LOG_TAG, "getPairedDevices permission failed" + PERMISSION_BLUETOOTH_CONNECT);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        }

        if (btManager_ == null) {
            Log.e(LOG_TAG, "getPairedDevices failed, btManager_ is null");
            return (BluetoothErrorCode.BT_ERR_INTERNAL_ERROR).getId();
        }
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "getPairedDevices failed, The device does not support Bluetooth");
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "getPairedDevices failed, User not enabled Bluetooth switch");
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
        } else {
            Set<BluetoothDevice> pairedDevices = bluetoothAdapter.getBondedDevices();
            if (pairedDevices == null) {
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            } else {
                Vector<String> vector = new Vector<>();
                for (BluetoothDevice device : pairedDevices) {
                    vector.add(device.getAddress());
                }
                pairedAddr[0] = BluetoothHelper.convertVectorToJString(vector);
            }
        }
        return errCode.getId();
    }

    public int getPairState(String address, int[] pairState) {
        if (Build.VERSION.SDK_INT < API_31) {
            if (!checkPermissions(PERMISSION_BLUETOOTH)) {
                Log.e(LOG_TAG, "getPairState permission failed" + PERMISSION_BLUETOOTH);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        } else {
            if (!checkPermissions(PERMISSION_BLUETOOTH_CONNECT)) {
                Log.e(LOG_TAG, "getPairState permission failed" + PERMISSION_BLUETOOTH_CONNECT);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        }

        if (btManager_ == null) {
            Log.e(LOG_TAG, "getPairState failed, btManager_ is null");
            return (BluetoothErrorCode.BT_ERR_INTERNAL_ERROR).getId();
        }
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "getPairState failed, The device does not support Bluetooth");
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "getPairState failed, User not enabled Bluetooth switch");
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
        } else {
            BluetoothDevice device = bluetoothAdapter.getRemoteDevice(address);
            if (device == null) {
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            } else {
                pairState[0] = device.getBondState();
            }
        }
        return errCode.getId();
    }

    public int getDeviceClass(String address, int[] code) {
        if (Build.VERSION.SDK_INT < API_31) {
            if (!checkPermissions(PERMISSION_BLUETOOTH)) {
                Log.e(LOG_TAG, "getDeviceClass permission failed" + PERMISSION_BLUETOOTH);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        } else {
            if (!checkPermissions(PERMISSION_BLUETOOTH_CONNECT)) {
                Log.e(LOG_TAG, "getDeviceClass permission failed" + PERMISSION_BLUETOOTH_CONNECT);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        }

        if (btManager_ == null) {
            Log.e(LOG_TAG, "getDeviceClass failed, btManager_ is null");
            return (BluetoothErrorCode.BT_ERR_INTERNAL_ERROR).getId();
        }
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "getDeviceClass failed, The device does not support Bluetooth");
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "getDeviceClass failed, User not enabled Bluetooth switch");
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
        } else {
            BluetoothDevice device = bluetoothAdapter.getRemoteDevice(address);
            if (device == null) {
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            } else {
                BluetoothClass deviceClass = device.getBluetoothClass();
                if (deviceClass == null) {
                    errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
                    Log.e(LOG_TAG, "BluetoothPlugin JAVA getDeviceClass is null");
                } else {
                    code[0] = deviceClass.getDeviceClass();
                }
            }
        }
        return errCode.getId();
    }

    public int getDeviceUuids(String address, String[] uuids) {
        if (Build.VERSION.SDK_INT < API_31) {
            if (!checkPermissions(PERMISSION_BLUETOOTH)) {
                Log.e(LOG_TAG, "getDeviceUuids permission failed" + PERMISSION_BLUETOOTH);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        } else {
            if (!checkPermissions(PERMISSION_BLUETOOTH_CONNECT)) {
                Log.e(LOG_TAG, "getDeviceUuids permission failed" + PERMISSION_BLUETOOTH_CONNECT);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        }

        if (btManager_ == null) {
            Log.e(LOG_TAG, "getDeviceUuids failed, btManager_ is null");
            return (BluetoothErrorCode.BT_ERR_INTERNAL_ERROR).getId();
        }
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "getDeviceUuids failed, The device does not support Bluetooth");
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "getDeviceUuids failed, User not enabled Bluetooth switch");
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
        } else {
            BluetoothDevice device = bluetoothAdapter.getRemoteDevice(address);
            if (device == null) {
                Log.e(LOG_TAG, "getDeviceUuids failed, device is null");
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            } else {
                ParcelUuid[] uuidsArray = device.getUuids();
                if (uuidsArray == null) {
                    Log.e(LOG_TAG, "getDeviceUuids failed, return uuidsArray is null");
                    errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
                } else {
                    Vector<String> vector = new Vector<>();
                    for (ParcelUuid uuid : uuidsArray) {
                        vector.add(uuid.toString().toUpperCase());
                    }
                    uuids[0] = BluetoothHelper.convertVectorToJString(vector);
                }
            }
        }
        return errCode.getId();
    }

    public int enableBt() {
        if (Build.VERSION.SDK_INT >= API_33) {
            Log.e(LOG_TAG, "enableBt API_33 not support!");
            return (BluetoothErrorCode.BT_ERR_CAPABILITY_NOT_SUPPORT).getId();
        }

        if (btManager_ == null) {
            Log.e(LOG_TAG, "enableBt btManager_ is null!");
            return (BluetoothErrorCode.BT_ERR_INTERNAL_ERROR).getId();
        }
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "enableBt bluetoothAdapter is null!");
            return errCode.getId();
        }

        if (checkPermissions(PERMISSION_BLUETOOTH_ADMIN)) {
            if (Build.VERSION.SDK_INT >= API_31) {
                if (!checkPermissions(PERMISSION_BLUETOOTH_CONNECT)) {
                    Log.e(LOG_TAG, " enableBt permission failed");
                    return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
                }
            }
        } else {
            Log.e(LOG_TAG, "enableBt permission failed");
            return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
        }

        if (bluetoothAdapter.enable()) {
            return (BluetoothErrorCode.BT_NO_ERROR).getId();
        }

        Log.e(LOG_TAG, "enableBt return false!");
        return errCode.getId();
    }

    public int disableBt() {
        if (Build.VERSION.SDK_INT >= API_33) {
            Log.e(LOG_TAG, "disableBt API_33 not support!");
            return (BluetoothErrorCode.BT_ERR_CAPABILITY_NOT_SUPPORT).getId();
        }

        if (checkPermissions(PERMISSION_BLUETOOTH_ADMIN)) {
            if (Build.VERSION.SDK_INT >= API_31) {
                if (!checkPermissions(PERMISSION_BLUETOOTH_CONNECT)) {
                    Log.e(LOG_TAG, " disableBt permission failed");
                    return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
                }
            }
        } else {
            Log.e(LOG_TAG, "disableBt permission failed");
            return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
        }

        if (btManager_ == null) {
            Log.e(LOG_TAG, "disableBt btManager_ is null!");
            return (BluetoothErrorCode.BT_ERR_INTERNAL_ERROR).getId();
        }
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "disableBt bluetoothAdapter is null!");
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            return errCode.getId();
        }

        bluetoothAdapter.disable();
        return errCode.getId();
    }

    public int getBtState(int[] state) {
        if (Build.VERSION.SDK_INT < API_31) {
            if (!checkPermissions(PERMISSION_BLUETOOTH)) {
                Log.e(LOG_TAG, " getBtState permission failed");
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        }

        if (btManager_ == null) {
            Log.e(LOG_TAG, "getBtState btManager_ is null!");
            return (BluetoothErrorCode.BT_ERR_INTERNAL_ERROR).getId();
        }
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "getBtState bluetoothAdapter is null!");
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            return errCode.getId();
        }

        state[0] = bluetoothAdapter.getState();
        Log.i(LOG_TAG, "getBtState state: " + Integer.toString(state[0]));
        return errCode.getId();
    }

    public boolean isBtEnable() {
        if (Build.VERSION.SDK_INT < API_31) {
            if (!checkPermissions(PERMISSION_BLUETOOTH)) {
                Log.e(LOG_TAG, " isBtEnable permission failed");
                return false;
            }
        }

        if (btManager_ == null) {
            Log.e(LOG_TAG, "isBtEnable btManager_ is null!");
            return false;
        }
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "isBtEnable bluetoothAdapter is null!");
            return false;
        }

        return bluetoothAdapter.isEnabled();
    }

    public int setLocalName(String name) {
        if (btManager_ == null) {
            Log.e(LOG_TAG, "setLocalName btManager_ is null!");
            return (BluetoothErrorCode.BT_ERR_INTERNAL_ERROR).getId();
        }
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "setLocalName bluetoothAdapter is null!");
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            return errCode.getId();
        }

        if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "setLocalName failed, User not enabled Bluetooth switch");
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
            return errCode.getId();
        }

        if (bluetoothAdapter.setName(name)) {
            Log.i(LOG_TAG, "setLocalName setName success. name: " + name);
            return errCode.getId();
        }
        errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        return errCode.getId();
    }

    public int getBtScanMode(int[] scanMode) {
        if (Build.VERSION.SDK_INT < API_31) {
            if (!checkPermissions(PERMISSION_BLUETOOTH)) {
                Log.e(LOG_TAG, "getBtScanMode permission failed" + PERMISSION_BLUETOOTH);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        } else {
            if (!checkPermissions(PERMISSION_BLUETOOTH_SCAN)) {
                Log.e(LOG_TAG, "getBtScanMode permission failed" + PERMISSION_BLUETOOTH_SCAN);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        }

        if (btManager_ == null) {
            Log.e(LOG_TAG, "getBtScanMode btManager_ is null!");
            return (BluetoothErrorCode.BT_ERR_INTERNAL_ERROR).getId();
        }
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "getBtScanMode bluetoothAdapter is null!");
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            return errCode.getId();
        }

        if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "getBtScanMode getBtScanMode failed, User not enabled Bluetooth switch");
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
            return errCode.getId();
        }

        scanMode[0] = bluetoothAdapter.getScanMode();
        Log.i(LOG_TAG, "getBtScanMode scanmode: " + Integer.toString(scanMode[0]));
        return errCode.getId();
    }

    public int startBtDiscovery() {
        if (Build.VERSION.SDK_INT < API_31) {
            if (!checkPermissions(PERMISSION_BLUETOOTH_ADMIN, PERMISSION_ACCESS_FINE_LOCATION)) {
                Log.e(LOG_TAG, "startBtDiscovery permission failed" + PERMISSION_BLUETOOTH_ADMIN);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        } else {
            if (!checkPermissions(PERMISSION_BLUETOOTH_SCAN, PERMISSION_ACCESS_FINE_LOCATION)) {
                Log.e(LOG_TAG, "startBtDiscovery permission failed" + PERMISSION_BLUETOOTH_SCAN);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        }

        if (btManager_ == null) {
            Log.e(LOG_TAG, "startBtDiscovery btManager_ is null!");
            return (BluetoothErrorCode.BT_ERR_INTERNAL_ERROR).getId();
        }
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "startBtDiscovery bluetoothAdapter is null!");
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            return errCode.getId();
        }

        if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "startBtDiscovery failed, User not enabled Bluetooth switch");
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
            return errCode.getId();
        }

        bluetoothAdapter.cancelDiscovery();
        if (bluetoothAdapter.startDiscovery()) {
            return errCode.getId();
        }
        Log.e(LOG_TAG, "startBtDiscovery faild");
        errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        return errCode.getId();
    }

    public int cancelBtDiscovery() {
        if (Build.VERSION.SDK_INT < API_31) {
            if (!checkPermissions(PERMISSION_BLUETOOTH_ADMIN)) {
                Log.e(LOG_TAG, "cancelBtDiscovery permission failed" + PERMISSION_BLUETOOTH_ADMIN);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        } else {
            if (!checkPermissions(PERMISSION_BLUETOOTH_SCAN)) {
                Log.e(LOG_TAG, "cancelBtDiscovery permission failed" + PERMISSION_BLUETOOTH_SCAN);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        }

        if (btManager_ == null) {
            Log.e(LOG_TAG, "cancelBtDiscovery btManager_ is null!");
            return (BluetoothErrorCode.BT_ERR_INTERNAL_ERROR).getId();
        }
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "cancelBtDiscovery bluetoothAdapter is null!");
            return errCode.getId();
        }

        if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "cancelBtDiscovery cancelBtDiscovery failed, User not enabled Bluetooth switch");
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
            return errCode.getId();
        }

        if (bluetoothAdapter.cancelDiscovery()) {
            return (BluetoothErrorCode.BT_NO_ERROR).getId();
        }
        Log.e(LOG_TAG, "cancelBtDiscovery faild");
        return errCode.getId();
    }

    public int isBtDiscovering(boolean[] isDisCovering) {
        if (Build.VERSION.SDK_INT < API_31) {
            if (!checkPermissions(PERMISSION_BLUETOOTH_ADMIN)) {
                Log.e(LOG_TAG, "isBtDiscovering permission failed" + PERMISSION_BLUETOOTH_ADMIN);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        } else {
            if (!checkPermissions(PERMISSION_BLUETOOTH_SCAN)) {
                Log.e(LOG_TAG, "isBtDiscovering permission failed" + PERMISSION_BLUETOOTH_SCAN);
                return (BluetoothErrorCode.BT_ERR_PERMISSION_FAILED).getId();
            }
        }

        if (btManager_ == null) {
            Log.e(LOG_TAG, "isBtDiscovering btManager_ is null!");
            return (BluetoothErrorCode.BT_ERR_INTERNAL_ERROR).getId();
        }
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "isBtDiscovering bluetoothAdapter is null!");
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            return errCode.getId();
        }

        if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "isBtDiscovering failed, User not enabled Bluetooth switch");
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
            return errCode.getId();
        }

        isDisCovering[0] = bluetoothAdapter.isDiscovering();
        return errCode.getId();
    }

    public void registerBluetoothReceiver(Context context) {
        if (btBroadcastReceive_ == null) {
            btBroadcastReceive_ = new BluetoothBroadcastReceive();
        }

        IntentFilter itFilter = new IntentFilter();
        if (itFilter == null) {
            Log.e(LOG_TAG, "registerBluetoothReceiver itFilter is null!");
            btBroadcastReceive_ = null;
            return;
        }
        itFilter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        itFilter.addAction("android.bluetooth.BluetoothAdapter.STATE_OFF");
        itFilter.addAction("android.bluetooth.BluetoothAdapter.STATE_ON");
        itFilter.addAction("android.bluetooth.BluetoothAdapter.STATE_TURNING_OFF");
        itFilter.addAction("android.bluetooth.BluetoothAdapter.STATE_TURNING_ON");
        itFilter.addAction(BluetoothDevice.ACTION_FOUND);
        itFilter.addAction("android.bluetooth.device.action.FOUND");
        itFilter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
        itFilter.addAction("android.bluetooth.device.action.BOND_STATE_CHANGED");
        itFilter.addAction(BluetoothA2dp.ACTION_CONNECTION_STATE_CHANGED);
        itFilter.addAction(BLE_CLIENT_SERVICES_DISCOVERED);
        itFilter.addAction(BLE_CLIENT_CONNECTION_STATE_CHANGE);
        itFilter.addAction(BLE_CLIENT_READ_CHARACTERISTIC);
        itFilter.addAction(BLE_CLIENT_WRITE_CHARACTERISTIC);
        itFilter.addAction(BLE_CLIENT_READ_DESCRIPTOR);
        itFilter.addAction(BLE_CLIENT_WRITE_DESCRIPTOR);
        itFilter.addAction(BLE_CLIENT_SET_MTU);
        itFilter.addAction(BLE_SERVER_MTU_CHANGE);
        itFilter.addAction(BLE_SERVER_CONNECTION_STATE_CHANGE);
        itFilter.addAction(BLE_SERVER_READ_CHARACTERISTIC);
        itFilter.addAction(BLE_SERVER_WRITE_CHARACTERISTIC);
        itFilter.addAction(BLE_SERVER_READ_DESCRIPTOR);
        itFilter.addAction(BLE_SERVER_WRITE_DESCRIPTOR);
        itFilter.addAction(BLE_ADVERTISER_RESULT);
        context.registerReceiver(btBroadcastReceive_, itFilter);
    }

    public void unregisterBluetoothReceiver(Context context) {
        if (btBroadcastReceive_ != null) {
            context.unregisterReceiver(btBroadcastReceive_);
            btBroadcastReceive_ = null;
        }
    }

    public static void sendBleBroadcast(Intent intent) {
        Log.i(LOG_TAG, "sendBleBroadcast enter.");
        if (btManager_ == null) {
            Log.e(LOG_TAG, "sendBleBroadcast btManager_ is null!");
            return;
        }
        Context context = btManager_.getContext();
        if (context != null) {
            context.sendBroadcast(intent);
        } else {
            Log.e(LOG_TAG, "sendBleBroadcast failed. context is null");
        }
    }

    public void registerBluetoothServiceListener(Context context)
    {
        if (btManager_ == null) {
            Log.e(LOG_TAG, "registerBluetoothServiceListener btManager_ is null!");
            return;
        }
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "registerBluetoothServiceListener bluetoothAdapter is null!");
            return;
        }
        btServiceListener_ = new BluetoothProfile.ServiceListener() {
            @Override
            public void onServiceConnected(int profile, BluetoothProfile service) {
                if (profile == BluetoothProfile.A2DP) {
                    synchronized (Profile_Lock) {
                        bluetoothA2dp_ = (BluetoothA2dp) service;
                    }
                }
            }

            @Override
            public void onServiceDisconnected(int profile) {
                if (profile == BluetoothProfile.A2DP) {
                    synchronized (Profile_Lock) {
                        bluetoothA2dp_ = null;
                    }
                }
            }
        };
        if (!checkPermissions(PERMISSION_BLUETOOTH)) {
            btServiceListener_ = null;
            Log.e(LOG_TAG, "registerBluetoothServiceListener Permission check failed");
            return;
        }
        if (!bluetoothAdapter.getProfileProxy(context, btServiceListener_, BluetoothProfile.A2DP)) {
            Log.e(LOG_TAG, "registerBluetoothServiceListener getProfileProxy error!");
            btServiceListener_ = null;
        }
    }

    public void unregisterBluetoothServiceListener(Context context) {
        if (btManager_ == null) {
            Log.e(LOG_TAG, "unregisterBluetoothServiceListener btManager_ is null!");
            return;
        }
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "unregisterBluetoothServiceListener bluetoothAdapter is null!");
            return;
        }

        synchronized (Profile_Lock) {
            if (bluetoothA2dp_ != null) {
                bluetoothAdapter.closeProfileProxy(BluetoothProfile.A2DP, bluetoothA2dp_);
            }      
            bluetoothA2dp_ = null;
        }
    }

    private BluetoothErrorCode getA2dpConnectionState(BluetoothDevice device, int[] state) {
        if (Build.VERSION.SDK_INT >= API_31) {
            if (!checkPermissions(PERMISSION_BLUETOOTH_CONNECT)) {
                Log.e(LOG_TAG, "getA2dpConnectionState permission failed" + PERMISSION_BLUETOOTH_CONNECT);
                return BluetoothErrorCode.BT_ERR_PERMISSION_FAILED; 
            }
        }

        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        int stateTemp = BluetoothProfile.STATE_DISCONNECTED;
        synchronized (Profile_Lock) {
            if (bluetoothA2dp_ != null) {
                stateTemp = bluetoothA2dp_.getConnectionState(device);
                state[0] = stateTemp;
            } else {
                Log.e(LOG_TAG, "getA2dpConnectionState bluetoothA2dp_ is null!");
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            }
        }
        return errCode;
    }

    private BluetoothErrorCode getA2dpConnectedDevices(BluetoothAdapter bluetoothAdapter, String[] devices) {
        if (Build.VERSION.SDK_INT >= API_31) {
            if (!checkPermissions(PERMISSION_BLUETOOTH_CONNECT)) {
                Log.e(LOG_TAG, "getA2dpConnectedDevices permission failed" + PERMISSION_BLUETOOTH_CONNECT);
                return BluetoothErrorCode.BT_ERR_PERMISSION_FAILED; 
            }
        }

        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        Vector<String> vector = new Vector<>();
        synchronized (Profile_Lock) {
            if (bluetoothA2dp_ != null) {
                List<BluetoothDevice> deviceList = bluetoothA2dp_.getConnectedDevices();
                for (BluetoothDevice device : deviceList) {
                    vector.add(device.getAddress());
                }
                devices[0] = BluetoothHelper.convertVectorToJString(vector);
            } else {
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            }
        }
        return errCode;
    }

    public int getConnectionState(String address, int[] state) {
        if (btManager_ == null) {
            Log.e(LOG_TAG, "getConnectionState btManager_ is null!");
            return (BluetoothErrorCode.BT_ERR_INTERNAL_ERROR).getId();
        }
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "getConnectionState failed, The device does not support Bluetooth");
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "getConnectionState failed, User not enabled Bluetooth switch");
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
        } else {
            BluetoothDevice device = bluetoothAdapter.getRemoteDevice(address);
            if (device == null) {
                Log.e(LOG_TAG, "getConnectionState failed, device is null");
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            } else {
                errCode = getA2dpConnectionState(device, state);
            }
        }
        return errCode.getId();
    }

    public int getDevicesByStates(String[] devices) {
        if (btManager_ == null) {
            Log.e(LOG_TAG, "getDevicesByStates btManager_ is null!");
            return (BluetoothErrorCode.BT_ERR_INTERNAL_ERROR).getId();
        }
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null || btServiceListener_ == null) {
            Log.e(LOG_TAG, "getDevicesByStates failed, The device does not support Bluetooth");
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "getDevicesByStates failed, User not enabled Bluetooth switch");
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
        } else {
            errCode = getA2dpConnectedDevices(bluetoothAdapter, devices);
        }
        return errCode.getId();
    }

    public int registerApplicationGattServer(int appId) {
        int errCode = BluetoothErrorCode.BT_NO_ERROR.getId();
        if (btManager_ == null) {
            Log.e(LOG_TAG, "CustomBluetoothManager is null");
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        } else if (!checkGattServerPermission()) {
            Log.e(LOG_TAG, "Permission check failed");
            return BluetoothErrorCode.BT_ERR_PERMISSION_FAILED.getId();
        } 
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "registerApplicationGattServer failed, The device does not support Bluetooth");
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "registerApplicationGattServer failed, User not enabled Bluetooth switch");
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE.getId();
        } else {
            errCode = btManager_.registerBluetoothGattServer(appId, mContext_);
        }
        return errCode;
    }

    public int addService(int appId, int serviceHandle, String services) {
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        if (btManager_ == null) {
            Log.e(LOG_TAG, "addService failed, CustomBluetoothManager is null");
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        } else if (!checkGattServerPermission()) {
            Log.e(LOG_TAG, "Permission check failed");
            return BluetoothErrorCode.BT_ERR_PERMISSION_FAILED.getId();
        }
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "addService failed, The device does not support Bluetooth");
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "addService failed, User not enabled Bluetooth switch");
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
        } else {
            CustomGattServer structure = btManager_.getCustomGattServer(appId);
            if (structure == null || BluetoothHelper.addServiceToCustomGattServer(services, structure) !=
                BluetoothErrorCode.BT_NO_ERROR.getId()) {
                return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
            }
            BluetoothGattServer bluetoothGattServer = structure.getBluetoothGattServer();
            BluetoothGattService service = structure.getBluetoothGattService(serviceHandle);
            if (service == null || bluetoothGattServer == null) {
                Log.e(LOG_TAG, "addService failed, BluetoothGattServer or BluetoothGattService is null");
                structure.removeBluetoothGattService(serviceHandle);
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            } else {
                boolean ret = bluetoothGattServer.addService(service);
                if (!ret) {
                    structure.removeBluetoothGattService(serviceHandle);
                }
                errCode = ret ? BluetoothErrorCode.BT_NO_ERROR
                                : BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            }
        }
        return errCode.getId();
    }

    public int removeService(int appId, int handle) {
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        if (btManager_ == null) {
            Log.e(LOG_TAG, "CustomBluetoothManager is null");
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        } else if (!checkGattServerPermission()) {
            Log.e(LOG_TAG, "Permission check failed");
            return BluetoothErrorCode.BT_ERR_PERMISSION_FAILED.getId();
        }
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "removeService failed, The device does not support Bluetooth");
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "removeService failed, User not enabled Bluetooth switch");
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
        } else {
            BluetoothGattService service = btManager_.getBluetoothGattService(appId, handle);
            BluetoothGattServer bluetoothGattServer = btManager_.getGattServer(appId);
            if (service == null || bluetoothGattServer == null) {
                Log.e(LOG_TAG, "removeService failed, BluetoothGattServer or BluetoothGattService is null");
                return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
            }
            boolean ret = bluetoothGattServer.removeService(service);
            if (ret) {
                btManager_.removeBluetoothGattService(appId, handle);
            }
            errCode = ret ? BluetoothErrorCode.BT_NO_ERROR : BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        }
        return errCode.getId();
    }

    public int gattServerClose(int appId) {
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        if (btManager_ == null) {
            Log.e(LOG_TAG, "gattServerClose failed, CustomBluetoothManager is null");
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        } else if (!checkGattServerPermission()) {
            Log.e(LOG_TAG, "Permission check failed");
            return BluetoothErrorCode.BT_ERR_PERMISSION_FAILED.getId();
        }
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "gattServerClose failed, The device does not support Bluetooth");
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "gattServerClose failed, User not enabled Bluetooth switch");
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
        } else {
            BluetoothGattServer bluetoothGattServer = btManager_.getGattServer(appId);
            if (bluetoothGattServer == null) {
                Log.e(LOG_TAG, "gattServerClose failed, BluetoothGattServer is null");
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            } else {
                bluetoothGattServer.close();
                btManager_.gattServerClose(appId);
                errCode = BluetoothErrorCode.BT_NO_ERROR;
            }
        }
        return errCode.getId();
    }

    public int respondCharacteristicRead(String address, String value, int appId, int status, int length) {
        Log.i(LOG_TAG, "AppId is " + appId);
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        if (btManager_ == null) {
            Log.e(LOG_TAG, "CustomBluetoothManager is null");
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        } else if (!checkGattServerPermission()) {
            Log.e(LOG_TAG, "Permission check failed");
            return BluetoothErrorCode.BT_ERR_PERMISSION_FAILED.getId();
        }
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        } else if (!bluetoothAdapter.isEnabled()) {
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
        } else {
            CustomGattServer customGattServer = btManager_.getCustomGattServer(appId);
            BluetoothDevice bluetoothDevice = bluetoothAdapter.getRemoteDevice(address);
            if (customGattServer != null && bluetoothDevice != null) {
                BluetoothGattServer bluetoothGattServer = customGattServer.getBluetoothGattServer();
                if (bluetoothGattServer != null) {
                    int requestId = customGattServer.getRequestId();
                    bluetoothGattServer.sendResponse(bluetoothDevice, requestId, status, DEFAULT_OFFSET,
                        value.getBytes());
                } else {
                    errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
                }
            } else {
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            }
        }
        return errCode.getId();
    }

    public int respondCharacteristicWrite(String address, int appId, int status, int handle) {
        Log.i(LOG_TAG, "AppId is " + appId);
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        if (btManager_ == null) {
            Log.e(LOG_TAG, "CustomBluetoothManager is null");
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        } else if (!checkGattServerPermission()) {
            Log.e(LOG_TAG, "Permission check failed");
            return BluetoothErrorCode.BT_ERR_PERMISSION_FAILED.getId();
        }
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        } else if (!bluetoothAdapter.isEnabled()) {
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
        } else {
            CustomGattServer customGattServer = btManager_.getCustomGattServer(appId);
            BluetoothDevice bluetoothDevice = bluetoothAdapter.getRemoteDevice(address);
            if (customGattServer != null && bluetoothDevice != null) {
                BluetoothGattServer bluetoothGattServer = customGattServer.getBluetoothGattServer();
                BluetoothGattCharacteristic characteristic = customGattServer.getBluetoothGattCharacter(handle);
                if (bluetoothGattServer != null && characteristic != null) {
                    int requestId = customGattServer.getRequestId();
                    bluetoothGattServer.sendResponse(bluetoothDevice, requestId, status, DEFAULT_OFFSET,
                        characteristic.getValue());
                } else {
                    errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
                }
            } else {
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            }
        }
        return errCode.getId();
    }

    public int respondDescriptorRead(String address, String value, int appId, int status, int length) {
        Log.i(LOG_TAG, "AppId is " + appId);
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        if (btManager_ == null) {
            Log.e(LOG_TAG, "CustomBluetoothManager is null");
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        } else if (!checkGattServerPermission()) {
            Log.e(LOG_TAG, "Permission check failed");
            return BluetoothErrorCode.BT_ERR_PERMISSION_FAILED.getId();
        }
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        } else if (!bluetoothAdapter.isEnabled()) {
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
        } else {
            CustomGattServer customGattServer = btManager_.getCustomGattServer(appId);
            BluetoothDevice bluetoothDevice = bluetoothAdapter.getRemoteDevice(address);
            if (customGattServer != null && bluetoothDevice != null) {
                BluetoothGattServer bluetoothGattServer = customGattServer.getBluetoothGattServer();
                if (bluetoothGattServer != null) {
                    int requestId = customGattServer.getRequestId();
                    bluetoothGattServer.sendResponse(bluetoothDevice, requestId, status, DEFAULT_OFFSET,
                        value.getBytes());
                } else {
                    errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
                }
            } else {
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            }
        }
        return errCode.getId();
    }

    public int respondDescriptorWrite(String address, int appId, int status, int handle) {
        Log.i(LOG_TAG, "AppId is " + appId);
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        if (btManager_ == null) {
            Log.e(LOG_TAG, "CustomBluetoothManager is null");
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        } else if (!checkGattServerPermission()) {
            Log.e(LOG_TAG, "Permission check failed");
            return BluetoothErrorCode.BT_ERR_PERMISSION_FAILED.getId();
        }
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        } else if (!bluetoothAdapter.isEnabled()) {
            errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
        } else {
            CustomGattServer customGattServer = btManager_.getCustomGattServer(appId);
            BluetoothDevice bluetoothDevice = bluetoothAdapter.getRemoteDevice(address);
            if (customGattServer != null && bluetoothDevice != null) {
                BluetoothGattServer bluetoothGattServer = customGattServer.getBluetoothGattServer();
                BluetoothGattDescriptor descriptor = customGattServer.getBluetoothGattDescriptor(handle);
                if (bluetoothGattServer != null && descriptor != null) {
                    int requestId = customGattServer.getRequestId();
                    bluetoothGattServer.sendResponse(bluetoothDevice, requestId, status, DEFAULT_OFFSET,
                        descriptor.getValue());
                } else {
                    errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
                }
            } else {
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            }
        }
        return errCode.getId();
    }

    public int notifyCharacteristicChanged(
        int appId, int characterHandle, String address, String characteristicString, boolean isConfirm) {
        Log.i(LOG_TAG, "AppId is " + appId);
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_NO_ERROR;
        if (btManager_ == null) {
            Log.e(LOG_TAG, "notifyCharacteristicChanged failed, CustomBluetoothManager is null");
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        } else if (!checkGattServerPermission()) {
            Log.e(LOG_TAG, "Permission check failed");
            return BluetoothErrorCode.BT_ERR_PERMISSION_FAILED.getId();
        }
        try {
            BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
            if (bluetoothAdapter == null) {
                Log.e(LOG_TAG, "notifyCharacteristicChanged failed, The device does not support Bluetooth");
                errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
            } else if (!bluetoothAdapter.isEnabled()) {
                Log.e(LOG_TAG, "notifyCharacteristicChanged failed, User not enabled Bluetooth switch");
                errCode = BluetoothErrorCode.BT_ERR_INVALID_STATE;
            } else {
                if (Build.VERSION.SDK_INT < API_33) {
                    BluetoothDevice device = bluetoothAdapter.getRemoteDevice(address);
                    BluetoothGattServer bluetoothGattServer = btManager_.getGattServer(appId);
                    BluetoothGattCharacteristic characteristic = btManager_.getBluetoothGattCharacteristic(appId,
                        characterHandle);
                    byte[] characteristicValue = BluetoothHelper.getCharacteristicValue(characteristicString);
                    if (device == null || bluetoothGattServer == null) {
                        errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
                    } else if (characteristic == null || characteristicValue == null) {
                        errCode = BluetoothErrorCode.BT_ERR_INVALID_PARAM;
                    } else {
                        errCode = bluetoothGattServer.notifyCharacteristicChanged(device, characteristic, isConfirm)
                                ? BluetoothErrorCode.BT_NO_ERROR
                                : BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
                    }
                } else {
                    Log.e(LOG_TAG, "Not supported above Android API 33");
                    errCode = BluetoothErrorCode.BT_ERR_CAPABILITY_NOT_SUPPORT;
                }
            }
        } catch (IllegalArgumentException e) {
            Log.e(LOG_TAG, "respondDescriptorWrite failed, try-catch err is " + e);
            errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        }
        return errCode.getId();
    }

    public int gattClientConnect(int appId, String address, boolean autoConnect) {
        if (!checkGattClientPermission()) {
            return BluetoothErrorCode.BT_ERR_PERMISSION_FAILED.getId();
        }
        int result = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        if (btManager_ == null) {
            return result;
        }
        BluetoothDevice device = btManager_.findBluetoothDevice(address);
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (device == null) {
            Log.e(LOG_TAG, "gattClientConnect failed, Device not found");
        } else if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "gattClientConnect failed, The device does not support Bluetooth");
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "gattClientConnect failed, User not enabled Bluetooth switch");
            result = BluetoothErrorCode.BT_ERR_INVALID_STATE.getId();
        } else {
            BluetoothGattClient bluetoothGattClient = btManager_.findBluetoothGattClient(appId);
            if (bluetoothGattClient == null) {
                bluetoothGattClient = new BluetoothGattClient(appId);
                if (btManager_.addBluetoothGattClient(appId, bluetoothGattClient)) {
                    result = bluetoothGattClient.connect(mContext_, device);
                } else {
                    result = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
                }
            } else {
                result = bluetoothGattClient.connect(mContext_, device);
            }
        }
        return result;
    }

    public int gattClientDisconnect(int appId) {
        if (!checkGattClientPermission()) {
            return BluetoothErrorCode.BT_ERR_PERMISSION_FAILED.getId();
        }
        int result = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        if (btManager_ == null) {
            return result;
        }
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "gattClientDisconnect failed, The device does not support Bluetooth");
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "gattClientDisconnect failed, User not enabled Bluetooth switch");
            result = BluetoothErrorCode.BT_ERR_INVALID_STATE.getId();
        } else {
            BluetoothGattClient bluetoothGattClient = btManager_.findBluetoothGattClient(appId);
            if (bluetoothGattClient != null) {
                result = bluetoothGattClient.disconnect();
            } else {
                Log.e(LOG_TAG, "gattClientDisconnect failed, BluetoothGattClient not found");
            }
        }
        return result;
    }

    public int gattClientRequestExchangeMtu(int appId, int mtu) {
        Log.i(LOG_TAG, "gattClientRequestExchangeMtu enter, mtu: " + String.valueOf(mtu));
        if (!checkGattClientPermission()) {
            return BluetoothErrorCode.BT_ERR_PERMISSION_FAILED.getId();
        }
        int result = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        if (btManager_ == null) {
            return result;
        }
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "gattClientRequestExchangeMtu failed, The device does not support Bluetooth.");
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "gattClientRequestExchangeMtu failed, User not enabled Bluetooth switch.");
            result = BluetoothErrorCode.BT_ERR_INVALID_STATE.getId();
        } else {
            BluetoothGattClient bluetoothGattClient = btManager_.findBluetoothGattClient(appId);
            if (bluetoothGattClient != null) {
                result = bluetoothGattClient.requestExchangeMtu(mtu);
            } else {
                Log.e(LOG_TAG, "gattClientRequestExchangeMtu failed, BluetoothGattClient not found.");
            }
        }
        return result;
    }

    public int gattClientClose(int appId) {
        if (!checkGattClientPermission()) {
            return BluetoothErrorCode.BT_ERR_PERMISSION_FAILED.getId();
        }
        int result = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        if (btManager_ == null) {
            return result;
        }
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "gattClientClose failed, The device does not support Bluetooth");
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "gattClientClose failed, User not enabled Bluetooth switch");
            result = BluetoothErrorCode.BT_ERR_INVALID_STATE.getId();
        } else {
            BluetoothGattClient bluetoothGattClient = btManager_.findBluetoothGattClient(appId);
            if (bluetoothGattClient != null) {
                result = bluetoothGattClient.close();
                if (result == BluetoothErrorCode.BT_NO_ERROR.getId()) {
                    btManager_.deleteBluetoothGattClient(appId);
                }
            } else {
                Log.e(LOG_TAG, "gattClientClose failed, BluetoothGattClient not found");
            }
        }
        return result;
    }

    public int clientDiscoverServices(int appId) {
        if (btManager_ == null) {
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        }
        BluetoothGattClient bluetoothGattClient = btManager_.findBluetoothGattClient(appId);
        if (bluetoothGattClient != null) {
            return bluetoothGattClient.discoverServices();
        } else {
            Log.e(LOG_TAG, "clientDiscoverServices failed, BluetoothGattClient not found.");
            return BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        }
    }

    public int clientReadCharacter(int appId, String sUuidString, String cUuidString) {
        int result = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        if (btManager_ == null) {
            return result;
        }
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "clientReadDescriptor failed, The device does not support Bluetooth.");
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "clientReadDescriptor failed, User not enabled Bluetooth switch.");
            result = BluetoothErrorCode.BT_ERR_INVALID_STATE.getId();
        } else {
            BluetoothGattClient bluetoothGattClient = btManager_.findBluetoothGattClient(appId);
            if (bluetoothGattClient != null) {
                result = bluetoothGattClient.readCharacter(sUuidString, cUuidString);
            } else {
                Log.e(LOG_TAG, "clientReadCharacter failed, BluetoothGattClient not found.");
            }
        }
        return result;
    }

    public int clientWriteCharacter(
        int appId, String sUuidString, String cUuidString, byte[] valueString, int writeType) {
        int result = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        if (btManager_ == null) {
            return result;
        }
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "clientReadDescriptor failed, The device does not support Bluetooth.");
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "clientReadDescriptor failed, User not enabled Bluetooth switch.");
            result = BluetoothErrorCode.BT_ERR_INVALID_STATE.getId();
        } else {
            BluetoothGattClient bluetoothGattClient = btManager_.findBluetoothGattClient(appId);
            if (bluetoothGattClient != null) {
                result = bluetoothGattClient.writeCharacter(sUuidString, cUuidString, valueString, writeType);
            } else {
                Log.e(LOG_TAG, "clientWriteCharacter failed, BluetoothGattClient not found.");
            }
        }
        return result;
    }

    public int clientReadDescriptor(int appId, String sUuidString, String cUuidString, String dUuidString) {
        int result = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        if (btManager_ == null) {
            return result;
        }
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "clientReadDescriptor failed, The device does not support Bluetooth.");
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "clientReadDescriptor failed, User not enabled Bluetooth switch.");
            result = BluetoothErrorCode.BT_ERR_INVALID_STATE.getId();
        } else {
            BluetoothGattClient bluetoothGattClient = btManager_.findBluetoothGattClient(appId);
            if (bluetoothGattClient != null) {
                result = bluetoothGattClient.readDescriptor(sUuidString, cUuidString, dUuidString);
            } else {
                Log.e(LOG_TAG, "clientReadDescriptor failed, BluetoothGattClient not found.");
            }
        }
        return result;
    }

    public int clientWriteDescriptor(
        int appId, String sUuidString, String cUuidString, String dUuidString, byte[] valueString) {
        int result = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR.getId();
        if (btManager_ == null) {
            return result;
        }
        BluetoothAdapter bluetoothAdapter = btManager_.getBluetoothAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOG_TAG, "clientReadDescriptor failed, The device does not support Bluetooth.");
        } else if (!bluetoothAdapter.isEnabled()) {
            Log.e(LOG_TAG, "clientReadDescriptor failed, User not enabled Bluetooth switch.");
            result = BluetoothErrorCode.BT_ERR_INVALID_STATE.getId();
        } else {
            BluetoothGattClient bluetoothGattClient = btManager_.findBluetoothGattClient(appId);
            if (bluetoothGattClient != null) {
                result = bluetoothGattClient.writeDescriptor(sUuidString, cUuidString, dUuidString, valueString);
            } else {
                Log.e(LOG_TAG, "clientReadDescriptor failed, BluetoothGattClient not found.");
            }
        }
        return result;
    }

    public void serverMtuChanged(Intent intent) {
        if (intent == null) {
            Log.e(LOG_TAG, "Intent is null");
            return;
        }
        int applicationId = intent.getIntExtra("applicationId", 0);
        String device = intent.getStringExtra("device");
        int mtu = intent.getIntExtra("mtu", 0);
        nativeServerOnMtuChangedCallback(device, mtu, applicationId);
    }

    public void serverConnectionStateChange(Intent intent) {
        if (intent == null) {
            Log.e(LOG_TAG, "Intent is null");
            return;
        }
        int applicationId = intent.getIntExtra("applicationId", 0);
        String device = intent.getStringExtra("device");
        int state = intent.getIntExtra("state", 0);
        nativeServerOnConnectionStateChangeCallback(device, state, applicationId);
    }

    public void serverCharacteristicRead(Intent intent) {
        if (intent == null) {
            Log.e(LOG_TAG, "Intent is null");
            return;
        }
        int applicationId = intent.getIntExtra("applicationId", 0);
        String device = intent.getStringExtra("device");
        String characteristic = intent.getStringExtra("characteristic");
        nativeServerOnCharacteristicReadRequestCallback(device, characteristic, applicationId);
    }

    public void serverCharacteristicWrite(Intent intent) {
        if (intent == null) {
            Log.e(LOG_TAG, "Intent is null");
            return;
        }
        int applicationId = intent.getIntExtra("applicationId", 0);
        String device = intent.getStringExtra("device");
        String characteristic = intent.getStringExtra("characteristic");
        boolean responseNeeded = intent.getIntExtra("responseNeeded", 0) == 1 ? true : false;
        nativeServerOnCharacteristicWriteRequestCallback(device, characteristic, responseNeeded, applicationId);
    }

    public void serverDescriptorRead(Intent intent) {
        if (intent == null) {
            Log.e(LOG_TAG, "Intent is null");
            return;
        }
        int applicationId = intent.getIntExtra("applicationId", 0);
        String device = intent.getStringExtra("device");
        String descriptor = intent.getStringExtra("descriptor");
        nativeServerOnDescriptorReadRequestCallback(device, descriptor, applicationId);
    }

    public void serverDescriptorWrite(Intent intent) {
        if (intent == null) {
            Log.e(LOG_TAG, "Intent is null");
            return;
        }
        int applicationId = intent.getIntExtra("applicationId", 0);
        String device = intent.getStringExtra("device");
        String descriptor = intent.getStringExtra("descriptor");
        nativeServerOnDescriptorWriteRequestCallback(device, descriptor, applicationId);
    }

    public void actionBluetoothStateChange(Intent intent) {
        int bluetoothState = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, 0);
        onChangeStateCallBack(bluetoothState);
    }

    public void actionBondStateChange(Intent intent) {
        BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
        String address = device.getAddress();
        int transport = intent.getIntExtra("android.bluetooth.device.extra.TRANSPORT", 0);
        int status = device.getBondState();
        BluetoothErrorCode errCode = BluetoothErrorCode.BT_ERR_INTERNAL_ERROR;
        int cause = 0;
        onPairStatusChangedCallBack(transport, address, status, cause);
    }

    public void actionFondChange(Intent intent) {
        BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
        String address = device.getAddress();
        int rssi = intent.getIntExtra(BluetoothDevice.EXTRA_RSSI, 0);
        String deviceName = device.getName();
        int deviceClass = device.getBluetoothClass().getDeviceClass();
        onDiscoveryResultCallBack(address, rssi, deviceName, deviceClass);
    }

    public void actionConnectionStateChange(Intent intent) {
        BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
        int state = intent.getIntExtra(BluetoothProfile.EXTRA_STATE, 0);
        OnConnectionStateChanged(device.getAddress(), state, 0);
    }

    public void actionBleClientServiceDiscovered(Intent intent) {
        int appId = intent.getIntExtra("appId", 0);
        String serviceJSONString = intent.getStringExtra("serviceJSONString");
        List<String> characterStrings = intent.getStringArrayListExtra("characterStrings");
        HashMap<String, String[]> clientCharacteristicMap =
            (HashMap<String, String[]>) intent.getSerializableExtra("clientCharacteristicMap");
        BluetoothPlugin.nativeGattClientSetServices(
            appId, serviceJSONString, characterStrings.toArray(new String[] {}), clientCharacteristicMap);
    }

    public void actionBleClientConnectionStateChange(Intent intent) {
        int appId = intent.getIntExtra("appId", 0);
        int newState = intent.getIntExtra("newState", 0);
        int status = intent.getIntExtra("status", 0);
        BluetoothPlugin.nativeOnConnectionStateChanged(appId, status, newState);
    }

    public void actionBleClientReadCharacteristic(Intent intent) {
        int appId = intent.getIntExtra("appId", 0);
        String result = intent.getStringExtra("result");
        int status = intent.getIntExtra("status", 0);
        BluetoothPlugin.nativeOnCharacteristicRead(appId, result, status);
    }

    public void actionBleClientWriteCharacteristic(Intent intent) {
        int appId = intent.getIntExtra("appId", 0);
        String result = intent.getStringExtra("result");
        int status = intent.getIntExtra("status", 0);
        BluetoothPlugin.nativeOnCharacteristicWrite(appId, result, status);
    }

    public void actionBleClientReadDescriptor(Intent intent) {
        int appId = intent.getIntExtra("appId", 0);
        String result = intent.getStringExtra("result");
        int status = intent.getIntExtra("status", 0);
        BluetoothPlugin.nativeOnDescriptorRead(appId, result, status);
    }

    public void actionBleClientWriteDescriptor(Intent intent) {
        int appId = intent.getIntExtra("appId", 0);
        String result = intent.getStringExtra("result");
        int status = intent.getIntExtra("status", 0);
        BluetoothPlugin.nativeOnDescriptorWrite(appId, result, status);
    }

    public void actionBleClientSetMtu(Intent intent) {
        int appId = intent.getIntExtra("appId", 0);
        int mtu = intent.getIntExtra("appId", 0);
        int status = intent.getIntExtra("mtu", 23);
        BluetoothPlugin.nativeOnMtuChanged(appId, status, mtu);
    }

    public void actionBleAdvertiserResult(Intent intent) {
        int result = intent.getIntExtra("result", 0);
        int advHandle = intent.getIntExtra("advHandle", 0);
        int opCode = intent.getIntExtra("opCode", 0);
        BluetoothPlugin.nativeOnStartAdvertisingResult(result, advHandle, opCode);
    }

    class BluetoothBroadcastReceive extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String strAction = intent.getAction();
            if (BluetoothAdapter.ACTION_STATE_CHANGED.equals(strAction)) {
                actionBluetoothStateChange(intent);
            } else if (BluetoothDevice.ACTION_BOND_STATE_CHANGED.equals(strAction)) {
                actionBondStateChange(intent);
            } else if (BluetoothDevice.ACTION_FOUND.equals(strAction)) {
                actionFondChange(intent);
            } else if (BluetoothA2dp.ACTION_CONNECTION_STATE_CHANGED.equals(strAction)) {
                actionConnectionStateChange(intent);
            } else if (BLE_CLIENT_SERVICES_DISCOVERED.equals(strAction)) {
                actionBleClientServiceDiscovered(intent);
            } else if (BLE_CLIENT_CONNECTION_STATE_CHANGE.equals(strAction)) {
                actionBleClientConnectionStateChange(intent);
            } else if (BLE_CLIENT_READ_CHARACTERISTIC.equals(strAction)) {
                actionBleClientReadCharacteristic(intent);
            } else if (BLE_CLIENT_WRITE_CHARACTERISTIC.equals(strAction)) {
                actionBleClientWriteCharacteristic(intent);
            } else if (BLE_CLIENT_READ_DESCRIPTOR.equals(strAction)) {
                actionBleClientReadDescriptor(intent);
            } else if (BLE_CLIENT_WRITE_DESCRIPTOR.equals(strAction)) {
                actionBleClientWriteDescriptor(intent);
            } else if (BLE_CLIENT_SET_MTU.equals(strAction)) {
                actionBleClientSetMtu(intent);
            } else if (BLE_SERVER_MTU_CHANGE.equals(strAction)) {
                serverMtuChanged(intent);
            } else if (BLE_SERVER_CONNECTION_STATE_CHANGE.equals(strAction)) {
                serverConnectionStateChange(intent);
            } else if (BLE_SERVER_READ_CHARACTERISTIC.equals(strAction)) {
                serverCharacteristicRead(intent);
            } else if (BLE_SERVER_WRITE_CHARACTERISTIC.equals(strAction)) {
                serverCharacteristicWrite(intent);
            } else if (BLE_SERVER_READ_DESCRIPTOR.equals(strAction)) {
                serverDescriptorRead(intent);
            } else if (BLE_SERVER_WRITE_DESCRIPTOR.equals(strAction)) {
                serverDescriptorWrite(intent);
            } else if (BLE_ADVERTISER_RESULT.equals(strAction)) {
                actionBleAdvertiserResult(intent);
            }
        }
    };

    /**
     * nativeInit
     * Register the initialization method of the plugin for the plugin construction
     * to call.
     *
     */
    public native void nativeInit();

    protected static native void nativeOnScanResult(String scanResultData, int scannerId);

    protected static native void nativeOnStartAdvertisingResult(int errCode, int advHandle, int opCode);

    protected native void onChangeStateCallBack(int state);

    protected native void onDiscoveryResultCallBack(String address, int rssi, String deviceName, int deviceClass);

    protected native void onPairStatusChangedCallBack(int transport, String address, int status, int cause);

    protected native void OnConnectionStateChanged(String address, int status, int cause);

    protected static native void nativeOnCharacteristicRead(int appId, String jsonString, int status);

    protected static native void nativeOnCharacteristicWrite(int appId, String jsonString, int status);

    protected static native void nativeOnConnectionStateChanged(int appId, int state, int newState);

    protected static native void nativeGattClientSetServices(
        int appId, String sUuid, String[] cUuids, HashMap<String, String[]>clientCharacteristicMap);

    protected static native void nativeServerOnMtuChangedCallback(String deviceData, int mtu, int appId);

    protected static native void nativeServerOnConnectionStateChangeCallback(
        String deviceData, int newState, int appId);

    protected static native void nativeServerOnCharacteristicReadRequestCallback(
        String deviceData, String characteristicData, int appId);

    protected static native void nativeServerOnCharacteristicWriteRequestCallback(
        String deviceData, String characteristicData, boolean responseNeeded, int appId);

    protected static native void nativeServerOnDescriptorReadRequestCallback(
        String deviceData, String descriptorData, int appId);

    protected static native void nativeServerOnDescriptorWriteRequestCallback(
        String deviceData, String descriptorData, int appId);

    protected static native void nativeOnDescriptorRead(int appId, String jsonString, int status);

    protected static native void nativeOnMtuChanged(int appId, int state, int mtu);

    protected static native void nativeOnDescriptorWrite(int appId, String jsonString, int status);

    public enum AdvertisingStatus {
        ADVERTISING_STATUS_NOT_START,
        ADVERTISING_STATUS_STOP,
        ADVERTISING_STATUS_START,
        ADVERTISING_STATUS_ALREADY_START,
        ADVERTISING_STATUS_INTERNAL;
    }

    public enum AdvOptCode {
        ADV_DEFAULT_CODE,
        ADV_STOP_COMPLETE_CODE,
        ADV_FAILED_CODE;
    }
}
