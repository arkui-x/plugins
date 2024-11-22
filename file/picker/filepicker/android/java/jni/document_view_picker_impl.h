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
#ifndef NATIVE_DOCUMENT_VIEW_PICKER_IMPL_H
#define NATIVE_DOCUMENT_VIEW_PICKER_IMPL_H

#include "../../../native_document_view_picker.h"
#include "filepicker_jni.h"
#include "js_native_api.h"
#include "js_native_api_types.h"
namespace OHOS::Plugin {

class DocumentFilePickerImpl final : public DocumentFilePicker {
public:
    DocumentFilePickerImpl() = default;
    virtual ~DocumentFilePickerImpl() {};

    napi_value select(napi_env env, DocumentSelectOptions&) override;
    napi_value save(napi_env env, DocumentSaveOptions&) override;
};
} // namespace OHOS::Plugin
#endif // NATIVE_DOCUMENT_VIEW_PICKER_IMPL_H