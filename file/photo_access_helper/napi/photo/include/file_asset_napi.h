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

#ifndef INTERFACES_KITS_JS_MEDIALIBRARY_INCLUDE_FILE_ASSET_NAPI_H_
#define INTERFACES_KITS_JS_MEDIALIBRARY_INCLUDE_FILE_ASSET_NAPI_H_

#include <mutex>

#include "file_asset.h"
#include "medialibrary_type_const.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_error.h"
#include "values_bucket.h"
#include "datashare_predicates.h"
#include "datashare_values_bucket.h"

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))
static const std::string FILE_ASSET_NAPI_CLASS_NAME = "FileAsset";
static const std::string USERFILEMGR_FILEASSET_NAPI_CLASS_NAME = "UserFileMgrFileAsset";
static const std::string PHOTOACCESSHELPER_FILEASSET_NAPI_CLASS_NAME = "PhotoAccessHelperFileAsset";

struct AnalysisSourceInfo {
    std::string uriStr;
    std::vector<std::string> fetchColumn;
};

class FileAssetNapi {
public:
    EXPORT FileAssetNapi();
    EXPORT ~FileAssetNapi();

    EXPORT static napi_value Init(napi_env env, napi_value exports);
    EXPORT static napi_value UserFileMgrInit(napi_env env, napi_value exports);
    EXPORT static napi_value PhotoAccessHelperInit(napi_env env, napi_value exports);
    EXPORT static napi_value CreateFileAsset(napi_env env, std::unique_ptr<FileAsset> &iAsset);
    EXPORT static napi_value CreatePhotoAsset(napi_env env, std::shared_ptr<FileAsset> &fileAsset);
    EXPORT static napi_value AttachCreateFileAsset(napi_env env, std::shared_ptr<FileAsset> &iAsset);

    std::string GetFileDisplayName() const;
    std::string GetRelativePath() const;
    std::string GetFilePath() const;
    std::string GetTitle() const;
    std::string GetFileUri() const;
    int32_t GetFileId() const;
    int32_t GetOrientation() const;
    MediaType GetMediaType() const;
    std::string GetNetworkId() const;
    bool IsFavorite() const;
    void SetFavorite(bool isFavorite);
    bool IsHidden() const;
    void SetHidden(bool isHidden);
    std::shared_ptr<FileAsset> GetFileAssetInstance() const;

private:
    EXPORT static void FileAssetNapiDestructor(napi_env env, void *nativeObject, void *finalize_hint);
    EXPORT static napi_value FileAssetNapiConstructor(napi_env env, napi_callback_info info);

    EXPORT static napi_value JSGetFileUri(napi_env env, napi_callback_info info);
    EXPORT static napi_value JSGetFileDisplayName(napi_env env, napi_callback_info info);
    EXPORT static napi_value JSGetMediaType(napi_env env, napi_callback_info info);
    EXPORT static napi_value JSGetAlbumId(napi_env env, napi_callback_info info);
    EXPORT static napi_value JSGetAlbumName(napi_env env, napi_callback_info info);
    EXPORT static napi_value JSSetFileDisplayName(napi_env env, napi_callback_info info);

    EXPORT static napi_value JSCommitModify(napi_env env, napi_callback_info info);
    EXPORT static napi_value JSGetCount(napi_env env, napi_callback_info info);
    void UpdateFileAssetInfo();
    EXPORT static napi_value UserFileMgrSet(napi_env env, napi_callback_info info);
    EXPORT static napi_value UserFileMgrGet(napi_env env, napi_callback_info info);
    EXPORT static napi_value UserFileMgrOpen(napi_env env, napi_callback_info info);
    EXPORT static napi_value UserFileMgrClose(napi_env env, napi_callback_info info);
    EXPORT static napi_value UserFileMgrCommitModify(napi_env env, napi_callback_info info);
    EXPORT static napi_value JSGetExif(napi_env env, napi_callback_info info);

    EXPORT static napi_value PhotoAccessHelperCommitModify(napi_env env, napi_callback_info info);
    EXPORT static napi_value PhotoAccessHelperFavorite(napi_env env, napi_callback_info info);
    EXPORT static napi_value PhotoAccessHelperGetThumbnail(napi_env env, napi_callback_info info);

    bool HandleParamSet(const std::string &inputKey, const std::string &value, ResultNapiType resultNapiType);
    napi_env env_;

    static thread_local napi_ref sConstructor_;
    static thread_local napi_ref userFileMgrConstructor_;
    static thread_local napi_ref photoAccessHelperConstructor_;
    static thread_local std::shared_ptr<FileAsset> sFileAsset_;
    std::shared_ptr<FileAsset> fileAssetPtr = nullptr;
    std::unordered_map<std::string, std::variant<int32_t, int64_t, std::string, double>> member_;
};
struct FileAssetAsyncContext : public NapiError {
    napi_async_work work;
    napi_deferred deferred;
    napi_ref callbackRef;
    bool status;
    FileAssetNapi *objectInfo;
    std::shared_ptr<FileAsset> objectPtr = nullptr;
    DataShare::DataShareValuesBucket valuesBucket;
    bool isDirectory;
    int32_t changedRows;
    int32_t fd;
    bool isFavorite = false;
    bool isTrash = false;
    bool isHidden = false;
    bool isPending = false;
    bool hasEdit = false;
    std::string networkId;
    std::string analysisData;

    size_t argc;
    napi_value argv[NAPI_ARGC_MAX];
    ResultNapiType resultNapiType;
    std::string userComment;
    std::string jsonStr;
    std::string editData;
    std::string uri;
    char* editDataBuffer;
};
} // namespace Media
} // namespace OHOS

#endif  // INTERFACES_KITS_JS_MEDIALIBRARY_INCLUDE_FILE_ASSET_NAPI_H_
