/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "document_view_picker_impl.h"

#include "utils/log.h"

namespace OHOS::Plugin {

napi_value DocumentFilePickerImpl::select(napi_env env, DocumentSelectOptions& options)
{
    HILOG_INFO("DocumentFilePickerImpl::select enter");
    DocumentFilePicker::napienv = env;
    napi_value promise = nullptr;
    DocumentFilePicker::deferred = nullptr;
    napi_create_promise(env, &DocumentFilePicker::deferred, &promise);
    FilePickerJni::Select(options);
    return promise;
}

napi_value DocumentFilePickerImpl::save(napi_env env, DocumentSaveOptions& options)
{
    HILOG_INFO("DocumentFilePickerImpl::save enter");
    DocumentFilePicker::napienv = env;
    napi_value promise = nullptr;
    DocumentFilePicker::deferred = nullptr;
    napi_create_promise(env, &DocumentFilePicker::deferred, &promise);
    FilePickerJni::Save(options);
    return promise;
}
} // namespace OHOS::Plugin