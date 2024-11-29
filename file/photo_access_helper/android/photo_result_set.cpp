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
#include "photo_result_set.h"

#include <securec.h>
#include <sstream>
#include "java/jni/photo_cursor_jni.h"

namespace OHOS {
namespace Media {
namespace {
    // The default position of the cursor
    static const int INITIAL_POS = -1;
} // namespace

PhotoResultSet::PhotoResultSet(JNIEnv* env, jobject object)
    : cursorObject_(Plugin::PhotoCursorJni::GetInstance())
{
    cursorObject_->Register(env, object);
}

PhotoResultSet::~PhotoResultSet()
{
    Close();
}

int PhotoResultSet::GetAllColumnNames(std::vector<std::string> &columnNames)
{
    if (cursorObject_) {
        return cursorObject_->GetAllColumnNames(columnNames);
    }
    return 0;
}

int PhotoResultSet::GetRowCount(int &count)
{
    if (cursorObject_) {
        return cursorObject_->GetRowCount(count);
    }
    return 0;
}

int PhotoResultSet::GoToRow(int position)
{
    if (cursorObject_) {
        return cursorObject_->GoToRow(position);
    }
    return 0;
}

int PhotoResultSet::GetBlob(int columnIndex, std::vector<uint8_t> &value)
{
    if (cursorObject_) {
        return cursorObject_->GetBlob(columnIndex, value);
    }
    return 0;
}

int PhotoResultSet::GetString(int columnIndex, std::string &value)
{
    if (cursorObject_) {
        return cursorObject_->GetString(columnIndex, value);
    }
    return 0;
}

int PhotoResultSet::GetInt(int columnIndex, int &value)
{
    if (cursorObject_) {
        return cursorObject_->GetInt(columnIndex, value);
    }
    return 0;
}

int PhotoResultSet::GetLong(int columnIndex, int64_t &value)
{
    if (cursorObject_) {
        return cursorObject_->GetLong(columnIndex, value);
    }
    return 0;
}

int PhotoResultSet::GetDouble(int columnIndex, double &value)
{
    if (cursorObject_) {
        return cursorObject_->GetDouble(columnIndex, value);
    }
    return 0;
}

int PhotoResultSet::Close()
{
    return 0;
}

int PhotoResultSet::GoTo(int offset) {
    if (cursorObject_) {
        return cursorObject_->GoTo(offset);
    }
    return 0;
}

int PhotoResultSet::GoToFirstRow() {
    if (cursorObject_) {
        return cursorObject_->GoToFirstRow();
    }
    return 0;
}

int PhotoResultSet::GoToLastRow() {
    if (cursorObject_) {
        return cursorObject_->GoToLastRow();
    }
    return 0;
}

int PhotoResultSet::GoToNextRow() {
    if (cursorObject_) {
        return cursorObject_->GoToNextRow();
    }
    return 0;
}

int PhotoResultSet::IsAtLastRow(bool &result) {
    if (cursorObject_) {
        return cursorObject_->IsAtLastRow(result);
    }
    return 0;
}

int PhotoResultSet::GetColumnCount(int &count) {
    if (cursorObject_) {
        return cursorObject_->GetColumnCount(count);
    }
    return 0;
}

int PhotoResultSet::GetColumnIndex(const std::string &columnName, int &columnIndex) {
    if (cursorObject_) {
        return cursorObject_->GetColumnIndex(columnName, columnIndex);
    }
    return 0;
}

int PhotoResultSet::GetColumnName(int columnIndex, std::string &columnName) {
    if (cursorObject_) {
        return cursorObject_->GetColumnName(columnIndex, columnName);
    }
    return 0;
}

} // namespace DataShare
} // namespace OHOS