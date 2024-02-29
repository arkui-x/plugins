/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef OHOS_WIFI_MSG_H
#define OHOS_WIFI_MSG_H

#include <algorithm>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace OHOS::Plugin {
#define INVALID_NETWORK_ID (-1)

enum class SupplicantState {
    DISCONNECTED = 0,
    INTERFACE_DISABLED = 1,
    INACTIVE = 2,
    SCANNING = 3,
    AUTHENTICATING = 4,
    ASSOCIATING = 5,
    ASSOCIATED = 6,
    FOUR_WAY_HANDSHAKE = 7,
    GROUP_HANDSHAKE = 8,
    COMPLETED = 9,
    UNKNOWN = 10,

    INVALID = 0xFF,
};

enum class WifiChannelWidth {
    WIDTH_20MHZ = 0,
    WIDTH_40MHZ = 1,
    WIDTH_80MHZ = 2,
    WIDTH_160MHZ = 3,
    WIDTH_80MHZ_PLUS = 4,
    WIDTH_INVALID
};

enum class DetailedState {
    AUTHENTICATING = 0,
    BLOCKED = 1,
    CAPTIVE_PORTAL_CHECK = 2,
    CONNECTED = 3,
    CONNECTING = 4,
    DISCONNECTED = 5,
    DISCONNECTING = 6,
    FAILED = 7,
    IDLE = 8,
    OBTAINING_IPADDR = 9,
    WORKING = 10,
    NOTWORKING = 11,
    SCANNING = 12,
    SUSPENDED = 13,
    VERIFYING_POOR_LINK = 14,
    PASSWORD_ERROR = 15,
    CONNECTION_REJECT = 16,
    CONNECTION_FULL = 17,
    CONNECTION_TIMEOUT = 18,
    OBTAINING_IPADDR_FAIL = 19,
    INVALID = 0xFF,
};

enum ConnState {
    /** The device is searching for an available AP. */
    SCANNING,

    /** The Wi-Fi connection is being set up. */
    CONNECTING,

    /** The Wi-Fi connection is being authenticated. */
    AUTHENTICATING,

    /** The IP address of the Wi-Fi connection is being obtained. */
    OBTAINING_IPADDR,

    /** The Wi-Fi connection has been set up. */
    CONNECTED,

    /** The Wi-Fi connection is being torn down. */
    DISCONNECTING,

    /** The Wi-Fi connection has been torn down. */
    DISCONNECTED,

    /** Failed to set up the Wi-Fi connection. */
    UNKNOWN
};

struct WifiLinkedInfo {
    int networkId;
    std::string ssid;
    std::string bssid;
    int rssi; /* signal level */
    int band; /* 2.4G / 5G */
    int frequency;
    int linkSpeed; /* units: Mbps */
    std::string macAddress;
    int macType;
    unsigned int ipAddress;
    ConnState connState;
    bool ifHiddenSSID;
    int rxLinkSpeed; /* Downstream network speed */
    int txLinkSpeed; /* Upstream network speed */
    int chload;
    int snr; /* Signal-to-Noise Ratio */
    int isDataRestricted;
    std::string platformType;
    std::string portalUrl;
    SupplicantState supplicantState; /* wpa_supplicant state */
    DetailedState detailedState;     /* connection state */
    int wifiStandard;                /* wifi standard */
    int maxSupportedRxLinkSpeed;
    int maxSupportedTxLinkSpeed;
    WifiChannelWidth channelWidth; /* curr ap channel width */
    int lastPacketDirection;
    int lastRxPackets;
    int lastTxPackets;
    int retryedConnCount;
    bool isAncoConnected;
    WifiLinkedInfo()
    {
        networkId = INVALID_NETWORK_ID;
        rssi = 0;
        band = 0;
        frequency = 0;
        linkSpeed = 0;
        macType = 0;
        ipAddress = 0;
        connState = ConnState::UNKNOWN;
        ifHiddenSSID = false;
        rxLinkSpeed = 0;
        txLinkSpeed = 0;
        chload = 0;
        snr = 0;
        isDataRestricted = 0;
        supplicantState = SupplicantState::INVALID;
        detailedState = DetailedState::INVALID;
        wifiStandard = 0;
        maxSupportedRxLinkSpeed = 0;
        maxSupportedTxLinkSpeed = 0;
        channelWidth = WifiChannelWidth::WIDTH_INVALID;
        lastPacketDirection = 0;
        lastRxPackets = 0;
        lastTxPackets = 0;
        retryedConnCount = 0;
        isAncoConnected = false;
    }
};
} // namespace OHOS::Plugin

#endif
