/*
 * Copyright (C) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_MEDIALIBRARY_INCLUDE_MEDIA_LIBRARY_NAPI_H_
#define INTERFACES_KITS_JS_MEDIALIBRARY_INCLUDE_MEDIA_LIBRARY_NAPI_H_

#include <mutex>
#include <vector>
#include <list>
#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_error.h"
#include "photo_album.h"
#include "values_bucket.h"
#include "datashare_predicates.h"
#include "datashare_values_bucket.h"
#include "fetch_result.h"
#include "uv.h"

using namespace std;

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))
static const std::string USERFILE_MGR_NAPI_CLASS_NAME = "UserFileManager";
static const std::string PHOTOACCESSHELPER_NAPI_CLASS_NAME = "PhotoAccessHelper";

class MediaLibraryNapi {
public:
    EXPORT static napi_value Init(napi_env env, napi_value exports);
    EXPORT static napi_value PhotoAccessHelperInit(napi_env env, napi_value exports);
    EXPORT MediaLibraryNapi();
    EXPORT ~MediaLibraryNapi();

private:
    EXPORT static void MediaLibraryNapiDestructor(napi_env env, void *nativeObject, void *finalize_hint);
    EXPORT static napi_value MediaLibraryNapiConstructor(napi_env env, napi_callback_info info);

    EXPORT static napi_value JSGetAlbums(napi_env env, napi_callback_info info);
    EXPORT static napi_value JSRelease(napi_env env, napi_callback_info info);
    EXPORT static napi_value CreatePhotoKeysEnum(napi_env env);
    EXPORT static napi_value CreateMediaTypeUserFileEnum(napi_env env);
    EXPORT static napi_value CreateAlbumKeyEnum(napi_env env);
    EXPORT static napi_value CreatePhotoSubTypeEnum(napi_env env);

    EXPORT static napi_value GetPhotoAccessHelper(napi_env env, napi_callback_info info);
    EXPORT static napi_value GetPhotoAccessHelperAsync(napi_env env, napi_callback_info info);
    EXPORT static napi_value PhotoAccessHelperCreatePhotoAsset(napi_env env, napi_callback_info info);
    EXPORT static napi_value PhotoAccessHelperOnCallback(napi_env env, napi_callback_info info);
    EXPORT static napi_value PhotoAccessHelperOffCallback(napi_env env, napi_callback_info info);
    EXPORT static napi_value PhotoAccessGetPhotoAssets(napi_env env, napi_callback_info info);
    EXPORT static napi_value PhotoAccessGetPhotoAlbums(napi_env env, napi_callback_info info);
    EXPORT static napi_value SetHidden(napi_env env, napi_callback_info info);

    EXPORT static napi_value CreateAlbumTypeEnum(napi_env env);
    EXPORT static napi_value CreateAlbumSubTypeEnum(napi_env env);
    EXPORT static napi_value CreateNotifyTypeEnum(napi_env env);
    EXPORT static napi_value CreateDefaultChangeUriEnum(napi_env env);
    EXPORT static napi_value CreateAnalysisTypeEnum(napi_env env);
    EXPORT static napi_value CreateRequestPhotoTypeEnum(napi_env env);
    EXPORT static napi_value CreateResourceTypeEnum(napi_env env);
    EXPORT static napi_value GetPhotoAlbums(napi_env env, napi_callback_info info);
    EXPORT static napi_value JSGetPhotoIndex(napi_env env, napi_callback_info info);
    EXPORT static napi_value PhotoAccessGetPhotoIndex(napi_env env, napi_callback_info info);

    EXPORT static napi_value JSApplyChanges(napi_env env, napi_callback_info info);

    napi_env env_;

    static thread_local napi_ref sConstructor_;
    static thread_local napi_ref userFileMgrConstructor_;
    static thread_local napi_ref photoAccessHelperConstructor_;
    static thread_local napi_ref sMediaTypeEnumRef_;
    static thread_local napi_ref sFileKeyEnumRef_;
    static thread_local napi_ref sPhotoKeysEnumRef_;
    static thread_local napi_ref sAlbumKeyEnumRef_;
    static thread_local napi_ref sAlbumType_;
    static thread_local napi_ref sAlbumSubType_;
    static thread_local napi_ref sPositionTypeEnumRef_;
    static thread_local napi_ref sPhotoSubType_;
    static thread_local napi_ref sRequestPhotoTypeEnumRef_;
    static thread_local napi_ref sResourceTypeEnumRef_;

    static std::mutex sOnOffMutex_;
};

constexpr int32_t DEFAULT_PRIVATEALBUMTYPE = 3;
struct MediaLibraryAsyncContext : public NapiError {
    napi_async_work work;
    napi_deferred deferred;
    napi_ref callbackRef;
    bool status;
    bool isDelete;
    bool isCreateByComponent;
    NapiAssetType assetType;
    AlbumType albumType;
    MediaLibraryNapi *objectInfo;
    std::string selection;
    std::vector<std::string> selectionArgs;
    std::string order;
    std::string uri;
    std::string networkId;
    std::string extendArgs;
    std::unique_ptr<FetchResult<FileAsset>> fetchFileResult;
    std::unique_ptr<FetchResult<PhotoAlbum>> fetchPhotoAlbumResult;
    std::unique_ptr<FileAsset> fileAsset;
    std::unique_ptr<PhotoAlbum> photoAlbumData;
    DataShare::DataShareValuesBucket valuesBucket;
    unsigned int dirType = 0;
    int32_t privateAlbumType = DEFAULT_PRIVATEALBUMTYPE;
    int32_t retVal;
    std::string directoryRelativePath;
    std::string storeMediaSrc;
    int32_t imagePreviewIndex;
    int32_t parentSmartAlbumId = 0;
    int32_t smartAlbumId = -1;
    int32_t isAnalysisAlbum = 0;
    int32_t isLocationAlbum = 0;
    size_t argc;
    napi_value argv[NAPI_ARGC_MAX];
    ResultNapiType resultNapiType;
    std::string tableName;
    std::vector<uint32_t> mediaTypes;
    OHOS::DataShare::DataSharePredicates predicates;
    std::vector<std::string> fetchColumn;
    std::vector<std::string> uris;
    bool hiddenOnly = false;
    int32_t hiddenAlbumFetchMode = -1;
    std::string formId;
    MediaType mediaType;
};

struct MediaLibraryInitContext : public NapiError  {
    napi_async_work work;
    napi_deferred deferred;
    napi_ref callbackRef;
    size_t argc;
    napi_value argv[NAPI_ARGC_MAX];
    napi_ref resultRef_;
};
} // namespace Media
} // namespace OHOS

#endif  // INTERFACES_KITS_JS_MEDIALIBRARY_INCLUDE_MEDIA_LIBRARY_NAPI_H_
