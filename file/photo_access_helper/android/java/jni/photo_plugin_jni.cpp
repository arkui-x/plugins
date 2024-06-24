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

#include "photo_plugin_jni.h"

#include <jni.h>
#include <string>
#include <codecvt>
#include <locale>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"
#include "plugins/file/photo_access_helper/napi/include/photo_picker_callback.h"

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
    LOGI("PhotoPluginJni: onPickerResult enter");
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
} // namespace OHOS::Plugin
