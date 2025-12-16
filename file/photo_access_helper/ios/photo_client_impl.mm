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

#import <Photos/Photos.h>
#import "photoPluginManager.h"

#include <string>
#include <vector>
#include "photo_client_impl.h"
#include "result_set_album.h"
#include "result_set_ios.h"
#include "log.h"
#include "plugins/file/photo_access_helper/napi/photo/photopicker/include/photo_picker_callback.h"
#include "rdb_utils_ios.h"
#include "rdb_utils_result.h"
#include "medialibrary_errno.h"

#define RESULT_OK 0

namespace OHOS {
namespace Media {
std::shared_ptr<PhotoClient> PhotoClient::GetInstance()
{
    return std::make_shared<PhotoClientImpl>();
}

void PhotoClientImpl::startPhotoPicker(std::string &type) {
    NSString *ocType = [NSString stringWithCString:type.c_str() encoding:[NSString defaultCStringEncoding]];
    [[photoPluginManager shareManager] startPhotoPickerIosWithType:ocType callBack:^(NSArray<NSString *> * _Nonnull results, int errorCode) {
        std::vector<std::string> uriNow(0);
        for(int i = 0; i<results.count; i++) {
            NSString *ocString = [results objectAtIndex:i];
            std::string cStringNow = [ocString UTF8String];
            uriNow.emplace_back(cStringNow);
        }
        std::shared_ptr<OHOS::Media::PickerCallBack> photoPickerCallback = OHOS::Media::PhotoPickerCallback::pickerCallBack;
        if (photoPickerCallback != nullptr) {
            photoPickerCallback->resultCode = RESULT_OK;
            photoPickerCallback->uris = uriNow;
            photoPickerCallback->ready = true;
            photoPickerCallback->isOrigin = true;
        }
    }];
}

std::shared_ptr<ResultSet> PhotoClientImpl::QueryAlbum(DataShare::DataSharePredicates &predicates,
        std::vector<std::string> &columns, int &errCode)
{
    bool hasPermission = [[photoPluginManager shareManager] checkPhotoPermission];
    if (!hasPermission) {
        errCode = E_PERMISSION_DENIED;
        return nullptr;
    }
    std::optional<RdbUtilsResult> rdbUtilsResultOptional = RdbUtilsIos::ToPredicates(predicates, true);
    if (!rdbUtilsResultOptional.has_value()) {
        PHFetchResult *result;
        return std::make_shared<Media::ResultSetAlbum>(result);
    }
    RdbUtilsResult rdbUtilsResult = rdbUtilsResultOptional.value();
    int limit = rdbUtilsResult.GetLimit();
    int offset = rdbUtilsResult.GetOffset();
    int albumType = rdbUtilsResult.GetAlbumType();
    long long albumSubType = rdbUtilsResult.GetAlbumSubType();
    NSMutableDictionary *orderDic = [NSMutableDictionary dictionary];
    std::map<std::string, int> order = rdbUtilsResult.GetOrder();
    if (order.size() > 0) {
        for (const auto& pair : order) {
            NSString *field = [NSString stringWithCString:pair.first.c_str() encoding:[NSString defaultCStringEncoding]];
            int isAscending = pair.second;
            [orderDic setObject:[NSNumber numberWithInt:isAscending] forKey:field];
        }
    }

    PHFetchResult *result = [[photoPluginManager shareManager] getAlbumFetchResultLimit:limit
                                                                                 offset:offset
                                                                                sortKey:orderDic
                                                                              albumType:albumType
                                                                           albumSubType:albumSubType];
    std::shared_ptr<Media::ResultSetAlbum> resultSet = std::make_shared<Media::ResultSetAlbum>(result);
    return resultSet;
}

std::shared_ptr<ResultSet> PhotoClientImpl::Query(DataShare::DataSharePredicates &predicates,
    std::vector<std::string> &columns, int &errCode)
{
    bool hasPermission = [[photoPluginManager shareManager] checkPhotoPermission];
    if (!hasPermission) {
        errCode = E_PERMISSION_DENIED;
        return nullptr;
    }
    std::optional<RdbUtilsResult> rdbUtilsResultOptional = RdbUtilsIos::ToPredicates(predicates, false);
    if (!rdbUtilsResultOptional.has_value()) {
        PHFetchResult *result;
        return std::make_shared<Media::ResultSetIos>(result);
    }
    RdbUtilsResult rdbUtilsResult = rdbUtilsResultOptional.value();
    std::string queryArgs = rdbUtilsResult.GetWhereClause();
    std::string localIdentifier = rdbUtilsResult.GetLocalIdentifier();
    NSString *localIdentifierIos = [NSString stringWithCString:localIdentifier.c_str() encoding:[NSString defaultCStringEncoding]];

    NSString *predicate = [NSString stringWithCString:queryArgs.c_str() encoding:[NSString defaultCStringEncoding]];
    int limit = rdbUtilsResult.GetLimit();
    int offset = rdbUtilsResult.GetOffset();
    NSMutableDictionary *orderDic = [NSMutableDictionary dictionary];
    std::map<std::string, int> order = rdbUtilsResult.GetOrder();
    if (order.size() > 0) {
        for (const auto& pair : order) {
            NSString *field = [NSString stringWithCString:pair.first.c_str() encoding:[NSString defaultCStringEncoding]];
            int isAscending = pair.second;
            [orderDic setObject:[NSNumber numberWithInt:isAscending] forKey:field];
        }
    }
    PHFetchResult *result = [[photoPluginManager shareManager] getPhotoFetchResultPredicate:predicate
                                                                                 fetchLimit:limit
                                                                                     offset:offset
                                                                                    sortKey:orderDic
                                                                            localIdentifier:localIdentifierIos];
    std::shared_ptr<Media::ResultSetIos> resultSet = std::make_shared<Media::ResultSetIos>(result);
    return resultSet;
}

int PhotoClientImpl::Update(std::string &uri, const std::string &predicates,
    const std::string &value)
{
    return RESULT_OK;
}

std::string PhotoClientImpl::InsertExt(int photoType, const std::string &extension,
    const std::string &title, int &errCode)
{
    return "";
}

std::string PhotoClientImpl::GetMimeTypeFromExtension(const std::string &extension)
{
    return "";
}
} // namespace Media
} // namespace OHOS