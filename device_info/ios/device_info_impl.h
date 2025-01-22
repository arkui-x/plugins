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

#ifndef PLUGINS_DEVICEINFO_IOS_DEVICEINFO_IMPL_H
#define PLUGINS_DEVICEINFO_IOS_DEVICEINFO_IMPL_H

#include <memory>

#include "plugins/device_info/device_info.h"

namespace OHOS::Plugin {
class DeviceInfoImpl final : public DeviceInfo {
public:
    DeviceInfoImpl() = default;
    ~DeviceInfoImpl() override = default;

    const std::string GetDeviceInfo(int id, const std::string &defValue) override;
    int GetDeviceInfo(int id, int defValue) override;
private:
    const std::string GetOSFullName(void);
    const std::string GetDeviceType(void);
    const std::string GetManufacture(void);
    const std::string GetBrand(void);
    const std::string GetMarketName(void);
    const std::string GetProductSeries(void);
    const std::string GetProductModel(void);
    const std::string GetProductModelAlias(void);
    const std::string GetSoftwareModel(void);
    const std::string GetHardwareModel(void);
    const std::string GetBootLoaderVersion(const std::string &def);
    const std::string GetAbiList(const std::string &def);
    const std::string GetDisplayVersion(void);
    const std::string GetIncrementalVersion(const std::string &def);
    const std::string GetHardwareProfile(const std::string &def);
    int GetFirstApiVersion(int def);
};
} // namespace OHOS::Plugin
#endif // PLUGINS_DEVICEINFO_IOS_DEVICEINFO_IMPL_H
