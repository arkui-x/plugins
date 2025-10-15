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

#ifndef LOCATION_BLUETOOTH_SCAN_RESULT_H
#define LOCATION_BLUETOOTH_SCAN_RESULT_H

#include <parcel.h>
#include <string>
#include "string_ex.h"

namespace OHOS {
namespace Location {
class BluetoothScanResult : public Parcelable {
public:
    BluetoothScanResult();
    explicit BluetoothScanResult(BluetoothScanResult &bluetoothScanResult);
    ~BluetoothScanResult() override;

    inline std::string GetDeviceId() const
    {
        return deviceId_;
    }

    inline void SetDeviceId(std::string deviceId)
    {
        deviceId_ = deviceId;
    }

    inline std::string GetDeviceName() const
    {
        return deviceName_;
    }

    inline void SetDeviceName(std::string deviceName)
    {
        deviceName_ = deviceName;
    }

    inline int64_t GetRssi() const
    {
        return rssi_;
    }

    inline void SetRssi(int64_t rssi)
    {
        rssi_ = rssi;
    }

    inline std::vector<uint8_t> GetData() const
    {
        return data_;
    }

    inline void SetData(std::vector<uint8_t> data)
    {
        for (auto it = data.begin(); it != data.end(); ++it) {
            data_.push_back(*it);
        }
    }

    inline bool GetConnectable() const
    {
        return connectable_;
    }

    inline void SetConnectable(bool connectable)
    {
        connectable_ = connectable;
    }

    bool Marshalling(Parcel& parcel) const override;

private:
    std::string deviceId_;
    std::string deviceName_;
    int64_t rssi_;
    std::vector<uint8_t> data_;
    bool connectable_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_BLUETOOTH_SCAN_RESULT_H