/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <cstdint>
#include <cstdlib>

#include "log.h"
#include "plugin_utils.h"
#include "securec.h"
#include "etc/parameter_map.h"
#include "plugins/device_info/device_info.h"

using namespace OHOS::Plugin;

std::unique_ptr<DeviceInfo> SdkVersionInfo::Create()
{
    auto device = std::make_unique<SdkVersionInfo>();
    device->Init();
    return device;
}

void SdkVersionInfo::Init(void)
{
    fullName_ = const_cast<char *>(GetParameter("const.ohos.fullname", "default-0.0.0.0"));
    if (fullName_ == NULL) {
        LOGE("Invalid full name.");
        return;
    }
    const char *tmp = strstr(fullName_, "-");
    if (tmp == NULL) {
        LOGE("Invalid full name %s.", fullName_);
        return;
    }
    tmp++;  // skip "-"
    int ret = sscanf_s(tmp, "%d.%d.%d.%d", &majorVersion, &seniorVersion, &featureVersion, &buildVersion);
    if (ret != 4) {  // must 4 parameter
        LOGE("Failed to get sdk version from full name %s.", fullName_);
        fullName_ = nullptr;
    }

    memberFuncMap_[METHOD_ID_getMajorVersion] = &SdkVersionInfo::GetMajorVersion;
    memberFuncMap_[METHOD_ID_getSeniorVersion] = &SdkVersionInfo::GetSeniorVersion;
    memberFuncMap_[METHOD_ID_getFeatureVersion] = &SdkVersionInfo::GetFeatureVersion;
    memberFuncMap_[METHOD_ID_getBuildVersion] = &SdkVersionInfo::GetBuildVersion;
    memberFuncMap_[METHOD_ID_getSdkApiVersion] = &SdkVersionInfo::GetSdkApiVersion;
    memberFuncMap_[METHOD_ID_getFirstApiVersion] = &SdkVersionInfo::GetFirstApiVersion;

    memberStringFuncMap_[METHOD_ID_getOsReleaseType] = &SdkVersionInfo::GetOsReleaseType;
    memberStringFuncMap_[METHOD_ID_getBuildType] = &SdkVersionInfo::GetBuildType;
    memberStringFuncMap_[METHOD_ID_getBuildUser] = &SdkVersionInfo::GetBuildUser;
    memberStringFuncMap_[METHOD_ID_getBuildHost] = &SdkVersionInfo::GetBuildHost;
    memberStringFuncMap_[METHOD_ID_getBuildTime] = &SdkVersionInfo::GetBuildTime;
    memberStringFuncMap_[METHOD_ID_getBuildRootHash] = &SdkVersionInfo::GetBuildRootHash;
    memberStringFuncMap_[METHOD_ID_getSecurityPatchTag] = &SdkVersionInfo::GetSecurityPatchTag;
    return;
}

const char *SdkVersionInfo::GetParameter(const char *key, const char *def)
{
    size_t len = sizeof(g_paramDefCfgNodes) / sizeof(g_paramDefCfgNodes[0]);
    for (size_t i = 0; i < len; i++) {
        LOGD("GetParameter name %s value %s", g_paramDefCfgNodes[i].name, g_paramDefCfgNodes[i].value);
        if (strcmp(g_paramDefCfgNodes[i].name, key) == 0) {
            return g_paramDefCfgNodes[i].value;
        }
    }
    return def;
}

int SdkVersionInfo::GetMajorVersion(void)
{
    Init();
    return majorVersion;
}

int SdkVersionInfo::GetSeniorVersion(void)
{
    Init();
    return seniorVersion;
}

int SdkVersionInfo::GetFeatureVersion(void)
{
    Init();
    return featureVersion;
}

int SdkVersionInfo::GetBuildVersion(void)
{
    Init();
    return buildVersion;
}

int SdkVersionInfo::GetSdkApiVersion(void)
{
    if (sdkApiVersion == -1) {
        const char *version = GetParameter("const.ohos.apiversion", "-1");
        sdkApiVersion = atoi(version);
    }
    return sdkApiVersion;
}

const std::string SdkVersionInfo::GetOsReleaseType(void)
{
    return GetParameter("const.ohos.releasetype", "Canary1");
}

const std::string SdkVersionInfo::GetBuildRootHash(void)
{
#ifdef BUILD_ROOTHASH
    return BUILD_ROOTHASH;
#else
    return GetParameter("const.ohos.buildroothash", "default");
#endif
}

const std::string SdkVersionInfo::GetSecurityPatchTag(void)
{
    return GetParameter("const.ohos.version.security_patch", "default");
}

const std::string SdkVersionInfo::GetBuildType(void)
{
#ifdef BUILD_TYPE
    return BUILD_TYPE;
#else
    return g_defaultValue;
#endif
}
const std::string SdkVersionInfo::GetBuildUser(void)
{
#ifdef BUILD_USER
    return BUILD_USER;
#else
    return GetParameter("const.product.build.user", g_defaultValue.c_str());
#endif
}
const std::string SdkVersionInfo::GetBuildHost(void)
{
#ifdef BUILD_HOST
    return BUILD_HOST;
#else
    return g_defaultValue;
#endif
}
const std::string SdkVersionInfo::GetBuildTime(void)
{
#ifdef BUILD_TIME
    return BUILD_TIME;
#else
    return GetParameter("const.product.build.date", g_defaultValue.c_str());
#endif
}
int SdkVersionInfo::GetFirstApiVersion(void)
{
    if (firstApiVersion == -1) {
        const char *version = GetParameter("const.product.firstapiversion", "-1");
        firstApiVersion = atoi(version);
    }
    return firstApiVersion;
}

const std::string SdkVersionInfo::GetDeviceInfo(int id, const std::string &defValue)
{
    auto itFunc = memberStringFuncMap_.find(id);
    if (itFunc != memberStringFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)();
        }
    }
    return defValue;
}

int SdkVersionInfo::GetDeviceInfo(int id, int defValue)
{
    auto itFunc = memberFuncMap_.find(id);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)();
        }
    }
    return defValue;
}
