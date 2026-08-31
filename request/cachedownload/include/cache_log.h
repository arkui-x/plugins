/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_LOG_H
#define PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_LOG_H

#include <string>

//
// Unified diagnostic logging for the cross-platform cacheDownload plugin.
//
// Matches the purpose of the native HarmonyOS request module, which uses hilog
// (domain 0xD001C50, tag "PreloadNative"/"Requestkit") at key diagnostic points:
// task lifecycle, cache operations, file I/O, and error paths.
//
// Platform mapping:
//   Android  -> __android_log_print (logcat, tag "CacheDownload")
//   iOS      -> hilog HILOG_* macros (domain 0xD002B01, tag "CacheDownload")
//   Other    -> no-op (e.g., demo host compile)
//
// Format string: use the CDLOG_FMT_* helpers for arguments. On Android they
// expand to standard printf specifiers; on iOS they expand to hilog
// %{public}/%{private} specifiers.
//
// DEBUG gating (Android): CDLOG_DEBUG is runtime-gated by reading the system
// property log.tag.CacheDownload (same property __android_log_is_loggable
// consults, but that API requires __ANDROID_API__ >= 30 while this plugin
// targets API 26). Default (unset/empty) => DEBUG off, mirroring the native
// hilog runtime level filtering. To enable DEBUG logs on a device:
//   adb shell setprop log.tag.CacheDownload DEBUG
// then restart the app (the gate is cached for the process lifetime). On iOS
// the HILOG_DEBUG macro is already filtered by the hilog runtime level.

#ifdef ANDROID_PLATFORM
#include <android/log.h>
#include <atomic>
#include <sys/system_properties.h>
#define CDLOG_TAG "CacheDownload"
namespace OHOS::Plugin::CacheDownload {
// Runtime DEBUG gate, mirroring native hilog level filtering intent.
// Cached after first query: hot paths pay one relaxed atomic load.
enum class CdLogGateState {
    CDLOG_GATE_UNKNOWN = 0, // Property not queried yet
    CDLOG_GATE_OFF = 1,     // Property absent/empty or level above DEBUG
    CDLOG_GATE_ON = 2,      // Explicit DEBUG (or VERBOSE) setting
};

// Reads log.tag.CacheDownload and maps it to a gate state (called once).
inline CdLogGateState CdLogResolveGate()
{
    char value[PROP_VALUE_MAX] = {0};
    int len = __system_property_get("log.tag.CacheDownload", value);
    // Enable only on an explicit DEBUG (or lower) setting; unset, empty,
    // or higher levels (INFO/WARN/...) keep DEBUG gated off.
    return (len > 0 && value[0] != '\0' &&
            (value[0] == 'D' || value[0] == 'd' ||
             value[0] == 'V' || value[0] == 'v')) ? CdLogGateState::CDLOG_GATE_ON
                                                  : CdLogGateState::CDLOG_GATE_OFF;
}

inline bool CdLogDebugEnabled()
{
    static std::atomic<CdLogGateState> cached{CdLogGateState::CDLOG_GATE_UNKNOWN};
    CdLogGateState state = cached.load(std::memory_order_relaxed);
    if (state == CdLogGateState::CDLOG_GATE_UNKNOWN) {
        state = CdLogResolveGate();
        cached.store(state, std::memory_order_relaxed);
    }
    return state == CdLogGateState::CDLOG_GATE_ON;
}
} // namespace OHOS::Plugin::CacheDownload
#define CDLOG_INFO(fmt, ...)  __android_log_print(ANDROID_LOG_INFO,  CDLOG_TAG, fmt, ##__VA_ARGS__)
#define CDLOG_ERROR(fmt, ...) __android_log_print(ANDROID_LOG_ERROR, CDLOG_TAG, fmt, ##__VA_ARGS__)
#define CDLOG_WARN(fmt, ...)  __android_log_print(ANDROID_LOG_WARN,  CDLOG_TAG, fmt, ##__VA_ARGS__)
#define CDLOG_DEBUG(fmt, ...) \
    do { if (CdLogDebugEnabled()) \
        __android_log_print(ANDROID_LOG_DEBUG, CDLOG_TAG, fmt, ##__VA_ARGS__); } while (0)
#elif defined(IOS_PLATFORM)
#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xD002B01
#define LOG_TAG "CacheDownload"
#include "hilog/log.h"
#define CDLOG_INFO(fmt, ...)  HILOG_INFO(LOG_CORE, fmt, ##__VA_ARGS__)
#define CDLOG_ERROR(fmt, ...) HILOG_ERROR(LOG_CORE, fmt, ##__VA_ARGS__)
#define CDLOG_WARN(fmt, ...)  HILOG_WARN(LOG_CORE, fmt, ##__VA_ARGS__)
#define CDLOG_DEBUG(fmt, ...) HILOG_DEBUG(LOG_CORE, fmt, ##__VA_ARGS__)
#else
#define CDLOG_INFO(fmt, ...)  ((void)0)
#define CDLOG_ERROR(fmt, ...) ((void)0)
#define CDLOG_WARN(fmt, ...)  ((void)0)
#define CDLOG_DEBUG(fmt, ...) ((void)0)
#endif

// Format specifier helpers — expand to the correct format for each platform.
//
// Privacy defaults (iOS/hilog): CDLOG_FMT_S is %{private}s so free-form
// strings (paths, error messages, class names) are masked by the hilog privacy
// switch. Only content that is non-sensitive by construction — the brief task
// id (SHA-256 of the URL, first 16 hex chars) — is printed via
// CDLOG_FMT_PUBLIC_S (%{public}s) so log-based task correlation keeps working
// in release builds. Numeric specifiers carry no privacy concerns.
#ifdef IOS_PLATFORM
#define CDLOG_FMT_S         "%{private}s"
#define CDLOG_FMT_PUBLIC_S  "%{public}s"
#define CDLOG_FMT_D         "%{public}d"
#define CDLOG_FMT_ZU        "%{public}zu"
#define CDLOG_FMT_LLD       "%{public}lld"
#define CDLOG_FMT_LLU       "%{public}llu"
#define CDLOG_FMT_X         "%{public}x"
#else
// Android logcat has no privacy switch — both specifiers are plain %s. The
// call-site discipline still applies: only brief task ids may use
// CDLOG_FMT_PUBLIC_S; anything else uses CDLOG_FMT_S and must not contain
// sensitive content.
#define CDLOG_FMT_S         "%s"
#define CDLOG_FMT_PUBLIC_S  "%s"
#define CDLOG_FMT_D         "%d"
#define CDLOG_FMT_ZU        "%zu"
#define CDLOG_FMT_LLD       "%lld"
#define CDLOG_FMT_LLU       "%llu"
#define CDLOG_FMT_X         "%x"
#endif


// ============================================================
// Brief task ID for diagnostic logging (sensitive-content safe).
//
// Native HarmonyOS uses task_id.brief() = sha256(url) truncated to 16 hex
// chars (1/4 of the 64-char SHA-256 hex). The full URL is never printed.
// ArkUI-X port aligns with this purpose: all log call sites use
// GetBriefTaskId(url) instead of the raw URL to avoid leaking sensitive
// content (URLs may contain tokens, query params, user data).
//
// Implementation lives in cache_file.cpp (alongside the SHA-256 impl).
// ============================================================
namespace OHOS::Plugin::CacheDownload {
std::string GetBriefTaskId(const std::string& url);

// Returns the last path component of dirPath (e.g. "/a/b/preload_caches" ->
// "preload_caches"). Used by log call sites that report which cache directory
// is active without leaking the sandbox-absolute path (which contains the
// bundle id / container UUID). Empty input yields "<empty>".
inline std::string GetLastPathDir(const std::string& dirPath)
{
    if (dirPath.empty()) {
        return "<empty>";
    }
    // Strip trailing separators, then take the substring after the last '/'.
    // Windows separators are not handled — target platforms use '/'.
    size_t end = dirPath.find_last_not_of('/');
    if (end == std::string::npos) {
        return "/";
    }
    size_t pos = dirPath.find_last_of('/', end);
    return dirPath.substr(pos == std::string::npos ? 0 : pos + 1, end - pos);
}
} // namespace OHOS::Plugin::CacheDownload

#endif // PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_LOG_H
