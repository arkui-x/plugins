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

#include "filepicker_jni.h"

#include "log.h"

namespace OHOS::Plugin {
namespace {
const char CLASS_NAME[] = "ohos/ace/plugin/file/fs/picker/filepicker/FilePicker";

const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(FilePickerJni::NativeInit) },
    { "onPickerResult", "(Ljava/util/List;I)V", reinterpret_cast<void*>(FilePickerJni::onPickerResult) },
};

const char METHOD_SELECT[] = "select";
const char METHOD_SAVE[] = "save";

const char SIGNATURE_SELECT[] = "(ILjava/lang/String;[Ljava/lang/String;I)V";
const char SIGNATURE_SAVE[] = "([Ljava/lang/String;Ljava/lang/String;)V";

struct {
    jmethodID select;
    jmethodID save;
    jobject globalRef;
} g_pluginClass;
} // namespace

bool FilePickerJni::Register(void* env)
{
    LOGI("FilePicker JNI: Register enter");
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);

    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("FilePicker JNI: RegisterNatives fail");
        return false;
    }
    return true;
}

void FilePickerJni::NativeInit(JNIEnv* env, jobject jobj)
{
    LOGI("FilePicker JNI: NativeInit enter");
    CHECK_NULL_VOID(env);
    g_pluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_pluginClass.save = env->GetMethodID(cls, METHOD_SAVE, SIGNATURE_SAVE);
    CHECK_NULL_VOID(g_pluginClass.save);
    g_pluginClass.select = env->GetMethodID(cls, METHOD_SELECT, SIGNATURE_SELECT);
    CHECK_NULL_VOID(g_pluginClass.select);

    env->DeleteLocalRef(cls);
}

std::vector<std::string> jstringListToStdStringVector(JNIEnv* env, jobject list)
{
    std::vector<std::string> result;
    jclass listClass = env->FindClass("java/util/List");
    jmethodID sizeMethod = env->GetMethodID(listClass, "size", "()I");
    jmethodID getMethod = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");
    jsize listSize = env->CallIntMethod(list, sizeMethod);

    for (jsize i = 0; i < listSize; ++i) {
        jobject element = env->CallObjectMethod(list, getMethod, i);
        auto jstr = (jstring)element;
        const char* str = env->GetStringUTFChars(jstr, nullptr);
        result.emplace_back(str);
        env->ReleaseStringUTFChars(jstr, str);
    }

    return result;
}

void FilePickerJni::onPickerResult(JNIEnv* env, jobject thiz, jobject rst, jint errCode)
{
    LOGI("FilePicker JNI: onPickerResult enter");
    CHECK_NULL_VOID(env);
    std::vector<std::string> cppStringList = jstringListToStdStringVector(env, rst);
    DocumentFilePicker::onPickerResult(cppStringList, errCode);
}

void FilePickerJni::Select(DocumentSelectOptions& options)
{
    LOGI("FilePicker JNI: Select enter");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    jstring defaultFilePathUri = env->NewStringUTF(options.defaultFilePathUri.c_str());

    jobjectArray fileSuffixFilters =
        env->NewObjectArray(options.fileSuffixFilters.size(), env->FindClass("java/lang/String"), nullptr);
    for (size_t i = 0; i < options.fileSuffixFilters.size(); ++i) {
        env->SetObjectArrayElement(fileSuffixFilters, i, env->NewStringUTF(options.fileSuffixFilters[i].c_str()));
    }

    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.select, options.maxSelectNumber, defaultFilePathUri,
        fileSuffixFilters, options.key_select_mode);
    env->DeleteLocalRef(defaultFilePathUri);
    env->DeleteLocalRef(fileSuffixFilters);
}

void FilePickerJni::Save(DocumentSaveOptions& options)
{
    LOGI("FilePicker JNI: Save enter");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    jstring defaultFilePathUri = env->NewStringUTF(options.defaultFilePathUri.c_str());
    jobjectArray newFileNames =
        env->NewObjectArray(options.newFileNames.size(), env->FindClass("java/lang/String"), nullptr);
    for (size_t i = 0; i < options.newFileNames.size(); ++i) {
        env->SetObjectArrayElement(newFileNames, i, env->NewStringUTF(options.newFileNames[i].c_str()));
    }

    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.save, newFileNames, defaultFilePathUri);
    env->DeleteLocalRef(defaultFilePathUri);
}
} // namespace OHOS::Plugin
