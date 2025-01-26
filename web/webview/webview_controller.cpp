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
thread_local std::vector<std::shared_ptr<AsyncEvaluteJSResultCallbackInfo>> WebviewController::asyncCallbackInfoContainer_;
thread_local std::vector<AsyncJavaScriptExtEvaluteJSResultCallbackInfo*>
    WebviewController::asyncCallbackJavaScriptExtInfoContainer_;

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
} // namespace OHOS::Plugin
