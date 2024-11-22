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
#include <algorithm>
#define MLOG_TAG "PhotoAlbumNapi"

#include <nlohmann/json.hpp>

#include "photo_album_napi.h"
#include "fetch_file_result_napi.h"
#include "file_asset_napi.h"
#include "medialibrary_client_errno.h"
#include "log.h"
#include "userfile_client.h"
#include "userfilemgr_uri.h"
#include "userfile_manager_types.h"

using namespace std;
using namespace OHOS::DataShare;

namespace OHOS::Media {
thread_local PhotoAlbum *PhotoAlbumNapi::pAlbumData_ = nullptr;
thread_local napi_ref PhotoAlbumNapi::constructor_ = nullptr;
thread_local napi_ref PhotoAlbumNapi::photoAccessConstructor_ = nullptr;
static const string PHOTO_ALBUM_CLASS = "UserFileMgrPhotoAlbum";
static const string PHOTOACCESS_PHOTO_ALBUM_CLASS = "PhotoAccessPhotoAlbum";
static const string COUNT_GROUP_BY = "count(*) AS count";

struct TrashAlbumExecuteOpt {
    napi_env env;
    void *data;
    string tracerLabel;
    string uri;
};

using CompleteCallback = napi_async_complete_callback;

PhotoAlbumNapi::PhotoAlbumNapi() : env_(nullptr) {}

PhotoAlbumNapi::~PhotoAlbumNapi() = default;

napi_value PhotoAlbumNapi::PhotoAccessInit(napi_env env, napi_value exports)
{
    NapiClassInfo info = {
        .name = PHOTOACCESS_PHOTO_ALBUM_CLASS,
        .ref = &photoAccessConstructor_,
        .constructor = PhotoAlbumNapiConstructor,
        .props = {
            DECLARE_NAPI_GETTER_SETTER("albumName", JSPhotoAccessGetAlbumName, JSPhotoAccessSetAlbumName),
            DECLARE_NAPI_GETTER("albumUri", JSPhotoAccessGetAlbumUri),
            DECLARE_NAPI_GETTER("count", JSPhotoAccessGetAlbumCount),
            DECLARE_NAPI_GETTER("imageCount", JSPhotoAccessGetAlbumImageCount),
            DECLARE_NAPI_GETTER("videoCount", JSPhotoAccessGetAlbumVideoCount),
            DECLARE_NAPI_GETTER("albumType", JSGetPhotoAlbumType),
            DECLARE_NAPI_GETTER("albumSubtype", JSGetPhotoAlbumSubType),
            DECLARE_NAPI_FUNCTION("commitModify", PhotoAccessHelperCommitModify),
            DECLARE_NAPI_FUNCTION("getAssets", JSPhoteAccessGetPhotoAssets),
        }
    };

    MediaLibraryNapiUtils::NapiDefineClass(env, exports, info);
    return exports;
}

napi_value PhotoAlbumNapi::CreatePhotoAlbumNapi(napi_env env, unique_ptr<PhotoAlbum> &albumData)
{
    if (albumData == nullptr) {
        return nullptr;
    }

    napi_value constructor;
    napi_ref constructorRef;
    constructorRef = photoAccessConstructor_;
    CHECK_ARGS(env, napi_get_reference_value(env, constructorRef, &constructor), JS_INNER_FAIL);

    napi_value result = nullptr;
    pAlbumData_ = albumData.release();
    CHECK_ARGS(env, napi_new_instance(env, constructor, 0, nullptr, &result), JS_INNER_FAIL);
    pAlbumData_ = nullptr;
    return result;
}

napi_value PhotoAlbumNapi::CreatePhotoAlbumNapi(napi_env env, shared_ptr<PhotoAlbum>& albumData)
{
    if (albumData == nullptr) {
        LOGE("Unsupported photo album data");
        return nullptr;
    }

    napi_value constructor = nullptr;
    napi_value result = nullptr;
    CHECK_ARGS(env, napi_get_reference_value(env, photoAccessConstructor_, &constructor), JS_INNER_FAIL);
    CHECK_ARGS(env, napi_new_instance(env, constructor, 0, nullptr, &result), JS_INNER_FAIL);
    CHECK_COND(env, result != nullptr, JS_INNER_FAIL);

    PhotoAlbumNapi* photoAlbumNapi = nullptr;
    CHECK_ARGS(env, napi_unwrap(env, result, reinterpret_cast<void**>(&photoAlbumNapi)), JS_INNER_FAIL);
    CHECK_COND(env, photoAlbumNapi != nullptr, JS_INNER_FAIL);
    photoAlbumNapi->photoAlbumPtr = albumData;
    return result;
}

int32_t PhotoAlbumNapi::GetAlbumId() const
{
    return photoAlbumPtr->GetAlbumId();
}

int32_t PhotoAlbumNapi::GetCount() const
{
    return photoAlbumPtr->GetCount();
}

void PhotoAlbumNapi::SetCount(int32_t count)
{
    return photoAlbumPtr->SetCount(count);
}

int32_t PhotoAlbumNapi::GetImageCount() const
{
    return photoAlbumPtr->GetImageCount();
}

void PhotoAlbumNapi::SetImageCount(int32_t count)
{
    return photoAlbumPtr->SetImageCount(count);
}

int32_t PhotoAlbumNapi::GetVideoCount() const
{
    return photoAlbumPtr->GetVideoCount();
}

void PhotoAlbumNapi::SetVideoCount(int32_t count)
{
    return photoAlbumPtr->SetVideoCount(count);
}

const string& PhotoAlbumNapi::GetAlbumUri() const
{
    return photoAlbumPtr->GetAlbumUri();
}

const string& PhotoAlbumNapi::GetCoverUri() const
{
    return photoAlbumPtr->GetCoverUri();
}

int64_t PhotoAlbumNapi::GetDateModified() const
{
    return photoAlbumPtr->GetDateModified();
}

const string& PhotoAlbumNapi::GetAlbumName() const
{
    return photoAlbumPtr->GetAlbumName();
}

const string& PhotoAlbumNapi::GetAlbumLocalIdentifier() const
{
    return photoAlbumPtr->GetAlbumLocalIdentifier();
}

PhotoAlbumType PhotoAlbumNapi::GetPhotoAlbumType() const
{
    return photoAlbumPtr->GetPhotoAlbumType();
}

PhotoAlbumSubType PhotoAlbumNapi::GetPhotoAlbumSubType() const
{
    return photoAlbumPtr->GetPhotoAlbumSubType();
}

double PhotoAlbumNapi::GetLatitude() const
{
    return photoAlbumPtr->GetLatitude();
}

double PhotoAlbumNapi::GetLongitude() const
{
    return photoAlbumPtr->GetLongitude();
}

shared_ptr<PhotoAlbum> PhotoAlbumNapi::GetPhotoAlbumInstance() const
{
    return photoAlbumPtr;
}

bool PhotoAlbumNapi::GetHiddenOnly() const
{
    return photoAlbumPtr->GetHiddenOnly();
}

void PhotoAlbumNapi::SetHiddenOnly(const bool hiddenOnly_)
{
    return photoAlbumPtr->SetHiddenOnly(hiddenOnly_);
}

void PhotoAlbumNapi::SetPhotoAlbumNapiProperties()
{
    photoAlbumPtr = shared_ptr<PhotoAlbum>(pAlbumData_);
}

// Constructor callback
napi_value PhotoAlbumNapi::PhotoAlbumNapiConstructor(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    CHECK_ARGS(env, napi_get_undefined(env, &result), JS_INNER_FAIL);

    napi_value thisVar = nullptr;
    CHECK_ARGS(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr), JS_INNER_FAIL);
    if (thisVar == nullptr) {
        NapiError::ThrowError(env, JS_ERR_PARAMETER_INVALID);
        return result;
    }

    unique_ptr<PhotoAlbumNapi> obj = make_unique<PhotoAlbumNapi>();
    obj->env_ = env;
    if (pAlbumData_ != nullptr) {
        obj->SetPhotoAlbumNapiProperties();
    }
    CHECK_ARGS(env, napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()),
        PhotoAlbumNapi::PhotoAlbumNapiDestructor, nullptr, nullptr), JS_INNER_FAIL);
    obj.release();
    return thisVar;
}

void PhotoAlbumNapi::PhotoAlbumNapiDestructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    auto *album = reinterpret_cast<PhotoAlbumNapi*>(nativeObject);
    if (album != nullptr) {
        delete album;
        album = nullptr;
    }
}

napi_value UnwrapPhotoAlbumObject(napi_env env, napi_callback_info info, PhotoAlbumNapi** obj)
{
    napi_value result = nullptr;
    CHECK_ARGS(env, napi_get_undefined(env, &result), JS_INNER_FAIL);

    napi_value thisVar = nullptr;
    CHECK_ARGS(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr), JS_INNER_FAIL);
    if (thisVar == nullptr) {
        NapiError::ThrowError(env, JS_ERR_PARAMETER_INVALID);
        return result;
    }

    CHECK_ARGS(env, napi_unwrap(env, thisVar, reinterpret_cast<void **>(obj)), JS_INNER_FAIL);
    if (obj == nullptr) {
        NapiError::ThrowError(env, JS_ERR_PARAMETER_INVALID);
        return result;
    }

    CHECK_ARGS(env, napi_get_boolean(env, true, &result), JS_INNER_FAIL);
    return result;
}

napi_value PhotoAlbumNapi::JSPhotoAccessGetAlbumName(napi_env env, napi_callback_info info)
{
    PhotoAlbumNapi *obj = nullptr;
    CHECK_NULLPTR_RET(UnwrapPhotoAlbumObject(env, info, &obj));
    LOGE("JSPhotoAccessGetAlbumName");
    napi_value jsResult = nullptr;
    CHECK_ARGS(env, napi_create_string_utf8(env, obj->GetAlbumName().c_str(), NAPI_AUTO_LENGTH, &jsResult),
        JS_INNER_FAIL);
    return jsResult;
}

napi_value PhotoAlbumNapi::JSPhotoAccessGetAlbumUri(napi_env env, napi_callback_info info)
{
    PhotoAlbumNapi *obj = nullptr;
    CHECK_NULLPTR_RET(UnwrapPhotoAlbumObject(env, info, &obj));

    napi_value jsResult = nullptr;
    CHECK_ARGS(env, napi_create_string_utf8(env, obj->GetAlbumUri().c_str(), NAPI_AUTO_LENGTH, &jsResult),
        JS_INNER_FAIL);
    return jsResult;
}

napi_value PhotoAlbumNapi::JSPhotoAccessGetAlbumCount(napi_env env, napi_callback_info info)
{
    PhotoAlbumNapi *obj = nullptr;
    CHECK_NULLPTR_RET(UnwrapPhotoAlbumObject(env, info, &obj));

    napi_value jsResult = nullptr;
    CHECK_ARGS(env, napi_create_int32(env, obj->GetCount(), &jsResult), JS_INNER_FAIL);
    return jsResult;
}

napi_value PhotoAlbumNapi::JSPhotoAccessGetAlbumImageCount(napi_env env, napi_callback_info info)
{
    PhotoAlbumNapi *obj = nullptr;
    CHECK_NULLPTR_RET(UnwrapPhotoAlbumObject(env, info, &obj));

    napi_value jsResult = nullptr;
    CHECK_ARGS(env, napi_create_int32(env, obj->GetImageCount(), &jsResult), JS_INNER_FAIL);
    return jsResult;
}

napi_value PhotoAlbumNapi::JSPhotoAccessGetAlbumVideoCount(napi_env env, napi_callback_info info)
{
    PhotoAlbumNapi *obj = nullptr;
    CHECK_NULLPTR_RET(UnwrapPhotoAlbumObject(env, info, &obj));

    napi_value jsResult = nullptr;
    CHECK_ARGS(env, napi_create_int32(env, obj->GetVideoCount(), &jsResult), JS_INNER_FAIL);
    return jsResult;
}

napi_value PhotoAlbumNapi::JSGetPhotoAlbumType(napi_env env, napi_callback_info info)
{
    PhotoAlbumNapi *obj = nullptr;
    CHECK_NULLPTR_RET(UnwrapPhotoAlbumObject(env, info, &obj));

    napi_value jsResult = nullptr;
    CHECK_ARGS(env, napi_create_int32(env, obj->GetPhotoAlbumType(), &jsResult), JS_INNER_FAIL);
    return jsResult;
}

napi_value PhotoAlbumNapi::JSGetPhotoAlbumSubType(napi_env env, napi_callback_info info)
{
    PhotoAlbumNapi *obj = nullptr;
    CHECK_NULLPTR_RET(UnwrapPhotoAlbumObject(env, info, &obj));

    napi_value jsResult = nullptr;
    CHECK_ARGS(env, napi_create_int32(env, obj->GetPhotoAlbumSubType(), &jsResult), JS_INNER_FAIL);
    return jsResult;
}

napi_value GetStringArg(napi_env env, napi_callback_info info, PhotoAlbumNapi **obj, string &output)
{
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE];
    napi_value thisVar = nullptr;
    CHECK_ARGS(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr), JS_INNER_FAIL);
    CHECK_COND(env, argc == ARGS_ONE, JS_ERR_PARAMETER_INVALID);

    napi_value result = nullptr;
    CHECK_ARGS(env, napi_get_undefined(env, &result), JS_INNER_FAIL);
    napi_valuetype valueType = napi_undefined;
    if ((thisVar == nullptr) || (napi_typeof(env, argv[PARAM0], &valueType) != napi_ok) || (valueType != napi_string)) {
        NapiError::ThrowError(env, JS_ERR_PARAMETER_INVALID);
        return result;
    }

    size_t res = 0;
    char buffer[FILENAME_MAX];
    CHECK_ARGS(env, napi_get_value_string_utf8(env, argv[PARAM0], buffer, FILENAME_MAX, &res), JS_INNER_FAIL);
    output = string(buffer);

    CHECK_ARGS(env, napi_unwrap(env, thisVar, reinterpret_cast<void **>(obj)), JS_INNER_FAIL);
    if (obj == nullptr) {
        NapiError::ThrowError(env, JS_ERR_PARAMETER_INVALID);
        return result;
    }

    CHECK_ARGS(env, napi_get_boolean(env, true, &result), JS_INNER_FAIL);
    return result;
}

napi_value PhotoAlbumNapi::JSPhotoAccessSetAlbumName(napi_env env, napi_callback_info info)
{
    PhotoAlbumNapi *obj = nullptr;
    string albumName;
    CHECK_NULLPTR_RET(GetStringArg(env, info, &obj, albumName));
    obj->photoAlbumPtr->SetAlbumName(albumName);

    napi_value result = nullptr;
    CHECK_ARGS(env, napi_get_undefined(env, &result), JS_INNER_FAIL);
    return result;
}

static napi_value ParseArgsCommitModify(napi_env env, napi_callback_info info,
    unique_ptr<PhotoAlbumNapiAsyncContext> &context)
{
    constexpr size_t minArgs = ARGS_ZERO;
    constexpr size_t maxArgs = ARGS_ONE;
    CHECK_ARGS(env, MediaLibraryNapiUtils::AsyncContextSetObjectInfo(env, info, context, minArgs, maxArgs),
        JS_ERR_PARAMETER_INVALID);

    auto photoAlbum = context->objectInfo->GetPhotoAlbumInstance();
    if (photoAlbum == nullptr) {
        NapiError::ThrowError(env, JS_ERR_PARAMETER_INVALID);
        return nullptr;
    }
    if (!PhotoAlbum::IsUserPhotoAlbum(photoAlbum->GetPhotoAlbumType(), photoAlbum->GetPhotoAlbumSubType())) {
        NapiError::ThrowError(env, JS_ERR_PARAMETER_INVALID);
        return nullptr;
    }
    context->predicates.EqualTo(PhotoAlbumColumns::ALBUM_ID, to_string(photoAlbum->GetAlbumId()));
    context->valuesBucket.Put(PhotoAlbumColumns::ALBUM_NAME, photoAlbum->GetAlbumName());
    context->valuesBucket.Put(PhotoAlbumColumns::ALBUM_COVER_URI, photoAlbum->GetCoverUri());

    napi_value result = nullptr;
    CHECK_ARGS(env, napi_get_boolean(env, true, &result), JS_INNER_FAIL);
    return result;
}

static void JSCommitModifyExecute(napi_env env, void *data)
{
    auto *context = static_cast<PhotoAlbumNapiAsyncContext*>(data);
    string commitModifyUri = (context->resultNapiType == ResultNapiType::TYPE_USERFILE_MGR) ?
        UFM_UPDATE_PHOTO_ALBUM : PAH_UPDATE_PHOTO_ALBUM;
    std::string uri(commitModifyUri);
    int changedRows = UserFileClient::Update(uri, context->predicates, context->valuesBucket);
    context->SaveError(changedRows);
    context->changedRows = changedRows;
}

static void JSCommitModifyCompleteCallback(napi_env env, napi_status status, void *data)
{
    auto *context = static_cast<PhotoAlbumNapiAsyncContext*>(data);
    auto jsContext = make_unique<JSAsyncContextOutput>();
    jsContext->status = false;
    CHECK_ARGS_RET_VOID(env, napi_get_undefined(env, &jsContext->data), JS_INNER_FAIL);
    if (context->error == ERR_DEFAULT) {
        CHECK_ARGS_RET_VOID(env, napi_get_undefined(env, &jsContext->error), JS_INNER_FAIL);
        jsContext->status = true;
    } else {
        context->HandleError(env, jsContext->error);
    }

    if (context->work != nullptr) {
        MediaLibraryNapiUtils::InvokeJSAsyncMethod(env, context->deferred, context->callbackRef,
                                                   context->work, *jsContext);
    }
    delete context;
}

napi_value PhotoAlbumNapi::PhotoAccessHelperCommitModify(napi_env env, napi_callback_info info)
{
    auto asyncContext = make_unique<PhotoAlbumNapiAsyncContext>();
    CHECK_NULLPTR_RET(ParseArgsCommitModify(env, info, asyncContext));
    asyncContext->resultNapiType = ResultNapiType::TYPE_PHOTOACCESS_HELPER;

    return MediaLibraryNapiUtils::NapiCreateAsyncWork(env, asyncContext, "JSCommitModify", JSCommitModifyExecute,
        JSCommitModifyCompleteCallback);
}

static int32_t GetPredicatesByAlbumTypes(const shared_ptr<PhotoAlbum> &photoAlbum,
    DataSharePredicates &predicates, const bool hiddenOnly)
{
    auto albumId = photoAlbum->GetAlbumId();
    auto subType = photoAlbum->GetPhotoAlbumSubType();
    auto type = photoAlbum->GetPhotoAlbumType();
    if ((!PhotoAlbum::CheckPhotoAlbumType(type)) || (!PhotoAlbum::CheckPhotoAlbumSubType(subType))) {
        return E_INVALID_ARGUMENTS;
    }
#ifdef IOS_PLATFORM
    predicates.EqualTo(PhotoAlbumColumns::ALBUM_LOCAL_IDENTIFIER, photoAlbum->GetAlbumLocalIdentifier());
#endif
    if (PhotoAlbum::IsUserPhotoAlbum(type, subType)) {
        return MediaLibraryNapiUtils::GetUserAlbumPredicates(photoAlbum->GetAlbumName(), predicates, hiddenOnly);
    }

    if ((type != PhotoAlbumType::SYSTEM) || (subType == PhotoAlbumSubType::USER_GENERIC) ||
        (subType == PhotoAlbumSubType::ANY)) {
        return E_INVALID_ARGUMENTS;
    }
    return MediaLibraryNapiUtils::GetSystemAlbumPredicates(subType, predicates, hiddenOnly);
}

static napi_value ParseArgsGetPhotoAssets(napi_env env, napi_callback_info info,
    unique_ptr<PhotoAlbumNapiAsyncContext> &context)
{
    constexpr size_t minArgs = ARGS_ONE;
    constexpr size_t maxArgs = ARGS_TWO;
    CHECK_ARGS(env, MediaLibraryNapiUtils::AsyncContextSetObjectInfo(env, info, context, minArgs, maxArgs),
        JS_ERR_PARAMETER_INVALID);

    /* Parse the first argument */
    CHECK_ARGS(env, MediaLibraryNapiUtils::GetFetchOption(env, context->argv[PARAM0], ASSET_FETCH_OPT, context),
        JS_INNER_FAIL);

    auto photoAlbum = context->objectInfo->GetPhotoAlbumInstance();
    GetPredicatesByAlbumTypes(photoAlbum, context->predicates, photoAlbum->GetHiddenOnly());
    CHECK_NULLPTR_RET(MediaLibraryNapiUtils::AddDefaultAssetColumns(env, context->fetchColumn,
        PhotoColumn::IsPhotoColumn));
    if (photoAlbum->GetHiddenOnly() || photoAlbum->GetPhotoAlbumSubType() == PhotoAlbumSubType::HIDDEN) {
        if (!MediaLibraryNapiUtils::IsSystemApp()) {
            NapiError::ThrowError(env, E_CHECK_SYSTEMAPP_FAIL, "This interface can be called only by system apps");
            return nullptr;
        }
        // sort by hidden time desc if is hidden asset
        context->predicates.IndexedBy(PhotoColumn::PHOTO_HIDDEN_TIME_INDEX);
    }

    napi_value result = nullptr;
    CHECK_ARGS(env, napi_get_boolean(env, true, &result), JS_INNER_FAIL);
    return result;
}

void ConvertColumnsForPortrait(PhotoAlbumNapiAsyncContext *context)
{
    if (context == nullptr) {
        return;
    }
    auto photoAlbum = context->objectInfo->GetPhotoAlbumInstance();
    if (photoAlbum->GetPhotoAlbumSubType() != PhotoAlbumSubType::PORTRAIT) {
        return;
    }
    for (size_t i = 0; i < context->fetchColumn.size(); i++) {
        context->fetchColumn[i] = PhotoColumn::PHOTOS_TABLE + "." + context->fetchColumn[i];
    }
}

static void JSPhotoAccessGetPhotoAssetsExecute(napi_env env, void *data)
{
    auto *context = static_cast<PhotoAlbumNapiAsyncContext *>(data);
    ConvertColumnsForPortrait(context);
    int32_t errCode = 0;
    auto resultSet = UserFileClient::Query(context->predicates, context->fetchColumn, errCode);
    if (resultSet == nullptr) {
        context->SaveError(E_HAS_DB_ERROR);
        return;
    }
    context->fetchResult = make_unique<FetchResult<FileAsset>>(move(resultSet));
    context->fetchResult->SetResultNapiType(ResultNapiType::TYPE_PHOTOACCESS_HELPER);
}

static void GetPhotoMapQueryResult(napi_env env, PhotoAlbumNapiAsyncContext *context,
    unique_ptr<JSAsyncContextOutput> &jsContext)
{
    napi_value fetchRes = FetchFileResultNapi::CreateFetchFileResult(env, move(context->fetchResult));
    if (fetchRes == nullptr) {
        MediaLibraryNapiUtils::CreateNapiErrorObject(env, jsContext->error, ERR_MEM_ALLOCATION,
            "Failed to create js object for FetchFileResult");
        return;
    }
    jsContext->data = fetchRes;
    jsContext->status = true;
}

static void JSGetPhotoAssetsCallbackComplete(napi_env env, napi_status status, void *data)
{
    auto *context = static_cast<PhotoAlbumNapiAsyncContext *>(data);

    unique_ptr<JSAsyncContextOutput> jsContext = make_unique<JSAsyncContextOutput>();
    jsContext->status = false;

    CHECK_ARGS_RET_VOID(env, napi_get_undefined(env, &jsContext->data), JS_INNER_FAIL);
    CHECK_ARGS_RET_VOID(env, napi_get_undefined(env, &jsContext->error), JS_INNER_FAIL);
    if (context->fetchResult != nullptr) {
        GetPhotoMapQueryResult(env, context, jsContext);
    } else {
        LOGE("No fetch file result found!");
        MediaLibraryNapiUtils::CreateNapiErrorObject(env, jsContext->error, ERR_INVALID_OUTPUT,
            "Failed to get fetchFileResult from DB");
    }

    if (context->work != nullptr) {
        MediaLibraryNapiUtils::InvokeJSAsyncMethod(env, context->deferred, context->callbackRef,
                                                   context->work, *jsContext);
    }
    delete context;
}

napi_value PhotoAlbumNapi::JSPhoteAccessGetPhotoAssets(napi_env env, napi_callback_info info)
{
    unique_ptr<PhotoAlbumNapiAsyncContext> asyncContext = make_unique<PhotoAlbumNapiAsyncContext>();
    CHECK_NULLPTR_RET(ParseArgsGetPhotoAssets(env, info, asyncContext));

    return MediaLibraryNapiUtils::NapiCreateAsyncWork(env, asyncContext, "JSGetPhotoAssets",
        JSPhotoAccessGetPhotoAssetsExecute, JSGetPhotoAssetsCallbackComplete);
}
} // namespace OHOS::Media
