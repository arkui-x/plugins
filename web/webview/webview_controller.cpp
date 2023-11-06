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

namespace OHOS::Plugin {
std::vector<std::shared_ptr<AsyncEvaluteJSResultCallbackInfo>> WebviewController::asyncCallbackInfoContainer_;
int32_t WebviewController::index_ = 0;
std::mutex WebviewController::mutex_;

void WebviewController::OnReceiveValue(const std::string& result)
{
    if (!result.empty() && ExcuteAsyncCallbackInfo(result)) {
        IncreaseIndex();
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
    for (auto it = asyncCallbackInfoContainer_.begin(); it != asyncCallbackInfoContainer_.end(); it++) {
        if ((*it) && (*it).get() == asyncCallbackInfo) {
            asyncCallbackInfoContainer_.erase(it);
            DecreaseIndex();
            return true;
        }
    }
    return false;
}

void WebviewController::IncreaseIndex()
{
    std::lock_guard<std::mutex> guard(mutex_);
    index_++;
    if (asyncCallbackInfoContainer_.empty() || index_ >= asyncCallbackInfoContainer_.size()) {
        index_ = 0;
    }
}

void WebviewController::DecreaseIndex()
{
    std::lock_guard<std::mutex> guard(mutex_);
    index_--;
    if (asyncCallbackInfoContainer_.empty() || index_ < 0) {
        index_ = 0;
    }
}

bool WebviewController::ExcuteAsyncCallbackInfo(const std::string& result)
{
    LOGD("index_ == %{public}d, asyncCallbackInfoContainer_ size == %{public}d", index_,
        asyncCallbackInfoContainer_.size());
    if (asyncCallbackInfoContainer_.empty()) {
        return false;
    }
    auto asyncCallbackInfo = asyncCallbackInfoContainer_.at(index_);
    CHECK_NULL_RETURN(asyncCallbackInfo, false);
    asyncCallbackInfo->result = result;
    if ((asyncCallbackInfo->env) && (asyncCallbackInfo->asyncWork)) {
        napi_queue_async_work(asyncCallbackInfo->env, asyncCallbackInfo->asyncWork);
        return true;
    }
    return false;
}
} // namespace OHOS::Plugin