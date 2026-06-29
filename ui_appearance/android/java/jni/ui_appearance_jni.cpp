/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plugins/ui_appearance/android/java/jni/ui_appearance_jni.h"

#include "log.h"
#include "plugin_utils.h"
#include "inner_api/plugin_utils_inner.h"

namespace OHOS::Plugin {
namespace {
const char UI_APPEARANCE_CLASS_NAME[] = "ohos/ace/plugin/uiappearance/UiAppearancePlugin";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(UiAppearanceJni::NativeInit) },
};

struct {
    jmethodID getDarkMode;
    jmethodID getFontScale;
    jobject globalRef;
} g_pluginClass;
} // namespace

bool UiAppearanceJni::Register(void* env)
{
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(UI_APPEARANCE_CLASS_NAME);
    if (jniEnv->ExceptionCheck()) {
        LOGE("UiAppearanceJni: Register has exception %{public}s", UI_APPEARANCE_CLASS_NAME);
        jniEnv->ExceptionDescribe();
        jniEnv->ExceptionClear();
        return false;
    }
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("UiAppearanceJni: RegisterNatives fail");
        return false;
    }
    return true;
}

void UiAppearanceJni::NativeInit(JNIEnv* env, jobject jobj)
{
    CHECK_NULL_VOID(env);
    g_pluginClass.globalRef = env->NewGlobalRef(jobj);
    if (env->ExceptionCheck()) {
        LOGW("UiAppearanceJni: NewGlobalRef has exception");
        env->ExceptionClear();
    }
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_pluginClass.getDarkMode = env->GetMethodID(cls, "getDarkMode", "()I");
    if (env->ExceptionCheck()) {
        LOGW("UiAppearanceJni: no getDarkMode method");
        env->ExceptionClear();
    }

    g_pluginClass.getFontScale = env->GetMethodID(cls, "getFontScale", "()D");
    if (env->ExceptionCheck()) {
        LOGW("UiAppearanceJni: no getFontScale method");
        env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

int32_t UiAppearanceJni::GetDarkMode()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, 1);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, 1);
    CHECK_NULL_RETURN(g_pluginClass.getDarkMode, 1);

    int32_t result = env->CallIntMethod(g_pluginClass.globalRef, g_pluginClass.getDarkMode);
    if (env->ExceptionCheck()) {
        LOGE("UiAppearanceJni: getDarkMode has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return 1;
    }
    return result;
}

double UiAppearanceJni::GetFontScale()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, 1.0);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, 1.0);
    CHECK_NULL_RETURN(g_pluginClass.getFontScale, 1.0);

    double result = env->CallDoubleMethod(g_pluginClass.globalRef, g_pluginClass.getFontScale);
    if (env->ExceptionCheck()) {
        LOGE("UiAppearanceJni: getFontScale has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return 1.0;
    }
    return result;
}

} // namespace OHOS::Plugin
