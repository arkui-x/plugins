/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "method_result.h"

#include "bridge_binary_codec.h"
#include "bridge_json_codec.h"
#include "error_code.h"
#include "log.h"
#include "method_data_converter.h"
#include "napi_utils.h"
#include "plugins/interfaces/native/inner_api/plugin_utils_napi.h"
#include "plugins/interfaces/native/plugin_utils.h"

namespace OHOS::Plugin::Bridge {
void MethodResult::SetMethodName(const std::string& methodName)
{
    methodName_ = methodName;
}

const std::string& MethodResult::GetMethodName(void) const
{
    return methodName_;
}

void MethodResult::SetErrorCode(int errorCode)
{
    errorCode_ = errorCode;
}

void MethodResult::GetErrorInfoByErrorCode(void)
{
    if (errorCode_ < 0) {
        errorCode_ = static_cast<int>(ErrorCode::BRIDGE_DATA_ERROR);
    }
    if (errorCode_ >= 0 && errorCode_ < static_cast<int>(ErrorCode::BRIDGE_END)) {
        errcodeMessage_ = CodeMessage[errorCode_];
    }
    if (errcodeMessage_.empty()) {
        errcodeMessage_ = "unknown error!";
    }
}

void MethodResult::SetErrorCodeInfo(int errorCode)
{
    errorCode_ = errorCode;
    GetErrorInfoByErrorCode();
}

int MethodResult::GetErrorCode(void) const
{
    return errorCode_;
}

void MethodResult::SetResult(const std::string& value)
{
    result_ = value;
}

const std::string& MethodResult::GetResult(void) const
{
    return result_;
}

void MethodResult::SetErrorResult(napi_value value)
{
    errorResult_ = value;
}

napi_value MethodResult::GetErrorResult(void) const
{
    return errorResult_;
}

void MethodResult::SetOkResult(napi_value value)
{
    okResult_ = value;
}

napi_value MethodResult::GetOkResult(void) const
{
    return okResult_;
}

void MethodResult::CreateErrorObject(napi_env env)
{
    GetErrorInfoByErrorCode();
    errorResult_ = PluginUtilsNApi::CreateErrorMessage(env, errorCode_, errcodeMessage_);
}

void MethodResult::CreateDefaultJsonString(void)
{
    NapiRawValue rawValue { .errorCode = errorCode_, .errorMessage = errcodeMessage_ };
    auto encoded = BridgeJsonCodec::GetInstance().Encode(rawValue);
    result_ = encoded->value;
}

void MethodResult::ParsePlatformMethodResult(napi_env env, const std::string& result)
{
    LOGI("ParsePlatformMethodResult: result=%{public}s", result.c_str());
    errorCode_ = static_cast<int>(ErrorCode::BRIDGE_ERROR_NO);

    napi_value resultValue = nullptr;
    if (!result.empty()) {
        DecodeValue decodeValue { .env = env, .value = result };
        auto decoded = BridgeJsonCodec::GetInstance().Decode(decodeValue);
        errorCode_ = decoded->errorCode;
        errcodeMessage_ = decoded->errorMessage;
        resultValue = decoded->value;
    }

    CreateErrorObject(env);
    if (errorCode_ == 0) {
        okResult_ = resultValue;
    } else {
        napi_get_null(env, &okResult_);
    }
}

void MethodResult::ParsePlatformMethodResultBinary(napi_env env, int errorCode,
    const std::string& errorMessage, std::unique_ptr<Ace::Platform::BufferMapping> resultData)
{
    errorCode_ = errorCode;
    errcodeMessage_ = errorMessage;
    okResult_ = nullptr;
    CreateErrorObject(env);
    if (errorCode_ == 0) {
        auto decoded = BridgeBinaryCodec::GetInstance().DecodeBuffer(resultData->GetMapping(), resultData->GetSize());
        okResult_ = MethodDataConverter::ConvertToNapiValue(env, *decoded);
    } else {
        napi_get_null(env, &okResult_);
    }
}

void MethodResult::ParseJSMethodResultBinary(napi_env env, napi_value result)
{
    const auto& resultValue = MethodDataConverter::ConvertToCodecableValue(env, result);
    binaryResult_ = BridgeBinaryCodec::GetInstance().EncodeBuffer(resultValue);
}

std::vector<uint8_t>* MethodResult::GetResultBinary(void)
{
    return binaryResult_;
}

std::string MethodResult::GetErrorMessage() const
{
    return errcodeMessage_;
}

void MethodResult::ParseJSMethodResult(napi_env env, napi_value result)
{
    NapiRawValue rawValue { .env = env, .value = result, 
        .errorCode = errorCode_, .errorMessage = errcodeMessage_ };
    auto encoded = BridgeJsonCodec::GetInstance().Encode(rawValue);
    result_ = encoded->value;
}

void MethodResult::CreateMethodResultForError(void)
{
    NapiRawValue rawValue { .errorCode = errorCode_,
        .errorMessage = errcodeMessage_, .isForError = true };
    auto encoded = BridgeJsonCodec::GetInstance().Encode(rawValue);
    result_ = encoded->value;
}
} // namespace OHOS::Plugin::Bridge