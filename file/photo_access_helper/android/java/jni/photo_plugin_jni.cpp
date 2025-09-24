/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "photo_plugin_jni.h"

#include <jni.h>
#include <string>
#include <codecvt>
#include <locale>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"
#include "plugins/file/photo_access_helper/napi/photo/photopicker/include/photo_picker_callback.h"

namespace OHOS::Plugin {
namespace {
const char PLUGIN_CLASS_NAME[] = "ohos/ace/plugin/photoaccesshelper/PhotoPlugin";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(PhotoPluginJni::NativeInit) },
    { "onPickerResult", "(Ljava/util/List;I)V", reinterpret_cast<void*>(PhotoPluginJni::onPickerResult) },
};

const char METHOD_START_PHOTO_PICKER[] = "startPhotoPicker";

const char SIGNATURE_START_PHOTO_PICKER[] = "(Ljava/lang/String;)V";

struct {
    jmethodID startPhotoPicker;
    jmethodID queryPhoto;
    jmethodID queryAlbum;
    jmethodID checkPermission;
    jmethodID checkWritePermission;
    jmethodID createPhoto;
    jmethodID getMimeTypeFromExtension;
    jobject globalRef;
} g_pluginClass;
} // namespace

bool PhotoPluginJni::Register(void* env)
{
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(PLUGIN_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("PhotoPluginJni JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void PhotoPluginJni::NativeInit(JNIEnv* env, jobject jobj)
{
    CHECK_NULL_VOID(env);
    g_pluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_pluginClass.startPhotoPicker =
        env->GetMethodID(cls, METHOD_START_PHOTO_PICKER, SIGNATURE_START_PHOTO_PICKER);
    CHECK_NULL_VOID(g_pluginClass.startPhotoPicker);

    g_pluginClass.queryPhoto = env->GetMethodID(cls, "queryPhoto",
    "(Landroid/os/Bundle;[Ljava/lang/String;[Ljava/lang/String;)Landroid/database/Cursor;");
    CHECK_NULL_VOID(g_pluginClass.queryPhoto);

    g_pluginClass.queryAlbum = env->GetMethodID(cls, "queryAlbum",
    "(Landroid/os/Bundle;[Ljava/lang/String;[Ljava/lang/String;)Lohos/ace/plugin/photoaccesshelper/AlbumValues;");
    CHECK_NULL_VOID(g_pluginClass.queryAlbum);

    g_pluginClass.checkPermission = env->GetMethodID(cls, "checkPermission", "()Z");
    CHECK_NULL_VOID(g_pluginClass.checkPermission);

    g_pluginClass.checkWritePermission = env->GetMethodID(cls, "checkWritePermission", "()Z");
    CHECK_NULL_VOID(g_pluginClass.checkWritePermission);

    g_pluginClass.createPhoto = env->GetMethodID(cls, "createPhoto",
        "(ILjava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
    CHECK_NULL_VOID(g_pluginClass.createPhoto);

    g_pluginClass.getMimeTypeFromExtension = env->GetMethodID(cls, "getMimeTypeFromExtension",
        "(Ljava/lang/String;)Ljava/lang/String;");
    CHECK_NULL_VOID(g_pluginClass.getMimeTypeFromExtension);

    env->DeleteLocalRef(cls);
}

static jstring StringToJavaString(JNIEnv* env, const std::string& string)
{
    std::u16string str =
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(string);
    return env->NewString(reinterpret_cast<const jchar*>(str.data()), str.length());
}

static std::vector<std::string> jstringListToStdStringVector(JNIEnv *env, jobject list) {
    std::vector<std::string> result;
    jclass listClass = env->FindClass("java/util/List");
    jmethodID sizeMethod = env->GetMethodID(listClass, "size", "()I");
    jmethodID getMethod = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");
    jsize listSize = env->CallIntMethod(list, sizeMethod);

    for (jsize i = 0; i < listSize; ++i) {
        jobject element = env->CallObjectMethod(list, getMethod, i);
        jstring jstr = (jstring)element;
        const char *str = env->GetStringUTFChars(jstr, nullptr);
        result.push_back(str);
        env->ReleaseStringUTFChars(jstr, str);
    }

    return result;
}

void PhotoPluginJni::startPhotoPicker(std::string &type) {
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.startPhotoPicker)) {
        LOGW("PhotoPluginJni get none ptr error");
        return;
    }
    jstring jType = StringToJavaString(env, type);
    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.startPhotoPicker, jType);
    if (env->ExceptionCheck()) {
        LOGE("PhotoPluginJni: call startPhotoPicker failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void PhotoPluginJni::onPickerResult(JNIEnv* env, jobject thiz, jobject rst, jint errorCode)
{
    CHECK_NULL_VOID(env);
    std::vector<std::string> cppStringList = jstringListToStdStringVector(env, rst);
    std::shared_ptr<OHOS::Media::PickerCallBack> photoPickerCallback
        = OHOS::Media::PhotoPickerCallback::pickerCallBack;
    if (photoPickerCallback != nullptr) {
        LOGI("PhotoPluginJni: photoPickerCallback enter");
        photoPickerCallback->resultCode = errorCode;
        photoPickerCallback->uris = cppStringList;
        if (!cppStringList.empty()) {
            photoPickerCallback->isOrigin = true;
        }
        photoPickerCallback->ready = true;
    }
}

static jobjectArray ListToJavaList(JNIEnv* env, std::vector<std::string> &array) {
    jclass jclz = env->FindClass("java/lang/String");
    jobjectArray jArray = env->NewObjectArray(array.size(), jclz, NULL);
    for (size_t i = 0; i < array.size(); i++) {
        jstring jStr = StringToJavaString(env, array[i]);
        env->SetObjectArrayElement(jArray, i, jStr);
    }
    return jArray;
}

static jobject getSelectionBundle(JNIEnv* env, const NativeRdb::RdbPredicates &predicates,
    std::vector<std::string> &selectionArgs) {
    jclass jclazz = env->FindClass("android/os/Bundle");
    jmethodID init = env->GetMethodID(jclazz, "<init>", "()V");
    jobject bundle = env->NewObject(jclazz, init);
    jmethodID jSetStringID =env->GetMethodID(jclazz, "putString", "(Ljava/lang/String;Ljava/lang/String;)V");
    CHECK_NULL_RETURN(jSetStringID, bundle);
    if (!predicates.GetWhereClause().empty()) {
        jstring where = StringToJavaString(env, predicates.GetWhereClause());
        jstring whereKey = StringToJavaString(env, "android:query-arg-sql-selection");
        env->CallVoidMethod(bundle, jSetStringID, whereKey, where);
    }
    if (!predicates.GetGroup().empty()) {
        jstring groupBy = StringToJavaString(env, predicates.GetGroup());
        jstring key = StringToJavaString(env, "android:query-arg-sql-group-by");
        env->CallVoidMethod(bundle, jSetStringID, key, groupBy);
    }

    std::string limitStr =
        (predicates.GetLimit() == NativeRdb::AbsPredicates::INIT_LIMIT_VALUE) ? "" : std::to_string(predicates.GetLimit());
    if (!limitStr.empty()) {
        jstring jLimit = StringToJavaString(env, limitStr);
        jstring key = StringToJavaString(env, "android:query-arg-sql-limit");
        env->CallVoidMethod(bundle, jSetStringID, key, jLimit);
    }

    std::string offsetStr =
        (predicates.GetOffset() == NativeRdb::AbsPredicates::INIT_OFFSET_VALUE) ? "" : std::to_string(predicates.GetOffset());
    if (!offsetStr.empty()) {
        jstring jOffset = StringToJavaString(env, offsetStr);
        jstring key = StringToJavaString(env, "android:query-arg-offset");
        env->CallVoidMethod(bundle, jSetStringID, key, jOffset);
    }

    if (!predicates.GetOrder().empty()) {
        jstring jOrder = StringToJavaString(env, predicates.GetOrder());
        jstring key = StringToJavaString(env, "android:query-arg-sql-sort-order");
        env->CallVoidMethod(bundle, jSetStringID, key, jOrder);
    }

    if (!selectionArgs.empty()) {
        jmethodID jSetStringArrayID =env->GetMethodID(jclazz, "putStringArray",
            "(Ljava/lang/String;[Ljava/lang/String;)V");
        jobject jArgs = ListToJavaList(env, selectionArgs);
        jstring key = StringToJavaString(env, "android:query-arg-sql-selection-args");
        env->CallVoidMethod(bundle, jSetStringArrayID, key, jArgs);
    }
    return bundle;
}

std::shared_ptr<Media::ResultSet> PhotoPluginJni::queryPhoto(const NativeRdb::RdbPredicates &queryArgs,
    std::vector<std::string> &selectionArgs, std::vector<std::string> &projection)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.queryPhoto)) {
        LOGW("queryPhoto get none ptr error");
        return nullptr;
    }
    jobject jQueryArgs = getSelectionBundle(env, queryArgs, selectionArgs);
    jobjectArray jSelectionArgs = ListToJavaList(env, selectionArgs);
    jobjectArray jProjection = ListToJavaList(env, projection);

    jobject result = env->CallObjectMethod(g_pluginClass.globalRef,
        g_pluginClass.queryPhoto, jQueryArgs, jSelectionArgs, jProjection);
    std::shared_ptr<Media::PhotoResultSet> resultSet
        = std::make_shared<Media::PhotoResultSet>(env, result);
    if (env->ExceptionCheck()) {
        LOGE("PhotoPluginJni: call queryPhoto failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return resultSet;
}

std::shared_ptr<Media::ResultSet> PhotoPluginJni::queryAlbum(const NativeRdb::RdbPredicates &queryArgs,
    std::vector<std::string> &selectionArgs, std::vector<std::string> &projection)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.queryAlbum)) {
        LOGW("queryAlbum get none ptr error");
        return nullptr;
    }
    jobject jQueryArgs = getSelectionBundle(env, queryArgs, selectionArgs);
    jobjectArray jSelectionArgs = ListToJavaList(env, selectionArgs);
    jobjectArray jProjection = ListToJavaList(env, projection);

    jobject result = env->CallObjectMethod(g_pluginClass.globalRef,
        g_pluginClass.queryAlbum, jQueryArgs, jSelectionArgs, jProjection);
    std::shared_ptr<Media::AlbumResultSet> resultSet = std::make_shared<Media::AlbumResultSet>(env, result);
    if (env->ExceptionCheck()) {
        LOGE("PhotoPluginJni: call queryPhoto failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return resultSet;
}

std::string PhotoPluginJni::CreatePhoto(int photoType, const std::string &extension, const std::string &title)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    std::string result = "";

    if (!(env) || !g_pluginClass.globalRef || !g_pluginClass.createPhoto) {
        LOGE("CreatePhoto get none ptr error");
        return result;
    }

    jint jPhotoType = photoType;
    jstring jExtension = StringToJavaString(env, extension);
    jstring jTitle = StringToJavaString(env, title);

    jstring jOutUri = static_cast<jstring>(env->CallObjectMethod(
        g_pluginClass.globalRef, g_pluginClass.createPhoto, jPhotoType, jExtension, jTitle));
    if (jOutUri != nullptr) {
        const char* uriStr = env->GetStringUTFChars(jOutUri, nullptr);
        if (uriStr != nullptr) {
            result = uriStr;
            env->ReleaseStringUTFChars(jOutUri, uriStr);
        } else {
            LOGE("CreatePhoto GetStringUTFChars failed");
        }
        env->DeleteLocalRef(jOutUri);
    }

    if (env->ExceptionCheck()) {
        LOGE("Java exception occurred in CreatePhoto");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(jExtension);
    env->DeleteLocalRef(jTitle);
    return result;
}

bool PhotoPluginJni::checkPermission() {
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.checkPermission)) {
        LOGW("checkPermission get none ptr error");
        return false;
    }
    jboolean result = env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.checkPermission);
    if (env->ExceptionCheck()) {
        LOGE("checkPermission: call queryPhoto failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return result;
}

bool PhotoPluginJni::CheckWritePermission()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env || !g_pluginClass.globalRef || !g_pluginClass.checkWritePermission) {
        LOGE("checkWritePermission get none ptr error");
        return false;
    }
    jboolean result = env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.checkWritePermission);
    if (env->ExceptionCheck()) {
        LOGE("checkWritePermission: call createAsset failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return result;
}

std::string PhotoPluginJni::GetMimeTypeFromExtension(const std::string &extension)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    std::string result = "";

    if (!env || !g_pluginClass.globalRef || !g_pluginClass.getMimeTypeFromExtension) {
        LOGE("GetMimeTypeFromExtension get none ptr error");
        return result;
    }

    jstring jExtension = StringToJavaString(env, extension);
    if (jExtension == nullptr) {
        LOGE("GetMimeTypeFromExtension jExtension is null");
        return result;
    }

    jstring jMimeType = static_cast<jstring>(env->CallObjectMethod(
        g_pluginClass.globalRef, g_pluginClass.getMimeTypeFromExtension, jExtension));
    if (jMimeType != nullptr) {
        const char* mimeType = env->GetStringUTFChars(jMimeType, nullptr);
        if (mimeType != nullptr) {
            result = mimeType;
            env->ReleaseStringUTFChars(jMimeType, mimeType);
        } else {
            LOGE("GetMimeTypeFromExtension GetStringUTFChars failed");
        }
        env->DeleteLocalRef(jMimeType);
    }
    
    if (env->ExceptionCheck()) {
        LOGE("Java exception occurred in GetMimeTypeFromExtension");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(jExtension);
    return result;
}
} // namespace OHOS::Plugin
