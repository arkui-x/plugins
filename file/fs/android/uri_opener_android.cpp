/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0
 */

#include <jni.h>
#include "uri_opener.h"

#include <cerrno>
#include <unistd.h>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace {
constexpr char FS_URI_OPENER_CLASS[] = "ohos/ace/plugin/fsuriopener/FsUriOpenerPlugin";
constexpr char OPEN_FD_METHOD[] = "openFd";
constexpr char OPEN_FD_SIGNATURE[] = "(Ljava/lang/String;I)I";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(NativeInit) },
};

struct {
    jmethodID openFd;
    jobject globalRef;
} g_plugin;
} // namespace

void NativeInit(JNIEnv* env, jobject jobj)
{
    g_plugin.globalRef = env->NewGlobalRef(jobj);
    jclass cls = env->GetObjectClass(jobj);
    g_plugin.openFd = env->GetMethodID(cls, OPEN_FD_METHOD, OPEN_FD_SIGNATURE);
    env->DeleteLocalRef(cls);
    if (g_plugin.openFd == nullptr) {
        LOGE("NativeInit: GetMethodID openFd failed");
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
    }
}

bool RegisterFsUriOpenerJavaPlugin(void* env)
{
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(FS_URI_OPENER_CLASS);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    return ret;
}

PlatformFdResult OpenByPlatformNative(const std::string& uri, unsigned int mode)
{
    PlatformFdResult result;
    auto* env = ARKUI_X_Plugin_GetJniEnv();
    if (env == nullptr) {
        LOGE("OpenByPlatformNative: jni env is null");
        return result;
    }
    if (g_plugin.globalRef == nullptr || g_plugin.openFd == nullptr) {
        LOGE("OpenByPlatformNative: plugin not initialized (globalRef=%p openFd=%p)",
             g_plugin.globalRef, g_plugin.openFd);
        return result;
    }
    jstring jUri = env->NewStringUTF(uri.c_str());
    if (jUri == nullptr) {
        return result;
    }
    jint fd = env->CallIntMethod(g_plugin.globalRef, g_plugin.openFd, jUri, static_cast<jint>(mode));
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        LOGE("OpenByPlatformNative: java openFd threw exception");
        fd = -1;
    }
    env->DeleteLocalRef(jUri);
    if (fd < 0) {
        LOGE("OpenByPlatformNative: java openFd returned %{public}d", fd);
        return result;
    }
    // detachFd() returns an unowned fd; dup()+close(orig) so the new fd has no fdsan owner tag,
    // making it safe for both uv_fs_close (fs.close(fdNumber)) and fdsan_close_with_tag.
    int origFd = static_cast<int>(fd);
    int newFd = dup(origFd);
    if (newFd < 0) {
        LOGE("OpenByPlatformNative: dup(%{public}d) failed, errno=%{public}d", origFd, errno);
        close(origFd);
        return result;
    }
    close(origFd);
    LOGI("OpenByPlatformNative: dup origFd=%{public}d -> newFd=%{public}d", origFd, newFd);
    result.fd = newFd;
    result.opened = true;
    return result;
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
