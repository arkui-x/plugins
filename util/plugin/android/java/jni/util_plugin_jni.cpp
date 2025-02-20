/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "plugins/util/plugin/android/java/jni/util_plugin_jni.h"

#include <jni.h>
#include <string>
#include <locale>
#include <codecvt>

#include "log.h"
#include "plugin_utils.h"
#include "inner_api/plugin_utils_inner.h"

namespace OHOS::Plugin {
namespace {
const char UTIL_PLUGIN_CLASS_NAME[] = "ohos/ace/plugin/utilplugin/UtilPlugin";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(UtilPluginJni::NativeInit) },
};

const char METHOD_ENCODE_INTO_CHINESE[] = "encodeIntoChinese";

const char SIGNATURE_ENCODE_INTO_CHINESE[] = "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;";

const char METHOD_DECODE[] = "decode";

const char SIGNATURE_DECODE[] = "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;";

struct {
    jmethodID encodeIntoChinese;
    jmethodID decode;
    jobject globalRef;
} g_pluginClass;
} // namespace

bool UtilPluginJni::Register(void* env)
{
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(UTIL_PLUGIN_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("UtilPlugin JNI: RegisterNatives fail.");
        return false;
    }
    return ret;
}

void UtilPluginJni::NativeInit(JNIEnv* env, jobject jobj)
{
    CHECK_NULL_VOID(env);
    g_pluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_pluginClass.encodeIntoChinese = env->GetMethodID(cls, METHOD_ENCODE_INTO_CHINESE, SIGNATURE_ENCODE_INTO_CHINESE);
    CHECK_NULL_VOID(g_pluginClass.encodeIntoChinese);

    g_pluginClass.decode = env->GetMethodID(cls, METHOD_DECODE, SIGNATURE_DECODE);
    CHECK_NULL_VOID(g_pluginClass.decode);

    env->DeleteLocalRef(cls);
}

static jstring StringToJavaString(JNIEnv* env, const std::string &string)
{
    std::u16string str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(string);
    return env->NewString(reinterpret_cast<const jchar *>(str.data()), str.length());
}

static jstring charToLatin1JavaString(JNIEnv* env, const char* charData)
{
    jclass strClass = env->FindClass("java/lang/String");
    jmethodID ctorID = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    jbyteArray bytes = env->NewByteArray(strlen(charData));
    env->SetByteArrayRegion(bytes, 0, strlen(charData), (jbyte*) charData);
    jstring encoding = env->NewStringUTF("latin1");
    return (jstring) env->NewObject(strClass, ctorID, bytes, encoding);
}

std::string hexTostring(const std::string& str)
{
    std::string result = "";
    for (size_t i = 0; i < str.length(); i += 2) {   // 2: two hex characters combined to one character
        std::string byte = str.substr(i, 2);
        char chr = (char)(int)strtol(byte.c_str(), NULL, 16);
        if (chr == NULL) {
            LOGW("UtilPluginJni convert string failed");
        }
        result.push_back(chr);
    }
    return result;
}

std::string UtilPluginJni::EncodeIntoChinese(std::string input, std::string encoding)
{
    jstring result;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.encodeIntoChinese)) {
        LOGW("UtilPluginJni get none ptr error");
        return "";
    }

    jstring jinput = StringToJavaString(env, input);
    jstring jencoding = StringToJavaString(env, encoding);

    result = static_cast<jstring>(
        env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.encodeIntoChinese, jinput, jencoding));
    if (env->ExceptionCheck()) {
        LOGE("Util JNI: call encodeIntoChinese failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    std::string hexstr = env->GetStringUTFChars(result, NULL);
    std::string output = hexTostring(hexstr);
    return output;
}

std::string UtilPluginJni::Decode(std::string input, std::string encoding)
{
    jstring jresult;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.decode)) {
        LOGW("UtilPluginJni get none ptr error");
        return "";
    }

    const char* inputData = input.data();
    const char* encodingData = encoding.data();
    
    jstring jinput = charToLatin1JavaString(env, inputData);
    jstring jencoding = charToLatin1JavaString(env, encodingData);
    
    jresult = static_cast<jstring>(
        env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.decode, jinput, jencoding));
    if (env->ExceptionCheck()) {
        LOGE("Util JNI: call decode failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    std::string result = env->GetStringUTFChars(jresult, NULL);
    return result;
}
} // namespace OHOS::Plugin
