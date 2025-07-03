/*
 * Copyright (C) 2022-2023 Huawei Device Co., Ltd.
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

#include "progress_notify.h"
#include "log.h"
#include "napi_utils.h"
#include "task_info_json.h"

namespace OHOS::Plugin::Request {
ProgressNotify::ProgressNotify(napi_env env, napi_value cb, Version version, Action action)
    : JsNotify(env, cb, version, action)
{
}

void ProgressNotify::HandleCallback(napi_env env, napi_value cb, const std::string& param)
{
    REQUEST_HILOGI("ProgressNotify::HandleCallback start");
    auto infoJson = nlohmann::json::parse(param.c_str(), nullptr, false);
    if (infoJson.is_null() || infoJson.is_discarded()) {
        REQUEST_HILOGE("invalid json of task info");
        return;
    }
    auto info = infoJson.get<TaskInfo>();
    napi_value callbackResult = nullptr;
    napi_value callbackValues[NapiUtils::TWO_ARG] = { 0 };
    if (version_ == Version::API9) {
        auto receivedSize = NapiUtils::Convert2JSValue(env, GetProcessSize(info));
        auto totalSize = NapiUtils::Convert2JSValue(env, GetTotalSize(info));
        callbackValues[NapiUtils::FIRST_ARGV] = receivedSize;
        callbackValues[NapiUtils::SECOND_ARGV] = totalSize;
        napi_call_function(env, nullptr, cb, NapiUtils::TWO_ARG, callbackValues, &callbackResult);
    } else {
        if (action_ == Action::DOWNLOAD) {
            if (info.progress.lastProcessed > info.progress.processed) {
                info.progress.processed = info.progress.lastProcessed;
            }
        }
        auto jsProgress = NapiUtils::Convert2JSValue(env, info.progress);
        callbackValues[NapiUtils::FIRST_ARGV] = jsProgress;
        napi_call_function(env, nullptr, cb, NapiUtils::ONE_ARG, callbackValues, &callbackResult);
    }

    REQUEST_HILOGI("ProgressNotify::HandleCallback end");
}

int64_t ProgressNotify::GetProcessSize(const TaskInfo &info) const
{
    int64_t result = info.progress.totalProcessed;
    if (action_ == Action::DOWNLOAD) {
        result = info.progress.processed;
    }
    return result;
}

int64_t ProgressNotify::GetTotalSize(const TaskInfo &info) const
{
    int64_t sum = 0;
    for (auto item : info.progress.sizes) {
        sum += item;
    }
    return sum;
}
}