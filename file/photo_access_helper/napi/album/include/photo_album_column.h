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

#ifndef INTERFACES_INNERKITS_NATIVE_INCLUDE_PHOTO_ALBUM_COLUMN_H
#define INTERFACES_INNERKITS_NATIVE_INCLUDE_PHOTO_ALBUM_COLUMN_H

#include <set>
#include <string>

#include "rdb_predicates.h"
#include "userfile_manager_types.h"

namespace OHOS::Media {
#define EXPORT __attribute__ ((visibility ("default")))

const std::string COVER_URI = "cover_uri";
const std::string COUNT = "count";
const std::string ALBUM_TYPE = "album_type";
const std::string IS_REMOVED = "is_removed";
const std::string USER_OPERATION = "user_operation";

const std::string MAP_ALBUM = "map_album";
const std::string MAP_ASSET = "map_asset";

const std::string TARGET_ALBUM_ID = "target_album_id";

const std::string ANALYSIS_ALBUM_TABLE = "AnalysisAlbum";
const std::string ANALYSIS_PHOTO_MAP_TABLE = "AnalysisPhotoMap";

const std::string ALBUM_ID = "album_id";
const std::string ALBUM_SUBTYPE = "album_subtype";
const std::string ALBUM_NAME = "album_name";
const std::string DATE_MODIFIED = "date_modified";
const std::string RANK = "rank";
const std::string IS_LOCAL = "is_local";

const std::string SMARTALBUM_DB_ID = "album_id";

const std::string FORM_MAP_TABLE  = "FormMap";
const std::string FORMMAP_FORM_ID = "form_id";
const std::string FORMMAP_URI = "uri";

const std::string LATITUDE = "latitude";
const std::string LONGITUDE = "longitude";
const std::string LANGUAGE = "language";
const std::string COUNTRY = "country";
const std::string ADMIN_AREA = "admin_area";
const std::string SUB_ADMIN_AREA = "sub_admin_area";
const std::string LOCALITY = "locality";
const std::string SUB_LOCALITY = "sub_locality";
const std::string THOROUGHFARE = "thoroughfare";
const std::string SUB_THOROUGHFARE = "sub_thoroughfare";
const std::string FEATURE_NAME = "feature_name";
const std::string DICTIONARY_INDEX = "dictionary_index";
const std::string KNOWLEDG_INDEX = "knowledge_index";

class PhotoAlbumColumns {
public:
    // columns only in PhotoAlbumTable
    static const std::string ALBUM_ID EXPORT;
    static const std::string ALBUM_TYPE EXPORT;
    static const std::string ALBUM_SUBTYPE EXPORT;
    static const std::string ALBUM_NAME EXPORT;
    static const std::string ALBUM_COVER_URI EXPORT;
    static const std::string ALBUM_COUNT EXPORT;
    static const std::string ALBUM_DATE_MODIFIED EXPORT;
    static const std::string ALBUM_DIRTY EXPORT;
    static const std::string ALBUM_CLOUD_ID EXPORT;
    static const std::string ALBUM_IMAGE_COUNT EXPORT;
    static const std::string ALBUM_VIDEO_COUNT EXPORT;
    static const std::string ALBUM_LATITUDE EXPORT;
    static const std::string ALBUM_LONGITUDE EXPORT;
    static const std::string ALBUM_BUNDLE_NAME EXPORT;
    static const std::string ALBUM_LOCAL_LANGUAGE EXPORT;
    static const std::string ALBUM_IS_LOCAL EXPORT;

    // For api9 compatibility
    static const std::string ALBUM_RELATIVE_PATH EXPORT;

    static const std::string CONTAINS_HIDDEN EXPORT;
    static const std::string HIDDEN_COUNT EXPORT;
    static const std::string HIDDEN_COVER EXPORT;

    // For sorting albums
    static const std::string ALBUM_ORDER EXPORT;
    static const std::string REFERENCE_ALBUM_ID EXPORT;

    static const std::string ALBUM_LOCAL_IDENTIFIER EXPORT;

    // default fetch columns
    static const std::set<std::string> DEFAULT_FETCH_COLUMNS EXPORT;

    // table name
    static const std::string TABLE EXPORT;
    // create PhotoAlbumTable sql
    static const std::string CREATE_TABLE EXPORT;

    // util constants
    static const std::string ALBUM_URI_PREFIX EXPORT;
    static const std::string DEFAULT_PHOTO_ALBUM_URI EXPORT;
    static const std::string HIDDEN_ALBUM_URI_PREFIX EXPORT;
    static const std::string DEFAULT_HIDDEN_ALBUM_URI EXPORT;
    static const std::string ANALYSIS_ALBUM_URI_PREFIX EXPORT;

    EXPORT static bool IsPhotoAlbumColumn(const std::string &columnName);
};
} // namespace OHOS::Media
#endif // INTERFACES_INNERKITS_NATIVE_INCLUDE_PHOTO_ALBUM_COLUMN_H
