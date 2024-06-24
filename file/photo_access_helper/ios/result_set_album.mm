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

#include "result_set_album.h"

#include <securec.h>
#include <sstream>
#include <map>

#include "plugins/file/photo_access_helper/napi/album/include/photo_album_column.h"

namespace OHOS {
namespace Media {
namespace {
static const int INVALID_POS = -1;
static const int SUCCESS_CODE = 0;
static std::map<std::string, std::string> ALBUM_COLUMN_MAP = {
    { PhotoAlbumColumns::ALBUM_TYPE, "assetCollectionType" },
    { PhotoAlbumColumns::ALBUM_SUBTYPE , "assetCollectionSubtype"},
    { PhotoAlbumColumns::ALBUM_NAME , "localizedTitle"},
    { PhotoAlbumColumns::ALBUM_ID, "hash" },
    { PhotoAlbumColumns::ALBUM_COUNT, "count" },
    { PhotoAlbumColumns::ALBUM_IMAGE_COUNT, "photosCount" },
    { PhotoAlbumColumns::ALBUM_VIDEO_COUNT, "videosCount" },
    { PhotoAlbumColumns::ALBUM_LOCAL_IDENTIFIER, "localIdentifier" },
};
}

ResultSetAlbum::ResultSetAlbum(PHFetchResult *result)
{
    photoCursorAlbum_ = [[photoCursorAlbum alloc]initWithFetchResult:result];
}

ResultSetAlbum::~ResultSetAlbum()
{
    Close();
}

int ResultSetAlbum::GetAllColumnNames(std::vector<std::string> &columnNames)
{
    for (const auto& pair : ALBUM_COLUMN_MAP) {
        columnNames.push_back(std::string(pair.first));
    }
    return SUCCESS_CODE;
}

int ResultSetAlbum::GetRowCount(int &count)
{
    if (photoCursorAlbum_) {
        count = [photoCursorAlbum_ getRowCount];
    }
    return SUCCESS_CODE;
}

int ResultSetAlbum::GoToRow(int position)
{
    if (photoCursorAlbum_) {
        return [photoCursorAlbum_ goToRow:position];
    }
    return INVALID_POS;
}

int ResultSetAlbum::GetBlob(int columnIndex, std::vector<uint8_t> &value)
{
    return SUCCESS_CODE;
}

int ResultSetAlbum::GetString(int columnIndex, std::string &value)
{
    if (columnIndex >= ALBUM_COLUMN_MAP.size()) {
        NSLog(@"ResultSetAlbum GetString columnIndex is invalid");
        return INVALID_POS;
    }
    auto it = std::next(ALBUM_COLUMN_MAP.begin(), columnIndex);
    std::string field = it->second;
    if (photoCursorAlbum_) {
        NSString *fieldIos = [NSString stringWithCString:field.c_str() encoding:[NSString defaultCStringEncoding]];
        NSString *result = [photoCursorAlbum_ getString:fieldIos];
        value = std::string([result UTF8String]);
        return SUCCESS_CODE;  
    }
    return INVALID_POS;
}

int ResultSetAlbum::GetInt(int columnIndex, int &value)
{
    if (columnIndex >= ALBUM_COLUMN_MAP.size()) {
        NSLog(@"ResultSetAlbum GetInt columnIndex is invalid");
        return INVALID_POS;
    }
    auto it = std::next(ALBUM_COLUMN_MAP.begin(), columnIndex);
    std::string field = it->second;
    if (photoCursorAlbum_) {
        NSString *fieldIos = [NSString stringWithCString:field.c_str() encoding:[NSString defaultCStringEncoding]];
        value = [photoCursorAlbum_ getInt:fieldIos];
        return SUCCESS_CODE;  
    }
    return INVALID_POS;
}

int ResultSetAlbum::GetLong(int columnIndex, int64_t &value)
{
    if (columnIndex >= ALBUM_COLUMN_MAP.size()) {
        NSLog(@"ResultSetAlbum GetLong columnIndex is invalid");
        return INVALID_POS;
    }
    auto it = std::next(ALBUM_COLUMN_MAP.begin(), columnIndex);
    std::string field = it->second;
    if (photoCursorAlbum_) {
        NSString *fieldIos = [NSString stringWithCString:field.c_str() encoding:[NSString defaultCStringEncoding]];
        value = [photoCursorAlbum_ getLong:fieldIos];
        return SUCCESS_CODE;  
    }
    return INVALID_POS;
}

int ResultSetAlbum::GetDouble(int columnIndex, double &value)
{
    return SUCCESS_CODE;
}

int ResultSetAlbum::IsColumnNull(int columnIndex, bool &isNull)
{
    return SUCCESS_CODE;
}

int ResultSetAlbum::Close()
{
    isClose_ = true;
    return SUCCESS_CODE;  
}

int ResultSetAlbum::GoTo(int offset)
{
    return SUCCESS_CODE;
}

int ResultSetAlbum::GoToFirstRow()
{
    if (photoCursorAlbum_) {
        return [photoCursorAlbum_ goToFirstRow];
    }
    return INVALID_POS;
}

int ResultSetAlbum::GoToLastRow()
{
    if (photoCursorAlbum_) {
        return [photoCursorAlbum_ goToLastRow];
    }
    return INVALID_POS;
}

int ResultSetAlbum::GoToNextRow()
{
    if (photoCursorAlbum_) {
        return [photoCursorAlbum_ goToNextRow];
    }
    return INVALID_POS;
}

int ResultSetAlbum::GoToPreviousRow()
{
    if (photoCursorAlbum_) {
        return [photoCursorAlbum_ goToPreviousRow];
    }
    return INVALID_POS;
}

int ResultSetAlbum::IsAtLastRow(bool &result)
{
    if (photoCursorAlbum_) {
        result = [photoCursorAlbum_ isAtLastRow];
        return SUCCESS_CODE;
    }
    return INVALID_POS;
}

int ResultSetAlbum::GetColumnCount(int &count)
{
    count = ALBUM_COLUMN_MAP.size();
    return SUCCESS_CODE;
}

int ResultSetAlbum::GetColumnIndex(const std::string &columnName, int &columnIndex)
{
    int index = INVALID_POS;
    std::map<std::string, std::string>::iterator iter;
    for (iter = ALBUM_COLUMN_MAP.begin(); iter != ALBUM_COLUMN_MAP.end(); ++iter) {
        index++;
        if (iter->first == columnName) {
            break;
        }
    }
    columnIndex = index;
    return index == INVALID_POS ? INVALID_POS : SUCCESS_CODE;
}

int ResultSetAlbum::GetColumnName(int columnIndex, std::string &columnName)
{
    if (columnIndex >= ALBUM_COLUMN_MAP.size()) {
        NSLog(@"ResultSetAlbum GetColumnName columnIndex is invalid");
        return INVALID_POS;
    }
    auto it = std::next(ALBUM_COLUMN_MAP.begin(), columnIndex);
    columnName = it->first;
    return SUCCESS_CODE;
}

bool ResultSetAlbum::IsClosed() const
{
    return isClose_;
}
} // namespace Media
} // namespace OHOS