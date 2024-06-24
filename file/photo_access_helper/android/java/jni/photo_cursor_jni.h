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

#ifndef PLUGINS_PHOTO_CURSOR_ANDROID_JAVA_JNI_PHOTO_CURSOR_JNI_H
#define PLUGINS_PHOTO_CURSOR_ANDROID_JAVA_JNI_PHOTO_CURSOR_JNI_H

#include <jni.h>
#include <memory>
#include <string>
#include <vector>

namespace OHOS::Plugin {
class PhotoCursorJni final {
public:
    PhotoCursorJni() {};
    ~PhotoCursorJni() {};

    static std::shared_ptr<PhotoCursorJni> GetInstance();
    void Register(JNIEnv* env, jobject jobj);
    int GoTo(int offset);
    int GoToFirstRow();
    int GoToLastRow();
    int GoToNextRow();
    int IsAtLastRow(bool &result);
    int GetColumnCount(int &count);
    int GetRowCount(int &count);
    int GoToRow(int position);
    int GetColumnIndex(const std::string &columnName, int &columnIndex);
    int GetColumnName(int columnIndex, std::string &columnName);
    int GetAllColumnNames(std::vector<std::string> &columnNames);
    int GetBlob(int columnIndex, std::vector<uint8_t> &blob);
    int GetString(int columnIndex, std::string &value);
    int GetInt(int columnIndex, int &value);
    int GetLong(int columnIndex, int64_t &value);
    int GetDouble(int columnIndex, double &value);
};
} // namespace OHOS::Plugin
#endif // PLUGINS_PHOTO_CURSOR_ANDROID_JAVA_JNI_PHOTO_CURSOR_JNI_H
