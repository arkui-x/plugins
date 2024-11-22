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

#ifndef PHOTO_PICKER_NAPI_H
#define PHOTO_PICKER_NAPI_H

#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_error.h"
#include "photo_picker_callback.h"
#include "userfile_manager_types.h"
#include "medialibrary_napi_utils.h"

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))
static const std::string PHOTO_PICKER_NAPI_CLASS_NAME = "PhotoViewPicker";
static const std::string PHOTO_SELECT_OPTION_CLASS_NAME = "PhotoSelectOptions";
static const std::string PHOTO_SELECT_RESULT_CLASS_NAME = "PhotoSelectResult";

const std::string DEFAULT_VIDEO_MIME_TYPE = "video/*";
const std::string DEFAULT_IMAGE_MIME_TYPE = "image/*";
const std::string ALL_MIME_TYPE = "*/*";

const std::vector<std::pair<std::string, std::string>> MIME_ENUM_PROPERTIES = {
    std::make_pair("IMAGE_TYPE",                DEFAULT_IMAGE_MIME_TYPE),
    std::make_pair("VIDEO_TYPE",                DEFAULT_VIDEO_MIME_TYPE),
    std::make_pair("IMAGE_VIDEO_TYPE",          ALL_MIME_TYPE)
};

class PhotoPickerNapi {
public:
    EXPORT static napi_value PhotoPickerInit(napi_env env, napi_value exports);

    EXPORT PhotoPickerNapi();
    EXPORT ~PhotoPickerNapi();

    static std::mutex sUserFileClientMutex_;

private:
    EXPORT static void PhotoPickerNapiDestructor(napi_env env, void *nativeObject, void *finalize_hint);
    EXPORT static napi_value PhotoPickerNapiConstructor(napi_env env, napi_callback_info info);
    EXPORT static napi_value SelectOptionsInit(napi_env env, napi_value exports);
    EXPORT static napi_value CreatePhotoSelectResult(napi_env env, napi_value exports);

    EXPORT static napi_value CreateMIMETypesEnum(napi_env env);
    EXPORT static napi_value StartPhotoPicker(napi_env env, napi_callback_info info);

    napi_env env_;

    static thread_local napi_ref sPhotoViewPickerConstructor_;
    static thread_local napi_ref sPhotoSelectConstructor_;
    static thread_local napi_ref sMIMETypeEnumRef_;

    std::shared_ptr<PickerCallBack> pickerCallBack;
};

class SelectResultNapi {
public:
    EXPORT static napi_value SelectResultInit(napi_env env, napi_value exports);
    EXPORT static napi_value ParseResult(napi_env env, std::shared_ptr<PickerCallBack> pickerCallBack);

    EXPORT SelectResultNapi() = default;
    EXPORT ~SelectResultNapi() = default;

private:
    EXPORT static void SelectResultNapiDestructor(napi_env env, void *nativeObject, void *finalize_hint);
    EXPORT static napi_value SelectResultNapiConstructor(napi_env env, napi_callback_info info);
    EXPORT static napi_value JSGetUris(napi_env env, napi_callback_info info);
    EXPORT static napi_value JSGetIsOrigin(napi_env env, napi_callback_info info);

    napi_env env_;
    static inline thread_local napi_value isOrigin_;
    static inline thread_local napi_value jsUris_;

    static thread_local napi_ref sSelectResultConstructor_;
};

struct PhotoPickerAsyncContext : public NapiError {
    napi_async_work work;
    napi_deferred deferred;
    napi_ref callbackRef;
    size_t argc;
    napi_value argv[NAPI_ARGC_MAX];
    ResultNapiType resultNapiType;
    napi_value MIMEType;
    std::shared_ptr<PickerCallBack> pickerCallBack;

    PhotoPickerNapi* objectInfo;
};
} // namespace Media
} // namespace OHOS
#endif /* PHOTO_PICKER_NAPI_H */