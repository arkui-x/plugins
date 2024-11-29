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
#include "album_result_set.h"

#include <algorithm>
#include <iterator>
#include <securec.h>
#include <sstream>
#include <map>
#include <codecvt>
#include <locale>

#include "inner_api/plugin_utils_inner.h"
#include "java/jni/photo_cursor_jni.h"
#include "log.h"
#include "plugin_utils.h"
#include "plugins/file/photo_access_helper/napi/album/include/photo_album_column.h"

namespace OHOS {
namespace Media {
namespace {
    // The default position of the cursor
    static const int INVILID_CODE = -1;
    static const int SUCCESS_CODE = 0;
    static int CURSOR_INDEX = -1;
    static const std::map<std::string, std::string> ALBUM_COLUMN_MAP = {
    { "bucket_id", Media::PhotoAlbumColumns::ALBUM_ID },
    { "bucket_display_name", Media::PhotoAlbumColumns::ALBUM_NAME },
    { "count(_data)", Media::PhotoAlbumColumns::ALBUM_COUNT },
    { "sum(media_type)", "sum" },
};
} // namespace

struct {
    jmethodID getColumns;
    jmethodID getFirstValue;
    jmethodID getNextValue;
    jmethodID getLastValue;
    jmethodID gotoRow;
    jmethodID move;
    jmethodID size;
    jmethodID getAsBoolean;
    jmethodID getAsString;
    jmethodID getAsInteger;
    jmethodID getAsLong;
    jmethodID getAsDouble;
    jmethodID IsAtLastRow;
    jobject globalRef;
    jobject currentValue;
} g_pluginClass;

static jstring StringToJavaString(JNIEnv* env, const std::string& string)
{
    std::u16string str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(string);
    return env->NewString(reinterpret_cast<const jchar*>(str.data()), str.length());
}

static void jstringListToStdStringVector(JNIEnv *env, jobject list, std::vector<std::string> &columnNames) {
    jclass listClass = env->FindClass("java/util/List");
    jmethodID sizeMethod = env->GetMethodID(listClass, "size", "()I");
    jmethodID getMethod = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");
    jsize listSize = env->CallIntMethod(list, sizeMethod);

    for (jsize i = 0; i < listSize; ++i) {
        jobject element = env->CallObjectMethod(list, getMethod, i);
        jstring jstr = (jstring)element;
        const char *str = env->GetStringUTFChars(jstr, nullptr);
        auto it = ALBUM_COLUMN_MAP.find(str);
        if (it != ALBUM_COLUMN_MAP.end()) {
            columnNames.push_back(it->second);
        } else {
            columnNames.push_back(str);
        }
        env->ReleaseStringUTFChars(jstr, str);
    }
}

std::string AlbumResultSet::getAndroidName(int columnIndex) {
    std::string str = columnNames_[columnIndex];
    for (auto it = ALBUM_COLUMN_MAP.begin(); it != ALBUM_COLUMN_MAP.end(); it++) {
        if (it->second == str) {
            str = it->first;
            break;
        }
    }
    return str;
}

AlbumResultSet::AlbumResultSet(JNIEnv* env, jobject jobj)
{
    CHECK_NULL_VOID(env);
    g_pluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    jclass QueryMapCls = env->GetObjectClass(jobj); 
    CHECK_NULL_VOID(QueryMapCls);

    g_pluginClass.getColumns = env->GetMethodID(QueryMapCls, "getColumns", "()Ljava/util/List;");
    CHECK_NULL_VOID(g_pluginClass.getColumns);

    g_pluginClass.getFirstValue = env->GetMethodID(QueryMapCls, "getFirstValue", "()Landroid/content/ContentValues;");
    CHECK_NULL_VOID(g_pluginClass.getFirstValue);

    g_pluginClass.getNextValue = env->GetMethodID(QueryMapCls, "getNextValue", "()Landroid/content/ContentValues;");
    CHECK_NULL_VOID(g_pluginClass.getNextValue);

    g_pluginClass.getLastValue = env->GetMethodID(QueryMapCls, "getLastValue", "()Landroid/content/ContentValues;");
    CHECK_NULL_VOID(g_pluginClass.getLastValue);

    g_pluginClass.gotoRow = env->GetMethodID(QueryMapCls, "gotoRow", "(I)Landroid/content/ContentValues;");
    CHECK_NULL_VOID(g_pluginClass.gotoRow);

    g_pluginClass.move = env->GetMethodID(QueryMapCls, "move", "(I)Landroid/content/ContentValues;");
    CHECK_NULL_VOID(g_pluginClass.move);

    g_pluginClass.size = env->GetMethodID(QueryMapCls, "size", "()I");
    CHECK_NULL_VOID(g_pluginClass.size);

    g_pluginClass.getAsBoolean = env->GetMethodID(QueryMapCls, "getAsBoolean",
        "(Landroid/content/ContentValues;Ljava/lang/String;)[B");
    CHECK_NULL_VOID(g_pluginClass.getAsBoolean);

    g_pluginClass.IsAtLastRow = env->GetMethodID(QueryMapCls, "IsAtLastRow", "()Z");
    CHECK_NULL_VOID(g_pluginClass.IsAtLastRow);

    jclass ContentValuesCls = env->FindClass("android/content/ContentValues");
    CHECK_NULL_VOID(ContentValuesCls);

    g_pluginClass.getAsString =
        env->GetMethodID(ContentValuesCls, "getAsString", "(Ljava/lang/String;)Ljava/lang/String;");
    CHECK_NULL_VOID(g_pluginClass.getAsString);

    g_pluginClass.getAsInteger =
        env->GetMethodID(ContentValuesCls, "getAsInteger", "(Ljava/lang/String;)Ljava/lang/Integer;");
    CHECK_NULL_VOID(g_pluginClass.getAsInteger);

    g_pluginClass.getAsLong =
        env->GetMethodID(ContentValuesCls, "getAsLong", "(Ljava/lang/String;)Ljava/lang/Long;");
    CHECK_NULL_VOID(g_pluginClass.getAsLong);

    g_pluginClass.getAsDouble =
        env->GetMethodID(ContentValuesCls, "getAsDouble", "(Ljava/lang/String;)Ljava/lang/Double;");
    CHECK_NULL_VOID(g_pluginClass.getAsDouble);

    jobject columns = env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getColumns);
    jstringListToStdStringVector(env, columns, columnNames_);
}

AlbumResultSet::~AlbumResultSet()
{
    Close();
}

int AlbumResultSet::GetAllColumnNames(std::vector<std::string> &columnNames)
{
    jint result = INVILID_CODE;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getColumns)) {
        LOGW("Jni get none ptr error");
        return result;
    }
    jobject columns = env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getColumns);
    if (env->ExceptionCheck() || !columns) {
        LOGE("JNI: call createPhoto failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVILID_CODE;
    }
    jstringListToStdStringVector(env, columns, columnNames);
    columnNames_ = columnNames;
    return 0;
}

int AlbumResultSet::GetRowCount(int &count)
{
    jint result = INVILID_CODE;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.size)) {
        LOGW("PhotoCursorJni get none ptr error");
        return result;
    }
    result = env->CallIntMethod(g_pluginClass.globalRef, g_pluginClass.size);
    count = result;
    if (env->ExceptionCheck()) {
        LOGE("AlbumResultSet: call GetRowCount failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return result > 0 ? 0 : INVILID_CODE;
}

int AlbumResultSet::GoToRow(int position)
{
    jint result = INVILID_CODE;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.gotoRow)) {
        LOGW("Jni get none ptr error");
        return result;
    }
    g_pluginClass.currentValue = env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.gotoRow, position);
    if (env->ExceptionCheck() || !g_pluginClass.currentValue) {
        LOGE(" JNI: call createPhoto failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return 0;
}

int AlbumResultSet::GetBlob(int columnIndex, std::vector<uint8_t> &value)
{
    jint result = INVILID_CODE;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getAsBoolean)) {
        LOGW("PhotoCursorJni Jni get none ptr error");
        return result;
    }
    std::string str = getAndroidName(columnIndex);
    jstring jcolumnName = StringToJavaString(env, str);
    jbyteArray columnValue = static_cast<jbyteArray>(env->CallObjectMethod(
        g_pluginClass.globalRef, g_pluginClass.getAsBoolean, g_pluginClass.currentValue, jcolumnName));
    jsize arrayLength = env->GetArrayLength(columnValue);
    jbyte* jstr = env->GetByteArrayElements(columnValue, NULL);
    value.insert(value.begin(), jstr, jstr + arrayLength * sizeof(jstr[0]));
    env->ReleaseByteArrayElements(columnValue, jstr, 0);
    env->DeleteLocalRef(columnValue);
    if (env->ExceptionCheck()) {
        LOGE("PhotoCursorJni JNI: call createPhoto failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return 0;
}

int AlbumResultSet::GetString(int columnIndex, std::string &value)
{
    jint result = INVILID_CODE;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.currentValue) || !(g_pluginClass.getAsString)) {
        LOGW(" Jni get none ptr error");
        return result;
    }
    std::string str = getAndroidName(columnIndex);
    jstring jcolumnName = StringToJavaString(env, str);
    jstring columnValue = static_cast<jstring>(env->CallObjectMethod(
        g_pluginClass.currentValue, g_pluginClass.getAsString, jcolumnName));
    value = env->GetStringUTFChars(columnValue, NULL);
    if (env->ExceptionCheck()) {
        LOGE(" JNI: call createPhoto failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return 0;
}

int AlbumResultSet::GetInt(int columnIndex, int &value)
{
    jint result = INVILID_CODE;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.currentValue) || !(g_pluginClass.getAsInteger)) {
        LOGW(" Jni get none ptr error");
        return result;
    }
    std::string str = getAndroidName(columnIndex);
    jstring jcolumnName = StringToJavaString(env, str);
    jobject jValue = env->CallObjectMethod(
        g_pluginClass.currentValue, g_pluginClass.getAsInteger, jcolumnName);
    jclass IntegerClass = env->FindClass("java/lang/Integer");
    jmethodID intMethod = env->GetMethodID(IntegerClass, "intValue", "()I");
    value = env->CallIntMethod(jValue, intMethod);
    if (env->ExceptionCheck()) {
        LOGE(" JNI: call createPhoto failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return 0;
}

int AlbumResultSet::GetLong(int columnIndex, int64_t &value)
{
    jint result = INVILID_CODE;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.currentValue) || !(g_pluginClass.getAsLong)) {
        LOGW(" Jni get none ptr error");
        return result;
    }
    std::string str = getAndroidName(columnIndex);
    jstring jcolumnName = StringToJavaString(env, str);
    jobject jValue = env->CallObjectMethod(
        g_pluginClass.currentValue, g_pluginClass.getAsLong, jcolumnName);
    jclass LongClass = env->FindClass("java/lang/Long");
    jmethodID LongMethod = env->GetMethodID(LongClass, "longValue", "()J");
    value = env->CallLongMethod(jValue, LongMethod);
    if (env->ExceptionCheck()) {
        LOGE(" JNI: call createPhoto failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return 0;
}

int AlbumResultSet::GetDouble(int columnIndex, double &value)
{
    jint result = INVILID_CODE;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.currentValue) || !(g_pluginClass.getAsDouble)) {
        LOGW(" Jni get none ptr error");
        return result;
    }
    std::string str = getAndroidName(columnIndex);
    jstring jcolumnName = StringToJavaString(env, str);
    jobject jValue = env->CallObjectMethod(
        g_pluginClass.currentValue, g_pluginClass.getAsDouble, jcolumnName);
    jclass DoubleClass = env->FindClass("java/lang/Double");
    jmethodID DoubleMethod = env->GetMethodID(DoubleClass, "doubleValue", "()D");
    value = env->CallDoubleMethod(jValue, DoubleMethod);
    if (env->ExceptionCheck()) {
        LOGE(" JNI: call createPhoto failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return 0;
}

int AlbumResultSet::Close()
{
    return 0;
}

int AlbumResultSet::GoTo(int offset)
{
    jint result = INVILID_CODE;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.move)) {
        LOGW("Jni get none ptr error");
        return result;
    }
    g_pluginClass.currentValue = env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.move, offset);
    if (env->ExceptionCheck() || !g_pluginClass.currentValue) {
        LOGE("JNI: call createPhoto failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVILID_CODE;
    }
    return 0;
}

int AlbumResultSet::GoToFirstRow() {
    jint result = INVILID_CODE;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getFirstValue)) {
        LOGW("Jni get none ptr error");
        return result;
    }
    g_pluginClass.currentValue = env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getFirstValue);
    LOGE("AlbumResultSet::GoToFirstRow");
    if (env->ExceptionCheck() || !g_pluginClass.currentValue) {
        LOGE(" JNI: call createPhoto failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVILID_CODE;
    }
    return 0;
}

int AlbumResultSet::GoToLastRow() {
    jint result = INVILID_CODE;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getLastValue)) {
        LOGW("Jni get none ptr error");
        return result;
    }
    g_pluginClass.currentValue = env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getLastValue);
    if (env->ExceptionCheck() || !g_pluginClass.currentValue) {
        LOGE(" JNI: call createPhoto failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVILID_CODE;
    }
    return 0;
}

int AlbumResultSet::GoToNextRow() {
    jint result = INVILID_CODE;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getNextValue)) {
        LOGW("Jni get none ptr error");
        return result;
    }
    g_pluginClass.currentValue = env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getNextValue);
    if (env->ExceptionCheck() || !g_pluginClass.currentValue) {
        LOGE("JNI: call GoToNextRow failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVILID_CODE;
    }
    return 0;
}

int AlbumResultSet::IsAtLastRow(bool &result) {
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.IsAtLastRow)) {
        LOGW("Jni get none ptr error");
        return INVILID_CODE;
    }
    result = env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.IsAtLastRow);
    if (env->ExceptionCheck()) {
        LOGE("JNI: call createPhoto failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return SUCCESS_CODE;
}

int AlbumResultSet::GetColumnCount(int &count) {
    jint result = INVILID_CODE;
    if (columnNames_.empty()) {
        auto env = ARKUI_X_Plugin_GetJniEnv();
        if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getColumns)) {
            LOGW("Jni get none ptr error");
            return result;
        }
        jobject columns = env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getColumns);
        std::vector<std::string> columnNames;
        jstringListToStdStringVector(env, columns, columnNames);
        columnNames_ = columnNames;
    }
    count = columnNames_.size();
    return 0;
}

int AlbumResultSet::GetColumnIndex(const std::string &columnName, int &columnIndex) {
    jint result = INVILID_CODE;
    std::string name = columnName;
    for (auto it = ALBUM_COLUMN_MAP.begin(); it != ALBUM_COLUMN_MAP.end(); it++) {
        if (it->second == columnName) {
            name = it->first;
            break;
        }
    }
    for (int index = 0; index < columnNames_.size(); index++) {
        if (columnName.compare(columnNames_[index]) == 0) {
            columnIndex = index;
            return SUCCESS_CODE;
        }
    }

    return INVILID_CODE;
}

int AlbumResultSet::GetColumnName(int columnIndex, std::string &columnName)
{
    if (columnIndex >= columnNames_.size()) {
        return INVILID_CODE;
    }
    columnName = columnNames_[columnIndex];
    return 0;
}
} // namespace DataShare
} // namespace OHOS