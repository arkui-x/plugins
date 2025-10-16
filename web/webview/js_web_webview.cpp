/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <securec.h>

#include "business_error.h"
#include "inner_api/plugin_utils_inner.h"
#include "inner_api/plugin_utils_napi.h"
#include "log.h"
#include "napi_parse_utils.h"
#include "napi_web_message_ext.h"
#include "napi_web_scheme_handler_request.h"
#include "plugin_utils.h"
#include "web_download_delegate.h"
#include "web_download_item.h"
#include "web_download_manager.h"
#include "web_message_port.h"
#include "webview_controller.h"
#include "webview_javascript_async_work_callback.h"
#include "web_scheme_handler_request.h"
#ifdef ANDROID_PLATFORM
#include "android/java/jni/android_asset_helper_jni.h"
#include "android/java/jni/web_cookie_manager_android.h"
#include "android/java/jni/web_cookie_manager_jni.h"
#include "android/java/jni/web_data_base_android.h"
#include "android/java/jni/web_data_base_jni.h"
#include "android/java/jni/web_download_delegate_jni.h"
#include "android/java/jni/web_download_item_jni.h"
#include "android/java/jni/web_message_port_android.h"
#include "android/java/jni/geolocation_permissions_android.h"
#include "android/java/jni/geolocation_permissions_jni.h"
#include "android/java/jni/web_storage_android.h"
#include "android/java/jni/web_storage_jni.h"
#include "android/java/jni/webview_controller_android.h"
#include "android/java/jni/webview_controller_jni.h"
#include "android/java/jni/web_scheme_handler_jni.h"
#endif
#ifdef IOS_PLATFORM
#include "ios/webview_controller_ios.h"
#include "ios/web_data_base_ios.h"
#include "ios/web_message_port_ios.h"
#endif

using namespace OHOS::NWebError;

namespace OHOS::Plugin {
namespace {
thread_local napi_ref g_historyListRef;
thread_local napi_ref g_classWebMsgPort;
thread_local napi_ref g_jsMsgExtClassRef;
thread_local int32_t g_asyncCallbackInfoId = 0;
constexpr int32_t MAX_COUNT_ID = 1024;
const std::string ANDROID_ASSET = "file:///android_asset/";
const std::string ARKUIX_PATH_MARKER = "files/arkui-x/";
const std::string HTTP = "http://";
const std::string HTTPS = "https://";
const std::string RESOURCE = "resource://rawfile/";
const std::string FILE = "file";
constexpr int32_t MAX_WEB_STRING_LENGTH = 40960;
constexpr uint32_t URL_MAXIMUM = 2048;
constexpr char URL_REGEXPR[] = "^http(s)?:\\/\\/.+";

const char* rawfile = "resource://rawfile/";
const char* replacement = "file:///";
std::regex reg("^(https?|file)://[\\w.-/]+(:\\d+)?(/.*)?$");

bool ExistsWebFileUrl(const std::string &webFileUrl)
{
#ifdef ANDROID_PLATFORM
    if (webFileUrl.substr(INTEGER_ZERO, ANDROID_ASSET.size()) == ANDROID_ASSET) {
        return AndroidAssetHelperJni::ExistsVirtualAsset(webFileUrl);
    }
#endif

    std::string fileProtocolName = "file";
    if (webFileUrl.substr(INTEGER_ZERO, fileProtocolName.size()) == "file") {
        std::string filePath = webFileUrl;
        std::string fileProtocol = "file:///";
        filePath.erase(INTEGER_ZERO, fileProtocol.size());
        return access(filePath.c_str(), F_OK) != -1;
    }

    return true;
}

bool GetRawFileUrl(const std::string &fileName, std::string &result)
{
    if (fileName.empty()) {
        return false;
    }
    result = NapiWebviewController::appHapPath_ + "/resources/rawfile/" + fileName;

#ifdef ANDROID_PLATFORM
    size_t pos = NapiWebviewController::appHapPath_.find(ARKUIX_PATH_MARKER);
    if (pos != std::string::npos) {
        std::string assetPath = ANDROID_ASSET +
            NapiWebviewController::appHapPath_.substr(pos + std::strlen("files/")) + "/resources/rawfile/" + fileName;
        if (AndroidAssetHelperJni::ExistsVirtualAsset(assetPath)) {
            result = assetPath;
        }
    }
#endif

    LOGD("The parsed url is: %{public}s", result.c_str());
    return true;
}

bool ParsePrepareUrl(napi_env env, napi_value urlObj, std::string& url)
{
    napi_valuetype valueType = napi_null;
    napi_typeof(env, urlObj, &valueType);

    if (valueType == napi_string) {
        NapiParseUtils::ParseString(env, urlObj, url);
        if (url.size() > URL_MAXIMUM) {
            LOGE("The URL exceeds the maximum length of %{public}d", URL_MAXIMUM);
            return false;
        }

        if (!regex_match(url, std::regex(URL_REGEXPR, std::regex_constants::icase))) {
            LOGE("ParsePrepareUrl error");
            return false;
        }

        return true;
    }

    LOGE("Unable to parse type from url object.");
    return false;
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

bool ParseRegisterJavaScriptProxyParam(napi_env env, size_t argc, napi_value* argv,
    RegisterJavaScriptProxyParam* param)
{
    std::string objName;
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ONE], objName)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "name", "string"));
        return false;
    }
    std::vector<std::string> methodList;
    if (!NapiParseUtils::ParseStringArray(env, argv[INTEGER_TWO], methodList)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "methodList", "array"));
        return false;
    }
    std::vector<std::string> asyncMethodList;
    if (argc >= INTEGER_FOUR && !NapiParseUtils::ParseStringArray(env, argv[INTEGER_THREE], asyncMethodList)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return false;
    }
    std::string permission;
    if (argc == INTEGER_FIVE) {
        if (!NapiParseUtils::ParseString(env, argv[INTEGER_FOUR], permission)) {
            permission = "";
        }
    }
    param->env = env;
    napi_status status = napi_create_reference(env, argv[INTEGER_ZERO], INTEGER_ONE, &param->objRef);
    if (status != napi_ok) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return false;
    }
    param->objName = objName;
    param->syncMethodList = methodList;
    param->asyncMethodList = asyncMethodList;
    param->permission = permission;
    return true;
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

napi_value RemoveDownloadDelegateRef(napi_env env, napi_value thisVar)
{
    WebviewController* webviewController = nullptr;
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void **)&webviewController));
    if (webviewController == nullptr || !webviewController->IsInit()) {
        LOGE("create message port failed, napi unwrap webviewController failed");
        return nullptr;
    }

    WebDownloadManager::RemoveDownloadDelegateRef(webviewController->GetWebId());
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
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
    if (!ExistsWebFileUrl(webSrc)) {
        BusinessError::ThrowErrorByErrcode(env, INVALID_RESOURCE);
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

napi_value NapiWebviewController::PostUrl(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_TWO;
    napi_value argv[INTEGER_TWO] = { 0 };
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != INTEGER_TWO) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_ONE, "two"));
        return result;
    }
    WebviewController *webviewController = nullptr;
    napi_status status = napi_unwrap(env, thisVar, (void **)&webviewController);
    if ((!webviewController) || (status != napi_ok) || !webviewController->IsInit()) {
        BusinessError::ThrowErrorByErrcode(env, INIT_ERROR);
        return nullptr;
    }
    std::string url;
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ZERO], url)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "url", "string"));
        return result;
    }
    if (url.empty()) {
    BusinessError::ThrowErrorByErrcode(env, INVALID_URL);
    return result;
    }
    bool isArrayBuffer = false;
    NAPI_CALL(env, napi_is_arraybuffer(env, argv[INTEGER_ONE], &isArrayBuffer));
    if (!isArrayBuffer) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "postData", "array"));
        return result;
    }
    if ((url.substr(INTEGER_ZERO, HTTP.size()) == HTTP || url.substr(INTEGER_ZERO, HTTPS.size()) == HTTPS)) {        
        char *arrBuf = nullptr;
        size_t byteLength = 0;
        napi_get_arraybuffer_info(env, argv[INTEGER_ONE], (void **)&arrBuf, &byteLength);
        std::vector<uint8_t> postData(arrBuf, arrBuf + byteLength);
        ErrCode ret = webviewController->PostUrl(url, postData);
        if (ret != NO_ERROR) {
            if (ret == NWEB_ERROR) {
                LOGE("PostData failed");
                return result;
            }
            BusinessError::ThrowErrorByErrcode(env, ret);
            return result;
        }
        NAPI_CALL(env, napi_get_undefined(env, &result));
        return result;
    }
    if (url.substr(INTEGER_ZERO, FILE.size()) == FILE) {
        std::string filePath = url;
        std::string fileProtocol = FILE + ":///";
        filePath.erase(INTEGER_ZERO, fileProtocol.size());
        int isFileExist = access(filePath.c_str(), F_OK);
        if (isFileExist == -1) {
            BusinessError::ThrowErrorByErrcode(env, INVALID_RESOURCE);
        }
    }
    if (url.substr(INTEGER_ZERO, RESOURCE.size()) == RESOURCE)
    {
       std::string resourcePath = url;
       resourcePath.erase(INTEGER_ZERO, RESOURCE.size());
       GetRawFileUrl(resourcePath,url);
    }    
    ErrCode ret = webviewController->LoadUrl(url);
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

napi_value NapiWebviewController::PageDown(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };
    bool bottom = false;

    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_ONE, "one"));
        return result;
    }

    if (!NapiParseUtils::ParseBoolean(env, argv[0], bottom)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "bottom", "boolean"));
        return result;
    }

    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);
    ErrCode ret = webviewController->PageDown(bottom);
    if (ret != NO_ERROR) {
        if (ret == INIT_ERROR) {
            LOGE("PageDown failed.");
            return result;
        }
        BusinessError::ThrowErrorByErrcode(env, ret);
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

napi_value NapiWebviewController::RunJavaScriptExt(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    size_t argcPromise = INTEGER_ONE;
    size_t argcCallback = INTEGER_TWO;
    napi_value argv[INTEGER_TWO] = { 0 };

    NAPI_CALL(env, napi_get_undefined(env, &result));
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));

    if (argc != argcPromise && argc != argcCallback) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }
    std::string script;
    napi_valuetype scriptType = napi_undefined;
    napi_typeof(env, argv[INTEGER_ZERO], &scriptType);
    if (scriptType == napi_undefined || scriptType == napi_null) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ZERO], script)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }

    WebviewController* webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);
    AsyncJavaScriptExtEvaluteJSResultCallbackInfo* asyncCallbackInfo =
        new AsyncJavaScriptExtEvaluteJSResultCallbackInfo(env, g_asyncCallbackInfoId);
    CHECK_NULL_RETURN(asyncCallbackInfo, result);
    napi_value promise = nullptr;
    if (argc == argcCallback) {
        napi_valuetype valueType = napi_null;
        napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
        napi_typeof(env, argv[argcCallback - 1], &valueType);
        if (valueType != napi_function) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
            BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
            return result;
        }
        NAPI_CALL(env, napi_create_reference(env, argv[argcCallback - 1], 1, &asyncCallbackInfo->callback));
        NAPI_CALL(env, napi_get_undefined(env, &promise));
    } else if (argc == argcPromise) {
        NAPI_CALL(env, napi_create_promise(env, &asyncCallbackInfo->deferred, &promise));
    }
    napi_value resource = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "RunJavaScriptExt", NAPI_AUTO_LENGTH, &resource));
    NAPI_CALL(env,
        napi_create_async_work(
            env, nullptr, resource, [](napi_env env, void* data) {},
            [](napi_env env, napi_status status, void* data) {
                AsyncJavaScriptExtEvaluteJSResultCallbackInfo* asyncCallbackInfo =
                    reinterpret_cast<AsyncJavaScriptExtEvaluteJSResultCallbackInfo*>(data);
                if (!asyncCallbackInfo) {
                    return;
                }

                napi_value setResult[INTEGER_TWO] = { 0 };
                if (asyncCallbackInfo->result_->GetType() == WebValue::Type::STRING &&
                    asyncCallbackInfo->result_->GetString().empty()) {
                    setResult[INTEGER_ZERO] = BusinessError::CreateError(env, NWebError::INVALID_RESOURCE);
                    napi_get_null(env, &setResult[INTEGER_ONE]);
                } else {
                    napi_get_undefined(env, &setResult[INTEGER_ZERO]);
                    napi_value jsMsgExt = nullptr;
                    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, g_jsMsgExtClassRef, &jsMsgExt));
                    NAPI_CALL_RETURN_VOID(env, napi_new_instance(env, jsMsgExt, 0, NULL, &setResult[INTEGER_ONE]));

                    WebJsMessageExt* webJsMessageExt = new (std::nothrow) WebJsMessageExt(asyncCallbackInfo->result_);
                    if (webJsMessageExt == nullptr) {
                        LOGE("new WebJsMessageExt failed.");
                        return;
                    }

                    napi_status status = napi_wrap(
                        env, setResult[INTEGER_ONE], webJsMessageExt,
                        [](napi_env env, void* data, void* hint) {
                            WebJsMessageExt* webJsMessageExt = static_cast<WebJsMessageExt*>(data);
                            delete webJsMessageExt;
                            webJsMessageExt = nullptr;
                        },
                        nullptr, nullptr);
                    if (status != napi_status::napi_ok) {
                        LOGE("napi_wrap failed");
                        return;
                    }
                }
                napi_value args[INTEGER_TWO] = { setResult[INTEGER_ZERO], setResult[INTEGER_ONE] };
                if (asyncCallbackInfo->deferred) {
                    if (!asyncCallbackInfo->result_) {
                        NAPI_CALL_RETURN_VOID(
                            env, napi_reject_deferred(env, asyncCallbackInfo->deferred, args[INTEGER_ZERO]));
                    } else {
                        NAPI_CALL_RETURN_VOID(
                            env, napi_resolve_deferred(env, asyncCallbackInfo->deferred, args[INTEGER_ONE]));
                    }
                } else {
                    napi_value callback = nullptr;
                    napi_value callbackResult = nullptr;
                    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, asyncCallbackInfo->callback, &callback));
                    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, nullptr, callback,
                                                   sizeof(args) / sizeof(args[0]), args, &callbackResult));
                }
                WebviewController::EraseAsyncCallbackJavaScriptExtInfo(asyncCallbackInfo);
            },
            reinterpret_cast<void*>(asyncCallbackInfo), &asyncCallbackInfo->asyncWork));
    WebviewController::InsertAsyncCallbackJavaScriptExtInfo(asyncCallbackInfo);
    webviewController->EvaluateJavaScriptExt(script, g_asyncCallbackInfoId);
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

napi_value NapiWebviewController::ZoomIn(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);

    ErrCode ret = webviewController->ZoomIn();
    if (ret != NO_ERROR) {
        if (ret == NWEB_ERROR) {
            LOGE("ZoomIn failed.");
            return result;
        }
        BusinessError::ThrowErrorByErrcode(env, ret);
    }

    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}
    
napi_value NapiWebviewController::ZoomOut(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);

    ErrCode ret = webviewController->ZoomOut();
    if (ret != NO_ERROR) {
        if (ret == NWEB_ERROR) {
            LOGE("ZoomOut failed.");
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


napi_value NapiWebviewController::GetOriginalUrl(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);
    std::string url = webviewController->GetOriginalUrl();
    napi_create_string_utf8(env, url.c_str(), url.length(), &result);
    return result;
}

napi_value NapiWebviewController::PageUp(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };
    bool top = false;
    
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_ONE, "one"));
        return result;
    }

    if (!NapiParseUtils::ParseBoolean(env, argv[0], top)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "top", "boolean"));
        return result;
    }

    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);

    ErrCode ret = webviewController->PageUp(top);
    if (ret != NO_ERROR) {
        if (ret == INIT_ERROR) {
            LOGE("PageUp failed.");
            return result;
        }
        BusinessError::ThrowErrorByErrcode(env, ret);
    }
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

napi_value NapiWebviewController::SetWebDebuggingAccess(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };
    bool webDebuggingAccess = false;

    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return result;
    }

    if (!NapiParseUtils::ParseBoolean(env, argv[0], webDebuggingAccess)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "webDebuggingAccess","boolean"));
        return result;
    }

    WebviewController::SetWebDebuggingAccess(webDebuggingAccess);
    NAPI_CALL(env, napi_get_undefined(env, &result));
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

napi_value NapiWebviewController::GetWebId(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    WebviewController *webviewController = GetWebviewController(env, info);
    CHECK_NULL_RETURN(webviewController, result);
    int32_t newebId = webviewController->GetWebId();
    #ifdef ANDROID_PLATFORM
        int32_t resultWebId = newebId+1;
        napi_create_int32(env, resultWebId, &result);
    #else
        napi_create_int32(env, newebId, &result);
    #endif
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
    WebMessagePort *msgPort = new (std::nothrow) WebMessagePortAndroid(webId, portHandle, isExtentionType);
#endif
#ifdef IOS_PLATFORM
    WebMessagePort *msgPort = new (std::nothrow) WebMessagePortIOS(webId, portHandle, isExtentionType);
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

napi_value NapiWebMessagePort::PostMessageEventExt(napi_env env, napi_callback_info info)
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
    if (valueType != napi_object) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_TYPE_INVALID, "message"));
        return result;
    }

    WebMessageExt *webMessageExt = nullptr;
    NAPI_CALL(env, napi_unwrap(env, argv[INTEGER_ZERO], (void **)&webMessageExt));
    if (webMessageExt == nullptr) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NOT_NULL, "message"));
        return nullptr;
    }
    WebMessagePort *msgPort = nullptr;
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void **)&msgPort));
    if (msgPort == nullptr) {
        return nullptr;
    }

    if (!msgPort->IsExtentionType()) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_TYPE_INVALID, "message"));
        return result;
    }

    auto errorCode = msgPort->PostMessageEventExt(webMessageExt);
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

napi_value NapiWebMessagePort::OnMessageEventExt(napi_env env, napi_callback_info info)
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

    auto webMessageCallbackInfo = std::make_shared<NapiJsCallBackParmExt>(env);
    auto param = webMessageCallbackInfo.get();
    CHECK_NULL_RETURN(param, nullptr);
    param->callback = onMsgEventFunc;
    msgPort->SetWebMessageExtCallback(webMessageCallbackInfo);

    ErrCode ret = msgPort->OnMessageEventExt();
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

napi_value NapiWebviewController::StartDownload(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    WebviewController* webviewController = nullptr;
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&webviewController));
    if (webviewController == nullptr || !webviewController->IsInit()) {
        BusinessError::ThrowErrorByErrcode(env, INIT_ERROR);
        LOGE("create message port failed, napi unwrap webviewController failed");
        return nullptr;
    }

    std::string url;
    if (!ParsePrepareUrl(env, argv[INTEGER_ZERO], url)) {
        BusinessError::ThrowErrorByErrcode(env, INVALID_URL);
        return nullptr;
    }
    webviewController->StartDownload(url);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiWebviewController::SetDownloadDelegate(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    WebDownloadDelegate* delegate = nullptr;
    napi_value obj = argv[0];
    napi_unwrap(env, obj, (void**)&delegate);
    if (!delegate) {
        LOGE("WebDownloader::JS_SetDownloadDelegate delegate is null");
        (void)RemoveDownloadDelegateRef(env, thisVar);
        return nullptr;
    }
    napi_create_reference(env, obj, 1, &delegate->delegate_);

    WebviewController* webviewController = nullptr;
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&webviewController));
    if (webviewController == nullptr || !webviewController->IsInit()) {
        BusinessError::ThrowErrorByErrcode(env, INIT_ERROR);
        LOGE("create message port failed, napi unwrap webviewController failed");
        return nullptr;
    }
    int32_t nwebId = webviewController->GetWebId();
    WebDownloadManager::AddDownloadDelegateForWeb(nwebId, delegate);
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiWebviewController::RegisterJavaScriptProxy(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_FIVE;
    napi_value argv[INTEGER_FIVE] = { 0 };
    NAPI_CALL(env, napi_get_undefined(env, &result));
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    if (argc < INTEGER_THREE || argc > INTEGER_FIVE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_THREE, "three", "four", "five"));
        return result;
    }
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[INTEGER_ZERO], &valueType));
    if (valueType != napi_object) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "object", "object"));
        return result;
    }
    RegisterJavaScriptProxyParam param;
    if (!ParseRegisterJavaScriptProxyParam(env, argc, argv, &param)) {
        return result;
    }
    WebviewController* webviewController = nullptr;
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&webviewController));
    if (webviewController == nullptr || !webviewController->IsInit()) {
        BusinessError::ThrowErrorByErrcode(env, INIT_ERROR);
        return result;
    }
    WebviewController::InsertJavaScriptProxy(param);
    webviewController->RegisterJavaScriptProxy(param);
    return result;
}

napi_value NapiWebviewController::SetWebSchemeHandler(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    WebviewController *webviewController = nullptr;
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void **)&webviewController));
    if (webviewController == nullptr || !webviewController->IsInit()) {
        BusinessError::ThrowErrorByErrcode(env, INIT_ERROR);
        LOGE("create message port failed, napi unwrap webviewController failed");
        return nullptr;
    }

    std::string scheme = "";
    if (!NapiParseUtils::ParseString(env, argv[0], scheme)) {
        LOGE("NapiWebviewController::SetWebSchemeHandler parse scheme failed");
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "scheme", "string"));
        return nullptr;
    }

    WebSchemeHandler* handler = nullptr;
    napi_value obj = argv[1];
    napi_unwrap(env, obj, (void**)&handler);
    if (!handler) {
        LOGE("NapiWebviewController::SetWebSchemeHandler handler is null");
        return nullptr;
    }
    if (!webviewController->SetWebSchemeHandler(scheme.c_str(), handler)) {
        LOGE("NapiWebviewController::SetWebSchemeHandler failed");
    }
    return nullptr;
}

napi_value NapiWebviewController::ClearWebSchemeHandler(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    WebviewController *webviewController = GetWebviewController(env, info);
    if (!webviewController) {
        napi_get_undefined(env, &result);
        return result;
    }

    if (!webviewController->ClearWebSchemeHandler()) {
        LOGE("NapiWebviewController::ClearWebSchemeHandler failed");
    }
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

napi_value NapiWebviewController::DeleteJavaScriptRegister(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    if (argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_ONE, "one"));
        return result;
    }

    std::string objName;
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ZERO], objName)) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "name", "string"));
        return result;
    }

    WebviewController* controller = nullptr;
    NAPI_CALL(env, napi_unwrap(env, thisVar, (void **)&controller));
    if (!controller || !controller->IsInit()) {
        BusinessError::ThrowErrorByErrcode(env, INIT_ERROR);
        return result;
    }

    ErrCode ret = WebviewController::CheckObjectName(objName);
    if (ret != NO_ERROR) {
        BusinessError::ThrowErrorByErrcode(env, ret);
        return result;
    }

    controller->DeleteJavaScriptRegister(objName);
    WebviewController::DeleteJavaScriptProxy(objName);
    return result;
}

napi_value NapiWebviewController::GetUserAgent(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    WebviewController *webviewController = GetWebviewController(env, info);
    if (!webviewController) {
        return nullptr;
    }

    std::string userAgent = "";
    userAgent = webviewController->GetUserAgent();
    napi_create_string_utf8(env, userAgent.c_str(), userAgent.length(), &result);

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
        DECLARE_NAPI_FUNCTION("runJavaScriptExt", NapiWebviewController::RunJavaScriptExt),
        DECLARE_NAPI_FUNCTION("accessStep", NapiWebviewController::AccessStep),
        DECLARE_NAPI_FUNCTION("scrollTo", NapiWebviewController::ScrollTo),
        DECLARE_NAPI_FUNCTION("scrollBy", NapiWebviewController::ScrollBy),
        DECLARE_NAPI_FUNCTION("zoom", NapiWebviewController::Zoom),
        DECLARE_NAPI_FUNCTION("zoomIn", NapiWebviewController::ZoomIn),
        DECLARE_NAPI_FUNCTION("zoomOut", NapiWebviewController::ZoomOut),
        DECLARE_NAPI_FUNCTION("stop", NapiWebviewController::Stop),
        DECLARE_NAPI_FUNCTION("getOriginalUrl", NapiWebviewController::GetOriginalUrl),
        DECLARE_NAPI_FUNCTION("pageUp", NapiWebviewController::PageUp),
        DECLARE_NAPI_FUNCTION("clearHistory", NapiWebviewController::ClearHistory),
        DECLARE_NAPI_FUNCTION("setCustomUserAgent", NapiWebviewController::SetCustomUserAgent),
        DECLARE_NAPI_FUNCTION("getCustomUserAgent", NapiWebviewController::GetCustomUserAgent),
        DECLARE_NAPI_FUNCTION("getBackForwardEntries", NapiWebviewController::GetBackForwardEntries),
        DECLARE_NAPI_FUNCTION("removeCache", NapiWebviewController::RemoveCache),
        DECLARE_NAPI_FUNCTION("backOrForward", NapiWebviewController::BackOrForward),
        DECLARE_NAPI_FUNCTION("getTitle", NapiWebviewController::GetTitle),
        DECLARE_NAPI_FUNCTION("getPageHeight", NapiWebviewController::GetPageHeight),
        DECLARE_NAPI_FUNCTION("getWebId", NapiWebviewController::GetWebId),
        DECLARE_NAPI_FUNCTION("createWebMessagePorts", NapiWebviewController::CreateWebMessagePorts),
        DECLARE_NAPI_FUNCTION("postMessage", NapiWebviewController::PostMessage),
        DECLARE_NAPI_STATIC_FUNCTION("setWebDebuggingAccess", NapiWebviewController::SetWebDebuggingAccess),
        DECLARE_NAPI_FUNCTION("pageDown", NapiWebviewController::PageDown),
        DECLARE_NAPI_FUNCTION("postUrl", NapiWebviewController::PostUrl),
        DECLARE_NAPI_FUNCTION("startDownload", NapiWebviewController::StartDownload),
        DECLARE_NAPI_FUNCTION("setDownloadDelegate", NapiWebviewController::SetDownloadDelegate),
        DECLARE_NAPI_FUNCTION("registerJavaScriptProxy", NapiWebviewController::RegisterJavaScriptProxy),
        DECLARE_NAPI_FUNCTION("deleteJavaScriptRegister", NapiWebviewController::DeleteJavaScriptRegister),
        DECLARE_NAPI_FUNCTION("jsProxy", NapiWebviewController::RegisterJavaScriptProxy),
        DECLARE_NAPI_FUNCTION("setWebSchemeHandler", NapiWebviewController::SetWebSchemeHandler),
        DECLARE_NAPI_FUNCTION("clearWebSchemeHandler", NapiWebviewController::ClearWebSchemeHandler),
        DECLARE_NAPI_FUNCTION("getUserAgent", NapiWebviewController::GetUserAgent),
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
        DECLARE_NAPI_FUNCTION("postMessageEventExt", NapiWebMessagePort::PostMessageEventExt),
        DECLARE_NAPI_FUNCTION("onMessageEvent", NapiWebMessagePort::OnMessageEvent),
        DECLARE_NAPI_FUNCTION("onMessageEventExt", NapiWebMessagePort::OnMessageEventExt),
    };
    NAPI_CALL(env, napi_define_class(env, WEB_MESSAGE_PORT_CLASS_NAME.c_str(), WEB_MESSAGE_PORT_CLASS_NAME.length(),
        NapiWebMessagePort::JsConstructor, nullptr, sizeof(msgPortProperties) / sizeof(msgPortProperties[0]),
        msgPortProperties, &msgPortCons));
    NAPI_CALL(env, napi_create_reference(env, msgPortCons, 1, &g_classWebMsgPort));
    NAPI_CALL(env, napi_set_named_property(env, exports, WEB_MESSAGE_PORT_CLASS_NAME.c_str(), msgPortCons));

    AsyncJavaScriptExtEvaluteJSResultCallbackInfo::InitJSExcute(env, exports, g_jsMsgExtClassRef);
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
    NapiGeolocationPermissions::Init(env, exports);
    NapiWebStorage::Init(env, exports);
    NapiWebDownloadDelegate::Init(env, exports);
    NapiWebDownloadManager::Init(env, exports);
    NapiWebDownloadItem::Init(env, exports);
    NapiWebMessageExt::Init(env, exports);
    NapiWebDownloadDelegate::Init(env, exports);
    NapiWebDownloadManager::Init(env, exports);
    NapiWebDownloadItem::Init(env, exports);
    NapiWebMessageExt::Init(env, exports);
    NapiWebSchemeHandler::Init(env, exports);
    NapiWebSchemeHandlerRequest::Init(env, exports);
    NapiWebSchemeHandlerResponse::Init(env, exports);
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
    const char GeolocationPermissionsClassName[] = "ohos.ace.adapter.capability.web.AceGeolocationPermissions";
    ARKUI_X_Plugin_RegisterJavaPlugin(&GeolocationPermissionsJni::Register, GeolocationPermissionsClassName);
    const char webStorageClassName[] = "ohos.ace.plugin.webviewplugin.webstorage.WebStoragePlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&WebStorageJni::Register, webStorageClassName);
    const char webDownloadDelegateClassName[] = "ohos.ace.adapter.capability.web.AceWebPluginBase";
    ARKUI_X_Plugin_RegisterJavaPlugin(&WebDownloadDelegateJni::Register, webDownloadDelegateClassName);
    const char webDownloadItemClassName[] = "ohos.ace.adapter.capability.web.AceWebPluginBase";
    ARKUI_X_Plugin_RegisterJavaPlugin(&WebDownloadItemJni::Register, webDownloadItemClassName);
    const char webSchemeRequestClassName[] = "ohos.ace.adapter.capability.web.AceWebPluginBase";
    ARKUI_X_Plugin_RegisterJavaPlugin(&WebSchemeHandlerJni::Register, webSchemeRequestClassName);
    const char AndroidAssetClassName[] = "ohos.ace.plugin.webviewplugin.androidasset.AndroidAssetPlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&AndroidAssetHelperJni::Register, AndroidAssetClassName);
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

std::regex NapiGeolocationPermissions::originPattern(".*://.*");

napi_value NapiGeolocationPermissions::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_STATIC_FUNCTION("allowGeolocation", NapiGeolocationPermissions::JsAllowGeolocation),
        DECLARE_NAPI_STATIC_FUNCTION("deleteGeolocation", NapiGeolocationPermissions::JsDeleteGeolocation),
        DECLARE_NAPI_STATIC_FUNCTION("deleteAllGeolocation", NapiGeolocationPermissions::JsDeleteAllGeolocation),
        DECLARE_NAPI_STATIC_FUNCTION("getAccessibleGeolocation", NapiGeolocationPermissions::JsGetAccessibleGeolocation),
        DECLARE_NAPI_STATIC_FUNCTION("getStoredGeolocation", NapiGeolocationPermissions::JsGetStoredGeolocation),
    };
    napi_value constructor = nullptr;

    napi_define_class(env, GEOLOCATION_PERMISSIONS_CLASS_NAME.c_str(), GEOLOCATION_PERMISSIONS_CLASS_NAME.length(),
        NapiGeolocationPermissions::JsConstructor, nullptr, sizeof(properties) / sizeof(properties[0]), properties,
        &constructor);
    NAPI_ASSERT(env, constructor != nullptr, "NapiGeolocationPermissions define js class failed");
    napi_status status = napi_set_named_property(env, exports, "GeolocationPermissions", constructor);
    NAPI_ASSERT(env, status == napi_ok, "NapiGeolocationPermissions set property failed");
    return exports;
}

napi_value NapiGeolocationPermissions::JsConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    size_t argc = INTEGER_TWO;
    napi_value argv[INTEGER_TWO] = { 0 };
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    return thisVar;
}

napi_value NapiWebStorage::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_STATIC_FUNCTION("getOriginQuota", NapiWebStorage::JsGetOriginQuotaAsync),
        DECLARE_NAPI_STATIC_FUNCTION("getOriginUsage", NapiWebStorage::JsGetOriginUsageAsync),
        DECLARE_NAPI_STATIC_FUNCTION("getOrigins", NapiWebStorage::JsGetOriginsAsync),
        DECLARE_NAPI_STATIC_FUNCTION("deleteAllData", NapiWebStorage::DeleteAllData),
        DECLARE_NAPI_STATIC_FUNCTION("deleteOrigin", NapiWebStorage::DeleteOrigin),
    };
    napi_value constructor = nullptr;

    napi_define_class(env, WEB_STORAGE_CLASS_NAME.c_str(), WEB_STORAGE_CLASS_NAME.length(),
        NapiWebStorage::JsConstructor, nullptr, sizeof(properties) / sizeof(properties[0]),
        properties, &constructor);
    NAPI_ASSERT(env, constructor != nullptr, "NapiWebStorage define js class failed");
    napi_status status = napi_set_named_property(env, exports, "WebStorage", constructor);
    NAPI_ASSERT(env, status == napi_ok, "NapiWebStorage set property failed");
    return exports;
}

napi_value NapiWebStorage::JsConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    return thisVar;
}

napi_value NapiGeolocationPermissions::JsAllowGeolocation(napi_env env, napi_callback_info info)
{
    napi_value retValue = nullptr;
    size_t argc = INTEGER_TWO;
    napi_value argv[INTEGER_TWO] = {0};
    napi_get_cb_info(env, info, &argc, argv, &retValue, nullptr);
    if (argc != INTEGER_ONE && argc != INTEGER_TWO) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_TWO, "one", "two"));
        return retValue;
    }

    std::string origin;
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ZERO], origin)) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "origin", "string"));
        return retValue;
    }

    if (!std::regex_match(origin, originPattern)) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::INVALID_ORIGIN);
        return retValue;
    }

    bool incognito = false;
    if (argc == INTEGER_TWO) {
        napi_get_value_bool(env, argv[INTEGER_ONE], &incognito);
    }
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    GeolocationPermissions::AllowGeolocation(origin, incognito);
    return result;
}

napi_value NapiGeolocationPermissions::JsDeleteGeolocation(napi_env env, napi_callback_info info)
{
    napi_value retValue = nullptr;
    size_t argc = INTEGER_TWO;
    napi_value argv[INTEGER_TWO] = {0};
    napi_get_cb_info(env, info, &argc, argv, &retValue, nullptr);
    if (argc != INTEGER_ONE && argc != INTEGER_TWO) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_TWO, "one", "two"));
        return retValue;
    }

    std::string origin;
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ZERO], origin)) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "origin", "string"));
        return retValue;
    }

    if (!std::regex_match(origin, originPattern)) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::INVALID_ORIGIN);
        return retValue;
    }

    bool incognito = false;
    if (argc == INTEGER_TWO) {
        napi_get_value_bool(env, argv[INTEGER_ONE], &incognito);
    }
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    GeolocationPermissions::DeleteGeolocation(origin, incognito);
    return result;
}

napi_value NapiGeolocationPermissions::JsDeleteAllGeolocation(napi_env env, napi_callback_info info)
{
    napi_value retValue = nullptr;
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };
    napi_get_cb_info(env, info, &argc, argv, &retValue, nullptr);
    if (argc != INTEGER_ZERO && argc != INTEGER_ONE) {
        return nullptr;
    }
    bool incognito = false;
    if (argc == INTEGER_ONE) {
        napi_get_value_bool(env, argv[INTEGER_ZERO], &incognito);
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    GeolocationPermissions::DeleteAllGeolocation(incognito);
    return result;
}

napi_value NapiGeolocationPermissions::JsGetAccessibleGeolocation(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_value retValue = nullptr;
    size_t argc = INTEGER_THREE;
    size_t argcPromise = INTEGER_ONE;
    size_t argcPromiseOrCallback = INTEGER_TWO;
    size_t argcCallback = INTEGER_THREE;
    napi_value argv[INTEGER_THREE] = {0};
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != argcPromise && argc != argcPromiseOrCallback && argc != argcCallback) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_THREE, "one", "two", "three"));
        return retValue;
    }

    std::string origin;
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ZERO], origin)) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "origin", "string"));
        return retValue;
    }

    if (!std::regex_match(origin, originPattern)) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::INVALID_ORIGIN);
        return retValue;
    }

    std::shared_ptr<GeolocationPermissionsResultCallbackInfo> asyncCallbackInfoInstance =
        std::make_shared<GeolocationPermissionsResultCallbackInfo>(env, g_asyncCallbackInfoId);
    auto asyncCallbackInfo = asyncCallbackInfoInstance.get();
    napi_value promise = nullptr;
    bool incognito = false;
    if (argc == argcCallback) {
        napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
        NAPI_CALL(env, napi_create_reference(env, argv[INTEGER_ONE], 1, &asyncCallbackInfo->callback));
        NAPI_CALL(env, napi_get_undefined(env, &promise));
        NapiParseUtils::ParseBoolean(env, argv[INTEGER_TWO], incognito);
    } else if (argc == argcPromiseOrCallback) {
        napi_valuetype valueType = napi_null;
        napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
        napi_typeof(env, argv[INTEGER_ONE], &valueType);
        if (valueType == napi_function) {
            NAPI_CALL(env, napi_create_reference(env, argv[INTEGER_ONE], 1, &asyncCallbackInfo->callback));
            NAPI_CALL(env, napi_get_undefined(env, &promise));
        } else if (valueType == napi_boolean) {
            NAPI_CALL(env, napi_create_promise(env, &asyncCallbackInfo->deferred, &promise));
            NapiParseUtils::ParseBoolean(env, argv[INTEGER_ONE], incognito);
        } else {
            BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
            return retValue;
        }
    } else if (argc == argcPromise) {
        NAPI_CALL(env, napi_create_promise(env, &asyncCallbackInfo->deferred, &promise));
    }
    CreateGetAccessibleGeolocationAsyncWork(env, asyncCallbackInfoInstance);
    GeolocationPermissions::GetAccessibleGeolocation(origin, g_asyncCallbackInfoId, incognito);
    if (++g_asyncCallbackInfoId >= MAX_COUNT_ID) {
        g_asyncCallbackInfoId = 0;
    }
    return promise;
}

void NapiWebStorage::CreateGetOriginQuotaAsyncWork(
    napi_env env, const std::shared_ptr<AsyncWebStorageResultCallbackInfo> &callbackInfo)
{
    napi_value resource = nullptr;
    auto asyncCallbackInfo = callbackInfo.get();
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "JsGetOriginQuotaAsync", NAPI_AUTO_LENGTH, &resource));
    NAPI_CALL_RETURN_VOID(env, napi_create_async_work(env, nullptr, resource,
        [](napi_env env, void* data) {
        },
        [](napi_env env, napi_status status, void* data) {
            AsyncWebStorageResultCallbackInfo* asyncCallbackInfo =
                reinterpret_cast<AsyncWebStorageResultCallbackInfo*>(data);
            if (!asyncCallbackInfo) {
                return;
            }
            napi_value args[INTEGER_TWO] = { 0 }; 
            napi_value jsResult = nullptr;
            NAPI_CALL_RETURN_VOID(env, napi_create_int64(env, asyncCallbackInfo->result, &jsResult));
            args[INTEGER_ONE] = jsResult;

            if (asyncCallbackInfo->deferred) {
                NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, asyncCallbackInfo->deferred, args[INTEGER_ONE]));
            } else {
                napi_value callback = nullptr;
                napi_value callbackResult = nullptr;
                NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, asyncCallbackInfo->callback, &callback));
                NAPI_CALL_RETURN_VOID(env, napi_call_function(env, nullptr, callback,
                    sizeof(args) / sizeof(args[0]), args, &callbackResult));
            }
            WebStorage::EraseCallbackInfo(asyncCallbackInfo, StorageTaskType::GET_ORIGINQUOTA);
        },
        reinterpret_cast<void*>(asyncCallbackInfo), &asyncCallbackInfo->asyncWork));
    WebStorage::InsertCallbackInfo(callbackInfo, StorageTaskType::GET_ORIGINQUOTA);
}

void NapiWebStorage::CreateGetOriginUsageAsyncWork(
    napi_env env, const std::shared_ptr<AsyncWebStorageResultCallbackInfo> &callbackInfo)
{
    napi_value resource = nullptr;
    auto asyncCallbackInfo = callbackInfo.get();
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "JsGetOriginUsageAsync", NAPI_AUTO_LENGTH, &resource));
    NAPI_CALL_RETURN_VOID(env, napi_create_async_work(env, nullptr, resource,
        [](napi_env env, void* data) {
        },
        [](napi_env env, napi_status status, void* data) {
            AsyncWebStorageResultCallbackInfo* asyncCallbackInfo =
                reinterpret_cast<AsyncWebStorageResultCallbackInfo*>(data);
            if (!asyncCallbackInfo) {
                return;
            }
            napi_value args[2] = { 0 };
            napi_value jsResult = nullptr;
            NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, asyncCallbackInfo->result, &jsResult));
            args[INTEGER_ONE] = jsResult;
            if (asyncCallbackInfo->deferred) {
                NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, asyncCallbackInfo->deferred, args[INTEGER_ONE]));    
            } else {
                napi_value callback = nullptr;
                napi_value callbackResult = nullptr;
                NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, asyncCallbackInfo->callback, &callback));
                NAPI_CALL_RETURN_VOID(env, napi_call_function(env, nullptr, callback,
                    sizeof(args) / sizeof(args[0]), args, &callbackResult));
            }
            WebStorage::EraseCallbackInfo(asyncCallbackInfo, StorageTaskType::GET_ORIGINUSAGE);
        },
        reinterpret_cast<void*>(asyncCallbackInfo), &asyncCallbackInfo->asyncWork));
    WebStorage::InsertCallbackInfo(callbackInfo, StorageTaskType::GET_ORIGINUSAGE);
}

void NapiWebStorage::GetNapiWebStorageOriginForResult(napi_env env,
    const std::vector<WebStorageStruct> &info, napi_value result)
{
    int32_t index = 0;
    for (auto item : info) {
        napi_value napiWebStorageOrigin = nullptr;
        napi_create_object(env, &napiWebStorageOrigin);

        napi_value origin = nullptr;
        napi_create_string_utf8(env, item.origin.c_str(), NAPI_AUTO_LENGTH, &origin);
        napi_set_named_property(env, napiWebStorageOrigin, "origin", origin);

        napi_value quota = nullptr;
        long long intQuotaValue = std::stoll(item.quota.c_str());
        napi_create_int64(env, static_cast<int64_t>(intQuotaValue), &quota);
        napi_set_named_property(env, napiWebStorageOrigin, "quota", quota);

        napi_value usage = nullptr;
        long long intUsageValue = std::stoll(item.usage.c_str());
        napi_create_int64(env, static_cast<int64_t>(intUsageValue), &usage);
        napi_set_named_property(env, napiWebStorageOrigin, "usage", usage);

        napi_set_element(env, result, index, napiWebStorageOrigin);
        index++;
    }
}

void NapiWebStorage::CreateGetOriginsAsyncWork(
    napi_env env, const std::shared_ptr<AsyncWebStorageResultCallbackInfo>& callbackInfo)
{
    napi_value resource = nullptr;
    auto asyncCallbackInfo = callbackInfo.get();
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "JsGetOriginsAsync", NAPI_AUTO_LENGTH, &resource));
    NAPI_CALL_RETURN_VOID(env, napi_create_async_work(env, nullptr, resource,
        [](napi_env env, void* data) {},
        [](napi_env env, napi_status status, void* data) {
            AsyncWebStorageResultCallbackInfo* asyncCallbackInfo =
                reinterpret_cast<AsyncWebStorageResultCallbackInfo*>(data);
            if (!asyncCallbackInfo) {
                return;
            }
            napi_value args[INTEGER_TWO] = { 0 };
            int business_error = asyncCallbackInfo->resultVector.empty() || asyncCallbackInfo->resultVector.size() <= 0 ? 1 : 2;
            if (business_error == 1) {
                args[INTEGER_ZERO] = NWebError::BusinessError::CreateError(env, NWebError::NO_WEBSTORAGE_ORIGIN);
                napi_get_undefined(env, &args[INTEGER_ONE]);
            } else {
                napi_get_undefined(env, &args[INTEGER_ZERO]);
                napi_create_array(env, &args[INTEGER_ONE]);
                GetNapiWebStorageOriginForResult(env, asyncCallbackInfo->resultVector, args[INTEGER_ONE]);
            }
            if (asyncCallbackInfo->deferred) {
                if (business_error == 1) {
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
            WebStorage::EraseCallbackInfo(asyncCallbackInfo, StorageTaskType::GET_ORIGINS);
        },
        reinterpret_cast<void*>(asyncCallbackInfo), &asyncCallbackInfo->asyncWork));
    WebStorage::InsertCallbackInfo(callbackInfo, StorageTaskType::GET_ORIGINS);
}

napi_value NapiWebStorage::JsGetOriginQuotaAsync(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    size_t argc = INTEGER_ONE;
    size_t argcPromise = INTEGER_ONE;
    size_t argcCallback = INTEGER_TWO;
    napi_value argv[INTEGER_TWO] = { 0 };
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);

    if (argc != argcPromise && argc != argcCallback) {
       BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR, 
            FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_TWO, "one", "two"));
        return nullptr;
    }

    napi_valuetype valueType = napi_null;
    napi_typeof(env, argv[INTEGER_ZERO], &valueType);
    if (valueType != napi_string) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "origin", "string"));
        return nullptr;
    }

    size_t bufferSize = 0;
    napi_get_value_string_utf8(env, argv[INTEGER_ZERO], nullptr, 0, &bufferSize);
    if (bufferSize >= MAX_WEB_STRING_LENGTH) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            "BusinessError 401: Parameter error. The length of 'origin' must less than 40960.");
        return nullptr;
    }
    
    char stringValue[bufferSize + 1];
    size_t jsStringLength = 0;
    napi_get_value_string_utf8(env, argv[INTEGER_ZERO], stringValue, bufferSize + 1, &jsStringLength);
    if (jsStringLength != bufferSize) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            "BusinessError 401: Parameter error. The length of 'origin' obtained twice are different");
        return nullptr;
    }

    std::string origin(stringValue);
    std::shared_ptr<AsyncWebStorageResultCallbackInfo> asyncCallbackInfoInstance =
        std::make_shared<AsyncWebStorageResultCallbackInfo>(env, g_asyncCallbackInfoId);
    auto asyncCallbackInfo = asyncCallbackInfoInstance.get();
    napi_value promise = nullptr;
    if (argc == argcCallback) {
        napi_valuetype valueType = napi_null;
        napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
        napi_typeof(env, argv[argcCallback - 1], &valueType);
        if (valueType != napi_function) {
             BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
                FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "callback","function"));
            return nullptr;
        }
        NAPI_CALL(env, napi_create_reference(env, argv[argcCallback - 1], 1, &asyncCallbackInfo->callback));
        NAPI_CALL(env, napi_get_undefined(env, &promise));
    } else if (argc == argcPromise) {
        NAPI_CALL(env, napi_create_promise(env, &asyncCallbackInfo->deferred, &promise));
    }
    CreateGetOriginQuotaAsyncWork(env, asyncCallbackInfoInstance);
    WebStorage::GetOriginQuota(origin, g_asyncCallbackInfoId);
    if (++g_asyncCallbackInfoId >= MAX_COUNT_ID) {
        g_asyncCallbackInfoId = 0;
    }
    return promise;
}

napi_value NapiGeolocationPermissions::JsGetStoredGeolocation(napi_env env, napi_callback_info info)
{
    LOGI("GeolocationPermissions JsGetStoredGeolocation coming");
    napi_value thisVar = nullptr;
    napi_value retValue = nullptr;
    size_t argc = INTEGER_TWO;
    size_t argcPromise = INTEGER_ZERO;
    size_t argcPromiseOrCallback = INTEGER_ONE;
    size_t argcCallback = INTEGER_TWO;
    napi_value argv[INTEGER_TWO] = {0};
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != argcPromise && argc != argcPromiseOrCallback && argc != argcCallback) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_THREE, "one", "two", "three"));
        return retValue;
    }

    std::shared_ptr<GeolocationPermissionsResultCallbackInfo> asyncCallbackInfoInstance =
        std::make_shared<GeolocationPermissionsResultCallbackInfo>(env, g_asyncCallbackInfoId);
    auto asyncCallbackInfo = asyncCallbackInfoInstance.get();
    napi_value promise = nullptr;
    bool incognito = false;
    if (argc == argcCallback) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[INTEGER_ZERO], &valueType);
        if (valueType != napi_function) {
            NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
                NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "callback", "function"));
            return nullptr;
        }
        if (!NapiParseUtils::ParseBoolean(env, argv[INTEGER_ONE], incognito)) {
            NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
                NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "incognito", "boolean"));
            return nullptr;
        }
        napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
        NAPI_CALL(env, napi_create_reference(env, argv[INTEGER_ZERO], 1, &asyncCallbackInfo->callback));
        NAPI_CALL(env, napi_get_undefined(env, &promise));
    } else if (argc == argcPromiseOrCallback) {
        napi_valuetype valueType = napi_null;
        napi_typeof(env, argv[INTEGER_ZERO], &valueType);
        if (valueType != napi_function) {
            if (!NapiParseUtils::ParseBoolean(env, argv[INTEGER_ZERO], incognito)) {
                NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
                    NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "incognito", "boolean"));
                return nullptr;
            }
            NAPI_CALL(env, napi_create_promise(env, &asyncCallbackInfo->deferred, &promise));
        } else {
            NAPI_CALL(env, napi_create_reference(env, argv[INTEGER_ZERO], 1, &asyncCallbackInfo->callback));
            NAPI_CALL(env, napi_get_undefined(env, &promise));
        }
    } else if (argc == argcPromise) {
        NAPI_CALL(env, napi_create_promise(env, &asyncCallbackInfo->deferred, &promise));
    }
    CreateGetStoredGeolocationAsyncWork(env, asyncCallbackInfoInstance);
    GeolocationPermissions::GetStoredGeolocation(g_asyncCallbackInfoId, incognito);
    if (++g_asyncCallbackInfoId >= MAX_COUNT_ID) {
        g_asyncCallbackInfoId = 0;
    }
    return promise;
}

napi_value NapiWebStorage::JsGetOriginUsageAsync(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    size_t argc = INTEGER_ONE;
    size_t argcPromise = INTEGER_ONE;
    size_t argcCallback = INTEGER_TWO;
    napi_value argv[INTEGER_TWO] = {0};

    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != argcPromise && argc != argcCallback) {
       BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR, 
            FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_TWO, "one", "two"));
        return nullptr;
    }

    napi_valuetype valueType = napi_null;
    napi_typeof(env, argv[INTEGER_ZERO], &valueType);
    if (valueType != napi_string) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "origin", "string"));
        return nullptr;
    }

    size_t bufferSize = 0;
    napi_get_value_string_utf8(env, argv[INTEGER_ZERO], nullptr, 0, &bufferSize);
    if (bufferSize >= MAX_WEB_STRING_LENGTH) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            "BusinessError 401: Parameter error. The length of 'origin' must less than 40960.");
        return nullptr;
    }

    char stringValue[bufferSize + 1];
    size_t jsStringLength = 0;
    napi_get_value_string_utf8(env, argv[INTEGER_ZERO], stringValue, bufferSize + 1, &jsStringLength);
    if (jsStringLength != bufferSize) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            "BusinessError 401: Parameter error. The length of 'origin' obtained twice are different");
        return nullptr;
    }

    std::string origin(stringValue);
    std::shared_ptr<AsyncWebStorageResultCallbackInfo> asyncCallbackInfoInstance =
        std::make_shared<AsyncWebStorageResultCallbackInfo>(env, g_asyncCallbackInfoId);
    auto asyncCallbackInfo = asyncCallbackInfoInstance.get();
    napi_value promise = nullptr;
    if (argc == argcCallback) {
        napi_valuetype valueType = napi_null;
        napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
        napi_typeof(env, argv[argcCallback - 1], &valueType);
        if (valueType != napi_function) {
             BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
                FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "callback","function"));
            return nullptr;
        }
        NAPI_CALL(env, napi_create_reference(env, argv[argcCallback - 1], 1, &asyncCallbackInfo->callback));
        NAPI_CALL(env, napi_get_undefined(env, &promise));
    } else if (argc == argcPromise) {
        NAPI_CALL(env, napi_create_promise(env, &asyncCallbackInfo->deferred, &promise));
    }
    CreateGetOriginUsageAsyncWork(env, asyncCallbackInfoInstance);
    WebStorage::GetOriginUsage(origin, g_asyncCallbackInfoId);
    if (++g_asyncCallbackInfoId >= MAX_COUNT_ID) {
        g_asyncCallbackInfoId = 0;
    }
    return promise;
}

napi_value NapiWebStorage::JsGetOriginsAsync(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    size_t argc = INTEGER_ZERO;
    size_t argcPromise = INTEGER_ZERO;
    size_t argcCallback = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };
    napi_value result = nullptr;

    napi_get_undefined(env, &result);
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != argcPromise && argc != argcCallback) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR, 
            FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_TWO, "zero", "one"));
        return nullptr;
    }

    std::shared_ptr<AsyncWebStorageResultCallbackInfo> asyncCallbackInfoInstance =
        std::make_shared<AsyncWebStorageResultCallbackInfo>(env, g_asyncCallbackInfoId);
    auto asyncCallbackInfo = asyncCallbackInfoInstance.get();
    asyncCallbackInfo->storageTaskType = StorageTaskType::GET_ORIGINS;
    napi_value promise = nullptr;
    if (argc == argcCallback) {
        napi_valuetype valueType = napi_null;
        napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
        napi_typeof(env, argv[argcCallback - 1], &valueType);
        if (valueType != napi_function) {
             BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR, 
                FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "callback","function"));
            return nullptr;
        }
        NAPI_CALL(env, napi_create_reference(env, argv[argcCallback - 1], 1, &asyncCallbackInfo->callback));
        NAPI_CALL(env, napi_get_undefined(env,  &promise));
    } else if (argc == argcPromise) {
        NAPI_CALL(env, napi_create_promise(env, &asyncCallbackInfo->deferred, &promise));
    }
    CreateGetOriginsAsyncWork(env, asyncCallbackInfoInstance);
    WebStorage::GetOrigins(g_asyncCallbackInfoId);
    if (++g_asyncCallbackInfoId >= MAX_COUNT_ID) {
        g_asyncCallbackInfoId = 0;
    }
    return promise;
}

void NapiGeolocationPermissions::CreateGetAccessibleGeolocationAsyncWork(napi_env env,
    const std::shared_ptr<GeolocationPermissionsResultCallbackInfo> &callbackInfo)
{
    napi_value resource = nullptr;
    auto asyncCallbackInfo = callbackInfo.get();
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "JsGetAccessibleGeolocation", NAPI_AUTO_LENGTH, &resource));
    NAPI_CALL_RETURN_VOID(env, napi_create_async_work(
        env, nullptr, resource, [](napi_env env, void *data) {},
        [](napi_env env, napi_status status, void *data) {
            GeolocationPermissionsResultCallbackInfo *asyncCallbackInfo =
                reinterpret_cast<GeolocationPermissionsResultCallbackInfo *>(data);
            if (!asyncCallbackInfo) {
                return;
            }
            napi_value args[INTEGER_TWO] = { 0 };
            if (asyncCallbackInfo->errCode == NWebError::INVALID_ORIGIN) {
                napi_value error;
                napi_create_object(env, &error);
                napi_value errorCode;
                napi_create_int32(env, NWebError::INVALID_ORIGIN, &errorCode);
                napi_value errorMessage;
                napi_create_string_utf8(env, GetErrMsgByErrCode(NWebError::INVALID_ORIGIN).c_str(), NAPI_AUTO_LENGTH,
                    &errorMessage);
                napi_set_named_property(env, error, "code", errorCode);
                napi_set_named_property(env, error, "message", errorMessage);
                args[INTEGER_ZERO] = error;
            } else {
                NAPI_CALL_RETURN_VOID(env, napi_get_null(env, &args[INTEGER_ZERO]));
                NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, asyncCallbackInfo->result, &args[INTEGER_ONE]));
            }
            if (asyncCallbackInfo->deferred) {
                if (asyncCallbackInfo->errCode == NWebError::INVALID_ORIGIN) {
                    NAPI_CALL_RETURN_VOID(env,
                        napi_reject_deferred(env, asyncCallbackInfo->deferred, args[INTEGER_ZERO]));
                } else {
                    NAPI_CALL_RETURN_VOID(env,
                        napi_resolve_deferred(env, asyncCallbackInfo->deferred, args[INTEGER_ONE]));
                }
            } else {
                napi_value callback = nullptr;
                napi_value callbackResult = nullptr;
                NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, asyncCallbackInfo->callback, &callback));
                NAPI_CALL_RETURN_VOID(env,
                    napi_call_function(env, nullptr, callback, sizeof(args) / sizeof(args[0]), args, &callbackResult));
            }
            GeolocationPermissions::EraseCallbackInfo(asyncCallbackInfo,
                GeolocationPermissionsTaskType::GET_ACCESSIBLE_GEOLOCATION);
        },
        reinterpret_cast<void *>(asyncCallbackInfo), &asyncCallbackInfo->asyncWork));
    GeolocationPermissions::InsertCallbackInfo(callbackInfo,
        GeolocationPermissionsTaskType::GET_ACCESSIBLE_GEOLOCATION);
}

void NapiGeolocationPermissions::CreateGetStoredGeolocationAsyncWork(napi_env env,
    const std::shared_ptr<GeolocationPermissionsResultCallbackInfo> &callbackInfo)
{
    napi_value resource = nullptr;
    auto asyncCallbackInfo = callbackInfo.get();
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "JsGetStoredGeolocation", NAPI_AUTO_LENGTH, &resource));
    NAPI_CALL_RETURN_VOID(env, napi_create_async_work(env, nullptr, resource,
        [](napi_env env, void *data) {},
        [](napi_env env, napi_status status, void *data) {
            GeolocationPermissionsResultCallbackInfo *asyncCallbackInfo =
                reinterpret_cast<GeolocationPermissionsResultCallbackInfo *>(data);
            if (!asyncCallbackInfo) {
                return;
            }
            napi_value args[INTEGER_TWO] = { 0 };
            if (asyncCallbackInfo->originsArray.empty()) {
                NAPI_CALL_RETURN_VOID(env, napi_get_null(env, &args[INTEGER_ZERO]));
            }
            NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &args[INTEGER_ONE]));
            for (size_t i = 0; i < asyncCallbackInfo->originsArray.size(); ++i) {
                napi_value jsString = nullptr;
                NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, asyncCallbackInfo->originsArray[i].c_str(),
                    asyncCallbackInfo->originsArray[i].length(), &jsString));
                NAPI_CALL_RETURN_VOID(env, napi_set_element(env, args[INTEGER_ONE], i, jsString));
            }
            if (asyncCallbackInfo->deferred) {
                NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, asyncCallbackInfo->deferred, args[INTEGER_ONE]));
            } else {
                napi_value callback = nullptr;
                napi_value callbackResult = nullptr;
                NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, asyncCallbackInfo->callback, &callback));
                NAPI_CALL_RETURN_VOID(env,
                    napi_call_function(env, nullptr, callback, sizeof(args) / sizeof(args[0]), args, &callbackResult));
            }
            GeolocationPermissions::EraseCallbackInfo(asyncCallbackInfo,
                GeolocationPermissionsTaskType::GET_STORED_GEOLOCATION);
        },
        reinterpret_cast<void *>(asyncCallbackInfo), &asyncCallbackInfo->asyncWork));
    GeolocationPermissions::InsertCallbackInfo(callbackInfo, GeolocationPermissionsTaskType::GET_STORED_GEOLOCATION);
}

napi_value NapiWebStorage::DeleteAllData(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_value retValue = nullptr;

    size_t argc = 1;
    size_t argcForOld = 0;
    napi_value argv[1] = { 0 };
    napi_get_cb_info(env, info, &argc, argv, &retValue, nullptr);
    if (argc != 1 && argc != argcForOld) {
        return nullptr;
    }

    bool incognitoMode = false;
    if (argc == 1) {
        napi_get_value_bool(env, argv[0], &incognitoMode);
    }

    WebStorage::DeleteAllData();
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiWebStorage::DeleteOrigin(napi_env env, napi_callback_info info)
{
    napi_value retValue = nullptr;
    size_t argc = 1;
    napi_value argv = nullptr;
    napi_value result = nullptr;
    napi_get_cb_info(env, info, &argc, &argv, &retValue, nullptr);
    if (argc != 1) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR, 
            FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_ONE, "one"));
        return nullptr;
    }

    size_t bufferSize = 0;
    napi_valuetype valueType = napi_null;
    napi_typeof(env, argv, &valueType);
    if (valueType != napi_string) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR, 
            FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "origin", "string"));
        return nullptr;
    }

    napi_get_value_string_utf8(env, argv, nullptr, 0, &bufferSize);
    if (bufferSize >= MAX_WEB_STRING_LENGTH) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR, 
            "BusinessError 401: Parameter error. The length of 'origin' must less than 40960.");
        return nullptr;
    }

    char stringValue[bufferSize + 1];
    size_t jsStringLength = 0;
    napi_get_value_string_utf8(env, argv, stringValue, bufferSize + 1, &jsStringLength);
    if (jsStringLength != bufferSize) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR, 
            "BusinessError 401: Parameter error. The length of 'origin' obtained twice are different");
        return nullptr;
    }

    if (strcmp(stringValue, rawfile) == 0){     
        strncpy(stringValue, replacement, sizeof(stringValue));
        stringValue[sizeof(stringValue) - 1] = '\0';
    }
    
    std::string origin(stringValue);
    if (!std::regex_match(origin, reg)) {
        BusinessError::ThrowErrorByErrcode(env, INVALID_ORIGIN);
        return nullptr;
    }

    WebStorage::DeleteOrigin(origin);
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiWebDownloadDelegate::JS_DownloadBeforeStart(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebDownloadDelegate* webDownloadDelegate = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&webDownloadDelegate);
    if (!webDownloadDelegate) {
        LOGE("[DOWNLOAD] webDownloadDelegate is null");
        return thisVar;
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);

    webDownloadDelegate->PutDownloadBeforeStart(env, argv[0]);
    return thisVar;
}

napi_value NapiWebDownloadDelegate::JS_DownloadDidUpdate(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebDownloadDelegate* webDownloadDelegate = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&webDownloadDelegate);
    if (!webDownloadDelegate) {
        LOGE("[DOWNLOAD] webDownloadDelegate is null");
        return thisVar;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);

    webDownloadDelegate->PutDownloadDidUpdate(env, argv[0]);
    return thisVar;
}

napi_value NapiWebDownloadDelegate::JS_DownloadDidFinish(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebDownloadDelegate* webDownloadDelegate = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&webDownloadDelegate);
    if (!webDownloadDelegate) {
        LOGE("[DOWNLOAD] webDownloadDelegate is null");
        return thisVar;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);

    webDownloadDelegate->PutDownloadDidFinish(env, argv[0]);
    return thisVar;
}

napi_value NapiWebDownloadDelegate::JS_DownloadDidFail(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebDownloadDelegate* webDownloadDelegate = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&webDownloadDelegate);
    if (!webDownloadDelegate) {
        LOGE("[DOWNLOAD] webDownloadDelegate is null");
        return thisVar;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);

    webDownloadDelegate->PutDownloadDidFail(env, argv[0]);
    return thisVar;
}

napi_value NapiWebDownloadDelegate::JS_Constructor(napi_env env, napi_callback_info cbinfo)
{
    WebDownloadDelegate* delegate = new (std::nothrow) WebDownloadDelegate(env);
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, nullptr);
    napi_status status = napi_wrap(
        env, thisVar, delegate,
        [](napi_env, void* data, void*) {
            if (data) {
                WebDownloadDelegate* delegate = static_cast<WebDownloadDelegate*>(data);
                if (delegate) {
                    delete delegate;
                    delegate = nullptr;
                }
            }
        },
        nullptr, nullptr);
    if (status != napi_ok) {
        if (delegate) {
            delete delegate;
            delegate = nullptr;
        }
        return nullptr;
    }

    return thisVar;
}

napi_value NapiWebDownloadDelegate::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("onBeforeDownload", JS_DownloadBeforeStart),
        DECLARE_NAPI_FUNCTION("onDownloadUpdated", JS_DownloadDidUpdate),
        DECLARE_NAPI_FUNCTION("onDownloadFinish", JS_DownloadDidFinish),
        DECLARE_NAPI_FUNCTION("onDownloadFailed", JS_DownloadDidFail),
    };
    const std::string WEB_DOWNLOAD_DELEGATE = "WebDownloadDelegate";
    napi_value webDownloadDelegateClass = nullptr;
    napi_define_class(env, WEB_DOWNLOAD_DELEGATE.c_str(), WEB_DOWNLOAD_DELEGATE.length(), JS_Constructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &webDownloadDelegateClass);
    napi_set_named_property(env, exports, WEB_DOWNLOAD_DELEGATE.c_str(), webDownloadDelegateClass);

    return exports;
}

napi_value NapiWebDownloadManager::JS_SetDownloadDelegate(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    napi_value obj = argv[0];
    napi_valuetype objType = napi_undefined;
    napi_typeof(env, argv[0], &objType);

    WebDownloadDelegate *delegate = nullptr;
    napi_unwrap(env, obj, (void**)&delegate);
    if (!delegate) {
        LOGE("[DOWNLOAD] NapiWebDownloadManager::JS_SetDownloadDelegate delegate is null");
        return nullptr;
    }
    napi_create_reference(env, obj, 1, &delegate->delegate_);
    WebDownloadManager::SetDownloadDelegate(delegate);
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiWebDownloadManager::JS_Constructor(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    return thisVar;
}

napi_value NapiWebDownloadManager::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        { "setDownloadDelegate", nullptr, JS_SetDownloadDelegate, nullptr, nullptr, nullptr, napi_static, nullptr },
    };
    napi_value webDownloadManagerClass = nullptr;
    napi_define_class(env, WEB_DOWNLOAD_MANAGER.c_str(), WEB_DOWNLOAD_MANAGER.length(), JS_Constructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &webDownloadManagerClass);
    napi_set_named_property(env, exports, WEB_DOWNLOAD_MANAGER.c_str(), webDownloadManagerClass);

    return exports;
}

napi_value NapiWebDownloadItem::JS_GetMethod(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebDownloadItem* webDownloadItem = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&webDownloadItem);
    if (!webDownloadItem) {
        LOGE("[DOWNLOAD] NapiWebDownloadItem::JS_GetMethod webDownloadItem is null");
        return nullptr;
    }

    napi_value methodValue;
    napi_status status = napi_create_string_utf8(env, webDownloadItem->method.c_str(), NAPI_AUTO_LENGTH, &methodValue);
    if (status != napi_ok) {
        LOGE("[DOWNLOAD] NapiWebDownloadItem::JS_GetMethod failed");
        return nullptr;
    }
    return methodValue;
}

napi_value NapiWebDownloadItem::JS_GetMimeType(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebDownloadItem* webDownloadItem = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&webDownloadItem);
    if (!webDownloadItem) {
        LOGE("[DOWNLOAD] NapiWebDownloadItem::JS_GetMimeType webDownloadItem is null");
        return nullptr;
    }

    napi_value mimeTypeValue;
    napi_status status =
        napi_create_string_utf8(env, webDownloadItem->mimeType.c_str(), NAPI_AUTO_LENGTH, &mimeTypeValue);
    if (status != napi_ok) {
        LOGE("[DOWNLOAD] NapiWebDownloadItem::JS_GetMimeType failed");
        return nullptr;
    }
    return mimeTypeValue;
}

napi_value NapiWebDownloadItem::JS_GetUrl(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebDownloadItem* webDownloadItem = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&webDownloadItem);
    if (!webDownloadItem) {
        LOGE("[DOWNLOAD] NapiWebDownloadItem::JS_GetUrl webDownloadItem is null");
        return nullptr;
    }

    napi_value urlValue;
    napi_status status = napi_create_string_utf8(env, webDownloadItem->url.c_str(), NAPI_AUTO_LENGTH, &urlValue);
    if (status != napi_ok) {
        LOGE("[DOWNLOAD] NapiWebDownloadItem::JS_GetUrl failed");
        return nullptr;
    }
    return urlValue;
}

napi_value NapiWebDownloadItem::JS_GetSuggestedFileName(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebDownloadItem* webDownloadItem = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&webDownloadItem);
    if (!webDownloadItem) {
        LOGE("[DOWNLOAD] NapiWebDownloadItem::JS_GetSuggestedFileName webDownloadItem is null");
        return nullptr;
    }

    napi_value fileNameValue;
    napi_status status =
        napi_create_string_utf8(env, webDownloadItem->suggestedFileName.c_str(), NAPI_AUTO_LENGTH, &fileNameValue);
    if (status != napi_ok) {
        LOGE("[DOWNLOAD] NapiWebDownloadItem::JS_GetSuggestedFileName failed");
        return nullptr;
    }
    return fileNameValue;
}

napi_value NapiWebDownloadItem::JS_GetCurrentSpeed(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebDownloadItem* webDownloadItem = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&webDownloadItem);
    if (!webDownloadItem) {
        LOGE("[DOWNLOAD] NapiWebDownloadItem::JS_GetCurrentSpeed webDownloadItem is null");
        return nullptr;
    }

    napi_value currentSpeed;
    napi_status status = napi_create_int64(env, webDownloadItem->currentSpeed, &currentSpeed);
    if (status != napi_ok) {
        LOGE("[DOWNLOAD] NapiWebDownloadItem::JS_GetCurrentSpeed failed");
        return nullptr;
    }
    return currentSpeed;
}

napi_value NapiWebDownloadItem::JS_GetPercentComplete(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebDownloadItem* webDownloadItem = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&webDownloadItem);
    if (!webDownloadItem) {
        LOGE("[DOWNLOAD] NapiWebDownloadItem::JS_GetPercentComplete webDownloadItem is null");
        return nullptr;
    }

    napi_value percentComplete;
    napi_status status = napi_create_int64(env, webDownloadItem->percentComplete, &percentComplete);
    if (status != napi_ok) {
        LOGE("[DOWNLOAD] NapiWebDownloadItem::JS_GetPercentComplete failed");
        return nullptr;
    }
    return percentComplete;
}

napi_value NapiWebDownloadItem::JS_GetTotalBytes(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebDownloadItem* webDownloadItem = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&webDownloadItem);
    if (!webDownloadItem) {
        LOGE("[DOWNLOAD] NapiWebDownloadItem::JS_GetTotalBytes webDownloadItem is null");
        return nullptr;
    }

    napi_value totalBytes;
    napi_status status = napi_create_int64(env, webDownloadItem->totalBytes, &totalBytes);
    if (status != napi_ok) {
        LOGE("[DOWNLOAD] NapiWebDownloadItem::JS_GetTotalBytes failed");
        return nullptr;
    }
    return totalBytes;
}

napi_value NapiWebDownloadItem::JS_GetReceivedBytes(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebDownloadItem* webDownloadItem = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&webDownloadItem);
    if (!webDownloadItem) {
        LOGE("[DOWNLOAD] NapiWebDownloadItem::JS_GetReceivedBytes webDownloadItem is null");
        return nullptr;
    }

    napi_value receivedBytes;
    napi_status status = napi_create_int64(env, webDownloadItem->receivedBytes, &receivedBytes);
    if (status != napi_ok) {
        LOGE("[DOWNLOAD] NapiWebDownloadItem::JS_GetReceivedBytes failed");
        return nullptr;
    }
    return receivedBytes;
}

napi_value NapiWebDownloadItem::JS_GetState(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebDownloadItem* webDownloadItem = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);
    napi_unwrap(env, thisVar, (void**)&webDownloadItem);

    if (!webDownloadItem) {
        LOGE("[DOWNLOAD] unwrap webDownloadItem failed");
        return nullptr;
    }

    napi_value state;
    napi_status status = napi_create_int32(env, static_cast<int32_t>(webDownloadItem->state), &state);
    if (status != napi_ok) {
        LOGE("napi_create_int32 failed.");
        return nullptr;
    }
    return state;
}

napi_value NapiWebDownloadItem::JS_GetLastErrorCode(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebDownloadItem* webDownloadItem = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&webDownloadItem);

    if (!webDownloadItem) {
        LOGE("[DOWNLOAD] unwrap webDownloadItem failed");
        return nullptr;
    }

    napi_value errorCode;
    napi_status status = napi_create_int32(env, static_cast<int32_t>(webDownloadItem->lastErrorCode), &errorCode);
    if (status != napi_ok) {
        LOGE("napi_create_int32 failed.");
        return nullptr;
    }
    return errorCode;
}

napi_value NapiWebDownloadItem::JS_GetGuid(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebDownloadItem* webDownloadItem = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&webDownloadItem);

    if (!webDownloadItem) {
        LOGE("[DOWNLOAD] unwrap webDownloadItem failed");
        return nullptr;
    }

    napi_value guid;
    napi_status status = napi_create_string_utf8(env, webDownloadItem->guid.c_str(), NAPI_AUTO_LENGTH, &guid);
    if (status != napi_ok) {
        LOGE("[DOWNLOAD] NapiWebDownloadItem::JS_GetGuid failed");
        return nullptr;
    }
    return guid;
}

napi_value NapiWebDownloadItem::JS_GetFullPath(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebDownloadItem* webDownloadItem = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&webDownloadItem);

    if (!webDownloadItem) {
        LOGE("[DOWNLOAD] unwrap webDownloadItem failed");
        return nullptr;
    }

    napi_value fullPath;
    napi_status status = napi_create_string_utf8(env, webDownloadItem->fullPath.c_str(), NAPI_AUTO_LENGTH, &fullPath);
    if (status != napi_ok) {
        LOGE("[DOWNLOAD] NapiWebDownloadItem::JS_GetFullPath failed");
        return nullptr;
    }
    return fullPath;
}

napi_value NapiWebDownloadItem::JS_Continue(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, &data);

    WebDownloadItem* webDownloadItem = nullptr;
    napi_unwrap(env, thisVar, (void**)&webDownloadItem);

    if (!webDownloadItem) {
        LOGE("[DOWNLOAD] unwrap webDownloadItem failed");
        return nullptr;
    }
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiWebDownloadItem::JS_Cancel(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, &data);

    WebDownloadItem* webDownloadItem = nullptr;
    napi_unwrap(env, thisVar, (void**)&webDownloadItem);

    if (!webDownloadItem) {
        LOGE("[DOWNLOAD] unwrap webDownloadItem failed");
        return nullptr;
    }
    if (webDownloadItem->downloadCancelCallback) {
        webDownloadItem->downloadCancelCallback();
    }
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiWebDownloadItem::JS_Pause(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, &data);

    WebDownloadItem* webDownloadItem = nullptr;
    napi_unwrap(env, thisVar, (void**)&webDownloadItem);

    if (!webDownloadItem) {
        LOGE("[DOWNLOAD] unwrap webDownloadItem failed");
        return nullptr;
    }
    if (!webDownloadItem->downloadPauseCallback || webDownloadItem->downloadPauseCallback() != NO_ERROR) {
        BusinessError::ThrowErrorByErrcode(env, DOWNLOAD_NOT_START);
    }
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiWebDownloadItem::JS_Resume(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, &data);

    WebDownloadItem* webDownloadItem = nullptr;
    napi_unwrap(env, thisVar, (void**)&webDownloadItem);

    if (!webDownloadItem) {
        LOGE("[DOWNLOAD] unwrap webDownloadItem failed");
        return nullptr;
    }

    if (webDownloadItem->state != PluginWebDownloadItemState::PAUSED) {
        BusinessError::ThrowErrorByErrcode(env, DOWNLOAD_NOT_PAUSED);
        return nullptr;
    }

    if (!webDownloadItem->downloadResumeCallback || webDownloadItem->downloadResumeCallback() != NO_ERROR) {
        BusinessError::ThrowErrorByErrcode(env, DOWNLOAD_NOT_PAUSED);
    }
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiWebDownloadItem::JS_Constructor(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, &data);

    WebDownloadItem* webDownloadItem = new (std::nothrow) WebDownloadItem(env);

    napi_status status = napi_wrap(
        env, thisVar, webDownloadItem,
        [](napi_env, void* data, void*) {
            if (data) {
                WebDownloadItem* webDownloadItem = static_cast<WebDownloadItem*>(data);
                if (webDownloadItem) {
                    delete webDownloadItem;
                    webDownloadItem = nullptr;
                }
            }

        },
        nullptr, nullptr);
    if (status != napi_ok) {
        if (webDownloadItem) {
            delete webDownloadItem;
            webDownloadItem = nullptr;
        }
        return nullptr;
    }
    return thisVar;
}

napi_value NapiWebDownloadItem::JS_Start(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_valuetype value_type = napi_undefined;
    napi_typeof(env, argv[0], &value_type);

    size_t pathLen = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &pathLen);
    WebDownloadItem* webDownloadItem = nullptr;
    napi_unwrap(env, thisVar, (void**)&webDownloadItem);

    if (!webDownloadItem) {
        LOGE("[DOWNLOAD] unwrap webDownloadItem failed");
        return nullptr;
    }

    char stringValue[pathLen + 1];
    size_t jsStringLength = 0;
    napi_get_value_string_utf8(env, argv[0], stringValue, pathLen + 1, &jsStringLength);
    if (jsStringLength != pathLen) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
                "BusinessError: 401. Parameter error. The type of 'downloadPath' must be a valid path string.");
        return nullptr;
    }
    webDownloadItem->downloadPath = std::string(stringValue);
    webDownloadItem->beforeCallback(webDownloadItem->downloadPath);
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiWebDownloadItem::Init(napi_env env, napi_value exports)
{
    ExportWebDownloadItemClass(env, &exports);
    ExportWebDownloadStateEnum(env, &exports);
    ExportWebDownloadErrorCodeEnum(env, &exports);
    return exports;
}

void NapiWebDownloadItem::ExportWebDownloadItemClass(napi_env env, napi_value* exportsPointer)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getCurrentSpeed", JS_GetCurrentSpeed),
        DECLARE_NAPI_FUNCTION("getPercentComplete", JS_GetPercentComplete),
        DECLARE_NAPI_FUNCTION("getTotalBytes", JS_GetTotalBytes),
        DECLARE_NAPI_FUNCTION("getState", JS_GetState),
        DECLARE_NAPI_FUNCTION("getLastErrorCode", JS_GetLastErrorCode),
        DECLARE_NAPI_FUNCTION("getMethod", JS_GetMethod),
        DECLARE_NAPI_FUNCTION("getMimeType", JS_GetMimeType),
        DECLARE_NAPI_FUNCTION("getUrl", JS_GetUrl),
        DECLARE_NAPI_FUNCTION("getSuggestedFileName", JS_GetSuggestedFileName),
        DECLARE_NAPI_FUNCTION("start", JS_Start),
        DECLARE_NAPI_FUNCTION("continue", JS_Continue),
        DECLARE_NAPI_FUNCTION("pause", JS_Pause),
        DECLARE_NAPI_FUNCTION("cancel", JS_Cancel),
        DECLARE_NAPI_FUNCTION("resume", JS_Resume),
        DECLARE_NAPI_FUNCTION("getReceivedBytes", JS_GetReceivedBytes),
        DECLARE_NAPI_FUNCTION("getFullPath", JS_GetFullPath),
        DECLARE_NAPI_FUNCTION("getGuid", JS_GetGuid),
    };
    napi_value webDownloadClass = nullptr;
    napi_define_class(env, WEB_DOWNLOAD_ITEMT.c_str(), WEB_DOWNLOAD_ITEMT.length(), JS_Constructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &webDownloadClass);
    napi_set_named_property(env, *exportsPointer, WEB_DOWNLOAD_ITEMT.c_str(), webDownloadClass);
}

void NapiWebDownloadItem::ExportWebDownloadStateEnum(napi_env env, napi_value* exportsPointer)
{
    napi_value webDownloadStateTypeEnum = nullptr;
    napi_property_descriptor webDownloadStateProperties[] = {
        DECLARE_NAPI_STATIC_PROPERTY(
            "IN_PROGRESS",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(PluginWebDownloadItemState::IN_PROGRESS))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "COMPLETED",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(PluginWebDownloadItemState::COMPLETE))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "CANCELED",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(PluginWebDownloadItemState::CANCELED))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "INTERRUPTED",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(PluginWebDownloadItemState::INTERRUPTED))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "PENDING",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(PluginWebDownloadItemState::PENDING))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "PAUSED",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(PluginWebDownloadItemState::PAUSED))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "UNKNOWN", NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(
                PluginWebDownloadItemState::MAX_DOWNLOAD_STATE))),
    };
    napi_define_class(env, WEB_DOWNLOAD_STATE_ENUM_NAME.c_str(), WEB_DOWNLOAD_STATE_ENUM_NAME.length(),
        NapiParseUtils::CreateEnumConstructor, nullptr, sizeof(webDownloadStateProperties) / sizeof(webDownloadStateProperties[0]),
        webDownloadStateProperties, &webDownloadStateTypeEnum);
    napi_set_named_property(env, *exportsPointer, WEB_DOWNLOAD_STATE_ENUM_NAME.c_str(), webDownloadStateTypeEnum);
}

void NapiWebDownloadItem::ExportWebDownloadErrorCodeEnum(napi_env env, napi_value* exportsPointer)
{
    napi_value webDownloadErrorCodeEnum = nullptr;
    napi_property_descriptor webDownloadErrorCodeEnumProperties[] = {
        DECLARE_NAPI_STATIC_PROPERTY(
            "ERROR_UNKNOWN",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_NONE))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "FILE_FAILED",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_FILE_FAILED))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "FILE_ACCESS_DENIED",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_FILE_ACCESS_DENIED))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "FILE_NO_SPACE",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_FILE_NO_SPACE))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "FILE_NAME_TOO_LONG",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_FILE_NAME_TOO_LONG))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "FILE_TOO_LARGE",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_FILE_TOO_LARGE))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "FILE_VIRUS_INFECTED",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_FILE_VIRUS_INFECTED))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "FILE_TRANSIENT_ERROR",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_FILE_TRANSIENT_ERROR))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "FILE_BLOCKED",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_FILE_BLOCKED))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "FILE_SECURITY_CHECK_FAILED",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_FILE_SECURITY_CHECK_FAILED))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "FILE_TOO_SHORT",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_FILE_TOO_SHORT))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "FILE_HASH_MISMATCH",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_FILE_HASH_MISMATCH))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "FILE_SAME_AS_SOURCE",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_FILE_SAME_AS_SOURCE))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "NETWORK_FAILED",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_NETWORK_FAILED))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "NETWORK_TIMEOUT",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_NETWORK_TIMEOUT))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "NETWORK_DISCONNECTED",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_NETWORK_DISCONNECTED))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "NETWORK_SERVER_DOWN",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_NETWORK_SERVER_DOWN))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "NETWORK_INVALID_REQUEST",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_NETWORK_INVALID_REQUEST))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "SERVER_FAILED",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_SERVER_FAILED))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "SERVER_NO_RANGE",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_SERVER_NO_RANGE))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "SERVER_BAD_CONTENT",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_SERVER_BAD_CONTENT))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "SERVER_UNAUTHORIZED",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_SERVER_UNAUTHORIZED))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "SERVER_CERT_PROBLEM",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_SERVER_CERT_PROBLEM))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "SERVER_FORBIDDEN",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_SERVER_FORBIDDEN))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "SERVER_UNREACHABLE",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_SERVER_UNREACHABLE))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "SERVER_CONTENT_LENGTH_MISMATCH",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_SERVER_CONTENT_LENGTH_MISMATCH))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "SERVER_CROSS_ORIGIN_REDIRECT",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_SERVER_CROSS_ORIGIN_REDIRECT))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "USER_CANCELED",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_USER_CANCELED))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "USER_SHUTDOWN",
            NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_USER_SHUTDOWN))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "CRASH", NapiParseUtils::ToInt32Value(
                env, static_cast<int32_t>(download::DOWNLOAD_INTERRUPT_REASON_CRASH))),
    };

    napi_define_class(env, WEB_DOWNLOAD_ERROR_CODE_ENUM_NAME.c_str(), WEB_DOWNLOAD_ERROR_CODE_ENUM_NAME.length(),
        NapiParseUtils::CreateEnumConstructor, nullptr,
        sizeof(webDownloadErrorCodeEnumProperties) / sizeof(webDownloadErrorCodeEnumProperties[0]),
        webDownloadErrorCodeEnumProperties, &webDownloadErrorCodeEnum);
    napi_set_named_property(env, *exportsPointer, WEB_DOWNLOAD_ERROR_CODE_ENUM_NAME.c_str(), webDownloadErrorCodeEnum);
}

napi_status NapiWebDownloadItem::DefineProperties(napi_env env, napi_value* object)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getCurrentSpeed", JS_GetCurrentSpeed),
        DECLARE_NAPI_FUNCTION("getPercentComplete", JS_GetPercentComplete),
        DECLARE_NAPI_FUNCTION("getTotalBytes", JS_GetTotalBytes),
        DECLARE_NAPI_FUNCTION("getState", JS_GetState),
        DECLARE_NAPI_FUNCTION("getLastErrorCode", JS_GetLastErrorCode),
        DECLARE_NAPI_FUNCTION("getMethod", JS_GetMethod),
        DECLARE_NAPI_FUNCTION("getMimeType", JS_GetMimeType),
        DECLARE_NAPI_FUNCTION("getUrl", JS_GetUrl),
        DECLARE_NAPI_FUNCTION("getSuggestedFileName", JS_GetSuggestedFileName),
        DECLARE_NAPI_FUNCTION("continue", JS_Continue),
        DECLARE_NAPI_FUNCTION("start", JS_Start),
        DECLARE_NAPI_FUNCTION("pause", JS_Pause),
        DECLARE_NAPI_FUNCTION("cancel", JS_Cancel),
        DECLARE_NAPI_FUNCTION("resume", JS_Resume),
        DECLARE_NAPI_FUNCTION("getReceivedBytes", JS_GetReceivedBytes),
        DECLARE_NAPI_FUNCTION("getFullPath", JS_GetFullPath),
        DECLARE_NAPI_FUNCTION("getGuid", JS_GetGuid),
    };
    return napi_define_properties(env, *object, sizeof(properties) / sizeof(properties[0]), properties);
}

napi_value NapiWebSchemeHandler::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("onRequestStart", JS_RequestStart),
        DECLARE_NAPI_FUNCTION("onRequestStop", JS_RequestStop),
    };
    napi_value webSchemeHandler = nullptr;
    napi_define_class(env, WEB_SCHEME_HANDLER.c_str(), WEB_SCHEME_HANDLER.length(),
        JS_Constructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &webSchemeHandler);
    napi_set_named_property(env, exports, WEB_SCHEME_HANDLER.c_str(),
        webSchemeHandler);
    return exports;
}

napi_value NapiWebSchemeHandler::JS_Constructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, &data);

    WebSchemeHandler *handler = new WebSchemeHandler(env);

    napi_wrap(
        env, thisVar, handler,
        [](napi_env /* env */, void *data, void * /* hint */) {
            WebSchemeHandler *handler = (WebSchemeHandler *)data;
            delete handler;
        },
        nullptr, nullptr);

    return thisVar;
}

napi_value NapiWebSchemeHandler::JS_RequestStart(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    WebSchemeHandler *handler = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void **)&handler);
    if (!handler) {
        LOGE("webSchemeHandler is null");
        return thisVar;
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);

    if (argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_ONE, "one"));
        return nullptr;
    }
    if (valueType != napi_function) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "callback", "function"));
        return nullptr;
    }

    handler->PutRequestStart(env, argv[0]);
    return thisVar;
}

napi_value NapiWebSchemeHandler::JS_RequestStop(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    WebSchemeHandler *handler = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (argc != INTEGER_ONE) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }

    napi_unwrap(env, thisVar, (void **)&handler);
    if (!handler) {
        LOGE("webSchemeHandler is null");
        return thisVar;
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);

    if (valueType != napi_function) {
        BusinessError::ThrowErrorByErrcode(env, PARAM_CHECK_ERROR);
        return nullptr;
    }

    handler->PutRequestStop(env, argv[0]);
    return thisVar;
}
} // namespace OHOS::Plugin
