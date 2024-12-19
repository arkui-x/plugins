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

#include "js_web_webview.h"

#include <cstddef>
#include <unistd.h>
#include <string.h>

#include "log.h"
#include "inner_api/plugin_utils_inner.h"
#include "inner_api/plugin_utils_napi.h"
#include "plugin_utils.h"
#include "webview_controller.h"
#include "web_message_port.h"
#ifdef ANDROID_PLATFORM
#include "android/java/jni/web_cookie_manager_android.h"
#include "android/java/jni/web_cookie_manager_jni.h"
#include "android/java/jni/webview_controller_android.h"
#include "android/java/jni/webview_controller_jni.h"
#include "android/java/jni/web_data_base_android.h"
#include "android/java/jni/web_data_base_jni.h"
#include "android/java/jni/web_message_port_android.h"
#endif
#ifdef IOS_PLATFORM
#include "ios/webview_controller_ios.h"
#include "ios/web_data_base_ios.h"
#include "ios/web_message_port_ios.h"
#endif
#include "napi_parse_utils.h"
#include "business_error.h"

using namespace OHOS::NWebError;

namespace OHOS::Plugin {
namespace {
thread_local napi_ref g_historyListRef;
thread_local napi_ref g_classWebMsgPort;
thread_local int32_t g_asyncCallbackInfoId = 0;
constexpr int32_t MAX_COUNT_ID = 1024;

bool GetRawFileUrl(const std::string &fileName, std::string &result)
{
    if (fileName.empty()) {
        return false;
    }
    result = NapiWebviewController::appHapPath_ + "/resources/rawfile/" + fileName;
    LOGE("The parsed url is: %{public}s", result.c_str());
    return true;
}

bool ParseUrl(napi_env env, napi_value urlObj, std::string& result)
{
    napi_valuetype valueType = napi_null;
    napi_typeof(env, urlObj, &valueType);
    if ((valueType != napi_object) && (valueType != napi_string)) {
        LOGE("Unable to parse url object.");
        return false;
    }
    if (valueType == napi_string) {
        NapiParseUtils::ParseString(env, urlObj, result);
        LOGD("The parsed url is: %{public}s", result.c_str());
        return true;
    }
    napi_value type = nullptr;
    napi_valuetype typeVlueType = napi_null;
    napi_get_named_property(env, urlObj, "type", &type);
    napi_typeof(env, type, &typeVlueType);
    if (typeVlueType == napi_number) {
        int32_t typeInteger;
        NapiParseUtils::ParseInt32(env, type, typeInteger);
        if (typeInteger == static_cast<int>(ResourceType::RAWFILE)) {
            napi_value paraArray = nullptr;
            napi_get_named_property(env, urlObj, "params", &paraArray);
            bool isArray = false;
            napi_is_array(env, paraArray, &isArray);
            if (!isArray) {
                LOGE("Unable to parse parameter array from url object.");
                return false;
            }
            napi_value fileNameObj;
            std::string fileName;
            napi_get_element(env, paraArray, 0, &fileNameObj);
            NapiParseUtils::ParseString(env, fileNameObj, fileName);
            return GetRawFileUrl(fileName, result);
        }
        LOGE("The type parsed from url object is not RAWFILE.");
        return false;
    }
    LOGE("Unable to parse type from url object.");
    return false;
}

WebviewController* GetWebviewController(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);

    WebviewController *webviewController = nullptr;
    napi_status status = napi_unwrap(env, thisVar, (void **)&webviewController);
    if ((!webviewController) || (status != napi_ok) || !webviewController->IsInit()) {
        BusinessError::ThrowErrorByErrcode(env, INIT_ERROR);
        return nullptr;
    }
    return webviewController;
}
}

std::string NapiWebviewController::appHapPath_;

napi_value NapiWebviewController::JsConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

#ifdef ANDROID_PLATFORM
    WebviewController *webviewController = new (std::nothrow) WebviewControllerAndroid();
#endif
#ifdef IOS_PLATFORM  
    WebviewController *webviewController = new (std::nothrow) WebviewControllerIOS();
#endif
    if (webviewController == nullptr) {
        LOGE("new webview controller failed");
        return nullptr;
    }
    napi_status status = napi_wrap(
        env, thisVar, webviewController,
        [](napi_env env, void *data, void *hint) {
            WebviewController *webviewController = static_cast<WebviewController *>(data);
            delete webviewController;
        },
        nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("Wrap native webviewController failed.");
        return nullptr;
    }
    return thisVar;
}

napi_value NapiWebviewController::SetWebId(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE];
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    int32_t webId = -1;
    if (!NapiParseUtils::ParseInt32(env, argv[0], webId)) {
        LOGE("Parse web id failed.");
        return nullptr;
    }
    WebviewController *webviewController = nullptr;
    napi_status status = napi_unwrap(env, thisVar, (void **)&webviewController);
    if ((!webviewController) || (status != napi_ok)) {
        LOGE("webviewController is nullptr.");
        return nullptr;
    }
    webviewController->SetWebId(webId);
    return thisVar;
}

napi_value NapiWebviewController::LoadUrl(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_TWO;
    napi_value argv[INTEGER_TWO];
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if ((argc != INTEGER_ONE) && (argc != INTEGER_TWO)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }
    WebviewController *webviewController = nullptr;
    napi_status status = napi_unwrap(env, thisVar, (void **)&webviewController);
    if ((!webviewController) || (status != napi_ok) || !webviewController->IsInit()) {
        BusinessError::ThrowErrorByErrcode(env, INIT_ERROR);
        return nullptr;
    }
    napi_valuetype webSrcType;
    napi_typeof(env, argv[INTEGER_ZERO], &webSrcType);
    if (webSrcType != napi_string && webSrcType != napi_object) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }
    std::string webSrc;
    if (!ParseUrl(env, argv[INTEGER_ZERO], webSrc)) {
        LOGE("ParseUrl failed");
        return nullptr;
    }
    std::string fileProtocolName = "file";
    if (webSrc.substr(INTEGER_ZERO, fileProtocolName.size()) == "file") {
        std::string filePath = webSrc;
        std::string fileProtocol = "file:///";
        filePath.erase(INTEGER_ZERO, fileProtocol.size());
        int isFileExist = access(filePath.c_str(), F_OK);
        if (isFileExist == -1) {
            BusinessError::ThrowErrorByErrcode(env, INVALID_RESOURCE);
        }
    }
    if (argc == INTEGER_ONE) {
        ErrCode ret = webviewController->LoadUrl(webSrc);
        if (ret != NO_ERROR) {
            if (ret == NWEB_ERROR) {
                return nullptr;
            }
            BusinessError::ThrowErrorByErrcode(env, ret);
            return nullptr;
        }
        NAPI_CALL(env, napi_get_undefined(env, &result));
        return result;
    }
    return LoadUrlWithHttpHeaders(env, info, webSrc, argv, webviewController);
}

napi_value NapiWebviewController::LoadUrlWithHttpHeaders(napi_env env, napi_callback_info info, const std::string& url,
    const napi_value* argv, WebviewController* webviewController)
{
    napi_value result = nullptr;
    std::map<std::string, std::string> httpHeaders;
    napi_value array = argv[INTEGER_ONE];
    bool isArray = false;
    napi_is_array(env, array, &isArray);
    if (isArray) {
        uint32_t arrayLength = INTEGER_ZERO;
        napi_get_array_length(env, array, &arrayLength);
        for (uint32_t i = 0; i < arrayLength; ++i) {
            std::string key;
            std::string value;
            napi_value obj = nullptr;
            napi_value keyObj = nullptr;
            napi_value valueObj = nullptr;
            napi_get_element(env, array, i, &obj);
            if (napi_get_named_property(env, obj, "headerKey", &keyObj) != napi_ok) {
                continue;
            }
            if (napi_get_named_property(env, obj, "headerValue", &valueObj) != napi_ok) {
                continue;
            }
            NapiParseUtils::ParseString(env, keyObj, key);
            NapiParseUtils::ParseString(env, valueObj, value);
            httpHeaders[key] = value;
        }
    } else {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }

    ErrCode ret = webviewController->LoadUrl(url, httpHeaders);
    if (ret != NO_ERROR) {
        if (ret == NWEB_ERROR) {
            LOGE("LoadUrl failed.");
            return nullptr;
        }
        BusinessError::ThrowErrorByErrcode(env, ret);
        return nullptr;
    }
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

napi_value NapiWebviewController::InnerSetHapPath(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &result));
    napi_value thisVar = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE];
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != INTEGER_ONE) {
        LOGE("Failed to run InnerSetHapPath beacuse of wrong Param number.");
        return result;
    }
    std::string hapPath;
    if (!NapiParseUtils::ParseString(env, argv[0], hapPath)) {
        LOGE("Parse hap path failed.");
        return result;
    }
    WebviewController *webviewController = nullptr;
    napi_status status = napi_unwrap(env, thisVar, (void **)&webviewController);
    if ((!webviewController) || (status != napi_ok) || !webviewController->IsInit()) {
        LOGE("Wrap webviewController failed. WebviewController must be associated with a Web component.");
        return result;
    }
    appHapPath_ = hapPath;
    return result;
}

napi_value NapiWebviewController::LoadData(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_FIVE;
    napi_value argv[INTEGER_FIVE];
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if ((argc != INTEGER_THREE) && (argc != INTEGER_FOUR) &&
        (argc != INTEGER_FIVE)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }
    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);
    std::string data;
    std::string mimeType;
    std::string encoding;
    std::string baseUrl;
    std::string historyUrl;
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ZERO], data) ||
        !NapiParseUtils::ParseString(env, argv[INTEGER_ONE], mimeType) ||
        !NapiParseUtils::ParseString(env, argv[INTEGER_TWO], encoding)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }
    if ((argc >= INTEGER_FOUR)) {
        if (!ParseUrl(env, argv[INTEGER_THREE], baseUrl)) {
            BusinessError::ThrowErrorByErrcode(env, INVALID_URL);
            return result;
        }
        if ((argc == INTEGER_FIVE) && !baseUrl.empty()) {
            if (!ParseUrl(env, argv[INTEGER_FOUR], historyUrl)) {
                BusinessError::ThrowErrorByErrcode(env, INVALID_URL);
                return result;
            }
        }
    }

    ErrCode ret = webviewController->LoadData(data, mimeType, encoding, baseUrl, historyUrl);
    if (ret != NO_ERROR) {
        if (ret == NWEB_ERROR) {
            return result;
        }
        BusinessError::ThrowErrorByErrcode(env, ret);
        return result;
    }
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

napi_value NapiWebviewController::GetUrl(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);
    std::string url = webviewController->GetUrl();
    napi_create_string_utf8(env, url.c_str(), url.length(), &result);
    return result;
}

napi_value NapiWebviewController::AccessForward(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);
    bool access = webviewController->AccessForward();
    NAPI_CALL(env, napi_get_boolean(env, access, &result));
    return result;
}

napi_value NapiWebviewController::AccessBackward(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);
    bool access = webviewController->AccessBackward();
    NAPI_CALL(env, napi_get_boolean(env, access, &result));
    return result;
}

napi_value NapiWebviewController::Forward(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);
    webviewController->Forward();
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

napi_value NapiWebviewController::Backward(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);
    webviewController->Backward();
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

napi_value NapiWebviewController::Refresh(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);
    webviewController->Refresh();
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

napi_value NapiWebviewController::RunJavaScript(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    size_t argcPromise = INTEGER_ONE;
    size_t argcCallback = INTEGER_TWO;
    napi_value argv[INTEGER_TWO] = { 0 };

    napi_get_undefined(env, &result);
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);

    if (argc != argcPromise && argc != argcCallback) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }
    std::string script;
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ZERO], script)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }
    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);
    std::shared_ptr<AsyncEvaluteJSResultCallbackInfo> asyncCallbackInfoInstance =
        std::make_shared<AsyncEvaluteJSResultCallbackInfo>(env, g_asyncCallbackInfoId);
    auto asyncCallbackInfo = asyncCallbackInfoInstance.get();
    napi_value promise = nullptr;
    if (argc == argcCallback) {
        napi_valuetype valueType = napi_null;
        napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
        napi_typeof(env, argv[argcCallback - 1], &valueType);
        if (valueType != napi_function) {
            BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
            return result;
        }
        NAPI_CALL(env, napi_create_reference(env, argv[argcCallback - 1], 1, &asyncCallbackInfo->callback));
        NAPI_CALL(env, napi_get_undefined(env,  &promise));
    } else if (argc == argcPromise) {
        NAPI_CALL(env, napi_create_promise(env, &asyncCallbackInfo->deferred, &promise));
    }
    napi_value resource = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "RunJavaScript", NAPI_AUTO_LENGTH, &resource));
    NAPI_CALL(env, napi_create_async_work(env, nullptr, resource,
        [](napi_env env, void* data) {},
        [](napi_env env, napi_status status, void* data) {
            AsyncEvaluteJSResultCallbackInfo* asyncCallbackInfo =
                reinterpret_cast<AsyncEvaluteJSResultCallbackInfo*>(data);
            if (!asyncCallbackInfo) {
                return;
            }
            napi_value args[INTEGER_TWO] = { 0 };
            if (asyncCallbackInfo->result.empty()) {
                NAPI_CALL_RETURN_VOID(env, napi_get_null(env, &args[INTEGER_ZERO]));
            }
            napi_value jsResult = nullptr;
            LOGI("asyncCallbackInfo result: %{public}s", asyncCallbackInfo->result.c_str());
            NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(
                env, asyncCallbackInfo->result.c_str(), asyncCallbackInfo->result.length(), &jsResult));
            args[INTEGER_ONE] = jsResult;
            if (asyncCallbackInfo->deferred) {
                if (asyncCallbackInfo->result.empty()) {
                    NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, asyncCallbackInfo->deferred, args[INTEGER_ZERO]));
                } else {
                    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, asyncCallbackInfo->deferred, args[INTEGER_ONE]));
                }
            } else {
                napi_value callback = nullptr;
                napi_value callbackResult = nullptr;
                NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, asyncCallbackInfo->callback, &callback));
                NAPI_CALL_RETURN_VOID(env, napi_call_function(env, nullptr, callback,
                    sizeof(args) / sizeof(args[0]), args, &callbackResult));
            }
            WebviewController::EraseAsyncCallbackInfo(asyncCallbackInfo);
        },
        reinterpret_cast<void*>(asyncCallbackInfo), &asyncCallbackInfo->asyncWork));
    WebviewController::InsertAsyncCallbackInfo(asyncCallbackInfoInstance);
    webviewController->EvaluateJavaScript(script, g_asyncCallbackInfoId);
    if (++g_asyncCallbackInfoId >= MAX_COUNT_ID) {
        g_asyncCallbackInfoId = 0;
    }
    return promise;
}

napi_value NapiWebviewController::AccessStep(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE];
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }

    int32_t step = INTEGER_ZERO;
    if (!NapiParseUtils::ParseInt32(env, argv[INTEGER_ZERO], step)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }

    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);

    bool access = webviewController->AccessStep(step);
    NAPI_CALL(env, napi_get_boolean(env, access, &result));
    return result;

}

napi_value NapiWebviewController::ScrollTo(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_TWO;
    napi_value argv[INTEGER_TWO] = { 0 };
    float x;
    float y;

    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != INTEGER_TWO) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }

    if (!NapiParseUtils::ParseFloat(env, argv[INTEGER_ZERO], x)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }

    if (!NapiParseUtils::ParseFloat(env, argv[INTEGER_ONE], y)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }

    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);

    webviewController->ScrollTo(x, y);
    return result;
}

napi_value NapiWebviewController::ScrollBy(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_TWO;
    napi_value argv[INTEGER_TWO] = { 0 };
    float deltaX;
    float deltaY;

    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != INTEGER_TWO) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }

    if (!NapiParseUtils::ParseFloat(env, argv[INTEGER_ZERO], deltaX)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }

    if (!NapiParseUtils::ParseFloat(env, argv[INTEGER_ONE], deltaY)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }

    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);

    webviewController->ScrollBy(deltaX, deltaY);
    return result;
}

napi_value NapiWebviewController::Zoom(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };

    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }
    float factor = 0.0;
    if (!NapiParseUtils::ParseFloat(env, argv[0], factor)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }

#ifdef ANDROID_PLATFORM
    if (std::to_string(factor) == ZOOM_FACTOR_NAN || factor <= ZOOM_FACTOR_LOW_LIMIT_ANDROID ||
        factor > ZOOM_FACTOR_HIGH_LIMIT_ANDROID) {
        return result;
    }
#endif

    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);

    ErrCode ret = webviewController->Zoom(factor);
    if (ret != NO_ERROR) {
        if (ret == NWEB_ERROR) {
            LOGE("Zoom failed.");
            return result;
        }
        BusinessError::ThrowErrorByErrcode(env, ret);
    }

    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

napi_value NapiWebviewController::Stop(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);
    webviewController->Stop();
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

napi_value NapiWebviewController::SetCustomUserAgent(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE];
    NAPI_CALL(env, napi_get_undefined(env, &result));

    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }

    std::string userAgent;
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ZERO], userAgent)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }

    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);

    ErrCode ret = webviewController->SetCustomUserAgent(userAgent);
    if (ret != NO_ERROR) {
        BusinessError::ThrowErrorByErrcode(env, ret);
    }
    return result;
}

napi_value NapiWebviewController::GetCustomUserAgent(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);
    std::string userAgent = webviewController->GetCustomUserAgent();
    napi_create_string_utf8(env, userAgent.c_str(), userAgent.length(), &result);
    return result;
}

napi_value NapiWebviewController::ClearHistory(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);
    webviewController->ClearHistory();
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

napi_value NapiWebviewController::GetBackForwardEntries(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);

    auto webHistoryListInstance = webviewController->GetBackForwardEntries();
    CHECK_NULL_RETURN(webHistoryListInstance, result);
    WebHistoryList *webHistoryList = new (std::nothrow) WebHistoryList(*webHistoryListInstance);
    CHECK_NULL_RETURN(webHistoryList, result);
    int32_t currentIndex = webHistoryList->GetCurrentIndex();
    int32_t size = webHistoryList->GetListSize();
    napi_value jsHistoryList = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, g_historyListRef, &jsHistoryList));
    NAPI_CALL(env, napi_new_instance(env, jsHistoryList, 0, NULL, &result));

    napi_value js_currentIndex;
    napi_create_int32(env, currentIndex, &js_currentIndex);
    napi_set_named_property(env, result, "currentIndex", js_currentIndex);

    napi_value js_size;
    napi_create_int32(env, size, &js_size);
    napi_set_named_property(env, result, "size", js_size);
    NAPI_CALL(env, napi_wrap(env, result, webHistoryList,
        [](napi_env env, void *data, void *hint) {
            WebHistoryList *webHistoryList = static_cast<WebHistoryList *>(data);
            delete webHistoryList;
            webHistoryList = nullptr;
        },
        nullptr, nullptr));

    return result;
}

napi_value NapiWebHistoryList::JsConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    return thisVar;
}

napi_value NapiWebHistoryList::GetItem(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };
    int32_t index;
    WebHistoryList *historyList = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void **)&historyList));
    if (historyList == nullptr) {
        LOGE("unwrap historyList failed.");
        return result;
    }
    if (argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }
    if (!NapiParseUtils::ParseInt32(env, argv[0], index)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }
    if (index >= historyList->GetListSize() || index < 0) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }
    auto item = historyList->GetItemAtIndex(index);
    CHECK_NULL_RETURN(item, result);
    napi_create_object(env, &result);
    std::string historyUrl = item->historyUrl;
    std::string historyRawUrl = item->historyRawUrl;
    std::string title = item->title;
    napi_value js_historyUrl;
    napi_create_string_utf8(env, historyUrl.c_str(), historyUrl.length(), &js_historyUrl);
    napi_set_named_property(env, result, "historyUrl", js_historyUrl);
    napi_value js_historyRawUrl;
    napi_create_string_utf8(env, historyRawUrl.c_str(), historyRawUrl.length(), &js_historyRawUrl);
    napi_set_named_property(env, result, "historyRawUrl", js_historyRawUrl);
    napi_value js_title;
    napi_create_string_utf8(env, title.c_str(), title.length(), &js_title);
    napi_set_named_property(env, result, "title", js_title);
    return result;
}

napi_value NapiWebviewController::RemoveCache(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };
    bool include_disk_files = false;

    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }

    if (!NapiParseUtils::ParseBoolean(env, argv[0], include_disk_files)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }

    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);
    webviewController->RemoveCache(include_disk_files);
    return result;
}

napi_value NapiWebviewController::BackOrForward(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);

    if (argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }

    int32_t step = 0;
    if (!NapiParseUtils::ParseInt32(env, argv[INTEGER_ZERO], step)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }

    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);
    webviewController->BackOrForward(step);
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

napi_value NapiWebviewController::GetTitle(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);

    std::string title = webviewController->GetTitle();
    napi_create_string_utf8(env, title.c_str(), title.length(), &result);

    return result;
}

napi_value NapiWebviewController::GetPageHeight(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);

    int32_t pageHeight = webviewController->GetPageHeight();
    napi_create_int32(env, pageHeight, &result);

    return result;
}

napi_value NapiWebMessagePort::JsConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    size_t argc = INTEGER_THREE;
    napi_value argv[INTEGER_THREE] = {0};
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    int32_t webId = -1;
    if (!NapiParseUtils::ParseInt32(env, argv[INTEGER_ZERO], webId)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }

    std::string portHandle;
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ONE], portHandle)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }

    bool isExtentionType = false;
    if (!NapiParseUtils::ParseBoolean(env, argv[INTEGER_TWO], isExtentionType)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }

#ifdef ANDROID_PLATFORM
    WebMessagePort *msgPort = new (std::nothrow) WebMessagePortAndroid(webId, portHandle);
#endif
#ifdef IOS_PLATFORM
    WebMessagePort *msgPort = new (std::nothrow) WebMessagePortIOS(webId, portHandle);
#endif
    if (msgPort == nullptr) {
        LOGE("new webMessagePort failed");
        return nullptr;
    }
    WebMessagePort::InsertPort(msgPort);

    NAPI_CALL(env, napi_wrap(env, thisVar, msgPort,
        [](napi_env env, void *data, void *hint) {
            WebMessagePort *msgPort = static_cast<WebMessagePort *>(data);
            WebMessagePort::ErasePort(msgPort);
            delete msgPort;
            msgPort = nullptr;
        },
        nullptr, nullptr));
    return thisVar;
}

napi_value NapiWebMessagePort::Close(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    WebMessagePort *msgPort = nullptr;
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void **)&msgPort));
    if (msgPort == nullptr) {
        LOGE("close message port failed, napi unwrap msg port failed");
        return nullptr;
    }
    msgPort->ClosePort();
    NAPI_CALL(env, napi_get_undefined(env, &result));

    return result;
}

napi_value NapiWebMessagePort::PostMessageEvent(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = 1;
    napi_value argv[INTEGER_ONE];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    if (argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[INTEGER_ZERO], &valueType);
    std::string webMessage;
    if (valueType != napi_string) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ZERO], webMessage)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }

    WebMessagePort *msgPort = nullptr;
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void **)&msgPort));
    if (msgPort == nullptr) {
        LOGE("close message port failed, napi unwrap msg port failed");
        return nullptr;
    }
    auto errorCode = msgPort->PostMessageEvent(webMessage);
    if (errorCode == CAN_NOT_POST_MESSAGE) {
        BusinessError::ThrowErrorByErrcode(env, CAN_NOT_POST_MESSAGE);
        return nullptr;
    }
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

napi_value NapiWebMessagePort::OnMessageEvent(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    if (argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[INTEGER_ZERO], &valueType);
    if (valueType != napi_function) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }

    napi_ref onMsgEventFunc = nullptr;
    NAPI_CALL(env, napi_create_reference(env, argv[INTEGER_ZERO], INTEGER_ONE, &onMsgEventFunc));

    WebMessagePort *msgPort = nullptr;
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void **)&msgPort));
    CHECK_NULL_RETURN(msgPort, nullptr);

    auto webMessageCallbackInfo = std::make_shared<NapiJsCallBackParm>(env);
    auto param = webMessageCallbackInfo.get();
    CHECK_NULL_RETURN(param, nullptr);
    param->callback = onMsgEventFunc;
    msgPort->SetWebMessageCallback(webMessageCallbackInfo);
    ErrCode ret = msgPort->OnMessageEvent();
    if (ret != NO_ERROR) {
        BusinessError::ThrowErrorByErrcode(env, ret);
    }
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

napi_value NapiWebviewController::CreateWebMessagePorts(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };
    bool isExtentionType = false;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    if (argc != INTEGER_ZERO && argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }

    if (argc == INTEGER_ONE) {
        if (!NapiParseUtils::ParseBoolean(env, argv[0], isExtentionType)) {
            BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
            return result;
        }
    }

    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);
    if (webviewController == nullptr || !webviewController->IsInit()) {
        BusinessError::ThrowErrorByErrcode(env, INIT_ERROR);
        LOGE("create message port failed, napi unwrap webviewController failed");
        return nullptr;
    }
    int32_t nwebId = webviewController->GetWebId();
    std::vector<std::string> ports;
    webviewController->CreateWebMessagePorts(ports);
    if (ports.size() != INTEGER_TWO) {
        LOGE("create web message port failed");
        return result;
    }
    napi_value msgPortcons = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, g_classWebMsgPort, &msgPortcons));
    napi_create_array(env, &result);
    napi_value consParam[INTEGER_TWO][INTEGER_THREE] = {{0}};
    for (uint32_t i = 0; i < INTEGER_TWO; i++) {
        napi_value msgPortObj = nullptr;
        NAPI_CALL(env, napi_create_int32(env, nwebId, &consParam[i][INTEGER_ZERO]));
        NAPI_CALL(env, napi_create_string_utf8(env, ports[i].c_str(), ports[i].length(), &consParam[i][INTEGER_ONE]));
        NAPI_CALL(env, napi_get_boolean(env, isExtentionType, &consParam[i][INTEGER_TWO]));
        NAPI_CALL(env, napi_new_instance(env, msgPortcons, INTEGER_THREE, consParam[i], &msgPortObj));

        napi_set_element(env, result, i, msgPortObj);
    }

    return result;
}

bool GetSendPorts(napi_env env, napi_value argv, std::vector<std::string>& sendPorts)
{
    uint32_t arrayLen = 0;
    napi_get_array_length(env, argv, &arrayLen);
    if (arrayLen == 0) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return false;
    }

    napi_valuetype valueType = napi_undefined;
    for (uint32_t i = 0; i < arrayLen; i++) {
        napi_value portItem = nullptr;
        napi_get_element(env, argv, i, &portItem);
        napi_typeof(env, portItem, &valueType);
        if (valueType != napi_object) {
            BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
            return false;
        }
        WebMessagePort *msgPort = nullptr;
        napi_status status = napi_unwrap(env, portItem, (void **)&msgPort);
        if ((!msgPort) || (status != napi_ok)) {
            LOGE("post port to html failed, napi unwrap msg port fail");
            return false;
        }
        std::string portHandle = msgPort->GetPortHandle();
        sendPorts.emplace_back(portHandle);
    }
    return true;
}

napi_value NapiWebviewController::PostMessage(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_THREE;
    napi_value argv[INTEGER_THREE];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    if (argc != INTEGER_THREE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }

    std::string portName;
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ZERO], portName)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }

    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, argv[INTEGER_ONE], &isArray));
    if (!isArray) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }
    std::vector<std::string> sendPorts;
    if (!GetSendPorts(env, argv[INTEGER_ONE], sendPorts)) {
        LOGE("post port to html failed, getSendPorts fail");
        return result;
    }

    std::string urlStr;
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_TWO], urlStr)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }

    WebviewController *webviewController = nullptr;
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void **)&webviewController));
    if (webviewController == nullptr || !webviewController->IsInit()) {
        BusinessError::ThrowErrorByErrcode(env, INIT_ERROR);
        LOGE("post port to html failed, napi unwrap webviewController failed");
        return nullptr;
    }
    webviewController->PostWebMessage(portName, sendPorts, urlStr);
    NAPI_CALL(env, napi_get_undefined(env, &result));

    return result;
}

napi_value NapiWebviewController::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("loadUrl", NapiWebviewController::LoadUrl),
        DECLARE_NAPI_FUNCTION("setWebId", NapiWebviewController::SetWebId),
        DECLARE_NAPI_FUNCTION("innerSetHapPath", NapiWebviewController::InnerSetHapPath),
        DECLARE_NAPI_FUNCTION("loadData", NapiWebviewController::LoadData),
        DECLARE_NAPI_FUNCTION("getUrl", NapiWebviewController::GetUrl),
        DECLARE_NAPI_FUNCTION("accessForward", NapiWebviewController::AccessForward),
        DECLARE_NAPI_FUNCTION("accessBackward", NapiWebviewController::AccessBackward),
        DECLARE_NAPI_FUNCTION("forward", NapiWebviewController::Forward),
        DECLARE_NAPI_FUNCTION("backward", NapiWebviewController::Backward),
        DECLARE_NAPI_FUNCTION("refresh", NapiWebviewController::Refresh),
        DECLARE_NAPI_FUNCTION("runJavaScript", NapiWebviewController::RunJavaScript),
        DECLARE_NAPI_FUNCTION("accessStep", NapiWebviewController::AccessStep),
        DECLARE_NAPI_FUNCTION("scrollTo", NapiWebviewController::ScrollTo),
        DECLARE_NAPI_FUNCTION("scrollBy", NapiWebviewController::ScrollBy),
        DECLARE_NAPI_FUNCTION("zoom", NapiWebviewController::Zoom),
        DECLARE_NAPI_FUNCTION("stop", NapiWebviewController::Stop),
        DECLARE_NAPI_FUNCTION("clearHistory", NapiWebviewController::ClearHistory),
        DECLARE_NAPI_FUNCTION("setCustomUserAgent", NapiWebviewController::SetCustomUserAgent),
        DECLARE_NAPI_FUNCTION("getCustomUserAgent", NapiWebviewController::GetCustomUserAgent),
        DECLARE_NAPI_FUNCTION("getBackForwardEntries", NapiWebviewController::GetBackForwardEntries),
        DECLARE_NAPI_FUNCTION("removeCache", NapiWebviewController::RemoveCache),
        DECLARE_NAPI_FUNCTION("backOrForward", NapiWebviewController::BackOrForward),
        DECLARE_NAPI_FUNCTION("getTitle", NapiWebviewController::GetTitle),
        DECLARE_NAPI_FUNCTION("getPageHeight", NapiWebviewController::GetPageHeight),
        DECLARE_NAPI_FUNCTION("createWebMessagePorts", NapiWebviewController::CreateWebMessagePorts),
        DECLARE_NAPI_FUNCTION("postMessage", NapiWebviewController::PostMessage),
    };
    napi_value constructor = nullptr;
    napi_define_class(env, WEBVIEW_CONTROLLER_CLASS_NAME.c_str(), WEBVIEW_CONTROLLER_CLASS_NAME.length(),
        NapiWebviewController::JsConstructor, nullptr, sizeof(properties) / sizeof(properties[0]),
        properties, &constructor);
    NAPI_ASSERT(env, constructor != nullptr, "define js class WebviewController failed");
    napi_status status = napi_set_named_property(env, exports, "WebviewController", constructor);
    NAPI_ASSERT(env, status == napi_ok, "set property WebviewController failed");

    napi_value historyList = nullptr;
    napi_property_descriptor historyListProperties[] = {
        DECLARE_NAPI_FUNCTION("getItemAtIndex", NapiWebHistoryList::GetItem)
    };
    napi_define_class(env, WEB_HISTORY_LIST_CLASS_NAME.c_str(), WEB_HISTORY_LIST_CLASS_NAME.length(),
        NapiWebHistoryList::JsConstructor, nullptr, sizeof(historyListProperties) / sizeof(historyListProperties[0]),
        historyListProperties, &historyList);
    napi_create_reference(env, historyList, 1, &g_historyListRef);
    napi_set_named_property(env, exports, WEB_HISTORY_LIST_CLASS_NAME.c_str(), historyList);

    napi_value msgPortCons = nullptr;
    napi_property_descriptor msgPortProperties[] = {
        DECLARE_NAPI_FUNCTION("close", NapiWebMessagePort::Close),
        DECLARE_NAPI_FUNCTION("postMessageEvent", NapiWebMessagePort::PostMessageEvent),
        DECLARE_NAPI_FUNCTION("onMessageEvent", NapiWebMessagePort::OnMessageEvent),
    };
    NAPI_CALL(env, napi_define_class(env, WEB_MESSAGE_PORT_CLASS_NAME.c_str(), WEB_MESSAGE_PORT_CLASS_NAME.length(),
        NapiWebMessagePort::JsConstructor, nullptr, sizeof(msgPortProperties) / sizeof(msgPortProperties[0]),
        msgPortProperties, &msgPortCons));
    NAPI_CALL(env, napi_create_reference(env, msgPortCons, 1, &g_classWebMsgPort));
    NAPI_CALL(env, napi_set_named_property(env, exports, WEB_MESSAGE_PORT_CLASS_NAME.c_str(), msgPortCons));

    return exports;
}

napi_value NapiWebDataBase::Init(napi_env env, napi_value exports)
{
    const std::string WEB_DATA_BASE_CLASS_NAME = "WebDataBase";
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_STATIC_FUNCTION("deleteHttpAuthCredentials", NapiWebDataBase::DeleteHttpAuthCredentials),
        DECLARE_NAPI_STATIC_FUNCTION("saveHttpAuthCredentials", NapiWebDataBase::SaveHttpAuthCredentials),
        DECLARE_NAPI_STATIC_FUNCTION("getHttpAuthCredentials", NapiWebDataBase::GetHttpAuthCredentials),
        DECLARE_NAPI_STATIC_FUNCTION("existHttpAuthCredentials", NapiWebDataBase::ExistHttpAuthCredentials),
    };
    napi_value constructor = nullptr;
    napi_define_class(env, WEB_DATA_BASE_CLASS_NAME.c_str(), WEB_DATA_BASE_CLASS_NAME.length(), JsConstructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);
    NAPI_ASSERT(env, constructor != nullptr, "define js class WebDataBase failed");
    napi_status status = napi_set_named_property(env, exports, "WebDataBase", constructor);
    NAPI_ASSERT(env, status == napi_ok, "set property WebDataBase failed");
    return exports;
}

napi_value NapiWebDataBase::JsConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    return thisVar;
}

napi_value NapiWebDataBase::ExistHttpAuthCredentials(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;

#ifdef ANDROID_PLATFORM
    bool isExist = WebDataBaseAndroid::ExistHttpAuthCredentials();
#endif
#ifdef IOS_PLATFORM
    bool isExist = WebDataBaseIOS::ExistHttpAuthCredentials();
#endif
    NAPI_CALL(env, napi_get_boolean(env, isExist, &result));
    return result;
}

napi_value NapiWebDataBase::DeleteHttpAuthCredentials(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;

#ifdef ANDROID_PLATFORM
    WebDataBaseAndroid::DeleteHttpAuthCredentials();
#endif
#ifdef IOS_PLATFORM
    WebDataBaseIOS::DeleteHttpAuthCredentials();
#endif
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiWebDataBase::SaveHttpAuthCredentials(napi_env env, napi_callback_info info)
{
    napi_value retValue = nullptr;
    size_t argc = 4;
    napi_value argv[4] = { 0 };

    napi_get_cb_info(env, info, &argc, argv, &retValue, nullptr);
    if (argc != INTEGER_FOUR) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }

    std::string host;
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ZERO], host)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }

    std::string realm;
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ONE], realm)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }

    std::string username;
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_TWO], username)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }

    if (host.empty() || username.empty()) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }

    size_t bufferSize = 0;
    if (!NapiParseUtils::ParseSize(env, argv[INTEGER_THREE], bufferSize) || bufferSize > MAX_PWD_LENGTH) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }
    if (bufferSize > 0) {
        char password[bufferSize + 1];
        if (!NapiParseUtils::ParseChar(env, argv[INTEGER_THREE], password, bufferSize)) {
            BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
            return nullptr;
        }

#ifdef ANDROID_PLATFORM
    WebDataBaseAndroid::SaveHttpAuthCredentials(host, realm, username, password);
#endif
#ifdef IOS_PLATFORM
    WebDataBaseIOS::SaveHttpAuthCredentials(host, realm, username, password);
#endif
        (void)memset(password, 0, sizeof(password));
    }
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiWebDataBase::GetHttpAuthCredentials(napi_env env, napi_callback_info info)
{
    napi_value retValue = nullptr;
    size_t argc = 2;
    napi_value argv[2] = { 0 };

    napi_get_cb_info(env, info, &argc, argv, &retValue, nullptr);
    if (argc != INTEGER_TWO) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }

    std::string host;
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ZERO], host)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }

    std::string realm;
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ONE], realm)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }

    if (host.empty()) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }

    std::string username;
    char password[MAX_PWD_LENGTH + 1] = {0};
    napi_value result = nullptr;
    napi_create_array(env, &result);

#ifdef ANDROID_PLATFORM
    WebDataBaseAndroid::GetHttpAuthCredentials(host, realm, username, password, MAX_PWD_LENGTH + 1);
#endif
#ifdef IOS_PLATFORM
    WebDataBaseIOS::GetHttpAuthCredentials(host, realm, username, password, MAX_PWD_LENGTH + 1);
#endif
    if (!username.empty() && strlen(password) > 0) {
        napi_value nameVal = nullptr;
        napi_value pwdVal = nullptr;
        napi_create_string_utf8(env, username.c_str(), username.length(), &nameVal);
        napi_set_element(env, result, INTEGER_ZERO, nameVal);
        napi_create_string_utf8(env, password, strlen(password), &pwdVal);
        napi_set_element(env, result, INTEGER_ONE, pwdVal);
    }
    (void)memset(password, 0, MAX_PWD_LENGTH + 1);
    return result;
}

static napi_value WebWebviewExport(napi_env env, napi_value exports)
{
    NapiWebviewController::Init(env, exports);
    NapiWebDataBase::Init(env, exports);
    NapiWebCookieManager::Init(env, exports);
    return exports;
}

static napi_module webWebviewModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = WebWebviewExport,
    .nm_modname = "web.webview",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

#ifdef ANDROID_PLATFORM
static void WebWebviewJniRegister()
{
    const char className[] = "ohos.ace.adapter.capability.web.AceWebBase";
    ARKUI_X_Plugin_RegisterJavaPlugin(&WebviewControllerJni::Register, className);
    const char dataBaseClassName[] = "ohos.ace.adapter.capability.web.AceWebBase";
    ARKUI_X_Plugin_RegisterJavaPlugin(&WebDataBaseJni::Register, dataBaseClassName);
    const char webCookieClassName[] = "ohos.ace.plugin.webviewplugin.webcookie.WebCookiePlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&WebCookieManagerJni::Register, webCookieClassName);
}
#endif

extern "C" __attribute__((constructor)) void WebWebviewRegister()
{
#ifdef ANDROID_PLATFORM
    WebWebviewJniRegister();
#endif
    napi_module_register(&webWebviewModule);
}

napi_value NapiWebCookieManager::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_STATIC_FUNCTION("fetchCookie", NapiWebCookieManager::JsFetchCookieAsync),
        DECLARE_NAPI_STATIC_FUNCTION("configCookie", NapiWebCookieManager::JsConfigCookieAsync),
        DECLARE_NAPI_STATIC_FUNCTION("clearAllCookies", NapiWebCookieManager::JsClearAllCookiesAsync),
        DECLARE_NAPI_STATIC_FUNCTION("existCookie", NapiWebCookieManager::JsExistCookie),
        DECLARE_NAPI_STATIC_FUNCTION("clearSessionCookie", NapiWebCookieManager::JsClearSessionCookie),
    };
    napi_value constructor = nullptr;

    napi_define_class(env, WEB_COOKIE_MANAGER_CLASS_NAME.c_str(), WEB_COOKIE_MANAGER_CLASS_NAME.length(),
        NapiWebCookieManager::JsConstructor, nullptr, sizeof(properties) / sizeof(properties[0]),
        properties, &constructor);
    NAPI_ASSERT(env, constructor != nullptr, "NapiWebCookieManager define js class failed");
    napi_status status = napi_set_named_property(env, exports, "WebCookieManager", constructor);
    NAPI_ASSERT(env, status == napi_ok, "NapiWebCookieManager set property failed");
    return exports;
}

napi_value NapiWebCookieManager::JsConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    return thisVar;
}

napi_value NapiWebCookieManager::JsConfigCookieAsync(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_THREE;
    size_t argcPromise = INTEGER_TWO;
    size_t argcCallback = INTEGER_THREE;
    napi_value argv[INTEGER_THREE] = { 0 };

    napi_get_undefined(env, &result);
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);

    if (argc != argcPromise && argc != argcCallback) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }
    std::string url;
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ZERO], url)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }
    std::string value;
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ONE], value)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }
    std::shared_ptr<AsyncCookieManagerResultCallbackInfo> asyncCallbackInfoInstance =
        std::make_shared<AsyncCookieManagerResultCallbackInfo>(env, g_asyncCallbackInfoId);
    auto asyncCallbackInfo = asyncCallbackInfoInstance.get();
    asyncCallbackInfo->taskType = TaskType::CONFIG_COOKIE;
    napi_value promise = nullptr;
    if (argc == argcCallback) {
        napi_valuetype valueType = napi_null;
        napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
        napi_typeof(env, argv[argcCallback - 1], &valueType);
        if (valueType != napi_function) {
            BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
            return result;
        }
        NAPI_CALL(env, napi_create_reference(env, argv[argcCallback - 1], 1, &asyncCallbackInfo->callback));
        NAPI_CALL(env, napi_get_undefined(env,  &promise));
    } else if (argc == argcPromise) {
        NAPI_CALL(env, napi_create_promise(env, &asyncCallbackInfo->deferred, &promise));
    }
    CreateCookieAsyncWork(env, "JsConfigCookieAsync", asyncCallbackInfoInstance);
    WebCookieManager::ConfigCookie(url, value, g_asyncCallbackInfoId);
    if (++g_asyncCallbackInfoId >= MAX_COUNT_ID) {
        g_asyncCallbackInfoId = 0;
    }
    return promise;
}

napi_value NapiWebCookieManager::JsFetchCookieAsync(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_TWO;
    size_t argcPromise = INTEGER_ONE;
    size_t argcCallback = INTEGER_TWO;
    napi_value argv[INTEGER_TWO] = { 0 };

    napi_get_undefined(env, &result);
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);

    if (argc != argcPromise && argc != argcCallback) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }
    std::string url;
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ZERO], url)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }
    std::shared_ptr<AsyncCookieManagerResultCallbackInfo> asyncCallbackInfoInstance =
        std::make_shared<AsyncCookieManagerResultCallbackInfo>(env, g_asyncCallbackInfoId);
    auto asyncCallbackInfo = asyncCallbackInfoInstance.get();
    napi_value promise = nullptr;
    if (argc == argcCallback) {
        napi_valuetype valueType = napi_null;
        napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
        napi_typeof(env, argv[argcCallback - 1], &valueType);
        if (valueType != napi_function) {
            BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
            return result;
        }
        NAPI_CALL(env, napi_create_reference(env, argv[argcCallback - 1], 1, &asyncCallbackInfo->callback));
        NAPI_CALL(env, napi_get_undefined(env, &promise));
    } else if (argc == argcPromise) {
        NAPI_CALL(env, napi_create_promise(env, &asyncCallbackInfo->deferred, &promise));
    }
    CreateFetchCookieAsyncWork(env, asyncCallbackInfoInstance);
    WebCookieManager::FetchCookie(url, g_asyncCallbackInfoId);
    if (++g_asyncCallbackInfoId >= MAX_COUNT_ID) {
        g_asyncCallbackInfoId = 0;
    }
    return promise;
}

napi_value NapiWebCookieManager::JsClearAllCookiesAsync(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    size_t argcPromise = INTEGER_ZERO;
    size_t argcCallback = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };

    napi_get_undefined(env, &result);
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);

    if (argc != argcPromise && argc != argcCallback) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }
    std::shared_ptr<AsyncCookieManagerResultCallbackInfo> asyncCallbackInfoInstance =
        std::make_shared<AsyncCookieManagerResultCallbackInfo>(env, g_asyncCallbackInfoId);
    auto asyncCallbackInfo = asyncCallbackInfoInstance.get();
    asyncCallbackInfo->taskType = TaskType::CLEAR_ALL_COOKIES;
    napi_value promise = nullptr;
    if (argc == argcCallback) {
        napi_valuetype valueType = napi_null;
        napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
        napi_typeof(env, argv[argcCallback - 1], &valueType);
        if (valueType != napi_function) {
            BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
            return result;
        }
        NAPI_CALL(env, napi_create_reference(env, argv[argcCallback - 1], 1, &asyncCallbackInfo->callback));
        NAPI_CALL(env, napi_get_undefined(env,  &promise));
    } else if (argc == argcPromise) {
        NAPI_CALL(env, napi_create_promise(env, &asyncCallbackInfo->deferred, &promise));
    }
    CreateCookieAsyncWork(env, "JsClearAllCookiesAsync", asyncCallbackInfoInstance);
    WebCookieManager::ClearAllCookies(g_asyncCallbackInfoId);
    if (++g_asyncCallbackInfoId >= MAX_COUNT_ID) {
        g_asyncCallbackInfoId = 0;
    }
    return promise;
}

napi_value NapiWebCookieManager::JsClearSessionCookie(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    size_t argcPromise = INTEGER_ZERO;
    size_t argcCallback = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };
    napi_get_undefined(env, &result);
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != argcPromise && argc != argcCallback) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }
    std::shared_ptr<AsyncCookieManagerResultCallbackInfo> asyncCallbackInfoInstance =
        std::make_shared<AsyncCookieManagerResultCallbackInfo>(env, g_asyncCallbackInfoId);
    auto asyncCallbackInfo = asyncCallbackInfoInstance.get();
    asyncCallbackInfo->taskType = TaskType::CLEAR_ALL_COOKIES;
    napi_value promise = nullptr;
    if (argc == argcCallback) {
        napi_valuetype valueType = napi_null;
        napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
        napi_typeof(env, argv[argcCallback - 1], &valueType);
        if (valueType != napi_function) {
            BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
            return result;
        }
        NAPI_CALL(env, napi_create_reference(env, argv[argcCallback - 1], 1, &asyncCallbackInfo->callback));
        NAPI_CALL(env, napi_get_undefined(env,  &promise));
    } else if (argc == argcPromise) {
        NAPI_CALL(env, napi_create_promise(env, &asyncCallbackInfo->deferred, &promise));
    }
    CreateCookieAsyncWork(env, "JsClearSessionCookie", asyncCallbackInfoInstance);
    WebCookieManager::ClearSessionCookie(g_asyncCallbackInfoId);
    if (++g_asyncCallbackInfoId >= MAX_COUNT_ID) {
        g_asyncCallbackInfoId = 0;
    }
    return promise;
}

napi_value NapiWebCookieManager::JsExistCookie(napi_env env, napi_callback_info info)
{
	napi_value thisVar = nullptr;
	napi_value result = nullptr;
	size_t argc = INTEGER_ONE; 
	napi_value argv[INTEGER_ONE] = { 0 };
	bool incognito = false; 
	napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
	if (argc > INTEGER_ZERO) {
	    if (!NapiParseUtils::ParseBoolean(env, argv[0], incognito)) {
	        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
	        return nullptr; 
	    }
	}
	bool exist = WebCookieManager::ExistCookie(incognito);
	NAPI_CALL(env, napi_get_boolean(env, exist, &result));
	return result;
}

void NapiWebCookieManager::CreateCookieAsyncWork(napi_env env, const std::string& taskName,
    const std::shared_ptr<AsyncCookieManagerResultCallbackInfo>& callbackInfo)
{
    napi_value resource = nullptr;
    auto asyncCallbackInfo = callbackInfo.get();
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, taskName.c_str(), NAPI_AUTO_LENGTH, &resource));
    NAPI_CALL_RETURN_VOID(env, napi_create_async_work(env, nullptr, resource,
        [](napi_env env, void* data) {},
        [](napi_env env, napi_status status, void* data) {
            AsyncCookieManagerResultCallbackInfo* asyncCallbackInfo =
                reinterpret_cast<AsyncCookieManagerResultCallbackInfo*>(data);
            if (!asyncCallbackInfo) {
                return;
            }
            if (asyncCallbackInfo->deferred) {
                napi_value jsResult = nullptr;
                napi_get_undefined(env, &jsResult);
                napi_resolve_deferred(env, asyncCallbackInfo->deferred, jsResult);
            } else {
                napi_value result[INTEGER_ONE] = { 0 };
                napi_get_null(env, &result[INTEGER_ZERO]);

                napi_value onCookieFunc = nullptr;
                napi_get_reference_value(env, asyncCallbackInfo->callback, &onCookieFunc);
                napi_value callbackResult = nullptr;
                napi_call_function(env, nullptr, onCookieFunc, INTEGER_ONE, &result[INTEGER_ZERO], &callbackResult);
            }
            WebCookieManager::EraseCallbackInfo(asyncCallbackInfo, asyncCallbackInfo->taskType);
        },
        reinterpret_cast<void*>(asyncCallbackInfo), &asyncCallbackInfo->asyncWork));
    WebCookieManager::InsertCallbackInfo(callbackInfo, callbackInfo->taskType);
}

void NapiWebCookieManager::CreateFetchCookieAsyncWork(
    napi_env env, const std::shared_ptr<AsyncCookieManagerResultCallbackInfo>& callbackInfo)
{
    napi_value resource = nullptr;
    auto asyncCallbackInfo = callbackInfo.get();
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "JsFetchCookieAsync", NAPI_AUTO_LENGTH, &resource));
    NAPI_CALL_RETURN_VOID(env, napi_create_async_work(env, nullptr, resource,
        [](napi_env env, void* data) {},
        [](napi_env env, napi_status status, void* data) {
            AsyncCookieManagerResultCallbackInfo* asyncCallbackInfo =
                reinterpret_cast<AsyncCookieManagerResultCallbackInfo*>(data);
            if (!asyncCallbackInfo) {
                return;
            }
            napi_value args[INTEGER_TWO] = { 0 };
            if (asyncCallbackInfo->result.empty()) {
                NAPI_CALL_RETURN_VOID(env, napi_get_null(env, &args[INTEGER_ZERO]));
            }
            napi_value jsCookies = nullptr;
            LOGD("asyncCallbackInfo result: %{public}s", asyncCallbackInfo->result.c_str());
            NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(
                env, asyncCallbackInfo->result.c_str(), asyncCallbackInfo->result.length(), &jsCookies));
            args[INTEGER_ONE] = jsCookies;
            if (asyncCallbackInfo->deferred) {
                NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, asyncCallbackInfo->deferred, args[INTEGER_ONE]));
            } else {
                napi_value callback = nullptr;
                napi_value callbackResult = nullptr;
                NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, asyncCallbackInfo->callback, &callback));
                NAPI_CALL_RETURN_VOID(env, napi_call_function(env, nullptr, callback,
                    sizeof(args) / sizeof(args[0]), args, &callbackResult));
            }
            WebCookieManager::EraseCallbackInfo(asyncCallbackInfo, TaskType::FETCH_COOKIE);
        },
        reinterpret_cast<void*>(asyncCallbackInfo), &asyncCallbackInfo->asyncWork));
    WebCookieManager::InsertCallbackInfo(callbackInfo, TaskType::FETCH_COOKIE);
}
} // namespace OHOS::Plugin
