/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <cctype>

#include "abs_predicates.h"
#include "log.h"
#include "rdb_utils_result.h"
#include "plugins/file/photo_access_helper/napi/photo/include/media_column.h"
#include "plugins/file/photo_access_helper/napi/album/include/photo_album_column.h"

namespace OHOS {
namespace Media {

static const std::map<std::string, std::string> PHOTO_COLUMN_MAP = {
    { PhotoColumn::MEDIA_TYPE, "mediaType" },
    { PhotoColumn::MEDIA_DURATION, "duration" },
    { PhotoColumn::PHOTO_HEIGHT, "pixelHeight" },
    { PhotoColumn::PHOTO_WIDTH, "pixelWidth" },
    { PhotoColumn::MEDIA_IS_FAV, "favorite" },
    { PhotoColumn::MEDIA_HIDDEN, "hidden"},
};

static const std::map<std::string, std::string> PHOTO_SORT_COLUMN_MAP = {
    { PhotoColumn::MEDIA_TYPE, "mediaType" },
    { PhotoColumn::MEDIA_DATE_ADDED, "creationDate" },
    { PhotoColumn::MEDIA_DATE_MODIFIED, "modificationDate" },
    { PhotoColumn::MEDIA_DURATION, "duration" },
    { PhotoColumn::PHOTO_HEIGHT, "pixelHeight" },
    { PhotoColumn::PHOTO_WIDTH, "pixelWidth" },
    { PhotoColumn::MEDIA_DATE_TAKEN, "creationDate" },
    { PhotoColumn::MEDIA_IS_FAV, "favorite" },
    { PhotoColumn::MEDIA_HIDDEN, "hidden"},
};

static const std::map<std::string, int> ALBUM_TYPE_MAP = {
    { "0", 1 },
    { "1024", 2},
};

static const std::map<std::string, int> ALBUM_SUBTYPE_MAP = {
    { "1",  2},
    { "1025", 203},
    { "1026", 202},
};

static const std::map<std::string, std::string> ALBUM_SORT_COLUMN_MAP = {
    { PhotoAlbumColumns::ALBUM_NAME, "localizedTitle" },
};

static int ChangeAlbumFieldToIosField(const std::string &value, const std::map<std::string, int> &columnMap) {
    auto it = columnMap.find(value);
    if (it != columnMap.end()) {
        return it->second;
    }
    return 0;
}

static std::string ChangePhotoFieldToIosField(const std::string &field, const bool isAlbum) {
    if (isAlbum) {
        return "";
    }
    auto it = PHOTO_COLUMN_MAP.find(field);
    if (it != PHOTO_COLUMN_MAP.end()) {
        return it->second;
    }
    return "";
}

static std::string GetPhotoSortFieldToIosField(const std::string &field, const std::map<std::string, std::string> &columnMap) {
    auto it = columnMap.find(field);
    if (it != columnMap.end()) {
        return it->second;
    }
    return "";
}

static std::string ChangePhotoSortFieldToIosField(const std::string &field, const bool isAlbum) {
    if (isAlbum) {
        return GetPhotoSortFieldToIosField(field, ALBUM_SORT_COLUMN_MAP);
    }
    return GetPhotoSortFieldToIosField(field, PHOTO_SORT_COLUMN_MAP);
}

static std::string ChangeDuring(const std::string &duringMs) {
    double value = 0;
    value = std::stod(duringMs) / 1000.0;
    return std::to_string(value);
}

static std::string SurroundWithFunction(const std::string &function, const std::string &separator,
    const std::vector<std::string> &array, bool isDuration)
{
    std::string builder(function);
    builder += "{";
    bool isFirst = true;
    for (const auto &text : array) {
        if (!isFirst) {
            builder = builder + " " + separator + " ";
        } else {
            isFirst = false;
        }

        isDuration ? builder += ChangeDuring(text) : builder += text;
    }
    builder += "}";
    return builder;
}

static bool checkStrEmpty(const std::string &str)
{
    return str.empty() || std::all_of(str.begin(), str.end(), ::isspace);
}

RdbUtilsResult::RdbUtilsResult(const bool isAlbum)
{
    isAlbum_ = isAlbum;
    Initial();
}

RdbUtilsResult::~RdbUtilsResult()
{
}

/**
 * Restricts the value of the field to be greater than the specified value.
 */
bool RdbUtilsResult::EqualTo(const std::string &field, const ValueObject &value)
{
    if (!CheckParameter("equalTo", field)) {
        return false;
    }

    ValueObject valObj = value;
    std::string str;
    valObj.GetString(str);
    if (checkStrEmpty(str)) {
        LOGW("RdbUtilsResult::EqualTo str is empty");
        return true;
    }
    std::string fieldLower = field;
    transform(fieldLower.begin(), fieldLower.end(), fieldLower.begin(), ::tolower);
    if (fieldLower == PhotoAlbumColumns::ALBUM_TYPE) {
        albumType = ChangeAlbumFieldToIosField(str, ALBUM_TYPE_MAP);
        return false;
    }
    if (fieldLower == PhotoAlbumColumns::ALBUM_SUBTYPE) {
        albumSubtype = ChangeAlbumFieldToIosField(str, ALBUM_SUBTYPE_MAP);
        return false;
    }
    if (fieldLower == PhotoAlbumColumns::ALBUM_LOCAL_IDENTIFIER) {
        localIdentifier = str;
        return false;
    }

    if (isAlbum_) {
        return true;
    }

    std::string newField = ChangePhotoFieldToIosField(fieldLower, isAlbum_);
    if (newField.empty()) {
        LOGW("Invalid field : %{public}s", field.c_str());
        return true;
    }
    CheckIsNeedAnd();

    if (newField == "duration") {
        whereClause += newField + " == " + ChangeDuring(str);
    } else {
        whereClause += newField + " == " + str;
    }
    
    return false;
}

bool RdbUtilsResult::And()
{
    return false;
}

/**
 * Restricts the ascending order of the return list. When there are several orders,
 * the one close to the head has the highest priority.
 */
bool RdbUtilsResult::OrderByAsc(const std::string &field)
{
    if (!CheckParameter("orderByAsc", field)) {
        return false;
    }
    std::string fieldLower = field;
    transform(fieldLower.begin(), fieldLower.end(), fieldLower.begin(), ::tolower);
    std::string newField = ChangePhotoSortFieldToIosField(fieldLower, isAlbum_);
    if (newField.empty()) {
        LOGW("Invalid field : %{public}s", field.c_str());
        return true;
    }
    order.insert(std::make_pair(newField, true));
    return false;
}

/**
 * Restricts the descending order of the return list. When there are several orders,
 * the one close to the head has the highest priority.
 */
bool RdbUtilsResult::OrderByDesc(const std::string &field)
{
    if (!CheckParameter("orderByDesc", field)) {
        return false;
    }
    std::string fieldLower = field;
    transform(fieldLower.begin(), fieldLower.end(), fieldLower.begin(), ::tolower);
    std::string newField = ChangePhotoSortFieldToIosField(fieldLower, isAlbum_);
    if (newField.empty()) {
        LOGW("Invalid field : %{public}s", field.c_str());
        return true;
    }
    order.insert(std::make_pair(newField, false));
    return false;
}

/**
 * Restricts the max number of return records.
 */
bool RdbUtilsResult::Limit(const int limit)
{
    this->limit = (limit <= 0) ? INT_MAX : limit;
    return false;
}

/**
 * Configures to specify the start position of the returned result.
 */
bool RdbUtilsResult::Offset(const int offset)
{
    this->offset = (offset <= 0) ? 0 : offset;
    return false;
}

/**
 * Configures to match the specified field whose data type is String array and values are within a given range.
 */
bool RdbUtilsResult::In(const std::string &field, const std::vector<std::string> &values)
{
    bool chekParaFlag = CheckParameter("in", field);
    if (!chekParaFlag) {
        return false;
    }

    std::vector<std::string> valuesStr;
    for (const auto &text : values) {
        if (checkStrEmpty(text)) {
            continue;
        }
        valuesStr.push_back(text);
    }
    if (valuesStr.empty()) {
        LOGW("in() fails because values can't be null.");
        return true;
    }
    std::string fieldLower = field;
    transform(fieldLower.begin(), fieldLower.end(), fieldLower.begin(), ::tolower);
    std::string newField = ChangePhotoFieldToIosField(fieldLower, isAlbum_);
    if (newField.empty()) {
        LOGW("Invalid field : %{public}s", field.c_str());
        return true;
    }

    CheckIsNeedAnd();
    AppendWhereClauseWithInOrNotIn(" IN ", newField, valuesStr);
    return false;
}

void RdbUtilsResult::Initial()
{
    isNeedAnd = false;
    whereClause.clear();
    order.clear();
    limit = INT_MAX;
    offset = 0;
    albumType = 0;
    albumSubtype = LONG_MAX;
}

/**
 * Check the parameter validity.
 */
bool RdbUtilsResult::CheckParameter(
    const std::string &methodName, const std::string &field) const
{
    if (field.empty()) {
        LOGW("%{public}s: string 'field' is empty.", methodName.c_str());
        return false;
    }
    return true;
}

void RdbUtilsResult::CheckIsNeedAnd()
{
    if (isNeedAnd) {
        whereClause += " AND ";
    } else {
        isNeedAnd = true;
    }
}

void RdbUtilsResult::AppendWhereClauseWithInOrNotIn(
    const std::string &methodName, const std::string &field, const std::vector<std::string> &replaceValues)
{
    if (field == "duration") {
        whereClause += field + SurroundWithFunction(methodName, ",", replaceValues, true);
    } else {
        whereClause += field + SurroundWithFunction(methodName, ",", replaceValues, false);
    }
}

std::string RdbUtilsResult::GetWhereClause() const
{
    return whereClause;
}

int RdbUtilsResult::GetLimit() const
{
    return limit;
}

int RdbUtilsResult::GetOffset() const
{
    return offset;
}

int RdbUtilsResult::GetAlbumType() const
{
    return albumType;
}

long long RdbUtilsResult::GetAlbumSubType() const
{
    return albumSubtype;
}

std::map<std::string, int> RdbUtilsResult::GetOrder() const
{
    return order;
}

std::string RdbUtilsResult::GetLocalIdentifier() const
{
    return localIdentifier;
}
} // namespace Media
} // namespace OHOS