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

#include "common_event_support.h"

namespace OHOS {
namespace Plugin {
/**
 * Indicates the action of a common event that the user has finished booting and the system has been loaded.
 * To subscribe to this common event, your application must have the ohos.permission.RECEIVER_STARTUP_COMPLETED
 * permission.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED = "usual.event.BOOT_COMPLETED";

/**
 * Indicates the action of a common event that the user has finished booting and the system has been loaded but the
 * screen is still locked.
 * To subscribe to this common event, your application must have the ohos.permission.RECEIVER_STARTUP_COMPLETED
 * permission.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_LOCKED_BOOT_COMPLETED = "usual.event.LOCKED_BOOT_COMPLETED";

/**
 * Indicates the action of a common event that the device is being shut down and the final shutdown will proceed.
 * This is different from sleeping. All unsaved data will be lost after shut down.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_SHUTDOWN = "usual.event.SHUTDOWN";

/**
 * Indicates the action of a common event that the charging state, level, and other information about the battery
 * have changed.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED = "usual.event.BATTERY_CHANGED";

/**
 * Indicates the action of a common event that the battery level is low.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_BATTERY_LOW = "usual.event.BATTERY_LOW";

/**
 * Indicates the action of a common event that the battery exit the low state.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_BATTERY_OKAY = "usual.event.BATTERY_OKAY";

/**
 * Indicates the action of a common event that the device is connected to the external power.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_POWER_CONNECTED = "usual.event.POWER_CONNECTED";

/**
 * Indicates the action of a common event that the device is disconnected from the external power.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED = "usual.event.POWER_DISCONNECTED";

/**
 * Indicates the action of a common event that the device screen is off and the device is sleeping.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_SCREEN_OFF = "usual.event.SCREEN_OFF";

/**
 * Indicates the action of a common event that the device screen is on and the device is interactive.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_SCREEN_ON = "usual.event.SCREEN_ON";

/**
 * Indicates the action of a common event that the thermal level changed.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_THERMAL_LEVEL_CHANGED = "usual.event.THERMAL_LEVEL_CHANGED";
/**
 * Indicates the action of a common event that the device is idle and charging,
 * services can do some business on the background.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_CHARGE_IDLE_MODE_CHANGED = "usual.event.CHARGE_IDLE_MODE_CHANGED";
/**
 * Indicates the action of a common event that the user unlocks the device.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_USER_PRESENT = "usual.event.USER_PRESENT";

/**
 * Indicates the action of a common event that the system time has changed.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_TIME_TICK = "usual.event.TIME_TICK";

/**
 * Indicates the action of a common event that the system time is set.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_TIME_CHANGED = "usual.event.TIME_CHANGED";

/**
 * Indicates the action of a common event that the system date has changed.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_DATE_CHANGED = "usual.event.DATE_CHANGED";

/**
 * Indicates the action of a common event that the system time zone has changed.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_TIMEZONE_CHANGED = "usual.event.TIMEZONE_CHANGED";

/**
 * Indicates the action of a common event that a user closes a temporary system dialog box.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_CLOSE_SYSTEM_DIALOGS = "usual.event.CLOSE_SYSTEM_DIALOGS";

/**
 * Indicates the action of a common event that bundle scan has finished.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_BUNDLE_SCAN_FINISHED = "usual.event.BUNDLE_SCAN_FINISHED";

/**
 * Indicates the action of a common event that a new application package has been installed on the device.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED = "usual.event.PACKAGE_ADDED";

/**
 * Indicates the action of a common event that a new version of an installed application package has replaced
 * the previous one on the device.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_PACKAGE_REPLACED = "usual.event.PACKAGE_REPLACED";

/**
 * Indicates the action of a common event that a new version of your application package has replaced
 * the previous one. This common event is sent only to the application that was replaced.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_MY_PACKAGE_REPLACED = "usual.event.MY_PACKAGE_REPLACED";

/**
 * Indicate the action of a common event that an installed application has been uninstalled from the device
 * with the application data remained.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED = "usual.event.PACKAGE_REMOVED";

/**
 * Indicates the action of a common event that an installed bundle has been uninstalled from the device with the
 * application data remained.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_BUNDLE_REMOVED = "usual.event.BUNDLE_REMOVED";

/**
 * Indicates the action of a common event that an installed application, including both the application data and
 * code, have been completely uninstalled from the device.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_PACKAGE_FULLY_REMOVED = "usual.event.PACKAGE_FULLY_REMOVED";

/**
 * Indicates the action of a common event that an application package has been changed
 * (for example, a component in the package has been enabled or disabled).
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED = "usual.event.PACKAGE_CHANGED";

/**
 * Indicates the action of a common event that the user has restarted the application package and killed all its
 * processes.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_PACKAGE_RESTARTED = "usual.event.PACKAGE_RESTARTED";

/**
 * Indicates the action of a common event that the user has cleared the application package data.
 * This common event is published after COMMON_EVENT_PACKAGE_RESTARTED is triggered and the data has been cleared.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED = "usual.event.PACKAGE_DATA_CLEARED";

/**
 * Indicates the action of a common event that the user has cleared the application package cache.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_PACKAGE_CACHE_CLEARED = "usual.event.PACKAGE_CACHE_CLEARED";

/**
 * Indicates the action of a common event that application packages have been suspended.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_PACKAGES_SUSPENDED = "usual.event.PACKAGES_SUSPENDED";

/**
 * Indicates the action of a common event that application packages have not been suspended.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_PACKAGES_UNSUSPENDED = "usual.event.PACKAGES_UNSUSPENDED";

/**
 * Indicates the action of a common event that an application package has been suspended.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_MY_PACKAGE_SUSPENDED = "usual.event.MY_PACKAGE_SUSPENDED";

/**
 * Indicates the action of a common event that an application package has not been suspended.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_MY_PACKAGE_UNSUSPENDED = "usual.event.MY_PACKAGE_UNSUSPENDED";

/**
 * Indicates the action of a common event that a user ID has been removed from the system.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_UID_REMOVED = "usual.event.UID_REMOVED";

/**
 * Indicates the action of a common event that an installed application is started for the first time.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_PACKAGE_FIRST_LAUNCH = "usual.event.PACKAGE_FIRST_LAUNCH";

/**
 * Indicates the action of a common event that an application requires system verification.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_PACKAGE_NEEDS_VERIFICATION =
    "usual.event.PACKAGE_NEEDS_VERIFICATION";
/**
 * Indicates the action of a common event that an application has been verified by the system.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_PACKAGE_VERIFIED = "usual.event.PACKAGE_VERIFIED";

/**
 * Indicates the action of a common event that applications installed on the external storage become
 * available for the system.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_EXTERNAL_APPLICATIONS_AVAILABLE =
    "usual.event.EXTERNAL_APPLICATIONS_AVAILABLE";

/**
 * Indicates the action of a common event that applications installed on the external storage become unavailable for
 * the system.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_EXTERNAL_APPLICATIONS_UNAVAILABLE =
    "usual.event.EXTERNAL_APPLICATIONS_UNAVAILABLE";

/**
 * Indicates the action of a common event that the device state (for example, orientation and locale) has changed.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_CONFIGURATION_CHANGED = "usual.event.CONFIGURATION_CHANGED";

/**
 * Indicates the action of a common event that the device locale has changed.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_LOCALE_CHANGED = "usual.event.LOCALE_CHANGED";

/**
 * Indicates the action of a common event that the device storage is insufficient.
 */
const std::string CommonEventSupport::COMMON_EVENT_MANAGE_PACKAGE_STORAGE = "usual.event.MANAGE_PACKAGE_STORAGE";

/**
 * Indicates the action of a common event that one sandbox package is installed.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_SANDBOX_PACKAGE_ADDED = "usual.event.SANDBOX_PACKAGE_ADDED";

/**
 * Indicates the action of a common event that one sandbox package is uninstalled.
 * This common event can only be published by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_SANDBOX_PACKAGE_REMOVED = "usual.event.SANDBOX_PACKAGE_REMOVED";

/**
 * Indicates the action of a common event that the system is in driving mode.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_DRIVE_MODE = "common.event.DRIVE_MODE";

/**
 * Indicates the action of a common event that the system is in home mode.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_HOME_MODE = "common.event.HOME_MODE";

/**
 * Indicates the action of a common event that the system is in office mode.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_OFFICE_MODE = "common.event.OFFICE_MODE";

/**
 * Indicates the action of a common event that the window mode is split screen.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_SPLIT_SCREEN = "common.event.SPLIT_SCREEN";

/**
 * Indicates the action of a common event that the user has been started.
 */
const std::string CommonEventSupport::COMMON_EVENT_USER_STARTED = "usual.event.USER_STARTED";

/**
 * Indicates the action of a common event that the user has been brought to the background.
 */
const std::string CommonEventSupport::COMMON_EVENT_USER_BACKGROUND = "usual.event.USER_BACKGROUND";

/**
 * Indicates the action of a common event that the user has been brought to the foreground.
 */
const std::string CommonEventSupport::COMMON_EVENT_USER_FOREGROUND = "usual.event.USER_FOREGROUND";

/**
 * Indicates the action of a common event that a user switch is happening.
 * To subscribe to this common event, your application must have the ohos.permission.MANAGE_LOCAL_ACCOUNTS permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_USER_SWITCHED = "usual.event.USER_SWITCHED";

/**
 * Indicates the action of a common event that the user is going to be started.
 * To subscribe to this common event, your application must have the ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS
 * permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_USER_STARTING = "usual.event.USER_STARTING";

/**
 * Indicates the action of a common event that the credential-encrypted storage has become unlocked
 * for the current user when the device is unlocked after being restarted.
 */
const std::string CommonEventSupport::COMMON_EVENT_USER_UNLOCKED = "usual.event.USER_UNLOCKED";

/**
 * Indicates the action of a common event that the user is going to be stopped.
 * To subscribe to this common event, your application must have the ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS
 * permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_USER_STOPPING = "usual.event.USER_STOPPING";

/**
 * Indicates the action of a common event that the user has been stopped.
 */
const std::string CommonEventSupport::COMMON_EVENT_USER_STOPPED = "usual.event.USER_STOPPED";

/**
 * Indicates the action of a common event about a login of a user with account ID.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_HWID_LOGIN = "common.event.HWID_LOGIN";

/**
 * Indicates the action of a common event about a logout of a user with account ID.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_HWID_LOGOUT = "common.event.HWID_LOGOUT";

/**
 * Indicates the action of a common event that the account ID is invalid.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_HWID_TOKEN_INVALID = "common.event.HWID_TOKEN_INVALID";

/**
 * Indicates the action of a common event about a logoff of a account ID.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_HWID_LOGOFF = "common.event.HWID_LOGOFF";

/**
 * Indicates the action of a common event about the Wi-Fi state, such as enabled and disabled.
 */
const std::string CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE = "usual.event.wifi.POWER_STATE";

/**
 * Indicates the action of a common event that the Wi-Fi access point has been scanned and proven to be available.
 * To subscribe to this common event, your application must have the ohos.permission.LOCATION permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_WIFI_SCAN_FINISHED = "usual.event.wifi.SCAN_FINISHED";

/**
 * Indicates the action of a common event that the Wi-Fi signal strength (RSSI) has changed.
 * To subscribe to this common event, your application must have the ohos.permission.GET_WIFI_INFO permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_WIFI_RSSI_VALUE = "usual.event.wifi.RSSI_VALUE";

/**
 * Indicates the action of a common event that the Wi-Fi connection state has changed.
 */
const std::string CommonEventSupport::COMMON_EVENT_WIFI_CONN_STATE = "usual.event.wifi.CONN_STATE";

/**
 * Indicates the action of a common event about the Wi-Fi hotspot state, such as enabled or disabled.
 */
const std::string CommonEventSupport::COMMON_EVENT_WIFI_HOTSPOT_STATE = "usual.event.wifi.HOTSPOT_STATE";

/**
 * Indicates the action of a common event that a client has joined the Wi-Fi hotspot of the current device. You can
 * register this common event to listen for information about the clients joining your hotspot.
 * To subscribe to this common event, your application must have the ohos.permission.GET_WIFI_INFO permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_WIFI_AP_STA_JOIN = "usual.event.wifi.WIFI_HS_STA_JOIN";

/**
 * Indicates the action of a common event that a client has dropped connection to the Wi-Fi hotspot of the current
 * device. You can register this common event to listen for information about the clients leaving your hotspot.
 * To subscribe to this common event, your application must have the ohos.permission.GET_WIFI_INFO permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_WIFI_AP_STA_LEAVE = "usual.event.wifi.WIFI_HS_STA_LEAVE";

/**
 * Indicates the action of a common event that the state of MPLink (an enhanced Wi-Fi feature) has changed.
 * To subscribe to this common event, your application must have the ohos.permission.MPLINK_CHANGE_STATE permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_WIFI_MPLINK_STATE_CHANGE = "usual.event.wifi.mplink.STATE_CHANGE";

/**
 * Indicates the action of a common event that the Wi-Fi P2P connection state has changed.
 * To subscribe to this common event, your application must have the ohos.permission.GET_WIFI_INFO and
 * ohos.permission.LOCATION permissions.
 */
const std::string CommonEventSupport::COMMON_EVENT_WIFI_P2P_CONN_STATE = "usual.event.wifi.p2p.CONN_STATE_CHANGE";

/**
 * Indicates the action of a common event about the Wi-Fi P2P state, such as enabled and disabled.
 * To subscribe to this common event, your application must have the ohos.permission.GET_WIFI_INFO permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_WIFI_P2P_STATE_CHANGED = "usual.event.wifi.p2p.STATE_CHANGE";

/**
 * Indicates that the Wi-Fi P2P peers state change.
 * To subscribe to this common event, your application must have the ohos.permission.GET_WIFI_INFO permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_WIFI_P2P_PEERS_STATE_CHANGED = "usual.event.wifi.p2p.DEVICES_CHANGE";

/**
 * Indicates that the Wi-Fi P2P discovery state change.
 * To subscribe to this common event, your application must have the ohos.permission.GET_WIFI_INFO permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_WIFI_P2P_PEERS_DISCOVERY_STATE_CHANGED =
    "usual.event.wifi.p2p.PEER_DISCOVERY_STATE_CHANGE";

/**
 * Indicates that the Wi-Fi P2P current device state change.
 * To subscribe to this common event, your application must have the ohos.permission.GET_WIFI_INFO permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_WIFI_P2P_CURRENT_DEVICE_STATE_CHANGED =
    "usual.event.wifi.p2p.CURRENT_DEVICE_CHANGE";

/**
 * Indicates that the Wi-Fi P2P group info is changed.
 * To subscribe to this common event, your application must have the ohos.permission.GET_WIFI_INFO permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_WIFI_P2P_GROUP_STATE_CHANGED =
    "usual.event.wifi.p2p.GROUP_STATE_CHANGED";

/**
 * Indicates that network traffic statistics have been updated.
 */
const std::string CommonEventSupport::COMMON_EVENT_NETMANAGER_NETSTATES_UPDATED =
    "usual.event.netmanager.NETSTATES_UPDATED";

/**
 * Indicates that the network traffic has exceeded the limit.
 */
const std::string CommonEventSupport::COMMON_EVENT_NETMANAGER_NETSTATES_LIMITED =
    "usual.event.netmanager.NETSTATES_LIMITED";

/**
 * Indicates the action of a common event about the connection state of Bluetooth handsfree communication.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREE_AG_CONNECT_STATE_UPDATE =
    "usual.event.bluetooth.handsfree.ag.CONNECT_STATE_UPDATE";

/**
 * Indicates the action of a common event that the device connected to the Bluetooth handsfree is active.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREE_AG_CURRENT_DEVICE_UPDATE =
    "usual.event.bluetooth.handsfree.ag.CURRENT_DEVICE_UPDATE";

/**
 * Indicates the action of a common event that the connection state of Bluetooth A2DP has changed.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREE_AG_AUDIO_STATE_UPDATE =
    "usual.event.bluetooth.handsfree.ag.AUDIO_STATE_UPDATE";

/**
 * Indicates the action of a common event about the connection state of Bluetooth A2DP.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CONNECT_STATE_UPDATE =
    "usual.event.bluetooth.a2dpsource.CONNECT_STATE_UPDATE";

/**
 * Indicates the action of a common event that the device connected using Bluetooth A2DP is active.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CURRENT_DEVICE_UPDATE =
    "usual.event.bluetooth.a2dpsource.CURRENT_DEVICE_UPDATE";

/**
 * Indicates the action of a common event that the playing state of Bluetooth A2DP has changed.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSOURCE_PLAYING_STATE_UPDATE =
    "usual.event.bluetooth.a2dpsource.PLAYING_STATE_UPDATE";

/**
 * Indicates the action of a common event that the AVRCP connection state of Bluetooth A2DP has changed.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSOURCE_AVRCP_CONNECT_STATE_UPDATE =
    "usual.event.bluetooth.a2dpsource.AVRCP_CONNECT_STATE_UPDATE";

/**
 * Indicates the action of a common event that the audio codec state of Bluetooth A2DP has changed.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CODEC_VALUE_UPDATE =
    "usual.event.bluetooth.a2dpsource.CODEC_VALUE_UPDATE";

/**
 * Indicates the action of a common event that a remote Bluetooth device has been discovered.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH and
 * ohos.permission.LOCATION permissions.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_DISCOVERED =
    "usual.event.bluetooth.remotedevice.DISCOVERED";

/**
 * Indicates the action of a common event that the Bluetooth class of a remote Bluetooth device has changed.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_CLASS_VALUE_UPDATE =
    "usual.event.bluetooth.remotedevice.CLASS_VALUE_UPDATE";

/**
 * Indicates the action of a common event that a low level (ACL) connection has been established with a remote
 * Bluetooth device.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_ACL_CONNECTED =
    "usual.event.bluetooth.remotedevice.ACL_CONNECTED";

/**
 * Indicates the action of a common event that a low level (ACL) connection has been disconnected from a remote
 * Bluetooth device.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_ACL_DISCONNECTED =
    "usual.event.bluetooth.remotedevice.ACL_DISCONNECTED";

/**
 * Indicates the action of a common event that the friendly name of a remote Bluetooth device has been retrieved for
 * the first time or has been changed since the last retrieval.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_NAME_UPDATE =
    "usual.event.bluetooth.remotedevice.NAME_UPDATE";

/**
 * Indicates the action of a common event that the connection state of a remote Bluetooth device has changed.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_PAIR_STATE =
    "usual.event.bluetooth.remotedevice.PAIR_STATE";

/**
 * Indicates the action of a common event that the battery level of a remote Bluetooth device has been retrieved
 * for the first time or has been changed since the last retrieval.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_BATTERY_VALUE_UPDATE =
    "usual.event.bluetooth.remotedevice.BATTERY_VALUE_UPDATE";

/**
 * Indicates the action of a common event about the SDP state of a remote Bluetooth device.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_SDP_RESULT =
    "usual.event.bluetooth.remotedevice.SDP_RESULT";

/**
 * Indicates the action of a common event about the UUID connection state of a remote Bluetooth device.
 * To subscribe to this common event, your application must have the ohos.permission.DISCOVER_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_UUID_VALUE =
    "usual.event.bluetooth.remotedevice.UUID_VALUE";

/**
 * Indicates the action of a common event about the pairing request from a remote Bluetooth device.
 * To subscribe to this common event, your application must have the ohos.permission.DISCOVER_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_PAIRING_REQ =
    "usual.event.bluetooth.remotedevice.PAIRING_REQ";

/**
 * Indicates the action of a common event that Bluetooth pairing is canceled.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_PAIRING_CANCEL =
    "usual.event.bluetooth.remotedevice.PAIRING_CANCEL";

/**
 * Indicates the action of a common event about the connection request from a remote Bluetooth device.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_CONNECT_REQ =
    "usual.event.bluetooth.remotedevice.CONNECT_REQ";

/**
 * Indicates the action of a common event about the response to the connection request from a remote Bluetooth
 * device.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_CONNECT_REPLY =
    "usual.event.bluetooth.remotedevice.CONNECT_REPLY";

/**
 * Indicates the action of a common event that the connection to a remote Bluetooth device has been canceled.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_CONNECT_CANCEL =
    "usual.event.bluetooth.remotedevice.CONNECT_CANCEL";

/**
 * Indicates the action of a common event that the connection state of a Bluetooth handsfree has changed.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREEUNIT_CONNECT_STATE_UPDATE =
    "usual.event.bluetooth.handsfreeunit.CONNECT_STATE_UPDATE";

/**
 * Indicates the action of a common event that the audio state of a Bluetooth handsfree has changed.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREEUNIT_AUDIO_STATE_UPDATE =
    "usual.event.bluetooth.handsfreeunit.AUDIO_STATE_UPDATE";

/**
 * Indicates the action of a common event that the audio gateway state of a Bluetooth handsfree has changed.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREEUNIT_AG_COMMON_EVENT =
    "usual.event.bluetooth.handsfreeunit.AG_COMMON_EVENT";

/**
 * Indicates the action of a common event that the calling state of a Bluetooth handsfree has changed.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREEUNIT_AG_CALL_STATE_UPDATE =
    "usual.event.bluetooth.handsfreeunit.AG_CALL_STATE_UPDATE";

/**
 * Indicates the action of a common event that the state of a Bluetooth adapter has been changed, for example,
 * Bluetooth has been turned on or off.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE =
    "usual.event.bluetooth.host.STATE_UPDATE";

/**
 * Indicates the action of a common event about the requests for the user to allow Bluetooth to be scanned.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_REQ_DISCOVERABLE =
    "usual.event.bluetooth.host.REQ_DISCOVERABLE";

/**
 * Indicates the action of a common event about the requests for the user to turn on Bluetooth.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_REQ_ENABLE = "usual.event.bluetooth.host.REQ_ENABLE";

/**
 * Indicates the action of a common event about the requests for the user to turn off Bluetooth.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_REQ_DISABLE =
    "usual.event.bluetooth.host.REQ_DISABLE";

/**
 * Indicates the action of a common event that the Bluetooth scanning mode of a device has changed.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_SCAN_MODE_UPDATE =
    "usual.event.bluetooth.host.SCAN_MODE_UPDATE";

/**
 * Indicates the action of a common event that the Bluetooth scanning has been started on the device.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_DISCOVERY_STARTED =
    "usual.event.bluetooth.host.DISCOVERY_STARTED";

/**
 * Indicates the action of a common event that the Bluetooth scanning is finished on the device.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_DISCOVERY_FINISHED =
    "usual.event.bluetooth.host.DISCOVERY_FINISHED";

/**
 * Indicates the action of a common event that the Bluetooth adapter name of the device has changed.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_NAME_UPDATE =
    "usual.event.bluetooth.host.NAME_UPDATE";

/**
 * Indicates the action of a common event that the connection state of Bluetooth A2DP Sink has changed.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSINK_CONNECT_STATE_UPDATE =
    "usual.event.bluetooth.a2dpsink.CONNECT_STATE_UPDATE";

/**
 * Indicates the action of a common event that the playing state of Bluetooth A2DP Sink has changed.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSINK_PLAYING_STATE_UPDATE =
    "usual.event.bluetooth.a2dpsink.PLAYING_STATE_UPDATE";

/**
 * Indicates the action of a common event that the audio state of Bluetooth A2DP Sink has changed.
 * To subscribe to this common event, your application must have the ohos.permission.USE_BLUETOOTH permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSINK_AUDIO_STATE_UPDATE =
    "usual.event.bluetooth.a2dpsink.AUDIO_STATE_UPDATE";

/**
 * Indicates the action of a common event that the state of the device NFC adapter has changed.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_NFC_ACTION_ADAPTER_STATE_CHANGED =
    "usual.event.nfc.action.ADAPTER_STATE_CHANGED";

/**
 * Indicates the action of a common event that the NFC RF field is detected to be in the enabled state.
 * To subscribe to this common event, your application must have the ohos.permission.MANAGE_SECURE_SETTINGS
 * permission.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_NFC_ACTION_RF_FIELD_ON_DETECTED =
    "usual.event.nfc.action.RF_FIELD_ON_DETECTED";

/**
 * Indicates the action of a common event that the NFC RF field is detected to be in the disabled state.
 * To subscribe to this common event, your application must have the ohos.permission.MANAGE_SECURE_SETTINGS
 * permission.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_NFC_ACTION_RF_FIELD_OFF_DETECTED =
    "usual.event.nfc.action.RF_FIELD_OFF_DETECTED";

/**
 * Indicates the action of a common event that the system stops charging the battery.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_DISCHARGING = "usual.event.DISCHARGING";

/**
 * Indicates the action of a common event that the system starts charging the battery.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_CHARGING = "usual.event.CHARGING";

/**
 * Indicates the action of a common event that a charge type has been updated.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_CHARGE_TYPE_CHANGED = "usual.event.CHARGE_TYPE_CHANGED";

/**
 * Indicates the action of a common event that the system idle mode has changed.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_DEVICE_IDLE_MODE_CHANGED = "usual.event.DEVICE_IDLE_MODE_CHANGED";

/**
 * Indicates the action of a common event that the list of exempt applications is updated in the idle mode.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_DEVICE_IDLE_EXEMPTION_LIST_UPDATED =
    "usual.event.DEVICE_IDLE_EXEMPTION_LIST_UPDATED";

/**
 * Indicates the action of a common event that the power save mode of the system has changed.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED = "usual.event.POWER_SAVE_MODE_CHANGED";

/**
 * Indicates the action of a common event that a user has been added to the system.
 * To subscribe to this common event, your application must have the ohos.permission.MANAGE_LOCAL_ACCOUNTS permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_USER_ADDED = "usual.event.USER_ADDED";
/**
 * Indicates the action of a common event that a user has been removed from the system.
 * To subscribe to this common event, your application must have the ohos.permission.MANAGE_LOCAL_ACCOUNTS permission.
 */
const std::string CommonEventSupport::COMMON_EVENT_USER_REMOVED = "usual.event.USER_REMOVED";

/**
 * Indicates the action of a common event that an ability has been added.
 * To subscribe to this common event, your application must have the ohos.permission.LISTEN_BUNDLE_CHANGE
 * permission.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_ABILITY_ADDED = "common.event.ABILITY_ADDED";

/**
 * Indicates the action of a common event that an ability has been removed.
 * To subscribe to this common event, your application must have the ohos.permission.LISTEN_BUNDLE_CHANGE
 * permission.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_ABILITY_REMOVED = "common.event.ABILITY_REMOVED";

/**
 * Indicates the action of a common event that an ability has been updated.
 * To subscribe to this common event, your application must have the ohos.permission.LISTEN_BUNDLE_CHANGE
 * permission.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_ABILITY_UPDATED = "common.event.ABILITY_UPDATED";

/**
 * Indicates the action of a common event that the location mode of the system has changed.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_LOCATION_MODE_STATE_CHANGED =
    "usual.event.location.MODE_STATE_CHANGED";

/**
 * Indicates the action of a common event that the in-vehicle infotainment (IVI) system of a vehicle is sleeping.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_IVI_SLEEP = "common.event.IVI_SLEEP";

/**
 * The ivi is slept and notify the app stop playing.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_IVI_PAUSE = "common.event.IVI_PAUSE";

/**
 * Indicates the action of a common event that a third-party application is instructed to pause the current work.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_IVI_STANDBY = "common.event.IVI_STANDBY";

/**
 * Indicates the action of a common event that a third-party application is instructed to save its last mode.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_IVI_LASTMODE_SAVE = "common.event.IVI_LASTMODE_SAVE";

/**
 * Indicates the action of a common event that the voltage of the vehicle power system is abnormal.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_IVI_VOLTAGE_ABNORMAL = "common.event.IVI_VOLTAGE_ABNORMAL";

/**
 * The ivi temperature is too high.
 * This is a protected common event that can only be sent by system.
 * This common event will be delete later, please use COMMON_EVENT_IVI_TEMPERATURE_ABNORMAL.
 */
const std::string CommonEventSupport::COMMON_EVENT_IVI_HIGH_TEMPERATURE = "common.event.IVI_HIGH_TEMPERATURE";

/**
 * The ivi temperature is extreme high.
 * This is a protected common event that can only be sent by system.
 * This common event will be delete later, please use COMMON_EVENT_IVI_TEMPERATURE_ABNORMAL.
 */
const std::string CommonEventSupport::COMMON_EVENT_IVI_EXTREME_TEMPERATURE = "common.event.IVI_EXTREME_TEMPERATURE";

/**
 * Indicates the action of a common event that the in-vehicle system has an extreme temperature.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_IVI_TEMPERATURE_ABNORMAL = "common.event.IVI_TEMPERATURE_ABNORMAL";

/**
 * Indicates the action of a common event that the voltage of the vehicle power system is restored to normal.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_IVI_VOLTAGE_RECOVERY = "common.event.IVI_VOLTAGE_RECOVERY";

/**
 * Indicates the action of a common event that the temperature of the in-vehicle system is restored to normal.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_IVI_TEMPERATURE_RECOVERY = "common.event.IVI_TEMPERATURE_RECOVERY";

/**
 * Indicates the action of a common event that the battery service is active.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_IVI_ACTIVE = "common.event.IVI_ACTIVE";

/**
 * The usb state changed.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_USB_STATE = "usual.event.hardware.usb.action.USB_STATE";

/**
 * The usb port changed.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_USB_PORT_CHANGED =
    "usual.event.hardware.usb.action.USB_PORT_CHANGED";

/**
 * Indicates the action of a common event that a USB device has been attached when the user device functions as a USB
 * host.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_USB_DEVICE_ATTACHED =
    "usual.event.hardware.usb.action.USB_DEVICE_ATTACHED";

/**
 * Indicates the action of a common event that a USB device has been detached when the user device functions as a USB
 * host.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_USB_DEVICE_DETACHED =
    "usual.event.hardware.usb.action.USB_DEVICE_DETACHED";

/**
 * Indicates the action of a common event that a USB accessory has been attached.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_USB_ACCESSORY_ATTACHED =
    "usual.event.hardware.usb.action.USB_ACCESSORY_ATTACHED";

/**
 * Indicates the action of a common event that a USB accessory has been detached.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_USB_ACCESSORY_DETACHED =
    "usual.event.hardware.usb.action.USB_ACCESSORY_DETACHED";

/**
 * The storage space is low.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_LOW = "usual.event.DEVICE_STORAGE_LOW";

/**
 * The storage space is normal.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_OK = "usual.event.DEVICE_STORAGE_OK";

/**
 * The storage space is full.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_FULL = "usual.event.DEVICE_STORAGE_FULL";

/**
 * The network connection was changed.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE = "usual.event.CONNECTIVITY_CHANGE";

/**
 * The global http proxy was changed.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_HTTP_PROXY_CHANGE = "usual.event.HTTP_PROXY_CHANGE";

/**
 * Indicates the action of a common event that an external storage device was removed.
 * To subscribe to this common event, your application must have the ohos.permission.WRITE_USER_STORAGE or
 * ohos.permission.STORAGE_MANAGER permission.
 * This common event can be published only by system applications.
 */
const std::string CommonEventSupport::COMMON_EVENT_DISK_REMOVED = "usual.event.data.DISK_REMOVED";

/**
 * Indicates the action of a common event that an external storage device was unmounted.
 * To subscribe to this common event, your application must have the ohos.permission.WRITE_USER_STORAGE or
 * ohos.permission.STORAGE_MANAGER permission.
 * This common event can be published only by system applications.
 */
const std::string CommonEventSupport::COMMON_EVENT_DISK_UNMOUNTED = "usual.event.data.DISK_UNMOUNTED";

/**
 * Indicates the action of a common event that an external storage device was mounted.
 * To subscribe to this common event, your application must have the ohos.permission.WRITE_USER_STORAGE or
 * ohos.permission.STORAGE_MANAGER permission.
 * This common event can be published only by system applications.
 */
const std::string CommonEventSupport::COMMON_EVENT_DISK_MOUNTED = "usual.event.data.DISK_MOUNTED";

/**
 * Indicates the action of a common event that an external storage device was removed without being unmounted.
 * To subscribe to this common event, your application must have the ohos.permission.WRITE_USER_STORAGE or
 * ohos.permission.STORAGE_MANAGER permission.
 * This common event can be published only by system applications.
 */
const std::string CommonEventSupport::COMMON_EVENT_DISK_BAD_REMOVAL = "usual.event.data.DISK_BAD_REMOVAL";

/**
 * Indicates the action of a common event that an external storage device becomes unmountable.
 * To subscribe to this common event, your application must have the ohos.permission.WRITE_USER_STORAGE or
 * ohos.permission.STORAGE_MANAGER permission.
 * This common event can be published only by system applications.
 */
const std::string CommonEventSupport::COMMON_EVENT_DISK_UNMOUNTABLE = "usual.event.data.DISK_UNMOUNTABLE";

/**
 * Indicates the action of a common event that an external storage device was ejected.
 * To subscribe to this common event, your application must have the ohos.permission.WRITE_USER_STORAGE or
 * ohos.permission.STORAGE_MANAGER permission.
 * This common event can be published only by system applications.
 */
const std::string CommonEventSupport::COMMON_EVENT_DISK_EJECT = "usual.event.data.DISK_EJECT";

/**
 * Indicates the action of a common event that an external storage device was removed.
 * To subscribe to this common event, your application must have the ohos.permission.WRITE_USER_STORAGE or
 * ohos.permission.STORAGE_MANAGER permission.
 * This common event can be published only by system applications.
 */
const std::string CommonEventSupport::COMMON_EVENT_VOLUME_REMOVED = "usual.event.data.VOLUME_REMOVED";

/**
 * Indicates the action of a common event that an external storage device was unmounted.
 * To subscribe to this common event, your application must have the ohos.permission.WRITE_USER_STORAGE or
 * ohos.permission.STORAGE_MANAGER permission.
 * This common event can be published only by system applications.
 */
const std::string CommonEventSupport::COMMON_EVENT_VOLUME_UNMOUNTED = "usual.event.data.VOLUME_UNMOUNTED";

/**
 * Indicates the action of a common event that an external storage device was mounted.
 * To subscribe to this common event, your application must have the ohos.permission.WRITE_USER_STORAGE or
 * ohos.permission.STORAGE_MANAGER permission.
 * This common event can be published only by system applications.
 */
const std::string CommonEventSupport::COMMON_EVENT_VOLUME_MOUNTED = "usual.event.data.VOLUME_MOUNTED";

/**
 * Indicates the action of a common event that an external storage device was removed without being unmounted.
 * To subscribe to this common event, your application must have the ohos.permission.WRITE_USER_STORAGE or
 * ohos.permission.STORAGE_MANAGER permission.
 * This common event can be published only by system applications.
 */
const std::string CommonEventSupport::COMMON_EVENT_VOLUME_BAD_REMOVAL = "usual.event.data.VOLUME_BAD_REMOVAL";

/**
 * Indicates the action of a common event that an external storage device was ejected.
 * To subscribe to this common event, your application must have the ohos.permission.WRITE_USER_STORAGE or
 * ohos.permission.STORAGE_MANAGER permission.
 * This common event can be published only by system applications.
 */
const std::string CommonEventSupport::COMMON_EVENT_VOLUME_EJECT = "usual.event.data.VOLUME_EJECT";

/**
 * Indicates the action of a common event that the account visible changed.
 * To subscribe to this common event, your application must have the ohos.permission.GET_APP_ACCOUNTS permission.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_VISIBLE_ACCOUNTS_UPDATED =
    "usual.event.data.VISIBLE_ACCOUNTS_UPDATED";

/**
 * Indicates the action of a common event that the account is deleted.
 * To subscribe to this common event, your application must have the ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS
 * permission.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_ACCOUNT_DELETED = "usual.event.data.ACCOUNT_DELETED";

/**
 * Indicates the action of a common event that the foundation is ready.
 * To subscribe to this common event, your application must have the ohos.permission.RECEIVER_STARTUP_COMPLETED
 * permission.
 * This is a protected common event, which can be sent only by the system.
 */
const std::string CommonEventSupport::COMMON_EVENT_FOUNDATION_READY = "common.event.FOUNDATION_READY";

/**
 * Indicates the action of a common event that the default voice subscription has changed.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_SIM_CARD_DEFAULT_VOICE_SUBSCRIPTION_CHANGED =
    "usual.event.SIM.DEFAULT_VOICE_SUBSCRIPTION_CHANGED";

/**
 * Indicates the action of a common event that the phone SIM card state has changed.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_SIM_STATE_CHANGED = "usual.event.SIM_STATE_CHANGED";

/**
 * Indicates the action of a common event that the airplane mode of the device has changed.
 * This common event can be triggered only by system applications.
 */
const std::string CommonEventSupport::COMMON_EVENT_AIRPLANE_MODE_CHANGED = "usual.event.AIRPLANE_MODE";

/**
 * Indicates the action of a common event that a new sms bas been received by the device.
 * To subscribe to this common event, your application must have the ohos.permission.RECEIVE_SMS permission.
 * This common event can be triggered only by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_SMS_RECEIVE_COMPLETED = "usual.event.SMS_RECEIVE_COMPLETED";

/**
 * Indicates the action of a common event that a new sms emergency cell broadcast bas been received by the device.
 * This common event can be triggered only by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_SMS_EMERGENCY_CB_RECEIVE_COMPLETED =
    "usual.event.SMS_EMERGENCY_CB_RECEIVE_COMPLETED";

/**
 * Indicates the action of a common event that a new sms normal cell broadcast bas been received by the device.
 * This common event can be triggered only by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_SMS_CB_RECEIVE_COMPLETED = "usual.event.SMS_CB_RECEIVE_COMPLETED";

/**
 * Indicates the action of a common event that a STK command has been received by the device.
 * This common event can be triggered only by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_STK_COMMAND = "usual.event.STK_COMMAND";

/**
 * Indicates the action of a common event that STK session end.
 * This common event can be triggered only by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_STK_SESSION_END = "usual.event.STK_SESSION_END";

/**
 * Indicates the action of a common event that the STK phone card state has changed.
 * This common event can be triggered only by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_STK_CARD_STATE_CHANGED = "usual.event.STK_CARD_STATE_CHANGED";

/**
 * Indicates the action of a common event that an alpha string during call control  has been received by the device.
 * This common event can be triggered only by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_STK_ALPHA_IDENTIFIER = "usual.event.STK_ALPHA_IDENTIFIER";

/**
 * Indicates the action of a common event that the spn display information has been updated.
 * This common event can be triggered only by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_SPN_INFO_CHANGED = "usual.event.SPN_INFO_CHANGED";

/**
 * Indicates the action of a common event that the NITZ time has been updated.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_NITZ_TIME_CHANGED = "usual.event.NITZ_TIME_CHANGED";

/**
 * Indicates the action of a common event that the NITZ time zone has been updated.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_NITZ_TIMEZONE_CHANGED = "usual.event.NITZ_TIMEZONE_CHANGED";

/**
 * Indicates the action of a common event that a new sms wappush has been received by the device.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_SMS_WAPPUSH_RECEIVE_COMPLETED =
    "usual.event.SMS_WAPPUSH_RECEIVE_COMPLETED";

/**
 * Indicates the action of a common event that the operator config has been updated.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_OPERATOR_CONFIG_CHANGED = "usual.event.OPERATOR_CONFIG_CHANGED";

/**
 * Indicates the action of a common event that the notification slot has been updated.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_SLOT_CHANGE = "usual.event.SLOT_CHANGE";

/**
 * Only for test case.
 */
const std::string CommonEventSupport::COMMON_EVENT_TEST_ACTION1 = "usual.event.test1";

/**
 * Only for test case.
 */
const std::string CommonEventSupport::COMMON_EVENT_TEST_ACTION2 = "usual.event.test2";

/**
 * Indicates the action of a common event that the default SMS subscription has
 * been changed. This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_SIM_CARD_DEFAULT_SMS_SUBSCRIPTION_CHANGED =
    "usual.event.SIM.DEFAULT_SMS_SUBSCRIPTION_CHANGED";

/**
 * Indicates the action of a common event that the default data subscription has been changed.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_SIM_CARD_DEFAULT_DATA_SUBSCRIPTION_CHANGED =
    "usual.event.SIM.DEFAULT_DATA_SUBSCRIPTION_CHANGED";

/**
 * Indicates the action of a common event that the default main subscription has been changed.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_SIM_CARD_DEFAULT_MAIN_SUBSCRIPTION_CHANGED =
    "usual.event.SIM.DEFAULT_MAIN_SUBSCRIPTION_CHANGED";

/**
 * Indicates the action of a common event that the call state has been changed.
 * To subscribe to this protected common event, your application must have the ohos.permission.GET_TELEPHONY_STATE
 * permission.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED = "usual.event.CALL_STATE_CHANGED";

/**
 * Indicates the action of a common event that the cellular data state has been changed.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_CELLULAR_DATA_STATE_CHANGED =
    "usual.event.CELLULAR_DATA_STATE_CHANGED";

/**
 * Indicates the action of a common event that the network state has been changed.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_NETWORK_STATE_CHANGED = "usual.event.NETWORK_STATE_CHANGED";

/**
 * Indicates the action of a common event that the signal info has been changed.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_SIGNAL_INFO_CHANGED = "usual.event.SIGNAL_INFO_CHANGED";

/**
 * Indicates the action of a common event that the incoming call has been missed.
 * To subscribe to this protected common event, your application must have the ohos.permission.GET_TELEPHONY_STATE
 * permission.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_INCOMING_CALL_MISSED = "usual.event.INCOMING_CALL_MISSED";

/**
 * Indicate the result of quick fix apply.
 * This common event can be triggered only by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_QUICK_FIX_APPLY_RESULT = "usual.event.QUICK_FIX_APPLY_RESULT";

/**
 * Indicate the result of quick fix revoke.
 * This common event can be triggered only by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_QUICK_FIX_REVOKE_RESULT = "usual.event.QUICK_FIX_REVOKE_RESULT";

/**
 * Indicates the action of a common event that radio state change.
 * To subscribe to this protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_RADIO_STATE_CHANGE = "usual.event.RADIO_STATE_CHANGE";

/**
 * Indicates the action of a common event about a login of a distributed account.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGIN = "common.event.DISTRIBUTED_ACCOUNT_LOGIN";

/**
 * Indicates the action of a common event about a logout of a distributed account.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGOUT =
    "common.event.DISTRIBUTED_ACCOUNT_LOGOUT";

/**
 * Indicates the action of a common event that the token of a distributed account is invalid.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_TOKEN_INVALID =
    "common.event.DISTRIBUTED_ACCOUNT_TOKEN_INVALID";

/**
 * Indicates the action of a common event about a logoff of a distributed account.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGOFF =
    "common.event.DISTRIBUTED_ACCOUNT_LOGOFF";

/**
 * Indicates the action of a common event that the os account information has been updated.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_USER_INFO_UPDATED = "usual.event.USER_INFO_UPDATED";

/**
 * Indicate the action of a common event that domain account status has been changed.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_DOMAIN_ACCOUNT_STATUS_CHANGED =
    "usual.event.DOMAIN_ACCOUNT_STATUS_CHANGED";

/**
 * Indicates the action of a common event that the screen lock.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_SCREEN_LOCKED = "usual.event.SCREEN_LOCKED";

/**
 * Indicates the action of a common event that the screen unlock.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED = "usual.event.SCREEN_UNLOCKED";

/**
 * Indicates the action of a common event that the call audio quality information has been updated.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_AUDIO_QUALITY_CHANGE = "usual.event.AUDIO_QUALITY_CHANGE";

/**
 * Indicates the action of a common event about special code.
 * This is a protected common event that can only be sent by system.
 */
const std::string CommonEventSupport::COMMON_EVENT_SPECIAL_CODE = "common.event.SPECIAL_CODE";

CommonEventSupport::CommonEventSupport()
{
    Init();
}

CommonEventSupport::~CommonEventSupport() {}

void CommonEventSupport::Init()
{
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_LOCKED_BOOT_COMPLETED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_SHUTDOWN);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BATTERY_LOW);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BATTERY_OKAY);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_THERMAL_LEVEL_CHANGED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_CHARGE_IDLE_MODE_CHANGED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_USER_PRESENT);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_SPLIT_SCREEN);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_TIME_TICK);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_TIME_CHANGED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_DATE_CHANGED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_TIMEZONE_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_CLOSE_SYSTEM_DIALOGS);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BUNDLE_SCAN_FINISHED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_PACKAGE_REPLACED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_MY_PACKAGE_REPLACED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BUNDLE_REMOVED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_PACKAGE_FULLY_REMOVED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_PACKAGE_RESTARTED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_PACKAGE_CACHE_CLEARED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_PACKAGES_SUSPENDED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_PACKAGES_UNSUSPENDED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_MY_PACKAGE_SUSPENDED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_MY_PACKAGE_UNSUSPENDED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_UID_REMOVED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_PACKAGE_FIRST_LAUNCH);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_PACKAGE_NEEDS_VERIFICATION);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_PACKAGE_VERIFIED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_EXTERNAL_APPLICATIONS_AVAILABLE);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_EXTERNAL_APPLICATIONS_UNAVAILABLE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_CONFIGURATION_CHANGED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_LOCALE_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_DRIVE_MODE);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_HOME_MODE);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_OFFICE_MODE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_HWID_LOGIN);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_HWID_LOGOUT);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_HWID_TOKEN_INVALID);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_HWID_LOGOFF);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_NFC_ACTION_ADAPTER_STATE_CHANGED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_NFC_ACTION_RF_FIELD_ON_DETECTED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_NFC_ACTION_RF_FIELD_OFF_DETECTED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_DISCHARGING);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_CHARGING);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_CHARGE_TYPE_CHANGED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_DEVICE_IDLE_MODE_CHANGED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_DEVICE_IDLE_EXEMPTION_LIST_UPDATED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_ABILITY_ADDED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_ABILITY_REMOVED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_ABILITY_UPDATED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_LOCATION_MODE_STATE_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_IVI_SLEEP);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_IVI_PAUSE);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_IVI_STANDBY);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_IVI_LASTMODE_SAVE);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_IVI_VOLTAGE_ABNORMAL);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_IVI_HIGH_TEMPERATURE);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_IVI_EXTREME_TEMPERATURE);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_IVI_TEMPERATURE_ABNORMAL);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_IVI_VOLTAGE_RECOVERY);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_IVI_TEMPERATURE_RECOVERY);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_IVI_ACTIVE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_USB_STATE);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_USB_PORT_CHANGED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_USB_DEVICE_ATTACHED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_USB_DEVICE_DETACHED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_USB_ACCESSORY_ATTACHED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_USB_ACCESSORY_DETACHED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_LOW);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_OK);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_FULL);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_HTTP_PROXY_CHANGE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_DISK_REMOVED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_DISK_UNMOUNTED);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_DISK_MOUNTED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_DISK_BAD_REMOVAL);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_DISK_UNMOUNTABLE);
    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_DISK_EJECT);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_VISIBLE_ACCOUNTS_UPDATED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_ACCOUNT_DELETED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_FOUNDATION_READY);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_SIM_CARD_DEFAULT_VOICE_SUBSCRIPTION_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_SIM_STATE_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_AIRPLANE_MODE_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_SMS_RECEIVE_COMPLETED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_SMS_EMERGENCY_CB_RECEIVE_COMPLETED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_SMS_CB_RECEIVE_COMPLETED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_STK_COMMAND);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_STK_SESSION_END);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_STK_CARD_STATE_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_STK_ALPHA_IDENTIFIER);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_SPN_INFO_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_NITZ_TIME_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_NITZ_TIMEZONE_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_NETMANAGER_NETSTATES_UPDATED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_NETMANAGER_NETSTATES_LIMITED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_SMS_WAPPUSH_RECEIVE_COMPLETED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_OPERATOR_CONFIG_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_SLOT_CHANGE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_SIM_CARD_DEFAULT_SMS_SUBSCRIPTION_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_SIM_CARD_DEFAULT_DATA_SUBSCRIPTION_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_SIM_CARD_DEFAULT_MAIN_SUBSCRIPTION_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_CELLULAR_DATA_STATE_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_SIGNAL_INFO_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_NETWORK_STATE_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_INCOMING_CALL_MISSED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_QUICK_FIX_APPLY_RESULT);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_QUICK_FIX_REVOKE_RESULT);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_RADIO_STATE_CHANGE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGIN);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGOUT);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGOFF);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_TOKEN_INVALID);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_MANAGE_PACKAGE_STORAGE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_USER_STARTED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_USER_BACKGROUND);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_USER_FOREGROUND);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_USER_SWITCHED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_USER_STARTING);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_USER_UNLOCKED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_USER_STOPPING);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_USER_STOPPED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_WIFI_SCAN_FINISHED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_WIFI_RSSI_VALUE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_WIFI_CONN_STATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_WIFI_HOTSPOT_STATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_WIFI_AP_STA_JOIN);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_WIFI_AP_STA_LEAVE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_WIFI_MPLINK_STATE_CHANGE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_WIFI_P2P_CONN_STATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_WIFI_P2P_STATE_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_WIFI_P2P_PEERS_STATE_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_WIFI_P2P_PEERS_DISCOVERY_STATE_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_WIFI_P2P_CURRENT_DEVICE_STATE_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_WIFI_P2P_GROUP_STATE_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREE_AG_CONNECT_STATE_UPDATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREE_AG_CURRENT_DEVICE_UPDATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREE_AG_AUDIO_STATE_UPDATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CONNECT_STATE_UPDATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CURRENT_DEVICE_UPDATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSOURCE_PLAYING_STATE_UPDATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSOURCE_AVRCP_CONNECT_STATE_UPDATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CODEC_VALUE_UPDATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_DISCOVERED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_CLASS_VALUE_UPDATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_ACL_CONNECTED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_ACL_DISCONNECTED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_NAME_UPDATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_PAIR_STATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_BATTERY_VALUE_UPDATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_SDP_RESULT);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_UUID_VALUE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_PAIRING_REQ);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_PAIRING_CANCEL);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_CONNECT_REQ);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_CONNECT_REPLY);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_CONNECT_CANCEL);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREEUNIT_CONNECT_STATE_UPDATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREEUNIT_AUDIO_STATE_UPDATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREEUNIT_AG_COMMON_EVENT);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREEUNIT_AG_CALL_STATE_UPDATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_REQ_DISCOVERABLE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_REQ_ENABLE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_REQ_DISABLE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_SCAN_MODE_UPDATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_DISCOVERY_STARTED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_DISCOVERY_FINISHED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_NAME_UPDATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSINK_CONNECT_STATE_UPDATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSINK_PLAYING_STATE_UPDATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSINK_AUDIO_STATE_UPDATE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_USER_ADDED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_USER_REMOVED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_VOLUME_REMOVED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_VOLUME_UNMOUNTED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_VOLUME_MOUNTED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_VOLUME_BAD_REMOVAL);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_VOLUME_EJECT);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_USER_INFO_UPDATED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_DOMAIN_ACCOUNT_STATUS_CHANGED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_SCREEN_LOCKED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_AUDIO_QUALITY_CHANGE);

    commonEventSupport_.emplace_back(CommonEventSupport::COMMON_EVENT_SPECIAL_CODE);

    return;
}

bool CommonEventSupport::IsSystemEvent(std::string& str)
{
    std::vector<std::string>::iterator iter = find(commonEventSupport_.begin(), commonEventSupport_.end(), str);
    if (iter != commonEventSupport_.end()) {
        return true;
    }
    return false;
}
} // namespace Plugin
} // namespace OHOS
