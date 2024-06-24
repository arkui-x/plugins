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

#include "photo_album_column.h"

#include <string>
#include "media_column.h"
#include "medialibrary_type_const.h"
#include "log.h"

namespace OHOS::Media {
using namespace std;
using namespace OHOS::NativeRdb;

// PhotoAlbum table
const string PhotoAlbumColumns::TABLE = "PhotoAlbum";
const string PhotoAlbumColumns::ALBUM_ID = "album_id";
const string PhotoAlbumColumns::ALBUM_TYPE = "album_type";
const string PhotoAlbumColumns::ALBUM_SUBTYPE = "album_subtype";
const string PhotoAlbumColumns::ALBUM_NAME = "album_name";
const string PhotoAlbumColumns::ALBUM_COVER_URI = "cover_uri";
const string PhotoAlbumColumns::ALBUM_COUNT = "count";
const string PhotoAlbumColumns::ALBUM_DATE_MODIFIED = "date_modified";
const string PhotoAlbumColumns::ALBUM_DIRTY = "dirty";
const string PhotoAlbumColumns::ALBUM_CLOUD_ID = "cloud_id";
const string PhotoAlbumColumns::ALBUM_IMAGE_COUNT = "image_count";
const string PhotoAlbumColumns::ALBUM_VIDEO_COUNT = "video_count";
const string PhotoAlbumColumns::ALBUM_LATITUDE = "latitude";
const string PhotoAlbumColumns::ALBUM_LONGITUDE = "longitude";
const string PhotoAlbumColumns::ALBUM_BUNDLE_NAME = "bundle_name";
const string PhotoAlbumColumns::ALBUM_LOCAL_LANGUAGE = "local_language";
const string PhotoAlbumColumns::ALBUM_IS_LOCAL = "is_local";

// For api9 compatibility
const string PhotoAlbumColumns::ALBUM_RELATIVE_PATH = "relative_path";

const string PhotoAlbumColumns::CONTAINS_HIDDEN = "contains_hidden";
const string PhotoAlbumColumns::HIDDEN_COUNT = "hidden_count";
const string PhotoAlbumColumns::HIDDEN_COVER = "hidden_cover";

// For sorting albums
const string PhotoAlbumColumns::ALBUM_ORDER = "album_order";
const string PhotoAlbumColumns::REFERENCE_ALBUM_ID = "reference_album_id";

const string PhotoAlbumColumns::ALBUM_LOCAL_IDENTIFIER = "localIdentifier";

// default fetch columns
const set<string> PhotoAlbumColumns::DEFAULT_FETCH_COLUMNS = {
    ALBUM_ID, ALBUM_NAME, ALBUM_COUNT, ALBUM_DATE_MODIFIED,
};

const string PhotoAlbumColumns::ALBUM_URI_PREFIX = "file://media/PhotoAlbum/";
const string PhotoAlbumColumns::DEFAULT_PHOTO_ALBUM_URI = "file://media/PhotoAlbum";
const string PhotoAlbumColumns::HIDDEN_ALBUM_URI_PREFIX = "file://media/HiddenAlbum/";
const string PhotoAlbumColumns::DEFAULT_HIDDEN_ALBUM_URI = "file://media/HiddenAlbum";
const string PhotoAlbumColumns::ANALYSIS_ALBUM_URI_PREFIX = "file://media/AnalysisAlbum/";

bool PhotoAlbumColumns::IsPhotoAlbumColumn(const string &columnName)
{
    static const set<string> PHOTO_ALBUM_COLUMNS = {
        PhotoAlbumColumns::ALBUM_ID, PhotoAlbumColumns::ALBUM_TYPE, PhotoAlbumColumns::ALBUM_SUBTYPE,
        PhotoAlbumColumns::ALBUM_NAME, PhotoAlbumColumns::ALBUM_COVER_URI, PhotoAlbumColumns::ALBUM_COUNT,
        PhotoAlbumColumns::ALBUM_RELATIVE_PATH, CONTAINS_HIDDEN, HIDDEN_COUNT, HIDDEN_COVER
    };
    return PHOTO_ALBUM_COLUMNS.find(columnName) != PHOTO_ALBUM_COLUMNS.end();
}
} // namespace OHOS::Media
