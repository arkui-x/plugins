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

#ifndef INTERFACES_INNERKITS_NATIVE_INCLUDE_MEDIA_DATA_ABILITY_CONST_H_
#define INTERFACES_INNERKITS_NATIVE_INCLUDE_MEDIA_DATA_ABILITY_CONST_H_

#include <map>

#include "medialibrary_type_const.h"
#include "media_column.h"

namespace OHOS {
namespace Media {
const int32_t MEDIA_RDB_VERSION = 61;

enum {
    MEDIA_API_VERSION_DEFAULT = 8,
    MEDIA_API_VERSION_V9,
    MEDIA_API_VERSION_V10,
};


const std::string MEDIA_LIBRARY_VERSION = "1.0";

const int32_t DEVICE_SYNCSTATUSING = 0;
const int32_t DEVICE_SYNCSTATUS_COMPLETE = 1;

const std::string MEDIA_DATA_DEVICE_PATH = "local";
const std::string MEDIALIBRARY_TABLE = "Files";
const std::string SMARTALBUM_TABLE = "SmartAlbum";
const std::string SMARTALBUM_MAP_TABLE = "SmartMap";
const std::string CATEGORY_SMARTALBUM_MAP_TABLE = "CategorySmartAlbumMap";
const std::string MEDIATYPE_DIRECTORY_TABLE = "MediaTypeDirectory";
const std::string DEVICE_TABLE = "Device";
const std::string BUNDLE_PERMISSION_TABLE = "BundlePermission";
const std::string MEDIA_DATA_ABILITY_DB_NAME = "media_library.db";

const std::string BUNDLE_NAME = "com.ohos.medialibrary.medialibrarydata";

const std::string ML_FILE_URI_PREFIX = "file://media";
const std::string MEDIALIBRARY_DATA_ABILITY_PREFIX = "datashare://";
const std::string MEDIALIBRARY_DATA_URI_IDENTIFIER = "/media";
const std::string MEDIALIBRARY_MEDIA_PREFIX = MEDIALIBRARY_DATA_ABILITY_PREFIX +
                                                     MEDIALIBRARY_DATA_URI_IDENTIFIER;
const std::string MEDIALIBRARY_TYPE_FILE_URI  =  "/file";
const std::string MEDIALIBRARY_TYPE_ALBUM_URI  =  "/album";

const std::string FILE_URI_PREFIX = ML_FILE_URI_PREFIX + MEDIALIBRARY_TYPE_FILE_URI;
const std::string ALBUM_URI_PREFIX = ML_FILE_URI_PREFIX + MEDIALIBRARY_TYPE_ALBUM_URI;

const std::string URI_TYPE_PHOTO = "Photo";
const std::string URI_TYPE_AUDIO_V10 = "Audio";
const std::string URI_TYPE_PHOTO_ALBUM = "PhotoAlbum";
constexpr int64_t AGING_TIME = 30LL * 60 * 60 * 24 * 1000;

const std::string MEDIALIBRARY_SMARTALBUM_URI = SMARTALBUM_TABLE;
const std::string MEDIALIBRARY_SMARTALBUM_MAP_URI =SMARTALBUM_MAP_TABLE;
const std::string MEDIALIBRARY_CATEGORY_SMARTALBUM_MAP_URI = CATEGORY_SMARTALBUM_MAP_TABLE;
const std::string MEDIALIBRARY_DIRECTORY_URI = MEDIATYPE_DIRECTORY_TABLE;
const std::string MEDIALIBRARY_BUNDLEPERM_URI = "bundle_permission_insert_operation";

const std::string MEDIALIBRARY_AUDIO_URI = "audio";
const std::string MEDIALIBRARY_VIDEO_URI = "video";
const std::string MEDIALIBRARY_IMAGE_URI = "image";
const std::string MEDIALIBRARY_FILE_URI  =  "file";
const std::string MEDIALIBRARY_ALBUM_URI  ="album";
const std::string MEDIALIBRARY_SMARTALBUM_CHANGE_URI  ="smartalbum";
const std::string MEDIALIBRARY_DEVICE_URI  ="device";
const std::string MEDIALIBRARY_SMART_URI = "smart";
const std::string MEDIALIBRARY_REMOTEFILE_URI = "remotfile";

const std::string MEDIA_DATA_DB_ID = "file_id";
const std::string MEDIA_DATA_DB_URI = "uri";
const std::string MEDIA_DATA_DB_FILE_PATH = "data";
const std::string MEDIA_DATA_DB_SIZE = "size";
const std::string MEDIA_DATA_DB_PARENT_ID = "parent";
const std::string MEDIA_DATA_DB_DATE_MODIFIED = "date_modified";
const std::string MEDIA_DATA_DB_DATE_MODIFIED_S = "date_modified_s";
const std::string MEDIA_DATA_DB_DATE_MODIFIED_MS = "date_modified_ms";
const std::string MEDIA_DATA_DB_DATE_MODIFIED_TO_SECOND = "CAST(date_modified / 1000 AS BIGINT) AS date_modified_s";
const std::string MEDIA_DATA_DB_DATE_ADDED = "date_added";
const std::string MEDIA_DATA_DB_DATE_ADDED_S = "date_added_s";
const std::string MEDIA_DATA_DB_DATE_ADDED_MS = "date_added_ms";
const std::string MEDIA_DATA_DB_DATE_ADDED_TO_SECOND = "CAST(date_added / 1000 AS BIGINT) AS date_added_s";
const std::string MEDIA_DATA_DB_MIME_TYPE = "mime_type";
const std::string MEDIA_DATA_DB_TITLE = "title";
const std::string MEDIA_DATA_DB_DESCRIPTION = "description";
const std::string MEDIA_DATA_DB_NAME = "display_name";
const std::string MEDIA_DATA_DB_ORIENTATION = "orientation";
const std::string MEDIA_DATA_DB_LATITUDE = "latitude";
const std::string MEDIA_DATA_DB_LONGITUDE = "longitude";
const std::string MEDIA_DATA_DB_DATE_TAKEN = "date_taken";
const std::string MEDIA_DATA_DB_THUMBNAIL = "thumbnail";
const std::string MEDIA_DATA_DB_THUMB_ASTC = "astc";
const std::string MEDIA_DATA_DB_HAS_ASTC = "has_astc";
const std::string MEDIA_DATA_DB_CONTENT_CREATE_TIME = "content_create_time";
const std::string MEDIA_DATA_DB_POSITION = "position";
const std::string MEDIA_DATA_DB_DIRTY = "dirty";
const std::string MEDIA_DATA_DB_CLOUD_ID = "cloud_id";
const std::string MEDIA_DATA_DB_META_DATE_MODIFIED = "meta_date_modified";
const std::string MEDIA_DATA_DB_SYNC_STATUS = "sync_status";

const std::string MEDIA_DATA_DB_LCD = "lcd";
const std::string MEDIA_DATA_DB_BUCKET_ID = "bucket_id";
const std::string MEDIA_DATA_DB_BUCKET_NAME = "bucket_display_name";
const std::string MEDIA_DATA_DB_DURATION = "duration";
const std::string MEDIA_DATA_DB_ARTIST = "artist";

const std::string MEDIA_DATA_DB_AUDIO_ALBUM = "audio_album";
const std::string MEDIA_DATA_DB_MEDIA_TYPE = "media_type";

const std::string MEDIA_DATA_DB_HEIGHT = "height";
const std::string MEDIA_DATA_DB_WIDTH = "width";
const std::string MEDIA_DATA_DB_OWNER_PACKAGE = "owner_package";
const std::string MEDIA_DATA_DB_PACKAGE_NAME = "package_name";

const std::string MEDIA_DATA_DB_IS_FAV = "is_favorite";
const std::string MEDIA_DATA_DB_IS_TRASH = "is_trash";
const std::string MEDIA_DATA_DB_RECYCLE_PATH = "recycle_path";
const std::string MEDIA_DATA_DB_DATE_TRASHED = "date_trashed";
const std::string MEDIA_DATA_DB_DATE_TRASHED_S = "date_trashed_s";
const std::string MEDIA_DATA_DB_DATE_TRASHED_MS = "date_trashed_ms";
const std::string MEDIA_DATA_DB_DATE_TRASHED_TO_SECOND = "CAST(date_trashed / 1000 AS BIGINT) AS date_trashed_s";
const std::string MEDIA_DATA_DB_IS_PENDING = "is_pending";
const std::string MEDIA_DATA_DB_TIME_PENDING = "time_pending";
const std::string MEDIA_DATA_DB_RELATIVE_PATH = "relative_path";
const std::string MEDIA_DATA_DB_VOLUME_NAME = "volume_name";
const std::string MEDIA_DATA_DB_SELF_ID = "self_id";
const std::string MEDIA_DATA_DB_DEVICE_NAME = "device_name";

const std::string MEDIA_DATA_DB_ALBUM = "album";
const std::string MEDIA_DATA_DB_ALBUM_ID = "album_id";
const std::string MEDIA_DATA_DB_REFERENCE_ALBUM_ID = "reference_album_id";
const std::string MEDIA_DATA_DB_ALBUM_NAME = "album_name";
const std::string MEDIA_DATA_DB_COUNT = "count";
const std::string MEDIA_DATA_BUNDLENAME = "bundle_name";

// ringtone uri constants
const std::string MEDIA_DATA_DB_RINGTONE_URI = "ringtone_uri";
const std::string MEDIA_DATA_DB_ALARM_URI = "alarm_uri";
const std::string MEDIA_DATA_DB_NOTIFICATION_URI = "notification_uri";
const std::string MEDIA_DATA_DB_RINGTONE_TYPE = "ringtone_type";

const std::string MEDIA_DATA_DB_PHOTO_ID = "photo_id";
const std::string MEDIA_DATA_DB_PHOTO_QUALITY = "photo_quality";
const std::string MEDIA_DATA_DB_FIRST_VISIT_TIME = "first_visit_time";
const std::string MEDIA_DATA_DB_DEFERRED_PROC_TYPE = "deferred_proc_type";

const std::string MEDIA_COLUMN_COUNT = "count(*)";
const std::string MEDIA_COLUMN_COUNT_1 = "count(1)";
const std::string MEDIA_COLUMN_COUNT_DISTINCT_FILE_ID = "count(distinct file_id)";

const std::string PHOTO_INDEX = "photo_index";

const std::string PERMISSION_ID = "id";
const std::string PERMISSION_BUNDLE_NAME = "bundle_name";
const std::string PERMISSION_FILE_ID = "file_id";
const std::string PERMISSION_MODE = "mode";
const std::string PERMISSION_TABLE_TYPE = "table_type";

const std::string FILE_TABLE = "file";
const std::string ALBUM_TABLE = "album";
const std::string ALBUM_VIEW_NAME = "Album";

const std::string SMARTALBUMMAP_TABLE_NAME = "smartAlbumMap";
const std::string SMARTALBUMASSETS_VIEW_NAME = "SmartAsset";
const std::string SMARTALBUMASSETS_ALBUMCAPACITY = "size";
const std::string SMARTABLUMASSETS_PARENTID = "parentid";

const std::string ASSETMAP_VIEW_NAME = "AssetMap";

const std::string IMAGE_ASSET_TYPE = "image";
const std::string VIDEO_ASSET_TYPE = "video";
const std::string AUDIO_ASSET_TYPE = "audio";

// Caution: Keep same definition as MediaColumn! Only for where clause check in API9 getAlbums and album.getFileAssets
const std::string COMPAT_ALBUM_SUBTYPE = "album_subtype";
const std::string COMPAT_HIDDEN = "hidden";
const std::string COMPAT_PHOTO_SYNC_STATUS = "sync_status";
const std::string COMPAT_FILE_SUBTYPE = "subtype";
const std::string COMPAT_CAMERA_SHOT_KEY = "camera_shot_key";
const std::string PHOTO_DATE_YEAR = "date_year";
const std::string PHOTO_DATE_MONTH = "date_month";
const std::string PHOTO_DATE_DAY = "date_day";

// system relativePath and albumName, use for API9 compatible API10
const std::string CAMERA_PATH = "Camera/";
const std::string SCREEN_SHOT_PATH = "Pictures/Screenshots/";
const std::string SCREEN_RECORD_PATH = "Videos/ScreenRecordings/";

const std::string CAMERA_ALBUM_NAME = "Camera";
const std::string SCREEN_SHOT_ALBUM_NAME = "Screenshots";
const std::string SCREEN_RECORD_ALBUM_NAME = "ScreenRecordings";

// extension
const std::string ASSET_EXTENTION = "extention";

// edit param
const std::string EDIT_DATA_REQUEST = "edit_data_request";  // MEDIA_OPERN_KEYWORD=EDIT_DATA_REQUEST
const std::string SOURCE_REQUEST = "source_request";        // MEDIA_OPERN_KEYWORD=SOURCE_REQUEST
const std::string COMMIT_REQUEST = "commit_request";        // MEDIA_OPERN_KEYWORD=COMMIT_REQUEST
const std::string EDIT_DATA = "edit_data";
const std::string COMPATIBLE_FORMAT = "compatible_format";
const std::string FORMAT_VERSION = "format_version";
const std::string APP_ID = "app_id";

// write cache
const std::string CACHE_FILE_NAME = "cache_file_name";
} // namespace Media
} // namespace OHOS

#endif  // INTERFACES_INNERKITS_NATIVE_INCLUDE_MEDIA_DATA_ABILITY_CONST_H_
