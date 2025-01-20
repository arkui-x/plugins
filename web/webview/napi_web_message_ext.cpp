/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "napi_web_message_ext.h"

#include <unordered_map>

#include "business_error.h"

#include "inner_api/plugin_utils_inner.h"
#include "js_web_webview.h"
#include "log.h"
#include "web_errors.h"
#include "web_message_port.h"

using namespace OHOS::NWebError;

namespace OHOS::Plugin {
thread_local napi_ref NapiWebMessageExt::g_webMsgExtClassRef;
napi_value NapiWebMessageExt::Init(napi_env env, napi_value exports)
{
    napi_value jsMsgTypeEnum = nullptr;
    napi_property_descriptor webMsgTypeProperties[] = {
        DECLARE_NAPI_STATIC_PROPERTY("NOT_SUPPORT", NapiParseUtils::ToInt32Value(env,
            static_cast<int32_t>(WebMessageType::NOTSUPPORT))),
        DECLARE_NAPI_STATIC_PROPERTY("STRING", NapiParseUtils::ToInt32Value(env,
            static_cast<int32_t>(WebMessageType::STRING))),
        DECLARE_NAPI_STATIC_PROPERTY("NUMBER", NapiParseUtils::ToInt32Value(env,
            static_cast<int32_t>(WebMessageType::NUMBER))),
        DECLARE_NAPI_STATIC_PROPERTY("BOOLEAN", NapiParseUtils::ToInt32Value(env,
            static_cast<int32_t>(WebMessageType::BOOLEAN))),
        DECLARE_NAPI_STATIC_PROPERTY("ARRAY_BUFFER", NapiParseUtils::ToInt32Value(env,
            static_cast<int32_t>(WebMessageType::ARRAYBUFFER))),
        DECLARE_NAPI_STATIC_PROPERTY("ARRAY", NapiParseUtils::ToInt32Value(env,
            static_cast<int32_t>(WebMessageType::ARRAY))),
        DECLARE_NAPI_STATIC_PROPERTY("ERROR", NapiParseUtils::ToInt32Value(env,
            static_cast<int32_t>(WebMessageType::ERROR)))
    };
    napi_define_class(env, WEB_MESSAGE_TYPE_ENUM_NAME.c_str(), WEB_MESSAGE_TYPE_ENUM_NAME.length(),
        NapiParseUtils::CreateEnumConstructor, nullptr, sizeof(webMsgTypeProperties) /
        sizeof(webMsgTypeProperties[0]), webMsgTypeProperties, &jsMsgTypeEnum);
    napi_set_named_property(env, exports, WEB_MESSAGE_TYPE_ENUM_NAME.c_str(), jsMsgTypeEnum);

    napi_value webMsgExtClass = nullptr;
    napi_property_descriptor webMsgExtClsProperties[] = {
        DECLARE_NAPI_FUNCTION("getType", NapiWebMessageExt::GetType),
        DECLARE_NAPI_FUNCTION("getString", NapiWebMessageExt::GetString),
        DECLARE_NAPI_FUNCTION("getNumber", NapiWebMessageExt::GetNumber),
        DECLARE_NAPI_FUNCTION("getBoolean", NapiWebMessageExt::GetBoolean),
        DECLARE_NAPI_FUNCTION("getArrayBuffer", NapiWebMessageExt::GetArrayBuffer),
        DECLARE_NAPI_FUNCTION("getArray", NapiWebMessageExt::GetArray),
        DECLARE_NAPI_FUNCTION("getError", NapiWebMessageExt::GetError),
        DECLARE_NAPI_FUNCTION("setType", NapiWebMessageExt::SetType),
        DECLARE_NAPI_FUNCTION("setString", NapiWebMessageExt::SetString),
        DECLARE_NAPI_FUNCTION("setNumber", NapiWebMessageExt::SetNumber),
        DECLARE_NAPI_FUNCTION("setBoolean", NapiWebMessageExt::SetBoolean),
        DECLARE_NAPI_FUNCTION("setArrayBuffer", NapiWebMessageExt::SetArrayBuffer),
        DECLARE_NAPI_FUNCTION("setArray", NapiWebMessageExt::SetArray),
        DECLARE_NAPI_FUNCTION("setError", NapiWebMessageExt::SetError)
    };
    NAPI_CALL(env, napi_define_class(env, WEB_EXT_MSG_CLASS_NAME.c_str(), WEB_EXT_MSG_CLASS_NAME.length(),
        NapiWebMessageExt::JsConstructor, nullptr, sizeof(webMsgExtClsProperties) / sizeof(webMsgExtClsProperties[0]),
        webMsgExtClsProperties, &webMsgExtClass));
        napi_create_reference(env, webMsgExtClass, 1, &g_webMsgExtClassRef);
        napi_set_named_property(env, exports, WEB_EXT_MSG_CLASS_NAME.c_str(), webMsgExtClass);
    return exports;
}

WebMessageExt* GetWebMessageExt(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);

    WebMessageExt* webMessageExt = nullptr;
    napi_status status = napi_unwrap(env, thisVar, (void**)&webMessageExt);
    if ((!webMessageExt) || (status != napi_ok)) {
        BusinessError::ThrowErrorByErrcode(env, INIT_ERROR);
        return nullptr;
    }
    return webMessageExt;
}

napi_valuetype GetArrayValueType(napi_env env, napi_value array, bool& isDouble)
{
    uint32_t arrayLength = 0;
    napi_get_array_length(env, array, &arrayLength);
    napi_valuetype valueTypeFirst = napi_undefined;
    napi_valuetype valueTypeCur = napi_undefined;
    for (uint32_t i = 0; i < arrayLength; ++i) {
        napi_value obj = nullptr;
        napi_get_element(env, array, i, &obj);
        napi_typeof(env, obj, &valueTypeCur);
        if (i == 0) {
            valueTypeFirst = valueTypeCur;
        }
        if (valueTypeCur != napi_string && valueTypeCur != napi_number && valueTypeCur != napi_boolean) {
            BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
            return napi_undefined;
        }
        if (valueTypeCur != valueTypeFirst) {
            BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
            return napi_undefined;
        }
        if (valueTypeFirst == napi_number) {
            int32_t elementInt32 = 0;
            double elementDouble = 0.0;
            bool isReadValue32 = napi_get_value_int32(env, obj, &elementInt32) == napi_ok;
            bool isReadDouble = napi_get_value_double(env, obj, &elementDouble) == napi_ok;
            if (isReadValue32 && isReadDouble) {
                constexpr double MINIMAL_ERROR = 0.000001;
                isDouble = abs(elementDouble - elementInt32 * 1.0) > MINIMAL_ERROR;
            } else if (isReadDouble) {
                isDouble = true;
            }
        }
    }
    return valueTypeFirst;
}

void SetArrayHandlerBoolean(napi_env env, napi_value array, WebMessageExt* webMessageExt)
{
    std::vector<bool> outValue;
    if (!NapiParseUtils::ParseBooleanArray(env, array, outValue)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return;
    }
    webMessageExt->SetBooleanArray(outValue);
}

void SetArrayHandlerString(napi_env env, napi_value array, WebMessageExt* webMessageExt)
{
    std::vector<std::string> outValue;
    if (!NapiParseUtils::ParseStringArray(env, array, outValue)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return;
    }
    webMessageExt->SetStringArray(outValue);
}

void SetArrayHandlerInteger(napi_env env, napi_value array, WebMessageExt* webMessageExt)
{
    std::vector<int64_t> outValue;
    if (!NapiParseUtils::ParseInt64Array(env, array, outValue)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return;
    }
    webMessageExt->SetInt64Array(outValue);
}

void SetArrayHandlerDouble(napi_env env, napi_value array, WebMessageExt* webMessageExt)
{
    std::vector<double> outValue;
    if (!NapiParseUtils::ParseDoubleArray(env, array, outValue)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return;
    }
    webMessageExt->SetDoubleArray(outValue);
}

napi_value NapiWebMessageExt::JsConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);

    auto webMsg = std::make_shared<OHOS::Plugin::WebMessage>(WebValue::Type::NONE);
    WebMessageExt* webMessageExt = new (std::nothrow) WebMessageExt(webMsg);
    if (webMessageExt == nullptr) {
        LOGE("new msg port failed");
        return nullptr;
    }
    NAPI_CALL(env, napi_wrap(
                       env, thisVar, webMessageExt,
                       [](napi_env env, void* data, void* hint) {
                           WebMessageExt* webMessageExt = static_cast<WebMessageExt*>(data);
                           if (webMessageExt) {
                               delete webMessageExt;
                           }
                       },
                       nullptr, nullptr));
    return thisVar;
}

napi_value NapiWebMessageExt::GetType(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    if (status != napi_status::napi_ok) {
        LOGE("napi_get_cb_info status not ok");
        return result;
    }

    if (thisVar == nullptr) {
        LOGE("napi_get_cb_info thisVar is nullptr");
        return result;
    }

    WebMessageExt* webMessageExt = nullptr;
    status = napi_unwrap(env, thisVar, (void**)&webMessageExt);
    if ((!webMessageExt) || (status != napi_ok)) {
        BusinessError::ThrowErrorByErrcode(env, INIT_ERROR);
        return nullptr;
    }

    int32_t type = webMessageExt->GetType();
    status = napi_create_int32(env, type, &result);
    if (status != napi_status::napi_ok) {
        LOGE("napi_create_int32 failed.");
        return result;
    }
    return result;
}

napi_value NapiWebMessageExt::GetString(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };

    WebMessageExt* webJsMessageExt = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&webJsMessageExt));
    if (webJsMessageExt == nullptr) {
        LOGE("unwrap webJsMessageExt failed.");
        return result;
    }

    if (webJsMessageExt->GetType() != static_cast<int32_t>(WebMessageType::STRING)) {
        BusinessError::ThrowErrorByErrcode(env, TYPE_NOT_MATCH_WITCH_VALUE);
        return nullptr;
    }

    NapiParseUtils::ConvertWebToNapiValue(env, webJsMessageExt->GetData(), result);
    return result;
}

napi_value NapiWebMessageExt::GetNumber(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };

    WebMessageExt* webJsMessageExt = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&webJsMessageExt));
    if (webJsMessageExt == nullptr) {
        LOGE("unwrap webJsMessageExt failed.");
        return result;
    }

    if (webJsMessageExt->GetType() != static_cast<int32_t>(WebMessageType::NUMBER)) {
        BusinessError::ThrowErrorByErrcode(env, TYPE_NOT_MATCH_WITCH_VALUE);
        LOGE("GetNumber webJsMessageExt failed,not match");
        return nullptr;
    }

    NapiParseUtils::ConvertWebToNapiValue(env, webJsMessageExt->GetData(), result);
    return result;
}

napi_value NapiWebMessageExt::GetBoolean(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };

    WebMessageExt* webJsMessageExt = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&webJsMessageExt));
    if (webJsMessageExt == nullptr) {
        LOGE("unwrap webJsMessageExt failed.");
        return result;
    }

    if (webJsMessageExt->GetType() != static_cast<int32_t>(WebMessageType::BOOLEAN)) {
        BusinessError::ThrowErrorByErrcode(env, TYPE_NOT_MATCH_WITCH_VALUE);
        return nullptr;
    }

    NapiParseUtils::ConvertWebToNapiValue(env, webJsMessageExt->GetData(), result);
    return result;
}

napi_value NapiWebMessageExt::GetArrayBuffer(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };

    WebMessageExt* webJsMessageExt = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&webJsMessageExt));
    if (webJsMessageExt == nullptr) {
        LOGE("unwrap webJsMessageExt failed.");
        return result;
    }

    if (webJsMessageExt->GetType() != static_cast<int32_t>(WebMessageType::ARRAYBUFFER)) {
        BusinessError::ThrowErrorByErrcode(env, TYPE_NOT_MATCH_WITCH_VALUE);
        return nullptr;
    }
    NapiParseUtils::ConvertWebToNapiValue(env, webJsMessageExt->GetData(), result);
    return result;
}

napi_value NapiWebMessageExt::GetArray(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };

    WebMessageExt* webJsMessageExt = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&webJsMessageExt));
    if (webJsMessageExt == nullptr) {
        LOGE("unwrap webJsMessageExt failed.");
        return result;
    }

    if (webJsMessageExt->GetType() != static_cast<int32_t>(WebMessageType::ARRAY)) {
        BusinessError::ThrowErrorByErrcode(env, TYPE_NOT_MATCH_WITCH_VALUE);
        return nullptr;
    }

    NapiParseUtils::ConvertWebToNapiValue(env, webJsMessageExt->GetData(), result);
    return result;
}

napi_value NapiWebMessageExt::GetError(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };

    WebMessageExt* webJsMessageExt = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&webJsMessageExt));
    if (webJsMessageExt == nullptr) {
        LOGE("unwrap webJsMessageExt failed.");
        return result;
    }

    if (webJsMessageExt->GetType() != static_cast<int32_t>(WebMessageType::ERROR)) {
        BusinessError::ThrowErrorByErrcode(env, TYPE_NOT_MATCH_WITCH_VALUE);
        return nullptr;
    }

    NapiParseUtils::ConvertWebToNapiValue(env, webJsMessageExt->GetData(), result);
    return result;
}

napi_value NapiWebMessageExt::SetType(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };
    int type = -1;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (status != napi_ok) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_TYPE_INVALID, "type"));
        LOGE("NapiWebMessageExt::SetType napi_get_cb_info failed");
        return result;
    }
    if (thisVar == nullptr) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NOT_NULL, "type"));
        LOGE("NapiWebMessageExt::SetType thisVar is null");
        return result;
    }
    if (argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_ONE, "one"));
        return result;
    }
    if (!NapiParseUtils::ParseInt32(env, argv[0], type)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR, ParamCheckErrorMsgTemplate::TYPE_ALL_INT);
        return result;
    }
    if (type <= static_cast<int>(WebMessageType::NOTSUPPORT) || type > static_cast<int>(WebMessageType::ERROR)) {
        BusinessError::ThrowErrorByErrcode(env, TYPE_NOT_MATCH_WITCH_VALUE);
        return result;
    }
    WebMessageExt* webMessageExt = GetWebMessageExt(env, info);
    if (!webMessageExt) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NOT_NULL, "type"));
        LOGE("NapiWebMessageExt::SetType webMessageExt is null");
        return result;
    }
    std::shared_ptr<WebMessage> messagePtr = std::make_shared<WebMessage>(WebValue::Type::STRING);
    webMessageExt->SetData(messagePtr);
    webMessageExt->SetType(type);
    return result;
}

napi_value NapiWebMessageExt::SetString(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_ONE, "one"));
        return result;
    }

    std::string value;
    if (!NapiParseUtils::ParseString(env, argv[0], value)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "message", "string"));
        return result;
    }
    WebMessageExt* webMessageExt = GetWebMessageExt(env, info);
    int32_t type = webMessageExt->GetType();
    if (type != static_cast<int32_t>(WebMessageType::STRING)) {
        LOGE("web message SetString error type:%{public}d", type);
        BusinessError::ThrowErrorByErrcode(env, TYPE_NOT_MATCH_WITCH_VALUE);
        return result;
    }
    webMessageExt->SetString(value);
    return result;
}

napi_value NapiWebMessageExt::SetNumber(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_ONE, "one"));
        return result;
    }

    double value = 0;
    if (!NapiParseUtils::ParseDouble(env, argv[0], value)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "message", "number"));
        return result;
    }

    WebMessageExt* webMessageExt = nullptr;
    napi_status status = napi_unwrap(env, thisVar, (void**)&webMessageExt);
    if ((!webMessageExt) || (status != napi_ok)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_TYPE_INVALID, "message"));
        return result;
    }

    int32_t type = webMessageExt->GetType();
    if (type != static_cast<int32_t>(WebMessageType::NUMBER)) {
        LOGE("web message SetNumber error type:%{public}d", type);
        BusinessError::ThrowErrorByErrcode(env, TYPE_NOT_MATCH_WITCH_VALUE);
        return result;
    }
    webMessageExt->SetNumber(value);
    return result;
}

napi_value NapiWebMessageExt::SetBoolean(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_ONE, "one"));
        return result;
    }

    bool value = 0;
    if (!NapiParseUtils::ParseBoolean(env, argv[0], value)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "message", "boolean"));
        return result;
    }

    WebMessageExt* webMessageExt = nullptr;
    napi_status status = napi_unwrap(env, thisVar, (void**)&webMessageExt);
    if ((!webMessageExt) || (status != napi_ok)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_TYPE_INVALID, "message"));
        return result;
    }

    int32_t type = webMessageExt->GetType();
    if (type != static_cast<int32_t>(WebMessageType::BOOLEAN)) {
        LOGE("web message SetBoolean error type:%{public}d", type);
        BusinessError::ThrowErrorByErrcode(env, TYPE_NOT_MATCH_WITCH_VALUE);
        return result;
    }
    webMessageExt->SetBoolean(value);
    return result;
}

napi_value NapiWebMessageExt::SetArrayBuffer(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_ONE, "one"));
        return result;
    }

    bool isArrayBuffer = false;
    NAPI_CALL(env, napi_is_arraybuffer(env, argv[INTEGER_ZERO], &isArrayBuffer));
    if (!isArrayBuffer) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "message", "arrayBuffer"));
        return result;
    }

    uint8_t* arrBuf = nullptr;
    size_t byteLength = 0;
    napi_get_arraybuffer_info(env, argv[INTEGER_ZERO], (void**)&arrBuf, &byteLength);
    std::vector<uint8_t> vecData(arrBuf, arrBuf + byteLength);
    WebMessageExt* webMessageExt = nullptr;
    napi_status status = napi_unwrap(env, thisVar, (void**)&webMessageExt);
    if ((!webMessageExt) || (status != napi_ok)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_TYPE_INVALID, "message"));
        return result;
    }

    int32_t type = webMessageExt->GetType();
    if (type != static_cast<int32_t>(WebMessageType::ARRAYBUFFER)) {
        LOGE("web message SetArrayBuffer error type:%{public}d", type);
        BusinessError::ThrowErrorByErrcode(env, TYPE_NOT_MATCH_WITCH_VALUE);
        return result;
    }
    webMessageExt->SetArrayBuffer(vecData);
    return result;
}

napi_value NapiWebMessageExt::SetArray(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_ONE, "one"));
        return result;
    }
    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, argv[INTEGER_ZERO], &isArray));
    if (!isArray) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "message", "array"));
        return result;
    }
    WebMessageExt* webMessageExt = nullptr;
    napi_status status = napi_unwrap(env, thisVar, (void**)&webMessageExt);
    if ((!webMessageExt) || (status != napi_ok)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_TYPE_INVALID, "message"));
        return result;
    }
    int32_t type = webMessageExt->GetType();
    if (type != static_cast<int32_t>(WebMessageType::ARRAY)) {
        LOGE("web message SetArray error type:%{public}d", type);
        BusinessError::ThrowErrorByErrcode(env, TYPE_NOT_MATCH_WITCH_VALUE);
        return result;
    }
    bool isDouble = false;
    napi_valuetype valueType = GetArrayValueType(env, argv[INTEGER_ZERO], isDouble);
    if (valueType == napi_undefined) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "message", "number"));
        return result;
    }
    using SetArrayHandler = std::function<void(napi_env, napi_value, WebMessageExt*)>;
    const std::unordered_map<napi_valuetype, SetArrayHandler> functionMap = {
        { napi_boolean, SetArrayHandlerBoolean },
        { napi_string, SetArrayHandlerString },
        { napi_number, [isDouble](napi_env env, napi_value array, WebMessageExt* msgExt) {
            isDouble ? SetArrayHandlerDouble(env, array, msgExt)
                     : SetArrayHandlerInteger(env, array, msgExt);
        } }
    };
    auto it = functionMap.find(valueType);
    if (it != functionMap.end()) {
        it->second(env, argv[INTEGER_ZERO], webMessageExt);
    }
    return result;
}

napi_value NapiWebMessageExt::SetError(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_ONE, "one"));
        return result;
    }

    bool isError = false;
    NAPI_CALL(env, napi_is_error(env, argv[INTEGER_ZERO], &isError));
    if (!isError) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "message", "error"));
        return result;
    }

    napi_value nameObj = 0;
    napi_get_named_property(env, argv[INTEGER_ZERO], "name", &nameObj);
    std::string nameVal;
    if (!NapiParseUtils::ParseString(env, nameObj, nameVal)) {
        return result;
    }

    napi_value msgObj = 0;
    napi_get_named_property(env, argv[INTEGER_ZERO], "message", &msgObj);
    std::string msgVal;
    if (!NapiParseUtils::ParseString(env, msgObj, msgVal)) {
        return result;
    }

    WebMessageExt* webMessageExt = nullptr;
    napi_status status = napi_unwrap(env, thisVar, (void**)&webMessageExt);
    if ((!webMessageExt) || (status != napi_ok)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_TYPE_INVALID, "message"));
        return result;
    }

    int32_t type = webMessageExt->GetType();
    if (type != static_cast<int32_t>(WebMessageType::ERROR)) {
        LOGE("web message SetError error type:%{public}d", type);
        BusinessError::ThrowErrorByErrcode(env, TYPE_NOT_MATCH_WITCH_VALUE);
        return result;
    }
    webMessageExt->SetError(nameVal, msgVal);
    return result;
}
} // namespace OHOS::Plugin