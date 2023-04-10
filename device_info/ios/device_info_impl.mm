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
#import <mach/mach.h>
#import <mach-o/arch.h>
#import <UIKit/UIKit.h>

namespace OHOS::Plugin {
std::unique_ptr<DeviceInfo> DeviceInfo::Create()
{
    return std::make_unique<DeviceInfoImpl>();
}

//    [[UIDevice currentDevice] systemName]; // ϵͳ��
//    [[UIDevice currentDevice] systemVersion]; //�汾��
//    [[UIDevice currentDevice] model]; //���ͣ�ģ���������
//    [[UIDevice currentDevice] uniqueIdentifier]; //Ψһʶ����
//    [[UIDevice currentDevice] name]; //�豸����
//    [[UIDevice currentDevice] localizedModel]; // ����ģʽ

const std::string DeviceInfoImpl::GetDeviceType(void)
{
    UIDevice *device=[UIDevice currentDevice];
    LOGE("GetDefaultDisplay called.");
    NSLog(@"name: %@", device.name);
    NSLog(@"model: %@", device.model);
    NSLog(@"localizedModel: %@", device.localizedModel);
    NSLog(@"systemName: %@", device.systemName);
    NSLog(@"systemVersion: %@", device.systemVersion);
    NSLog(@"identifierForVendor: %@", device.identifierForVendor);

    NSString *value = @"Unknown";
    if (device.userInterfaceIdiom == UIUserInterfaceIdiomPhone) {
        value = @"Handset";
    } else if (device.userInterfaceIdiom == UIUserInterfaceIdiomPad) {
        value = @"Tablet";
    } else if (device.userInterfaceIdiom == UIUserInterfaceIdiomTV) {
        value = @"Tv";
    }
    NSLog(@"GetDeviceType: %@", value);
    LOGE("GetDeviceTyped.", value);
    return [value cStringUsingEncoding : NSUTF8StringEncoding];
}

const std::string DeviceInfoImpl::GetManufacture(void)
{
    NSString *value = @"Apple";
    return [value cStringUsingEncoding : NSUTF8StringEncoding];
}

const std::string DeviceInfoImpl::GetBrand(void)
{
    NSString *value = @"Apple";
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

const std::string DeviceInfoImpl::GetAbiList(const std::string &def)
{
    const NXArchInfo *info = NXGetLocalArchInfo();
    if (info) {
        NSString *typeOfCpu = [NSString stringWithUTF8String:info->description];
        return [typeOfCpu cStringUsingEncoding : NSUTF8StringEncoding];
    } else {
        return def;
    }
}

const std::string DeviceInfoImpl::GetDisplayVersion(void)
{
    NSString* value = [[UIDevice currentDevice] systemVersion];
    return [value cStringUsingEncoding : NSUTF8StringEncoding];
}

const std::string DeviceInfoImpl::GetIncrementalVersion(void)
{
    NSString* value = [[UIDevice currentDevice] systemName];
    return [value cStringUsingEncoding : NSUTF8StringEncoding];
}

const std::string DeviceInfoImpl::GetHardwareProfile(const std::string &def)
{
    return def;
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
            return GetHardwareProfile(defValue);
        case METHOD_ID_getBootLoaderVersion:
            return GetBootLoaderVersion();
        case METHOD_ID_getAbiList:
            return GetAbiList(defValue);
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
