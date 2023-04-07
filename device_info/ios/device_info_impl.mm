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

#include "plugins/device_info/ios/device_info_impl.h"

#include "log.h"
#include "plugin_utils.h"
#include <sys/sysctl.h>
#include <sys/utsname.h>
#import <UIKit/UIKit.h>

namespace OHOS::Plugin {
std::unique_ptr<DeviceInfo> DeviceInfo::Create()
{
    return std::make_unique<DeviceInfoImpl>();
}

const std::string DeviceInfoImpl::GetDeviceType(void)
{
    UIDevice *device=[UIDevice currentDevice];
    LOGE("GetDefaultDisplay called.");
    NSLog(@"name: %@", device.name);
    NSLog(@"model: %@", device.model);
    NSLog(@"localizedModel: %@", device.localizedModel);
    NSLog(@"systemName: %@", device.systemName);
    NSLog(@"systemVersion: %@", device.systemVersion);

    NSString *value = @"iMac";
    if (device.userInterfaceIdiom == UIUserInterfaceIdiomPhone) {
        value = @"iPhone";
    } else if (device.userInterfaceIdiom == UIUserInterfaceIdiomPad) {
        value = @"iPad";
    } else if (device.userInterfaceIdiom == UIUserInterfaceIdiomTV) {
        value = @"iTv";
    } else if (device.userInterfaceIdiom == UIUserInterfaceIdiomCarPlay) {
        value = @"iCarPlay";
    } else if (device.userInterfaceIdiom == UIUserInterfaceIdiomTV) {
        value = @"iTv";
    }
    NSLog(@"GetDeviceType: %@", value);
    LOGE("GetDeviceTyped.", value);
    return [value cStringUsingEncoding : NSUTF8StringEncoding];
}

const std::string DeviceInfoImpl::GetManufacture(void)
{
    NSString *value = @"apple";
    return [value cStringUsingEncoding : NSUTF8StringEncoding];
}

const std::string DeviceInfoImpl::GetBrand(void)
{
    // name: iPod touch (7th generation)
    NSString* value = [[UIDevice currentDevice] name];
    return [value cStringUsingEncoding : NSUTF8StringEncoding];
}

const std::string DeviceInfoImpl::GetMarketName(void)
{
    // name: iPod touch (7th generation)
    NSString* value = [[UIDevice currentDevice] name];
    return [value cStringUsingEncoding : NSUTF8StringEncoding];
}

const std::string DeviceInfoImpl::GetProductSeries(void)
{
    struct utsname systemInfo;
    uname(&systemInfo);
    NSString *value = [NSString stringWithCString:systemInfo.version encoding:NSUTF8StringEncoding];
    return [value cStringUsingEncoding : NSUTF8StringEncoding];
}

const std::string DeviceInfoImpl::GetProductModel(void)
{
    NSString* value = [[UIDevice currentDevice] model];
    return [value cStringUsingEncoding : NSUTF8StringEncoding];
}

const std::string DeviceInfoImpl::GetSoftwareModel(void)
{
    NSString* value = [[UIDevice currentDevice] model];
    return [value cStringUsingEncoding : NSUTF8StringEncoding];
}

const std::string DeviceInfoImpl::GetHardwareModel(void)
{
    NSString* value = [[UIDevice currentDevice] model];
    return [value cStringUsingEncoding : NSUTF8StringEncoding];
}

const std::string DeviceInfoImpl::GetBootLoaderVersion(void)
{
    NSString* value = [[UIDevice currentDevice] systemName];
    return [value cStringUsingEncoding : NSUTF8StringEncoding];
}

const std::string DeviceInfoImpl::GetAbiList(void)
{
    struct utsname systemInfo;
    uname(&systemInfo);
    // x86_64
    NSString *value = [NSString stringWithCString:systemInfo.machine encoding:NSUTF8StringEncoding];
    return [value cStringUsingEncoding : NSUTF8StringEncoding];
}

const std::string DeviceInfoImpl::GetDisplayVersion(void)
{
    struct utsname systemInfo;
    uname(&systemInfo);
    // Darwin Kernel Version 20.1.0: Sat Oct 31 00:07:11 PDT 2020
    return systemInfo.version;
}

const std::string DeviceInfoImpl::GetIncrementalVersion(void)
{
    NSString* value = [[UIDevice currentDevice] systemName];
    return [value cStringUsingEncoding : NSUTF8StringEncoding];
}

const std::string DeviceInfoImpl::GetHardwareProfile(void)
{
    NSString* value = [[UIDevice currentDevice] systemName];
    return [value cStringUsingEncoding : NSUTF8StringEncoding];
}

int DeviceInfoImpl::GetFirstApiVersion(int def)
{
    return def;
}

const std::string DeviceInfoImpl::GetDeviceInfo(int id, const std::string &defValue)
{
    switch(id) {
        case METHOD_ID_getDeviceType:
            return GetDeviceType();
        case METHOD_ID_getManufacture:
            return GetManufacture();
        case METHOD_ID_getBrand:
            return GetBrand();
        case METHOD_ID_getMarketName:
            return GetMarketName();
        case METHOD_ID_getProductSeries:
            return GetProductSeries();
        case METHOD_ID_getProductModel:
            return GetProductModel();
        case METHOD_ID_getSoftwareModel:
            return GetSoftwareModel();
        case METHOD_ID_getHardwareModel:
            return GetHardwareModel();
        case METHOD_ID_getHardwareProfile:
            return GetHardwareProfile();
        case METHOD_ID_getBootLoaderVersion:
            return GetBootLoaderVersion();
        case METHOD_ID_getAbiList:
            return GetAbiList();
        case METHOD_ID_getDisplayVersion:
            return GetDisplayVersion();
        case METHOD_ID_getIncrementalVersion:
            return GetIncrementalVersion();
        default:
            return defValue;
    }
}

int DeviceInfoImpl::GetDeviceInfo(int id, int defValue)
{
    switch(id) {
        case METHOD_ID_getFirstApiVersion:
            return GetFirstApiVersion(defValue);
        default:
            return defValue;
    }
}
}  // namespace OHOS::Plugin
