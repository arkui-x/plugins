/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "web_download_item_jni.h"

#include <jni.h>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"
#include "plugins/web/webview/web_message_port.h"

using namespace OHOS::NWebError;
namespace OHOS::Plugin {
namespace {
const char WEB_WEBVIEW_CLASS_NAME[] = "ohos/ace/adapter/capability/web/AceWebPluginBase";

static const JNINativeMethod METHODS[] = {
    { "nativeInitWebDownloadItem", "()V", reinterpret_cast<void*>(WebDownloadItemJni::NativeInit) },
};

static const char METHOD_START[] = "start";
static const char METHOD_CANCEL[] = "cancel";

static const char SIGNATURE_START[] = "(JLjava/lang/String;Ljava/lang/String;)V";
static const char SIGNATURE_CANCEL[] = "(JLjava/lang/String;)V";

struct {
    jmethodID start;
    jmethodID cancel;
    jobject globalRef;
} g_webWebviewClass;
}

namespace WebItemTool {
std::string GetStringFromJNI(const std::string& funcName, const jobject& obj)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, "");
    CHECK_NULL_RETURN(obj, "");

    const jclass clazz = env->GetObjectClass(obj);
    CHECK_NULL_RETURN(clazz, "");

    jmethodID method = env->GetMethodID(clazz, funcName.c_str(), "()Ljava/lang/String;");
    CHECK_NULL_RETURN(method, "");

    jstring jsReturnString = (jstring)env->CallObjectMethod(obj, method);
    const char* cReturnString = env->GetStringUTFChars(jsReturnString, 0);
    std::string returnString(cReturnString);
    env->ReleaseStringUTFChars(jsReturnString, cReturnString);
    return returnString;
}

int GetIntFromJNI(const std::string& funcName, const jobject& obj)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, 0);
    CHECK_NULL_RETURN(obj, 0);

    const jclass clazz = env->GetObjectClass(obj);
    CHECK_NULL_RETURN(clazz, 0);

    jmethodID method = env->GetMethodID(clazz, funcName.c_str(), "()I");
    CHECK_NULL_RETURN(method, 0);

    jint returnValue = env->CallIntMethod(obj, method);
    return static_cast<int>(returnValue);
}

long GetLongFromJNI(const std::string& funcName, const jobject& obj)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, 0);
    CHECK_NULL_RETURN(obj, 0);

    const jclass clazz = env->GetObjectClass(obj);
    CHECK_NULL_RETURN(clazz, 0);

    jmethodID method = env->GetMethodID(clazz, funcName.c_str(), "()J");
    CHECK_NULL_RETURN(method, 0);

    jlong returnValue = env->CallLongMethod(obj, method);
    return static_cast<long>(returnValue);
}
}

bool WebDownloadItemJni::Register(void* env)
{
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(WEB_WEBVIEW_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    return ret;
}

void WebDownloadItemJni::NativeInit(JNIEnv* env, jobject jobj)
{
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(jobj);
    g_webWebviewClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_webWebviewClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);
    g_webWebviewClass.start = env->GetMethodID(cls, METHOD_START, SIGNATURE_START);
    g_webWebviewClass.cancel = env->GetMethodID(cls, METHOD_CANCEL, SIGNATURE_CANCEL);
    env->DeleteLocalRef(cls);
}

std::string WebDownloadItemJni::GetGuid(void* object)
{
    CHECK_NULL_RETURN(object, "");
    return WebItemTool::GetStringFromJNI("getGuid", *(jobject*)object);
}

long WebDownloadItemJni::GetCurrentSpeed(void* object)
{
    CHECK_NULL_RETURN(object, 0);
    return WebItemTool::GetLongFromJNI("getCurrentSpeed", *(jobject*)object);
}

int WebDownloadItemJni::GetPercentComplete(void* object)
{
    CHECK_NULL_RETURN(object, 0);
    return WebItemTool::GetIntFromJNI("getPercentComplete", *(jobject*)object);
}

long WebDownloadItemJni::GetTotalBytes(void* object)
{
    CHECK_NULL_RETURN(object, 0);
    return WebItemTool::GetLongFromJNI("getTotalBytes", *(jobject*)object);
}

int WebDownloadItemJni::GetState(void* object)
{
    CHECK_NULL_RETURN(object, 0);
    return WebItemTool::GetIntFromJNI("getState", *(jobject*)object);
}

int WebDownloadItemJni::GetLastErrorCode(void* object)
{
    CHECK_NULL_RETURN(object, 0);
    return WebItemTool::GetIntFromJNI("getLastErrorCode", *(jobject*)object);
}

std::string WebDownloadItemJni::GetMethod(void* object)
{
    CHECK_NULL_RETURN(object, "");
    return WebItemTool::GetStringFromJNI("getMethod", *(jobject*)object);
}

std::string WebDownloadItemJni::GetMimeType(void* object)
{
    CHECK_NULL_RETURN(object, "");
    return WebItemTool::GetStringFromJNI("getMimeType", *(jobject*)object);
}

std::string WebDownloadItemJni::GetUrl(void* object)
{
    CHECK_NULL_RETURN(object, "");
    return WebItemTool::GetStringFromJNI("getUrl", *(jobject*)object);
}

std::string WebDownloadItemJni::GetSuggestedFileName(void* object)
{
    CHECK_NULL_RETURN(object, "");
    return WebItemTool::GetStringFromJNI("getSuggestedFileName", *(jobject*)object);
}

long WebDownloadItemJni::GetReceivedBytes(void* object)
{
    CHECK_NULL_RETURN(object, 0);
    return WebItemTool::GetLongFromJNI("getReceivedBytes", *(jobject*)object);
}

std::string WebDownloadItemJni::GetFullPath(void* object)
{
    CHECK_NULL_RETURN(object, "");
    return WebItemTool::GetStringFromJNI("getFullPath", *(jobject*)object);
}

void WebDownloadItemJni::Start(int webId, const std::string& guid, const std::string& path) {
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_webWebviewClass.globalRef);
    CHECK_NULL_VOID(g_webWebviewClass.start);

    jstring jStringGuid = env->NewStringUTF(guid.c_str());
    if (!jStringGuid) {
        return;
    }
    jstring jStringPath = env->NewStringUTF(path.c_str());
    if (!jStringPath) {
        env->DeleteLocalRef(jStringGuid);
        return;
    }
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.start, webId, jStringGuid, jStringPath);
    env->DeleteLocalRef(jStringGuid);
    env->DeleteLocalRef(jStringPath);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void WebDownloadItemJni::Cancel(int webId, const std::string& guid)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_webWebviewClass.globalRef);
    CHECK_NULL_VOID(g_webWebviewClass.cancel);

    jstring jStringGuid = env->NewStringUTF(guid.c_str());
    if (!jStringGuid) {
        return;
    }
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.cancel, webId, jStringGuid);
    env->DeleteLocalRef(jStringGuid);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}
}
