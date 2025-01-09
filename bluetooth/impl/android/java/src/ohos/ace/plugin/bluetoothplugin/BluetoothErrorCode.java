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

/**
 * Bluetooth error code.
 *
 */
public enum BluetoothErrorCode {
    BT_ERR_PERMISSION_FAILED(201, "Permission Failed"),
    BT_ERR_SYSTEM_PERMISSION_FAILED(202, "System Permission Failed"),
    BT_ERR_PROHIBITED_BY_EDM(203, "Prohibited By Edm"),
    BT_ERR_INVALID_PARAM(401, "Invalid Param"),
    BT_ERR_CAPABILITY_NOT_SUPPORT(801, "Capability Not Support"),

    BT_NO_ERROR(0, "Correct"),

    BT_ERR_BASE_SYSCAP(2900000, "Base Syscap"),

    BT_ERR_SERVICE_DISCONNECTED(BT_ERR_BASE_SYSCAP.getId() + 1, "Service Disconnected"),
    BT_ERR_UNBONDED_DEVICE(BT_ERR_BASE_SYSCAP.getId() + 2, "Unbonded Device"),
    BT_ERR_INVALID_STATE(BT_ERR_BASE_SYSCAP.getId() + 3, "Invalid State"),
    BT_ERR_PROFILE_DISABLED(BT_ERR_BASE_SYSCAP.getId() + 4, "Profile Disabled"),
    BT_ERR_DEVICE_DISCONNECTED(BT_ERR_BASE_SYSCAP.getId() + 5, "Device Disconnected"),
    BT_ERR_MAX_CONNECTION(BT_ERR_BASE_SYSCAP.getId() + 6, "Max Connection"),
    BT_ERR_TIMEOUT(BT_ERR_BASE_SYSCAP.getId() + 7, "Timeout"),
    BT_ERR_UNAVAILABLE_PROXY(BT_ERR_BASE_SYSCAP.getId() + 8, "Unavailable Proxy"),

    BT_ERR_INTERNAL_ERROR(BT_ERR_BASE_SYSCAP.getId() + 99, "Internal Error"),
    BT_ERR_IPC_TRANS_FAILED(BT_ERR_BASE_SYSCAP.getId() + 100, "Ipc Trans Failed"),

    BT_ERR_GATT_READ_NOT_PERMITTED(BT_ERR_BASE_SYSCAP.getId() + 1000, "Gatt Read Not Permitted"),
    BT_ERR_GATT_WRITE_NOT_PERMITTED(BT_ERR_BASE_SYSCAP.getId() + 1001, "Gatt Write Not Permitted"),
    BT_ERR_GATT_MAX_SERVER(BT_ERR_BASE_SYSCAP.getId() + 1002, "Gatt Max Server"),

    BT_ERR_SPP_SERVER_STATE(BT_ERR_BASE_SYSCAP.getId() + 1050, "Spp Server State"),
    BT_ERR_SPP_BUSY(BT_ERR_BASE_SYSCAP.getId() + 1051, "Spp Busy"),
    BT_ERR_SPP_DEVICE_NOT_FOUND(BT_ERR_BASE_SYSCAP.getId() + 1052, " Spp Device Not Found"),
    BT_ERR_SPP_IO(BT_ERR_SPP_BUSY.getId() + 1054, " Spp Io"),

    BT_ERR_NO_ACTIVE_HFP_DEVICE(BT_ERR_SPP_BUSY.getId() + 2000, "No Active Hfp Device"),
    BT_ERR_NULL_HFP_STATE_MACHINE(BT_ERR_SPP_BUSY.getId() + 2001, "Null Hfp State Machine"),
    BT_ERR_HFP_NOT_CONNECT(BT_ERR_SPP_BUSY.getId() + 2002, "Hfp Not Connect"),
    BT_ERR_SCO_HAS_BEEN_CONNECTED(BT_ERR_SPP_BUSY.getId() + 2003, "Sco Has Been Connected"),
    BT_ERR_VR_HAS_BEEN_STARTED(BT_ERR_SPP_BUSY.getId() + 2004, "Vr Has Been Started"),
    BT_ERR_AUDIO_NOT_IDLE(BT_ERR_SPP_BUSY.getId() + 2005, "Audio Not Idle"),
    BT_ERR_VIRTUAL_CALL_NOT_STARTED(BT_ERR_SPP_BUSY.getId() + 2006, "Virtual Call Not Started"),
    BT_ERR_DISCONNECT_SCO_FAILED(BT_ERR_SPP_BUSY.getId() + 2007, "Disconnect Sco Failed");

    private int id;
    private String errorMessage;

    /**
     * Constructor of BluetoothErrorCode.
     *
     * @param id           BluetoothErrorCodeId.
     * @param errorMessage BluetoothErrorMessage.
     */
    BluetoothErrorCode(int id, String errorMessage) {
        this.id = id;
        this.errorMessage = errorMessage;
    }

    /**
     * Get id of BluetoothErrorCode.
     *
     * @return The BluetoothErrorCodeId.
     */
    public int getId() {
        return id;
    }

    /**
     * Get errorMessage of BluetoothErrorCode.
     *
     * @return The BluetoothErrorMessage.
     */
    public String getErrorMessage() {
        return this.errorMessage;
    }
}
