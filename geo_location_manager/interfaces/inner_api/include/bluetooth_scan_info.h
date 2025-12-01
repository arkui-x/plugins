/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef LOCATION_BLUETOOTH_SCAN_INFO_H
#define LOCATION_BLUETOOTH_SCAN_INFO_H

#include <parcel.h>
#include <string>
#include "string_ex.h"

namespace OHOS {
namespace Location {
class BluetoothScanInfo : public Parcelable {
public:
    BluetoothScanInfo()
    {
        deviceName_ = "";
        mac_ = "";
        rssi_ = 0;
        timestamp_ = 0;
    }

    explicit BluetoothScanInfo(BluetoothScanInfo& bluetoothScanInfo)
    {
        SetDeviceName(bluetoothScanInfo.GetDeviceName());
        SetMac(bluetoothScanInfo.GetMac());
        SetRssi(bluetoothScanInfo.GetRssi());
        SetTimeStamp(bluetoothScanInfo.GetTimeStamp());
    }

    ~BluetoothScanInfo() override = default;

    inline std::string GetMac() const
    {
        return mac_;
    }

    inline void SetMac(std::string mac)
    {
        mac_ = mac;
    }

    inline int64_t GetRssi() const
    {
        return rssi_;
    }

    inline void SetRssi(int64_t rssi)
    {
        rssi_ = rssi;
    }

    inline std::string GetDeviceName() const
    {
        return deviceName_;
    }

    inline void SetDeviceName(std::string deviceName)
    {
        deviceName_ = deviceName;
    }

    inline int64_t GetTimeStamp() const
    {
        return timestamp_;
    }

    inline void SetTimeStamp(int64_t timestamp)
    {
        timestamp_ = timestamp;
    }

    bool Marshalling(Parcel& parcel) const override
    {
        return true;
    }

    std::string ToString()
    {
        std::string str = "deviceName_ : " + deviceName_ +
            ", mac_ : " + mac_ +
            ", rssi_ : " + std::to_string(rssi_) +
            ", timestamp_ : " + std::to_string(timestamp_);
        return str;
    }

private:
    std::string deviceName_;
    std::string mac_;
    int64_t rssi_;
    int64_t timestamp_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_BLUETOOTH_SCAN_INFO_H