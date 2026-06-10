/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
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

#include "storage_statistics_platform.h"

#include <cerrno>
#include <mutex>

#include "inner_api/plugin_utils_inner.h"
#include "jni.h"
#include "log.h"
#include "plugin_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleStorageStatistics {
namespace {
constexpr char STORAGE_STATISTICS_CLASS[] = "ohos/ace/plugin/storagestatistics/StorageStatisticsPlugin";
constexpr char QUERY_STATS_METHOD[] = "getCurrentBundleStats";
constexpr char QUERY_STATS_SIGNATURE[] = "()[J";
constexpr char QUERY_INODES_METHOD[] = "getCurrentBundleInodes";
constexpr char QUERY_INODES_SIGNATURE[] = "()J";
constexpr jsize APP_SIZE_INDEX = 0;
constexpr jsize CACHE_SIZE_INDEX = 1;
constexpr jsize DATA_SIZE_INDEX = 2;
constexpr jsize BUNDLE_STATS_LENGTH = DATA_SIZE_INDEX + 1;

void NativeInit(JNIEnv* env, jobject jobj);

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(NativeInit) },
};

struct {
    jmethodID getCurrentBundleStats;
    jmethodID getCurrentBundleInodes;
    jobject globalRef;
} g_pluginClass;

std::mutex g_pluginClassMutex;

void ResetPluginClass(JNIEnv* env)
{
    if (env != nullptr && g_pluginClass.globalRef != nullptr) {
        env->DeleteGlobalRef(g_pluginClass.globalRef);
    }
    g_pluginClass.getCurrentBundleStats = nullptr;
    g_pluginClass.getCurrentBundleInodes = nullptr;
    g_pluginClass.globalRef = nullptr;
}

void NativeInit(JNIEnv* env, jobject jobj)
{
    CHECK_NULL_VOID(env);
    std::lock_guard<std::mutex> lock(g_pluginClassMutex);
    ResetPluginClass(env);
    g_pluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);
    g_pluginClass.getCurrentBundleStats = env->GetMethodID(cls, QUERY_STATS_METHOD, QUERY_STATS_SIGNATURE);
    CHECK_NULL_VOID_DELREF(g_pluginClass.getCurrentBundleStats, env, cls);
    g_pluginClass.getCurrentBundleInodes = env->GetMethodID(cls, QUERY_INODES_METHOD, QUERY_INODES_SIGNATURE);
    CHECK_NULL_VOID_DELREF(g_pluginClass.getCurrentBundleInodes, env, cls);
    env->DeleteLocalRef(cls);
}

bool ReadBundleStats(JNIEnv* env, BundleStatsValue& stats)
{
    jmethodID getCurrentBundleStats = nullptr;
    jobject pluginObject = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_pluginClassMutex);
        if (g_pluginClass.globalRef == nullptr || g_pluginClass.getCurrentBundleStats == nullptr) {
            return false;
        }
        getCurrentBundleStats = g_pluginClass.getCurrentBundleStats;
        pluginObject = env->NewLocalRef(g_pluginClass.globalRef);
    }
    if (pluginObject == nullptr) {
        return false;
    }
    auto values = static_cast<jlongArray>(env->CallObjectMethod(pluginObject, getCurrentBundleStats));
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        if (values != nullptr) {
            env->DeleteLocalRef(values);
        }
        env->DeleteLocalRef(pluginObject);
        return false;
    }
    env->DeleteLocalRef(pluginObject);
    if (values == nullptr || env->GetArrayLength(values) != BUNDLE_STATS_LENGTH) {
        if (values != nullptr) {
            env->DeleteLocalRef(values);
        }
        return false;
    }
    jlong result[BUNDLE_STATS_LENGTH] = {0};
    env->GetLongArrayRegion(values, 0, BUNDLE_STATS_LENGTH, result);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        env->DeleteLocalRef(values);
        return false;
    }
    env->DeleteLocalRef(values);
    stats.appSize = static_cast<int64_t>(result[APP_SIZE_INDEX]);
    stats.cacheSize = static_cast<int64_t>(result[CACHE_SIZE_INDEX]);
    stats.dataSize = static_cast<int64_t>(result[DATA_SIZE_INDEX]);
    return true;
}

bool ReadBundleInodes(JNIEnv* env, int64_t& inodes)
{
    jmethodID getCurrentBundleInodes = nullptr;
    jobject pluginObject = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_pluginClassMutex);
        if (g_pluginClass.globalRef == nullptr || g_pluginClass.getCurrentBundleInodes == nullptr) {
            return false;
        }
        getCurrentBundleInodes = g_pluginClass.getCurrentBundleInodes;
        pluginObject = env->NewLocalRef(g_pluginClass.globalRef);
    }
    if (pluginObject == nullptr) {
        return false;
    }
    jlong value = env->CallLongMethod(pluginObject, getCurrentBundleInodes);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        env->DeleteLocalRef(pluginObject);
        return false;
    }
    env->DeleteLocalRef(pluginObject);
    if (value < 0) {
        return false;
    }
    inodes = static_cast<int64_t>(value);
    return true;
}
} // namespace

bool RegisterStorageStatisticsJavaPlugin(void* env)
{
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(STORAGE_STATISTICS_CLASS);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    return ret;
}

int32_t GetCurrentBundleStatsByPlatform(BundleStatsValue& stats)
{
    auto* env = ARKUI_X_Plugin_GetJniEnv();
    if (env == nullptr) {
        LOGE("failed to query bundle stats, jni env is null");
        return PLATFORM_ERROR;
    }
    stats = BundleStatsValue {};
    if (!ReadBundleStats(env, stats)) {
        LOGE("failed to query bundle stats from java plugin");
        return PLATFORM_ERROR;
    }
    return PLATFORM_SUCCESS;
}

int32_t GetCurrentBundleInodesByPlatform(int64_t& inodes)
{
    auto* env = ARKUI_X_Plugin_GetJniEnv();
    if (env == nullptr) {
        LOGE("failed to query bundle inodes, jni env is null");
        return PLATFORM_ERROR;
    }
    inodes = 0;
    if (!ReadBundleInodes(env, inodes)) {
        LOGE("failed to query bundle inodes from java plugin");
        return PLATFORM_ERROR;
    }
    return PLATFORM_SUCCESS;
}
} // namespace ModuleStorageStatistics
} // namespace FileManagement
} // namespace OHOS
