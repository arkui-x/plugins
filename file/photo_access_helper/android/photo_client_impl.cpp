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

#include "photo_client_impl.h"

#include <algorithm>
#include <vector>

#include "log.h"
#include "java/jni/photo_plugin_jni.h"
#include "java/jni/photo_cursor_jni.h"
#include "datashare_predicates.h"
#include "rdb_predicates.h"
#include "plugins/file/photo_access_helper/napi/photo/include/media_column.h"
#include "plugins/file/photo_access_helper/napi/base/include/rdb_utils.h"
#include "plugins/file/photo_access_helper/napi/album/include/photo_album_column.h"
#include "plugins/file/photo_access_helper/napi/photo/include/medialibrary_errno.h"

using namespace std;

namespace OHOS {
namespace Media {

static const double SINGLE_VALUE = 2.0;
static const int ANDROID_PHOTO_MEDIA_TYPE = 3;

static const std::map<std::string, std::string> PHOTO_COLUMN_MAP = {
    { PhotoColumn::MEDIA_FILE_PATH, "_data" },
    { PhotoColumn::MEDIA_ID, "_id" },
    { PhotoColumn::MEDIA_SIZE, "_size" },
    { PhotoColumn::MEDIA_NAME, "_display_name" },
    { PhotoColumn::MEDIA_OWNER_PACKAGE, "owner_package_name" },
    { PhotoColumn::MEDIA_DATE_TAKEN, "datetaken" },
    { Media::PhotoAlbumColumns::ALBUM_ID, "bucket_id" },
    { Media::PhotoAlbumColumns::ALBUM_NAME, "bucket_display_name" },
    { Media::PhotoAlbumColumns::ALBUM_COUNT, "count(_data)" },
    { "sum", "sum(media_type)" },
};

static void ChangeArgsToString(const std::vector<NativeRdb::ValueObject> &values, std::vector<std::string> &args)
{
    for (size_t i = 0; i < values.size(); i++) {
        const auto &arg = values[i];
        std::string str;
        arg.GetString(str);
        auto it = PHOTO_COLUMN_MAP.find(str);
        if (it != PHOTO_COLUMN_MAP.end()) {
            std::string androidColumnName = it->second;
            args.push_back(androidColumnName);
        } else {
            args.push_back(str);
        }
    }
}

static void ChangeColumnToAndroid(const std::vector<std::string> &columns, std::vector<std::string> &androidColumns)
{
    for (size_t i = 0; i < columns.size(); i++) {
        const auto &arg = columns[i];
        auto it = PHOTO_COLUMN_MAP.find(arg);
        if (it != PHOTO_COLUMN_MAP.end()) {
            std::string androidColumnName = it->second;
            androidColumns.push_back(androidColumnName);
        } else {
            androidColumns.push_back(arg);
        }
    }
}

static void AddAndroidPredicates(DataShare::DataSharePredicates &predicates)
{
    bool isContainMediaType = false;
    std::vector<DataShare::OperationItem> operations;
    auto &items = predicates.GetOperationList();
    for (auto &item : items) {
        if (item.singleParams.empty()) {
            operations.push_back(item);
            continue;
        }
        if (static_cast<string>(item.GetSingle(0)) == MediaColumn::MEDIA_TYPE) {
            isContainMediaType = true;
            if (fabs(static_cast<double>(item.GetSingle(1)) - SINGLE_VALUE) < 1e-6) {
                operations.push_back({ item.operation, { MediaColumn::MEDIA_TYPE, ANDROID_PHOTO_MEDIA_TYPE}});
            } else {
                operations.push_back(item);
            }
            continue;
        }
        operations.push_back(item);
    }
    if (!isContainMediaType) {
        std::vector<std::string> whereArgs = {"1", "3"};
        predicates.In(PhotoColumn::MEDIA_TYPE, whereArgs);
    } else {
        predicates = DataShare::DataSharePredicates(move(operations));
    }
}

static std::string changeStringToAndroidString(std::string predicatesString) {
    for (auto it = PHOTO_COLUMN_MAP.begin(); it != PHOTO_COLUMN_MAP.end(); it++) {
        size_t pos = predicatesString.find(it->first);
        if (pos != std::string::npos && (pos == 0 || predicatesString.at(pos - 1) == ' ' || predicatesString.at(pos - 1) == ',')) {
            predicatesString.replace(pos, it->first.length(), it->second);
        }
    }
    return predicatesString;
}

static void ProcessPredicates(NativeRdb::RdbPredicates &rdbPredicates)
{
    std::string whereClause = rdbPredicates.GetWhereClause();
    whereClause = changeStringToAndroidString(whereClause);
    rdbPredicates.SetWhereClause(whereClause);

    std::string order = rdbPredicates.GetOrder();
    order = changeStringToAndroidString(order);
    rdbPredicates.SetOrder(order);
}

std::shared_ptr<PhotoClient> PhotoClient::GetInstance()
{
    return std::make_shared<PhotoClientImpl>();
}

void PhotoClientImpl::startPhotoPicker(std::string &type) {
    OHOS::Plugin::PhotoPluginJni::startPhotoPicker(type);
}

std::shared_ptr<ResultSet> PhotoClientImpl::QueryAlbum(DataShare::DataSharePredicates &predicates,
    std::vector<std::string> &columns, int &errCode)
{
    if (!OHOS::Plugin::PhotoPluginJni::checkPermission()) {
        errCode = Media::E_PERMISSION_DENIED;
        return nullptr;
    }
    AddAndroidPredicates(predicates);
    columns.push_back("sum");
    columns.push_back("_id");
    NativeRdb::RdbPredicates rdbPredicates = RdbDataShareAdapter::RdbUtils::ToPredicates(predicates, "");

    ProcessPredicates(rdbPredicates);

    std::vector<std::string> args;
    ChangeArgsToString(rdbPredicates.GetBindArgs(), args);

    std::vector<std::string> androidColumns;
    ChangeColumnToAndroid(columns, androidColumns);
    return OHOS::Plugin::PhotoPluginJni::queryAlbum(rdbPredicates, args, androidColumns);
}

std::shared_ptr<ResultSet> PhotoClientImpl::Query(DataShare::DataSharePredicates &predicates,
    std::vector<std::string> &columns, int &errCode)
{
    if (!OHOS::Plugin::PhotoPluginJni::checkPermission()) {
        errCode = Media::E_PERMISSION_DENIED;
        return nullptr;
    }
    AddAndroidPredicates(predicates);
    NativeRdb::RdbPredicates rdbPredicates = RdbDataShareAdapter::RdbUtils::ToPredicates(predicates, "");

    ProcessPredicates(rdbPredicates);

    std::vector<std::string> args;
    ChangeArgsToString(rdbPredicates.GetBindArgs(), args);

    std::vector<std::string> androidColumns;
    ChangeColumnToAndroid(columns, androidColumns);
    return OHOS::Plugin::PhotoPluginJni::queryPhoto(rdbPredicates, args, androidColumns);
}

std::string PhotoClientImpl::InsertExt(int photoType, const std::string &extension,
    const std::string &title, int &errCode)
{
    if (!OHOS::Plugin::PhotoPluginJni::CheckWritePermission()) {
        errCode = Media::E_PERMISSION_DENIED;
        return "";
    }
    return OHOS::Plugin::PhotoPluginJni::CreatePhoto(photoType, extension, title);
}

std::string PhotoClientImpl::GetMimeTypeFromExtension(const std::string &extension)
{
    return OHOS::Plugin::PhotoPluginJni::GetMimeTypeFromExtension(extension);
}

int PhotoClientImpl::Update(std::string &uri, const std::string &predicates,
    const std::string &value)
{
     return 0;
}
}
}
