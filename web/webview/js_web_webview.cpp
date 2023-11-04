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
#include "inner_api/plugin_utils_napi.h"
#include "plugin_utils.h"
#include "webview_controller.h"
#ifdef ANDROID_PLATFORM
#include "android/java/jni/webview_controller_android.h"
#include "android/java/jni/webview_controller_jni.h"
#include "android/java/jni/web_data_base_android.h"
#include "android/java/jni/web_data_base_jni.h"
#endif
#ifdef IOS_PLATFORM
#include "ios/webview_controller_ios.h"
#include "ios/web_data_base_ios.h"
#endif
#include "napi_parse_utils.h"
#include "business_error.h"

using namespace OHOS::NWebError;

namespace OHOS::Plugin {
namespace {
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

napi_value NapiWebviewController::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("loadUrl", NapiWebviewController::LoadUrl),
        DECLARE_NAPI_FUNCTION("setWebId", NapiWebviewController::SetWebId),
        DECLARE_NAPI_FUNCTION("innerSetHapPath", NapiWebviewController::InnerSetHapPath),
    };
    napi_value constructor = nullptr;
    napi_define_class(env, WEBVIEW_CONTROLLER_CLASS_NAME.c_str(), WEBVIEW_CONTROLLER_CLASS_NAME.length(),
        NapiWebviewController::JsConstructor, nullptr, sizeof(properties) / sizeof(properties[0]),
        properties, &constructor);
    NAPI_ASSERT(env, constructor != nullptr, "define js class WebviewController failed");
    napi_status status = napi_set_named_property(env, exports, "WebviewController", constructor);
    NAPI_ASSERT(env, status == napi_ok, "set property WebviewController failed");
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
    const char dataBaseClassName[] = "ohos.ace.plugin.webviewplugin.WebDataBasePlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&WebDataBaseJni::Register, dataBaseClassName);
}
#endif

extern "C" __attribute__((constructor)) void WebWebviewRegister()
{
#ifdef ANDROID_PLATFORM
    WebWebviewJniRegister();
#endif
    napi_module_register(&webWebviewModule);
}
} // namespace OHOS::Plugin
