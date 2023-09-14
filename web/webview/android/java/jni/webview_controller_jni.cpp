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

#include "plugins/web/webview/android/java/jni/webview_controller_jni.h"

#include <jni.h>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"

using namespace OHOS::NWebError;

namespace OHOS::Plugin {
namespace {
const char WEB_WEBVIEW_CLASS_NAME[] = "ohos/ace/adapter/capability/web/AceWebPluginBase";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(WebviewControllerJni::NativeInit) },
};
static const char METHOD_LOADURL[] = "loadUrl";

static const char SIGNATURE_LOADURL[] = "(JLjava/lang/String;Ljava/util/HashMap;)V";
struct {
    jmethodID loadUrl;
    jobject globalRef;
} g_webWebviewClass;
}

bool WebviewControllerJni::Register(void* env)
{
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(WEB_WEBVIEW_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("WebviewControllerJni JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void WebviewControllerJni::NativeInit(JNIEnv* env, jobject jobj)
{
    LOGI("WebviewControllerJni JNI: NativeInit");
    CHECK_NULL_VOID(env);
    g_webWebviewClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_webWebviewClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);
    g_webWebviewClass.loadUrl = env->GetMethodID(cls, METHOD_LOADURL, SIGNATURE_LOADURL);
    CHECK_NULL_VOID(g_webWebviewClass.loadUrl);
    env->DeleteLocalRef(cls);
}

ErrCode WebviewControllerJni::LoadUrl(int id, const std::string& url, 
    const std::map<std::string, std::string>& httpHeaders)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, INIT_ERROR);
    jstring jUrl = env->NewStringUTF(url.c_str());
    CHECK_NULL_RETURN(jUrl, INIT_ERROR);
    if (httpHeaders.empty()) {
        env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.loadUrl, id, jUrl, nullptr);
    } else {
        jclass mapClass = env->FindClass("java/util/HashMap");
        CHECK_NULL_RETURN(mapClass, INIT_ERROR);
        jmethodID init = env->GetMethodID(mapClass, "<init>", "()V");
        CHECK_NULL_RETURN(init, INIT_ERROR);
        jmethodID put = env->GetMethodID(mapClass, "put",
            "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
        CHECK_NULL_RETURN(put, INIT_ERROR);
        jobject httpHeaderMap = env->NewObject(mapClass, init);
        CHECK_NULL_RETURN(httpHeaderMap, INIT_ERROR);
        std::map<std::string, std::string>::const_iterator iter = httpHeaders.begin();
        for (; iter != httpHeaders.end(); ++iter) {
            jstring key = env->NewStringUTF(iter->first.c_str());
            jstring value = env->NewStringUTF(iter->second.c_str());
            CHECK_NULL_RETURN(key, INIT_ERROR);
            CHECK_NULL_RETURN(value, INIT_ERROR);
            env->CallObjectMethod(httpHeaderMap, put, key, value);
            env->DeleteLocalRef(key);
            env->DeleteLocalRef(value);
        }
        env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.loadUrl, id, jUrl, httpHeaderMap);
        env->DeleteLocalRef(httpHeaderMap);
    }
    if (env->ExceptionCheck()) {
        LOGE("WebviewControllerJni JNI: call loadUrl has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteLocalRef(jUrl);
        return INIT_ERROR;
    }
    env->DeleteLocalRef(jUrl);
    return NO_ERROR;
}
}