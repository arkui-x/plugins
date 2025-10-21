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

#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <unistd.h>

#include "hilog/log.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "file_utils.h"
#include "miscdevice_log.h"
#include "vibrator_napi_error.h"
#include "vibrator_napi_utils.h"
#include "vibrator_pattern_js.h"
#ifdef ANDROID_PLATFORM
#include "vibrator_plugin_jni.h"
#endif // ANDROID_PLATFORM
#include "plugin_utils.h"

#undef LOG_TAG
#define LOG_TAG "VibratorJs"

namespace OHOS {
namespace Sensors {
namespace {
constexpr int32_t VIBRATE_SHORT_DURATION = 35;
constexpr int32_t VIBRATE_LONG_DURATION = 1000;
constexpr int32_t PARAMETER_TWO = 2;
constexpr int32_t PARAMETER_THREE = 3;
constexpr int32_t INTENSITY_ADJUST_MAX = 100;
constexpr int32_t EVENT_START_TIME_MAX = 1800000;
constexpr int32_t EVENT_NUM_MAX = 128;
constexpr int32_t INTENSITY_MIN = 0;
constexpr int32_t INTENSITY_MAX = 100;
constexpr int32_t FREQUENCY_MIN = 0;
constexpr int32_t FREQUENCY_MAX = 100;
constexpr int32_t CURVE_POINT_INTENSITY_MAX = 100;
constexpr int32_t CURVE_POINT_NUM_MIN = 4;
constexpr int32_t CURVE_POINT_NUM_MAX = 16;
constexpr int32_t CURVE_FREQUENCY_MIN = -100;
constexpr int32_t CURVE_FREQUENCY_MAX = 100;
constexpr int32_t CONTINUOUS_DURATION_MAX = 5000;
constexpr int32_t EVENT_INDEX_MAX = 2;
} // namespace

static std::map<std::string, int32_t> g_usageType = {
    {"unknown", USAGE_UNKNOWN},
    {"alarm", USAGE_ALARM},
    {"ring", USAGE_RING},
    {"notification", USAGE_NOTIFICATION},
    {"communication", USAGE_COMMUNICATION},
    {"touch", USAGE_TOUCH},
    {"media", USAGE_MEDIA},
    {"physicalFeedback", USAGE_PHYSICAL_FEEDBACK},
    {"simulateReality", USAGE_SIMULATE_REALITY},
};

static std::set<std::string> g_allowedTypes = {"time", "preset", "file", "pattern"};
static std::map<std::string, std::vector<sptr<AsyncCallbackInfo>>> g_onCallbackInfos;
static std::mutex g_Mutex;

static napi_value EmitAsyncWork(napi_value param, sptr<AsyncCallbackInfo> info)
{
    CHKPP(info);
    napi_status status = napi_generic_failure;
    if (param != nullptr) {
        status = napi_create_reference(info->env, param, 1, &info->callback[0]);
        if (status != napi_ok) {
            MISC_HILOGE("napi_create_reference fail");
            return nullptr;
        }
        EmitAsyncCallbackWork(info);
        return nullptr;
    }
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    status = napi_create_promise(info->env, &deferred, &promise);
    if (status != napi_ok) {
        MISC_HILOGE("napi_create_promise fail");
        return nullptr;
    }
    info->deferred = deferred;
    EmitPromiseWork(info);
    return promise;
}

static napi_value VibrateTime(napi_env env, napi_value args[], size_t argc)
{
    MISC_HILOGI("VibrateTime Start.");
    NAPI_ASSERT(env, (argc >= 1), "Wrong argument number");
    int32_t duration = 0;
    NAPI_ASSERT(env, GetInt32Value(env, args[0], duration), "Get int number fail");
    sptr<AsyncCallbackInfo> asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo(env);
    CHKPP(asyncCallbackInfo);
    asyncCallbackInfo->error.code = StartVibratorOnce(duration);
    if (argc >= PARAMETER_TWO && IsMatchType(env, args[1], napi_function)) {
        return EmitAsyncWork(args[1], asyncCallbackInfo);
    }
    return EmitAsyncWork(nullptr, asyncCallbackInfo);
}

static napi_value VibrateEffectId(napi_env env, napi_value args[], size_t argc)
{
    MISC_HILOGI("VibrateEffectId Start.");
    NAPI_ASSERT(env, (argc >= 1), "Wrong argument number");
    string effectId;
    NAPI_ASSERT(env, GetStringValue(env, args[0], effectId), "Wrong argument type. String or function expected");
    sptr<AsyncCallbackInfo> asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo(env);
    CHKPP(asyncCallbackInfo);
    asyncCallbackInfo->error.code = StartVibrator(effectId.c_str());
    if (argc >= PARAMETER_TWO && IsMatchType(env, args[1], napi_function)) {
        return EmitAsyncWork(args[1], asyncCallbackInfo);
    }
    return EmitAsyncWork(nullptr, asyncCallbackInfo);
}

static bool GetCallbackInfo(const napi_env &env, napi_value args[],
    sptr<AsyncCallbackInfo> &asyncCallbackInfo, string &mode)
{
    CHKPF(asyncCallbackInfo);
    CHKPF(args);
    napi_value value = nullptr;

    CHKCF(napi_get_named_property(env, args[0], "success", &value) == napi_ok, "napi get sucess property fail");
    CHKCF(napi_create_reference(env, value, 1, &asyncCallbackInfo->callback[0]) == napi_ok,
        "napi_create_reference fail");

    if (napi_get_named_property(env, args[0], "fail", &value) == napi_ok) {
        if (IsMatchType(env, value, napi_function)) {
            CHKCF(napi_create_reference(env, value, 1, &asyncCallbackInfo->callback[1]) == napi_ok,
                "napi_create_reference fail");
        }
    }
    if (napi_get_named_property(env, args[0], "complete", &value) == napi_ok) {
        if (IsMatchType(env, value, napi_function)) {
            CHKCF(napi_create_reference(env, value, 1, &asyncCallbackInfo->callback[PARAMETER_TWO]) == napi_ok,
                "napi_create_reference fail");
        }
    }
    if (napi_get_named_property(env, args[0], "mode", &value) == napi_ok) {
        bool result = GetStringValue(env, value, mode);
        if (!result || (mode != "long" && mode != "short")) {
            mode = "long";
        }
    }
    return true;
}

static napi_value VibrateMode(napi_env env, napi_value args[], size_t argc)
{
    MISC_HILOGI("VibrateMode Start.");
    if (argc == 0) {
        if (StartVibratorOnce(VIBRATE_LONG_DURATION) != 0) {
            MISC_HILOGE("Vibrate long mode fail");
        }
        return nullptr;
    }
    sptr<AsyncCallbackInfo> asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo(env);
    CHKPP(asyncCallbackInfo);
    asyncCallbackInfo->callbackType = SYSTEM_VIBRATE_CALLBACK;
    string mode = "long";
    if (!GetCallbackInfo(env, args, asyncCallbackInfo, mode)) {
        MISC_HILOGE("Get callback info fail");
        return nullptr;
    }
    int32_t duration = ((mode == "long") ? VIBRATE_LONG_DURATION : VIBRATE_SHORT_DURATION);
    asyncCallbackInfo->error.code = StartVibratorOnce(duration);
    if (asyncCallbackInfo->error.code != SUCCESS) {
        asyncCallbackInfo->error.message = "Vibrator vibrate fail";
    }
    EmitAsyncCallbackWork(asyncCallbackInfo);
    return nullptr;
}

static bool ParseVibratorCurvePoint(napi_env env, napi_value pointsArray, uint32_t index, VibratorCurvePoint &point)
{
    napi_value pointsElement = nullptr;
    CHKCF((napi_get_element(env, pointsArray, index, &pointsElement) == napi_ok), "napi_get_element pointsArray fail");
    CHKCF(IsMatchType(env, pointsElement, napi_object), "Wrong argument type, napi_object or function expected");
    CHKCF(GetPropertyInt32(env, pointsElement, "time", point.time), "Get points time fail");
    CHKCF(GetPropertyInt32(env, pointsElement, "intensity", point.intensity), "Get points intensity fail");
    CHKCF(GetPropertyInt32(env, pointsElement, "frequency", point.frequency), "Get points frequency fail");
    return true;
}

static bool ParseVibratorCurvePointArray(napi_env env, napi_value pointsArray, uint32_t pointsLength,
    VibratorEvent &event)
{
    if (pointsLength <= 0 || pointsLength > CURVE_POINT_NUM_MAX) {
        MISC_HILOGE("pointsLength should not be less than or equal to 0 or greater than CURVE_POINT_NUM_MAX");
        return false;
    }
    VibratorCurvePoint *points =
        static_cast<VibratorCurvePoint *>(malloc(sizeof(VibratorCurvePoint) * pointsLength));
    if (points == nullptr) {
        MISC_HILOGE("points is nullptr");
        return false;
    }
    for (uint32_t i = 0; i < pointsLength; ++i) {
        if (!ParseVibratorCurvePoint(env, pointsArray, i, points[i])) {
            MISC_HILOGE("ParseVibratorCurvePoint failed");
            free(points);
            points = nullptr;
            return false;
        }
    }
    event.points = points;
    return true;
}

static bool ParseVibrateEvent(napi_env env, napi_value eventArray, int32_t index, VibratorEvent &event)
{
    CALL_LOG_ENTER;
    napi_value element = nullptr;
    CHKCF((napi_get_element(env, eventArray, index, &element) == napi_ok), "napi_get_element eventArray fail");
    CHKCF(IsMatchType(env, element, napi_object), "Wrong argument type, napi_object or function expected");
    int32_t type = 0;
    CHKCF(GetPropertyInt32(env, element, "eventType", type), "Get type fail");
    event.type = static_cast<VibratorEventType>(type);
    CHKCF(GetPropertyInt32(env, element, "time", event.time), "Get time fail");
    CHKCF(GetPropertyInt32(env, element, "duration", event.duration), "Get duration fail");
    CHKCF(GetPropertyInt32(env, element, "intensity", event.intensity), "Get intensity fail");
    CHKCF(GetPropertyInt32(env, element, "frequency", event.frequency), "Get frequency fail");
    CHKCF(GetPropertyInt32(env, element, "index", event.index), "Get index fail");
    bool exist = false;
    napi_status status = napi_has_named_property(env, element, "points", &exist);
    if ((status == napi_ok) && exist) {
        napi_value pointsArray = nullptr;
        CHKCF(GetPropertyItem(env, element, "points", pointsArray), "Get points fail");
        CHKCF(IsMatchArrayType(env, pointsArray), "Wrong argument type, Napi array expected");
        uint32_t pointsLength = 0;
        CHKCF((napi_get_array_length(env, pointsArray, &pointsLength) == napi_ok),
            "napi_get_array_length pointsArray fail");
        event.pointNum = static_cast<int32_t>(pointsLength);
        if (pointsLength > 0) {
            if (!ParseVibratorCurvePointArray(env, pointsArray, pointsLength, event)) {
                MISC_HILOGE("ParseVibratorCurvePointArray failed");
                return false;
            }
        }
    }
    return true;
}

static bool ParseVibratorPattern(napi_env env, napi_value args[], VibrateInfo &info)
{
    CALL_LOG_ENTER;
    napi_value pattern = nullptr;
    CHKCF(GetPropertyItem(env, args[0], "pattern", pattern), "Get pattern fail");
    CHKCF(IsMatchType(env, pattern, napi_object), "Wrong argument type, Napi object expected");
    CHKCF(GetPropertyInt32(env, pattern, "time", info.vibratorPattern.time), "Get time fail");
    napi_value eventArray = nullptr;
    CHKCF(GetPropertyItem(env, pattern, "events", eventArray), "Get events fail");
    CHKCF(IsMatchArrayType(env, eventArray), "Wrong argument type, Napi array expected");
    uint32_t length = 0;
    CHKCF((napi_get_array_length(env, eventArray, &length) == napi_ok), "napi_get_array_length fail");
    info.vibratorPattern.eventNum = static_cast<int32_t>(length);
    if (length <= 0 || length > EVENT_NUM_MAX) {
        MISC_HILOGE("length should not be less than or equal to 0 or greater than EVENT_NUM_MAX");
        return false;
    }
    VibratorEvent *events = static_cast<VibratorEvent *>(malloc(sizeof(VibratorEvent) * length));
    if (events == nullptr) {
        MISC_HILOGE("Events is nullptr");
        return false;
    }
    for (uint32_t j = 0; j < length; ++j) {
        new (&events[j]) VibratorEvent();
        if (!ParseVibrateEvent(env, eventArray, j, events[j])) {
            MISC_HILOGE("ParseVibrateEvent failed");
            free(events);
            events = nullptr;
            return false;
        }
    }
    info.vibratorPattern.events = events;
    return true;
}

static void PrintVibratorPattern(VibratorPattern &vibratorPattern)
{
    CALL_LOG_ENTER;
    if (vibratorPattern.events == nullptr) {
        MISC_HILOGE("Events is nullptr");
        return;
    }
    MISC_HILOGI("PrintVibratorPattern, time:%{public}d, eventNum:%{public}d",
        vibratorPattern.time, vibratorPattern.eventNum);
    for (int32_t i = 0; i < vibratorPattern.eventNum; ++i) {
        MISC_HILOGI("PrintVibratorPattern, type:%{public}d, time:%{public}d, duration:%{public}d, \
            intensity:%{public}d, frequency:%{public}d, index:%{public}d, pointNum:%{public}d",
            static_cast<int32_t>(vibratorPattern.events[i].type), vibratorPattern.events[i].time,
            vibratorPattern.events[i].duration, vibratorPattern.events[i].intensity,
            vibratorPattern.events[i].frequency, vibratorPattern.events[i].index, vibratorPattern.events[i].pointNum);
        if (vibratorPattern.events[i].pointNum > 0) {
            VibratorCurvePoint *point = vibratorPattern.events[i].points;
            for (int32_t j = 0; j < vibratorPattern.events[i].pointNum; ++j) {
                MISC_HILOGI("PrintVibratorPattern, time:%{public}d, intensity:%{public}d, frequency:%{public}d",
                    point[j].time, point[j].intensity, point[j].frequency);
            }
        }
    }
}

static bool CheckVibratorCurvePoint(const VibratorEvent &event)
{
    if ((event.pointNum < CURVE_POINT_NUM_MIN) || (event.pointNum > CURVE_POINT_NUM_MAX)) {
        MISC_HILOGE("The points size is out of range, pointNum:%{public}d", event.pointNum);
        return false;
    }
    for (int32_t j = 0; j < event.pointNum; ++j) {
        if ((event.points[j].time < 0) || (event.points[j].time > event.duration)) {
            MISC_HILOGE("time in points is out of range, time:%{public}d", event.points[j].time);
            return false;
        }
        if ((event.points[j].intensity < 0) || (event.points[j].intensity > CURVE_POINT_INTENSITY_MAX)) {
            MISC_HILOGE("intensity in points is out of range, intensity:%{public}d", event.points[j].intensity);
            return false;
        }
        if ((event.points[j].frequency < CURVE_FREQUENCY_MIN) || (event.points[j].frequency > CURVE_FREQUENCY_MAX)) {
            MISC_HILOGE("frequency in points is out of range, frequency:%{public}d", event.points[j].frequency);
            return false;
        }
    }
    return true;
}

static bool CheckVibratorEvent(const VibratorEvent &event)
{
    if ((event.time < 0) || (event.time > EVENT_START_TIME_MAX)) {
        MISC_HILOGE("The event time is out of range, time:%{public}d", event.time);
        return false;
    }
    if ((event.frequency < FREQUENCY_MIN) || (event.frequency > FREQUENCY_MAX)) {
        MISC_HILOGE("The event frequency is out of range, frequency:%{public}d", event.frequency);
        return false;
    }
    if ((event.intensity < INTENSITY_MIN) || (event.intensity > INTENSITY_MAX)) {
        MISC_HILOGE("The event intensity is out of range, intensity:%{public}d", event.intensity);
        return false;
    }
    if ((event.duration <= 0) || (event.duration > CONTINUOUS_DURATION_MAX)) {
        MISC_HILOGE("The event duration is out of range, duration:%{public}d", event.duration);
        return false;
    }
    if ((event.index < 0) || (event.index > EVENT_INDEX_MAX)) {
        MISC_HILOGE("The event index is out of range, index:%{public}d", event.index);
        return false;
    }
    if ((event.type == VibratorEventType::EVENT_TYPE_CONTINUOUS) && (event.pointNum > 0)) {
        if (!CheckVibratorCurvePoint(event)) {
            MISC_HILOGE("CheckVibratorCurvePoint failed");
            return false;
        }
    }
    return true;
}

static bool CheckVibratorPatternParameter(VibratorPattern &vibratorPattern)
{
    CALL_LOG_ENTER;
    if (vibratorPattern.events == nullptr) {
        MISC_HILOGE("Events is nullptr");
        return false;
    }
    if ((vibratorPattern.eventNum <= 0) || (vibratorPattern.eventNum > EVENT_NUM_MAX)) {
        MISC_HILOGE("The event num  is out of range, eventNum:%{public}d", vibratorPattern.eventNum);
        return false;
    }
    for (int32_t i = 0; i < vibratorPattern.eventNum; ++i) {
        if (!CheckVibratorEvent(vibratorPattern.events[i])) {
            MISC_HILOGE("CheckVibratorEvent failed");
            return false;
        }
    }
    return true;
}

bool ParseVibratorIdentifier(napi_env env, napi_value value, VibratorIdentifier &identifier)
{
    CALL_LOG_ENTER;
    bool deviceIdResult = GetPropertyInt32(env, value, "deviceId", identifier.deviceId);
    bool vibratorIdResult = GetPropertyInt32(env, value, "vibratorId", identifier.vibratorId);
    if (deviceIdResult || vibratorIdResult) {
        MISC_HILOGI("Get vibrate identifier success");
        return true;
    }
    return false;
}

bool ParseParameter(napi_env env, napi_value args[], size_t argc, VibrateInfo &info, VibratorIdentifier &identifier)
{
    CHKCF((argc >= PARAMETER_TWO), "Wrong argument number");
    CHKCF(GetPropertyString(env, args[0], "type", info.type), "Get vibrate type fail");
    if (info.type == "time") {
        CHKCF(GetPropertyInt32(env, args[0], "duration", info.duration), "Get vibrate duration fail");
    } else if (info.type == "preset") {
        CHKCF(GetPropertyString(env, args[0], "effectId", info.effectId), "Get vibrate effectId fail");
        if (!GetPropertyInt32(env, args[0], "count", info.count)) {
            info.count = 1;
        }
        if (!GetPropertyInt32(env, args[0], "intensity", info.intensity)) {
            info.intensity = INTENSITY_ADJUST_MAX;
        }
    } else if (info.type == "file") {
        napi_value hapticFd = nullptr;
        CHKCF(GetPropertyItem(env, args[0], "hapticFd", hapticFd), "Get vibrate hapticFd fail");
        CHKCF(IsMatchType(env, hapticFd, napi_object), "Wrong argument type. Napi object expected");
        CHKCF(GetPropertyInt32(env, hapticFd, "fd", info.fd), "Get vibrate fd fail");
        GetPropertyInt64(env, hapticFd, "offset", info.offset);
        int64_t fdSize = GetFileSize(info.fd);
        CHKCF((info.offset >= 0) && (info.offset <= fdSize), "The parameter of offset is invalid");
        info.length = fdSize - info.offset;
        GetPropertyInt64(env, hapticFd, "length", info.length);
    } else if (info.type == "pattern") {
        CHKCF(ParseVibratorPattern(env, args, info), "ParseVibratorPattern fail");
        PrintVibratorPattern(info.vibratorPattern);
        CHKCF(CheckVibratorPatternParameter(info.vibratorPattern), "CheckVibratorPatternParameter fail");
    }
    CHKCF(GetPropertyString(env, args[1], "usage", info.usage), "Get vibrate usage fail");
    if (!GetPropertyBool(env, args[1], "systemUsage", info.systemUsage)) {
        info.systemUsage = false;
    }
    if (!GetPropertyInt32(env, args[1], "deviceId", identifier.deviceId)) {
        identifier.deviceId = -1;
    }
    if (!GetPropertyInt32(env, args[1], "id", identifier.vibratorId)) {
        identifier.vibratorId = -1;
    }
    if (identifier.vibratorId == 0) {
        identifier.vibratorId = -1;
    }
    return true;
}

bool SetUsage(const VibratorIdentifier &identifier, const std::string &usage, bool systemUsage)
{
    if (auto iter = g_usageType.find(usage); iter == g_usageType.end()) {
        MISC_HILOGE("Wrong usage type");
        return false;
    }
    return SetUsageEnhanced(identifier, g_usageType[usage], systemUsage);
    return true;
}

int32_t CheckParameters(const VibrateInfo &info, const VibratorIdentifier &identifier)
{
    CALL_LOG_ENTER;
    if (!SetUsage(identifier, info.usage, info.systemUsage)) {
        MISC_HILOGE("SetUsage fail");
        return PARAMETER_ERROR;
    }
    if (g_allowedTypes.find(info.type) == g_allowedTypes.end()) {
        MISC_HILOGE("Invalid vibrate type, type:%{public}s", info.type.c_str());
        return PARAMETER_ERROR;
    }
    return ERR_OK;
}

int32_t StartVibrate(const VibrateInfo &info, const VibratorIdentifier &identifier)
{
    MISC_HILOGI("StartVibrate enter.");
    CALL_LOG_ENTER;
    if (CheckParameters(info, identifier) != ERR_OK) {
        MISC_HILOGE("-CheckParameters fail");
        return PARAMETER_ERROR;
    }
    if (info.type == "file") {
        MISC_HILOGI("StartVibrate is file.");
        return PlayVibratorCustomEnhanced(identifier, info.fd, info.offset, info.length);
    } else if (info.type == "pattern") {
        MISC_HILOGI("StartVibrate is pattern.");
        return PlayPatternEnhanced(identifier, info.vibratorPattern);
    }
    MISC_HILOGI("StartVibrate leave.");
    return StartVibratorOnceEnhanced(identifier, info.duration);
}

static napi_value VibrateEffect(napi_env env, napi_value args[], size_t argc)
{
    MISC_HILOGI("VibrateEffect Start.");
    sptr<AsyncCallbackInfo> asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo(env);
    CHKPP(asyncCallbackInfo);
    VibratorIdentifier identifier;
    if (!ParseParameter(env, args, argc, asyncCallbackInfo->info, identifier)) {
        MISC_HILOGE("ParseParameter fail");
        ThrowErr(env, PARAMETER_ERROR, "ParseParameter fail");
        return nullptr;
    }
    if (asyncCallbackInfo->info.type == "preset") {
        if (!SetLoopCountEnhanced(identifier, asyncCallbackInfo->info.count) ||
            CheckParameters(asyncCallbackInfo->info, identifier) != ERR_OK ||
            asyncCallbackInfo->info.effectId.empty()) {
            MISC_HILOGE("SetLoopCount fail or parameter invalid");
            ThrowErr(env, PARAMETER_ERROR, "SetLoopCount fail or parameter invalid");
            return nullptr;
        }
        asyncCallbackInfo->flag = "preset";
        MISC_HILOGI("VibrateEffect is prese.");
    } else {
        asyncCallbackInfo->error.code = StartVibrate(asyncCallbackInfo->info, identifier);
        if (asyncCallbackInfo->info.vibratorPattern.events != nullptr) {
            CHKCP(ClearVibratorPattern(asyncCallbackInfo->info.vibratorPattern), "ClearVibratorPattern fail");
        }
        if (asyncCallbackInfo->error.code == PARAMETER_ERROR) {
            MISC_HILOGE("VibrateEffect is PARAMETER_ERROR.");
            ThrowErr(env, PARAMETER_ERROR, "Parameters invalid");
            return nullptr;
        }
        MISC_HILOGI("VibrateEffect not prese.");
    }
    if (argc >= PARAMETER_THREE && IsMatchType(env, args[PARAMETER_TWO], napi_function)) {
        MISC_HILOGE("VibrateEffect argc >= PARAMETER_THREE && IsMatchType(env, args[2], napi_function).");
        return EmitAsyncWork(args[PARAMETER_TWO], asyncCallbackInfo);
    }
    MISC_HILOGI("VibrateEffect leave.");
    return EmitAsyncWork(nullptr, asyncCallbackInfo);
}

static napi_value StartVibrate(napi_env env, napi_callback_info info)
{
    MISC_HILOGI("StartVibrate Start.");
    CALL_LOG_ENTER;
    CHKPP(env);
    CHKPP(info);
    size_t argc = 3;
    napi_value args[3] = {};
    napi_value thisArg = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &thisArg, nullptr);
    if (status != napi_ok || argc < PARAMETER_TWO) {
        MISC_HILOGE("StartVibrate napi_get_cb_info fail or number of parameter invalid.");
        ThrowErr(env, PARAMETER_ERROR, "napi_get_cb_info fail or number of parameter invalid");
        return nullptr;
    }
    if (!IsMatchType(env, args[0], napi_object) || !IsMatchType(env, args[1], napi_object)) {
        MISC_HILOGE("StartVibrate args[0] and args[1] should is napi_object.");
        ThrowErr(env, PARAMETER_ERROR, "args[0] and args[1] should is napi_object");
        return nullptr;
    }
    MISC_HILOGI("StartVibrate leave.");
    return VibrateEffect(env, args, argc);
}

static napi_value Vibrate(napi_env env, napi_callback_info info)
{
    MISC_HILOGI("Vibrate Start.");
    CHKPP(env);
    CHKPP(info);
    size_t argc = 2;
    napi_value args[2] = {};
    napi_value thisArg = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &thisArg, nullptr);
    if (status != napi_ok) {
        ThrowErr(env, PARAMETER_ERROR, "napi_get_cb_info fail");
        return nullptr;
    }
    if (argc >= 1 && IsMatchType(env, args[0], napi_number)) {
        return VibrateTime(env, args, argc);
    }
    if (argc >= 1 && IsMatchType(env, args[0], napi_string)) {
        return VibrateEffectId(env, args, argc);
    }
    return VibrateMode(env, args, argc);
}

static napi_value Cancel(napi_env env, napi_callback_info info)
{
    MISC_HILOGI("Cancel Start.");
    size_t argc = 2;
    napi_value args[2] = {};
    napi_value thisArg = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &thisArg, nullptr);
    if (status != napi_ok) {
        ThrowErr(env, PARAMETER_ERROR, "napi_get_cb_info fail");
        return nullptr;
    }

    VibratorIdentifier identifier = {
        .deviceId = -1,
        .vibratorId = -1
    };
    sptr<AsyncCallbackInfo> asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo(env);
    CHKPP(asyncCallbackInfo);
    if ((argc > 0) && (IsMatchType(env, args[0], napi_undefined))) {
        asyncCallbackInfo->error.code = PARAMETER_ERROR;
        return EmitAsyncWork(args[0], asyncCallbackInfo);
    }
    if ((argc > 0) && (IsMatchType(env, args[0], napi_function))) {
        asyncCallbackInfo->error.code = CancelEnhanced(identifier);
        return EmitAsyncWork(args[0], asyncCallbackInfo);
    } else if ((argc > 0) && IsMatchType(env, args[0], napi_object) &&
        ParseVibratorIdentifier(env, args[0], identifier)) {
        asyncCallbackInfo->error.code = CancelEnhanced(identifier);
        return EmitAsyncWork(nullptr, asyncCallbackInfo);
    } else {
        asyncCallbackInfo->error.code = CancelEnhanced(identifier);
        return EmitAsyncWork(nullptr, asyncCallbackInfo);
    }
}

static napi_value Stop(napi_env env, napi_callback_info info)
{
    MISC_HILOGI("Stop Start.");
    size_t argc = 3;
    napi_value args[3] = {};
    napi_value thisArg = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &thisArg, nullptr);
    if (status != napi_ok) {
        MISC_HILOGI("Stop napi_get_cb_info fail.");
        ThrowErr(env, PARAMETER_ERROR, "napi_get_cb_info fail");
        return nullptr;
    }
    VibratorIdentifier identifier = {
        .deviceId = -1,
        .vibratorId = -1
    };
    if (argc >= 1 && IsMatchType(env, args[0], napi_string)) {
        string mode;
        if (!GetStringValue(env, args[0], mode)) {
            MISC_HILOGE("Stop Parameters invalid mode:%s.", mode.c_str());
            ThrowErr(env, PARAMETER_ERROR, "Parameters invalid");
            return nullptr;
        }
        sptr<AsyncCallbackInfo> asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo(env);
        CHKPP(asyncCallbackInfo);
        asyncCallbackInfo->error.code = StopVibratorEnhanced(identifier, mode.c_str());
        if ((asyncCallbackInfo->error.code != SUCCESS) && (asyncCallbackInfo->error.code == PARAMETER_ERROR)) {
            MISC_HILOGE("Stop Parameters asyncCallbackInfo->error.code:%d.", asyncCallbackInfo->error.code);
            ThrowErr(env, PARAMETER_ERROR, "Parameters invalid");
            return nullptr;
        }
        if (argc >= PARAMETER_TWO && IsMatchType(env, args[1], napi_function)) {
            return EmitAsyncWork(args[1], asyncCallbackInfo);
        }
        return EmitAsyncWork(nullptr, asyncCallbackInfo);
    } else {
        return Cancel(env, info);
    }
}

static napi_value StopVibrationSync(napi_env env, napi_callback_info info)
{
    MISC_HILOGI("StopVibrationSync Start.");
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    napi_value thisArg = nullptr;
    size_t argc = 0;
    napi_status status = napi_get_cb_info(env, info, &argc, nullptr, &thisArg, nullptr);
    if (status != napi_ok) {
        ThrowErr(env, PARAMETER_ERROR, "Get the parameter info fail");
        return result;
    }
    int32_t ret = Cancel();
    if (ret != SUCCESS) {
        ThrowErr(env, ret, "Cancel execution fail");
    }
    return result;
}

static napi_value IsHdHapticSupported(napi_env env, napi_callback_info info)
{
    MISC_HILOGI("IsHdHapticSupported Start.");
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    napi_value thisArg = nullptr;
    size_t argc = 0;
    napi_status status = napi_get_cb_info(env, info, &argc, nullptr, &thisArg, nullptr);
    if (status != napi_ok) {
        ThrowErr(env, PARAMETER_ERROR, "Get the parameter info fail");
        return result;
    }
    status= napi_get_boolean(env, IsHdHapticSupported(), &result);
    if (status != napi_ok) {
        ThrowErr(env, PARAMETER_ERROR, "Get the value of boolean fail");
    }
    return result;
}

static napi_value IsSupportEffect(napi_env env, napi_callback_info info)
{
    MISC_HILOGI("IsSupportEffect Start.");
    size_t argc = 2;
    napi_value args[2] = {};
    napi_value thisArg = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &thisArg, nullptr);
    if ((status != napi_ok) || (argc == 0)) {
        ThrowErr(env, PARAMETER_ERROR, "napi_get_cb_info fail or number of parameter invalid");
        return nullptr;
    }
    string effectId;
    if (!GetStringValue(env, args[0], effectId)) {
        ThrowErr(env, PARAMETER_ERROR, "GetStringValue fail");
        return nullptr;
    }
    sptr<AsyncCallbackInfo> asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo(env);
    CHKPP(asyncCallbackInfo);
    asyncCallbackInfo->callbackType = IS_SUPPORT_EFFECT_CALLBACK;
    bool isSupportEffect = false;
    int32_t ret = IsSupportEffect(effectId.c_str(), &isSupportEffect);
    if (ret == PERMISSION_DENIED || ret == PARAMETER_ERROR) {
        asyncCallbackInfo->error.code = ret;
    }
    asyncCallbackInfo->isSupportEffect = isSupportEffect;
    if ((argc > 1) && (IsMatchType(env, args[1], napi_function))) {
        return EmitAsyncWork(args[1], asyncCallbackInfo);
    }
    return EmitAsyncWork(nullptr, asyncCallbackInfo);
}

static napi_value IsSupportEffectSync(napi_env env, napi_callback_info info)
{
    MISC_HILOGI("IsSupportEffectSync Start.");
    size_t argc = 1;
    napi_value args[1] = {};
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    napi_value thisArg = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &thisArg, nullptr);
    if ((status != napi_ok) || (argc == 0)) {
        ThrowErr(env, PARAMETER_ERROR, "Get the parameter info fail or number of parameter invalid");
        return result;
    }
    string effectId;
    if (!GetStringValue(env, args[0], effectId)) {
        ThrowErr(env, PARAMETER_ERROR, "Get the value of string fail");
        return result;
    }
    bool isSupportEffect = false;
    int32_t ret = IsSupportEffect(effectId.c_str(), &isSupportEffect);
    if (ret != SUCCESS) {
        ThrowErr(env, ret, "IsSupportEffect execution failed");
        return result;
    }
    status= napi_get_boolean(env, isSupportEffect, &result);
    if (status != napi_ok) {
        ThrowErr(env, PARAMETER_ERROR, "Get the value of boolean fail");
    }
    return result;
}

static napi_value EnumClassConstructor(const napi_env env, const napi_callback_info info)
{
    size_t argc = 0;
    napi_value args[1] = {0};
    napi_value res = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &res, &data));
    return res;
}

static napi_value CreateEnumStopMode(const napi_env env, napi_value exports)
{
    napi_value timeMode = nullptr;
    napi_value presetMode = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "time", NAPI_AUTO_LENGTH, &timeMode));
    NAPI_CALL(env, napi_create_string_utf8(env, "preset", NAPI_AUTO_LENGTH, &presetMode));

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("VIBRATOR_STOP_MODE_TIME", timeMode),
        DECLARE_NAPI_STATIC_PROPERTY("VIBRATOR_STOP_MODE_PRESET", presetMode),
    };
    napi_value result = nullptr;
    NAPI_CALL(env, napi_define_class(env, "VibratorStopMode", NAPI_AUTO_LENGTH, EnumClassConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result));
    NAPI_CALL(env, napi_set_named_property(env, exports, "VibratorStopMode", result));
    return exports;
}

static napi_value CreateEnumVibratorEventType(const napi_env env, napi_value exports)
{
    napi_value continuous = nullptr;
    NAPI_CALL(env, napi_create_int32(env, EVENT_TYPE_CONTINUOUS, &continuous));
    napi_value transient = nullptr;
    NAPI_CALL(env, napi_create_int32(env, EVENT_TYPE_TRANSIENT, &transient));

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("CONTINUOUS", continuous),
        DECLARE_NAPI_STATIC_PROPERTY("TRANSIENT", transient),
    };
    napi_value result = nullptr;
    NAPI_CALL(env, napi_define_class(env, "VibratorEventType", NAPI_AUTO_LENGTH, EnumClassConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result));
    NAPI_CALL(env, napi_set_named_property(env, exports, "VibratorEventType", result));
    return exports;
}

static napi_value CreateClassVibratePattern(const napi_env env, const napi_value exports)
{
    napi_property_descriptor clzDes[] = {
    DECLARE_NAPI_FUNCTION("addContinuousEvent", VibratorPatternBuilder::AddContinuousEvent),
    DECLARE_NAPI_FUNCTION("addTransientEvent", VibratorPatternBuilder::AddTransientEvent),
    DECLARE_NAPI_FUNCTION("build", VibratorPatternBuilder::Build),
    };
    napi_value cons = nullptr;
    NAPI_CALL(env, napi_define_class(env, "VibratorPatternBuilder", NAPI_AUTO_LENGTH,
        VibratorPatternBuilder::VibratorPatternConstructor, nullptr, sizeof(clzDes) / sizeof(napi_property_descriptor),
        clzDes, &cons));
    NAPI_CALL(env, napi_set_named_property(env, exports, "VibratorPatternBuilder", cons));
    return exports;
}

static napi_value CreateEnumEffectId(const napi_env env, const napi_value exports)
{
    napi_value clockTime = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "haptic.clock.timer", NAPI_AUTO_LENGTH, &clockTime));
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("EFFECT_CLOCK_TIMER", clockTime),
    };
    napi_value result = nullptr;
    NAPI_CALL(env, napi_define_class(env, "EffectId", NAPI_AUTO_LENGTH, EnumClassConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result));
    NAPI_CALL(env, napi_set_named_property(env, exports, "EffectId", result));
    return exports;
}

static napi_value GetVibratorListSync(napi_env env, napi_callback_info info)
{
    MISC_HILOGI("GetVibratorListSync Start.");
    CALL_LOG_ENTER;
    size_t argc = 1;
    napi_value args[1] = {0};
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        MISC_HILOGE("napi_get_cb_info get params failed");
        return nullptr;
    }
    VibratorIdentifier identifier;
    if (argc > 0) {
        if (!ParseVibratorIdentifier(env, args[0], identifier)) {
            MISC_HILOGW("deviceId and vibratorId is undefined, set default value deviceId = -1 and vibratorId = -1");
        }
        MISC_HILOGI("identifier=[deviceId=%{public}d, vibratorId=%{pubilc}d]",
            identifier.deviceId, identifier.vibratorId);
    }
    napi_value jsArray = nullptr;
    status = napi_create_array(env, &jsArray);
    if (status != napi_ok) {
        MISC_HILOGE("napi_create_array failed");
        return jsArray;
    }
    std::vector<VibratorInfos> vibratorInfo;
    int32_t ret = GetVibratorList(identifier, vibratorInfo);
    if (ret == SUCCESS) {
        int32_t length = 0;
        for (auto &info : vibratorInfo) {
            napi_value jsInfo = ConvertToJsVibratorInfo(env, info);
            status = napi_set_element(env, jsArray, length++, jsInfo);
            if (status != napi_ok) {
                MISC_HILOGE("napi construct JS result array failed");
                jsArray = nullptr;
                break;
            }
        }
    }
    return jsArray;
}

static napi_value GetSupportEffectInfoSync(napi_env env, napi_callback_info info)
{
    MISC_HILOGI("GetSupportEffectInfoSync Start.");
    CALL_LOG_ENTER;
    size_t argc = 2;
    napi_value args[2] = {0};
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        MISC_HILOGE("napi_get_cb_info get params failed");
        return nullptr;
    }
    if ((argc == 0) || !IsMatchType(env, args[0], napi_string)) {
        MISC_HILOGW("Parameter error, expect string");
        return nullptr;
    }
    std::string effectType;
    if (!GetStringValue(env, args[0], effectType)) {
        MISC_HILOGE("GetStringValue fail");
        return nullptr;
    }
    MISC_HILOGI("effectType = %{public}s", effectType.c_str());

    VibratorIdentifier identifier;
    if (argc > 1) {
        if (!ParseVibratorIdentifier(env, args[1], identifier)) {
            MISC_HILOGW("deviceId and vibratorId is undefined, set default value deviceId = -1 and vibratorId = -1");
        }
        MISC_HILOGI("identifier=[deviceId=%{public}d, vibratorId=%{pubilc}d]",
                    identifier.deviceId, identifier.vibratorId);
    }

    EffectInfo effectInfo;
    int32_t ret = GetEffectInfo(identifier, effectType, effectInfo);
    MISC_HILOGI("effectInfo=[%{public}s]", (effectInfo.isSupportEffect? "true" : "false"));

    napi_value jsEffectInfo = nullptr;
    if (ret == SUCCESS) {
        jsEffectInfo = ConvertToJsEffectInfo(env, effectInfo);
    }
    return jsEffectInfo;
}

static bool IsSubscribed(const napi_env &env, const std::string &vibratorEvent, napi_value callback)
{
    CALL_LOG_ENTER;
    MISC_HILOGI("g_onCallbackInfos.size() = %{public}zu", g_onCallbackInfos.size());
    if (g_onCallbackInfos.empty()) {
        return false;
    }
    if (auto iter = g_onCallbackInfos.find(vibratorEvent); iter == g_onCallbackInfos.end()) {
        MISC_HILOGW("No client subscribe, vibratorEvent:%{public}s", vibratorEvent.c_str());
        return false;
    }
    std::vector<sptr<AsyncCallbackInfo>> callbackInfos = g_onCallbackInfos[vibratorEvent];
    for (auto callbackInfo : callbackInfos) {
        CHKPC(callbackInfo);
        if (callbackInfo->env != env) {
            continue;
        }
        napi_value vibratorCallback = nullptr;
        CHKNRF(env, napi_get_reference_value(env, callbackInfo->callback[0], &vibratorCallback),
            "napi_get_reference_value");
        if (IsSameValue(env, callback, vibratorCallback)) {
            return true;
        }
    }
    return false;
}

static void UpdateCallbackInfos(const napi_env &env, const std::string &vibratorEvent, napi_value callback)
{
    CALL_LOG_ENTER;
    std::lock_guard<std::mutex> onSubcribeLock(g_Mutex);
    if (IsSubscribed(env, vibratorEvent, callback)) {
        MISC_HILOGW("The callback has been subscribed");
        return;
    }
    sptr<AsyncCallbackInfo> asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo(env);
    CHKPV(asyncCallbackInfo);

    if (vibratorEvent == "vibratorStateChange") {
        asyncCallbackInfo->callbackType = VIBRATOR_STATE_CHANGE;
    }
    napi_status status = napi_create_reference(env, callback, 1, &asyncCallbackInfo->callback[0]);
    if (status != napi_ok) {
        ThrowErr(env, PARAMETER_ERROR, "napi_create_reference fail");
        return;
    }
    g_onCallbackInfos[vibratorEvent].push_back(asyncCallbackInfo);
}

static napi_value On(napi_env env, napi_callback_info info)
{
    return nullptr;
}

static int32_t RemoveAllCallback(const napi_env &env, const std::string &eventType)
{
    CALL_LOG_ENTER;
    std::lock_guard<std::mutex> onCancelLock(g_Mutex);
    std::vector<sptr<AsyncCallbackInfo>> callbackInfos = g_onCallbackInfos[eventType];
    for (auto iter = callbackInfos.begin(); iter != callbackInfos.end();) {
        CHKPC(*iter);
        if ((*iter)->env != env) {
            ++iter;
            continue;
        }
        iter = callbackInfos.erase(iter);
    }
    if (callbackInfos.empty()) {
        MISC_HILOGI("No subscription to change");
        g_onCallbackInfos.erase(eventType);
        return callbackInfos.size();
    }
    g_onCallbackInfos[eventType] = callbackInfos;
    return callbackInfos.size();
}

static int32_t RemoveCallback(napi_env env, const std::string &eventType, napi_value callback)
{
    CALL_LOG_ENTER;
    std::lock_guard<std::mutex> onCallbackLock(g_Mutex);
    std::vector<sptr<AsyncCallbackInfo>> callbackInfos = g_onCallbackInfos[eventType];
    for (auto iter = callbackInfos.begin(); iter != callbackInfos.end();) {
        CHKPC(*iter);
        if ((*iter)->env != env) {
            continue;
        }
        napi_value VibratorCallback = nullptr;
        if (napi_get_reference_value(env, (*iter)->callback[0], &VibratorCallback) != napi_ok) {
            MISC_HILOGE("napi_get_reference_value fail");
            continue;
        }
        if (IsSameValue(env, callback, VibratorCallback)) {
            iter = callbackInfos.erase(iter);
            MISC_HILOGI("Remove callback success");
            break;
        } else {
            ++iter;
        }
    }
    if (callbackInfos.empty()) {
        MISC_HILOGI("No subscription to change data");
        g_onCallbackInfos.erase(eventType);
        return 0;
    }
    g_onCallbackInfos[eventType] = callbackInfos;
    return callbackInfos.size();
}

static napi_value Off(napi_env env, napi_callback_info info)
{
    return nullptr;
}

static napi_value CreateEnumHapticFeedback(const napi_env env, napi_value exports)
{
    napi_value effectSoft = nullptr;
    napi_value effectHard = nullptr;
    napi_value effectSharp = nullptr;
    napi_value effectNoticeSuccess = nullptr;
    napi_value effectNoticeFail = nullptr;
    napi_value effectNoticeWarning = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "haptic.effect.soft", NAPI_AUTO_LENGTH, &effectSoft));
    NAPI_CALL(env, napi_create_string_utf8(env, "haptic.effect.hard", NAPI_AUTO_LENGTH, &effectHard));
    NAPI_CALL(env, napi_create_string_utf8(env, "haptic.effect.sharp", NAPI_AUTO_LENGTH, &effectSharp));
    NAPI_CALL(env, napi_create_string_utf8(env, "haptic.notice.success", NAPI_AUTO_LENGTH, &effectNoticeSuccess));
    NAPI_CALL(env, napi_create_string_utf8(env, "haptic.notice.fail", NAPI_AUTO_LENGTH, &effectNoticeFail));
    NAPI_CALL(env, napi_create_string_utf8(env, "haptic.notice.warning", NAPI_AUTO_LENGTH, &effectNoticeWarning));

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("EFFECT_SOFT", effectSoft),
        DECLARE_NAPI_STATIC_PROPERTY("EFFECT_HARD", effectHard),
        DECLARE_NAPI_STATIC_PROPERTY("EFFECT_SHARP", effectSharp),
        DECLARE_NAPI_STATIC_PROPERTY("EFFECT_NOTICE_SUCCESS", effectNoticeSuccess),
        DECLARE_NAPI_STATIC_PROPERTY("EFFECT_NOTICE_FAILURE", effectNoticeFail),
        DECLARE_NAPI_STATIC_PROPERTY("EFFECT_NOTICE_WARNING", effectNoticeWarning),
    };
    napi_value result = nullptr;
    NAPI_CALL(env, napi_define_class(env, "HapticFeedback", NAPI_AUTO_LENGTH, EnumClassConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result));
    NAPI_CALL(env, napi_set_named_property(env, exports, "HapticFeedback", result));
    return exports;
}

static napi_value Init(napi_env env, napi_value exports)
{
    MISC_HILOGI("napi_value Init");
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("vibrate", Vibrate),
        DECLARE_NAPI_FUNCTION("stop", Stop),
        DECLARE_NAPI_FUNCTION("startVibration", StartVibrate),
        DECLARE_NAPI_FUNCTION("stopVibration", Stop),
        DECLARE_NAPI_FUNCTION("stopVibrationSync", StopVibrationSync),
        DECLARE_NAPI_FUNCTION("isHdHapticSupported", IsHdHapticSupported),
        DECLARE_NAPI_FUNCTION("isSupportEffect", IsSupportEffect),
        DECLARE_NAPI_FUNCTION("isSupportEffectSync", IsSupportEffectSync),
        DECLARE_NAPI_FUNCTION("getVibratorInfoSync", GetVibratorListSync),
        DECLARE_NAPI_FUNCTION("getEffectInfoSync", GetSupportEffectInfoSync),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc));
    NAPI_ASSERT_BASE(env, CreateEnumStopMode(env, exports) != nullptr, "Create enum stop mode fail", exports);
    NAPI_ASSERT_BASE(env, CreateEnumEffectId(env, exports) != nullptr, "Create enum effect id fail", exports);
    NAPI_ASSERT_BASE(env, CreateEnumHapticFeedback(env, exports) != nullptr, "Create enum haptic feedback fail",
                     exports);
    NAPI_ASSERT_BASE(env, CreateClassVibratePattern(env, exports) != nullptr, "CreateClassVibratePattern fail",
                     exports);
    NAPI_ASSERT_BASE(env, CreateEnumVibratorEventType(env, exports) != nullptr, "Create enum vibrator Event type fail",
                     exports);
    return exports;
}

#ifdef ANDROID_PLATFORM
static void PasteboardPluginJniRegister()
{
    const char className[] = "ohos.ace.plugin.vibratorplugin.VibratorPlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&Sensors::VibratorJni::Register, className);
}
#endif

static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "vibrator",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
#ifdef ANDROID_PLATFORM
    PasteboardPluginJniRegister();
#endif

    napi_module_register(&_module);
}
} // namespace Sensors
} // namespace OHOS
