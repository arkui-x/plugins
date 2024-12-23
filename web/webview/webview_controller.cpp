/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "webview_controller.h"
#include "android/java/jni/webview_controller_android.h"
#ifdef ANDROID_PLATFORM
#include "android/java/jni/webview_controller_android.h"
#endif
#ifdef IOS_PLATFORM
#include "ios/webview_controller_ios.h"
#endif

namespace OHOS::Plugin {
thread_local std::vector<std::shared_ptr<AsyncEvaluteJSResultCallbackInfo>> WebviewController::asyncCallbackInfoContainer_;

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
