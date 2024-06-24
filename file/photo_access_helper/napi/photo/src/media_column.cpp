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

#include "../include/media_column.h"

#include <string>
#include <vector>

#include "userfile_manager_types.h"

namespace OHOS {
namespace Media {
const std::string MediaColumn::MEDIA_ID = "file_id";
const std::string MediaColumn::MEDIA_FILE_PATH = "data";
const std::string MediaColumn::MEDIA_SIZE = "size";
const std::string MediaColumn::MEDIA_TITLE = "title";
const std::string MediaColumn::MEDIA_NAME = "display_name";
const std::string MediaColumn::MEDIA_TYPE = "media_type";
const std::string MediaColumn::MEDIA_MIME_TYPE = "mime_type";
const std::string MediaColumn::MEDIA_OWNER_PACKAGE = "owner_package";
const std::string MediaColumn::MEDIA_PACKAGE_NAME = "package_name";
const std::string MediaColumn::MEDIA_DEVICE_NAME = "device_name";
const std::string MediaColumn::MEDIA_DATE_MODIFIED = "date_modified";
const std::string MediaColumn::MEDIA_DATE_ADDED = "date_added";
const std::string MediaColumn::MEDIA_DATE_TAKEN = "date_taken";
const std::string MediaColumn::MEDIA_DURATION = "duration";
const std::string MediaColumn::MEDIA_TIME_PENDING = "time_pending";
const std::string MediaColumn::MEDIA_IS_FAV = "is_favorite";
const std::string MediaColumn::MEDIA_DATE_TRASHED = "date_trashed";
const std::string MediaColumn::MEDIA_DATE_DELETED = "date_deleted";
const std::string MediaColumn::MEDIA_HIDDEN = "hidden";
const std::string MediaColumn::MEDIA_PARENT_ID = "parent";
const std::string MediaColumn::MEDIA_RELATIVE_PATH = "relative_path";
const std::string MediaColumn::MEDIA_VIRTURL_PATH = "virtual_path";
const std::set<std::string> MediaColumn::MEDIA_COLUMNS = {
    MEDIA_ID, MEDIA_FILE_PATH, MEDIA_SIZE, MEDIA_TITLE, MEDIA_NAME, MEDIA_TYPE, MEDIA_MIME_TYPE,
    MEDIA_OWNER_PACKAGE, MEDIA_PACKAGE_NAME, MEDIA_DEVICE_NAME, MEDIA_DATE_MODIFIED, MEDIA_DATE_ADDED,
    MEDIA_DATE_TAKEN, MEDIA_DURATION, MEDIA_TIME_PENDING, MEDIA_IS_FAV, MEDIA_DATE_TRASHED,
    MEDIA_DATE_DELETED, MEDIA_HIDDEN, MEDIA_PARENT_ID, MEDIA_RELATIVE_PATH, MEDIA_VIRTURL_PATH
};
const std::set<std::string> MediaColumn::DEFAULT_FETCH_COLUMNS = {
    MEDIA_ID, MEDIA_FILE_PATH, MEDIA_NAME, MEDIA_TYPE
};

const std::string PhotoColumn::PHOTO_DIRTY = "dirty";
const std::string PhotoColumn::PHOTO_CLOUD_ID = "cloud_id";
const std::string PhotoColumn::PHOTO_META_DATE_MODIFIED = "meta_date_modified";
const std::string PhotoColumn::PHOTO_SYNC_STATUS = "sync_status";
const std::string PhotoColumn::PHOTO_CLOUD_VERSION = "cloud_version";
const std::string PhotoColumn::PHOTO_ORIENTATION = "orientation";
const std::string PhotoColumn::PHOTO_LATITUDE = "latitude";
const std::string PhotoColumn::PHOTO_LONGITUDE = "longitude";
const std::string PhotoColumn::PHOTO_HEIGHT = "height";
const std::string PhotoColumn::PHOTO_WIDTH = "width";
const std::string PhotoColumn::PHOTO_LCD_VISIT_TIME = "lcd_visit_time";
const std::string PhotoColumn::PHOTO_EDIT_TIME = "edit_time";
const std::string PhotoColumn::PHOTO_POSITION = "position";
const std::string PhotoColumn::PHOTO_SUBTYPE = "subtype";
const std::string PhotoColumn::CAMERA_SHOT_KEY = "camera_shot_key";
const std::string PhotoColumn::PHOTO_USER_COMMENT = "user_comment";
const std::string PhotoColumn::PHOTO_SHOOTING_MODE = "shooting_mode";
const std::string PhotoColumn::PHOTO_SHOOTING_MODE_TAG = "shooting_mode_tag";
const std::string PhotoColumn::PHOTO_ALL_EXIF = "all_exif";
const std::string PhotoColumn::PHOTO_DATE_YEAR = "date_year";
const std::string PhotoColumn::PHOTO_DATE_MONTH = "date_month";
const std::string PhotoColumn::PHOTO_DATE_DAY = "date_day";
const std::string PhotoColumn::PHOTO_LAST_VISIT_TIME = "last_visit_time";
const std::string PhotoColumn::PHOTO_HIDDEN_TIME = "hidden_time";
const std::string PhotoColumn::PHOTO_THUMB_STATUS = "thumb_status";
const std::string PhotoColumn::PHOTO_CLEAN_FLAG = "clean_flag";
const std::string PhotoColumn::PHOTO_ID = "photo_id";
const std::string PhotoColumn::PHOTO_QUALITY = "photo_quality";
const std::string PhotoColumn::PHOTO_FIRST_VISIT_TIME = "first_visit_time";
const std::string PhotoColumn::PHOTO_DEFERRED_PROC_TYPE = "deferred_proc_type";
const std::string PhotoColumn::PHOTO_HAS_ASTC = "has_astc";

const std::string PhotoColumn::PHOTO_CLOUD_ID_INDEX = "cloud_id_index";
const std::string PhotoColumn::PHOTO_DATE_YEAR_INDEX = "date_year_index";
const std::string PhotoColumn::PHOTO_DATE_MONTH_INDEX = "date_month_index";
const std::string PhotoColumn::PHOTO_DATE_DAY_INDEX = "date_day_index";
const std::string PhotoColumn::PHOTO_SCHPT_ADDED_INDEX = "idx_schpt_date_added";
const std::string PhotoColumn::PHOTO_SCHPT_MEDIA_TYPE_INDEX = "idx_schpt_media_type";
const std::string PhotoColumn::PHOTO_SCHPT_DAY_INDEX = "idx_schpt_date_day";
const std::string PhotoColumn::PHOTO_HIDDEN_TIME_INDEX = "hidden_time_index";
const std::string PhotoColumn::PHOTO_SCHPT_HIDDEN_TIME_INDEX = "idx_schpt_hidden_time";
const std::string PhotoColumn::PHOTO_FAVORITE_INDEX = "idx_photo_is_favorite";

const std::string PhotoColumn::PHOTO_DATE_YEAR_FORMAT = "%Y";
const std::string PhotoColumn::PHOTO_DATE_MONTH_FORMAT = "%Y%m";
const std::string PhotoColumn::PHOTO_DATE_DAY_FORMAT = "%Y%m%d";

const std::string PhotoColumn::PHOTOS_TABLE = "Photos";

const std::string PhotoColumn::PHOTO_URI_PREFIX = "file://media/Photo/";
const std::string PhotoColumn::DEFAULT_PHOTO_URI = "file://media/Photo";
const std::string PhotoColumn::PHOTO_CACHE_URI_PREFIX = "file://media/Photo/cache/";
const std::string PhotoColumn::PHOTO_TYPE_URI = "/Photo";

const std::set<std::string> PhotoColumn::PHOTO_COLUMNS = {
    PhotoColumn::PHOTO_ORIENTATION, PhotoColumn::PHOTO_LATITUDE, PhotoColumn::PHOTO_LONGITUDE,
    PhotoColumn::PHOTO_HEIGHT, PhotoColumn::PHOTO_WIDTH, PhotoColumn::PHOTO_LCD_VISIT_TIME, PhotoColumn::PHOTO_POSITION,
    PhotoColumn::PHOTO_DIRTY, PhotoColumn::PHOTO_CLOUD_ID, PhotoColumn::CAMERA_SHOT_KEY, PhotoColumn::PHOTO_ALL_EXIF,
    PhotoColumn::PHOTO_USER_COMMENT, PhotoColumn::PHOTO_DATE_YEAR, PhotoColumn::PHOTO_DATE_MONTH,
    PhotoColumn::PHOTO_DATE_DAY, PhotoColumn::PHOTO_EDIT_TIME, PhotoColumn::PHOTO_CLEAN_FLAG,
    PhotoColumn::PHOTO_SHOOTING_MODE, PhotoColumn::PHOTO_SHOOTING_MODE_TAG, PhotoColumn::PHOTO_THUMB_STATUS
};

bool PhotoColumn::IsPhotoColumn(const std::string &columnName)
{
    if (columnName == "count(*)") {
        return true;
    }
    return (PHOTO_COLUMNS.find(columnName) != PHOTO_COLUMNS.end()) ||
        (MEDIA_COLUMNS.find(columnName) != MEDIA_COLUMNS.end());
}
}  // namespace Media
}  // namespace OHOS
