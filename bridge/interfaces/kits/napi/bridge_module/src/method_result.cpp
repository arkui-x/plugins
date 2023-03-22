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

#include "error_code.h"
#include "log.h"
#include "napi_utils.h"
#include "plugins/interfaces/native/inner_utils/plugin_inner_napi_utils.h"
#include "plugins/interfaces/native/plugin_c_utils.h"

namespace OHOS::Plugin::Bridge {
/*
 * Method parameter:
 * { "0":0, "1":"OK", "2":value }
 * Method returns values in a uniform format：
 * { "errorcode":0, "errormessage":"OK", "result":value }
 * Message response returns values in a uniform format：
 * { "result":value, "errorcode":errorcode }
 */
static constexpr const char* METHOD_RESULT_ERROR_CODE = "errorcode";
static constexpr const char* METHOD_RESULT_ERROR_MESSAGE = "errormessage";
static constexpr const char* METHOD_RESULT_RESULT = "result";

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
    errorResult_ = PluginInnerNApiUtils::CreateErrorMessage(env, errorCode_, errcodeMessage_);
}

void MethodResult::CreateDefaultJsonString(void)
{
    Json json = Json {};
    json[METHOD_RESULT_ERROR_CODE] = errorCode_;
    json[METHOD_RESULT_ERROR_MESSAGE] = errcodeMessage_;
    json[METHOD_RESULT_RESULT] = "";
    result_ = json.dump();
}

void MethodResult::ParsePlatformMethodResult(napi_env env, const std::string& result)
{
    LOGI("ParsePlatformMethodResult: result=%{public}s", result.c_str());
    errorCode_ = static_cast<int>(ErrorCode::BRIDGE_ERROR_NO);
    Json resultJson;
    if (!result.empty()) {
        auto jsonObject = Json::parse(result, nullptr, false);
        auto it = jsonObject.find(METHOD_RESULT_ERROR_CODE);
        if (it != jsonObject.end()) {
            errorCode_ = jsonObject.at(METHOD_RESULT_ERROR_CODE).get<int>();
        }
        it = jsonObject.find(METHOD_RESULT_ERROR_MESSAGE);
        if (it != jsonObject.end()) {
            errcodeMessage_ = jsonObject.at(METHOD_RESULT_ERROR_MESSAGE).get<std::string>();
        }
        it = jsonObject.find(METHOD_RESULT_RESULT);
        if (it != jsonObject.end()) {
            resultJson = jsonObject.at(METHOD_RESULT_RESULT);
        }
    }

    CreateErrorObject(env);
    if (errorCode_ == 0) {
        okResult_ = NAPIUtils::NAPI_GetPremers(env, resultJson);
    } else {
        napi_get_null(env, &okResult_);
    }
}

void MethodResult::ParseJSMethodResult(napi_env env, napi_value result)
{
    Json jsonRsult = Json {};
    jsonRsult[METHOD_RESULT_ERROR_CODE] = errorCode_;
    jsonRsult[METHOD_RESULT_ERROR_MESSAGE] = errcodeMessage_;
    jsonRsult[METHOD_RESULT_RESULT] = NAPIUtils::PlatformPremers(env, result);
    result_ = jsonRsult.dump();
}

void MethodResult::CreateMethodResultForError(void)
{
    Json jsonRsult = Json {};
    jsonRsult[METHOD_RESULT_ERROR_CODE] = errorCode_;
    jsonRsult[METHOD_RESULT_ERROR_MESSAGE] = errcodeMessage_;
    jsonRsult[METHOD_RESULT_RESULT] = 0;
    result_ = jsonRsult.dump();
}
} // namespace OHOS::Plugin::Bridge