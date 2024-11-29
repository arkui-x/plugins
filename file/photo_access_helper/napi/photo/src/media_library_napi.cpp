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

#define MLOG_TAG "MediaLibraryNapi"
#define ABILITY_WANT_PARAMS_UIEXTENSIONTARGETTYPE "ability.want.params.uiExtensionTargetType"

#include "media_library_napi.h"

#include <fcntl.h>
#include <functional>
#include "securec.h"

#include "result_set.h"
#include "media_column.h"
#include "media_file_utils.h"
#include "medialibrary_client_errno.h"
#include "medialibrary_db_const.h"
#include "medialibrary_errno.h"
#include "userfilemgr_uri.h"
#include "log.h"
#include "photo_album_column.h"
#include "photo_album_napi.h"
#include "file_asset_napi.h"
#include "string_ex.h"
#include "userfile_client.h"
#include "rdb_errno.h"
#include "photo_album_column.h"
#include "fetch_file_result_napi.h"
#include "plugins/interfaces/native/inner_api/plugin_utils_napi.h"

using namespace std;
using namespace OHOS::NativeRdb;
using namespace OHOS::DataShare;

namespace OHOS {
namespace Media {
const int32_t SECOND_ENUM = 2;
const int32_t THIRD_ENUM = 3;
const int32_t FORMID_MAX_LEN = 19;
const int32_t SLEEP_TIME = 100;
const int64_t MAX_INT64 = 9223372036854775807;
const string DATE_FUNCTION = "DATE(";

mutex MediaLibraryNapi::sOnOffMutex_;

const std::string SUBTYPE = "subType";
const std::string PAH_SUBTYPE = "subtype";
const std::string CAMERA_SHOT_KEY = "cameraShotKey";

const std::string TITLE = "title";

thread_local napi_ref MediaLibraryNapi::sConstructor_ = nullptr;
thread_local napi_ref MediaLibraryNapi::sMediaTypeEnumRef_ = nullptr;
thread_local napi_ref MediaLibraryNapi::sFileKeyEnumRef_ = nullptr;
thread_local napi_ref MediaLibraryNapi::sPhotoSubType_ = nullptr;
using CompleteCallback = napi_async_complete_callback;
using Context = MediaLibraryAsyncContext* ;

thread_local napi_ref MediaLibraryNapi::photoAccessHelperConstructor_ = nullptr;
thread_local napi_ref MediaLibraryNapi::sPhotoKeysEnumRef_ = nullptr;
thread_local napi_ref MediaLibraryNapi::sAlbumKeyEnumRef_ = nullptr;
thread_local napi_ref MediaLibraryNapi::sAlbumType_ = nullptr;
thread_local napi_ref MediaLibraryNapi::sAlbumSubType_ = nullptr;
thread_local napi_ref MediaLibraryNapi::sRequestPhotoTypeEnumRef_ = nullptr;
thread_local napi_ref MediaLibraryNapi::sResourceTypeEnumRef_ = nullptr;

constexpr int32_t DEFAULT_REFCOUNT = 1;
constexpr int32_t DEFAULT_ALBUM_COUNT = 1; 
MediaLibraryNapi::MediaLibraryNapi()
    : env_(nullptr) {}

MediaLibraryNapi::~MediaLibraryNapi() = default;

void MediaLibraryNapi::MediaLibraryNapiDestructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    MediaLibraryNapi *mediaLibrary = reinterpret_cast<MediaLibraryNapi*>(nativeObject);
    if (mediaLibrary != nullptr) {
        delete mediaLibrary;
        mediaLibrary = nullptr;
    }
}

napi_value MediaLibraryNapi::PhotoAccessHelperInit(napi_env env, napi_value exports)
{
    NapiClassInfo info = {
        PHOTOACCESSHELPER_NAPI_CLASS_NAME,
        &photoAccessHelperConstructor_,
        MediaLibraryNapiConstructor,
        {
            DECLARE_NAPI_FUNCTION("getAssets", PhotoAccessGetPhotoAssets),
            DECLARE_NAPI_FUNCTION("release", JSRelease),
            DECLARE_NAPI_FUNCTION("getAlbums", PhotoAccessGetPhotoAlbums),
            DECLARE_NAPI_FUNCTION("getPhotoIndex", PhotoAccessGetPhotoIndex),
        }
    };
    MediaLibraryNapiUtils::NapiDefineClass(env, exports, info);

    const vector<napi_property_descriptor> staticProps = {
        DECLARE_NAPI_STATIC_FUNCTION("getPhotoAccessHelper", GetPhotoAccessHelper),
        DECLARE_NAPI_PROPERTY("PhotoType", CreateMediaTypeUserFileEnum(env)),
        DECLARE_NAPI_PROPERTY("AlbumKeys", CreateAlbumKeyEnum(env)),
        DECLARE_NAPI_PROPERTY("AlbumType", CreateAlbumTypeEnum(env)),
        DECLARE_NAPI_PROPERTY("PhotoKeys", CreatePhotoKeysEnum(env)),
        DECLARE_NAPI_PROPERTY("AlbumSubtype", CreateAlbumSubTypeEnum(env)),
        DECLARE_NAPI_PROPERTY("PhotoSubtype", CreatePhotoSubTypeEnum(env)),
        DECLARE_NAPI_PROPERTY("ResourceType", CreateResourceTypeEnum(env)),
    };
    MediaLibraryNapiUtils::NapiAddStaticProps(env, exports, staticProps);
    return exports;
}

static napi_status CheckWhetherAsync(napi_env env, napi_callback_info info, bool &isAsync)
{
    isAsync = false;
    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {0};
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("Error while obtaining js environment information");
        return status;
    }

    if (argc == ARGS_ONE || argc == ARGS_ZERO) {
        return napi_ok;
    } else if (argc == ARGS_TWO) {
        napi_valuetype valueType = napi_undefined;
        status = napi_typeof(env, argv[ARGS_ONE], &valueType);
        if (status != napi_ok) {
            LOGE("Error while obtaining js environment information");
            return status;
        }
        if (valueType == napi_boolean) {
            isAsync = true;
        }
        status = napi_get_value_bool(env, argv[ARGS_ONE], &isAsync);
        return status;
    } else {
        LOGE("argc %{public}d, is invalid", static_cast<int>(argc));
        return napi_invalid_arg;
    }
}

// Constructor callback
napi_value MediaLibraryNapi::MediaLibraryNapiConstructor(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_undefined(env, &result));
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        LOGE("Error while obtaining js environment information, status: %{public}d", status);
        return result;
    }

    unique_ptr<MediaLibraryNapi> obj = make_unique<MediaLibraryNapi>();
    if (obj == nullptr) {
        return result;
    }
    obj->env_ = env;

    bool isAsync = false;
    NAPI_CALL(env, CheckWhetherAsync(env, info, isAsync));
    if (!isAsync) {
        unique_lock<mutex> helperLock(PhotoPickerNapi::sUserFileClientMutex_);
        if (!UserFileClient::IsValid()) {
            UserFileClient::Init(env, info);
            if (!UserFileClient::IsValid()) {
                LOGE("UserFileClient creation failed");
                helperLock.unlock();
                return result;
            }
        }
        helperLock.unlock();
    }

    status = napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()),
                       MediaLibraryNapi::MediaLibraryNapiDestructor, nullptr, nullptr);
    if (status == napi_ok) {
        obj.release();
        return thisVar;
    } else {
        LOGE("Failed to wrap the native media lib client object with JS, status: %{public}d", status);
    }

    return result;
}

static bool CheckWhetherInitSuccess(napi_env env, napi_value value, bool checkIsValid)
{
    napi_value propertyNames;
    uint32_t propertyLength;
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL_BASE(env, napi_typeof(env, value, &valueType), false);
    if (valueType != napi_object) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_get_property_names(env, value, &propertyNames), false);
    NAPI_CALL_BASE(env, napi_get_array_length(env, propertyNames, &propertyLength), false);
    if (propertyLength == 0) {
        return false;
    }
    if (checkIsValid && (!UserFileClient::IsValid())) {
        LOGE("UserFileClient is not valid");
        return false;
    }
    return true;
}

static napi_value CreateNewInstance(napi_env env, napi_callback_info info, napi_ref ref,
    bool isAsync = false)
{
    constexpr size_t ARG_CONTEXT = 1;
    size_t argc = ARG_CONTEXT;
    napi_value argv[ARGS_TWO] = {0};

    napi_value thisVar = nullptr;
    napi_value ctor = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_CALL(env, napi_get_reference_value(env, ref, &ctor));

    if (isAsync) {
        argc = ARGS_TWO;
        NAPI_CALL(env, napi_get_boolean(env, true, &argv[ARGS_ONE]));
        argv[ARGS_ONE] = argv[ARG_CONTEXT];
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_new_instance(env, ctor, argc, argv, &result));
    if (!CheckWhetherInitSuccess(env, result, !isAsync)) {
        LOGE("Init MediaLibrary Instance is failed");
        NAPI_CALL(env, napi_get_undefined(env, &result));
    }
    return result;
}

napi_value MediaLibraryNapi::GetPhotoAccessHelper(napi_env env, napi_callback_info info)
{
    return CreateNewInstance(env, info, photoAccessHelperConstructor_);
}

static napi_status AddIntegerNamedProperty(napi_env env, napi_value object,
    const string &name, int32_t enumValue)
{
    napi_value enumNapiValue;
    napi_status status = napi_create_int32(env, enumValue, &enumNapiValue);
    if (status == napi_ok) {
        status = napi_set_named_property(env, object, name.c_str(), enumNapiValue);
    }
    return status;
}

static napi_value CreateNumberEnumProperty(napi_env env, vector<string> properties, napi_ref &ref, int32_t offset = 0)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    for (size_t i = 0; i < properties.size(); i++) {
        NAPI_CALL(env, AddIntegerNamedProperty(env, result, properties[i], i + offset));
    }
    NAPI_CALL(env, napi_create_reference(env, result, NAPI_INIT_REF_COUNT, &ref));
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

static void GetNapiFileResult(napi_env env, MediaLibraryAsyncContext *context,
    unique_ptr<JSAsyncContextOutput> &jsContext)
{
    // Create FetchResult object using the contents of resultSet
    if (context->fetchFileResult == nullptr) {
        LOGE("No fetch file result found!");
        MediaLibraryNapiUtils::CreateNapiErrorObject(env, jsContext->error, ERR_INVALID_OUTPUT,
            "Failed to obtain Fetch File Result");
        return;
    }
    napi_value fileResult = FetchFileResultNapi::CreateFetchFileResult(env, move(context->fetchFileResult));
    if (fileResult == nullptr) {
        MediaLibraryNapiUtils::CreateNapiErrorObject(env, jsContext->error, ERR_INVALID_OUTPUT,
            "Failed to create js object for Fetch File Result");
    } else {
        jsContext->data = fileResult;
        jsContext->status = true;
        napi_get_undefined(env, &jsContext->error);
    }
}

static void GetFileAssetsAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    MediaLibraryAsyncContext *context = static_cast<MediaLibraryAsyncContext*>(data);
    CHECK_NULL_PTR_RETURN_VOID(context, "Async context is null");

    unique_ptr<JSAsyncContextOutput> jsContext = make_unique<JSAsyncContextOutput>();
    jsContext->status = false;
    napi_get_undefined(env, &jsContext->data);

    if (context->error != ERR_DEFAULT) {
        context->HandleError(env, jsContext->error);
    } else {
        GetNapiFileResult(env, context, jsContext);
    }

    
    if (context->work != nullptr) {
        MediaLibraryNapiUtils::InvokeJSAsyncMethod(env, context->deferred, context->callbackRef,
                                                   context->work, *jsContext);
    }
    delete context;
}

static void JSReleaseCompleteCallback(napi_env env, napi_status status,
                                      MediaLibraryAsyncContext *context)
{
    CHECK_NULL_PTR_RETURN_VOID(context, "Async context is null");

    unique_ptr<JSAsyncContextOutput> jsContext = make_unique<JSAsyncContextOutput>();
    jsContext->status = false;
    if (context->objectInfo != nullptr) {
        napi_create_int32(env, SUCCESS, &jsContext->data);
        jsContext->status = true;
        napi_get_undefined(env, &jsContext->error);
    } else {
        LOGE("JSReleaseCompleteCallback context->objectInfo == nullptr");
        MediaLibraryNapiUtils::CreateNapiErrorObject(env, jsContext->error, ERR_INVALID_OUTPUT,
            "UserFileClient is invalid");
        napi_get_undefined(env, &jsContext->data);
    }

    
    if (context->work != nullptr) {
        MediaLibraryNapiUtils::InvokeJSAsyncMethod(env, context->deferred, context->callbackRef,
                                                   context->work, *jsContext);
    }

    delete context;
}

napi_value MediaLibraryNapi::JSRelease(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value thisVar = nullptr;
    napi_value resource = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, (argc == ARGS_ONE || argc == ARGS_ZERO), "requires 1 parameters maximum");
    napi_get_undefined(env, &result);

    unique_ptr<MediaLibraryAsyncContext> asyncContext = make_unique<MediaLibraryAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&asyncContext->objectInfo));
    NAPI_ASSERT(env, status == napi_ok && asyncContext->objectInfo != nullptr, "Failed to get object info");

    if (argc == PARAM1) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[PARAM0], &valueType);
        if (valueType == napi_function) {
            int32_t refCount = 1;
            napi_create_reference(env, argv[PARAM0], refCount, &asyncContext->callbackRef);
        }
    }
    CHECK_NULL_PTR_RETURN_UNDEFINED(env, result, result, "Failed to obtain arguments");

    NAPI_CALL(env, napi_remove_wrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo)));
    NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
    NAPI_CREATE_RESOURCE_NAME(env, resource, "JSRelease", asyncContext);

    status = napi_create_async_work(
        env, nullptr, resource, [](napi_env env, void *data) {},
        reinterpret_cast<CompleteCallback>(JSReleaseCompleteCallback),
        static_cast<void *>(asyncContext.get()), &asyncContext->work);
    if (status != napi_ok) {
        napi_get_undefined(env, &result);
    } else {
        napi_queue_async_work(env, asyncContext->work);
        asyncContext.release();
    }

    return result;
}

static napi_value AddDefaultPhotoAlbumColumns(napi_env env, vector<string> &fetchColumn)
{
    auto validFetchColumns = PhotoAlbumColumns::DEFAULT_FETCH_COLUMNS;
    for (const auto &column : fetchColumn) {
        if (PhotoAlbumColumns::IsPhotoAlbumColumn(column)) {
            validFetchColumns.insert(column);
        } else {
            NapiError::ThrowError(env, JS_ERR_PARAMETER_INVALID);
            return nullptr;
        }
    }
    fetchColumn.assign(validFetchColumns.begin(), validFetchColumns.end());

    napi_value result = nullptr;
    CHECK_ARGS(env, napi_get_boolean(env, true, &result), JS_INNER_FAIL);
    return result;
}

static napi_value ParseArgsGetAssets(napi_env env, napi_callback_info info,
    unique_ptr<MediaLibraryAsyncContext> &context)
{
    constexpr size_t minArgs = ARGS_ONE;
    constexpr size_t maxArgs = ARGS_TWO;
    CHECK_ARGS(env, MediaLibraryNapiUtils::AsyncContextSetObjectInfo(env, info, context, minArgs, maxArgs),
        JS_ERR_PARAMETER_INVALID);

    /* Parse the first argument */
    CHECK_ARGS(env, MediaLibraryNapiUtils::GetFetchOption(env, context->argv[PARAM0], ASSET_FETCH_OPT, context),
        JS_INNER_FAIL);
    auto &predicates = context->predicates;
    switch (context->assetType) {
        case TYPE_PHOTO: {
            CHECK_NULLPTR_RET(MediaLibraryNapiUtils::AddDefaultAssetColumns(env, context->fetchColumn,
                PhotoColumn::IsPhotoColumn));
            break;
        }
        default: {
            NapiError::ThrowError(env, JS_ERR_PARAMETER_INVALID);
            return nullptr;
        }
    }
    napi_value result = nullptr;
    CHECK_ARGS(env, napi_get_boolean(env, true, &result), JS_INNER_FAIL);
    return result;
}

static napi_status ParseArgsIndexUri(napi_env env, unique_ptr<MediaLibraryAsyncContext> &context, string &uri,
    string &albumUri)
{
    CHECK_STATUS_RET(MediaLibraryNapiUtils::GetParamStringPathMax(env, context->argv[ARGS_ZERO], uri),
        "Failed to get first string argument");
    CHECK_STATUS_RET(MediaLibraryNapiUtils::GetParamStringPathMax(env, context->argv[ARGS_ONE], albumUri),
        "Failed to get second string argument");
    return napi_ok;
}

static napi_value ParseArgsIndexof(napi_env env, napi_callback_info info,
    unique_ptr<MediaLibraryAsyncContext> &context)
{
    if (!MediaLibraryNapiUtils::IsSystemApp()) {
        NapiError::ThrowError(env, E_CHECK_SYSTEMAPP_FAIL, "This interface can be called only by system apps");
        return nullptr;
    }

    constexpr size_t minArgs = ARGS_THREE;
    constexpr size_t maxArgs = ARGS_FOUR;
    CHECK_ARGS(env, MediaLibraryNapiUtils::AsyncContextSetObjectInfo(env, info, context, minArgs, maxArgs),
        JS_ERR_PARAMETER_INVALID);

    string uri;
    string album;
    CHECK_ARGS(env, ParseArgsIndexUri(env, context, uri, album), JS_INNER_FAIL);
    CHECK_ARGS(env, MediaLibraryNapiUtils::GetFetchOption(env, context->argv[PARAM2], ASSET_FETCH_OPT, context),
        JS_INNER_FAIL);
    auto &predicates = context->predicates;
    predicates.And()->EqualTo(MediaColumn::MEDIA_DATE_TRASHED, to_string(0));
    predicates.And()->EqualTo(MediaColumn::MEDIA_TIME_PENDING, to_string(0));
    predicates.And()->EqualTo(MediaColumn::MEDIA_HIDDEN, to_string(0));

    context->fetchColumn.clear();
    napi_value result = nullptr;
    CHECK_ARGS(env, napi_get_boolean(env, true, &result), JS_INNER_FAIL);
    return result;
}

static void GetPhotoIndexAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    auto *context = static_cast<MediaLibraryAsyncContext*>(data);
    CHECK_NULL_PTR_RETURN_VOID(context, "Async context is null");

    auto jsContext = make_unique<JSAsyncContextOutput>();
    jsContext->status = false;
    CHECK_ARGS_RET_VOID(env, napi_get_undefined(env, &jsContext->data), JS_ERR_PARAMETER_INVALID);
    if (context->error != ERR_DEFAULT) {
        context->HandleError(env, jsContext->error);
    } else {
        int32_t count = -1;
        if (context->fetchFileResult != nullptr) {
            auto fileAsset = context->fetchFileResult->GetFirstObject();
            if (fileAsset != nullptr) {
                count = fileAsset->GetPhotoIndex();
            }
        }
        jsContext->status = true;
        napi_create_int32(env, count, &jsContext->data);
    }

    if (context->work != nullptr) {
        MediaLibraryNapiUtils::InvokeJSAsyncMethod(env, context->deferred, context->callbackRef,
                                                   context->work, *jsContext);
    }
    delete context;
}

static void GetPhotoIndexExec(napi_env env, void *data, ResultNapiType type)
{
    auto *context = static_cast<MediaLibraryAsyncContext*>(data);
    int errCode = 0;
    auto resultSet = UserFileClient::Query(context->predicates, context->fetchColumn, errCode);
    if (resultSet == nullptr) {
        context->SaveError(errCode);
        return;
    }
    context->fetchFileResult = make_unique<FetchResult<FileAsset>>(move(resultSet));
    context->fetchFileResult->SetResultNapiType(type);
}

static void PhotoAccessGetPhotoIndexExec(napi_env env, void *data)
{
    GetPhotoIndexExec(env, data, ResultNapiType::TYPE_PHOTOACCESS_HELPER);
}

napi_value MediaLibraryNapi::PhotoAccessGetPhotoIndex(napi_env env, napi_callback_info info)
{
    unique_ptr<MediaLibraryAsyncContext> asyncContext = make_unique<MediaLibraryAsyncContext>();
    CHECK_NULLPTR_RET(ParseArgsIndexof(env, info, asyncContext));
    return MediaLibraryNapiUtils::NapiCreateAsyncWork(env, asyncContext, "JSGetPhotoIndex",
        PhotoAccessGetPhotoIndexExec, GetPhotoIndexAsyncCallbackComplete);
}

static void PhotoAccessGetAssetsExecute(napi_env env, void *data)
{
    auto *context = static_cast<MediaLibraryAsyncContext*>(data);
    int errCode = 0;
    shared_ptr<ResultSet> resultSet = UserFileClient::Query(
        context->predicates, context->fetchColumn, errCode);
    if (resultSet == nullptr && errCode == E_PERMISSION_DENIED) {
        std::vector<std::string> FETCH_COLUMNS = {
            MEDIA_DATA_DB_TITLE, MEDIA_DATA_DB_MEDIA_TYPE, MEDIA_DATA_DB_MIME_TYPE, MEDIA_DATA_DB_DATE_ADDED,
            MEDIA_DATA_DB_DATE_MODIFIED, MEDIA_DATA_DB_NAME, MEDIA_DATA_DB_SIZE, MEDIA_DATA_DB_DATE_TAKEN,
            MEDIA_DATA_DB_ORIENTATION, MEDIA_DATA_DB_WIDTH, MEDIA_DATA_DB_HEIGHT, MEDIA_DATA_DB_DURATION,
            MEDIA_DATA_DB_FILE_PATH, MEDIA_DATA_DB_ID
        };
        resultSet = UserFileClient::Query(context->predicates, FETCH_COLUMNS, errCode);
    }
    if (resultSet == nullptr) {
        context->SaveError(errCode);
        return;
    }
    context->fetchFileResult = make_unique<FetchResult<FileAsset>>(move(resultSet));
    context->fetchFileResult->SetResultNapiType(ResultNapiType::TYPE_PHOTOACCESS_HELPER);
}

napi_value MediaLibraryNapi::PhotoAccessGetPhotoAssets(napi_env env, napi_callback_info info)
{
    unique_ptr<MediaLibraryAsyncContext> asyncContext = make_unique<MediaLibraryAsyncContext>();
    asyncContext->assetType = TYPE_PHOTO;
    CHECK_NULLPTR_RET(ParseArgsGetAssets(env, info, asyncContext));
    return MediaLibraryNapiUtils::NapiCreateAsyncWork(env, asyncContext, "JSGetPhotoAssets",
        PhotoAccessGetAssetsExecute, GetFileAssetsAsyncCallbackComplete);
}

static void GetPhotoAlbumQueryResult(napi_env env, MediaLibraryAsyncContext *context,
    unique_ptr<JSAsyncContextOutput> &jsContext)
{
    napi_value fileResult = FetchFileResultNapi::CreateFetchFileResult(env, move(context->fetchPhotoAlbumResult));
    if (fileResult == nullptr) {
        CHECK_ARGS_RET_VOID(env, napi_get_undefined(env, &jsContext->data), JS_INNER_FAIL);
        MediaLibraryNapiUtils::CreateNapiErrorObject(env, jsContext->error, ERR_INVALID_OUTPUT,
            "Failed to create js object for Fetch Album Result");
        return;
    }
    jsContext->data = fileResult;
    jsContext->status = true;
    CHECK_ARGS_RET_VOID(env, napi_get_undefined(env, &jsContext->error), JS_INNER_FAIL);
}

static void JSGetPhotoAlbumsExecute(napi_env env, void *data)
{
    auto *context = static_cast<MediaLibraryAsyncContext*>(data);
    int errCode = 0;
    DataShare::DataSharePredicates &predicates = context->predicates;
    std::vector<std::string> albumArray;
    albumArray.push_back("bucket_display_name");
    predicates.GroupBy(albumArray);
    auto resultSet = UserFileClient::QueryAlbum(context->predicates, context->fetchColumn, errCode);
    if (resultSet == nullptr) {
        LOGE("resultSet == nullptr, errCode is %{public}d", errCode);
        if (errCode == E_PERMISSION_DENIED) {
            if (context->hiddenOnly || context->hiddenAlbumFetchMode == ASSETS_MODE) {
                context->error = OHOS_PERMISSION_DENIED_CODE;
            } else {
                context->SaveError(E_HAS_DB_ERROR);
            }
        } else {
            context->SaveError(E_HAS_DB_ERROR);
        }
        return;
    }

    context->fetchPhotoAlbumResult = make_unique<FetchResult<PhotoAlbum>>(move(resultSet));
    context->fetchPhotoAlbumResult->SetResultNapiType(context->resultNapiType);
    context->fetchPhotoAlbumResult->SetHiddenOnly(context->hiddenOnly);
    context->fetchPhotoAlbumResult->SetLocationOnly(context->isLocationAlbum ==
        PhotoAlbumSubType::GEOGRAPHY_LOCATION);
}

static void JSGetPhotoAlbumsCompleteCallback(napi_env env, napi_status status, void *data)
{

    auto *context = static_cast<MediaLibraryAsyncContext*>(data);
    unique_ptr<JSAsyncContextOutput> jsContext = make_unique<JSAsyncContextOutput>();
    jsContext->status = false;
    CHECK_ARGS_RET_VOID(env, napi_get_undefined(env, &jsContext->error), JS_INNER_FAIL);
    if (context->error != ERR_DEFAULT  || context->fetchPhotoAlbumResult == nullptr) {
        CHECK_ARGS_RET_VOID(env, napi_get_undefined(env, &jsContext->data), JS_INNER_FAIL);
        context->HandleError(env, jsContext->error);
    } else {
        GetPhotoAlbumQueryResult(env, context, jsContext);
    }
    if (context->work != nullptr) {
        MediaLibraryNapiUtils::InvokeJSAsyncMethod(env, context->deferred, context->callbackRef,
                                                   context->work, *jsContext);
    }
    delete context;
}

napi_value MediaLibraryNapi::CreatePhotoKeysEnum(napi_env env)
{
    return CreateStringEnumProperty(env, IMAGEVIDEOKEY_ENUM_PROPERTIES, sPhotoKeysEnumRef_);
}

napi_value MediaLibraryNapi::CreateAlbumKeyEnum(napi_env env)
{
    return CreateStringEnumProperty(env, ALBUMKEY_ENUM_PROPERTIES, sAlbumKeyEnumRef_);
}

napi_value MediaLibraryNapi::CreateAlbumTypeEnum(napi_env env)
{
    napi_value result = nullptr;
    CHECK_ARGS(env, napi_create_object(env, &result), JS_INNER_FAIL);

    CHECK_ARGS(env, AddIntegerNamedProperty(env, result, "USER", PhotoAlbumType::USER), JS_INNER_FAIL);
    CHECK_ARGS(env, AddIntegerNamedProperty(env, result, "SYSTEM", PhotoAlbumType::SYSTEM), JS_INNER_FAIL);
    CHECK_ARGS(env, AddIntegerNamedProperty(env, result, "SMART", PhotoAlbumType::SMART), JS_INNER_FAIL);
    CHECK_ARGS(env, AddIntegerNamedProperty(env, result, "SOURCE", PhotoAlbumType::SOURCE), JS_INNER_FAIL);

    CHECK_ARGS(env, napi_create_reference(env, result, NAPI_INIT_REF_COUNT, &sAlbumType_), JS_INNER_FAIL);
    return result;
}

napi_value MediaLibraryNapi::CreateAlbumSubTypeEnum(napi_env env)
{
    napi_value result = nullptr;
    CHECK_ARGS(env, napi_create_object(env, &result), JS_INNER_FAIL);

    CHECK_ARGS(env, AddIntegerNamedProperty(env, result, "USER_GENERIC", PhotoAlbumSubType::USER_GENERIC),
        JS_INNER_FAIL);
    CHECK_ARGS(env, AddIntegerNamedProperty(env, result, "SOURCE_GENERIC", PhotoAlbumSubType::SOURCE_GENERIC),
        JS_INNER_FAIL);
    for (size_t i = 0; i < systemAlbumSubType.size(); i++) {
        CHECK_ARGS(env, AddIntegerNamedProperty(env, result, systemAlbumSubType[i],
            PhotoAlbumSubType::SYSTEM_START + i), JS_INNER_FAIL);
    }
    for (size_t i = 0; i < analysisAlbumSubType.size(); i++) {
        CHECK_ARGS(env, AddIntegerNamedProperty(env, result, analysisAlbumSubType[i],
            PhotoAlbumSubType::ANALYSIS_START + i), JS_INNER_FAIL);
    }
    CHECK_ARGS(env, AddIntegerNamedProperty(env, result, "ANY", PhotoAlbumSubType::ANY), JS_INNER_FAIL);

    CHECK_ARGS(env, napi_create_reference(env, result, NAPI_INIT_REF_COUNT, &sAlbumSubType_), JS_INNER_FAIL);
    return result;
}

napi_value MediaLibraryNapi::CreateResourceTypeEnum(napi_env env)
{
    const int32_t startIdx = 1;
    return CreateNumberEnumProperty(env, resourceTypeEnum, sResourceTypeEnumRef_, startIdx);
}

static napi_value GetAlbumFetchOption(napi_env env, unique_ptr<MediaLibraryAsyncContext> &context, bool hasCallback)
{
    if (context->argc < (ARGS_ONE + hasCallback)) {
        LOGE("No arguments to parse");
        return nullptr;
    }

    // The index of fetchOption should always be the last arg besides callback
    napi_value fetchOption = context->argv[context->argc - 1 - hasCallback];
    CHECK_ARGS(env, MediaLibraryNapiUtils::GetFetchOption(env, fetchOption, ALBUM_FETCH_OPT, context), JS_INNER_FAIL);
    if (!context->uri.empty()) {
        if (context->uri.find(PhotoAlbumColumns::ANALYSIS_ALBUM_URI_PREFIX) != std::string::npos) {
            context->isAnalysisAlbum = 1; // 1:is an analysis album
        }
    }
    napi_value result = nullptr;
    CHECK_ARGS(env, napi_get_boolean(env, true, &result), JS_INNER_FAIL);
    return result;
}

static napi_value ParseAlbumTypes(napi_env env, unique_ptr<MediaLibraryAsyncContext> &context)
{
    if (context->argc < ARGS_TWO) {
        LOGE("No arguments to parse");
        return nullptr;
    }

    /* Parse the first argument to photo album type */
    int32_t albumType;
    CHECK_NULLPTR_RET(MediaLibraryNapiUtils::GetInt32Arg(env, context->argv[PARAM0], albumType));
    if (!PhotoAlbum::CheckPhotoAlbumType(static_cast<PhotoAlbumType>(albumType))) {
        NapiError::ThrowError(env, JS_ERR_PARAMETER_INVALID);
        return nullptr;
    }
    context->isAnalysisAlbum = (albumType == PhotoAlbumType::SMART) ? 1 : 0;

    /* Parse the second argument to photo album subType */
    int32_t albumSubType;
    CHECK_NULLPTR_RET(MediaLibraryNapiUtils::GetInt32Arg(env, context->argv[PARAM1], albumSubType));
    if (!PhotoAlbum::CheckPhotoAlbumSubType(static_cast<PhotoAlbumSubType>(albumSubType))) {
        NapiError::ThrowError(env, JS_ERR_PARAMETER_INVALID);
        return nullptr;
    }
#ifdef IOS_PLATFORM
    context->predicates.And()->EqualTo(PhotoAlbumColumns::ALBUM_TYPE, to_string(albumType));
    if (albumSubType != ANY) {
        context->predicates.And()->EqualTo(PhotoAlbumColumns::ALBUM_SUBTYPE, to_string(albumSubType));
    }
#endif


    napi_value result = nullptr;
    CHECK_ARGS(env, napi_get_boolean(env, true, &result), JS_INNER_FAIL);
    return result;
}

static napi_value ParseArgsGetPhotoAlbum(napi_env env, napi_callback_info info,
    unique_ptr<MediaLibraryAsyncContext> &context)
{
    constexpr size_t minArgs = ARGS_ZERO;
    constexpr size_t maxArgs = ARGS_FOUR;
    CHECK_ARGS(env, MediaLibraryNapiUtils::AsyncContextSetObjectInfo(env, info, context, minArgs, maxArgs),
        JS_ERR_PARAMETER_INVALID);

    bool hasCallback = false;
    CHECK_ARGS(env, MediaLibraryNapiUtils::HasCallback(env, context->argc, context->argv, hasCallback),
        JS_ERR_PARAMETER_INVALID);
    if (context->argc == ARGS_THREE) {
        napi_valuetype valueType = napi_undefined;
        if (napi_typeof(env, context->argv[PARAM2], &valueType) == napi_ok &&
            (valueType == napi_undefined || valueType == napi_null)) {
            context->argc -= 1;
        }
    }
    switch (context->argc - hasCallback) {
        case ARGS_ZERO:
            break;
        case ARGS_ONE:
            CHECK_NULLPTR_RET(GetAlbumFetchOption(env, context, hasCallback));
            break;
        case ARGS_TWO:
            CHECK_NULLPTR_RET(ParseAlbumTypes(env, context));
            break;
        case ARGS_THREE:
            CHECK_NULLPTR_RET(GetAlbumFetchOption(env, context, hasCallback));
            CHECK_NULLPTR_RET(ParseAlbumTypes(env, context));
            break;
        default:
            return nullptr;
    }
    if (context->isLocationAlbum != PhotoAlbumSubType::GEOGRAPHY_LOCATION &&
        context->isLocationAlbum != PhotoAlbumSubType::GEOGRAPHY_CITY) {
        CHECK_NULLPTR_RET(AddDefaultPhotoAlbumColumns(env, context->fetchColumn));
    }
    napi_value result = nullptr;
    CHECK_ARGS(env, napi_get_boolean(env, true, &result), JS_INNER_FAIL);
    return result;
}

napi_value MediaLibraryNapi::PhotoAccessGetPhotoAlbums(napi_env env, napi_callback_info info)
{
    unique_ptr<MediaLibraryAsyncContext> asyncContext = make_unique<MediaLibraryAsyncContext>();
    asyncContext->resultNapiType = ResultNapiType::TYPE_PHOTOACCESS_HELPER;
    CHECK_NULLPTR_RET(ParseArgsGetPhotoAlbum(env, info, asyncContext));

    return MediaLibraryNapiUtils::NapiCreateAsyncWork(env, asyncContext, "GetPhotoAlbums",  JSGetPhotoAlbumsExecute,
        JSGetPhotoAlbumsCompleteCallback);
}

napi_value MediaLibraryNapi::CreatePhotoSubTypeEnum(napi_env env)
{
    return CreateNumberEnumProperty(env, photoSubTypeEnum, sPhotoSubType_);
}

napi_value MediaLibraryNapi::CreateMediaTypeUserFileEnum(napi_env env)
{
    const int32_t startIdx = 1;
    return CreateNumberEnumProperty(env, mediaTypesUserFileEnum, sMediaTypeEnumRef_, startIdx);
}
} // namespace Media
} // namespace OHOS
