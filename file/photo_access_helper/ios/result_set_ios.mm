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
#include "result_set_ios.h"

#include <securec.h>
#include <sstream>
#include <map>

#include "plugins/file/photo_access_helper/napi/photo/include/media_column.h"

namespace OHOS {
namespace Media {
namespace {
static const int INVALID_POS = -1;
static const int SUCCESS_CODE = 0;
static std::map<std::string, std::string> PHOTO_COLUMN_MAP = {
    { PhotoColumn::MEDIA_TYPE, "mediaType" },
    { PhotoColumn::MEDIA_NAME , "filename"},
    { PhotoColumn::MEDIA_SIZE , "size"},
    { PhotoColumn::MEDIA_FILE_PATH, "uri" },
    { PhotoColumn::MEDIA_DATE_MODIFIED, "modificationDate" },
    { PhotoColumn::MEDIA_DURATION, "duration" },
    { PhotoColumn::PHOTO_HEIGHT, "pixelHeight" },
    { PhotoColumn::PHOTO_WIDTH, "pixelWidth" },
    { PhotoColumn::MEDIA_DATE_TAKEN, "creationDate" },
    { PhotoColumn::PHOTO_ORIENTATION, "orientation" },
    { PhotoColumn::MEDIA_IS_FAV, "favorite" },
    { PhotoColumn::MEDIA_TITLE, "title" },
    { PhotoColumn::MEDIA_HIDDEN, "hidden"},
    { PhotoColumn::MEDIA_DATE_ADDED, "creationDate"},
};
}

ResultSetIos::ResultSetIos(PHFetchResult *result)
{
    photoCursorIos_ = [[photoCursorIos alloc]initWithFetchResult:result];
}

ResultSetIos::~ResultSetIos()
{
    Close();
}

int ResultSetIos::GetAllColumnNames(std::vector<std::string> &columnNames)
{
    for (const auto& pair : PHOTO_COLUMN_MAP) {
        columnNames.push_back(std::string(pair.first));
    }
    return SUCCESS_CODE;
}

int ResultSetIos::GetRowCount(int &count)
{
    if (photoCursorIos_) {
        count = [photoCursorIos_ getRowCount];
    }
    return SUCCESS_CODE;
}

int ResultSetIos::GoToRow(int position)
{
    if (photoCursorIos_) {
        return [photoCursorIos_ goToRow:position];
    }
    return INVALID_POS;
}

int ResultSetIos::GetBlob(int columnIndex, std::vector<uint8_t> &value)
{
    return SUCCESS_CODE;
}

int ResultSetIos::GetString(int columnIndex, std::string &value)
{
    if (columnIndex >= PHOTO_COLUMN_MAP.size()) {
        NSLog(@"ResultSetIos GetString columnIndex is invalid");
        return INVALID_POS;
    }
    auto it = std::next(PHOTO_COLUMN_MAP.begin(), columnIndex);
    std::string field = it->second;
    if (photoCursorIos_) {
        NSString *fieldIos = [NSString stringWithCString:field.c_str() encoding:[NSString defaultCStringEncoding]];
        NSString *result = [photoCursorIos_ getString:fieldIos];
        value = std::string([result UTF8String]);
        return SUCCESS_CODE;  
    }
    return INVALID_POS;
}

int ResultSetIos::GetInt(int columnIndex, int &value)
{
    if (columnIndex >= PHOTO_COLUMN_MAP.size()) {
        NSLog(@"ResultSetIos GetInt columnIndex is invalid");
        return INVALID_POS;
    }
    auto it = std::next(PHOTO_COLUMN_MAP.begin(), columnIndex);
    std::string field = it->second;
    if (photoCursorIos_) {
        NSString *fieldIos = [NSString stringWithCString:field.c_str() encoding:[NSString defaultCStringEncoding]];
        value = [photoCursorIos_ getInt:fieldIos];
        return SUCCESS_CODE;  
    }
    return INVALID_POS;
}

int ResultSetIos::GetLong(int columnIndex, int64_t &value)
{
    if (columnIndex >= PHOTO_COLUMN_MAP.size()) {
        NSLog(@"ResultSetIos GetLong columnIndex is invalid");
        return INVALID_POS;
    }
    auto it = std::next(PHOTO_COLUMN_MAP.begin(), columnIndex);
    std::string field = it->second;
    if (photoCursorIos_) {
        NSString *fieldIos = [NSString stringWithCString:field.c_str() encoding:[NSString defaultCStringEncoding]];
        value = [photoCursorIos_ getLong:fieldIos];
        return SUCCESS_CODE;  
    }
    return INVALID_POS;
}

int ResultSetIos::GetDouble(int columnIndex, double &value)
{
    return SUCCESS_CODE;
}

int ResultSetIos::IsColumnNull(int columnIndex, bool &isNull)
{
    return SUCCESS_CODE;
}

int ResultSetIos::Close()
{
    if (photoCursorIos_) {
        [photoCursorIos_ close];
        isClose_ = true;
        return SUCCESS_CODE;  
    }
    return INVALID_POS;
}

int ResultSetIos::GoTo(int offset)
{
    return SUCCESS_CODE;
}

int ResultSetIos::GoToFirstRow()
{
    if (photoCursorIos_) {
        return [photoCursorIos_ goToFirstRow];
    }
    return INVALID_POS;
}

int ResultSetIos::GoToLastRow()
{
    if (photoCursorIos_) {
        return [photoCursorIos_ goToLastRow];
    }
    return INVALID_POS;
}

int ResultSetIos::GoToNextRow()
{
    if (photoCursorIos_) {
        return [photoCursorIos_ goToNextRow];
    }
    return INVALID_POS;
}

int ResultSetIos::GoToPreviousRow()
{
    if (photoCursorIos_) {
        return [photoCursorIos_ goToPreviousRow];
    }
    return INVALID_POS;
}

int ResultSetIos::IsAtLastRow(bool &result)
{
    if (photoCursorIos_) {
        result = [photoCursorIos_ isAtLastRow];
        return SUCCESS_CODE;
    }
    return INVALID_POS;
}

int ResultSetIos::GetColumnCount(int &count)
{
    count = PHOTO_COLUMN_MAP.size();
    return SUCCESS_CODE;
}

int ResultSetIos::GetColumnIndex(const std::string &columnName, int &columnIndex)
{
    int index = INVALID_POS;
    std::map<std::string, std::string>::iterator iter;
    for (iter = PHOTO_COLUMN_MAP.begin(); iter != PHOTO_COLUMN_MAP.end(); ++iter) {
        index++;
        if (iter->first == columnName) {
            break;
        }
    }
    columnIndex = index;
    return index == INVALID_POS ? INVALID_POS : SUCCESS_CODE;
}

int ResultSetIos::GetColumnName(int columnIndex, std::string &columnName)
{
    if (columnIndex >= PHOTO_COLUMN_MAP.size()) {
        NSLog(@"ResultSetIos GetColumnName columnIndex is invalid");
        return INVALID_POS;
    }
    auto it = std::next(PHOTO_COLUMN_MAP.begin(), columnIndex);
    columnName = it->first;
    return SUCCESS_CODE;
}

bool ResultSetIos::IsClosed() const
{
    return isClose_;
}
} // namespace Media
} // namespace OHOS