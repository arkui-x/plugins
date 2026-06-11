/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "native_document_view_picker.h"

#include "js_native_api.h"
#include "js_native_api_types.h"
#include "utils/log.h"

namespace OHOS::Plugin {
napi_env DocumentFilePicker::napienv = nullptr;
napi_deferred DocumentFilePicker::deferred = nullptr;

DocumentFilePicker::DocumentFilePicker() = default;

DocumentFilePicker::~DocumentFilePicker()
{
    if (jsThisRef_ != nullptr && refEnv_ != nullptr) {
        napi_delete_reference(refEnv_, jsThisRef_);
        jsThisRef_ = nullptr;
    }
}

void DocumentFilePicker::RetainJsThis(napi_env env, napi_value thisVar)
{
    if (jsThisRef_ == nullptr) {
        napi_create_reference(env, thisVar, 1, &jsThisRef_);
        refEnv_ = env;
        refCount_ = 1;
    } else {
        napi_reference_ref(env, jsThisRef_, &refCount_);
    }
}

void DocumentFilePicker::ReleaseJsThis()
{
    if (jsThisRef_ == nullptr || refEnv_ == nullptr) {
        return;
    }
    uint32_t count = 0;
    napi_reference_unref(refEnv_, jsThisRef_, &count);
    if (count == 0) {
        napi_delete_reference(refEnv_, jsThisRef_);
        jsThisRef_ = nullptr;
        refEnv_ = nullptr;
    }
}

void DocumentFilePicker::onPickerResult(const std::vector<std::string>& result, int errCode)
{
    HILOG_INFO("DocumentFilePicker::onPickerResult enter");
    napi_value resultValue;
    napi_create_object(DocumentFilePicker::napienv, &resultValue);
    if (DocumentFilePicker::deferred == nullptr) {
        HILOG_ERROR("DocumentFilePicker::onPickerResult deferred is null");
        return;
    }
    if (errCode != 0) {
        HILOG_ERROR("DocumentFilePicker::onPickerResult errCode:%d", errCode);
        napi_value errorCode;
        napi_create_int32(DocumentFilePicker::napienv, errCode, &errorCode);
        napi_set_named_property(DocumentFilePicker::napienv, resultValue, "error", errorCode);
        napi_resolve_deferred(DocumentFilePicker::napienv, DocumentFilePicker::deferred, resultValue);
        return;
    }

    napi_value dataArray;
    napi_create_array(DocumentFilePicker::napienv, &dataArray);
    for (size_t i = 0; i < result.size(); ++i) {
        napi_value str;
        napi_create_string_utf8(DocumentFilePicker::napienv, result[i].c_str(), result[i].length(), &str);
        napi_set_element(DocumentFilePicker::napienv, dataArray, i, str);
    }
    napi_set_named_property(DocumentFilePicker::napienv, resultValue, "data", dataArray);
    napi_resolve_deferred(DocumentFilePicker::napienv, DocumentFilePicker::deferred, resultValue);
}
int32_t DocumentFilePicker::CreatePendingRequest(napi_env env, napi_value* outPromise)
{
    int32_t id = nextRequestId_++;
    PendingRequest req;
    req.env = env;
    napi_create_promise(env, &req.deferred, outPromise);
    pendingRequests_[id] = req;
    return id;
}

void DocumentFilePicker::ResolvePendingRequest(int32_t requestId,
    const std::vector<std::string>& result, int errCode)
{
    auto it = pendingRequests_.find(requestId);
    if (it == pendingRequests_.end()) {
        HILOG_ERROR("ResolvePendingRequest: requestId %d not found", requestId);
        return;
    }

    napi_env env = it->second.env;
    napi_deferred deferred = it->second.deferred;

    napi_value resultValue;
    napi_create_object(env, &resultValue);

    if (errCode != 0) {
        napi_value errorCode;
        napi_create_int32(env, errCode, &errorCode);
        napi_set_named_property(env, resultValue, "error", errorCode);
    } else {
        napi_value dataArray;
        napi_create_array(env, &dataArray);
        for (size_t i = 0; i < result.size(); ++i) {
            napi_value str;
            napi_create_string_utf8(env, result[i].c_str(), result[i].length(), &str);
            napi_set_element(env, dataArray, i, str);
        }
        napi_set_named_property(env, resultValue, "data", dataArray);
    }

    napi_resolve_deferred(env, deferred, resultValue);
    pendingRequests_.erase(it);
    ReleaseJsThis();
}
} // namespace OHOS::Plugin