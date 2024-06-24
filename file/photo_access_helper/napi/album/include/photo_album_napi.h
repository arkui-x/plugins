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

#ifndef INTERFACES_KITS_JS_MEDIALIBRARY_INCLUDE_PHOTO_ALBUM_NAPI_H_
#define INTERFACES_KITS_JS_MEDIALIBRARY_INCLUDE_PHOTO_ALBUM_NAPI_H_

#include "photo_album.h"

#include "fetch_result.h"
#include "file_asset.h"
#include "napi_error.h"
#include "datashare_values_bucket.h"

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))
class PhotoAlbumNapi {
public:
    EXPORT PhotoAlbumNapi();
    EXPORT ~PhotoAlbumNapi();

    // EXPORT static napi_value Init(napi_env env, napi_value exports);
    EXPORT static napi_value PhotoAccessInit(napi_env env, napi_value exports);
    EXPORT static napi_value CreatePhotoAlbumNapi(napi_env env, std::unique_ptr<PhotoAlbum> &albumData);
    EXPORT static napi_value CreatePhotoAlbumNapi(napi_env env, std::shared_ptr<PhotoAlbum> &albumData);

    int32_t GetAlbumId() const;
    int32_t GetCount() const;
    int32_t GetImageCount() const;
    int32_t GetVideoCount() const;
    void SetCount(int32_t count);
    void SetImageCount(int32_t count);
    void SetVideoCount(int32_t count);
    const std::string& GetAlbumUri() const;
    const std::string& GetCoverUri() const;
    int64_t GetDateModified() const;
    double GetLatitude() const;
    double GetLongitude() const;
    const std::string& GetAlbumName() const;
    const std::string& GetAlbumLocalIdentifier() const;
    PhotoAlbumType GetPhotoAlbumType() const;
    PhotoAlbumSubType GetPhotoAlbumSubType() const;
    std::shared_ptr<PhotoAlbum> GetPhotoAlbumInstance() const;

    void SetHiddenOnly(const bool hiddenOnly);
    bool GetHiddenOnly() const;

private:
    EXPORT void SetPhotoAlbumNapiProperties();
    EXPORT static napi_value PhotoAlbumNapiConstructor(napi_env env, napi_callback_info info);
    EXPORT static void PhotoAlbumNapiDestructor(napi_env env, void *nativeObject, void *finalizeHint);

    EXPORT static napi_value JSGetPhotoAlbumType(napi_env env, napi_callback_info info);
    EXPORT static napi_value JSGetPhotoAlbumSubType(napi_env env, napi_callback_info info);
    EXPORT static napi_value JSGetCoverUri(napi_env env, napi_callback_info info);
    EXPORT static napi_value JSGetLatitude(napi_env env, napi_callback_info info);
    EXPORT static napi_value JSGetLongitude(napi_env env, napi_callback_info info);

    EXPORT static napi_value PhotoAccessHelperSetCoverUri(napi_env env, napi_callback_info info);

    EXPORT static napi_value JSPhotoAccessGetAlbumName(napi_env env, napi_callback_info info);
    EXPORT static napi_value JSPhotoAccessSetAlbumName(napi_env env, napi_callback_info info);
    EXPORT static napi_value JSPhotoAccessGetAlbumUri(napi_env env, napi_callback_info info);
    EXPORT static napi_value JSPhotoAccessGetAlbumCount(napi_env env, napi_callback_info info);
    EXPORT static napi_value JSPhotoAccessGetAlbumImageCount(napi_env env, napi_callback_info info);
    EXPORT static napi_value JSPhotoAccessGetAlbumVideoCount(napi_env env, napi_callback_info info);
    EXPORT static napi_value JSPhoteAccessGetPhotoAssets(napi_env env, napi_callback_info info);

    EXPORT static napi_value PhotoAccessHelperCommitModify(napi_env env, napi_callback_info info);
    EXPORT static napi_value PhotoAccessHelperAddAssets(napi_env env, napi_callback_info info);
    EXPORT static napi_value PhotoAccessHelperRemoveAssets(napi_env env, napi_callback_info info);
    EXPORT static napi_value PhotoAccessHelperRecoverPhotos(napi_env env, napi_callback_info info);
    EXPORT static napi_value PhotoAccessHelperDeletePhotos(napi_env env, napi_callback_info info);

    napi_env env_;
    std::shared_ptr<PhotoAlbum> photoAlbumPtr;
    static thread_local PhotoAlbum *pAlbumData_;
    static thread_local napi_ref constructor_;
    static thread_local napi_ref photoAccessConstructor_;
};

struct PhotoAlbumNapiAsyncContext : public NapiError {
    int32_t changedRows;
    int32_t newCount;
    int32_t newImageCount;
    int32_t newVideoCount;
    std::vector<std::string> fetchColumn;
    DataShare::DataSharePredicates predicates;
    DataShare::DataShareValuesBucket valuesBucket;
    std::vector<DataShare::DataShareValuesBucket> valuesBuckets;
    std::string networkId;
    std::string uri;
    std::unique_ptr<FetchResult<FileAsset>> fetchResult;
    ResultNapiType resultNapiType;

    size_t argc;
    napi_value argv[NAPI_ARGC_MAX];
    napi_async_work work;
    napi_deferred deferred;
    napi_ref callbackRef;

    PhotoAlbumNapi *objectInfo;
};
} // namespace Media
} // namespace OHOS

#endif  // INTERFACES_KITS_JS_MEDIALIBRARY_INCLUDE_PHOTO_ALBUM_NAPI_H_
