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

#include "plugins/net/http/android/java/jni/http_jni.h"

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"

namespace OHOS::NetStack::Http {
namespace {
const char HTTP_CLASS_NAME[] = "ohos/ace/plugin/httpplugin/HttpPlugin";
const char ERROR[] = "jni error";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(HttpJni::NativeInit) },
};

static const char JAVA_GETCACHEDIR[] = "getCacheDir";
static const char JAVA_GETCACHEDIR_PARAM[] = "()Ljava/lang/String;";
struct {
    jmethodID getCacheDir_;
    jobject globalRef_;
} g_pluginClass;
} // namespace

bool HttpJni::Register(void* env)
{
    LOGI("HttpJni JNI: Register");
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(HTTP_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("HttpJni JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void HttpJni::NativeInit(JNIEnv* env, jobject jobj)
{
    LOGI("HttpJni JNI: NativeInit");
    CHECK_NULL_VOID(env);
    g_pluginClass.globalRef_ = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_pluginClass.globalRef_);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_pluginClass.getCacheDir_ = env->GetMethodID(cls, JAVA_GETCACHEDIR, JAVA_GETCACHEDIR_PARAM);
    CHECK_NULL_VOID(g_pluginClass.getCacheDir_);

    env->DeleteLocalRef(cls);
}

std::string HttpJni::GetCacheDirJni()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ERROR);
    CHECK_NULL_RETURN(g_pluginClass.globalRef_, ERROR);
    CHECK_NULL_RETURN(g_pluginClass.getCacheDir_, ERROR);
    jstring jCacheDir =
        static_cast<jstring>(env->CallObjectMethod(g_pluginClass.globalRef_, g_pluginClass.getCacheDir_));
    std::string cacheDir;
    auto cacheDirStr = env->GetStringUTFChars(jCacheDir, nullptr);
    if (cacheDirStr != nullptr) {
        cacheDir = cacheDirStr;
        env->ReleaseStringUTFChars(jCacheDir, cacheDirStr);
    }
    return cacheDir;
}
} // OHOS::NetStack::Http