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

#ifndef INTERFACES_KITS_JS_MEDIALIBRARY_INCLUDE_RESULT_SET_H
#define INTERFACES_KITS_JS_MEDIALIBRARY_INCLUDE_RESULT_SET_H

#include <string>
#include <vector>

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))
class ResultSet {
public:
    EXPORT ResultSet() {};
    EXPORT virtual ~ResultSet() {};
    
    virtual int GetRowCount(int &count) = 0;
    virtual int GetAllColumnNames(std::vector<std::string> &columnNames) = 0;
    virtual int GetBlob(int columnIndex, std::vector<uint8_t> &blob) = 0;
    virtual int GetString(int columnIndex, std::string &value) = 0;
    virtual int GetInt(int columnIndex, int &value) = 0;
    virtual int GetLong(int columnIndex, int64_t &value) = 0;
    virtual int GetDouble(int columnIndex, double &value) = 0;
    virtual int GoToRow(int position) = 0;
    virtual int GoTo(int offset) = 0;
    virtual int GoToFirstRow() = 0;
    virtual int GoToLastRow() = 0;
    virtual int GoToNextRow() = 0;
    virtual int IsAtLastRow(bool &result) = 0;
    virtual int GetColumnCount(int &count) = 0;
    virtual int GetColumnIndex(const std::string &columnName, int &columnIndex) = 0;
    virtual int GetColumnName(int columnIndex, std::string &columnName) = 0;
    virtual int Close() = 0;
};
}
}
#endif // INTERFACES_KITS_JS_MEDIALIBRARY_INCLUDE_RESULT_SET_H