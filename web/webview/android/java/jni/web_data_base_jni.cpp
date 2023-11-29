/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "plugins/web/webview/android/java/jni/web_data_base_jni.h"

#include <jni.h>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"

using namespace OHOS::NWebError;

namespace OHOS::Plugin {
namespace {
const char WEB_WEBVIEW_CLASS_NAME[] = "ohos/ace/adapter/capability/web/AceWebPluginBase";

static const JNINativeMethod METHODS[] = {
    { "nativeInitWebDataBase", "()V", reinterpret_cast<void*>(WebDataBaseJni::NativeInit) },
};
static const char METHOD_EXIST_HTTP_AUTH_CREDENTIALS[] = "existHttpAuthCredentials";
static const char METHOD_DELETE_HTTP_AUTH_CREDENTIALS[] = "deleteHttpAuthCredentials";
static const char METHOD_SAVE_HTTP_AUTH_CREDENTIALS[] = "saveHttpAuthCredentials";
static const char METHOD_GET_HTTP_AUTH_CREDENTIALS[] = "getHttpAuthCredentials";

static const char SIGNATURE_EXIST[] = "()Z";
static const char SIGNATURE_DELETE[] = "()V";
static const char SIGNATURE_SAVE[] = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V";
static const char SIGNATURE_GET[] = "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;";
struct {
    jmethodID existHttpAuthCredentials;
    jmethodID deleteHttpAuthCredentials;
    jmethodID saveHttpAuthCredentials;
    jmethodID getHttpAuthCredentials;
    jobject globalRef;
} g_webWebviewClass;
}

bool WebDataBaseJni::Register(void* env)
{
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(WEB_WEBVIEW_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("WebDataBaseJni JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void WebDataBaseJni::NativeInit(JNIEnv* env, jobject jobj)
{
    LOGI("WebDataBaseJni JNI: NativeInit");
    CHECK_NULL_VOID(env);
    g_webWebviewClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_webWebviewClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);
    g_webWebviewClass.existHttpAuthCredentials = env->GetMethodID(cls, METHOD_EXIST_HTTP_AUTH_CREDENTIALS,
        SIGNATURE_EXIST);
    CHECK_NULL_VOID(g_webWebviewClass.existHttpAuthCredentials);
    g_webWebviewClass.deleteHttpAuthCredentials = env->GetMethodID(cls, METHOD_DELETE_HTTP_AUTH_CREDENTIALS,
        SIGNATURE_DELETE);
    CHECK_NULL_VOID(g_webWebviewClass.deleteHttpAuthCredentials);
    g_webWebviewClass.saveHttpAuthCredentials = env->GetMethodID(cls, METHOD_SAVE_HTTP_AUTH_CREDENTIALS,
    SIGNATURE_SAVE);
    CHECK_NULL_VOID(g_webWebviewClass.saveHttpAuthCredentials);
    g_webWebviewClass.getHttpAuthCredentials = env->GetMethodID(cls, METHOD_GET_HTTP_AUTH_CREDENTIALS,
    SIGNATURE_GET);
    CHECK_NULL_VOID(g_webWebviewClass.getHttpAuthCredentials);
    env->DeleteLocalRef(cls);
}

bool WebDataBaseJni::ExistHttpAuthCredentials()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, INIT_ERROR);
    jboolean isExist = env->CallBooleanMethod(g_webWebviewClass.globalRef, g_webWebviewClass.existHttpAuthCredentials);
    if (env->ExceptionCheck()) {
        LOGE("WebDataBaseJni JNI: call ExistHttpAuthCredentials has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return isExist;
}

ErrCode WebDataBaseJni::DeleteHttpAuthCredentials()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, INIT_ERROR);
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.deleteHttpAuthCredentials);
    if (env->ExceptionCheck()) {
        LOGE("WebDataBaseJni JNI: call DeleteHttpAuthCredentials has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INIT_ERROR;
    }
    return NO_ERROR;
}

ErrCode WebDataBaseJni::SaveHttpAuthCredentials(const std::string& host, const std::string& realm,
    const std::string& username, const char* password)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, INIT_ERROR);
    jstring jHost = env->NewStringUTF(host.c_str());
    CHECK_NULL_RETURN(jHost, INIT_ERROR);
    jstring jRealm = env->NewStringUTF(realm.c_str());
    CHECK_NULL_RETURN(jRealm, INIT_ERROR);
    jstring jUsername = env->NewStringUTF(username.c_str());
    CHECK_NULL_RETURN(jUsername, INIT_ERROR);
    jstring jPassword = env->NewStringUTF(password);
    CHECK_NULL_RETURN(jPassword, INIT_ERROR);
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.saveHttpAuthCredentials, jHost, jRealm,
        jUsername, jPassword);
    if (env->ExceptionCheck()) {
        LOGE("WebDataBaseJni JNI: call SaveHttpAuthCredentials has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INIT_ERROR;
    }
    return NO_ERROR;
}

ErrCode WebDataBaseJni::GetHttpAuthCredentials(const std::string& host, const std::string& realm,
    std::string& username, char* password, uint32_t passwordSize)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, INIT_ERROR);
    jstring jHost = env->NewStringUTF(host.c_str());
    CHECK_NULL_RETURN(jHost, INIT_ERROR);
    jstring jRealm = env->NewStringUTF(realm.c_str());
    CHECK_NULL_RETURN(jRealm, INIT_ERROR);
    jobject jResult = env->CallObjectMethod(g_webWebviewClass.globalRef, g_webWebviewClass.getHttpAuthCredentials, jHost, jRealm);
    if (env->ExceptionCheck()) {
        LOGE("WebDataBaseJni JNI: call GetHttpAuthCredentials has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INIT_ERROR;
    }
    jclass cls = env->GetObjectClass(jResult);
    CHECK_NULL_RETURN(cls, INIT_ERROR);
    username = GetName(env, cls, jResult);
    std::string passwordResult = GetPassword(env, cls, jResult);
    if (passwordResult.size() > passwordSize) {
        passwordResult = passwordResult.substr(0, passwordSize);
    }
    strcpy(password, passwordResult.c_str());
    return NO_ERROR;
}

std::string WebDataBaseJni::GetName(JNIEnv* env, jclass cls, jobject jobj)
{
    std::string result;
    jmethodID getUsername = env->GetMethodID(cls, "getUsername", "()Ljava/lang/String;");
    CHECK_NULL_RETURN(getUsername, result);
    jstring jResult = static_cast<jstring>(env->CallObjectMethod(jobj, getUsername));
    const char* content = env->GetStringUTFChars(jResult, nullptr);
    if (content != nullptr) {
        result.assign(content);
        env->ReleaseStringUTFChars(jResult, content);
    }
    if (jResult != nullptr) {
        env->DeleteLocalRef(jResult);
    }
    return result;
}

std::string WebDataBaseJni::GetPassword(JNIEnv* env, jclass cls, jobject jobj)
{
    std::string result;
    jmethodID getPassword = env->GetMethodID(cls, "getPassword", "()Ljava/lang/String;");
    CHECK_NULL_RETURN(getPassword, result);
    jstring jResult = static_cast<jstring>(env->CallObjectMethod(jobj, getPassword));
    const char* content = env->GetStringUTFChars(jResult, nullptr);
    if (content != nullptr) {
        result.assign(content);
        env->ReleaseStringUTFChars(jResult, content);
    }
    if (jResult != nullptr) {
        env->DeleteLocalRef(jResult);
    }
    return result;
}
}