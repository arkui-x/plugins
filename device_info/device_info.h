/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_DEVICE_INFO_H
#define PLUGINS_DEVICE_INFO_H

#include <memory>
#include <string>
#include <map>

#include "plugin_utils.h"

#ifdef __cplusplus
extern "C" {
#endif
/* GetMajorVersion 开始为int返回值的，如果string类型返回，需要在这个前面添加 */
#define DEVICE_INFO_METHOD_MAP(XX)                                                                                     \
    XX(getDeviceType)                                                                                                  \
    XX(getManufacture)                                                                                                 \
    XX(getBrand)                                                                                                       \
    XX(getMarketName)                                                                                                  \
    XX(getProductSeries)                                                                                               \
    XX(getProductModel)                                                                                                \
    XX(getProductModelAlias)                                                                                           \
    XX(getSoftwareModel)                                                                                               \
    XX(getHardwareModel)                                                                                               \
    XX(getHardwareProfile)                                                                                             \
    XX(getBootLoaderVersion)                                                                                           \
    XX(getAbiList)                                                                                                     \
    XX(getSecurityPatchTag)                                                                                            \
    XX(getDisplayVersion)                                                                                              \
    XX(getIncrementalVersion)                                                                                          \
    XX(getOsReleaseType)                                                                                               \
    XX(getOSFullName)                                                                                                  \
    XX(getVersionId)                                                                                                   \
    XX(getBuildType)                                                                                                   \
    XX(getBuildUser)                                                                                                   \
    XX(getBuildHost)                                                                                                   \
    XX(getBuildTime)                                                                                                   \
    XX(getBuildRootHash)                                                                                               \
    XX(getMajorVersion)                                                                                                \
    XX(getSeniorVersion)                                                                                               \
    XX(getFeatureVersion)                                                                                              \
    XX(getBuildVersion)                                                                                                \
    XX(getSdkApiVersion)                                                                                               \
    XX(getFirstApiVersion)

typedef enum {
#define XX(name) METHOD_ID_##name,
    DEVICE_INFO_METHOD_MAP(XX)
#undef XX
        MAX_METHOD_ID
} DeviceInfoMethodId;

static const struct {
    DeviceInfoMethodId id;
    const char *methodName;
} METHOD_NAMES[] = {
#define XX(name) {METHOD_ID_##name, #name},
    DEVICE_INFO_METHOD_MAP(XX)
#undef XX
};

#ifdef __cplusplus
}
#endif

namespace OHOS::Plugin {
static const std::string g_defaultValue("default");

class DeviceInfo {
public:
    DeviceInfo() = default;
    virtual ~DeviceInfo() = default;
    static std::unique_ptr<DeviceInfo> Create();

    virtual const std::string GetDeviceInfo(int id, const std::string &defValue) = 0;
    virtual int GetDeviceInfo(int id, int defValue) = 0;
};

class SdkVersionInfo : public DeviceInfo {
public:
    SdkVersionInfo() = default;
    ~SdkVersionInfo() = default;
    static std::unique_ptr<DeviceInfo> Create();

    const std::string GetDeviceInfo(int id, const std::string &defValue) override;
    int GetDeviceInfo(int id, int defValue) override;

private:
    int GetMajorVersion(void);
    int GetSeniorVersion(void);
    int GetFeatureVersion(void);
    int GetBuildVersion(void);
    const std::string GetOsReleaseType(void);
    const std::string GetOSFullName(void);
    const std::string GetBuildRootHash(void);
    const std::string GetSecurityPatchTag(void);

    const std::string GetBuildType(void);
    const std::string GetBuildUser(void);
    const std::string GetBuildHost(void);
    const std::string GetBuildTime(void);
    int GetFirstApiVersion(void);
    int GetSdkApiVersion(void);

    void Init(void);
    const char *GetParameter(const char *key, const char *def);
    int majorVersion {0};
    int seniorVersion {0};
    int featureVersion {0};
    int buildVersion {0};
    int sdkApiVersion {-1};
    int firstApiVersion {-1};
    char *fullName_ {nullptr};
    using SystemGetSdkIntInfoFunc =
        int32_t (SdkVersionInfo::*)(void);
    using SystemGetSdkStringInfoFunc =
        const std::string (SdkVersionInfo::*)(void);
    std::map<uint32_t, SystemGetSdkIntInfoFunc> memberFuncMap_ {};
    std::map<uint32_t, SystemGetSdkStringInfoFunc> memberStringFuncMap_ {};
};
}  // namespace OHOS::Plugin
#endif  // PLUGINS_DEVICE_INFO_H
