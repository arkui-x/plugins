/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef WIFI_NAPI_ERRCODE_H_
#define WIFI_NAPI_ERRCODE_H_

#include <string>

#include "wifi_napi_errcode.h"
#include "wifi_napi_utils.h"

namespace OHOS::Plugin {
static const std::string BUSINESS_ERROR_PROPERTY_CODE = "code";
static const std::string BUSINESS_ERROR_PROPERTY_MESSAGE = "message";
static const std::string BUSINESS_ERROR_PROPERTY_DATA = "data";

enum WifiNapiErrCode {
    WIFI_ERRCODE_SUCCESS = 0,                    /* successfully */
    WIFI_ERRCODE_PERMISSION_DENIED = 201,        /* permission denied */
    WIFI_ERRCODE_NON_SYSTEMAPP = 202,            /* not system app */
    WIFI_ERRCODE_INVALID_PARAM = 401,            /* invalid params */
    WIFI_ERRCODE_NOT_SUPPORTED = 801,            /* not supported */
    WIFI_ERRCODE_OPERATION_FAILED = 1000,        /* failed */
    WIFI_ERRCODE_WIFI_NOT_OPENED = 1001,         /* sta service not opened */
    WIFI_ERRCODE_OPEN_FAIL_WHEN_CLOSING = 1003,  /* forbid when current airplane opened */
    WIFI_ERRCODE_CLOSE_FAIL_WHEN_OPENING = 1004, /* forbid when current powersaving opened */
};

#ifdef ENABLE_NAPI_WIFI_MANAGER
#ifndef WIFI_NAPI_ASSERT
#define WIFI_NAPI_ASSERT(env, cond, errCode, sysCap) \
    do {                                             \
        if (!(cond)) {                               \
            napi_value res = nullptr;                \
            HandleSyncErrCode(env, errCode, sysCap); \
            napi_get_undefined(env, &res);           \
            return res;                              \
        }                                            \
    } while (0)
#endif

#ifndef WIFI_NAPI_RETURN
#define WIFI_NAPI_RETURN(env, cond, errCode, sysCap) \
    do {                                             \
        napi_value res = nullptr;                    \
        if (!(cond)) {                               \
            HandleSyncErrCode(env, errCode, sysCap); \
        }                                            \
        napi_get_undefined(env, &res);               \
        return res;                                  \
    } while (0)
#endif

#else /* #else ENABLE_NAPI_WIFI_MANAGER */

#ifndef WIFI_NAPI_ASSERT
#define WIFI_NAPI_ASSERT(env, cond, errCode, sysCap) \
    do {                                             \
        if (!(cond)) {                               \
            napi_value res = nullptr;                \
            napi_get_boolean(env, cond, &res);       \
            return res;                              \
        }                                            \
    } while (0)
#endif

#ifndef WIFI_NAPI_RETURN
#define WIFI_NAPI_RETURN(env, cond, errCode, sysCap) \
    do {                                             \
        napi_value res = nullptr;                    \
        napi_get_boolean(env, cond, &res);           \
        return res;                                  \
    } while (0)
#endif
#endif /* #endif ENABLE_NAPI_WIFI_MANAGER */

/**
 * @brief Thow error code for async-callback function.
 *
 * @param env The env.
 * @param info The input data.
 */
void HandleCallbackErrCode(const napi_env& env, const AsyncContext& info);

/**
 * @brief Thow error code for async-promise function.
 *
 * @param env The env.
 * @param info The input data.
 */
void HandlePromiseErrCode(const napi_env& env, const AsyncContext& info);

#ifdef ENABLE_NAPI_WIFI_MANAGER
/**
 * @brief Thow error code for async function.
 *
 * @param env The env.
 * @param errCode The error code.
 * @param sysCap System capability code.
 */
void HandleSyncErrCode(const napi_env& env, int32_t errCode, int32_t sysCap);
#endif
} // namespace OHOS::Plugin

#endif
