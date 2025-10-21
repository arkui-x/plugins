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
#include "vibrator_agent.h"
#include "sensors_errors.h"
#include "vibrator_service_client.h"

namespace OHOS {
namespace Sensors {
using OHOS::Sensors::VibratorServiceClient;

namespace {
const std::string PHONE_TYPE = "phone";
const int32_t INTENSITY_ADJUST_MIN = 0;
const int32_t INTENSITY_ADJUST_MAX = 100;
const int32_t FREQUENCY_ADJUST_MIN = -100;
const int32_t FREQUENCY_ADJUST_MAX = 100;
const int32_t MAX_VIBRATOR_TIME = 1800000;
} // namespace

static int32_t NormalizeErrCode(int32_t code)
{
    switch (code) {
        case PERMISSION_DENIED: {
            return PERMISSION_DENIED;
        }
        case PARAMETER_ERROR: {
            return PARAMETER_ERROR;
        }
        case IS_NOT_SUPPORTED: {
            return IS_NOT_SUPPORTED;
        }
        default: {
            return DEVICE_OPERATION_FAILED;
        }
    }
}

bool SetLoopCount(int32_t count)
{
    VibratorIdentifier identifier = {
        .deviceId = -1,
        .vibratorId = -1
    };
    return SetLoopCountEnhanced(identifier, count);
}

bool SetLoopCountEnhanced(const VibratorIdentifier identifier, int32_t count)
{
    if (count <= 0) {
        MISC_HILOGE("Input invalid, count is %{public}d", count);
        return false;
    }
    auto &client = VibratorServiceClient::GetInstance();
    client.SetLoopCount(identifier, count);
    return true;
}

int32_t StartVibrator(const char *effectId)
{
    VibratorIdentifier identifier = {
        .deviceId = -1,
        .vibratorId = -1
    };
    return StartVibratorEnhanced(identifier, effectId);
}

int32_t StartVibratorEnhanced(const VibratorIdentifier identifier, const char *effectId)
{
    MISC_HILOGD("Time delay measurement:start time");
    CHKPR(effectId, PARAMETER_ERROR);
    auto &client = VibratorServiceClient::GetInstance();
    VibratorEffectParameter vibratorEffectParameter = client.GetVibratorEffectParameter(identifier);
    int32_t ret = client.Vibrate(identifier, effectId, vibratorEffectParameter.loopCount,
        vibratorEffectParameter.usage, vibratorEffectParameter.systemUsage);
    client.SetUsage(identifier, USAGE_UNKNOWN, false);
    client.SetLoopCount(identifier, 1);
    if (ret != ERR_OK) {
        MISC_HILOGE("Vibrate effectId failed, ret:%{public}d", ret);
        return NormalizeErrCode(ret);
    }
    return SUCCESS;
}

int32_t StartVibratorOnce(int32_t duration)
{
    MISC_HILOGI("StartVibratorOnce 2 Start.");
    VibratorIdentifier identifier = {
        .deviceId = -1,
        .vibratorId = -1
    };
    return StartVibratorOnceEnhanced(identifier, duration);
}

int32_t StartVibratorOnceEnhanced(const VibratorIdentifier identifier, int32_t duration)
{
    MISC_HILOGI("StartVibratorOnceEnhanced 2 Start.");
    if (duration <= 0 || duration > MAX_VIBRATOR_TIME) {
        MISC_HILOGE("duration is invalid");
        return PARAMETER_ERROR;
    }
    auto &client = VibratorServiceClient::GetInstance();
    VibratorEffectParameter vibratorEffectParameter = client.GetVibratorEffectParameter(identifier);
    int32_t ret = client.Vibrate(identifier, duration, vibratorEffectParameter.usage,
        vibratorEffectParameter.systemUsage);
    client.SetUsage(identifier, USAGE_UNKNOWN, false);
    if (ret != ERR_OK) {
        MISC_HILOGE("Vibrate duration failed, ret:%{public}d", ret);
        return NormalizeErrCode(ret);
    }
    return SUCCESS;
}

bool IsSupportVibratorCustom()
{
    MISC_HILOGI("IsSupportVibratorCustom 2 Start.");
    VibratorIdentifier identifier = {
        .deviceId = -1,
        .vibratorId = -1
    };
    return IsSupportVibratorCustomEnhanced(identifier);
}

bool IsSupportVibratorCustomEnhanced(const VibratorIdentifier identifier)
{
    MISC_HILOGI("IsSupportVibratorCustomEnhanced 2 Start.");
    auto &client = VibratorServiceClient::GetInstance();
    return client.IsSupportVibratorCustom(identifier);
}

int32_t PlayVibratorCustom(int32_t fd, int64_t offset, int64_t length)
{
    VibratorIdentifier identifier = {
        .deviceId = -1,
        .vibratorId = -1
    };
    return PlayVibratorCustomEnhanced(identifier, fd, offset, length);
}

int32_t PlayVibratorCustomEnhanced(const VibratorIdentifier identifier, int32_t fd, int64_t offset, int64_t length)
{
    MISC_HILOGI("PlayVibratorCustomEnhanced 2 Start.");
    if (fd < 0 || offset < 0 || length <= 0) {
        MISC_HILOGE("Input parameter invalid, fd:%{public}d, offset:%{public}lld, length:%{public}lld",
            fd, static_cast<long long>(offset), static_cast<long long>(length));
        return PARAMETER_ERROR;
    }
    auto &client = VibratorServiceClient::GetInstance();
    RawFileDescriptor rawFd = {
        .fd = fd,
        .offset = offset,
        .length = length
    };
    VibratorEffectParameter vibratorEffectParameter = client.GetVibratorEffectParameter(identifier);
    int32_t ret = client.PlayVibratorCustom(identifier, rawFd, vibratorEffectParameter.usage,
        vibratorEffectParameter.systemUsage, vibratorEffectParameter.vibratorParameter);
    vibratorEffectParameter.vibratorParameter.intensity = INTENSITY_ADJUST_MAX;
    vibratorEffectParameter.vibratorParameter.frequency = 0;
    client.SetUsage(identifier, USAGE_UNKNOWN, false);
    client.SetParameters(identifier, vibratorEffectParameter.vibratorParameter);
    if (ret != ERR_OK) {
        MISC_HILOGE("PlayVibratorCustom failed, ret:%{public}d", ret);
        return NormalizeErrCode(ret);
    }
    return SUCCESS;
}

int32_t StopVibrator(const char *mode)
{
    VibratorIdentifier identifier = {
        .deviceId = -1,
        .vibratorId = -1
    };
    return StopVibratorEnhanced(identifier, mode);
}

int32_t StopVibratorEnhanced(const VibratorIdentifier identifier, const char *mode)
{
    MISC_HILOGI("StopVibratorEnhanced 2 Start.");
    CHKPR(mode, PARAMETER_ERROR);
    if (strcmp(mode, "time") != 0 && strcmp(mode, "preset") != 0) {
        MISC_HILOGE("Input parameter invalid, mode is %{public}s", mode);
        return PARAMETER_ERROR;
    }
    auto &client = VibratorServiceClient::GetInstance();
    int32_t ret = client.StopVibrator(identifier, mode);
    if (ret != ERR_OK) {
        MISC_HILOGE("StopVibrator by mode failed, ret:%{public}d, mode:%{public}s", ret, mode);
        return NormalizeErrCode(ret);
    }
    return SUCCESS;
}

int32_t Cancel()
{
    VibratorIdentifier identifier = {
        .deviceId = -1,
        .vibratorId = -1
    };
    return CancelEnhanced(identifier);
}

int32_t CancelEnhanced(const VibratorIdentifier identifier)
{
    MISC_HILOGI("CancelEnhanced 2 Start.");
    auto &client = VibratorServiceClient::GetInstance();
    int32_t ret = client.StopVibrator(identifier);
    if (ret != ERR_OK) {
        MISC_HILOGE("StopVibrator failed, ret:%{public}d", ret);
        return NormalizeErrCode(ret);
    }
    return SUCCESS;
}

bool SetUsage(int32_t usage, bool systemUsage)
{
    VibratorIdentifier identifier = {
        .deviceId = -1,
        .vibratorId = -1
    };
    return SetUsageEnhanced(identifier, usage, systemUsage);
}

bool SetUsageEnhanced(const VibratorIdentifier identifier, int32_t usage, bool systemUsage)
{
    if ((usage < 0) || (usage >= USAGE_MAX)) {
        MISC_HILOGE("Input invalid, usage is %{public}d", usage);
        return false;
    }
    auto &client = VibratorServiceClient::GetInstance();
    client.SetUsage(identifier, usage, systemUsage);
    return true;
}

bool IsHdHapticSupported()
{
    MISC_HILOGI("IsHdHapticSupported 2 Start.");
    VibratorIdentifier identifier = {
        .deviceId = -1,
        .vibratorId = -1
    };
    return IsHdHapticSupportedEnhanced(identifier);
}

bool IsHdHapticSupportedEnhanced(const VibratorIdentifier identifier)
{
    auto &client = VibratorServiceClient::GetInstance();
    return client.IsHdHapticSupported(identifier);
}

int32_t IsSupportEffect(const char *effectId, bool *state)
{
    MISC_HILOGI("IsSupportEffect 2 Start.");
    VibratorIdentifier identifier = {
        .deviceId = -1,
        .vibratorId = -1
    };
    return IsSupportEffectEnhanced(identifier, effectId, state);
}

int32_t IsSupportEffectEnhanced(const VibratorIdentifier identifier, const char *effectId, bool *state)
{
    CHKPR(effectId, PARAMETER_ERROR);
    auto &client = VibratorServiceClient::GetInstance();
    int32_t ret = client.IsSupportEffect(identifier, effectId, *state);
    if (ret != ERR_OK) {
        MISC_HILOGE("Query effect support failed, ret:%{public}d, effectId:%{public}s", ret, effectId);
        return NormalizeErrCode(ret);
    }
    return SUCCESS;
}

int32_t PreProcess(const VibratorFileDescription &fd, VibratorPackage &package)
{
    return SUCCESS;
}

int32_t GetDelayTime(int32_t &delayTime)
{
    VibratorIdentifier identifier = {
        .deviceId = -1,
        .vibratorId = -1
    };
    return GetDelayTimeEnhanced(identifier, delayTime);
}

int32_t GetDelayTimeEnhanced(const VibratorIdentifier identifier, int32_t &delayTime)
{
    return SUCCESS;
}

int32_t PlayPattern(const VibratorPattern &pattern)
{
    VibratorIdentifier identifier = {
        .deviceId = -1,
        .vibratorId = -1
    };
    return PlayPatternEnhanced(identifier, pattern);
}

int32_t PlayPatternEnhanced(const VibratorIdentifier identifier, const VibratorPattern &pattern)
{
    auto &client = VibratorServiceClient::GetInstance();
    VibratorEffectParameter vibratorEffectParameter = client.GetVibratorEffectParameter(identifier);
    int32_t ret = client.PlayPattern(identifier, pattern, vibratorEffectParameter.usage,
        vibratorEffectParameter.systemUsage, vibratorEffectParameter.vibratorParameter);
    vibratorEffectParameter.vibratorParameter.intensity = INTENSITY_ADJUST_MAX;
    vibratorEffectParameter.vibratorParameter.frequency = 0;
    client.SetUsage(identifier, USAGE_UNKNOWN, false);
    client.SetParameters(identifier, vibratorEffectParameter.vibratorParameter);
    if (ret != ERR_OK) {
        MISC_HILOGE("PlayPattern failed, ret:%{public}d", ret);
        return NormalizeErrCode(ret);
    }
    return SUCCESS;
}

int32_t SeekTimeOnPackage(int32_t seekTime, const VibratorPackage &completePackage, VibratorPackage &seekPackage)
{
    return SUCCESS;
}

int32_t ModulatePackage(const VibratorCurvePoint* modulationCurve, const int32_t curvePointNum, const int32_t duration,
    const VibratorPackage &beforeModulationPackage, VibratorPackage &afterModulationPackage)
{
    return SUCCESS;
}

int32_t FreeVibratorPackage(VibratorPackage &package)
{
    return SUCCESS;
}

bool SetParameters(const VibratorParameter &parameter)
{
    VibratorIdentifier identifier = {
        .deviceId = -1,
        .vibratorId = -1
    };
    return SetParametersEnhanced(identifier, parameter);
}

bool SetParametersEnhanced(const VibratorIdentifier identifier, const VibratorParameter &parameter)
{
    if ((parameter.intensity < INTENSITY_ADJUST_MIN) || (parameter.intensity > INTENSITY_ADJUST_MAX) ||
        (parameter.frequency < FREQUENCY_ADJUST_MIN) || (parameter.frequency > FREQUENCY_ADJUST_MAX)) {
        MISC_HILOGE("Input invalid, intensity parameter is %{public}d, frequency parameter is %{public}d",
            parameter.intensity, parameter.frequency);
        return false;
    }
    auto &client = VibratorServiceClient::GetInstance();
    client.SetParameters(identifier, parameter);
    return true;
}

int32_t PlayPrimitiveEffect(const char *effectId, int32_t intensity)
{
    VibratorIdentifier identifier = {
        .deviceId = -1,
        .vibratorId = -1
    };
    return PlayPrimitiveEffectEnhanced(identifier, effectId, intensity);
}

int32_t PlayPrimitiveEffectEnhanced(const VibratorIdentifier identifier, const char *effectId, int32_t intensity)
{
    MISC_HILOGD("Time delay measurement:start time");
    CHKPR(effectId, PARAMETER_ERROR);
    auto &client = VibratorServiceClient::GetInstance();
    VibratorEffectParameter vibratorEffectParameter = client.GetVibratorEffectParameter(identifier);
    PrimitiveEffect primitiveEffect;
    primitiveEffect.intensity = intensity;
    primitiveEffect.usage = vibratorEffectParameter.usage;
    primitiveEffect.systemUsage = vibratorEffectParameter.systemUsage;
    primitiveEffect.count = vibratorEffectParameter.loopCount;
    int32_t ret = client.PlayPrimitiveEffect(identifier, effectId, primitiveEffect);
    client.SetUsage(identifier, USAGE_UNKNOWN, false);
    client.SetLoopCount(identifier, 1);
    if (ret != ERR_OK) {
        MISC_HILOGE("Play primitive effect failed, ret:%{public}d", ret);
        return NormalizeErrCode(ret);
    }
    return SUCCESS;
}

int32_t GetVibratorList(const VibratorIdentifier &identifier, std::vector<VibratorInfos> &vibratorInfo)
{
    MISC_HILOGI("GetVibratorList 2 Start.");
    CALL_LOG_ENTER;
    CHKCR(&identifier, PARAMETER_ERROR, "Invalid parameters");
    MISC_HILOGD("VibratorIdentifier = [deviceId = %{public}d, vibratorId = %{public}d]", identifier.deviceId,
        identifier.vibratorId);
    auto &client = VibratorServiceClient::GetInstance();
    int32_t ret = client.GetVibratorList(identifier, vibratorInfo);
    if (ret != ERR_OK) {
        MISC_HILOGE("Get vibrator list failed, ret:%{public}d", ret);
        return NormalizeErrCode(ret);
    }
    return SUCCESS;
}

int32_t GetEffectInfo(const VibratorIdentifier &identifier, const std::string &effectType, EffectInfo &effectInfo)
{
    MISC_HILOGI("GetEffectInfo 2 Start.");
    CALL_LOG_ENTER;
    CHKCR(&identifier, PARAMETER_ERROR, "Invalid parameters");
    MISC_HILOGD("VibratorIdentifier = [deviceId = %{public}d, vibratorId = %{public}d]", identifier.deviceId,
        identifier.vibratorId);
    auto &client = VibratorServiceClient::GetInstance();
    int32_t ret = client.GetEffectInfo(identifier, effectType, effectInfo);
    if (ret != ERR_OK) {
        effectInfo.isSupportEffect = false;
        MISC_HILOGW("Get effect info failed, ret:%{public}d", ret);
    }
    return SUCCESS;
}

int32_t SubscribeVibratorPlug(const VibratorUser &user)
{
    return SUCCESS;
}

int32_t UnSubscribeVibratorPlug(const VibratorUser &user)
{
    return SUCCESS;
}

int32_t PlayPatternBySessionId(uint32_t sessionId, const VibratorPattern &pattern)
{
    CALL_LOG_ENTER;
    if (sessionId == 0) {
        MISC_HILOGE("sessionId invalid, ret:%{public}d", sessionId);
        return PARAMETER_ERROR;
    }
    VibratorIdentifier identifier = {
        .deviceId = -1,
        .vibratorId = -1
    };
    auto &client = VibratorServiceClient::GetInstance();
    VibratorEffectParameter vibratorEffectParameter = client.GetVibratorEffectParameter(identifier);
    vibratorEffectParameter.vibratorParameter.sessionId = sessionId;
    int32_t ret = client.PlayPattern(identifier, pattern, vibratorEffectParameter.usage,
        vibratorEffectParameter.systemUsage, vibratorEffectParameter.vibratorParameter);
    vibratorEffectParameter.vibratorParameter.intensity = INTENSITY_ADJUST_MAX;
    vibratorEffectParameter.vibratorParameter.frequency = 0;
    client.SetUsage(identifier, USAGE_UNKNOWN, false);
    client.SetParameters(identifier, vibratorEffectParameter.vibratorParameter);
    if (ret != ERR_OK) {
        MISC_HILOGE("PlayPatternBySessionId failed, ret:%{public}d", ret);
        return NormalizeErrCode(ret);
    }
    return SUCCESS;
}

int32_t PlayPackageBySessionId(uint32_t sessionId, const VibratorPackage &package)
{
    return SUCCESS;
}

int32_t StopVibrateBySessionId(uint32_t sessionId)
{
    return SUCCESS;
}
} // namespace Sensors
} // namespace OHOS