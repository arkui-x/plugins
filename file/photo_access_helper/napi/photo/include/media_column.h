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

#ifndef INTERFACES_INNERKITS_NATIVE_INCLUDE_MEDIA_COLUMN_H_
#define INTERFACES_INNERKITS_NATIVE_INCLUDE_MEDIA_COLUMN_H_

#include <set>
#include <string>

namespace OHOS::Media {
#define EXPORT __attribute__ ((visibility ("default")))

class MediaColumn {
public:
    // Asset Base Parameter
    static const std::string MEDIA_ID EXPORT;
    static const std::string MEDIA_FILE_PATH EXPORT;
    static const std::string MEDIA_SIZE EXPORT;
    static const std::string MEDIA_TITLE EXPORT;
    static const std::string MEDIA_NAME EXPORT;
    static const std::string MEDIA_TYPE EXPORT;
    static const std::string MEDIA_MIME_TYPE EXPORT;
    static const std::string MEDIA_OWNER_PACKAGE EXPORT;
    static const std::string MEDIA_PACKAGE_NAME EXPORT;
    static const std::string MEDIA_DEVICE_NAME EXPORT;

    // As set Parameter about time
    static const std::string MEDIA_DATE_MODIFIED EXPORT;
    static const std::string MEDIA_DATE_ADDED EXPORT;
    static const std::string MEDIA_DATE_TAKEN EXPORT;
    static const std::string MEDIA_DURATION EXPORT;
    static const std::string MEDIA_TIME_PENDING EXPORT;
    static const std::string MEDIA_IS_FAV EXPORT;
    static const std::string MEDIA_DATE_TRASHED EXPORT;
    static const std::string MEDIA_DATE_DELETED EXPORT;
    static const std::string MEDIA_HIDDEN EXPORT;

    // Asset Parameter deperated
    static const std::string MEDIA_PARENT_ID EXPORT;
    static const std::string MEDIA_RELATIVE_PATH EXPORT;
    static const std::string MEDIA_VIRTURL_PATH EXPORT;

    // All Columns
    static const std::set<std::string> MEDIA_COLUMNS EXPORT;
    // Default fetch columns
    static const std::set<std::string> DEFAULT_FETCH_COLUMNS EXPORT;
};

class PhotoColumn : public MediaColumn {
public:
    // column only in PhotoTable
    static const std::string PHOTO_ORIENTATION EXPORT;
    static const std::string PHOTO_LATITUDE EXPORT;
    static const std::string PHOTO_LONGITUDE EXPORT;
    static const std::string PHOTO_HEIGHT EXPORT;
    static const std::string PHOTO_WIDTH EXPORT;
    static const std::string PHOTO_LCD_VISIT_TIME EXPORT;
    static const std::string PHOTO_EDIT_TIME EXPORT;
    static const std::string PHOTO_POSITION EXPORT;
    static const std::string PHOTO_DIRTY EXPORT;
    static const std::string PHOTO_CLOUD_ID EXPORT;
    static const std::string PHOTO_SUBTYPE EXPORT;
    static const std::string PHOTO_META_DATE_MODIFIED EXPORT;
    static const std::string PHOTO_SYNC_STATUS EXPORT;
    static const std::string PHOTO_CLOUD_VERSION EXPORT;
    static const std::string CAMERA_SHOT_KEY EXPORT;
    static const std::string PHOTO_USER_COMMENT EXPORT;
    static const std::string PHOTO_ALL_EXIF EXPORT;
    static const std::string PHOTO_CLEAN_FLAG EXPORT;
    static const std::string PHOTO_HAS_ASTC EXPORT;

    static const std::string PHOTO_SYNCING EXPORT;
    static const std::string PHOTO_DATE_YEAR EXPORT;
    static const std::string PHOTO_DATE_MONTH EXPORT;
    static const std::string PHOTO_DATE_DAY EXPORT;
    static const std::string PHOTO_SHOOTING_MODE EXPORT;
    static const std::string PHOTO_SHOOTING_MODE_TAG EXPORT;
    static const std::string PHOTO_LAST_VISIT_TIME EXPORT;
    static const std::string PHOTO_HIDDEN_TIME EXPORT;
    static const std::string PHOTO_THUMB_STATUS EXPORT;
    static const std::string PHOTO_ID EXPORT;
    static const std::string PHOTO_QUALITY EXPORT;
    static const std::string PHOTO_FIRST_VISIT_TIME EXPORT;
    static const std::string PHOTO_DEFERRED_PROC_TYPE EXPORT;

    // index in PhotoTable
    static const std::string PHOTO_CLOUD_ID_INDEX EXPORT;
    static const std::string PHOTO_DATE_YEAR_INDEX EXPORT;
    static const std::string PHOTO_DATE_MONTH_INDEX EXPORT;
    static const std::string PHOTO_DATE_DAY_INDEX EXPORT;
    static const std::string PHOTO_SCHPT_ADDED_INDEX EXPORT;
    static const std::string PHOTO_SCHPT_MEDIA_TYPE_INDEX EXPORT;
    static const std::string PHOTO_SCHPT_DAY_INDEX EXPORT;
    static const std::string PHOTO_HIDDEN_TIME_INDEX EXPORT;
    static const std::string PHOTO_SCHPT_HIDDEN_TIME_INDEX EXPORT;
    static const std::string PHOTO_FAVORITE_INDEX EXPORT;
    // format in PhotoTable year month day
    static const std::string PHOTO_DATE_YEAR_FORMAT EXPORT;
    static const std::string PHOTO_DATE_MONTH_FORMAT EXPORT;
    static const std::string PHOTO_DATE_DAY_FORMAT EXPORT;
    // table name
    static const std::string PHOTOS_TABLE EXPORT;

    // photo uri
    static const std::string PHOTO_URI_PREFIX EXPORT;
    static const std::string PHOTO_TYPE_URI EXPORT;
    static const std::string DEFAULT_PHOTO_URI EXPORT;
    static const std::string PHOTO_CACHE_URI_PREFIX EXPORT;

    // all columns
    static const std::set<std::string> PHOTO_COLUMNS EXPORT;

    EXPORT static bool IsPhotoColumn(const std::string &columnName);
};
} // namespace OHOS::Media
#endif // INTERFACES_INNERKITS_NATIVE_INCLUDE_MEDIA_COLUMN_H_
