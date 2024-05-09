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

#include "napi_document_view_picker.h"
#include "js_native_api_types.h"

#ifdef ANDROID_PLATFORM
#include "plugins/file/picker/filepicker/android/java/jni/document_view_picker_impl.h"
#endif
#ifdef IOS_PLATFORM
#include "ios/document_picker_impl.h"
#endif

namespace OHOS::Plugin {

napi_value GetDocumentSelectOptions(const napi_env& env, const napi_value& argv, DocumentSelectOptions& options)
{
    LOGI("GetDocumentSelectOptions enter");
    bool hasProperty = false;

    // maxSelectNumber
    NAPI_CALL_CHECK(napi_has_named_property(env, argv, "key_pick_num", &hasProperty));

    if (hasProperty) {
        napi_value maxSelectNumber;
        NAPI_CALL_CHECK(napi_get_named_property(env, argv, "key_pick_num", &maxSelectNumber));
        NAPI_CALL_CHECK(napi_get_value_int32(env, maxSelectNumber, &options.maxSelectNumber));
    }

    // defaultFilePathUri
    NAPI_CALL_CHECK(napi_has_named_property(env, argv, "key_pick_dir_path", &hasProperty));
    if (hasProperty) {
        napi_value defaultFilePathUri;
        NAPI_CALL_CHECK(napi_get_named_property(env, argv, "key_pick_dir_path", &defaultFilePathUri));
        size_t size;
        NAPI_CALL_CHECK(napi_get_value_string_utf8(env, defaultFilePathUri, nullptr, 0, &size));
        NAPI_CALL_CHECK(napi_get_value_string_utf8(
            env, defaultFilePathUri, (char*)options.defaultFilePathUri.c_str(), size + 1, &size));
    }

    // fileSuffixFilters
    NAPI_CALL_CHECK(napi_has_named_property(env, argv, "key_file_suffix_filter", &hasProperty));
    if (hasProperty) {
        napi_value value;
        NAPI_CALL_CHECK(napi_get_named_property(env, argv, "key_file_suffix_filter", &value));
        ParseStringArray(env, value, options.fileSuffixFilters);
    }

    // key_select_mode
    NAPI_CALL_CHECK(napi_has_named_property(env, argv, "key_select_mode", &hasProperty));
    if (hasProperty) {
        napi_value key_select_mode;
        NAPI_CALL_CHECK(napi_get_named_property(env, argv, "key_select_mode", &key_select_mode));
        NAPI_CALL_CHECK(napi_get_value_int32(env, key_select_mode, &options.key_select_mode));
    }

    // authMode
    NAPI_CALL_CHECK(napi_has_named_property(env, argv, "key_auth_mode", &hasProperty));
    if (hasProperty) {
        napi_value authMode;
        NAPI_CALL_CHECK(napi_get_named_property(env, argv, "key_auth_mode", &authMode));
        NAPI_CALL_CHECK(napi_get_value_bool(env, authMode, &options.authMode));
    }
    LOGI("GetDocumentSelectOptions end");
    return nullptr;
}

napi_value DocumentViewPickerSelect(napi_env env, napi_callback_info info)
{
    LOGI("DocumentViewPickerSelect enter");
    napi_value thisVar = nullptr;
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    NAPI_CALL_CHECK(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));

    DocumentSelectOptions options;
    GetDocumentSelectOptions(env, argv[0], options);

    DocumentFilePicker* mfp = nullptr;
    NAPI_CALL_CHECK(napi_unwrap(env, thisVar, reinterpret_cast<void**>(&mfp)));
    napi_value rst = mfp->select(env, options);
    LOGI("DocumentViewPickerSelect end");
    return rst;
}

napi_value GetDocumentSaveOptions(const napi_env& env, const napi_value& argv, DocumentSaveOptions& options)
{
    bool hasProperty = false;

    // newFileNames
    NAPI_CALL_CHECK(napi_has_named_property(env, argv, "key_pick_file_name", &hasProperty));
    if (hasProperty) {
        napi_value value;
        NAPI_CALL_CHECK(napi_get_named_property(env, argv, "key_pick_file_name", &value));
        ParseStringArray(env, value, options.newFileNames);
        if (options.newFileNames.size() > 0) {
            options.saveFile = options.newFileNames[0];
        }
    }

    // defaultFilePathUri
    NAPI_CALL_CHECK(napi_has_named_property(env, argv, "key_pick_dir_path", &hasProperty));
    if (hasProperty) {
        napi_value defaultFilePathUri;
        NAPI_CALL_CHECK(napi_get_named_property(env, argv, "key_pick_dir_path", &defaultFilePathUri));
        size_t size;
        NAPI_CALL_CHECK(napi_get_value_string_utf8(env, defaultFilePathUri, nullptr, 0, &size));
        NAPI_CALL_CHECK(napi_get_value_string_utf8(
            env, defaultFilePathUri, (char*)options.defaultFilePathUri.c_str(), size + 1, &size));
    }

    // fileSuffixFilters
    NAPI_CALL_CHECK(napi_has_named_property(env, argv, "key_file_suffix_choices", &hasProperty));
    if (hasProperty) {
        napi_value value;
        NAPI_CALL_CHECK(napi_get_named_property(env, argv, "key_file_suffix_choices", &value));
        ParseStringArray(env, value, options.fileSuffixFilters);
    }

    return nullptr;
}

napi_value DocumentViewPickerSave(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    NAPI_CALL_CHECK(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    DocumentSaveOptions options;
    GetDocumentSaveOptions(env, argv[0], options);

    DocumentFilePicker* mfp = nullptr;
    NAPI_CALL_CHECK(napi_unwrap(env, thisVar, reinterpret_cast<void**>(&mfp)));
    napi_value rst = mfp->save(env, options);
    return rst;
}

napi_value DocumentViewPickerInitConstructor(napi_env env, napi_callback_info info)
{
    LOGI(" DocumentViewPickerInitConstructor enter");
    napi_value thisVar = nullptr;
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    void* data = nullptr;
    NAPI_CALL_CHECK(napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));

    DocumentFilePicker* object = new DocumentFilePickerImpl();
    NAPI_CALL_CHECK(napi_wrap(
        env, thisVar, object,
        [](napi_env environment, void* data, void* hint) {
            auto obj = reinterpret_cast<DocumentFilePicker*>(data);
            delete obj;
        },
        nullptr, nullptr));

    return thisVar;
}

napi_value DocumentViewPickerInit(napi_env env, napi_value exports)
{
    LOGI("DocumentViewPickerInit enter");
    const char* DocumentViewPickerClass = "documentViewPicker";
    napi_value xmlClass = nullptr;
    napi_property_descriptor xmlDesc[] = {
        DECLARE_NAPI_FUNCTION("select", DocumentViewPickerSelect),
        DECLARE_NAPI_FUNCTION("save", DocumentViewPickerSave),

    };
    NAPI_CALL_CHECK(napi_define_class(env, DocumentViewPickerClass, strlen(DocumentViewPickerClass),
        DocumentViewPickerInitConstructor, nullptr, sizeof(xmlDesc) / sizeof(xmlDesc[0]), xmlDesc, &xmlClass));
    napi_property_descriptor desc[] = { DECLARE_NAPI_PROPERTY("documentViewPicker", xmlClass) };
    NAPI_CALL_CHECK(napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    return exports;
}
} // namespace OHOS::Plugin