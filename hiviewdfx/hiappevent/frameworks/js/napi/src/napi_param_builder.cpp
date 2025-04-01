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
#include "napi_param_builder.h"

#include "hiappevent_base.h"
#include "hiappevent_verify.h"
#include "hilog/log.h"
#include "napi_error.h"
#include "napi_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "NapiParamBuilder"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr size_t MIN_NUM_PARAMETERS = 2; // the min number of params for setEventParam
constexpr int INDEX_OF_PARAMS = 0;
constexpr int INDEX_OF_DOMAIN = 1;
constexpr int INDEX_OF_NAME = 2;
constexpr size_t MAX_LENGTH_OF_PARAM_NAME = 32;
const std::string PARAM_VALUE_TYPE = "boolean|number|string|array[string]";
} // namespace
using namespace OHOS::HiviewDFX::ErrorCode;

bool NapiParamBuilder::IsValidParams(const napi_env env, const napi_value params[], size_t len)
{
    if (!NapiHiAppEventBuilder::IsValidEventParam(env, params[INDEX_OF_PARAMS]) ||
        !NapiHiAppEventBuilder::IsValidEventDomain(env, params[INDEX_OF_DOMAIN])) {
        return false;
    }
    return (len > MIN_NUM_PARAMETERS) ? NapiHiAppEventBuilder::IsValidEventName(env, params[INDEX_OF_NAME]) : true;
}

bool NapiParamBuilder::AddArrayParam2EventPack(napi_env env, const std::string& key, const napi_value arr)
{
    napi_valuetype type = NapiUtil::GetArrayType(env, arr);
    if (type != napi_string) {
        HILOG_ERROR(LOG_CORE, "array param value type is invalid");
        result_ = ERROR_INVALID_LIST_PARAM_TYPE;
        std::string errMsg = NapiUtil::CreateErrMsg("param value", PARAM_VALUE_TYPE);
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, errMsg, isV9_);
        return false;
    }
    std::vector<std::string> strs;
    NapiUtil::GetStrings(env, arr, strs);
    appEventPack_->AddParam(key, strs);
    return true;
}

bool NapiParamBuilder::AddParams2EventPack(napi_env env, const napi_value paramObj)
{
    std::vector<std::string> keys;
    NapiUtil::GetPropertyNames(env, paramObj, keys);
    for (auto key : keys) {
        if (key.length() > MAX_LENGTH_OF_PARAM_NAME) {
            result_ = ERROR_INVALID_PARAM_NAME;
            HILOG_ERROR(LOG_CORE, "the length=%{public}zu of the param key is invalid", key.length());
            break;
        }
        napi_value value = NapiUtil::GetProperty(env, paramObj, key);
        if (value == nullptr) {
            result_ = ERROR_INVALID_PARAM_VALUE_TYPE;
            std::string errMsg = NapiUtil::CreateErrMsg("param value", PARAM_VALUE_TYPE);
            NapiUtil::ThrowError(env, NapiError::ERR_PARAM, errMsg, isV9_);
            return false;
        }
        if (!AddParam2EventPack(env, key, value)) {
            return false;
        }
    }
    return true;
}

bool NapiParamBuilder::BuildCustomEventParamPack(napi_env env, const napi_value params[], size_t len)
{
    std::string domain = NapiUtil::GetString(env, params[INDEX_OF_DOMAIN]);
    std::string name;
    if (len > MIN_NUM_PARAMETERS) {
        name = NapiUtil::GetString(env, params[INDEX_OF_NAME]);
    }
    appEventPack_ = std::make_shared<AppEventPack>(domain, name);
    return AddParams2EventPack(env, params[INDEX_OF_PARAMS]);
}

std::shared_ptr<AppEventPack> NapiParamBuilder::BuildEventParam(
    const napi_env env, const napi_value params[], size_t len)
{
    if (len < MIN_NUM_PARAMETERS) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("setEventParam"), isV9_);
        return nullptr;
    }
    if (!IsValidParams(env, params, len)) {
        return nullptr;
    }
    if (!BuildCustomEventParamPack(env, params, len)) {
        return nullptr;
    }

    // if the build is successful, the event verification is performed
    if (appEventPack_ != nullptr && result_ >= 0) {
        if (auto ret = VerifyCustomEventParams(appEventPack_); ret != 0) {
            result_ = ret;
        }
    }
    return appEventPack_;
}
} // namespace HiviewDFX
} // namespace OHOS
