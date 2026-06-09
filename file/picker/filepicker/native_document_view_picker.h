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
#ifndef NATIVE_DOCUMENT_VIEW_PICKER_H
#define NATIVE_DOCUMENT_VIEW_PICKER_H
#include <unordered_map>
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
namespace OHOS::Plugin {
struct PendingRequest {
    napi_env env = nullptr;
    napi_deferred deferred = nullptr;
};

struct DocumentSelectOptions {
    int32_t maxSelectNumber;
    std::string defaultFilePathUri;
    std::vector<std::string> fileSuffixFilters;
    int32_t key_select_mode;
    bool authMode;
};

struct DocumentSaveOptions {
    std::vector<std::string> newFileNames;
    std::string defaultFilePathUri;
    std::vector<std::string> fileSuffixFilters;
    std::string saveFile;
};

class DocumentFilePicker {
public:
    explicit DocumentFilePicker();
    virtual ~DocumentFilePicker();

    static napi_deferred deferred;
    static napi_env napienv;

    virtual napi_value select(napi_env env, DocumentSelectOptions&) = 0;
    virtual napi_value save(napi_env env, DocumentSaveOptions&) = 0;

    static void onPickerResult(const std::vector<std::string>& result, int errCode);

    // Instance-level Promise management
    int32_t CreatePendingRequest(napi_env env, napi_value* outPromise);
    void ResolvePendingRequest(int32_t requestId, const std::vector<std::string>& result, int errCode);

    // Prevent GC from collecting the JS wrapper during async operations
    void RetainJsThis(napi_env env, napi_value thisVar);
    void ReleaseJsThis();

private:
    int32_t nextRequestId_ = 0;
    std::unordered_map<int32_t, PendingRequest> pendingRequests_;
    napi_ref jsThisRef_ = nullptr;
    napi_env refEnv_ = nullptr;
    uint32_t refCount_ = 0;
};
} // namespace OHOS::Plugin
#endif // NATIVE_DOCUMENT_VIEW_PICKER_H