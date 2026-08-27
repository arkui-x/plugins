/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0
 */

#include <jni.h>
#include "uri_opener.h"

#include <atomic>
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

struct PluginState {
    std::atomic<jmethodID> openFd{nullptr};
    std::atomic<jobject> globalRef{nullptr};
    std::atomic<bool> initialized{false};
};

PluginState g_plugin;
} // namespace

void NativeInit(JNIEnv* env, jobject jobj)
{
    jobject ref = env->NewGlobalRef(jobj);
    jclass cls = env->GetObjectClass(jobj);
    jmethodID method = env->GetMethodID(cls, OPEN_FD_METHOD, OPEN_FD_SIGNATURE);
    env->DeleteLocalRef(cls);
    if (method == nullptr) {
        LOGE("NativeInit: GetMethodID openFd failed");
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        return;
    }
    g_plugin.globalRef.store(ref, std::memory_order_release);
    g_plugin.openFd.store(method, std::memory_order_release);
    g_plugin.initialized.store(true, std::memory_order_release);
    LOGI("NativeInit: FsUriOpenerPlugin initialized");
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
    if (!g_plugin.initialized.load(std::memory_order_acquire)) {
        LOGE("OpenByPlatformNative: plugin not initialized");
        return result;
    }
    auto* env = ARKUI_X_Plugin_GetJniEnv();
    if (env == nullptr) {
        LOGE("OpenByPlatformNative: jni env is null");
        return result;
    }
    jobject ref = g_plugin.globalRef.load(std::memory_order_acquire);
    jmethodID method = g_plugin.openFd.load(std::memory_order_acquire);
    if (ref == nullptr || method == nullptr) {
        LOGE("OpenByPlatformNative: invalid plugin state (globalRef=%p openFd=%p)", ref, method);
        return result;
    }
    jstring jUri = env->NewStringUTF(uri.c_str());
    if (jUri == nullptr) {
        return result;
    }
    jint fd = env->CallIntMethod(ref, method, jUri, static_cast<jint>(mode));
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        LOGE("OpenByPlatformNative: java openFd threw exception");
        env->DeleteLocalRef(jUri);
        return result;
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
