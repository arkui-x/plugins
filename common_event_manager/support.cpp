/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "support.h"

#include "common_event_support.h"
#include "log.h"

namespace OHOS {
namespace Plugin {
void SetNamedPropertyByStr(napi_env env, napi_value dstObj, const std::string& objName, const char* propName)
{
    napi_value prop = nullptr;
    if (napi_create_string_utf8(env, objName.c_str(), NAPI_AUTO_LENGTH, &prop) == napi_ok) {
        napi_set_named_property(env, dstObj, propName, prop);
    }
}

napi_value SupportInit(napi_env env, napi_value exports)
{
    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED, "COMMON_EVENT_BOOT_COMPLETED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_LOCKED_BOOT_COMPLETED,
        "COMMON_EVENT_LOCKED_BOOT_COMPLETED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_SHUTDOWN, "COMMON_EVENT_SHUTDOWN");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED, "COMMON_EVENT_BATTERY_CHANGED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_BATTERY_LOW, "COMMON_EVENT_BATTERY_LOW");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_BATTERY_OKAY, "COMMON_EVENT_BATTERY_OKAY");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_POWER_CONNECTED, "COMMON_EVENT_POWER_CONNECTED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED, "COMMON_EVENT_POWER_DISCONNECTED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_SCREEN_OFF, "COMMON_EVENT_SCREEN_OFF");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_SCREEN_ON, "COMMON_EVENT_SCREEN_ON");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_THERMAL_LEVEL_CHANGED,
        "COMMON_EVENT_THERMAL_LEVEL_CHANGED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_USER_PRESENT, "COMMON_EVENT_USER_PRESENT");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_TIME_TICK, "COMMON_EVENT_TIME_TICK");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_TIME_CHANGED, "COMMON_EVENT_TIME_CHANGED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_DATE_CHANGED, "COMMON_EVENT_DATE_CHANGED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_TIMEZONE_CHANGED, "COMMON_EVENT_TIMEZONE_CHANGED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_CLOSE_SYSTEM_DIALOGS,
        "COMMON_EVENT_CLOSE_SYSTEM_DIALOGS");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED, "COMMON_EVENT_PACKAGE_ADDED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_PACKAGE_REPLACED, "COMMON_EVENT_PACKAGE_REPLACED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_MY_PACKAGE_REPLACED,
        "COMMON_EVENT_MY_PACKAGE_REPLACED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED, "COMMON_EVENT_PACKAGE_REMOVED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_BUNDLE_REMOVED, "COMMON_EVENT_BUNDLE_REMOVED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_PACKAGE_FULLY_REMOVED,
        "COMMON_EVENT_PACKAGE_FULLY_REMOVED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED, "COMMON_EVENT_PACKAGE_CHANGED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_PACKAGE_RESTARTED, "COMMON_EVENT_PACKAGE_RESTARTED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED,
        "COMMON_EVENT_PACKAGE_DATA_CLEARED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_PACKAGE_CACHE_CLEARED,
        "COMMON_EVENT_PACKAGE_CACHE_CLEARED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_PACKAGES_SUSPENDED, "COMMON_EVENT_PACKAGES_SUSPENDED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_PACKAGES_UNSUSPENDED,
        "COMMON_EVENT_PACKAGES_UNSUSPENDED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_MY_PACKAGE_SUSPENDED,
        "COMMON_EVENT_MY_PACKAGE_SUSPENDED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_MY_PACKAGE_UNSUSPENDED,
        "COMMON_EVENT_MY_PACKAGE_UNSUSPENDED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_UID_REMOVED, "COMMON_EVENT_UID_REMOVED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_PACKAGE_FIRST_LAUNCH,
        "COMMON_EVENT_PACKAGE_FIRST_LAUNCH");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_PACKAGE_NEEDS_VERIFICATION,
        "COMMON_EVENT_PACKAGE_NEEDS_VERIFICATION");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_PACKAGE_VERIFIED, "COMMON_EVENT_PACKAGE_VERIFIED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_EXTERNAL_APPLICATIONS_AVAILABLE,
        "COMMON_EVENT_EXTERNAL_APPLICATIONS_AVAILABLE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_EXTERNAL_APPLICATIONS_UNAVAILABLE,
        "COMMON_EVENT_EXTERNAL_APPLICATIONS_UNAVAILABLE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_CONFIGURATION_CHANGED,
        "COMMON_EVENT_CONFIGURATION_CHANGED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_LOCALE_CHANGED, "COMMON_EVENT_LOCALE_CHANGED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_MANAGE_PACKAGE_STORAGE,
        "COMMON_EVENT_MANAGE_PACKAGE_STORAGE");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_DRIVE_MODE, "COMMON_EVENT_DRIVE_MODE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_HOME_MODE, "COMMON_EVENT_HOME_MODE");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_OFFICE_MODE, "COMMON_EVENT_OFFICE_MODE");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_SPLIT_SCREEN, "COMMON_EVENT_SPLIT_SCREEN");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_USER_STARTED, "COMMON_EVENT_USER_STARTED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_USER_BACKGROUND, "COMMON_EVENT_USER_BACKGROUND");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_USER_FOREGROUND, "COMMON_EVENT_USER_FOREGROUND");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_USER_SWITCHED, "COMMON_EVENT_USER_SWITCHED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_USER_STARTING, "COMMON_EVENT_USER_STARTING");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_USER_UNLOCKED, "COMMON_EVENT_USER_UNLOCKED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_USER_STOPPING, "COMMON_EVENT_USER_STOPPING");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_USER_STOPPED, "COMMON_EVENT_USER_STOPPED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_HWID_LOGIN, "COMMON_EVENT_HWID_LOGIN");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_HWID_LOGOUT, "COMMON_EVENT_HWID_LOGOUT");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_HWID_TOKEN_INVALID, "COMMON_EVENT_HWID_TOKEN_INVALID");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_HWID_LOGOFF, "COMMON_EVENT_HWID_LOGOFF");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE, "COMMON_EVENT_WIFI_POWER_STATE");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_WIFI_SCAN_FINISHED, "COMMON_EVENT_WIFI_SCAN_FINISHED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_WIFI_RSSI_VALUE, "COMMON_EVENT_WIFI_RSSI_VALUE");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_WIFI_CONN_STATE, "COMMON_EVENT_WIFI_CONN_STATE");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_WIFI_HOTSPOT_STATE, "COMMON_EVENT_WIFI_HOTSPOT_STATE");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_WIFI_AP_STA_JOIN, "COMMON_EVENT_WIFI_AP_STA_JOIN");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_WIFI_AP_STA_LEAVE, "COMMON_EVENT_WIFI_AP_STA_LEAVE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_WIFI_MPLINK_STATE_CHANGE,
        "COMMON_EVENT_WIFI_MPLINK_STATE_CHANGE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_WIFI_P2P_CONN_STATE,
        "COMMON_EVENT_WIFI_P2P_CONN_STATE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_WIFI_P2P_STATE_CHANGED,
        "COMMON_EVENT_WIFI_P2P_STATE_CHANGED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_WIFI_P2P_PEERS_STATE_CHANGED,
        "COMMON_EVENT_WIFI_P2P_PEERS_STATE_CHANGED");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_WIFI_P2P_PEERS_DISCOVERY_STATE_CHANGED,
        "COMMON_EVENT_WIFI_P2P_PEERS_DISCOVERY_STATE_CHANGED");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_WIFI_P2P_CURRENT_DEVICE_STATE_CHANGED,
        "COMMON_EVENT_WIFI_P2P_CURRENT_DEVICE_STATE_CHANGED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_WIFI_P2P_GROUP_STATE_CHANGED,
        "COMMON_EVENT_WIFI_P2P_GROUP_STATE_CHANGED");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREE_AG_CONNECT_STATE_UPDATE,
        "COMMON_EVENT_BLUETOOTH_HANDSFREE_AG_CONNECT_STATE_UPDATE");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREE_AG_CURRENT_DEVICE_UPDATE,
        "COMMON_EVENT_BLUETOOTH_HANDSFREE_AG_CURRENT_DEVICE_UPDATE");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREE_AG_AUDIO_STATE_UPDATE,
        "COMMON_EVENT_BLUETOOTH_HANDSFREE_AG_AUDIO_STATE_UPDATE");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CONNECT_STATE_UPDATE,
        "COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CONNECT_STATE_UPDATE");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CURRENT_DEVICE_UPDATE,
        "COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CURRENT_DEVICE_UPDATE");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSOURCE_PLAYING_STATE_UPDATE,
        "COMMON_EVENT_BLUETOOTH_A2DPSOURCE_PLAYING_STATE_UPDATE");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSOURCE_AVRCP_CONNECT_STATE_UPDATE,
        "COMMON_EVENT_BLUETOOTH_A2DPSOURCE_AVRCP_CONNECT_STATE_UPDATE");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CODEC_VALUE_UPDATE,
        "COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CODEC_VALUE_UPDATE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_DISCOVERED,
        "COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_DISCOVERED");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_CLASS_VALUE_UPDATE,
        "COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_CLASS_VALUE_UPDATE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_ACL_CONNECTED,
        "COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_ACL_CONNECTED");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_ACL_DISCONNECTED,
        "COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_ACL_DISCONNECTED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_NAME_UPDATE,
        "COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_NAME_UPDATE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_PAIR_STATE,
        "COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_PAIR_STATE");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_BATTERY_VALUE_UPDATE,
        "COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_BATTERY_VALUE_UPDATE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_SDP_RESULT,
        "COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_SDP_RESULT");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_UUID_VALUE,
        "COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_UUID_VALUE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_PAIRING_REQ,
        "COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_PAIRING_REQ");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_PAIRING_CANCEL,
        "COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_PAIRING_CANCEL");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_CONNECT_REQ,
        "COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_CONNECT_REQ");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_CONNECT_REPLY,
        "COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_CONNECT_REPLY");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_CONNECT_CANCEL,
        "COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_CONNECT_CANCEL");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREEUNIT_CONNECT_STATE_UPDATE,
        "COMMON_EVENT_BLUETOOTH_HANDSFREEUNIT_CONNECT_STATE_UPDATE");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREEUNIT_AUDIO_STATE_UPDATE,
        "COMMON_EVENT_BLUETOOTH_HANDSFREEUNIT_AUDIO_STATE_UPDATE");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREEUNIT_AG_COMMON_EVENT,
        "COMMON_EVENT_BLUETOOTH_HANDSFREEUNIT_AG_COMMON_EVENT");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREEUNIT_AG_CALL_STATE_UPDATE,
        "COMMON_EVENT_BLUETOOTH_HANDSFREEUNIT_AG_CALL_STATE_UPDATE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE,
        "COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_REQ_DISCOVERABLE,
        "COMMON_EVENT_BLUETOOTH_HOST_REQ_DISCOVERABLE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_REQ_ENABLE,
        "COMMON_EVENT_BLUETOOTH_HOST_REQ_ENABLE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_REQ_DISABLE,
        "COMMON_EVENT_BLUETOOTH_HOST_REQ_DISABLE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_SCAN_MODE_UPDATE,
        "COMMON_EVENT_BLUETOOTH_HOST_SCAN_MODE_UPDATE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_DISCOVERY_STARTED,
        "COMMON_EVENT_BLUETOOTH_HOST_DISCOVERY_STARTED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_DISCOVERY_FINISHED,
        "COMMON_EVENT_BLUETOOTH_HOST_DISCOVERY_FINISHED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_NAME_UPDATE,
        "COMMON_EVENT_BLUETOOTH_HOST_NAME_UPDATE");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSINK_CONNECT_STATE_UPDATE,
        "COMMON_EVENT_BLUETOOTH_A2DPSINK_CONNECT_STATE_UPDATE");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSINK_PLAYING_STATE_UPDATE,
        "COMMON_EVENT_BLUETOOTH_A2DPSINK_PLAYING_STATE_UPDATE");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSINK_AUDIO_STATE_UPDATE,
        "COMMON_EVENT_BLUETOOTH_A2DPSINK_AUDIO_STATE_UPDATE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_NFC_ACTION_ADAPTER_STATE_CHANGED,
        "COMMON_EVENT_NFC_ACTION_ADAPTER_STATE_CHANGED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_NFC_ACTION_RF_FIELD_ON_DETECTED,
        "COMMON_EVENT_NFC_ACTION_RF_FIELD_ON_DETECTED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_NFC_ACTION_RF_FIELD_OFF_DETECTED,
        "COMMON_EVENT_NFC_ACTION_RF_FIELD_OFF_DETECTED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_DISCHARGING, "COMMON_EVENT_DISCHARGING");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_CHARGING, "COMMON_EVENT_CHARGING");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_CHARGE_TYPE_CHANGED,
        "COMMON_EVENT_CHARGE_TYPE_CHANGED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_CHARGE_IDLE_MODE_CHANGED,
        "COMMON_EVENT_CHARGE_IDLE_MODE_CHANGED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_DEVICE_IDLE_MODE_CHANGED,
        "COMMON_EVENT_DEVICE_IDLE_MODE_CHANGED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_DEVICE_IDLE_EXEMPTION_LIST_UPDATED,
        "COMMON_EVENT_DEVICE_IDLE_EXEMPTION_LIST_UPDATED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED,
        "COMMON_EVENT_POWER_SAVE_MODE_CHANGED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_USER_ADDED, "COMMON_EVENT_USER_ADDED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_USER_REMOVED, "COMMON_EVENT_USER_REMOVED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_ABILITY_ADDED, "COMMON_EVENT_ABILITY_ADDED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_ABILITY_REMOVED, "COMMON_EVENT_ABILITY_REMOVED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_ABILITY_UPDATED, "COMMON_EVENT_ABILITY_UPDATED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_LOCATION_MODE_STATE_CHANGED,
        "COMMON_EVENT_LOCATION_MODE_STATE_CHANGED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_IVI_SLEEP, "COMMON_EVENT_IVI_SLEEP");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_IVI_PAUSE, "COMMON_EVENT_IVI_PAUSE");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_IVI_STANDBY, "COMMON_EVENT_IVI_STANDBY");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_IVI_LASTMODE_SAVE, "COMMON_EVENT_IVI_LASTMODE_SAVE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_IVI_VOLTAGE_ABNORMAL,
        "COMMON_EVENT_IVI_VOLTAGE_ABNORMAL");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_IVI_HIGH_TEMPERATURE,
        "COMMON_EVENT_IVI_HIGH_TEMPERATURE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_IVI_EXTREME_TEMPERATURE,
        "COMMON_EVENT_IVI_EXTREME_TEMPERATURE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_IVI_TEMPERATURE_ABNORMAL,
        "COMMON_EVENT_IVI_TEMPERATURE_ABNORMAL");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_IVI_VOLTAGE_RECOVERY,
        "COMMON_EVENT_IVI_VOLTAGE_RECOVERY");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_IVI_TEMPERATURE_RECOVERY,
        "COMMON_EVENT_IVI_TEMPERATURE_RECOVERY");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_IVI_ACTIVE, "COMMON_EVENT_IVI_ACTIVE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_USB_STATE, "COMMON_EVENT_USB_STATE");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_USB_PORT_CHANGED, "COMMON_EVENT_USB_PORT_CHANGED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_USB_DEVICE_ATTACHED,
        "COMMON_EVENT_USB_DEVICE_ATTACHED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_USB_DEVICE_DETACHED,
        "COMMON_EVENT_USB_DEVICE_DETACHED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_USB_ACCESSORY_ATTACHED,
        "COMMON_EVENT_USB_ACCESSORY_ATTACHED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_USB_ACCESSORY_DETACHED,
        "COMMON_EVENT_USB_ACCESSORY_DETACHED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_DISK_REMOVED, "COMMON_EVENT_DISK_REMOVED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_DISK_UNMOUNTED, "COMMON_EVENT_DISK_UNMOUNTED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_DISK_MOUNTED, "COMMON_EVENT_DISK_MOUNTED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_DISK_BAD_REMOVAL, "COMMON_EVENT_DISK_BAD_REMOVAL");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_DISK_UNMOUNTABLE, "COMMON_EVENT_DISK_UNMOUNTABLE");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_DISK_EJECT, "COMMON_EVENT_DISK_EJECT");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_VOLUME_REMOVED, "COMMON_EVENT_VOLUME_REMOVED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_VOLUME_UNMOUNTED, "COMMON_EVENT_VOLUME_UNMOUNTED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_VOLUME_MOUNTED, "COMMON_EVENT_VOLUME_MOUNTED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_VOLUME_BAD_REMOVAL, "COMMON_EVENT_VOLUME_BAD_REMOVAL");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_VOLUME_EJECT, "COMMON_EVENT_VOLUME_EJECT");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_VISIBLE_ACCOUNTS_UPDATED,
        "COMMON_EVENT_VISIBLE_ACCOUNTS_UPDATED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_ACCOUNT_DELETED, "COMMON_EVENT_ACCOUNT_DELETED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_FOUNDATION_READY, "COMMON_EVENT_FOUNDATION_READY");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_SIM_STATE_CHANGED, "COMMON_EVENT_SIM_STATE_CHANGED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_AIRPLANE_MODE_CHANGED,
        "COMMON_EVENT_AIRPLANE_MODE_CHANGED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_SMS_RECEIVE_COMPLETED,
        "COMMON_EVENT_SMS_RECEIVE_COMPLETED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_SMS_EMERGENCY_CB_RECEIVE_COMPLETED,
        "COMMON_EVENT_SMS_EMERGENCY_CB_RECEIVE_COMPLETED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_SMS_CB_RECEIVE_COMPLETED,
        "COMMON_EVENT_SMS_CB_RECEIVE_COMPLETED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_STK_COMMAND, "COMMON_EVENT_STK_COMMAND");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_STK_SESSION_END, "COMMON_EVENT_STK_SESSION_END");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_STK_CARD_STATE_CHANGED,
        "COMMON_EVENT_STK_CARD_STATE_CHANGED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_STK_ALPHA_IDENTIFIER,
        "COMMON_EVENT_STK_ALPHA_IDENTIFIER");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_SPN_INFO_CHANGED, "COMMON_EVENT_SPN_INFO_CHANGED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_SMS_WAPPUSH_RECEIVE_COMPLETED,
        "COMMON_EVENT_SMS_WAPPUSH_RECEIVE_COMPLETED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_OPERATOR_CONFIG_CHANGED,
        "COMMON_EVENT_OPERATOR_CONFIG_CHANGED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_SLOT_CHANGE, "COMMON_EVENT_SLOT_CHANGE");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED, "COMMON_EVENT_CALL_STATE_CHANGED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_CELLULAR_DATA_STATE_CHANGED,
        "COMMON_EVENT_CELLULAR_DATA_STATE_CHANGED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_INCOMING_CALL_MISSED,
        "COMMON_EVENT_INCOMING_CALL_MISSED");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_SIM_CARD_DEFAULT_DATA_SUBSCRIPTION_CHANGED,
        "COMMON_EVENT_SIM_CARD_DEFAULT_DATA_SUBSCRIPTION_CHANGED");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_SIM_CARD_DEFAULT_SMS_SUBSCRIPTION_CHANGED,
        "COMMON_EVENT_SIM_CARD_DEFAULT_SMS_SUBSCRIPTION_CHANGED");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_SIM_CARD_DEFAULT_MAIN_SUBSCRIPTION_CHANGED,
        "COMMON_EVENT_SIM_CARD_DEFAULT_MAIN_SUBSCRIPTION_CHANGED");
    SetNamedPropertyByStr(env, obj,
        CommonEventSupport::COMMON_EVENT_SIM_CARD_DEFAULT_VOICE_SUBSCRIPTION_CHANGED,
        "COMMON_EVENT_SIM_CARD_DEFAULT_VOICE_SUBSCRIPTION_CHANGED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_SIGNAL_INFO_CHANGED,
        "COMMON_EVENT_SIGNAL_INFO_CHANGED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_NETWORK_STATE_CHANGED,
        "COMMON_EVENT_NETWORK_STATE_CHANGED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_QUICK_FIX_APPLY_RESULT,
        "COMMON_EVENT_QUICK_FIX_APPLY_RESULT");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_QUICK_FIX_REVOKE_RESULT,
        "COMMON_EVENT_QUICK_FIX_REVOKE_RESULT");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_RADIO_STATE_CHANGE, "COMMON_EVENT_RADIO_STATE_CHANGE");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGIN,
        "COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGIN");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGOUT,
        "COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGOUT");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_TOKEN_INVALID,
        "COMMON_EVENT_DISTRIBUTED_ACCOUNT_TOKEN_INVALID");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGOFF,
        "COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGOFF");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_USER_INFO_UPDATED, "COMMON_EVENT_USER_INFO_UPDATED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_DOMAIN_ACCOUNT_STATUS_CHANGED,
        "COMMON_EVENT_DOMAIN_ACCOUNT_STATUS_CHANGED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_HTTP_PROXY_CHANGE, "COMMON_EVENT_HTTP_PROXY_CHANGE");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_SCREEN_LOCKED, "COMMON_EVENT_SCREEN_LOCKED");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED, "COMMON_EVENT_SCREEN_UNLOCKED");
    SetNamedPropertyByStr(env, obj, CommonEventSupport::COMMON_EVENT_AUDIO_QUALITY_CHANGE,
        "COMMON_EVENT_AUDIO_QUALITY_CHANGE");
    SetNamedPropertyByStr(
        env, obj, CommonEventSupport::COMMON_EVENT_SPECIAL_CODE, "COMMON_EVENT_SPECIAL_CODE");
    napi_property_descriptor exportFuncs[] = { DECLARE_NAPI_PROPERTY("Support", obj) };
    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);

    return exports;
}
} // namespace Plugin
} // namespace OHOS
