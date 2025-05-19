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

#include "log.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "plugin_utils.h"
#include "securec.h"
#include "plugins/device_info/device_info.h"

#ifdef ANDROID_PLATFORM
#include "plugins/device_info/android/java/jni/device_info_jni.h"
#endif

namespace OHOS::Plugin {
static napi_value CreateDeviceTypes(napi_env env, napi_value exports)
{
    napi_value deviceTypes = nullptr;
    napi_value typeDefault = nullptr;
    napi_value typePhone = nullptr;
    napi_value typeTablet = nullptr;
    napi_value type2in1 = nullptr;
    napi_value typeTv = nullptr;
    napi_value typeWearable = nullptr;
    napi_value typeCar = nullptr;

    napi_create_object(env, &deviceTypes);

    napi_create_string_utf8(env, "default", NAPI_AUTO_LENGTH, &typeDefault);
    napi_create_string_utf8(env, "phone", NAPI_AUTO_LENGTH, &typePhone);
    napi_create_string_utf8(env, "tablet", NAPI_AUTO_LENGTH, &typeTablet);
    napi_create_string_utf8(env, "2in1", NAPI_AUTO_LENGTH, &type2in1);
    napi_create_string_utf8(env, "tv", NAPI_AUTO_LENGTH, &typeTv);
    napi_create_string_utf8(env, "wearable", NAPI_AUTO_LENGTH, &typeWearable);
    napi_create_string_utf8(env, "car", NAPI_AUTO_LENGTH, &typeCar);

    napi_set_named_property(env, deviceTypes, "TYPE_DEFAULT", typeDefault);
    napi_set_named_property(env, deviceTypes, "TYPE_PHONE", typePhone);
    napi_set_named_property(env, deviceTypes, "TYPE_TABLET", typeTablet);
    napi_set_named_property(env, deviceTypes, "TYPE_2IN1", type2in1);
    napi_set_named_property(env, deviceTypes, "TYPE_TV", typeTv);
    napi_set_named_property(env, deviceTypes, "TYPE_WEARABLE", typeWearable);
    napi_set_named_property(env, deviceTypes, "TYPE_CAR", typeCar);

    napi_set_named_property(env, exports, "DeviceTypes", deviceTypes);

    return exports;
}

static napi_value GetDeviceType(napi_env env, napi_callback_info info)
{
    napi_value deviceType = nullptr;
    auto deviceinfo = DeviceInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetDeviceType");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getDeviceType, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &deviceType));
    return deviceType;
}

static napi_value GetManufacture(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = DeviceInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetManufacture");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getManufacture, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &napiValue));
    return napiValue;
}

static napi_value GetBrand(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = DeviceInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetBrand");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getBrand, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &napiValue));
    return napiValue;
}

static napi_value GetMarketName(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = DeviceInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetMarketName");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getMarketName, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &napiValue));
    return napiValue;
}

static napi_value GetProductSeries(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = DeviceInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetProductSeries");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getProductSeries, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &napiValue));
    return napiValue;
}

static napi_value GetProductModel(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = DeviceInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetProductModel");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getProductModel, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &napiValue));
    return napiValue;
}

static napi_value GetProductModelAlias(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = DeviceInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetProductModelAlias");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getProductModelAlias, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &napiValue));
    return napiValue;
}

static napi_value GetSoftwareModel(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = DeviceInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetSoftwareModel");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getSoftwareModel, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &napiValue));
    return napiValue;
}

static napi_value GetHardwareModel(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = DeviceInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetHardwareModel");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getHardwareModel, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &napiValue));
    return napiValue;
}

static napi_value GetHardwareProfile(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = DeviceInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetHardwareProfile");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getHardwareProfile, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &napiValue));
    return napiValue;
}

static napi_value GetBootLoaderVersion(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = DeviceInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetBootLoaderVersion");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getBootLoaderVersion, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &napiValue));
    return napiValue;
}

static napi_value GetAbiList(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = DeviceInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetAbiList");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getAbiList, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &napiValue));
    return napiValue;
}

static napi_value GetSecurityPatchTag(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = SdkVersionInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetSecurityPatchTag");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getSecurityPatchTag, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &napiValue));
    return napiValue;
}

static napi_value GetDisplayVersion(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = DeviceInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetDisplayVersion");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getDisplayVersion, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &napiValue));
    return napiValue;
}

static napi_value GetIncrementalVersion(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = DeviceInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetIncrementalVersion");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getIncrementalVersion, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &napiValue));
    return napiValue;
}

static napi_value GetOsReleaseType(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = SdkVersionInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetOsReleaseType");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getOsReleaseType, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &napiValue));
    return napiValue;
}

static napi_value GetOSFullName(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = DeviceInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetOSFullName");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getOSFullName, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &napiValue));
    return napiValue;
}

static napi_value GetMajorVersion(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = SdkVersionInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetMajorVersion");
    int value = deviceinfo->GetDeviceInfo(METHOD_ID_getMajorVersion, 0);
    NAPI_CALL(env, napi_create_int32(env, value, &napiValue));
    return napiValue;
}

static napi_value GetSeniorVersion(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = SdkVersionInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetSeniorVersion");
    int value = deviceinfo->GetDeviceInfo(METHOD_ID_getSeniorVersion, 0);
    NAPI_CALL(env, napi_create_int32(env, value, &napiValue));
    return napiValue;
}

static napi_value GetFeatureVersion(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = SdkVersionInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetFeatureVersion");
    int value = deviceinfo->GetDeviceInfo(METHOD_ID_getFeatureVersion, 0);
    NAPI_CALL(env, napi_create_int32(env, value, &napiValue));
    return napiValue;
}

static napi_value GetBuildVersion(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = SdkVersionInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetBuildVersion");
    int value = deviceinfo->GetDeviceInfo(METHOD_ID_getBuildVersion, 0);
    NAPI_CALL(env, napi_create_int32(env, value, &napiValue));
    return napiValue;
}

static napi_value GetSdkApiVersion(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = SdkVersionInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetSdkApiVersion");
    int value = deviceinfo->GetDeviceInfo(METHOD_ID_getSdkApiVersion, 0);
    NAPI_CALL(env, napi_create_int32(env, value, &napiValue));
    return napiValue;
}

static napi_value GetFirstApiVersion(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;

    auto sdkVersion = SdkVersionInfo::Create();
    NAPI_ASSERT(env, sdkVersion != nullptr, "deviceinfo is null for GetFirstApiVersion");
    int firstApiVersion = sdkVersion->GetDeviceInfo(METHOD_ID_getFirstApiVersion, 0);

    auto deviceinfo = DeviceInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetFirstApiVersion");
    int value = deviceinfo->GetDeviceInfo(METHOD_ID_getFirstApiVersion, firstApiVersion);
    NAPI_CALL(env, napi_create_int32(env, value, &napiValue));
    return napiValue;
}

static napi_value GetVersionId(napi_env env, napi_callback_info info)
{
    const int MAX_LEN = 256;
    napi_value napiValue = nullptr;
    auto deviceinfo = DeviceInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetVersionId");
    auto deviceinfoSdk = SdkVersionInfo::Create();
    NAPI_ASSERT(env, deviceinfoSdk != nullptr, "deviceinfo is null for GetVersionId");
    char buffer[MAX_LEN] = {0};
    int len = snprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1, "%s/%s/%s/%s/%s/%s/%s/%d/%s/%s",
        deviceinfo->GetDeviceInfo(METHOD_ID_getDeviceType, g_defaultValue).c_str(),
        deviceinfo->GetDeviceInfo(METHOD_ID_getManufacture, g_defaultValue).c_str(),
        deviceinfo->GetDeviceInfo(METHOD_ID_getBrand, g_defaultValue).c_str(),
        deviceinfo->GetDeviceInfo(METHOD_ID_getProductSeries, g_defaultValue).c_str(),
        deviceinfoSdk->GetDeviceInfo(METHOD_ID_getOSFullName, g_defaultValue).c_str(),
        deviceinfo->GetDeviceInfo(METHOD_ID_getProductModel, g_defaultValue).c_str(),
        deviceinfo->GetDeviceInfo(METHOD_ID_getSoftwareModel, g_defaultValue).c_str(),
        deviceinfoSdk->GetDeviceInfo(METHOD_ID_getSdkApiVersion, 0),
        deviceinfo->GetDeviceInfo(METHOD_ID_getIncrementalVersion, g_defaultValue).c_str(),
        deviceinfoSdk->GetDeviceInfo(METHOD_ID_getBuildType, g_defaultValue).c_str());
    NAPI_ASSERT(env, len > 0, "Failed to format version id");
    NAPI_CALL(env, napi_create_string_utf8(env, buffer, len, &napiValue));
    return napiValue;
}

static napi_value GetBuildType(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = SdkVersionInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetBuildType");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getBuildType, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &napiValue));
    return napiValue;
}

static napi_value GetBuildUser(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = SdkVersionInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetBuildUser");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getBuildUser, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &napiValue));
    return napiValue;
}

static napi_value GetBuildHost(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = SdkVersionInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetBuildHost");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getBuildHost, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &napiValue));
    return napiValue;
}

static napi_value GetBuildTime(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = SdkVersionInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetBuildTime");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getBuildTime, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &napiValue));
    return napiValue;
}

static napi_value GetBuildRootHash(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    auto deviceinfo = SdkVersionInfo::Create();
    NAPI_ASSERT(env, deviceinfo != nullptr, "deviceinfo is null for GetBuildRootHash");
    std::string value = deviceinfo->GetDeviceInfo(METHOD_ID_getBuildRootHash, g_defaultValue);
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.length(), &napiValue));
    return napiValue;
}

EXTERN_C_START
/*
 * Module init
 */
static napi_value Init(napi_env env, napi_value exports)
{
    /*
     * Attribute definition
     */
    napi_property_descriptor desc[] = {
        {"deviceType", nullptr, nullptr, GetDeviceType, nullptr, nullptr, napi_default, nullptr},
        {"manufacture", nullptr, nullptr, GetManufacture, nullptr, nullptr, napi_default, nullptr},
        {"brand", nullptr, nullptr, GetBrand, nullptr, nullptr, napi_default, nullptr},
        {"marketName", nullptr, nullptr, GetMarketName, nullptr, nullptr, napi_default, nullptr},
        {"productSeries", nullptr, nullptr, GetProductSeries, nullptr, nullptr, napi_default, nullptr},
        {"productModel", nullptr, nullptr, GetProductModel, nullptr, nullptr, napi_default, nullptr},
        {"productModelAlias", nullptr, nullptr, GetProductModelAlias, nullptr, nullptr, napi_default, nullptr},
        {"softwareModel", nullptr, nullptr, GetSoftwareModel, nullptr, nullptr, napi_default, nullptr},
        {"hardwareModel", nullptr, nullptr, GetHardwareModel, nullptr, nullptr, napi_default, nullptr},
        {"hardwareProfile", nullptr, nullptr, GetHardwareProfile, nullptr, nullptr, napi_default, nullptr},
        {"bootloaderVersion", nullptr, nullptr, GetBootLoaderVersion, nullptr, nullptr, napi_default, nullptr},
        {"abiList", nullptr, nullptr, GetAbiList, nullptr, nullptr, napi_default, nullptr},
        {"securityPatchTag", nullptr, nullptr, GetSecurityPatchTag, nullptr, nullptr, napi_default, nullptr},
        {"displayVersion", nullptr, nullptr, GetDisplayVersion, nullptr, nullptr, napi_default, nullptr},
        {"incrementalVersion", nullptr, nullptr, GetIncrementalVersion, nullptr, nullptr, napi_default, nullptr},
        {"osReleaseType", nullptr, nullptr, GetOsReleaseType, nullptr, nullptr, napi_default, nullptr},
        {"osFullName", nullptr, nullptr, GetOSFullName, nullptr, nullptr, napi_default, nullptr},
        {"majorVersion", nullptr, nullptr, GetMajorVersion, nullptr, nullptr, napi_default, nullptr},
        {"seniorVersion", nullptr, nullptr, GetSeniorVersion, nullptr, nullptr, napi_default, nullptr},
        {"featureVersion", nullptr, nullptr, GetFeatureVersion, nullptr, nullptr, napi_default, nullptr},
        {"buildVersion", nullptr, nullptr, GetBuildVersion, nullptr, nullptr, napi_default, nullptr},
        {"sdkApiVersion", nullptr, nullptr, GetSdkApiVersion, nullptr, nullptr, napi_default, nullptr},
        {"firstApiVersion", nullptr, nullptr, GetFirstApiVersion, nullptr, nullptr, napi_default, nullptr},
        {"versionId", nullptr, nullptr, GetVersionId, nullptr, nullptr, napi_default, nullptr},
        {"buildType", nullptr, nullptr, GetBuildType, nullptr, nullptr, napi_default, nullptr},
        {"buildUser", nullptr, nullptr, GetBuildUser, nullptr, nullptr, napi_default, nullptr},
        {"buildHost", nullptr, nullptr, GetBuildHost, nullptr, nullptr, napi_default, nullptr},
        {"buildTime", nullptr, nullptr, GetBuildTime, nullptr, nullptr, napi_default, nullptr},
        {"buildRootHash", nullptr, nullptr, GetBuildRootHash, nullptr, nullptr, napi_default, nullptr},
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc));
    CreateDeviceTypes(env, exports);
    LOGI("RegisterModule deviceinfo Init.");
    return exports;
}
EXTERN_C_END

/*
 * Module definition
 */
static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = NULL,
    .nm_register_func = Init,
    .nm_modname = "deviceInfo",
    .nm_priv = ((void *)0),
    .reserved = { 0 }
};

#ifdef ANDROID_PLATFORM
static void DeviceInfoPluginJniRegister()
{
    const char className[] = "ohos.ace.plugin.device_infoplugin.DeviceInfoPlugin";
    LOGI("DeviceInfoPluginJniRegister deviceinfo %s.", className);
    ARKUI_X_Plugin_RegisterJavaPlugin(&DeviceInfoJni::Register, className);
}
#endif
/*
 * Module registration function
 */
extern "C" __attribute__((constructor)) void DeviceInfoRegisterModule(void)
{
    LOGI("DeviceInfoRegisterModule deviceinfo.");
#ifdef ANDROID_PLATFORM
    DeviceInfoPluginJniRegister();
#endif
    napi_module_register(&_module);
}
} // namespace OHOS::Plugin
