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

#define MLOG_TAG "PhotoPickerNapi"

#include <fcntl.h>
#include <functional>
#include <mutex>
#include <thread>

#include "log.h"
#include "photo_picker_napi.h"
#include "medialibrary_napi_utils.h"
#include "medialibrary_client_errno.h"
#include "plugins/interfaces/native/inner_api/plugin_utils_napi.h"
#include "userfile_client.h"

using namespace std;

namespace OHOS {
namespace Media {
const int32_t SLEEP_TIME = 100;
thread_local napi_ref PhotoPickerNapi::sPhotoViewPickerConstructor_ = nullptr;
thread_local napi_ref PhotoPickerNapi::sPhotoSelectConstructor_ = nullptr;
thread_local napi_ref PhotoPickerNapi::sMIMETypeEnumRef_ = nullptr;
thread_local napi_ref SelectResultNapi::sSelectResultConstructor_ = nullptr;
std::shared_ptr<PickerCallBack> PhotoPickerCallback::pickerCallBack = nullptr;
mutex PhotoPickerNapi::sUserFileClientMutex_;

PhotoPickerNapi::PhotoPickerNapi()
    : env_(nullptr) {}

PhotoPickerNapi::~PhotoPickerNapi() = default;

void PhotoPickerNapi::PhotoPickerNapiDestructor(napi_env env, void *nativeObject, void *finalize_hint) {
    PhotoPickerNapi *picker = reinterpret_cast<PhotoPickerNapi*>(nativeObject);
    if (picker != nullptr) {
        delete picker;
        picker = nullptr;
    }
}

napi_value PhotoPickerNapi::PhotoPickerNapiConstructor(napi_env env, napi_callback_info info) {
    napi_status status;
    napi_value result = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_undefined(env, &result));
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        LOGE("Error while obtaining js environment information, status: %{public}d", status);
        return result;
    }

    unique_ptr<PhotoPickerNapi> obj = make_unique<PhotoPickerNapi>();
    if (obj == nullptr) {
        return result;
    }
    obj->env_ = env;
    std::unique_lock<std::mutex> helperLock(PhotoPickerNapi::sUserFileClientMutex_);
    if (!UserFileClient::IsValid()) {
        UserFileClient::Init(env, info);
    }
    helperLock.unlock();

    status = napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()),
                       PhotoPickerNapi::PhotoPickerNapiDestructor, nullptr, nullptr);
    if (status == napi_ok) {
        obj.release();
        return thisVar;
    } else {
        LOGE("Failed to wrap the native media lib client object with JS, status: %{public}d", status);
    }

    return result;
}

void SelectResultNapi::SelectResultNapiDestructor(napi_env env, void *nativeObject, void *finalize_hint) {
    SelectResultNapi *selectResult = reinterpret_cast<SelectResultNapi*>(nativeObject);
    if (selectResult != nullptr) {
        delete selectResult;
        selectResult = nullptr;
    }
}

napi_value SelectResultNapi::SelectResultNapiConstructor(napi_env env, napi_callback_info info) {
    napi_status status;
    napi_value result = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_undefined(env, &result));
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        LOGE("Error while obtaining js environment information, status: %{public}d", status);
        return result;
    }

    unique_ptr<SelectResultNapi> obj = make_unique<SelectResultNapi>();
    if (obj == nullptr) {
        return result;
    }
    obj->env_ = env;
    status = napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()),
                       SelectResultNapi::SelectResultNapiDestructor, nullptr, nullptr);
    if (status == napi_ok) {
        obj.release();
        return thisVar;
    } else {
        LOGE("Failed to wrap the native media lib client object with JS, status: %{public}d", status);
    }

    return result;
}

napi_value SelectResultNapi::ParseResult(napi_env env, std::shared_ptr<PickerCallBack> pickerCallBack)
{
    const vector<string> &uris = pickerCallBack->uris;
    napi_value jsUris = nullptr;
    napi_create_array_with_length(env, uris.size(), &jsUris);
    napi_value jsUri = nullptr;
    for (size_t i = 0; i < uris.size(); i++) {
        CHECK_ARGS(env, napi_create_string_utf8(env, uris[i].c_str(),
            NAPI_AUTO_LENGTH, &jsUri), JS_INNER_FAIL);
        if ((jsUri == nullptr) || (napi_set_element(env, jsUris, i, jsUri) != napi_ok)) {
            LOGE("failed to set uri array");
            break;
        }
    }
    jsUris_ = jsUris;
    napi_get_boolean(env, pickerCallBack->isOrigin, &isOrigin_);

    napi_value constructor;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, SelectResultNapi::sSelectResultConstructor_, &constructor));
    NAPI_CALL(env, napi_new_instance(env, constructor, 0, nullptr, &result));
    napi_set_named_property(env, result, "photoUris", jsUris);
    napi_set_named_property(env, result, "isOriginalPhoto", isOrigin_);
    return result;
}

static napi_status AddStringNamedProperty(napi_env env, napi_value object,
    const string &name, string enumValue)
{
    napi_value enumNapiValue;
    napi_status status = napi_create_string_utf8(env, enumValue.c_str(), NAPI_AUTO_LENGTH, &enumNapiValue);
    if (status == napi_ok) {
        status = napi_set_named_property(env, object, name.c_str(), enumNapiValue);
    }
    return status;
}

static napi_value CreateStringEnumProperty(napi_env env, vector<pair<string, string>> properties, napi_ref &ref)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    for (unsigned int i = 0; i < properties.size(); i++) {
        NAPI_CALL(env, AddStringNamedProperty(env, result, properties[i].first, properties[i].second));
    }
    NAPI_CALL(env, napi_create_reference(env, result, NAPI_INIT_REF_COUNT, &ref));
    return result;
}

napi_value PhotoPickerNapi::CreateMIMETypesEnum(napi_env env) {
    return CreateStringEnumProperty(env,  MIME_ENUM_PROPERTIES, sMIMETypeEnumRef_);
}

static void StartPhotoPickerExecute(napi_env env, void *data)
{
    auto *context = static_cast<PhotoPickerAsyncContext*>(data);
    while (!context->pickerCallBack->ready) {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
    }
}

static void StartPhotoPickerAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    auto *context = static_cast<PhotoPickerAsyncContext*>(data);
    CHECK_NULL_PTR_RETURN_VOID(context, "Async context is null");

    auto jsContext = make_unique<JSAsyncContextOutput>();
    jsContext->status = false;
    CHECK_ARGS_RET_VOID(env, napi_get_undefined(env, &jsContext->data), JS_ERR_PARAMETER_INVALID);
    CHECK_ARGS_RET_VOID(env, napi_get_undefined(env, &jsContext->error), JS_ERR_PARAMETER_INVALID);
    napi_value result = SelectResultNapi::ParseResult(env, context->pickerCallBack);

    if (result != nullptr) {
        jsContext->data = result;
        jsContext->status = true;
    } else {
        MediaLibraryNapiUtils::CreateNapiErrorObject(env, jsContext->error, ERR_MEM_ALLOCATION,
            "failed to create js object");
    }
    if (context->work != nullptr) {
        MediaLibraryNapiUtils::InvokeJSAsyncMethod(env, context->deferred, context->callbackRef,
            context->work, *jsContext);
    }
    delete context;
}

static napi_value StartPickerExtension(napi_env env, napi_callback_info info,
    unique_ptr<PhotoPickerAsyncContext> &AsyncContext)
{ 
    std::string MIMEType = Plugin::PluginUtilsNApi::GetStringFromValueUtf8(env, AsyncContext->MIMEType);
    PhotoPickerCallback::pickerCallBack = AsyncContext->pickerCallBack;
    UserFileClient::startPhotoPicker(MIMEType);
    return nullptr;
}

static napi_value ParseArgsStartPhotoPicker(napi_env env, napi_callback_info info,
    unique_ptr<PhotoPickerAsyncContext> &context)
{
    constexpr size_t minArgs = ARGS_ONE;
    constexpr size_t maxArgs = ARGS_TWO;
    CHECK_ARGS(env, MediaLibraryNapiUtils::AsyncContextSetObjectInfo(env, info, context, minArgs, maxArgs),
        JS_ERR_PARAMETER_INVALID);

    napi_value args = context->argv[ARGS_ZERO];
    napi_value MIMEType = MediaLibraryNapiUtils::GetPropertyValueByName(env, args, "MIMEType");
    if (MIMEType == nullptr) {
        MIMEType = Plugin::PluginUtilsNApi::CreateStringUtf8(env, ALL_MIME_TYPE);
    }
    context->MIMEType = MIMEType;
    NAPI_CALL(env, MediaLibraryNapiUtils::GetParamCallback(env, context));
    if (context->pickerCallBack == nullptr) {
        context->pickerCallBack = std::make_unique<PickerCallBack>();
    }
    CHECK_NULLPTR_RET(StartPickerExtension(env, info, context));
    napi_value result = nullptr;
    CHECK_ARGS(env, napi_get_boolean(env, true, &result), JS_INNER_FAIL);
    return result;
}

napi_value PhotoPickerNapi::StartPhotoPicker(napi_env env, napi_callback_info info)
{
    unique_ptr<PhotoPickerAsyncContext> asyncContext = make_unique<PhotoPickerAsyncContext>();
    auto pickerCallBack = make_shared<PickerCallBack>();
    asyncContext->resultNapiType = ResultNapiType::TYPE_PHOTOACCESS_HELPER;
    ParseArgsStartPhotoPicker(env, info, asyncContext);
    return MediaLibraryNapiUtils::NapiCreateAsyncWork(env, asyncContext, "StartPhotoPicker",
        StartPhotoPickerExecute, StartPhotoPickerAsyncCallbackComplete);
}

napi_value PhotoPickerNapi::SelectOptionsInit(napi_env env, napi_value exports) {
    const vector<napi_property_descriptor> staticProps = {
        DECLARE_NAPI_PROPERTY("PhotoViewMIMETypes", CreateMIMETypesEnum(env)),
    };
    MediaLibraryNapiUtils::NapiAddStaticProps(env, exports, staticProps);

    NapiClassInfo info = {
        PHOTO_SELECT_OPTION_CLASS_NAME,
        &sPhotoSelectConstructor_,
        PhotoPickerNapiConstructor,
        {
        }
    };
    MediaLibraryNapiUtils::NapiDefineClass(env, exports, info);
    return exports;
}

napi_value SelectResultNapi::SelectResultInit(napi_env env, napi_value exports) {
    NapiClassInfo info = {
        PHOTO_SELECT_RESULT_CLASS_NAME,
        &sSelectResultConstructor_,
        SelectResultNapiConstructor,
        {
        }
    };
    MediaLibraryNapiUtils::NapiDefineClass(env, exports, info);
    return exports;
}

napi_value PhotoPickerNapi::PhotoPickerInit(napi_env env, napi_value exports) {
    NapiClassInfo info = {
        PHOTO_PICKER_NAPI_CLASS_NAME,
        &sPhotoViewPickerConstructor_,
        PhotoPickerNapiConstructor,
        {
            DECLARE_NAPI_FUNCTION("select", StartPhotoPicker),
        }
    };
    MediaLibraryNapiUtils::NapiDefineClass(env, exports, info);
    SelectOptionsInit(env, exports);
    SelectResultNapi::SelectResultInit(env, exports);
    return exports;
}
}
}