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
} // namespace OHOS::Plugin