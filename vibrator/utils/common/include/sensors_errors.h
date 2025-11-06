/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef SENSORS_ERRORS_H
#define SENSORS_ERRORS_H

#include <errors.h>

#include "hilog/log.h"

#include "miscdevice_log.h"

namespace OHOS {
namespace Sensors {
// Error code for user
enum ErrorCode : int32_t {
    PERMISSION_DENIED = 201, // Use this error code when permission is denied.
    PARAMETER_ERROR = 401, // Use this error code when the input parameter type or range does not match.
    IS_NOT_SUPPORTED = 801, // Use this error code when capability not supported.
    DEVICE_OPERATION_FAILED = 14600101, // Use this error code when operating the device fail.
    CALLBACK_UNSUBSCRIBED = 14600102, // Use this error code when the user callback function is deregistered repeatedly.
};

enum {
    MODULE_COMMON = 0x00,
    MODULE_MISCDEVICE_DEVICE = 0x01,
    MODULE_MISCDEVICE_UTILS = 0x02,
    MODULE_MISCDEVICE_SERVICE = 0x03,
    MODULE_MISCDEVICE_NATIVE = 0X04,
};

// Error code for common
constexpr ErrCode COMMON_ERR_OFFSET = ErrCodeOffset(SUBSYS_SENSORS, MODULE_COMMON);

enum {
    ERROR = -1,
    SUCCESS = 0,
    COMMON_ERR = COMMON_ERR_OFFSET,
};

// Error code for adapter
constexpr ErrCode DEVICE_ERR_OFFSET = ErrCodeOffset(SUBSYS_SENSORS, MODULE_MISCDEVICE_DEVICE);

enum {
    DEVICE_ERR = DEVICE_ERR_OFFSET,
    DEVICE_ENABLE_SENSOR_ERR = DEVICE_ERR_OFFSET + 1,
    DEVICE_DISABLE_SENSOR_ERR = DEVICE_ENABLE_SENSOR_ERR + 1,
    DEVICE_RUN_COMMAND_ERR = DEVICE_DISABLE_SENSOR_ERR + 1,
    DEVICE_SET_SENSOR_CONFIG_ERR = DEVICE_RUN_COMMAND_ERR + 1,
    DEVICE_NOT_SUPPORT_CMD_ERR = DEVICE_SET_SENSOR_CONFIG_ERR + 1,
};

// Error code for miscdevice service
constexpr ErrCode MISCDEVICE_SERVICE_ERR_OFFSET = ErrCodeOffset(SUBSYS_SENSORS, MODULE_MISCDEVICE_SERVICE);

enum {
    LIGHT_HDF_CONNECT_ERR = MISCDEVICE_SERVICE_ERR_OFFSET,
    LIGHT_ID_NOT_SUPPORT = LIGHT_HDF_CONNECT_ERR + 1,
    LIGHT_ERR = LIGHT_ID_NOT_SUPPORT + 1,
    LIGHT_PLAY_EFFECT_ERROR = LIGHT_ERR + 1,
    LIGHT_STOP_EFFECT_ERROR = LIGHT_PLAY_EFFECT_ERROR + 1,
    LIGHT_END_ERROR = LIGHT_STOP_EFFECT_ERROR,
    VIBRATOR_HDF_CONNECT_ERR = LIGHT_END_ERROR + 1,
    VIBRATOR_ON_ERR = VIBRATOR_HDF_CONNECT_ERR + 1,
    VIBRATOR_OFF_ERR = VIBRATOR_ON_ERR + 1,
    VIBRATOR_PLAY_EFFECT_ERR = VIBRATOR_OFF_ERR + 1,
    VIBRATOR_STOP_EFFECT_ERR = VIBRATOR_PLAY_EFFECT_ERR + 1,
    VIBRATOR_SET_PARA_ERR = VIBRATOR_STOP_EFFECT_ERR + 1,
    DUMP_PARAM_ERR = VIBRATOR_SET_PARA_ERR + 1,
    WRITE_MSG_ERR = DUMP_PARAM_ERR + 1,
    READ_MSG_ERR = WRITE_MSG_ERR + 1,
};

// Error code for Sensor native
constexpr ErrCode MISCDEVICE_NATIVE_ERR_OFFSET = ErrCodeOffset(SUBSYS_SENSORS, MODULE_MISCDEVICE_NATIVE);

enum {
    MISCDEVICE_NATIVE_SAM_ERR = MISCDEVICE_NATIVE_ERR_OFFSET,
    MISC_NATIVE_GET_SERVICE_ERR = MISCDEVICE_NATIVE_SAM_ERR + 1,
    MISC_NATIVE_SAM_ERR = MISC_NATIVE_GET_SERVICE_ERR + 1,
    MISC_NO_INIT_ERR = MISC_NATIVE_SAM_ERR + 1,
    MISC_INVALID_OPERATION_ERR = MISC_NO_INIT_ERR + 1,
    MISC_NAME_NOT_FOUND_ERR = MISC_INVALID_OPERATION_ERR + 1,
};

class InnerFunctionTracer {
public:
    InnerFunctionTracer(const char *func, const char *tag)
        : func_ { func }, tag_ { tag }
    {
        if (HiLogIsLoggable(LOG_DOMAIN, tag_, LOG_DEBUG)) {
            if (func_ != nullptr && tag_ != nullptr) {
                HILOG_IMPL(LOG_CORE, LOG_DEBUG, LOG_DOMAIN, tag_, "in %{public}s, enter", func_);
            }
        }
    }
    ~InnerFunctionTracer()
    {
        if (HiLogIsLoggable(LOG_DOMAIN, tag_, LOG_DEBUG)) {
            if (func_ != nullptr && tag_ != nullptr) {
                HILOG_IMPL(LOG_CORE, LOG_DEBUG, LOG_DOMAIN, tag_, "in %{public}s, leave", func_);
            }
        }
    }
private:
    const char *func_ { nullptr };
    const char *tag_ { nullptr };
};

#define CALL_LOG_ENTER   InnerFunctionTracer ___innerFuncTracer___ { __FUNCTION__, LOG_TAG }

#ifdef DEBUG_CODE_TEST
#define CHKPL(cond, ...) \
    do { \
        if ((cond) == nullptr) { \
            MISC_HILOGW("%{public}s, (%{public}d), CHKPL(%{public}s) is null, do nothing", \
                __FILE__, __LINE__, #cond); \
        } \
    } while (0)

#define CHKPV(cond) \
    do { \
        if ((cond) == nullptr) { \
            MISC_HILOGE("%{public}s, (%{public}d), CHKPV(%{public}s) is null", \
                __FILE__, __LINE__, #cond); \
            return; \
        } \
    } while (0)

#define CHKPF(cond) \
    do { \
        if ((cond) == nullptr) { \
            MISC_HILOGE("%{public}s, (%{public}d), CHKPF(%{public}s) is null", \
                __FILE__, __LINE__, #cond); \
            return false; \
        } \
    } while (0)

#define CHKPC(cond) \
    { \
        if ((cond) == nullptr) { \
            MISC_HILOGW("%{public}s, (%{public}d), CHKPC(%{public}s) is null, skip then continue", \
                __FILE__, __LINE__, #cond); \
            continue; \
        } \
    }

#define CHKPB(cond) \
    { \
        if ((cond) == nullptr) { \
            MISC_HILOGW("%{public}s, (%{public}d), CHKPC(%{public}s) is null, skip then break", \
                __FILE__, __LINE__, #cond); \
            break; \
        } \
    }

#define CHKPR(cond, r) \
    do { \
        if ((cond) == nullptr) { \
            MISC_HILOGE("%{public}s, (%{public}d), CHKPR(%{public}s) is null, return value is %{public}d", \
                __FILE__, __LINE__, #cond, r); \
            return r; \
        } \
    } while (0)

#define CHKPP(cond) \
    do { \
        if ((cond) == nullptr) { \
            MISC_HILOGE("%{public}s, (%{public}d), CHKPP(%{public}s) is null, return value is null", \
                __FILE__, __LINE__, #cond); \
            return nullptr; \
        } \
    } while (0)

#define CK(cond, ec) \
    do { \
        if (!(cond)) { \
            MISC_HILOGE("%{public}s, (%{public}d), CK(%{public}s), errCode:%{public}d", \
                __FILE__, __LINE__, #cond, ec); \
        } \
    } while (0)

#else // DEBUG_CODE_TEST
#define CHKPL(cond) \
    do { \
        if ((cond) == nullptr) { \
            MISC_HILOGW("CHKPL(%{public}s) is null, do nothing", #cond); \
        } \
    } while (0)

#define CHKPV(cond) \
    do { \
        if ((cond) == nullptr) { \
            MISC_HILOGE("CHKPV(%{public}s) is null", #cond); \
            return; \
        } \
    } while (0)

#define CHKPF(cond) \
    do { \
        if ((cond) == nullptr) { \
            MISC_HILOGE("CHKPF(%{public}s) is null", #cond); \
            return false; \
        } \
    } while (0)

#define CHKPC(cond) \
    { \
        if ((cond) == nullptr) { \
            MISC_HILOGW("CHKPC(%{public}s) is null, skip then continue", #cond); \
            continue; \
        } \
    }

#define CHKPB(cond) \
    { \
        if ((cond) == nullptr) { \
            MISC_HILOGW("CHKPC(%{public}s) is null, skip then break", #cond); \
            break; \
        } \
    }

#define CHKPR(cond, r) \
    do { \
        if ((cond) == nullptr) { \
            MISC_HILOGE("CHKPR(%{public}s) is null, return value is %{public}d", #cond, r); \
            return r; \
        } \
    } while (0)

#define CHKPS(cond) \
    do { \
        if ((cond) == nullptr) { \
            MISC_HILOGW("CHKPS(%{public}s) is null, skip then break", #cond); \
            return ""; \
        } \
    } while (0)

#define CHKPP(cond) \
    do { \
        if ((cond) == nullptr) { \
            MISC_HILOGE("CHKPP(%{public}s) is null, return value is null", #cond); \
            return nullptr; \
        } \
    } while (0)

#define CK(cond, ec) \
    do { \
        if (!(cond)) { \
            MISC_HILOGE("CK(%{public}s), errCode:%{public}d", #cond, ec); \
        } \
    } while (0)

#define CHKCR(cond, r, errDesc) \
    do { \
        if (!(cond)) { \
            MISC_HILOGE("%{public}s, errCode:%{public}d", #errDesc, r); \
            return r; \
        } \
    } while (0)

#define CHKCP(cond, errDesc) \
    do { \
        if (!(cond)) { \
            MISC_HILOGE("%{public}s", #errDesc); \
            return nullptr; \
        } \
    } while (0)

#define CHKCF(cond, errDesc) \
    do { \
        if (!(cond)) { \
            MISC_HILOGE("%{public}s", #errDesc); \
            return false; \
        } \
    } while (0)

#define CHKCV(cond, errDesc) \
    do { \
        if (!(cond)) { \
            MISC_HILOGE("%{public}s", #errDesc); \
            return; \
        } \
    } while (0)
#endif
} // namespace Sensors
} // namespace OHOS
#endif // SENSORS_ERRORS_H
