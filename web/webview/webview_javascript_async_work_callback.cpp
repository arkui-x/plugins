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

#include "webview_javascript_async_work_callback.h"

#include "napi_parse_utils.h"
#include "log.h"
#include "business_error.h"
#include "web_errors.h"

using namespace OHOS::NWebError;

namespace OHOS::Plugin {
const std::string JS_EXCUTE_MSG_ENUM_NAME = "JsMessageType";
const std::string JS_EXT_MSG_CLASS_NAME = "JsMessageExt";
thread_local napi_ref g_jsMsgExtClassRef;

napi_value NapiJsMessageExt::JsConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    return thisVar;
}

int32_t WebJsMessageExt::ConvertToJsType(WebValue::Type type)
{
    JsMessageType jsMessageType = JsMessageType::NOTSUPPORT;
    switch (type) {
        case WebValue::Type::STRING:
            jsMessageType = JsMessageType::STRING;
            break;
        case WebValue::Type::INTEGER:
        case WebValue::Type::DOUBLE:
            jsMessageType = JsMessageType::NUMBER;
            break;
        case WebValue::Type::BOOLEAN:
            jsMessageType = JsMessageType::BOOLEAN;
            break;
        case WebValue::Type::BINARY:
            jsMessageType = JsMessageType::ARRAYBUFFER;
            break;
        case WebValue::Type::BOOLEANARRAY:
        case WebValue::Type::STRINGARRAY:
        case WebValue::Type::DOUBLEARRAY:
        case WebValue::Type::INT64ARRAY:
            jsMessageType = JsMessageType::ARRAY;
            break;
        default:
            jsMessageType = JsMessageType::NOTSUPPORT;
            break;
    }
    return static_cast<int32_t>(jsMessageType);
}

int32_t WebJsMessageExt::GetType()
{
    if (value_) {
        return ConvertToJsType(value_->GetType());
    }
    return static_cast<int32_t>(JsMessageType::NOTSUPPORT);
}

std::string WebJsMessageExt::GetString()
{
    if (value_) {
        return value_->GetString();
    }
    return "";
}

double WebJsMessageExt::GetNumber()
{
    if (value_) {
        return value_->GetDouble();
    }
    return 0;
}

bool WebJsMessageExt::GetBoolean()
{
    if (value_) {
        return value_->GetBoolean();
    }
    return false;
}

napi_value NapiJsMessageExt::GetType(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };

    WebJsMessageExt *webJsMessageExt = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (status != napi_status::napi_ok) {
        LOGE("napi_get_cb_info failed.");
        return result;
    }
    status = napi_unwrap(env, thisVar, (void **)&webJsMessageExt);
    if (status != napi_status::napi_ok) {
        LOGE("napi_unwrap failed.");
        return result;
    }
    if (webJsMessageExt == nullptr) {
        LOGE("unwrap webJsMessageExt failed.");
        return result;
    }

    int32_t type = webJsMessageExt->GetType();
    status = napi_create_int32(env, type, &result);
    if (status != napi_status::napi_ok) {
        LOGE("napi_create_int32 failed.");
        return result;
    }
    return result;
}

napi_value NapiJsMessageExt::GetString(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };

    WebJsMessageExt *webJsMessageExt = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void **)&webJsMessageExt));
    if (webJsMessageExt == nullptr) {
        LOGE("unwrap webJsMessageExt failed.");
        return result;
    }

    if (webJsMessageExt->GetType() != static_cast<int32_t>(JsMessageType::STRING)) {
        BusinessError::ThrowErrorByErrcode(env, TYPE_NOT_MATCH_WITCH_VALUE);
        return nullptr;
    }

    NapiParseUtils::ConvertWebToNapiValue(env, webJsMessageExt->GetJsMsgResult(), result);
    return result;
}

napi_value NapiJsMessageExt::GetNumber(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };

    WebJsMessageExt *webJsMessageExt = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void **)&webJsMessageExt));
    if (webJsMessageExt == nullptr) {
        LOGE("unwrap webJsMessageExt failed.");
        return result;
    }

    if (webJsMessageExt->GetType() != static_cast<int32_t>(JsMessageType::NUMBER)) {
        BusinessError::ThrowErrorByErrcode(env, TYPE_NOT_MATCH_WITCH_VALUE);
        LOGE("GetNumber webJsMessageExt failed not match");
        return nullptr;
    }

    NapiParseUtils::ConvertWebToNapiValue(env, webJsMessageExt->GetJsMsgResult(), result);
    
    return result;
}

napi_value NapiJsMessageExt::GetBoolean(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };

    WebJsMessageExt *webJsMessageExt = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void **)&webJsMessageExt));
    if (webJsMessageExt == nullptr) {
        LOGE("unwrap webJsMessageExt failed.");
        return result;
    }

    if (webJsMessageExt->GetType() != static_cast<int32_t>(JsMessageType::BOOLEAN)) {
        BusinessError::ThrowErrorByErrcode(env, TYPE_NOT_MATCH_WITCH_VALUE);
        return nullptr;
    }

    NapiParseUtils::ConvertWebToNapiValue(env, webJsMessageExt->GetJsMsgResult(), result);
    return result;
}

napi_value NapiJsMessageExt::GetArrayBuffer(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };

    WebJsMessageExt *webJsMessageExt = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void **)&webJsMessageExt));
    if (webJsMessageExt == nullptr) {
        LOGE("unwrap webJsMessageExt failed.");
        return result;
    }

    if (webJsMessageExt->GetType() != static_cast<int32_t>(JsMessageType::ARRAYBUFFER)) {
        BusinessError::ThrowErrorByErrcode(env, TYPE_NOT_MATCH_WITCH_VALUE);
        return nullptr;
    }
    NapiParseUtils::ConvertWebToNapiValue(env, webJsMessageExt->GetJsMsgResult(), result);
    return result;
}

napi_value NapiJsMessageExt::GetArray(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };

    WebJsMessageExt *webJsMessageExt = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void **)&webJsMessageExt));
    if (webJsMessageExt == nullptr) {
        LOGE("unwrap webJsMessageExt failed.");
        return result;
    }

    if (webJsMessageExt->GetType() != static_cast<int32_t>(JsMessageType::ARRAY)) {
        BusinessError::ThrowErrorByErrcode(env, TYPE_NOT_MATCH_WITCH_VALUE);
        return nullptr;
    }

    NapiParseUtils::ConvertWebToNapiValue(env, webJsMessageExt->GetJsMsgResult(), result);
    return result;
}

void AsyncJavaScriptExtEvaluteJSResultCallbackInfo::InitJSExcute(napi_env env, napi_value exports, napi_ref& jsMsgExtClassRef)
{
    napi_value jsMsgTypeEnum = nullptr;
    napi_property_descriptor jsMsgTypeProperties[] = {
        DECLARE_NAPI_STATIC_PROPERTY("NOT_SUPPORT", NapiParseUtils::ToInt32Value(env,
            static_cast<int32_t>(JsMessageType::NOTSUPPORT))),
        DECLARE_NAPI_STATIC_PROPERTY("STRING", NapiParseUtils::ToInt32Value(env,
            static_cast<int32_t>(JsMessageType::STRING))),
        DECLARE_NAPI_STATIC_PROPERTY("NUMBER", NapiParseUtils::ToInt32Value(env,
            static_cast<int32_t>(JsMessageType::NUMBER))),
        DECLARE_NAPI_STATIC_PROPERTY("BOOLEAN", NapiParseUtils::ToInt32Value(env,
            static_cast<int32_t>(JsMessageType::BOOLEAN))),
        DECLARE_NAPI_STATIC_PROPERTY("ARRAY_BUFFER", NapiParseUtils::ToInt32Value(env,
            static_cast<int32_t>(JsMessageType::ARRAYBUFFER))),
        DECLARE_NAPI_STATIC_PROPERTY("ARRAY", NapiParseUtils::ToInt32Value(env,
            static_cast<int32_t>(JsMessageType::ARRAY)))
    };
    napi_define_class(env, JS_EXCUTE_MSG_ENUM_NAME.c_str(), JS_EXCUTE_MSG_ENUM_NAME.length(),
        NapiParseUtils::CreateEnumConstructor, nullptr, sizeof(jsMsgTypeProperties) /
        sizeof(jsMsgTypeProperties[0]), jsMsgTypeProperties, &jsMsgTypeEnum);
    napi_set_named_property(env, exports, JS_EXCUTE_MSG_ENUM_NAME.c_str(), jsMsgTypeEnum);

    napi_value jsMsgExtClass = nullptr;
    napi_property_descriptor jsMsgExtClsProperties[] = {
        DECLARE_NAPI_FUNCTION("getType", NapiJsMessageExt::GetType),
        DECLARE_NAPI_FUNCTION("getString", NapiJsMessageExt::GetString),
        DECLARE_NAPI_FUNCTION("getNumber", NapiJsMessageExt::GetNumber),
        DECLARE_NAPI_FUNCTION("getBoolean", NapiJsMessageExt::GetBoolean),
        DECLARE_NAPI_FUNCTION("getArrayBuffer", NapiJsMessageExt::GetArrayBuffer),
        DECLARE_NAPI_FUNCTION("getArray", NapiJsMessageExt::GetArray)
    };
    napi_define_class(env, JS_EXT_MSG_CLASS_NAME.c_str(), JS_EXT_MSG_CLASS_NAME.length(),
        NapiJsMessageExt::JsConstructor, nullptr, sizeof(jsMsgExtClsProperties) / sizeof(jsMsgExtClsProperties[0]),
        jsMsgExtClsProperties, &jsMsgExtClass);
    napi_create_reference(env, jsMsgExtClass, 1, &g_jsMsgExtClassRef);
    napi_set_named_property(env, exports, JS_EXT_MSG_CLASS_NAME.c_str(), jsMsgExtClass);
    jsMsgExtClassRef = g_jsMsgExtClassRef;
}

AsyncJavaScriptExtWorkData::AsyncJavaScriptExtWorkData(napi_env napiEnv) : env(napiEnv) {
}

AsyncJavaScriptExtWorkData::~AsyncJavaScriptExtWorkData()
{
    if (!env) {
        return;
    }
    if (callback) {
        napi_delete_reference(env, callback);
        callback = nullptr;
    }
    if (asyncWork) {
        napi_delete_async_work(env, asyncWork);
        asyncWork = nullptr;
    }
}
}