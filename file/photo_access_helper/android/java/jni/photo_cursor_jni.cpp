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

#include "photo_cursor_jni.h"

#include <jni.h>
#include <codecvt>
#include <locale>
#include <map>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"
#include "photo_plugin_jni.h"
#include "plugins/file/photo_access_helper/napi/photo/include/media_column.h"
#include "plugins/file/photo_access_helper/napi/album/include/photo_album_column.h"

namespace OHOS::Plugin {
namespace {
static const int INVILID_CODE = -1;
static const int SUCCESS_CODE = 0;

static const std::map<std::string, std::string> COLUMN_MAP = {
    { "_data", Media::PhotoColumn::MEDIA_FILE_PATH },
    { "_id", Media::PhotoColumn::MEDIA_ID },
    { "_size", Media::PhotoColumn::MEDIA_SIZE },
    { "_display_name", Media::PhotoColumn::MEDIA_NAME },
    { "owner_package_name", Media::PhotoColumn::MEDIA_OWNER_PACKAGE },
    { "bucket_id", Media::PhotoAlbumColumns::ALBUM_ID },
    { "bucket_display_name", Media::PhotoAlbumColumns::ALBUM_NAME },
    { "datetaken", Media::PhotoColumn::MEDIA_DATE_TAKEN },
    { "count(_data)", Media::PhotoAlbumColumns::ALBUM_COUNT },
    { "sum(media_type)", "sum" },
};

struct {
    jmethodID goTo;
    jmethodID getColumnIndex;
    jmethodID getColumnName;
    jmethodID getColumnNames;
    jmethodID isClosed;
    jmethodID moveToPosition;

    jmethodID moveToFirst;
    jmethodID getColumnCount;
    jmethodID moveToLast;
    jmethodID moveToNext;
    jmethodID moveToPrevious;
    jmethodID isLast;
    jmethodID isAfterLast;
    jmethodID getCount;
    jmethodID getBlob;
    jmethodID getString;
    jmethodID getInt;
    jmethodID getLong;
    jmethodID getDouble;
    jobject globalRef;
} g_pluginClass;

} // namespace

std::shared_ptr<PhotoCursorJni> PhotoCursorJni::GetInstance()
{
    return std::make_shared<PhotoCursorJni>();
}

void PhotoCursorJni::Register(JNIEnv* env, jobject jobj)
{
    CHECK_NULL_VOID(env);
    g_pluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj); 
    CHECK_NULL_VOID(cls);

    g_pluginClass.goTo = env->GetMethodID(cls, "move", "(I)Z");
    CHECK_NULL_VOID(g_pluginClass.goTo);

    g_pluginClass.getColumnIndex = env->GetMethodID(cls, "getColumnIndex", "(Ljava/lang/String;)I");
    CHECK_NULL_VOID(g_pluginClass.getColumnIndex);

    g_pluginClass.getColumnName = env->GetMethodID(cls, "getColumnName", "(I)Ljava/lang/String;");
    CHECK_NULL_VOID(g_pluginClass.getColumnName);

    g_pluginClass.getColumnNames = env->GetMethodID(cls, "getColumnNames", "()[Ljava/lang/String;");
    CHECK_NULL_VOID(g_pluginClass.getColumnNames);

    g_pluginClass.isClosed = env->GetMethodID(cls, "isClosed", "()Z");
    CHECK_NULL_VOID(g_pluginClass.isClosed);

    g_pluginClass.moveToPosition = env->GetMethodID(cls, "moveToPosition", "(I)Z");
    CHECK_NULL_VOID(g_pluginClass.moveToPosition);

    g_pluginClass.moveToFirst = env->GetMethodID(cls, "moveToFirst", "()Z");
    CHECK_NULL_VOID(g_pluginClass.moveToFirst);

    g_pluginClass.getColumnCount = env->GetMethodID(cls, "getColumnCount", "()I");
    CHECK_NULL_VOID(g_pluginClass.getColumnCount);

    g_pluginClass.moveToLast = env->GetMethodID(cls, "moveToLast", "()Z");
    CHECK_NULL_VOID(g_pluginClass.moveToLast);

    g_pluginClass.moveToNext = env->GetMethodID(cls, "moveToNext", "()Z");
    CHECK_NULL_VOID(g_pluginClass.moveToNext);

    g_pluginClass.moveToPrevious = env->GetMethodID(cls, "moveToPrevious", "()Z");
    CHECK_NULL_VOID(g_pluginClass.moveToPrevious);

    g_pluginClass.isAfterLast = env->GetMethodID(cls, "isAfterLast", "()Z");
    CHECK_NULL_VOID(g_pluginClass.isAfterLast);

    g_pluginClass.isLast = env->GetMethodID(cls, "isLast", "()Z");
    CHECK_NULL_VOID(g_pluginClass.isLast);

    g_pluginClass.getCount = env->GetMethodID(cls, "getCount", "()I");
    CHECK_NULL_VOID(g_pluginClass.getCount);

    g_pluginClass.getBlob = env->GetMethodID(cls, "getBlob", "(I)[B");
    CHECK_NULL_VOID(g_pluginClass.getBlob);

    g_pluginClass.getInt = env->GetMethodID(cls, "getInt", "(I)I");
    CHECK_NULL_VOID(g_pluginClass.getInt);

    g_pluginClass.getString = env->GetMethodID(cls, "getString", "(I)Ljava/lang/String;");
    CHECK_NULL_VOID(g_pluginClass.getString);

    g_pluginClass.getLong = env->GetMethodID(cls, "getLong", "(I)J");
    CHECK_NULL_VOID(g_pluginClass.getLong);

    g_pluginClass.getDouble = env->GetMethodID(cls, "getDouble", "(I)D");
    CHECK_NULL_VOID(g_pluginClass.getDouble);

    LOGI("PhotoCursorJni NativeInit===");

    env->DeleteLocalRef(cls);
}

static jstring StringToJavaString(JNIEnv* env, const std::string& string)
{
    std::u16string str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(string);
    return env->NewString(reinterpret_cast<const jchar*>(str.data()), str.length());
}

int PhotoCursorJni::GoTo(int offset) {
    jboolean result = JNI_FALSE;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.goTo)) {
        LOGW("PhotoCursorJni get none ptr error");
        return INVILID_CODE;
    }
    result = env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.goTo, offset);
    if (env->ExceptionCheck()) {
        LOGE("PhotoCursorJni: call goTo failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVILID_CODE;
    }
    return result == JNI_FALSE ? INVILID_CODE : SUCCESS_CODE;
}

int PhotoCursorJni::GoToFirstRow()
{
    jboolean result = JNI_FALSE;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.moveToFirst)) {
        LOGW("PhotoCursorJni get none ptr error");
        return INVILID_CODE;
    }
    result = env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.moveToFirst);
    if (env->ExceptionCheck()) {
        LOGE("PhotoCursorJni: call GoToFirstRow failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVILID_CODE;
    }
    return result == JNI_FALSE ? INVILID_CODE : SUCCESS_CODE;
}

int PhotoCursorJni::GoToLastRow()
{
    jboolean result = JNI_FALSE;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.moveToLast)) {
        LOGW("PhotoCursorJni get none ptr error");
        return INVILID_CODE;
    }
    result = env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.moveToLast);
    if (env->ExceptionCheck()) {
        LOGE("PhotoCursorJni: call GoToLastRow failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVILID_CODE;
    }
    return result == JNI_FALSE ? INVILID_CODE : SUCCESS_CODE;
}

int PhotoCursorJni::GoToNextRow()
{
    jboolean result = JNI_FALSE;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.moveToNext)) {
        LOGW("PhotoCursorJni get none ptr error");
        return INVILID_CODE;
    }
    result = env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.moveToNext);
    if (env->ExceptionCheck()) {
        LOGE("PhotoCursorJni: call GoToNextRow failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVILID_CODE;
    }
    return result == JNI_FALSE ? INVILID_CODE : SUCCESS_CODE;
}

int PhotoCursorJni::IsAtLastRow(bool &result)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.isAfterLast) || !(g_pluginClass.isLast)) {
        LOGW("PhotoCursorJni get none ptr error");
        return INVILID_CODE;
    }
    result = env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.isAfterLast) ||
        env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.isLast);
    if (env->ExceptionCheck()) {
        LOGE("PhotoCursorJni: call IsAtLastRow failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVILID_CODE;
    }
    return SUCCESS_CODE;
}

int PhotoCursorJni::GetColumnCount(int &count)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getColumnCount)) {
        LOGW("PhotoCursorJni get none ptr error");
        return INVILID_CODE;
    }
    count = env->CallIntMethod(g_pluginClass.globalRef, g_pluginClass.getColumnCount);
    if (env->ExceptionCheck()) {
        LOGE("PhotoPluginJni: call GetColumnCount failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVILID_CODE;
    }
    return SUCCESS_CODE;
}

int PhotoCursorJni::GetRowCount(int &count)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getCount)) {
        LOGW("PhotoCursorJni get none ptr error");
        return INVILID_CODE;
    }
    count = env->CallIntMethod(g_pluginClass.globalRef, g_pluginClass.getCount);
    if (env->ExceptionCheck()) {
        LOGE("PhotoPluginJni: call GetRowCount failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVILID_CODE;
    }
    return SUCCESS_CODE;
}

int PhotoCursorJni::GoToRow(int position) {
    jboolean result = JNI_FALSE;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.moveToPosition)) {
        LOGW("PhotoCursorJni get none ptr error");
        return INVILID_CODE;
    }
    result = env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.moveToPosition, position);

    if (env->ExceptionCheck()) {
        LOGE("PhotoPluginJni: call GoToRow failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVILID_CODE;
    }
    return result == JNI_FALSE ? INVILID_CODE : SUCCESS_CODE;
}

int PhotoCursorJni::GetAllColumnNames(std::vector<std::string> &columnNames) {
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getColumnNames)) {
        LOGW("PhotoCursorJni Jni get none ptr error");
        return INVILID_CODE;
    }
    jobjectArray photoColumnNames = static_cast<jobjectArray>(env->CallObjectMethod(
        g_pluginClass.globalRef, g_pluginClass.getColumnNames));
    jsize arrayLength = env->GetArrayLength(photoColumnNames);
    jstring jstr = nullptr;
    const char *str = nullptr;
    for (int index = 0; index < arrayLength; index++) {
        jstr = static_cast<jstring>(env->GetObjectArrayElement(photoColumnNames, index));
        str = env->GetStringUTFChars(jstr, NULL);
        if (str != nullptr) {
            auto it = COLUMN_MAP.find(std::string(str));
            if (it != COLUMN_MAP.end()) {
                std::string androidColumnName = it->second;
                columnNames.push_back(androidColumnName);
            } else {
                columnNames.push_back(std::string(str));
            }
            env->ReleaseStringUTFChars(jstr, str);
        }
        if (jstr != nullptr) {
            env->DeleteLocalRef(jstr);
        }
        str = nullptr;
    }
    if (env->ExceptionCheck()) {
        LOGE("PhotoCursorJni JNI: call GetAllColumnNames failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVILID_CODE;
    }
    return SUCCESS_CODE;
}

int PhotoCursorJni::GetColumnIndex(const std::string &columnName, int &columnIndex)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getColumnIndex)) {
        LOGW("PhotoCursorJni get none ptr error");
        return INVILID_CODE;
    }
    std::string name = columnName;
    for (auto it = COLUMN_MAP.begin(); it != COLUMN_MAP.end(); it++) {
        if (it->second == columnName) {
            name = it->first;
            break;
        }
    }
    jstring jColumnName = StringToJavaString(env, name);
    columnIndex = env->CallIntMethod(g_pluginClass.globalRef, g_pluginClass.getColumnIndex, jColumnName);
    if (env->ExceptionCheck()) {
        LOGE("PhotoPluginJni: call GetColumnIndex failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVILID_CODE;
    }
    return SUCCESS_CODE;
}

int PhotoCursorJni::GetColumnName(int columnIndex, std::string &columnName)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getColumnName)) {
        LOGW("PhotoCursorJni Jni get none ptr error");
        return INVILID_CODE;
    }
    jstring photoColumnName = static_cast<jstring>(env->CallObjectMethod(
        g_pluginClass.globalRef, g_pluginClass.getColumnName, columnIndex));
    columnName = env->GetStringUTFChars(photoColumnName, NULL);
    if (env->ExceptionCheck()) {
        LOGE("PhotoCursorJni JNI: call GetColumnName failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVILID_CODE;
    }
    return SUCCESS_CODE;
}

int PhotoCursorJni::GetBlob(int columnIndex, std::vector<uint8_t> &blob) {
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getBlob)) {
        LOGW("PhotoCursorJni Jni get none ptr error");
        return INVILID_CODE;
    }
    jbyteArray columnValue = static_cast<jbyteArray>(env->CallObjectMethod(
        g_pluginClass.globalRef, g_pluginClass.getBlob, columnIndex));
    jsize arrayLength = env->GetArrayLength(columnValue);
    jbyte* jstr = env->GetByteArrayElements(columnValue, NULL);
    blob.insert(blob.begin(), jstr, jstr + arrayLength * sizeof(jstr[0]));
    env->ReleaseByteArrayElements(columnValue, jstr, 0);
    env->DeleteLocalRef(columnValue);
    if (env->ExceptionCheck()) {
        LOGE("PhotoCursorJni JNI: call GetBlob failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVILID_CODE;
    }
    return SUCCESS_CODE;
}

int PhotoCursorJni::GetString(int columnIndex, std::string &value) {
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getString)) {
        LOGW("PhotoCursorJni Jni get none ptr error");
        return INVILID_CODE;
    }
    jstring columnValue = static_cast<jstring>(env->CallObjectMethod(
        g_pluginClass.globalRef, g_pluginClass.getString, columnIndex));
    value = env->GetStringUTFChars(columnValue, NULL);
    if (env->ExceptionCheck()) {
        LOGE("PhotoCursorJni JNI: call GetString failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVILID_CODE;
    }
    return SUCCESS_CODE;
}

int PhotoCursorJni::GetInt(int columnIndex, int &value) {
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getInt)) {
        LOGW("PhotoCursorJni Jni get none ptr error");
        return INVILID_CODE;
    }
    jint columnValue = env->CallIntMethod(
        g_pluginClass.globalRef, g_pluginClass.getInt, columnIndex);
    value = columnValue;
    if (env->ExceptionCheck()) {
        LOGE("PhotoCursorJni JNI: call GetInt failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVILID_CODE;
    }
    return SUCCESS_CODE;
}

int PhotoCursorJni::GetLong(int columnIndex, int64_t &value) {
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getLong)) {
        LOGW("PhotoCursorJni Jni get none ptr error");
        return INVILID_CODE;
    }
    jlong columnValue = env->CallLongMethod(
        g_pluginClass.globalRef, g_pluginClass.getLong, columnIndex);
    value = columnValue;
    if (env->ExceptionCheck()) {
        LOGE("PhotoCursorJni JNI: call GetLong failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVILID_CODE;
    }
    return SUCCESS_CODE;
}

int PhotoCursorJni::GetDouble(int columnIndex, double &value) {
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getDouble)) {
        LOGW("PhotoCursorJni Jni get none ptr error");
        return INVILID_CODE;
    }
    jdouble columnValue = env->CallDoubleMethod(
        g_pluginClass.globalRef, g_pluginClass.getDouble, columnIndex);
    value = columnValue;
    if (env->ExceptionCheck()) {
        LOGE("PhotoCursorJni JNI: call GetDouble failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVILID_CODE;
    }
    return SUCCESS_CODE;
}
} // namespace OHOS::Plugin
