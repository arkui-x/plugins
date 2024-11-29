/*
 * Copyright (C) 2021-2024 Huawei Device Co., Ltd.
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
#define MLOG_TAG "FetchResultNapi"

#include "fetch_file_result_napi.h"

#include "result_set.h"
#include "medialibrary_client_errno.h"
#include "log.h"
#include "photo_album_napi.h"
#include "file_asset.h"
#include "file_asset_napi.h"

using namespace std;

namespace OHOS {
namespace Media {
thread_local napi_ref FetchFileResultNapi::sConstructor_ = nullptr;
thread_local napi_ref FetchFileResultNapi::userFileMgrConstructor_ = nullptr;
thread_local napi_ref FetchFileResultNapi::photoAccessHelperConstructor_ = nullptr;

FetchFileResultNapi::FetchFileResultNapi()
    : env_(nullptr) {}

FetchFileResultNapi::~FetchFileResultNapi()
{
    propertyPtr = nullptr;
}

void FetchFileResultNapi::FetchFileResultNapiDestructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    FetchFileResultNapi *fetchFileResultObj = reinterpret_cast<FetchFileResultNapi*>(nativeObject);
    if (fetchFileResultObj != nullptr) {
        delete fetchFileResultObj;
        fetchFileResultObj = nullptr;
    }
}

void FetchFileResultNapi::GetFetchResult(unique_ptr<FetchFileResultNapi> &obj)
{
    switch (sFetchResType_) {
        case FetchResType::TYPE_FILE: {
            auto fileResult = make_shared<FetchResult<FileAsset>>(move(sFetchFileResult_->GetDataShareResultSet()));
            obj->propertyPtr->fetchFileResult_ = fileResult;
            obj->propertyPtr->fetchFileResult_->SetInfo(sFetchFileResult_);
            break;
        }
        case FetchResType::TYPE_PHOTOALBUM: {
            auto photoAlbumResult =
                make_shared<FetchResult<PhotoAlbum>>(move(sFetchPhotoAlbumResult_->GetDataShareResultSet()));
            obj->propertyPtr->fetchPhotoAlbumResult_ = photoAlbumResult;
            obj->propertyPtr->fetchPhotoAlbumResult_->SetInfo(sFetchPhotoAlbumResult_);
            break;
        }
        default:
            LOGE("unsupported FetchResType");
            break;
    }
}

// Constructor callback
napi_value FetchFileResultNapi::FetchFileResultNapiConstructor(napi_env env, napi_callback_info info)
{

    napi_status status;
    napi_value result = nullptr;
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &result);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        LOGE("Get Js obj failed, status: %{public}d, (thisVar == nullptr) = %{public}d",
            status, (thisVar == nullptr));
        return result;
    }

    unique_ptr<FetchFileResultNapi> obj = make_unique<FetchFileResultNapi>();
    if (obj == nullptr) {
        LOGE("Get FetchFileResultNapi failed");
        return result;
    }
    obj->env_ = env;
    obj->propertyPtr = make_shared<FetchResultProperty>();
    GetFetchResult(obj);
    obj->propertyPtr->fetchResType_ = sFetchResType_;
    status = napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()),
                       FetchFileResultNapi::FetchFileResultNapiDestructor, nullptr, nullptr);
    if (status == napi_ok) {
        obj.release();
        return thisVar;
    } else {
        LOGE("Failure wrapping js to native napi, status: %{public}d", status);
    }
    return result;
}

FetchResType FetchFileResultNapi::GetFetchResType()
{
    return propertyPtr->fetchResType_;
}

void FetchFileResultNapi::SolveConstructorRef(unique_ptr<FetchResult<FileAsset>> &fileResult,
    napi_ref &constructorRef)
{
    switch(fileResult->GetResultNapiType()) {
        case ResultNapiType::TYPE_USERFILE_MGR: {
            constructorRef = userFileMgrConstructor_;
            break;
        }
        case ResultNapiType::TYPE_PHOTOACCESS_HELPER: {
            constructorRef = photoAccessHelperConstructor_;
            break;
        }
        default:
            constructorRef = sConstructor_;
            break;
    }
}

void FetchFileResultNapi::SolveConstructorRef(unique_ptr<FetchResult<PhotoAlbum>> &fileResult,
    napi_ref &constructorRef)
{
    switch(fileResult->GetResultNapiType()) {
        case ResultNapiType::TYPE_USERFILE_MGR: {
            constructorRef = userFileMgrConstructor_;
            break;
        }
        case ResultNapiType::TYPE_PHOTOACCESS_HELPER: {
            constructorRef = photoAccessHelperConstructor_;
            break;
        }
        default:
            constructorRef = sConstructor_;
            break;
    }
}

napi_value FetchFileResultNapi::CreateFetchFileResult(napi_env env, unique_ptr<FetchResult<FileAsset>> fileResult)
{
    
    napi_value constructor;
    napi_ref constructorRef;

    FetchFileResultNapi::SolveConstructorRef(fileResult, constructorRef);
    NAPI_CALL(env, napi_get_reference_value(env, constructorRef, &constructor));
    sFetchResType_ = fileResult->GetFetchResType();
    sFetchFileResult_ = move(fileResult);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_new_instance(env, constructor, 0, nullptr, &result));
    sFetchFileResult_ = nullptr;
    return result;
}

napi_value FetchFileResultNapi::CreateFetchFileResult(napi_env env, unique_ptr<FetchResult<PhotoAlbum>> fileResult)
{
    napi_value constructor;
    napi_ref constructorRef;
    FetchFileResultNapi::SolveConstructorRef(fileResult, constructorRef);
    NAPI_CALL(env, napi_get_reference_value(env, constructorRef, &constructor));
    sFetchResType_ = fileResult->GetFetchResType();
    sFetchPhotoAlbumResult_ = move(fileResult);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_new_instance(env, constructor, 0, nullptr, &result));
    sFetchPhotoAlbumResult_ = nullptr;
    return result;
}

std::shared_ptr<FetchResult<FileAsset>> FetchFileResultNapi::GetFetchFileResult() const
{
    return propertyPtr->fetchFileResult_;
}

napi_value FetchFileResultNapi::PhotoAccessHelperInit(napi_env env, napi_value exports)
{
    NapiClassInfo info = {
        .name = PAH_FETCH_FILE_RESULT_CLASS_NAME,
        .ref = &photoAccessHelperConstructor_,
        .constructor = FetchFileResultNapiConstructor,
        .props = {
            DECLARE_NAPI_FUNCTION("getCount", JSGetCount),
            DECLARE_NAPI_FUNCTION("isAfterLast", JSIsAfterLast),
            DECLARE_NAPI_FUNCTION("getFirstObject", JSGetFirstObject),
            DECLARE_NAPI_FUNCTION("getNextObject", JSGetNextObject),
            DECLARE_NAPI_FUNCTION("getLastObject", JSGetLastObject),
            DECLARE_NAPI_FUNCTION("getObjectByPosition", JSGetPositionObject),
            DECLARE_NAPI_FUNCTION("getAllObjects", JSGetAllObject),
            DECLARE_NAPI_FUNCTION("close", JSClose)
        }
    };
    MediaLibraryNapiUtils::NapiDefineClass(env, exports, info);
    return exports;
}

static bool CheckIfFFRNapiNotEmpty(FetchFileResultNapi* obj)
{
    if (obj == nullptr) {
        LOGI("FetchFileResultNapi is nullptr");
        return false;
    }
    if (obj->CheckIfPropertyPtrNull()) {
        LOGI("PropertyPtr in FetchFileResultNapi is nullptr");
        return false;
    }
    return true;
}

napi_value FetchFileResultNapi::JSGetCount(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    FetchFileResultNapi* obj = nullptr;
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &jsResult);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        LOGE("JSGetCount Invalid arguments!, status: %{public}d", status);
        NAPI_ASSERT(env, false, "JSGetCount thisVar == nullptr");
    }

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status == napi_ok) && CheckIfFFRNapiNotEmpty(obj)) {
        int32_t count = 0;
        switch (obj->GetFetchResType()) {
            case FetchResType::TYPE_FILE:
                count = obj->GetFetchFileResultObject()->GetCount();
                break;
            case FetchResType::TYPE_PHOTOALBUM:
                count = obj->GetFetchPhotoAlbumResultObject()->GetCount();
                break;
            default:
                LOGE("unsupported FetchResType");
                break;
        }
        if (count < 0) {
            NapiError::ThrowError(env, JS_INNER_FAIL, "Failed to get count");
            return nullptr;
        }
        napi_create_int32(env, count, &jsResult);
    } else {
        NapiError::ThrowError(env, JS_ERR_PARAMETER_INVALID, "Failed to get native obj");
        return nullptr;
    }

    return jsResult;
}

napi_value FetchFileResultNapi::JSIsAfterLast(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    FetchFileResultNapi* obj = nullptr;
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &jsResult);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        LOGE("JSIsAfterLast Invalid arguments!, status: %{public}d", status);
        NAPI_ASSERT(env, false, "JSIsAfterLast thisVar == nullptr");
    }

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status == napi_ok) && CheckIfFFRNapiNotEmpty(obj)) {
        bool isAfterLast = false;
        switch (obj->GetFetchResType()) {
            case FetchResType::TYPE_FILE:
                isAfterLast = obj->GetFetchFileResultObject()->IsAtLastRow();
                break;
            case FetchResType::TYPE_PHOTOALBUM:
                isAfterLast = obj->GetFetchPhotoAlbumResultObject()->IsAtLastRow();
                break;

            default:
                LOGE("unsupported FetchResType");
                break;
        }
        napi_get_boolean(env, isAfterLast, &jsResult);
    } else {
        LOGE("JSIsAfterLast obj == nullptr, status: %{public}d", status);
        NAPI_ASSERT(env, false, "JSIsAfterLast obj == nullptr");
    }

    return jsResult;
}

static void GetNapiResFromAsset(napi_env env, FetchFileResultAsyncContext *context,
    unique_ptr<JSAsyncContextOutput> &jsContext)
{
    napi_value jsAsset;
    switch (context->objectPtr->fetchResType_) {
        case FetchResType::TYPE_FILE:
            jsAsset = FileAssetNapi::CreateFileAsset(env, context->fileAsset);
            break;
        case FetchResType::TYPE_PHOTOALBUM:
            jsAsset = PhotoAlbumNapi::CreatePhotoAlbumNapi(env, context->photoAlbum);
            break;
        default:
            LOGE("unsupported FetchResType");
            break;
    }

    if (jsAsset == nullptr) {
        LOGE("Failed to get file asset napi object");
        napi_get_undefined(env, &jsContext->data);
        MediaLibraryNapiUtils::CreateNapiErrorObject(env, jsContext->error, JS_INNER_FAIL,
            "System inner fail");
    } else {
        jsContext->data = jsAsset;
        napi_get_undefined(env, &jsContext->error);
        jsContext->status = true;
    }
}

static void GetPositionObjectCompleteCallback(napi_env env, napi_status status, FetchFileResultAsyncContext* context)
{
  
    CHECK_NULL_PTR_RETURN_VOID(context, "Async context is null");

    unique_ptr<JSAsyncContextOutput> jsContext = make_unique<JSAsyncContextOutput>();
    jsContext->status = false;

    GetNapiResFromAsset(env, context, jsContext);

    if (context->work != nullptr) {
        MediaLibraryNapiUtils::InvokeJSAsyncMethod(env, context->deferred, context->callbackRef,
                                                   context->work, *jsContext);
    }

    delete context;
}

napi_value FetchFileResultNapi::JSGetFirstObject(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_value resource = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc <= ARGS_ONE, "requires 1 parameter");
    napi_get_undefined(env, &result);

    unique_ptr<FetchFileResultAsyncContext> asyncContext = make_unique<FetchFileResultAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&asyncContext->objectInfo));
    if (status == napi_ok && CheckIfFFRNapiNotEmpty(asyncContext->objectInfo)) {
        if (argc == ARGS_ONE) {
            GET_JS_ASYNC_CB_REF(env, argv[PARAM0], refCount, asyncContext->callbackRef);
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "JSGetFirstObject", asyncContext);

        asyncContext->objectPtr = asyncContext->objectInfo->propertyPtr;
        CHECK_NULL_PTR_RETURN_UNDEFINED(env, asyncContext->objectPtr, result, "propertyPtr is nullptr");

        status = napi_create_async_work(
            env, nullptr, resource, [](napi_env env, void *data) {
                auto context = static_cast<FetchFileResultAsyncContext*>(data);
                context->GetFirstAsset();
            },
            reinterpret_cast<napi_async_complete_callback>(GetPositionObjectCompleteCallback),
            static_cast<void *>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            napi_get_undefined(env, &result);
        } else {
            napi_queue_async_work(env, asyncContext->work);
            asyncContext.release();
        }
    } else {
        LOGE("JSGetFirstObject obj == nullptr, status: %{public}d", status);
        NAPI_ASSERT(env, false, "JSGetFirstObject obj == nullptr");
    }

    return result;
}

napi_value FetchFileResultNapi::JSGetNextObject(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_value resource = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc <= ARGS_ONE, "requires 1 parameter");

    napi_get_undefined(env, &result);
    unique_ptr<FetchFileResultAsyncContext> asyncContext = make_unique<FetchFileResultAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&asyncContext->objectInfo));
    if (status == napi_ok && CheckIfFFRNapiNotEmpty(asyncContext->objectInfo)) {
        if (argc == ARGS_ONE) {
            GET_JS_ASYNC_CB_REF(env, argv[PARAM0], refCount, asyncContext->callbackRef);
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "JSGetNextObject", asyncContext);

        asyncContext->objectPtr = asyncContext->objectInfo->propertyPtr;
        CHECK_NULL_PTR_RETURN_UNDEFINED(env, asyncContext->objectPtr, result, "propertyPtr is nullptr");

        status = napi_create_async_work(
            env, nullptr, resource, [](napi_env env, void *data) {
                auto context = static_cast<FetchFileResultAsyncContext*>(data);

                context->GetNextObject();
            },
            reinterpret_cast<napi_async_complete_callback>(GetPositionObjectCompleteCallback),
            static_cast<void *>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            napi_get_undefined(env, &result);
        } else {
            napi_queue_async_work(env, asyncContext->work);
            asyncContext.release();
        }
    } else {
        LOGE("JSGetNextObject obj == nullptr, status: %{public}d", status);
        NAPI_ASSERT(env, false, "JSGetNextObject obj == nullptr");
    }

    return result;
}

napi_value FetchFileResultNapi::JSGetLastObject(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_value resource = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc <= ARGS_ONE, "requires 1 parameter");

    napi_get_undefined(env, &result);
    unique_ptr<FetchFileResultAsyncContext> asyncContext = make_unique<FetchFileResultAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&asyncContext->objectInfo));
    if (status == napi_ok && CheckIfFFRNapiNotEmpty(asyncContext->objectInfo)) {
        if (argc == ARGS_ONE) {
            GET_JS_ASYNC_CB_REF(env, argv[PARAM0], refCount, asyncContext->callbackRef);
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "JSGetLastObject", asyncContext);

        asyncContext->objectPtr = asyncContext->objectInfo->propertyPtr;
        CHECK_NULL_PTR_RETURN_UNDEFINED(env, asyncContext->objectPtr, result, "propertyPtr is nullptr");

        status = napi_create_async_work(
            env, nullptr, resource, [](napi_env env, void *data) {
                auto context = static_cast<FetchFileResultAsyncContext*>(data);
                context->GetLastObject();
            },
            reinterpret_cast<napi_async_complete_callback>(GetPositionObjectCompleteCallback),
            static_cast<void *>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            napi_get_undefined(env, &result);
        } else {
            napi_queue_async_work(env, asyncContext->work);
            asyncContext.release();
        }
    } else {
        LOGE("JSGetLastObject obj == nullptr, status: %{public}d", status);
        NAPI_ASSERT(env, false, "JSGetLastObject obj == nullptr");
    }

    return result;
}

napi_value FetchFileResultNapi::JSGetPositionObject(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_valuetype type = napi_undefined;
    napi_value resource = nullptr;
    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, (argc == ARGS_ONE || argc == ARGS_TWO), "requires 2 parameter maximum");

    napi_get_undefined(env, &result);
    unique_ptr<FetchFileResultAsyncContext> asyncContext = make_unique<FetchFileResultAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&asyncContext->objectInfo));
    if (status == napi_ok && CheckIfFFRNapiNotEmpty(asyncContext->objectInfo)) {
        // Check the arguments and their types
        napi_typeof(env, argv[PARAM0], &type);
        if (type == napi_number) {
            napi_get_value_int32(env, argv[PARAM0], &(asyncContext->position));
        } else {
            LOGE("Argument mismatch, type: %{public}d", type);
            return result;
        }

        if (argc == ARGS_TWO) {
            GET_JS_ASYNC_CB_REF(env, argv[PARAM1], refCount, asyncContext->callbackRef);
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "JSGetPositionObject", asyncContext);

        asyncContext->objectPtr = asyncContext->objectInfo->propertyPtr;
        CHECK_NULL_PTR_RETURN_UNDEFINED(env, asyncContext->objectPtr, result, "propertyPtr is nullptr");

        status = napi_create_async_work(
            env, nullptr, resource, [](napi_env env, void *data) {
                auto context = static_cast<FetchFileResultAsyncContext*>(data);
                context->GetObjectAtPosition();
            },
            reinterpret_cast<napi_async_complete_callback>(GetPositionObjectCompleteCallback),
            static_cast<void *>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            napi_get_undefined(env, &result);
        } else {
            napi_queue_async_work(env, asyncContext->work);
            asyncContext.release();
        }
    } else {
        LOGE("JSGetPositionObject obj == nullptr, status: %{public}d", status);
        NAPI_ASSERT(env, false, "JSGetPositionObject obj == nullptr");
    }

    return result;
}

static napi_value GetAsset(napi_env env, vector<std::unique_ptr<FileAsset>> &array, int index)
{
    return FileAssetNapi::CreateFileAsset(env, array[index]);
}

static napi_value GetAsset(napi_env env, vector<std::unique_ptr<PhotoAlbum>> &array, int index)
{
    return PhotoAlbumNapi::CreatePhotoAlbumNapi(env, array[index]);
}

template<class T>
static void GetAssetFromArray(napi_env env, FetchFileResultAsyncContext* context, T& array,
    unique_ptr<JSAsyncContextOutput> &jsContext)
{
    napi_value jsFileArray = nullptr;
    napi_create_array_with_length(env, array.size(), &jsFileArray);
    napi_value jsFileAsset = nullptr;
    size_t i = 0;
    for (i = 0; i < array.size(); i++) {
        jsFileAsset = GetAsset(env, array, i);
        if ((jsFileAsset == nullptr) || (napi_set_element(env, jsFileArray, i, jsFileAsset) != napi_ok)) {
            LOGE("Failed to get file asset napi object");
            napi_get_undefined(env, &jsContext->data);
            MediaLibraryNapiUtils::CreateNapiErrorObject(env, jsContext->error, ERR_MEM_ALLOCATION,
                "Failed to create js object");
            break;
        }
    }
    if (i == array.size()) {
        jsContext->data = jsFileArray;
        napi_get_undefined(env, &jsContext->error);
        jsContext->status = true;
    }
}

static void GetAllObjectCompleteCallback(napi_env env, napi_status status, FetchFileResultAsyncContext* context)
{
    CHECK_NULL_PTR_RETURN_VOID(context, "Async context is null");
    unique_ptr<JSAsyncContextOutput> jsContext = make_unique<JSAsyncContextOutput>();
    jsContext->status = false;

    switch (context->objectPtr->fetchResType_) {
        case FetchResType::TYPE_FILE:
            GetAssetFromArray(env, context, context->fileAssetArray, jsContext);
            break;
        case FetchResType::TYPE_PHOTOALBUM:
            GetAssetFromArray(env, context, context->filePhotoAlbumArray, jsContext);
            break;
        default:
            LOGE("unsupported FetchResType");
            napi_get_undefined(env, &jsContext->data);
            MediaLibraryNapiUtils::CreateNapiErrorObject(env, jsContext->error, ERR_INVALID_OUTPUT,
                "Failed to obtain fileAsset array from DB");
    }

    if (context->work != nullptr) {
        MediaLibraryNapiUtils::InvokeJSAsyncMethod(env, context->deferred, context->callbackRef,
                                                   context->work, *jsContext);
    }

    delete context;
}

std::shared_ptr<FetchResult<FileAsset>> FetchFileResultNapi::GetFetchFileResultObject()
{
    return propertyPtr->fetchFileResult_;
}

std::shared_ptr<FetchResult<PhotoAlbum>> FetchFileResultNapi::GetFetchPhotoAlbumResultObject()
{
    return propertyPtr->fetchPhotoAlbumResult_;
}

void GetAllObjectFromFetchResult(const FetchFileResultAsyncContext &asyncContext)
{
  
    FetchFileResultAsyncContext *context = const_cast<FetchFileResultAsyncContext *>(&asyncContext);
    context->GetAllObjectFromFetchResult();
}

napi_value FetchFileResultNapi::JSGetAllObject(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_value resource = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc <= ARGS_ONE, "requires 1 parameter maximum");

    napi_get_undefined(env, &result);
    unique_ptr<FetchFileResultAsyncContext> asyncContext = make_unique<FetchFileResultAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&asyncContext->objectInfo));
    if (status == napi_ok && CheckIfFFRNapiNotEmpty(asyncContext->objectInfo)) {
        if (argc == ARGS_ONE) {
            GET_JS_ASYNC_CB_REF(env, argv[PARAM0], refCount, asyncContext->callbackRef);
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "JSGetAllObject", asyncContext);

        asyncContext->objectPtr = asyncContext->objectInfo->propertyPtr;
        CHECK_NULL_PTR_RETURN_UNDEFINED(env, asyncContext->objectPtr, result, "propertyPtr is nullptr");

        status = napi_create_async_work(
            env, nullptr, resource, [](napi_env env, void *data) {
                auto context = static_cast<FetchFileResultAsyncContext*>(data);
                GetAllObjectFromFetchResult(*context);
            },
            reinterpret_cast<napi_async_complete_callback>(GetAllObjectCompleteCallback),
            static_cast<void *>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            napi_get_undefined(env, &result);
        } else {
            napi_queue_async_work(env, asyncContext->work);
            asyncContext.release();
        }
    } else {
        LOGE("JSGetAllObject obj == nullptr, status: %{public}d", status);
        NAPI_ASSERT(env, false, "JSGetAllObject obj == nullptr");
    }

    return result;
}

napi_value FetchFileResultNapi::JSClose(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    FetchFileResultNapi* obj = nullptr;
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &jsResult);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        LOGE("Invalid arguments!, status: %{public}d", status);
        return jsResult;
    }
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status == napi_ok) && (obj != nullptr)) {
        obj->propertyPtr = nullptr;
    }
    status = napi_remove_wrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status == napi_ok) && (obj != nullptr)) {
        napi_create_int32(env, SUCCESS, &jsResult);
    } else {
        LOGI("JSClose obj == nullptr");
    }

    LOGD("JSClose OUT!");
    return jsResult;
}

void FetchFileResultAsyncContext::GetFirstAsset()
{
    switch (objectPtr->fetchResType_) {
        case FetchResType::TYPE_FILE: {
            LOGE("GetFirstAsset");
            fileAsset = objectPtr->fetchFileResult_->GetFirstObject();
            break;
        }
        case FetchResType::TYPE_PHOTOALBUM: {
            photoAlbum = objectPtr->fetchPhotoAlbumResult_->GetFirstObject();
            break;
        }
        default:
            LOGE("unsupported FetchResType");
            break;
    }
}

void FetchFileResultAsyncContext::GetObjectAtPosition()
{
    switch (objectPtr->fetchResType_) {
        case FetchResType::TYPE_FILE: {
            fileAsset = objectPtr->fetchFileResult_->GetObjectAtPosition(position);
            break;
        }
        case FetchResType::TYPE_PHOTOALBUM: {
            photoAlbum = objectPtr->fetchPhotoAlbumResult_->GetObjectAtPosition(position);
            break;
        }
        default:
            LOGE("unsupported FetchResType");
            break;
    }
}

void FetchFileResultAsyncContext::GetLastObject()
{
    switch (objectPtr->fetchResType_) {
        case FetchResType::TYPE_FILE: {
            fileAsset = objectPtr->fetchFileResult_->GetLastObject();
            break;
        }
        case FetchResType::TYPE_PHOTOALBUM: {
            photoAlbum = objectPtr->fetchPhotoAlbumResult_->GetLastObject();
            break;
        }
        default:
            LOGE("unsupported FetchResType");
            break;
    }
}

void FetchFileResultAsyncContext::GetNextObject()
{
    switch (objectPtr->fetchResType_) {
        case FetchResType::TYPE_FILE: {
            fileAsset = objectPtr->fetchFileResult_->GetNextObject();
            break;
        }
        case FetchResType::TYPE_PHOTOALBUM: {
            photoAlbum = objectPtr->fetchPhotoAlbumResult_->GetNextObject();
            break;
        }
        default:
            LOGE("unsupported FetchResType");
            break;
    }
}

void FetchFileResultAsyncContext::GetAllObjectFromFetchResult()
{
    switch (objectPtr->fetchResType_) {
        case FetchResType::TYPE_FILE: {
            auto fetchResult = objectPtr->fetchFileResult_;
            auto file = fetchResult->GetFirstObject();
            int index = 0;
            while (file != nullptr) {
                fileAssetArray.push_back(move(file));
                file = fetchResult->GetNextObject();
            }
            break;
        }
        case FetchResType::TYPE_PHOTOALBUM: {
            auto fetchResult = objectPtr->fetchPhotoAlbumResult_;
            auto photoAlbum = fetchResult->GetFirstObject();
            while (photoAlbum != nullptr) {
                filePhotoAlbumArray.push_back(move(photoAlbum));
                photoAlbum = fetchResult->GetNextObject();
            }
            break;
        }
        default:
            LOGE("unsupported FetchResType");
            break;
    }
}

bool FetchFileResultNapi::CheckIfPropertyPtrNull()
{
    return propertyPtr == nullptr;
}
} // namespace Media
} // namespace OHOS