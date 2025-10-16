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

#include "plugins/web/webview/android/java/jni/android_asset_helper_jni.h"

#include <jni.h>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"

namespace OHOS::Plugin {
namespace {
const char WEB_WEBVIEW_CLASS_NAME[] = "ohos/ace/plugin/webviewplugin/androidasset/AndroidAssetPlugin";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(AndroidAssetHelperJni::NativeInit) },
};
static const char METHOD_EXISTS_VIRTUAL_ASSET[] = "existsVirtualAsset";

static const char SIGNATURE_EXISTS_VIRTUAL_ASSET[] = "(Ljava/lang/String;)Z";
struct {
    jmethodID existsVirtualAsset;
    jobject globalRef;
} g_webWebviewClass;
} // namespace

bool AndroidAssetHelperJni::Register(void* env)
{
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(WEB_WEBVIEW_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("AndroidAssetHelperJni JNI: RegisterNatives fail.");
    }
    return ret;
}

void AndroidAssetHelperJni::NativeInit(JNIEnv* env, jobject jobj)
{
    CHECK_NULL_VOID(env);
    g_webWebviewClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_webWebviewClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);
    g_webWebviewClass.existsVirtualAsset = env->GetMethodID(cls, METHOD_EXISTS_VIRTUAL_ASSET,
        SIGNATURE_EXISTS_VIRTUAL_ASSET);
    env->DeleteLocalRef(cls);
}

bool AndroidAssetHelperJni::ExistsVirtualAsset(const std::string& url)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.existsVirtualAsset)) {
        LOGE("AndroidAssetHelperJni JNI: call ExistsVirtualAsset env fail");
        return false;
    }
    jstring jUrl = env->NewStringUTF(url.c_str());
    CHECK_NULL_RETURN(jUrl, false);
    jboolean isExists = env->CallBooleanMethod(g_webWebviewClass.globalRef, g_webWebviewClass.existsVirtualAsset,
        jUrl);
    if (env->ExceptionCheck()) {
        LOGE("AndroidAssetHelperJni JNI: call ExistsVirtualAsset has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(jUrl);
    return isExists;
}
} // namespace OHOS::Plugin