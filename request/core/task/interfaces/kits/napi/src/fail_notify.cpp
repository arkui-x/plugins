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

#include "fail_notify.h"
#include "log.h"
#include "napi_utils.h"
#include "task_info_json.h"

namespace OHOS::Plugin::Request {
std::map<Reason, DownloadErrorCode> FailNotify::failMap_ = {
    {REASON_OK, ERROR_FILE_ALREADY_EXISTS},
    {IO_ERROR, ERROR_FILE_ERROR},
    {REDIRECT_ERROR, ERROR_TOO_MANY_REDIRECTS},
    {OTHERS_ERROR, ERROR_UNKNOWN},
    {NETWORK_OFFLINE, ERROR_OFFLINE},
    {UNSUPPORTED_NETWORK_TYPE, ERROR_UNSUPPORTED_NETWORK_TYPE},
    {UNSUPPORT_RANGE_REQUEST, ERROR_UNKNOWN},
};

FailNotify::FailNotify(napi_env env, napi_value cb, Version version, Action action)
    : JsNotify(env, cb, version, action)
{
}

void FailNotify::HandleCallback(napi_env env, napi_value cb, const std::string& param)
{
    REQUEST_HILOGI("FailNotify::HandleCallback start");
    auto infoJson = nlohmann::json::parse(param.c_str(), nullptr, false);
    if (infoJson.is_null() || infoJson.is_discarded()) {
        REQUEST_HILOGE("invalid json of task info");
        return;
    }
    auto info = infoJson.get<TaskInfo>();
    napi_value callbackResult = nullptr;
    napi_value callbackValues[NapiUtils::TWO_ARG] = { 0 };
    if (version_ == Version::API9) {
        if (action_ == Action::UPLOAD) {
            callbackValues[NapiUtils::FIRST_ARGV] = NapiUtils::Convert2JSValue(env, info.taskStates);
        } else {
            DownloadErrorCode errCode = ERROR_UNKNOWN;
            if (info.progress.state == State::FAILED) {
                auto it = failMap_.find(info.code);
                if (it != failMap_.end()) {
                    errCode = it->second;
                }
            }
            callbackValues[NapiUtils::FIRST_ARGV] = NapiUtils::Convert2JSValue(env, errCode);
        }
        napi_call_function(env, nullptr, cb, NapiUtils::ONE_ARG, callbackValues, &callbackResult);
    } else {
        auto jsProgress = NapiUtils::Convert2JSValue(env, info.progress);
        callbackValues[NapiUtils::FIRST_ARGV] = jsProgress;
        napi_call_function(env, nullptr, cb, NapiUtils::ONE_ARG, callbackValues, &callbackResult);
    }

    REQUEST_HILOGI("FailNotify::HandleCallback end");
}
}