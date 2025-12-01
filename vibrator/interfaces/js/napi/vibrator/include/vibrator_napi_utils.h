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
#ifndef VIBRATOR_NAPI_UTILS_H
#define VIBRATOR_NAPI_UTILS_H

#include <cstring>
#include <iostream>
#include <map>
#include <optional>
#include <uv.h>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "refbase.h"

#include "sensors_errors.h"
#include "vibrator_agent.h"
#include "vibrator_agent_type.h"

namespace OHOS {
namespace Sensors {
using std::string;
constexpr int32_t CALLBACK_NUM = 3;
constexpr uint32_t STRING_LENGTH_MAX = 64;

#define CHKNRF(env, state, message) \
    do { \
        if ((state) != napi_ok) { \
            MISC_HILOGE("(%{public}s) fail", #message); \
            return false; \
        } \
    } while (0)

enum CallbackType {
    SYSTEM_VIBRATE_CALLBACK = 1,
    COMMON_CALLBACK,
    IS_SUPPORT_EFFECT_CALLBACK,
    VIBRATOR_STATE_CHANGE,
};

struct VibrateInfo {
    std::string type;
    std::string usage;
    bool systemUsage {false};
    int32_t duration = 0;
    std::string effectId;
    int32_t count = 0;
    int32_t fd = -1;
    int64_t offset = 0;
    int64_t length = -1;
    int32_t intensity = 0;
    VibratorPattern vibratorPattern;
};

class AsyncCallbackInfo : public RefBase {
public:
    struct AsyncCallbackError {
        int32_t code {0};
        string message;
        string name;
        string stack;
    };

    VibrateInfo info;
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    uv_work_t *work = nullptr;
    napi_ref callback[CALLBACK_NUM] = {0};
    AsyncCallbackError error;
    CallbackType callbackType = COMMON_CALLBACK;
    bool isSupportEffect {false};
    std::vector<VibratorInfos> vibratorInfos;
    int32_t arrayLength = -1;
    EffectInfo effectInfo;
    VibratorStatusEvent statusEvent;
    std::string flag;
    AsyncCallbackInfo(napi_env env) : env(env) {}
    ~AsyncCallbackInfo();
};

using ConstructResultFunc = bool(*)(const napi_env &env, sptr<AsyncCallbackInfo> asyncCallbackInfo,
    napi_value result[], int32_t length);

bool CreateInt32Property(napi_env env, napi_value &eventObj, const char* name, int32_t value);
bool CreateInt64Property(napi_env env, napi_value &eventObj, const char* name, int64_t value);
bool CreateStringProperty(napi_env env, napi_value &eventObj, const char* name,
    const char* value, int32_t valueLength);
bool CreateBooleanProperty(napi_env env, napi_value &eventObj, const char* name, bool value);
bool IsMatchArrayType(const napi_env &env, const napi_value &value);
bool IsMatchType(const napi_env &env, const napi_value &value, const napi_valuetype &type);
bool IsSameValue(const napi_env &env, const napi_value &lhs, const napi_value &rhs);
bool GetNapiInt32(const napi_env &env, const int32_t value, napi_value &result);
bool GetInt32Value(const napi_env &env, const napi_value &value, int32_t &result);
bool GetDoubleValue(const napi_env &env, const napi_value &value, double &result);
bool GetInt64Value(const napi_env &env, const napi_value &value, int64_t &result);
bool GetStringValue(const napi_env &env, const napi_value &value, string &result);
bool GetPropertyBool(const napi_env &env, const napi_value &value, const std::string &type, bool &result);
bool GetPropertyItem(const napi_env &env, const napi_value &value, const std::string &type, napi_value &item);
bool GetPropertyString(const napi_env &env, const napi_value &value, const std::string &type, std::string &result);
bool GetPropertyInt32(const napi_env &env, const napi_value &value, const std::string &type, int32_t &result);
bool GetPropertyInt64(const napi_env &env, const napi_value &value, const std::string &type, int64_t &result);
bool GetNapiParam(const napi_env &env, const napi_callback_info &info, size_t &argc, napi_value &argv);
bool ConvertErrorToResult(const napi_env &env, sptr<AsyncCallbackInfo> asyncCallbackInfo, napi_value &result);
napi_value ConvertToJsVibratorInfo(const napi_env &env, const VibratorInfos &vibratorInfo);
napi_value ConvertToJsEffectInfo(const napi_env &env, const EffectInfo &effectInfo);
bool ConstructCommonResult(const napi_env &env, sptr<AsyncCallbackInfo> asyncCallbackInfo, napi_value result[],
    int32_t length);
bool ConstructIsSupportEffectResult(const napi_env &env, sptr<AsyncCallbackInfo> asyncCallbackInfo,
    napi_value result[], int32_t length);
bool ConstructGetVibratorInfoListResult(const napi_env &env, sptr<AsyncCallbackInfo> asyncCallbackInfo,
    napi_value result[], int32_t length);
bool ConstructIsSupportEffectInfoResult(const napi_env &env, sptr<AsyncCallbackInfo> asyncCallbackInfo,
    napi_value result[], int32_t length);
bool ConstructVibratorPlugInfoResult(const napi_env &env, sptr<AsyncCallbackInfo> asyncCallbackInfo,
    napi_value result[], int32_t length);
void EmitAsyncCallbackWork(sptr<AsyncCallbackInfo> async_callback_info);
void EmitPromiseWork(sptr<AsyncCallbackInfo> asyncCallbackInfo);
void EmitUvEventLoop(sptr<AsyncCallbackInfo> asyncCallbackInfo);
bool ClearVibratorPattern(VibratorPattern &vibratorPattern);
void DeleteWork(uv_work_t *work);
} // namespace Sensors
} // namespace OHOS
#endif // VIBRATOR_NAPI_UTILS_H