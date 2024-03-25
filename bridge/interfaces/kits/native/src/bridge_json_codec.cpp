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

#include "bridge_json_codec.h"

#include "napi_utils.h"

namespace OHOS::Plugin::Bridge {
/*
 * Method parameter:
 * { "0":0, "1":"OK", "2":value }
 * Method returns values in a uniform format:
 * { "errorCode":0, "errorMessage":"OK", "result":value }
 * Message response returns values in a uniform format:
 * { "result":value, "errorCode":errorCode }
 */
static constexpr const char* MESSAGE_RESPONSE_RESULT = "result";
static constexpr const char* MESSAGE_RESPONSE_ERROR_CODE = "errorCode";
static constexpr const char* MESSAGE_RESPONSE_ERROR_MESSAGE = "errorMessage";

const BridgeJsonCodec& BridgeJsonCodec::GetInstance()
{
    static BridgeJsonCodec sInstance;
    return sInstance;
}

std::unique_ptr<DecodeValue> BridgeJsonCodec::EncodeInner(const NapiRawValue& data) const
{
    if (data.argc > -1) {
        auto parameter = std::make_unique<DecodeValue>();
        NAPIUtils::NapiValuesToJsonString(data.env, data.argc, data.argValue, parameter->value);
        return std::move(parameter);
    }

    Json json {};
    if (data.value != nullptr) {
        json[MESSAGE_RESPONSE_RESULT] = NAPIUtils::PlatformParams(data.env, data.value);
    } else if (data.isForError) {
        json[MESSAGE_RESPONSE_RESULT] = 0;
    } else {
        json[MESSAGE_RESPONSE_RESULT] = "";
    }

    if (data.errorCode != -1) {
        json[MESSAGE_RESPONSE_ERROR_CODE] = data.errorCode;
    }

    if (!data.errorMessage.empty()) {
        json[MESSAGE_RESPONSE_ERROR_MESSAGE] = data.errorMessage;
    }

    auto parameter = std::make_unique<DecodeValue>();
    parameter->value = json.dump();

    return std::move(parameter);
}

std::unique_ptr<NapiRawValue> BridgeJsonCodec::DecodeInner(const DecodeValue& decodeValue) const
{
    auto rawValue = std::make_unique<NapiRawValue>();

    auto jsonObject = Json::parse(decodeValue.value, nullptr, false);

    auto it = jsonObject.find(MESSAGE_RESPONSE_ERROR_CODE);
    if (it != jsonObject.end()) {
        rawValue->errorCode = NAPIUtils::NAPI_GetErrorCodeFromJson(jsonObject.at(MESSAGE_RESPONSE_ERROR_CODE));
    }

    it = jsonObject.find(MESSAGE_RESPONSE_RESULT);
    Json resultJson;
    rawValue->value = NAPIUtils::NAPI_GetParams(decodeValue.env, resultJson);
    if (it != jsonObject.end()) {
        rawValue->value = NAPIUtils::NAPI_GetParams(decodeValue.env, jsonObject.at(MESSAGE_RESPONSE_RESULT));
    }

    it = jsonObject.find(MESSAGE_RESPONSE_ERROR_MESSAGE);
    if (it != jsonObject.end()) {
        rawValue->errorMessage = jsonObject.at(MESSAGE_RESPONSE_ERROR_MESSAGE).get<std::string>();
    }

    return std::move(rawValue);
}
} // OHOS::Plugin::Bridge