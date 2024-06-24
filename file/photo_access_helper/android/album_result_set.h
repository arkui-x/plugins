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

#ifndef INTERFACES_KITS_JS_MEDIALIBRARY_INCLUDE_ALBUM_RESULT_SET_H
#define INTERFACES_KITS_JS_MEDIALIBRARY_INCLUDE_ALBUM_RESULT_SET_H

#include <string>
#include <vector>

#include <jni.h>
#include "java/jni/photo_cursor_jni.h"
#include "plugins/file/photo_access_helper/napi/base/include/result_set.h"

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))
class AlbumResultSet : public ResultSet {
public:
    EXPORT AlbumResultSet() {}
    EXPORT ~AlbumResultSet();
    EXPORT AlbumResultSet(JNIEnv* env, jobject object);
    
    EXPORT int GetRowCount(int &count);
    EXPORT int GetAllColumnNames(std::vector<std::string> &columnNames);
    EXPORT int GetBlob(int columnIndex, std::vector<uint8_t> &blob);
    EXPORT int GetString(int columnIndex, std::string &value);
    EXPORT int GetInt(int columnIndex, int &value);
    EXPORT int GetLong(int columnIndex, int64_t &value);
    EXPORT int GetDouble(int columnIndex, double &value);
    EXPORT int GoToRow(int position);
    EXPORT int GoTo(int offset);
    EXPORT int GoToFirstRow();
    EXPORT int GoToLastRow();
    EXPORT int GoToNextRow();
    EXPORT int IsAtLastRow(bool &result);
    EXPORT int GetColumnCount(int &count);
    EXPORT int GetColumnIndex(const std::string &columnName, int &columnIndex);
    EXPORT int GetColumnName(int columnIndex, std::string &columnName);
    EXPORT int Close();

private:
    std::string getAndroidName(int columnIndex);

    std::vector<std::string> columnNames_;
};
}
}

#endif // INTERFACES_KITS_JS_MEDIALIBRARY_INCLUDE_ALBUM_RESULT_SET_H