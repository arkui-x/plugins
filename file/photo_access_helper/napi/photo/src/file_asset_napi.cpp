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
#define MLOG_TAG "FileAssetNapi"

#include "file_asset_napi.h"

#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "abs_shared_result_set.h"
#include "datashare_predicates.h"
#include "datashare_values_bucket.h"
#include "result_set.h"
#include "fetch_result.h"
#include "log.h"
#include "media_column.h"
#include "media_file_utils.h"
#include "medialibrary_client_errno.h"
#include "medialibrary_db_const.h"
#include "medialibrary_errno.h"
#include "medialibrary_napi_utils.h"
#include "nlohmann/json.hpp"
#include "rdb_errno.h"
#include "unique_fd.h"
#include "userfile_client.h"
#include "userfilemgr_uri.h"
#include "photo_album_column.h"

using namespace std;
using namespace OHOS::NativeRdb;
using namespace OHOS::DataShare;
using std::string;

namespace OHOS {
namespace Media {
static const std::string MEDIA_FILEDESCRIPTOR = "fd";
static const std::string MEDIA_FILEMODE = "mode";

thread_local napi_ref FileAssetNapi::sConstructor_ = nullptr;
thread_local std::shared_ptr<FileAsset> FileAssetNapi::sFileAsset_ = nullptr;

constexpr int32_t IS_TRASH = 1;
constexpr int32_t NOT_TRASH = 0;

constexpr int32_t IS_FAV = 1;
constexpr int32_t NOT_FAV = 0;

constexpr int32_t IS_HIDDEN = 1;
constexpr int32_t NOT_HIDDEN = 0;

constexpr int32_t USER_COMMENT_MAX_LEN = 420;

using CompleteCallback = napi_async_complete_callback;

thread_local napi_ref FileAssetNapi::userFileMgrConstructor_ = nullptr;
thread_local napi_ref FileAssetNapi::photoAccessHelperConstructor_ = nullptr;

class TransferFileAsset {
public:
    std::shared_ptr<FileAsset> fileAsset = nullptr;
    ~TransferFileAsset() = default;
};

FileAssetNapi::FileAssetNapi()
    : env_(nullptr) {}

FileAssetNapi::~FileAssetNapi() = default;

void FileAssetNapi::FileAssetNapiDestructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    FileAssetNapi *fileAssetObj = reinterpret_cast<FileAssetNapi*>(nativeObject);
    if (fileAssetObj != nullptr) {
        delete fileAssetObj;
        fileAssetObj = nullptr;
    }
}

napi_value FileAssetNapi::PhotoAccessHelperInit(napi_env env, napi_value exports)
{
    NapiClassInfo info = {
        .name = PHOTOACCESSHELPER_FILEASSET_NAPI_CLASS_NAME,
        .ref = &photoAccessHelperConstructor_,
        .constructor = FileAssetNapiConstructor,
        .props = {
            DECLARE_NAPI_FUNCTION("get", UserFileMgrGet),
            DECLARE_NAPI_FUNCTION("set", UserFileMgrSet),
            DECLARE_NAPI_FUNCTION("commitModify", PhotoAccessHelperCommitModify),
            DECLARE_NAPI_FUNCTION("setFavorite", PhotoAccessHelperFavorite),
            DECLARE_NAPI_GETTER("uri", JSGetFileUri),
            DECLARE_NAPI_GETTER("photoType", JSGetMediaType),
            DECLARE_NAPI_GETTER_SETTER("displayName", JSGetFileDisplayName, JSSetFileDisplayName),
        }
    };
    MediaLibraryNapiUtils::NapiDefineClass(env, exports, info);
    return exports;
}

inline void *DetachFileAssetFunc(napi_env env, void *value, void *)
{
    if (value == nullptr) {
        LOGE("detach value is null");
        return nullptr;
    }
    auto fileAssetNapi = reinterpret_cast<FileAssetNapi*>(value);
    std::shared_ptr<FileAsset> detachFileAsset = fileAssetNapi->GetFileAssetInstance();
    TransferFileAsset *transferFileAsset = new TransferFileAsset();
    transferFileAsset->fileAsset = detachFileAsset;
    return transferFileAsset;
}

napi_value AttachFileAssetFunc(napi_env env, void *value, void *)
{
    if (value == nullptr) {
        LOGE("attach value is null");
        return nullptr;
    }
    auto transferFileAsset = reinterpret_cast<TransferFileAsset*>(value);
    std::shared_ptr<FileAsset> fileAsset = std::move(transferFileAsset->fileAsset);
    delete transferFileAsset;
    NAPI_ASSERT(env, fileAsset != nullptr, "AttachFileAssetFunc fileAsset is null");
    napi_value result = FileAssetNapi::AttachCreateFileAsset(env, fileAsset);
    NAPI_ASSERT(env, result != nullptr, "AttachFileAssetFunc result is null");
    return result;
}

// Constructor callback
napi_value FileAssetNapi::FileAssetNapiConstructor(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &result);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status == napi_ok && thisVar != nullptr) {
        std::unique_ptr<FileAssetNapi> obj = std::make_unique<FileAssetNapi>();
        if (obj != nullptr) {
            obj->env_ = env;
            if (sFileAsset_ != nullptr) {
                obj->UpdateFileAssetInfo();
            }
            napi_coerce_to_native_binding_object(
                env, thisVar, DetachFileAssetFunc, AttachFileAssetFunc, obj.get(), nullptr);
            status = napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()),
                               FileAssetNapi::FileAssetNapiDestructor, nullptr, nullptr);
            if (status == napi_ok) {
                obj.release();
                return thisVar;
            } else {
                LOGE("Failure wrapping js to native napi, status: %{public}d", status);
            }
        }
    }

    return result;
}

napi_value FileAssetNapi::AttachCreateFileAsset(napi_env env, std::shared_ptr<FileAsset> &iAsset)
{
    if (iAsset == nullptr) {
        return nullptr;
    }
    napi_value constructor = nullptr;
    napi_ref constructorRef = nullptr;
    napi_value exports = nullptr;
    if (iAsset->GetResultNapiType() == ResultNapiType::TYPE_PHOTOACCESS_HELPER) {
        if (photoAccessHelperConstructor_ == nullptr) {
            LOGI("AttachCreateFileAsset photoAccessHelperConstructor_ is null");
            napi_create_object(env, &exports);
            FileAssetNapi::PhotoAccessHelperInit(env, exports);
        }
        constructorRef = photoAccessHelperConstructor_;
    }
    if (constructorRef == nullptr) {
        NAPI_ASSERT(env, false, "AttachCreateFileAsset constructorRef is null");
    }
    napi_status status = napi_get_reference_value(env, constructorRef, &constructor);
    NAPI_ASSERT(env, status == napi_ok, "AttachCreateFileAsset napi_get_reference_value failed");
    sFileAsset_ = iAsset;
    napi_value result = nullptr;
    status = napi_new_instance(env, constructor, 0, nullptr, &result);
    NAPI_ASSERT(env, status == napi_ok, "AttachCreateFileAsset napi_new_instance failed");
    sFileAsset_ = nullptr;
    return result;
}


napi_value FileAssetNapi::CreateFileAsset(napi_env env, unique_ptr<FileAsset> &iAsset)
{
    if (iAsset == nullptr) {
        return nullptr;
    }

    napi_value constructor = nullptr;
    napi_ref constructorRef;
    if (iAsset->GetResultNapiType() == ResultNapiType::TYPE_USERFILE_MGR) {
        constructorRef = userFileMgrConstructor_;
    } else if (iAsset->GetResultNapiType() == ResultNapiType::TYPE_PHOTOACCESS_HELPER) {
        constructorRef = photoAccessHelperConstructor_;
    } else {
        constructorRef = sConstructor_;
    }

    NAPI_CALL(env, napi_get_reference_value(env, constructorRef, &constructor));

    sFileAsset_ = std::move(iAsset);

    napi_value result = nullptr;
    NAPI_CALL(env, napi_new_instance(env, constructor, 0, nullptr, &result));

    sFileAsset_ = nullptr;
    return result;
}

napi_value FileAssetNapi::CreatePhotoAsset(napi_env env, shared_ptr<FileAsset> &fileAsset)
{
    if (fileAsset == nullptr || fileAsset->GetResultNapiType() != ResultNapiType::TYPE_PHOTOACCESS_HELPER) {
        LOGE("Unsupported fileAsset");
        return nullptr;
    }

    napi_value constructor = nullptr;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, photoAccessHelperConstructor_, &constructor));
    NAPI_CALL(env, napi_new_instance(env, constructor, 0, nullptr, &result));
    CHECK_COND(env, result != nullptr, JS_INNER_FAIL);

    FileAssetNapi* fileAssetNapi = nullptr;
    CHECK_ARGS(env, napi_unwrap(env, result, reinterpret_cast<void**>(&fileAssetNapi)), JS_INNER_FAIL);
    CHECK_COND(env, fileAssetNapi != nullptr, JS_INNER_FAIL);
    fileAssetNapi->fileAssetPtr = fileAsset;
    return result;
}

std::string FileAssetNapi::GetFileDisplayName() const
{
    return fileAssetPtr->GetDisplayName();
}

std::string FileAssetNapi::GetRelativePath() const
{
    return fileAssetPtr->GetRelativePath();
}

std::string FileAssetNapi::GetFilePath() const
{
    return fileAssetPtr->GetPath();
}

std::string FileAssetNapi::GetTitle() const
{
    return fileAssetPtr->GetTitle();
}

std::string FileAssetNapi::GetFileUri() const
{
    return fileAssetPtr->GetUri();
}

int32_t FileAssetNapi::GetFileId() const
{
    return fileAssetPtr->GetId();
}

Media::MediaType FileAssetNapi::GetMediaType() const
{
    return fileAssetPtr->GetMediaType();
}

int32_t FileAssetNapi::GetOrientation() const
{
    return fileAssetPtr->GetOrientation();
}

std::string FileAssetNapi::GetNetworkId() const
{
    return MediaFileUtils::GetNetworkIdFromUri(GetFileUri());
}

bool FileAssetNapi::IsFavorite() const
{
    return fileAssetPtr->IsFavorite();
}

void FileAssetNapi::SetFavorite(bool isFavorite)
{
    fileAssetPtr->SetFavorite(isFavorite);
}

bool FileAssetNapi::IsHidden() const
{
    return fileAssetPtr->IsHidden();
}

void FileAssetNapi::SetHidden(bool isHidden)
{
    fileAssetPtr->SetHidden(isHidden);
}

napi_status GetNapiObject(napi_env env, napi_callback_info info, FileAssetNapi **obj)
{
    napi_value thisVar = nullptr;
    CHECK_STATUS_RET(napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr), "Failed to get cb info");
    CHECK_STATUS_RET(napi_unwrap(env, thisVar, reinterpret_cast<void **>(obj)), "Failed to unwrap thisVar");
    CHECK_COND_RET(*obj != nullptr, napi_invalid_arg, "Failed to get napi object!");
    return napi_ok;
}

napi_value FileAssetNapi::JSGetFileUri(napi_env env, napi_callback_info info)
{
    FileAssetNapi *obj = nullptr;
    CHECK_ARGS(env, GetNapiObject(env, info, &obj), JS_INNER_FAIL);

    napi_value jsResult = nullptr;
    CHECK_ARGS(env, napi_create_string_utf8(env, obj->GetFileUri().c_str(), NAPI_AUTO_LENGTH, &jsResult),
        JS_INNER_FAIL);
    return jsResult;
}

napi_value FileAssetNapi::JSGetFileDisplayName(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    FileAssetNapi *obj = nullptr;
    string displayName = "";
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &jsResult);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        LOGE("Invalid arguments! status: %{public}d", status);
        return jsResult;
    }

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status == napi_ok && obj != nullptr) {
        displayName = obj->GetFileDisplayName();
        napi_create_string_utf8(env, displayName.c_str(), NAPI_AUTO_LENGTH, &jsResult);
    }

    return jsResult;
}

napi_value FileAssetNapi::JSSetFileDisplayName(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value undefinedResult = nullptr;
    FileAssetNapi *obj = nullptr;
    napi_valuetype valueType = napi_undefined;
    size_t res = 0;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &undefinedResult);

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc == ARGS_ONE, "requires 1 parameter");

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status == napi_ok && obj != nullptr) {
        if (napi_typeof(env, argv[PARAM0], &valueType) != napi_ok || valueType != napi_string) {
            LOGE("Invalid arguments type! valueType: %{public}d", valueType);
            return undefinedResult;
        }
        char buffer[FILENAME_MAX];
        status = napi_get_value_string_utf8(env, argv[PARAM0], buffer, FILENAME_MAX, &res);
        if (status == napi_ok) {
            string displayName = string(buffer);
            obj->fileAssetPtr->SetDisplayName(displayName);
        }
    }

    return undefinedResult;
}

napi_value FileAssetNapi::JSGetMediaType(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    FileAssetNapi *obj = nullptr;
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &jsResult);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        LOGE("Invalid arguments! status: %{public}d", status);
        return jsResult;
    }

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status == napi_ok && obj != nullptr) {
        int32_t mediaType = static_cast<int32_t>(obj->GetMediaType());
        napi_create_int32(env, mediaType, &jsResult);
    }

    return jsResult;
}

void BuildCommitModifyValuesBucket(FileAssetAsyncContext* context, DataShareValuesBucket &valuesBucket)
{
    const auto fileAsset = context->objectPtr;
    if (context->resultNapiType == ResultNapiType::TYPE_PHOTOACCESS_HELPER) {
        valuesBucket.Put(MediaColumn::MEDIA_TITLE, fileAsset->GetTitle());
    } else if (context->resultNapiType == ResultNapiType::TYPE_USERFILE_MGR) {
        valuesBucket.Put(MediaColumn::MEDIA_NAME, fileAsset->GetDisplayName());
    } else {
        valuesBucket.Put(MEDIA_DATA_DB_URI, fileAsset->GetUri());
        valuesBucket.Put(MEDIA_DATA_DB_TITLE, fileAsset->GetTitle());

        if (fileAsset->GetOrientation() >= 0) {
            valuesBucket.Put(MEDIA_DATA_DB_ORIENTATION, fileAsset->GetOrientation());
        }
        valuesBucket.Put(MEDIA_DATA_DB_RELATIVE_PATH, fileAsset->GetRelativePath());
        valuesBucket.Put(MEDIA_DATA_DB_MEDIA_TYPE, fileAsset->GetMediaType());
        valuesBucket.Put(MEDIA_DATA_DB_NAME, fileAsset->GetDisplayName());
    }
}

static void BuildCommitModifyUriApi10(FileAssetAsyncContext *context, string &uri)
{
    if (context->objectPtr->GetMediaType() == MEDIA_TYPE_IMAGE ||
        context->objectPtr->GetMediaType() == MEDIA_TYPE_VIDEO) {
        uri = (context->resultNapiType == ResultNapiType::TYPE_USERFILE_MGR) ? UFM_UPDATE_PHOTO : PAH_UPDATE_PHOTO;
    }
}

static bool CheckDisplayNameInCommitModify(FileAssetAsyncContext *context)
{
    if (context->resultNapiType != ResultNapiType::TYPE_PHOTOACCESS_HELPER) {
        if (context->objectPtr->GetMediaType() != MediaType::MEDIA_TYPE_FILE) {
            if (MediaFileUtils::CheckDisplayName(context->objectPtr->GetDisplayName()) != E_OK) {
                context->error = JS_E_DISPLAYNAME;
                return false;
            }
        } else {
            if (MediaFileUtils::CheckFileDisplayName(context->objectPtr->GetDisplayName()) != E_OK) {
                context->error = JS_E_DISPLAYNAME;
                return false;
            }
        }
    }
    return true;
}

static void JSCommitModifyExecute(napi_env env, void *data)
{
    auto *context = static_cast<FileAssetAsyncContext*>(data);
   
    if (!CheckDisplayNameInCommitModify(context)) {
        return;
    }
    string uri;
    if (context->resultNapiType == ResultNapiType::TYPE_USERFILE_MGR ||
        context->resultNapiType == ResultNapiType::TYPE_PHOTOACCESS_HELPER) {
        BuildCommitModifyUriApi10(context, uri);
        MediaLibraryNapiUtils::UriAppendKeyValue(uri, API_VERSION, to_string(MEDIA_API_VERSION_V10));
    }

    std::string updateAssetUri(uri);
    MediaType mediaType = context->objectPtr->GetMediaType();
    DataSharePredicates predicates;
    DataShareValuesBucket valuesBucket;
    BuildCommitModifyValuesBucket(context, valuesBucket);
    predicates.SetWhereClause(MEDIA_DATA_DB_ID + " = ? ");
    predicates.SetWhereArgs({std::to_string(context->objectPtr->GetId())});

    int32_t changedRows = UserFileClient::Update(updateAssetUri, predicates, valuesBucket);
    if (changedRows < 0) {
        context->SaveError(changedRows);
        LOGE("File asset modification failed, err: %{public}d", changedRows);
    } else {
        context->changedRows = changedRows;
    }
}

static void JSCommitModifyCompleteCallback(napi_env env, napi_status status, void *data)
{
    FileAssetAsyncContext *context = static_cast<FileAssetAsyncContext*>(data);
    CHECK_NULL_PTR_RETURN_VOID(context, "Async context is null");
    unique_ptr<JSAsyncContextOutput> jsContext = make_unique<JSAsyncContextOutput>();
    jsContext->status = false;


    if (context->error == ERR_DEFAULT) {
        if (context->changedRows < 0) {
            MediaLibraryNapiUtils::CreateNapiErrorObject(env, jsContext->error, context->changedRows,
                                                         "File asset modification failed");
            napi_get_undefined(env, &jsContext->data);
        } else {
            napi_create_int32(env, context->changedRows, &jsContext->data);
            jsContext->status = true;
            napi_get_undefined(env, &jsContext->error);
        }
    } else {
        LOGE("JSCommitModify fail %{public}d", context->error);
        context->HandleError(env, jsContext->error);
        napi_get_undefined(env, &jsContext->data);
    }

    if (context->work != nullptr) {
        MediaLibraryNapiUtils::InvokeJSAsyncMethod(env, context->deferred, context->callbackRef,
                                                   context->work, *jsContext);
    }
    delete context;
}

void FileAssetNapi::UpdateFileAssetInfo()
{
    fileAssetPtr = sFileAsset_;
}

shared_ptr<FileAsset> FileAssetNapi::GetFileAssetInstance() const
{
    return fileAssetPtr;
}

static bool IsSpecialKey(const string &key)
{
    static const set<string> SPECIAL_KEY = {
        PENDING_STATUS
    };

    if (SPECIAL_KEY.find(key) != SPECIAL_KEY.end()) {
        return true;
    }
    return false;
}

static napi_value HandleGettingSpecialKey(napi_env env, const string &key, const shared_ptr<FileAsset> &fileAssetPtr)
{
    napi_value jsResult = nullptr;
    if (key == PENDING_STATUS) {
        if (fileAssetPtr->GetTimePending() == 0) {
            napi_get_boolean(env, false, &jsResult);
        } else {
            napi_get_boolean(env, true, &jsResult);
        }
    }

    return jsResult;
}

napi_value FileAssetNapi::UserFileMgrGet(napi_env env, napi_callback_info info)
{
    napi_value ret = nullptr;
    unique_ptr<FileAssetAsyncContext> asyncContext = make_unique<FileAssetAsyncContext>();
    CHECK_NULL_PTR_RETURN_UNDEFINED(env, asyncContext, ret, "asyncContext context is null");

    string inputKey;
    CHECK_ARGS(env, MediaLibraryNapiUtils::ParseArgsStringCallback(env, info, asyncContext, inputKey),
        JS_ERR_PARAMETER_INVALID);

    napi_value jsResult = nullptr;
    auto obj = asyncContext->objectInfo;
    napi_get_undefined(env, &jsResult);
    LOGE("UserFileMgrGet inputKey is %s", inputKey.c_str());
    if (obj->fileAssetPtr->GetMemberMap().count(inputKey) == 0) {
        // no exist throw error
        NapiError::ThrowError(env, JS_E_FILE_KEY);
        return jsResult;
    }

    if (IsSpecialKey(inputKey)) {
        return HandleGettingSpecialKey(env, inputKey, obj->fileAssetPtr);
    }
    auto m = obj->fileAssetPtr->GetMemberMap().at(inputKey);
    if (m.index() == MEMBER_TYPE_STRING) {
        napi_create_string_utf8(env, get<string>(m).c_str(), NAPI_AUTO_LENGTH, &jsResult);
    } else if (m.index() == MEMBER_TYPE_INT32) {
        napi_create_int32(env, get<int32_t>(m), &jsResult);
    } else if (m.index() == MEMBER_TYPE_INT64) {
        napi_create_int64(env, get<int64_t>(m), &jsResult);
    } else {
        NapiError::ThrowError(env, JS_ERR_PARAMETER_INVALID);
        return jsResult;
    }
    return jsResult;
}

bool FileAssetNapi::HandleParamSet(const string &inputKey, const string &value, ResultNapiType resultNapiType)
{
    if (resultNapiType == ResultNapiType::TYPE_PHOTOACCESS_HELPER) {
        if (inputKey == MediaColumn::MEDIA_TITLE) {
            fileAssetPtr->SetTitle(value);
        } else {
            LOGE("invalid key %{private}s, no support key", inputKey.c_str());
            return false;
        }
    } else if (resultNapiType == ResultNapiType::TYPE_USERFILE_MGR) {
        if (inputKey == MediaColumn::MEDIA_NAME) {
            fileAssetPtr->SetDisplayName(value);
            fileAssetPtr->SetTitle(MediaFileUtils::GetTitleFromDisplayName(value));
        } else if (inputKey == MediaColumn::MEDIA_TITLE) {
            fileAssetPtr->SetTitle(value);
            string displayName = fileAssetPtr->GetDisplayName();
            if (!displayName.empty()) {
                string extention = MediaFileUtils::SplitByChar(displayName, '.');
                fileAssetPtr->SetDisplayName(value + "." + extention);
            }
        } else {
            LOGE("invalid key %{private}s, no support key", inputKey.c_str());
            return false;
        }
    } else {
        LOGE("invalid resultNapiType");
        return false;
    }
    return true;
}

napi_value FileAssetNapi::UserFileMgrSet(napi_env env, napi_callback_info info)
{
   
    napi_value ret = nullptr;
    unique_ptr<FileAssetAsyncContext> asyncContext = make_unique<FileAssetAsyncContext>();
    CHECK_NULL_PTR_RETURN_UNDEFINED(env, asyncContext, ret, "asyncContext context is null");
    string inputKey;
    CHECK_ARGS(env, MediaLibraryNapiUtils::ParseArgsStringCallback(env, info, asyncContext, inputKey),
        JS_ERR_PARAMETER_INVALID);
    string value;
    CHECK_ARGS(env, MediaLibraryNapiUtils::GetParamStringPathMax(env, asyncContext->argv[ARGS_ONE], value),
        JS_ERR_PARAMETER_INVALID);
    napi_value jsResult = nullptr;
    napi_get_undefined(env, &jsResult);
    auto obj = asyncContext->objectInfo;
    if (!obj->HandleParamSet(inputKey, value, obj->fileAssetPtr->GetResultNapiType())) {
        NapiError::ThrowError(env, JS_E_FILE_KEY);
        return jsResult;
    }
    return jsResult;
}

napi_value FileAssetNapi::PhotoAccessHelperCommitModify(napi_env env, napi_callback_info info)
{
    
    napi_value ret = nullptr;
    unique_ptr<FileAssetAsyncContext> asyncContext = make_unique<FileAssetAsyncContext>();
    CHECK_NULL_PTR_RETURN_UNDEFINED(env, asyncContext, ret, "asyncContext context is null");
    asyncContext->resultNapiType = ResultNapiType::TYPE_PHOTOACCESS_HELPER;
    NAPI_ASSERT(env, MediaLibraryNapiUtils::ParseArgsOnlyCallBack(env, info, asyncContext) == napi_ok,
        "Failed to parse js args");
    asyncContext->objectPtr = asyncContext->objectInfo->fileAssetPtr;
    CHECK_NULL_PTR_RETURN_UNDEFINED(env, asyncContext->objectPtr, ret, "FileAsset is nullptr");

    return MediaLibraryNapiUtils::NapiCreateAsyncWork(env, asyncContext, "PhotoAccessHelperCommitModify",
        JSCommitModifyExecute, JSCommitModifyCompleteCallback);
}

static void PhotoAccessHelperFavoriteComplete(napi_env env, napi_status status, void *data)
{
    FileAssetAsyncContext *context = static_cast<FileAssetAsyncContext*>(data);
    CHECK_NULL_PTR_RETURN_VOID(context, "Async context is null");
    unique_ptr<JSAsyncContextOutput> jsContext = make_unique<JSAsyncContextOutput>();
    jsContext->status = false;

    if (context->error == ERR_DEFAULT) {
        napi_create_int32(env, context->changedRows, &jsContext->data);
        jsContext->status = true;
        napi_get_undefined(env, &jsContext->error);
    } else {
        MediaLibraryNapiUtils::CreateNapiErrorObject(env, jsContext->error, context->changedRows,
            "Failed to modify favorite state");
        napi_get_undefined(env, &jsContext->data);
    }

    if (context->work != nullptr) {
        MediaLibraryNapiUtils::InvokeJSAsyncMethod(env, context->deferred, context->callbackRef,
            context->work, *jsContext);
    }
    delete context;
}

static void PhotoAccessHelperFavoriteExecute(napi_env env, void *data)
{
    auto *context = static_cast<FileAssetAsyncContext *>(data);
    string uri;
    if (context->objectPtr->GetMediaType() == MEDIA_TYPE_IMAGE ||
        context->objectPtr->GetMediaType() == MEDIA_TYPE_VIDEO) {
        uri = PAH_UPDATE_PHOTO;
    } else {
        context->SaveError(-EINVAL);
        return;
    }

    MediaLibraryNapiUtils::UriAppendKeyValue(uri, API_VERSION, to_string(MEDIA_API_VERSION_V10));
    std::string updateAssetUri(uri);
    DataSharePredicates predicates;
    DataShareValuesBucket valuesBucket;
    int32_t changedRows = 0;
    valuesBucket.Put(MediaColumn::MEDIA_IS_FAV, context->isFavorite ? IS_FAV : NOT_FAV);
    predicates.SetWhereClause(MediaColumn::MEDIA_ID + " = ? ");
    predicates.SetWhereArgs({ std::to_string(context->objectPtr->GetId()) });

    changedRows = UserFileClient::Update(updateAssetUri, predicates, valuesBucket);
    if (changedRows < 0) {
        context->SaveError(changedRows);
        LOGE("Failed to modify favorite state, err: %{public}d", changedRows);
    } else {
        context->objectPtr->SetFavorite(context->isFavorite);
        context->changedRows = changedRows;
    }
}

napi_value FileAssetNapi::PhotoAccessHelperFavorite(napi_env env, napi_callback_info info)
{
    
    if (!MediaLibraryNapiUtils::IsSystemApp()) {
        NapiError::ThrowError(env, E_CHECK_SYSTEMAPP_FAIL, "This interface can be called only by system apps");
        return nullptr;
    }

    napi_value ret = nullptr;
    unique_ptr<FileAssetAsyncContext> asyncContext = make_unique<FileAssetAsyncContext>();
    CHECK_NULL_PTR_RETURN_UNDEFINED(env, asyncContext, ret, "asyncContext context is null");
    asyncContext->resultNapiType = ResultNapiType::TYPE_PHOTOACCESS_HELPER;
    NAPI_ASSERT(
        env, MediaLibraryNapiUtils::ParseArgsBoolCallBack(env, info, asyncContext, asyncContext->isFavorite) == napi_ok,
        "Failed to parse js args");
    asyncContext->objectPtr = asyncContext->objectInfo->fileAssetPtr;
    CHECK_NULL_PTR_RETURN_UNDEFINED(env, asyncContext->objectPtr, ret, "FileAsset is nullptr");

    return MediaLibraryNapiUtils::NapiCreateAsyncWork(env, asyncContext, "PhotoAccessHelperFavorite",
        PhotoAccessHelperFavoriteExecute, PhotoAccessHelperFavoriteComplete);
}
} // namespace Media
} // namespace OHOS
