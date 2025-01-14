/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_BLUETOOTH_COMMON_MOCK_HITRACE_METER_H
#define PLUGINS_BLUETOOTH_COMMON_MOCK_HITRACE_METER_H

#include <string>
#ifdef HITRACE_UNITTEST
#include "param/sys_param.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif

constexpr uint64_t HITRACE_TAG_NEVER = 0;                         // This tag is never enabled.
constexpr uint64_t HITRACE_TAG_ALWAYS = (1ULL << 0);              // This tag is always enabled.
constexpr uint64_t HITRACE_TAG_DRM = (1ULL << 6);                 // Media subsystem Digital Rights Management tag.
constexpr uint64_t HITRACE_TAG_SECURITY = (1ULL << 7);            // Security subsystem tag.
constexpr uint64_t HITRACE_TAG_CLOUD_FILE = (1ULL << 8);          // Cloud file system tag.
constexpr uint64_t HITRACE_TAG_ANIMATION = (1ULL << 9);           // Animation tag.
constexpr uint64_t HITRACE_TAG_PUSH = (1ULL << 10);               // Push subsystem tag.
constexpr uint64_t HITRACE_TAG_VIRSE = (1ULL << 11);              // virtualization service.
constexpr uint64_t HITRACE_TAG_MUSL = (1ULL << 12);               // musl module.
constexpr uint64_t HITRACE_TAG_FFRT = (1ULL << 13);               // ffrt tasks.
constexpr uint64_t HITRACE_TAG_CLOUD = (1ULL << 14);              // Cloud subsystem tag.
constexpr uint64_t HITRACE_TAG_DEV_AUTH = (1ULL << 15);           // Device auth module tag.
constexpr uint64_t HITRACE_TAG_COMMONLIBRARY = (1ULL << 16);      // Commonlibrary subsystem tag.
constexpr uint64_t HITRACE_TAG_HDCD = (1ULL << 17);               // HDCD tag.
constexpr uint64_t HITRACE_TAG_HDF = (1ULL << 18);                // HDF subsystem tag.
constexpr uint64_t HITRACE_TAG_USB = (1ULL << 19);                // USB subsystem tag.
constexpr uint64_t HITRACE_TAG_INTERCONNECTION = (1ULL << 20);    // Interconnection subsystem tag.
constexpr uint64_t HITRACE_TAG_DLP_CREDENTIAL = (1ULL << 21);     // This tag is dlp credential service.
constexpr uint64_t HITRACE_TAG_ACCESS_CONTROL = (1ULL << 22);     // This tag is access control tag.
constexpr uint64_t HITRACE_TAG_NET = (1ULL << 23);                // Net tag.
constexpr uint64_t HITRACE_TAG_NWEB = (1ULL << 24);               // NWeb tag.
constexpr uint64_t HITRACE_TAG_HUKS = (1ULL << 25);               // This tag is huks.
constexpr uint64_t HITRACE_TAG_USERIAM = (1ULL << 26);            // This tag is useriam.
constexpr uint64_t HITRACE_TAG_DISTRIBUTED_AUDIO = (1ULL << 27);  // Distributed audio tag.
constexpr uint64_t HITRACE_TAG_DLSM = (1ULL << 28);               // device security level tag.
constexpr uint64_t HITRACE_TAG_FILEMANAGEMENT = (1ULL << 29);     // filemanagement tag.
constexpr uint64_t HITRACE_TAG_OHOS = (1ULL << 30);               // OHOS generic tag.
constexpr uint64_t HITRACE_TAG_ABILITY_MANAGER = (1ULL << 31);    // Ability Manager tag.
constexpr uint64_t HITRACE_TAG_ZCAMERA = (1ULL << 32);            // Camera module tag.
constexpr uint64_t HITRACE_TAG_ZMEDIA = (1ULL << 33);             // Media module tag.
constexpr uint64_t HITRACE_TAG_ZIMAGE = (1ULL << 34);             // Image module tag.
constexpr uint64_t HITRACE_TAG_ZAUDIO = (1ULL << 35);             // Audio module tag.
constexpr uint64_t HITRACE_TAG_DISTRIBUTEDDATA = (1ULL << 36);    // Distributeddata manager module tag.
constexpr uint64_t HITRACE_TAG_MDFS = (1ULL << 37);               // Mobile distributed file system tag.
constexpr uint64_t HITRACE_TAG_GRAPHIC_AGP = (1ULL << 38);        // Graphic module tag.
constexpr uint64_t HITRACE_TAG_ACE = (1ULL << 39);                // ACE development framework tag.
constexpr uint64_t HITRACE_TAG_NOTIFICATION = (1ULL << 40);       // Notification module tag.
constexpr uint64_t HITRACE_TAG_MISC = (1ULL << 41);               // Notification module tag.
constexpr uint64_t HITRACE_TAG_MULTIMODALINPUT = (1ULL << 42);    // Multi modal module tag.
constexpr uint64_t HITRACE_TAG_SENSORS = (1ULL << 43);            // Sensors mudule tag.
constexpr uint64_t HITRACE_TAG_MSDP = (1ULL << 44);               // Multimodal Sensor Data Platform module tag.
constexpr uint64_t HITRACE_TAG_DSOFTBUS = (1ULL << 45);           // Distributed Softbus tag.
constexpr uint64_t HITRACE_TAG_RPC = (1ULL << 46);                // RPC and IPC tag.
constexpr uint64_t HITRACE_TAG_ARK = (1ULL << 47);                // ARK tag.
constexpr uint64_t HITRACE_TAG_WINDOW_MANAGER = (1ULL << 48);     // window manager tag.
constexpr uint64_t HITRACE_TAG_ACCOUNT_MANAGER = (1ULL << 49);    // account manager tag.
constexpr uint64_t HITRACE_TAG_DISTRIBUTED_SCREEN = (1ULL << 50); // Distributed screen tag.
constexpr uint64_t HITRACE_TAG_DISTRIBUTED_CAMERA = (1ULL << 51); // Distributed camera tag.
constexpr uint64_t HITRACE_TAG_DISTRIBUTED_HARDWARE_FWK = (1ULL << 52); // Distributed hardware fwk tag.
constexpr uint64_t HITRACE_TAG_GLOBAL_RESMGR = (1ULL << 53);            // Global resource manager tag.
constexpr uint64_t HITRACE_TAG_DEVICE_MANAGER = (1ULL << 54);           // Distributed hardware devicemanager tag.
constexpr uint64_t HITRACE_TAG_SAMGR = (1ULL << 55);                    // SA tag.
constexpr uint64_t HITRACE_TAG_POWER = (1ULL << 56);                    // power manager tag.
constexpr uint64_t HITRACE_TAG_DISTRIBUTED_SCHEDULE = (1ULL << 57);     // Distributed schedule tag.
constexpr uint64_t HITRACE_TAG_DEVICE_PROFILE = (1ULL << 58);           // device profile tag.
constexpr uint64_t HITRACE_TAG_DISTRIBUTED_INPUT = (1ULL << 59);        // Distributed input tag.
constexpr uint64_t HITRACE_TAG_BLUETOOTH = (1ULL << 60);                // bluetooth tag.
constexpr uint64_t HITRACE_TAG_APP = (1ULL << 62);                      // App tag.
constexpr uint64_t HITRACE_TAG_LAST = HITRACE_TAG_APP;
constexpr uint64_t HITRACE_TAG_NOT_READY = (1ULL << 63); // Reserved for initialization.
constexpr uint64_t HITRACE_TAG_VALID_MASK = ((HITRACE_TAG_LAST - 1) | HITRACE_TAG_LAST);
constexpr uint64_t HITRACE_TAG_COMMERCIAL = (1ULL << 5); // Tag for commercial version.

#ifndef HITRACE_TAG
#define HITRACE_TAG HITRACE_TAG_NEVER
#elif HITRACE_TAG > HITRACE_TAG_VALID_MASK
#error HITRACE_TAG must be defined to be one of the tags defined in hitrace_meter.h
#elif HITRACE_TAG < HITRACE_TAG_OHOS
#error HITRACE_TAG must be defined to be one of the tags defined in hitrace_meter.h
#endif

#define TOKENPASTE(x, y) x##y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define HITRACE_METER_NAME(TAG, str) HitraceScoped TOKENPASTE2(tracer, __LINE__)(TAG, str)
#define HITRACE_METER(TAG) HITRACE_METER_NAME(TAG, __func__)
#define HITRACE_METER_FMT(TAG, fmt, ...) HitraceMeterFmtScoped TOKENPASTE2(tracer, __LINE__)(TAG, fmt, ##__VA_ARGS__)

void StartTrace(uint64_t label, const std::string& value, float limit = -1) {}
void FinishTrace(uint64_t label) {}

class HitraceScoped {
public:
    inline HitraceScoped(uint64_t tag, const std::string& value) : mTag(tag)
    {
        StartTrace(mTag, value);
    }

    inline ~HitraceScoped()
    {
        FinishTrace(mTag);
    }

private:
    uint64_t mTag;
};
#ifdef __cplusplus
}
#endif
#endif // PLUGINS_BLUETOOTH_COMMON_MOCK_HITRACE_METER_H
