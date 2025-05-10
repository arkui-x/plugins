/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include <regex>
#include "webview_controller.h"
#ifdef ANDROID_PLATFORM
#include "android/java/jni/webview_controller_android.h"
#endif
#ifdef IOS_PLATFORM
#include "ios/webview_controller_ios.h"
#endif

namespace OHOS::Plugin {
static constexpr const int MAX_STR_LENGTH = 65536;
thread_local std::mutex WebviewController::objMapMutex;
void CreateArrayNapi(napi_env env, const std::shared_ptr<Ace::WebJSValue>& arg, napi_value napi_arg);
void CreateDictionaryNapi(napi_env env, const std::shared_ptr<Ace::WebJSValue>& arg, napi_value napi_arg);
std::shared_ptr<Ace::WebJSValue> GetNapiValue(napi_env env, napi_value ret);
thread_local std::vector<std::shared_ptr<AsyncEvaluteJSResultCallbackInfo>> WebviewController::asyncCallbackInfoContainer_;
thread_local std::vector<AsyncJavaScriptExtEvaluteJSResultCallbackInfo*>
    WebviewController::asyncCallbackJavaScriptExtInfoContainer_;
thread_local std::map<std::string, RegisterJavaScriptProxyParam> WebviewController::objMap_;

std::vector<std::string> parseStringVector(const std::string& str) {
    std::vector<std::string> result;
    std::smatch match;
    std::string s = str;
    std::regex re("\"([^\"]*)\"");
    while (std::regex_search(s, match, re)) {
        result.push_back(match[1].str());
        s = match.suffix().str();
    }
    return result;
}

std::vector<bool> parseBooleanVector(const std::string& str) {
    std::vector<bool> result;
    std::smatch match;
    std::string s = str;
    std::regex re("(true|false)");
    while (std::regex_search(s, match, re)) {
        result.push_back(match[1].str() == "true");
        s = match.suffix().str();
    }

    return result;
}

std::vector<int64_t> parseInt64Vector(const std::string& str) {
    std::vector<int64_t> result;
    std::smatch match;
    std::string s = str;
    std::regex re("\\d+");
    while (std::regex_search(s, match, re)) {
        result.push_back(std::stoll(match[0].str()));
        s = match.suffix().str();
    }

    return result;
}

std::vector<double> parseDoubleVector(const std::string& str) {
    std::vector<double> result;
    std::smatch match;
    std::string s = str;
    std::regex re("[-+]?[0-9]*\\.?[0-9]+");
    while (std::regex_search(s, match, re)) {
        result.push_back(std::stod(match[0].str()));
        s = match.suffix().str();
    }

    return result;
}

std::vector<napi_value> CreateNapiArgv(napi_env env, const std::vector<std::shared_ptr<Ace::WebJSValue>>& argsList)
{
    std::vector<napi_value> argv;
    for (const auto& arg: argsList) {
        napi_value napiArg = nullptr;
        if (arg == nullptr) {
            LOGE("CreateNapiArgv arg is nullptr");
            continue;
        }
        Ace::WebJSValue::Type type = arg->GetType();
        switch (type) {
            case Ace::WebJSValue::Type::STRING:
                napi_create_string_utf8(env, arg->GetString().c_str(), NAPI_AUTO_LENGTH, &napiArg);
                break;
            case Ace::WebJSValue::Type::INTEGER:
                napi_create_int32(env, arg->GetInt(), &napiArg);
                break;
            case Ace::WebJSValue::Type::DOUBLE:
                napi_create_double(env, arg->GetDouble(), &napiArg);
                break;
            case Ace::WebJSValue::Type::BOOLEAN:
                napi_get_boolean(env, arg->GetBoolean(), &napiArg);
                break;
            case Ace::WebJSValue::Type::LIST:
                napi_create_array(env, &napiArg);
                CreateArrayNapi(env, arg, napiArg);
                break;
            case Ace::WebJSValue::Type::DICTIONARY:
                napi_create_object(env, &napiArg);
                CreateDictionaryNapi(env, arg, napiArg);
                break;
            default:
                break;
        }
        argv.push_back(napiArg);
    }
    return argv;
}

void CreateArrayNapi(napi_env env, const std::shared_ptr<Ace::WebJSValue>& arg, napi_value napi_arg)
{
    CHECK_NULL_VOID(arg);
    auto list = arg->GetListValue();
    napi_value element = nullptr;
    for (size_t i = 0; i < list.size(); ++i) {
        element = nullptr;
        Ace::WebJSValue::Type elementType = list[i].GetType();
        switch (elementType) {
            case Ace::WebJSValue::Type::STRING:
                napi_create_string_utf8(env, list[i].GetString().c_str(), NAPI_AUTO_LENGTH, &element);
                break;
            case Ace::WebJSValue::Type::INTEGER:
                napi_create_int32(env, list[i].GetInt(), &element);
                break;
            case Ace::WebJSValue::Type::DOUBLE:
                napi_create_double(env, list[i].GetDouble(), &element);
                break;
            case Ace::WebJSValue::Type::BOOLEAN:
                napi_get_boolean(env, list[i].GetBoolean(), &element);
                break;
            case Ace::WebJSValue::Type::LIST:
                napi_create_array(env, &element);
                CreateArrayNapi(env, std::make_shared<Ace::WebJSValue>(list[i]), element);
                break;
            case Ace::WebJSValue::Type::DICTIONARY:
                napi_create_object(env, &element);
                CreateDictionaryNapi(env, std::make_shared<Ace::WebJSValue>(list[i]), element);
                break;
            default:
                break;
        }
        if (element == nullptr) {
            napi_get_undefined(env, &element);
        }
        napi_set_element(env, napi_arg, i, element);
    }
}

void CreateDictionaryNapi(napi_env env, const std::shared_ptr<Ace::WebJSValue>& arg, napi_value napi_arg)
{
    CHECK_NULL_VOID(arg);
    auto dict = arg->GetDictionaryValue();
    napi_value element = nullptr;
    for (auto& [key, value]: dict) {
        element = nullptr;
        Ace::WebJSValue::Type valueType = value.GetType();
        switch (valueType) {
            case Ace::WebJSValue::Type::STRING:
                napi_create_string_utf8(env, value.GetString().c_str(), NAPI_AUTO_LENGTH, &element);
                break;
            case Ace::WebJSValue::Type::INTEGER:
                napi_create_int32(env, value.GetInt(), &element);
                break;
            case Ace::WebJSValue::Type::DOUBLE:
                napi_create_double(env, value.GetDouble(), &element);
                break;
            case Ace::WebJSValue::Type::BOOLEAN:
                napi_get_boolean(env, value.GetBoolean(), &element);
                break;
            case Ace::WebJSValue::Type::LIST: {
                napi_create_array(env, &element);
                auto listSharedItem = std::make_shared<OHOS::Ace::WebJSValue>(value);
                if (listSharedItem != nullptr) {
                    CreateArrayNapi(env, listSharedItem, element);
                }
                break;
            }
            case Ace::WebJSValue::Type::DICTIONARY: {
                napi_create_object(env, &element);
                auto dictionarySharedItem = std::make_shared<OHOS::Ace::WebJSValue>(value);
                if (dictionarySharedItem != nullptr) {
                    CreateDictionaryNapi(env, dictionarySharedItem, element);
                }
                break;
            }
            default:
                napi_get_null(env, &element);
                break;
        }
        if (element == nullptr) {
            napi_get_undefined(env, &element);
        }
        napi_set_named_property(env, napi_arg, key.c_str(), element);
    }
}

void GetStringNapiValue(napi_env env, napi_value ret, std::shared_ptr<Ace::WebJSValue>& argument)
{
    CHECK_NULL_VOID(argument);
    std::string result;
    std::vector<char> str(MAX_STR_LENGTH + 1, '\0');
    size_t length = 0;
    napi_get_value_string_utf8(env, ret, &str[0], MAX_STR_LENGTH, &length);
    if (length > 0) {
        result.append(&str[0], length);
    }
    argument->SetType(Ace::WebJSValue::Type::STRING);
    argument->SetString(result);
}

void GetNumberNapiValue(napi_env env, napi_value ret, std::shared_ptr<Ace::WebJSValue>& argument)
{
    CHECK_NULL_VOID(argument);
    double value = 0;
    napi_get_value_double(env, ret, &value);
    argument->SetType(Ace::WebJSValue::Type::DOUBLE);
    argument->SetDouble(value);
}

void GetBooleanNapiValue(napi_env env, napi_value ret, std::shared_ptr<Ace::WebJSValue>& argument)
{
    CHECK_NULL_VOID(argument);
    bool value = false;
    napi_get_value_bool(env, ret, &value);
    argument->SetType(Ace::WebJSValue::Type::BOOLEAN);
    argument->SetBoolean(value);
}

void ProcessArrayElement(napi_env env, napi_value element, std::shared_ptr<Ace::WebJSValue>& item)
{
    CHECK_NULL_VOID(item);
    napi_valuetype elementType;
    napi_typeof(env, element, &elementType);
    if (elementType == napi_string) {
        GetStringNapiValue(env, element, item);
    } else if (elementType == napi_number) {
        GetNumberNapiValue(env, element, item);
    } else if (elementType == napi_boolean) {
        GetBooleanNapiValue(env, element, item);
    } else if (elementType == napi_object) {
        bool isNestedArray = false;
        napi_is_array(env, element, &isNestedArray);
        if (isNestedArray) {
            item->SetType(Ace::WebJSValue::Type::LIST);
            auto nestedList = GetNapiValue(env, element);
            if (nestedList) {
                item = nestedList;
            }
        } else {
            item->SetType(Ace::WebJSValue::Type::DICTIONARY);
            auto nestedDict = GetNapiValue(env, element);
            if (nestedDict) {
                item = nestedDict;
            }
        }
    } else {
        LOGE("Unsupported element type in array");
    }
}

void GetArrayNapiValue(napi_env env, napi_value ret, std::shared_ptr<Ace::WebJSValue>& argument)
{
    CHECK_NULL_VOID(argument);
    argument->SetType(Ace::WebJSValue::Type::LIST);
    uint32_t length = 0;
    napi_get_array_length(env, ret, &length);
    napi_value element = nullptr;
    napi_valuetype elementType;
    for (uint32_t i = 0; i < length; ++i) {
        napi_status status = napi_get_element(env, ret, i, &element);
        if (status != napi_ok) {
            LOGE("Failed to get element at index %{public}u", i);
            continue;
        }
        status = napi_typeof(env, element, &elementType);
        if (status != napi_ok) {
            LOGE("Failed to get type of element at index %{public}u", i);
            continue;
        }
        std::shared_ptr<Ace::WebJSValue> item = std::make_shared<Ace::WebJSValue>(Ace::WebJSValue::Type::NONE);
        if (item) {
            ProcessArrayElement(env, element, item);
            argument->AddListValue(*item);
        }
    }
}

std::shared_ptr<Ace::WebJSValue> ProcessDictionaryValue(napi_env env, napi_value value, const std::string& keyStr)
{
    CHECK_NULL_RETURN(value, nullptr);
    napi_valuetype valueType;
    napi_typeof(env, value, &valueType);

    std::shared_ptr<Ace::WebJSValue> item = std::make_shared<Ace::WebJSValue>(Ace::WebJSValue::Type::NONE);
    if (item == nullptr) {
        LOGE("ProcessDictionaryValue failed to create WebJSValue");
        return nullptr;
    }
    if (valueType == napi_string) {
        GetStringNapiValue(env, value, item);
    } else if (valueType == napi_number) {
        GetNumberNapiValue(env, value, item);
    } else if (valueType == napi_boolean) {
        GetBooleanNapiValue(env, value, item);
    } else if (valueType == napi_object) {
        bool isNestedArray = false;
        napi_is_array(env, value, &isNestedArray);

        if (isNestedArray) {
            item->SetType(Ace::WebJSValue::Type::LIST);
            auto nestedList = GetNapiValue(env, value);
            if (nestedList) {
                item = nestedList;
            }
        } else {
            item->SetType(Ace::WebJSValue::Type::DICTIONARY);
            auto nestedDict = GetNapiValue(env, value);
            if (nestedDict) {
                item = nestedDict;
            }
        }
    } else {
        LOGE("Unsupported value type in dictionary for key: %{public}s", keyStr.c_str());
    }

    return item;
}

void GetDictionaryNapiValue(napi_env env, napi_value ret, std::shared_ptr<Ace::WebJSValue>& argument)
{
    CHECK_NULL_VOID(argument);
    argument->SetType(Ace::WebJSValue::Type::DICTIONARY);
    napi_value keys = nullptr;
    napi_get_all_property_names(env, ret, napi_key_own_only, napi_key_all_properties,
        napi_key_numbers_to_strings, &keys);
    uint32_t length = 0;
    napi_get_array_length(env, keys, &length);
    napi_value key = nullptr;
    napi_value value = nullptr;
    size_t strSize = 0;
    for (uint32_t i = 0; i < length; ++i) {
        key = nullptr;
        strSize = 0;
        napi_get_element(env, keys, i, &key);
        std::string result;
        std::vector<char> str(MAX_STR_LENGTH + 1, '\0');
        napi_get_value_string_utf8(env, key, &str[0], MAX_STR_LENGTH, &strSize);
        if (strSize > 0) {
            result.append(&str[0], strSize);
        }
        napi_get_named_property(env, ret, result.c_str(), &value);
        std::shared_ptr<Ace::WebJSValue> item = ProcessDictionaryValue(env, value, result);
        if (item) {
            argument->AddDictionaryValue(result, *item);
        }
    }
}

std::shared_ptr<Ace::WebJSValue> GetNapiValue(napi_env env, napi_value ret)
{
    napi_valuetype valueType;
    std::shared_ptr<Ace::WebJSValue> argument =
            std::make_shared<Ace::WebJSValue>(Ace::WebJSValue::Type::NONE);
    if (argument == nullptr) {
        LOGE("GetNapiValue failed to create WebJSValue");
        return nullptr;
    }
    if (napi_typeof(env, ret, &valueType) == napi_ok) {
        if (valueType == napi_string) {
            GetStringNapiValue(env, ret, argument);
        } else if (valueType == napi_object) {
            bool isArray = false;
            napi_is_array(env, ret, &isArray);
            if (isArray) {
                GetArrayNapiValue(env, ret, argument);
            } else {
                GetDictionaryNapiValue(env, ret, argument);
            }
        } else if (valueType == napi_number) {
            GetNumberNapiValue(env, ret, argument);
        } else if (valueType == napi_boolean) {
            GetBooleanNapiValue(env, ret, argument);
        } else {
            LOGE("GetNapiValue Unsupported value type");
        }
        
        return argument;
    }
    return nullptr;
}

void WebviewController::OnReceiveValue(const std::string& result, int32_t asyncCallbackInfoId)
{
    if (!result.empty()) {
        ExcuteAsyncCallbackInfo(result, asyncCallbackInfoId);
    }
}

void WebviewController::InsertAsyncCallbackInfo(
    const std::shared_ptr<AsyncEvaluteJSResultCallbackInfo>& asyncCallbackInfo)
{
    if (asyncCallbackInfo) {
        asyncCallbackInfoContainer_.push_back(asyncCallbackInfo);
    }
}

bool WebviewController::EraseAsyncCallbackInfo(const AsyncEvaluteJSResultCallbackInfo* asyncCallbackInfo)
{
    if (asyncCallbackInfoContainer_.empty() || !asyncCallbackInfo) {
        return false;
    }
    for (auto it = asyncCallbackInfoContainer_.cbegin(); it != asyncCallbackInfoContainer_.cend(); it++) {
        if ((*it) && (*it).get() == asyncCallbackInfo) {
            asyncCallbackInfoContainer_.erase(it);
            return true;
        }
    }
    return false;
}

bool WebviewController::ExcuteAsyncCallbackInfo(const std::string& result, int32_t asyncCallbackInfoId)
{
    LOGI("WebviewController OnReceiveValue result: %{public}s, asyncCallbackInfoId: %{public}d.",
        result.c_str(), asyncCallbackInfoId);
    for (const auto& asyncCallbackInfo : asyncCallbackInfoContainer_) {
        if (!asyncCallbackInfo) {
            continue;
        }
        if (asyncCallbackInfo->GetUniqueId() == asyncCallbackInfoId) {
            if ((asyncCallbackInfo->env) && (asyncCallbackInfo->asyncWork)) {
                asyncCallbackInfo->result = result;
                napi_queue_async_work(asyncCallbackInfo->env, asyncCallbackInfo->asyncWork);
                return true;
            }
        }
    }
    return false;
}

void WebviewController::OnReceiveRunJavaScriptExtValue(const std::string& type, const std::string& result, int32_t asyncCallbackInfoId)
{
    if (result.empty()) {
        LOGE("WebviewController OnReceiveRunJavaScriptExtValue result is empty.");
        return;
    }
    std::shared_ptr<WebMessage> webMessage = nullptr;
    if (type == "STRING") {
        webMessage = std::make_shared<WebMessage>(WebValue::Type::STRING);
        if (webMessage) {
            webMessage->SetString(result);
        }
    } else if (type == "BOOL") {
        webMessage = std::make_shared<WebMessage>(WebValue::Type::BOOLEAN);
        if (webMessage) {
            webMessage->SetBoolean(result == "true");
        }
    } else if (type == "INT") {
        webMessage = std::make_shared<WebMessage>(WebValue::Type::INTEGER);
        if (webMessage) {
            webMessage->SetInt(std::stoi(result));
        }
    } else if (type == "DOUBLE") {
        webMessage = std::make_shared<WebMessage>(WebValue::Type::DOUBLE);
        if (webMessage) {
            webMessage->SetDouble(std::stod(result));
        }
    } else if (type == "STRINGARRAY") {
        webMessage = std::make_shared<WebMessage>(WebValue::Type::STRINGARRAY);
        if (webMessage) {
            std::vector<std::string> vector = parseStringVector(result); 
            webMessage->SetStringArray(vector);
        }
    } else if (type == "BOOLEANARRAY") {
        webMessage = std::make_shared<WebMessage>(WebValue::Type::BOOLEANARRAY);
        if (webMessage) {
            std::vector<bool> vector = parseBooleanVector(result); 
            webMessage->SetBooleanArray(vector);
        }
    } else if (type == "INTARRAY") {
        webMessage = std::make_shared<WebMessage>(WebValue::Type::INT64ARRAY);
        if (webMessage) {
            std::vector<int64_t> vector = parseInt64Vector(result); 
            webMessage->SetInt64Array(vector);
        }
    } else if (type == "DOUBLEARRAY") {
        webMessage = std::make_shared<WebMessage>(WebValue::Type::DOUBLEARRAY);
        if (webMessage) {
            std::vector<double> vector = parseDoubleVector(result); 
            webMessage->SetDoubleArray(vector);
        }
    } else {
        webMessage = std::make_shared<WebMessage>(WebValue::Type::ERROR);
        if (webMessage) {
            webMessage->SetErrName("JavaScriptError");
            webMessage->SetErrMsg(result);
        }
    }
    if (webMessage) {
        ExcuteAsyncCallbackJavaScriptExtInfo(webMessage, asyncCallbackInfoId);
    }
}

void WebviewController::InsertAsyncCallbackJavaScriptExtInfo(
    const AsyncJavaScriptExtEvaluteJSResultCallbackInfo* asyncCallbackInfo)
{
    if (asyncCallbackInfo) {
        asyncCallbackJavaScriptExtInfoContainer_.push_back(
            const_cast<AsyncJavaScriptExtEvaluteJSResultCallbackInfo*>(asyncCallbackInfo));
    }
}

bool WebviewController::EraseAsyncCallbackJavaScriptExtInfo(
    const AsyncJavaScriptExtEvaluteJSResultCallbackInfo* asyncCallbackInfo)
{
    if (asyncCallbackJavaScriptExtInfoContainer_.empty() || !asyncCallbackInfo) {
        return false;
    }
    for (auto it = asyncCallbackJavaScriptExtInfoContainer_.cbegin();
         it != asyncCallbackJavaScriptExtInfoContainer_.cend(); it++) {
        if ((*it) == asyncCallbackInfo) {
            asyncCallbackJavaScriptExtInfoContainer_.erase(it);
            delete asyncCallbackInfo;
            return true;
        }
    }
    return false;
}

bool WebviewController::ExcuteAsyncCallbackJavaScriptExtInfo(const std::shared_ptr<WebMessage>& result, int32_t asyncCallbackInfoId)
{
    for (const auto& asyncCallbackInfo : asyncCallbackJavaScriptExtInfoContainer_) {
        if (!asyncCallbackInfo) {
            continue;
        }
        if (asyncCallbackInfo->GetUniqueId() == asyncCallbackInfoId) {
            if ((asyncCallbackInfo->env) && (asyncCallbackInfo->asyncWork)) {
                asyncCallbackInfo->result_ = result;
                napi_queue_async_work(asyncCallbackInfo->env, asyncCallbackInfo->asyncWork);
                return true;
            }
        }
    }
    return false;
}

void WebviewController::InsertJavaScriptProxy(const RegisterJavaScriptProxyParam& param)
{
    if (!param.objName.empty()) {
        std::lock_guard<std::mutex> lock(objMapMutex);
        objMap_[param.objName] = param;
    }
}

void WebviewController::DeleteJavaScriptProxy(const std::string& objName)
{
    std::lock_guard<std::mutex> lock(objMapMutex);
    auto it = objMap_.find(objName);
    if (it != objMap_.end()) {
        napi_env env = it->second.env;
        napi_ref objRef = it->second.objRef;
        if (objRef) {
            napi_delete_reference(env, objRef);
        }
        objMap_.erase(it);
    }
}

std::shared_ptr<Ace::WebJSValue> WebviewController::OnReceiveJavascriptExecuteCall(
    const std::string& objName, const std::string& methodName,
    const std::vector<std::shared_ptr<OHOS::Ace::WebJSValue>>& argsList)
{
    std::lock_guard<std::mutex> lock(objMapMutex);
    auto it = objMap_.find(objName);
    if (it != objMap_.end()) {
        RegisterJavaScriptProxyParam param = it->second;
        napi_value fn = nullptr;
        napi_value napi_obj = nullptr;
        napi_value ret = nullptr;
        if (napi_get_reference_value(param.env, param.objRef, &napi_obj) != napi_ok) {
            LOGE("OnReceiveJavascriptExecuteCallBack get reference value fail");
            return nullptr;
        }
        if (napi_get_named_property(param.env, napi_obj, methodName.c_str(), &fn) != napi_ok) {
            LOGE("OnReceiveJavascriptExecuteCallBack get fail %{public}s", methodName.c_str());
            return nullptr;
        }
        auto argv = CreateNapiArgv(param.env, argsList);
        auto argc = argv.size();
        if (napi_call_function(param.env, napi_obj, fn, argc, argv.data(), &ret) != napi_ok) {
            LOGE("OnReceiveJavascriptExecuteCallBack call function fail %{public}s", methodName.c_str());
            return nullptr;
        }
        return GetNapiValue(param.env, ret);
    }
    return nullptr;
}

WebHistoryItem::WebHistoryItem(const WebHistoryItem& other) : historyUrl(other.historyUrl),
    historyRawUrl(other.historyRawUrl), title(other.title)
{}

WebHistoryList::WebHistoryList(const WebHistoryList& other) : currentIndex_(other.currentIndex_),
    size_(other.size_)
{
    for (const auto& webHistoryItem : other.webHistoryItemContainer_) {
        if (!webHistoryItem) {
            continue;
        }
        auto item = std::make_shared<WebHistoryItem>(*webHistoryItem);
        webHistoryItemContainer_.push_back(item);
    }
}

WebHistoryList::~WebHistoryList()
{
    webHistoryItemContainer_.clear();
}

bool WebHistoryList::InsertHistoryItem(const std::shared_ptr<WebHistoryItem>& value)
{
    CHECK_NULL_RETURN(value, false);
    webHistoryItemContainer_.push_back(value);
    return true;
}

std::shared_ptr<WebHistoryItem> WebHistoryList::GetItemAtIndex(int32_t index)
{
    if (webHistoryItemContainer_.empty() || index < 0 || index >= size_) {
        return nullptr;
    }
    return webHistoryItemContainer_.at(index);
}

void WebviewController::SetWebDebuggingAccess(bool webDebuggingAccess)
{
#ifdef ANDROID_PLATFORM
    WebviewControllerAndroid::SetWebDebuggingAccess(webDebuggingAccess);
#endif
#ifdef IOS_PLATFORM
    WebviewControllerIOS::SetWebDebuggingAccess(webDebuggingAccess);
#endif
}

ErrCode WebviewController::CheckObjectName(const std::string& objName)
{
    if (objName.empty()) {
        return NWebError::CANNOT_DEL_JAVA_SCRIPT_PROXY;
    }

    std::lock_guard<std::mutex> lock(objMapMutex);
    auto it = objMap_.find(objName);
    if (it == objMap_.end()) {
        return NWebError::CANNOT_DEL_JAVA_SCRIPT_PROXY;
    }

    return NWebError::NO_ERROR;
}
} // namespace OHOS::Plugin
