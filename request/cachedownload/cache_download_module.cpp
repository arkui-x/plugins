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

#include <napi/native_api.h>
#include <napi/native_common.h>
#include <napi/native_node_api.h>

#include "cache_download_callback.h"
#include "cache_download_interface.h"
#include "cache_types.h"

#ifdef CACHE_DOWNLOAD_ENABLE_JNI
#include "cache_download_jni.h"
#include "plugin_utils.h"
#endif

#ifdef IOS_PLATFORM
#include "cache_download_ios_utils.h"
#endif

#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <list>

namespace OHOS::Plugin::CacheDownload {

namespace {

constexpr size_t NAPI_ARGC_TWO = 2;
constexpr size_t MAX_URL_LENGTH = 8192;
constexpr int64_t MAX_MEMORY_CACHE_SIZE = 1073741824; // 1 GB
constexpr int64_t MAX_FILE_CACHE_SIZE = 4294967296;   // 4 GB
constexpr int64_t MAX_INFO_LIST_SIZE = 8192;

// Parameter validation error code (matches request plugin E_PARAMETER_CHECK).
constexpr int32_t PARAM_ERROR_CODE = 401;
// Cache strategy values exposed to JS.
constexpr int32_t CACHE_STRATEGY_FORCE = 0;
constexpr int32_t CACHE_STRATEGY_LAZY = 1;
// Retry / timeout bounds and defaults.
constexpr int32_t DEFAULT_MAX_RETRY_COUNT = 1;
constexpr int32_t MAX_RETRY_COUNT = 10;
constexpr int32_t DEFAULT_NETWORK_CHECK_TIMEOUT = 20;
constexpr int32_t MAX_NETWORK_CHECK_TIMEOUT = 20;
constexpr int32_t MIN_HTTP_TOTAL_TIMEOUT = 1;
constexpr int32_t DEFAULT_HTTP_TOTAL_TIMEOUT = 60;
constexpr int32_t MAX_HTTP_TOTAL_TIMEOUT = 4294967;
// Error code enum values exposed to JS.
constexpr int32_t ERROR_CODE_DNS = 0;
constexpr int32_t ERROR_CODE_TCP = 0x10;
constexpr int32_t ERROR_CODE_SSL = 0x20;
constexpr int32_t ERROR_CODE_HTTP = 0x30;
constexpr int32_t ERROR_CODE_OTHERS = 0xFF;

napi_value CreateUndefined(napi_env env)
{
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

napi_value CreateInt32(napi_env env, int32_t value)
{
    napi_value result;
    napi_create_int32(env, value, &result);
    return result;
}

napi_value CreateDouble(napi_env env, double value)
{
    napi_value result;
    napi_create_double(env, value, &result);
    return result;
}

napi_value CreateInt64(napi_env env, int64_t value)
{
    napi_value result;
    napi_create_int64(env, value, &result);
    return result;
}

napi_value CreateString(napi_env env, const std::string& value)
{
    napi_value result;
    napi_create_string_utf8(env, value.c_str(), value.size(), &result);
    return result;
}

std::string GetStringArg(napi_env env, napi_value value)
{
    size_t len = 0;
    napi_get_value_string_utf8(env, value, nullptr, 0, &len);
    std::string result(len, '\0');
    napi_get_value_string_utf8(env, value, result.data(), len + 1, &len);
    return result;
}

bool IsNullOrUndefined(napi_env env, napi_value value)
{
    napi_valuetype type;
    napi_typeof(env, value, &type);
    return type == napi_undefined || type == napi_null;
}

// Throws a BusinessError with numeric code 401 (parameter validation), matching the
// convention used by the request plugin (NapiUtils::ThrowError, E_PARAMETER_CHECK = 401).
// Passes codeValue directly to napi_create_error (not nullptr + napi_set_named_property)
// because napi_set_named_property silently fails on Error objects in the ArkUI-X panda
// runtime (IsObjectWithoutSwitchState guard returns false for Error).
void ThrowParamError(napi_env env, const std::string& msg)
{
    napi_value businessError = nullptr;
    napi_value codeValue = nullptr;
    napi_value message = nullptr;
    napi_create_int32(env, PARAM_ERROR_CODE, &codeValue);
    napi_create_string_utf8(env, msg.c_str(), msg.size(), &message);
    napi_create_error(env, codeValue, message, &businessError);
    napi_throw(env, businessError);
}

bool GetObjectString(napi_env env, napi_value obj, const char* name, std::string& out)
{
    napi_value prop;
    napi_status status = napi_get_named_property(env, obj, name, &prop);
    if (status != napi_ok) {
        return false;
    }
    napi_valuetype type;
    napi_typeof(env, prop, &type);
    if (type == napi_undefined || type == napi_null) {
        return false;
    }
    if (type != napi_string) {
        return false;
    }
    out = GetStringArg(env, prop);
    return true;
}

bool GetObjectNumber(napi_env env, napi_value obj, const char* name, int64_t& out)
{
    napi_value prop;
    napi_status status = napi_get_named_property(env, obj, name, &prop);
    if (status != napi_ok) {
        return false;
    }
    napi_valuetype type;
    napi_typeof(env, prop, &type);
    if (type != napi_number) {
        return false;
    }
    napi_get_value_int64(env, prop, &out);
    return true;
}

bool GetObjectInt32(napi_env env, napi_value obj, const char* name, int32_t& out)
{
    int64_t value = 0;
    if (!GetObjectNumber(env, obj, name, value)) {
        return false;
    }
    out = static_cast<int32_t>(value);
    return true;
}

bool GetObjectMap(napi_env env, napi_value obj, const char* name,
                  std::map<std::string, std::string>& out)
{
    napi_value prop;
    napi_status status = napi_get_named_property(env, obj, name, &prop);
    if (status != napi_ok) {
        return false;
    }
    napi_valuetype type;
    napi_typeof(env, prop, &type);
    if (type != napi_object) {
        return false;
    }

    napi_value keys;
    napi_get_property_names(env, prop, &keys);
    uint32_t count = 0;
    napi_get_array_length(env, keys, &count);
    for (uint32_t i = 0; i < count; ++i) {
        napi_value key;
        napi_get_element(env, keys, i, &key);
        std::string keyStr = GetStringArg(env, key);
        napi_value val;
        napi_get_property(env, prop, key, &val);
        napi_valuetype valType;
        napi_typeof(env, val, &valType);
        if (valType == napi_string) {
            out[keyStr] = GetStringArg(env, val);
        }
    }
    return true;
}

bool ParseRetryOptions(napi_env env, napi_value obj, CacheDownloadOptions& options)
{
    napi_value retryObj;
    if (napi_get_named_property(env, obj, "retry", &retryObj) != napi_ok) {
        return true;
    }
    napi_valuetype retryType;
    napi_typeof(env, retryObj, &retryType);
    if (retryType != napi_object) {
        return true;
    }
    int32_t maxRetry = DEFAULT_MAX_RETRY_COUNT;
    bool hasField = GetObjectInt32(env, retryObj, "maxRetryCount", maxRetry);
    if (hasField && (maxRetry < 0 || maxRetry > MAX_RETRY_COUNT)) {
        return false;
    }
    options.retry = RetryOptions{static_cast<size_t>(std::max(0, maxRetry))};
    return true;
}

bool ParseTimeoutOptions(napi_env env, napi_value obj, CacheDownloadOptions& options)
{
    napi_value timeoutObj;
    if (napi_get_named_property(env, obj, "timeout", &timeoutObj) != napi_ok) {
        return true;
    }
    napi_valuetype timeoutType;
    napi_typeof(env, timeoutObj, &timeoutType);
    if (timeoutType != napi_object) {
        return true;
    }
    int32_t networkCheck = DEFAULT_NETWORK_CHECK_TIMEOUT;
    int32_t httpTotal = DEFAULT_HTTP_TOTAL_TIMEOUT;
    bool hasNc = GetObjectInt32(env, timeoutObj, "networkCheckTimeout", networkCheck);
    if (hasNc && (networkCheck < 0 || networkCheck > MAX_NETWORK_CHECK_TIMEOUT)) {
        return false;
    }
    bool hasHt = GetObjectInt32(env, timeoutObj, "httpTotalTimeout", httpTotal);
    if (hasHt && (httpTotal < MIN_HTTP_TOTAL_TIMEOUT || httpTotal > MAX_HTTP_TOTAL_TIMEOUT)) {
        return false;
    }
    options.timeout = TimeoutOptions{
        static_cast<uint32_t>(std::max(0, networkCheck)),
        static_cast<uint32_t>(std::max(MIN_HTTP_TOTAL_TIMEOUT, httpTotal))};
    return true;
}

bool ParseCacheDownloadOptions(napi_env env, napi_value obj, CacheDownloadOptions& options)
{
    napi_valuetype type;
    napi_typeof(env, obj, &type);
    if (type != napi_object) {
        return false;
    }

    GetObjectMap(env, obj, "headers", options.headers);

    std::string sslType;
    if (GetObjectString(env, obj, "sslType", sslType) && sslType == "TLCP") {
        options.sslType = SslType::TLCP;
    }

    GetObjectString(env, obj, "caPath", options.caPath);

    int32_t strategy = 0;
    if (GetObjectInt32(env, obj, "cacheStrategy", strategy) &&
        strategy == CACHE_STRATEGY_LAZY) {
        options.cacheStrategy = CacheStrategy::LAZY;
    }

    if (!ParseRetryOptions(env, obj, options)) {
        return false;
    }
    if (!ParseTimeoutOptions(env, obj, options)) {
        return false;
    }

    return true;
}

void CallJsCallback(napi_env env, napi_ref ref, const std::function<void(napi_env, napi_value&)>& buildArgs)
{
    auto task = [env, ref, buildArgs]() {
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env, &scope);

        napi_value callback;
        napi_status status = napi_get_reference_value(env, ref, &callback);
        if (status == napi_ok && callback != nullptr) {
            napi_value arg = nullptr;
            buildArgs(env, arg);
            napi_value global;
            napi_get_global(env, &global);
            napi_value argv[1] = { arg };
            size_t argc = (arg != nullptr) ? 1 : 0;
            napi_call_function(env, global, callback, argc, argv, nullptr);
        }
        napi_delete_reference(env, ref);

        napi_close_handle_scope(env, scope);
    };

    int32_t ret = napi_send_event(env, task, napi_eprio_high, "request:cachedownload");
    if (ret != napi_ok) {
        napi_delete_reference(env, ref);
    }
}

// ============================================================
// Callback registry.
//
// onDownloadSuccess/Error callbacks PERSIST across downloads (fired on every
// completion of their URL) and are removed only via off*. Each URL has ONE
// dispatcher registered with the service; when a download completes, the
// dispatcher posts a single napi event, and the event — executing on the JS
// thread — iterates the LIVE callback list for that URL, calling each
// callback synchronously (matching the OHOS InvokeSuccessCallbacks semantics
// in preload_module.cpp):
//   - A callback registered AFTER download() but in the same JS turn still
//     receives the completion event, because the list is read when the event
//     runs, not when the download finishes. LAZY cache hits post the event
//     from within download(), so the same guarantee holds there too.
//   - A callback registered DURING another callback's execution is appended
//     to the list and receives the CURRENT event in the same dispatch loop.
//   - off() during dispatch only marks the entry removed; it is skipped and
//     erased lazily, so dispatch iterators stay valid.
// Dispatchers are registered at download() time (not only at on()), because a
// fast completion can beat a same-turn on() registration — OHOS posts
// completion events unconditionally and gates on the listener list when the
// event runs on the JS thread; keeping the dispatcher in place once a URL is
// downloaded mirrors that. Empty listener lists simply make the event a no-op.
// All registry access happens on the JS thread (NAPI calls and napi event
// tasks), so no locking is needed.
// ============================================================
struct CbEntry {
    napi_ref ref = nullptr;
    bool removed = false; // set by off() during dispatch; erased lazily
};

struct UrlCallbacks {
    std::list<CbEntry> entries;
    int dispatching = 0;
    int64_t serviceToken = 0; // service-side identity of the single dispatcher
};

struct CallbackRegistry {
    std::unordered_map<std::string, UrlCallbacks> success;
    std::unordered_map<std::string, UrlCallbacks> error;
    int64_t nextToken = 1;
};

CallbackRegistry& GetCallbackRegistry()
{
    static CallbackRegistry reg;
    return reg;
}

void DeleteEntryRef(napi_env env, CbEntry& entry)
{
    if (entry.ref) {
        napi_delete_reference(env, entry.ref);
        entry.ref = nullptr;
    }
}

// Drops the per-URL service dispatcher once no JS callbacks remain. Must only
// be called when the dispatch loop is not running (it holds a UrlCallbacks&).
void MaybeUnregisterDispatcher(std::unordered_map<std::string, UrlCallbacks>& map,
                               const std::string& url, bool isSuccess)
{
    auto it = map.find(url);
    if (it == map.end() || it->second.dispatching > 0 || !it->second.entries.empty()) {
        return;
    }
    int64_t token = it->second.serviceToken;
    map.erase(it);
    if (token != 0) {
        if (isSuccess) {
            CacheDownloadService::GetInstance().OffDownloadSuccess(url, token);
        } else {
            CacheDownloadService::GetInstance().OffDownloadError(url, token);
        }
    }
}

// Runs on the JS thread (posted via napi_send_event). Iterates the live list:
// callbacks appended during the loop receive this same event; entries marked
// removed are skipped and erased.
void FireSuccessCallbacks(napi_env env, const std::string& url)
{
    auto& reg = GetCallbackRegistry();
    auto it = reg.success.find(url);
    if (it == reg.success.end()) {
        return;
    }
    UrlCallbacks& cbs = it->second;
    cbs.dispatching++;

    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    napi_value global = nullptr;
    napi_get_global(env, &global);

    for (auto eit = cbs.entries.begin(); eit != cbs.entries.end();) {
        if (eit->removed) {
            DeleteEntryRef(env, *eit);
            eit = cbs.entries.erase(eit);
            continue;
        }
        napi_value callback = nullptr;
        if (eit->ref && napi_get_reference_value(env, eit->ref, &callback) == napi_ok && callback) {
            napi_call_function(env, global, callback, 0, nullptr, nullptr);
        }
        // The callback may have called off() (marks removed) or on() (appends
        // to the tail — picked up by this same loop).
        if (eit->removed) {
            DeleteEntryRef(env, *eit);
            eit = cbs.entries.erase(eit);
        } else {
            ++eit;
        }
    }

    napi_close_handle_scope(env, scope);
    cbs.dispatching--;
    // Note: the dispatcher stays registered even with an empty listener list —
    // a later completion must still post its (no-op) event, because listeners
    // registered after that completion but before the event runs have to fire.
}

void FireErrorCallbacks(napi_env env, const std::string& url, const CacheDownloadError& error)
{
    auto& reg = GetCallbackRegistry();
    auto it = reg.error.find(url);
    if (it == reg.error.end()) {
        return;
    }
    UrlCallbacks& cbs = it->second;
    cbs.dispatching++;

    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    napi_value global = nullptr;
    napi_get_global(env, &global);

    for (auto eit = cbs.entries.begin(); eit != cbs.entries.end();) {
        if (eit->removed) {
            DeleteEntryRef(env, *eit);
            eit = cbs.entries.erase(eit);
            continue;
        }
        napi_value callback = nullptr;
        if (eit->ref && napi_get_reference_value(env, eit->ref, &callback) == napi_ok && callback) {
            napi_value arg = nullptr;
            napi_create_object(env, &arg);
            napi_set_named_property(env, arg, "errorCode",
                                    CreateInt32(env, static_cast<int32_t>(error.errorCode)));
            napi_set_named_property(env, arg, "message", CreateString(env, error.message));
            napi_value argv[1] = { arg };
            napi_call_function(env, global, callback, 1, argv, nullptr);
        }
        if (eit->removed) {
            DeleteEntryRef(env, *eit);
            eit = cbs.entries.erase(eit);
        } else {
            ++eit;
        }
    }

    napi_close_handle_scope(env, scope);
    cbs.dispatching--;
}

// Called from the service on an arbitrary native thread; posts one event per
// completion. The callback list is read when the event executes on the JS
// thread, so callbacks registered after download() but before the event runs
// still fire.
void PostSuccessEvent(napi_env env, std::string url)
{
    auto task = [env, url = std::move(url)]() {
        FireSuccessCallbacks(env, url);
    };
    napi_send_event(env, task, napi_eprio_high, "request:cachedownload");
}

void PostErrorEvent(napi_env env, std::string url, CacheDownloadError error)
{
    auto task = [env, url = std::move(url), error = std::move(error)]() {
        FireErrorCallbacks(env, url, error);
    };
    napi_send_event(env, task, napi_eprio_high, "request:cachedownload");
}

// Registers the per-URL dispatcher with the service if not yet registered.
void EnsureSuccessDispatcher(napi_env env, const std::string& url)
{
    auto& reg = GetCallbackRegistry();
    UrlCallbacks& cbs = reg.success[url];
    if (cbs.serviceToken != 0) {
        return;
    }
    cbs.serviceToken = reg.nextToken++;
    int64_t token = cbs.serviceToken;
    CacheDownloadService::GetInstance().OnDownloadSuccess(url, token, [env, url]() {
        PostSuccessEvent(env, url);
    });
}

void EnsureErrorDispatcher(napi_env env, const std::string& url)
{
    auto& reg = GetCallbackRegistry();
    UrlCallbacks& cbs = reg.error[url];
    if (cbs.serviceToken != 0) {
        return;
    }
    cbs.serviceToken = reg.nextToken++;
    int64_t token = cbs.serviceToken;
    CacheDownloadService::GetInstance().OnDownloadError(
        url, token, [env, url](const CacheDownloadError& error) {
            PostErrorEvent(env, url, error);
        });
}

} // namespace

napi_value Download(napi_env env, napi_callback_info info)
{
    size_t argc = NAPI_ARGC_TWO;
    napi_value args[NAPI_ARGC_TWO] = { nullptr };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NAPI_ARGC_TWO) {
        ThrowParamError(env, "download requires url and options");
        return CreateUndefined(env);
    }

    if (IsNullOrUndefined(env, args[0])) {
        ThrowParamError(env, "invalid url");
        return CreateUndefined(env);
    }

    std::string url = GetStringArg(env, args[0]);
    if (url.size() > MAX_URL_LENGTH) {
        ThrowParamError(env, "invalid url");
        return CreateUndefined(env);
    }

    if (url.empty()) {
        // An empty url is a no-op (SUB_Request_cacheDownload_supplemental_2300).
        return CreateUndefined(env);
    }

    CacheDownloadOptions options;
    if (IsNullOrUndefined(env, args[1]) || !ParseCacheDownloadOptions(env, args[1], options)) {
        ThrowParamError(env, "invalid options");
        return CreateUndefined(env);
    }

    auto& service = CacheDownloadService::GetInstance();
    if (!service.IsInitialized()) {
#ifdef IOS_PLATFORM
        std::string cacheDir = GetIosCacheDirectory();
        service.Initialize(cacheDir);
#else
        // Target platform initialization is performed in NativeInit from CacheDownloadPlugin.
        (void)service;
#endif
    }

    // Register the completion dispatchers up front: a fast (e.g. localhost or
    // cache-hit) completion can fire before a same-turn onDownloadSuccess/
    // onDownloadError call, and the event posted at completion time must still
    // reach callbacks registered before the event executes on the JS thread.
    EnsureSuccessDispatcher(env, url);
    EnsureErrorDispatcher(env, url);

    service.Download(url, options);
    return CreateUndefined(env);
}

napi_value Cancel(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1 || IsNullOrUndefined(env, args[0])) {
        ThrowParamError(env, "cancel requires url");
        return CreateUndefined(env);
    }

    std::string url = GetStringArg(env, args[0]);
    if (url.size() > MAX_URL_LENGTH) {
        ThrowParamError(env, "invalid url");
        return CreateUndefined(env);
    }
    CacheDownloadService::GetInstance().Cancel(url);
    return CreateUndefined(env);
}

napi_value SetMemoryCacheSize(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1 || IsNullOrUndefined(env, args[0])) {
        ThrowParamError(env, "setMemoryCacheSize requires bytes");
        return CreateUndefined(env);
    }

    int64_t bytes = 0;
    napi_get_value_int64(env, args[0], &bytes);
    if (bytes < 0 || bytes > MAX_MEMORY_CACHE_SIZE) {
        ThrowParamError(env, "bytes out of range");
        return CreateUndefined(env);
    }

    CacheDownloadService::GetInstance().SetMemoryCacheSize(static_cast<uint64_t>(bytes));
    return CreateUndefined(env);
}

napi_value SetFileCacheSize(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1 || IsNullOrUndefined(env, args[0])) {
        ThrowParamError(env, "setFileCacheSize requires bytes");
        return CreateUndefined(env);
    }

    int64_t bytes = 0;
    napi_get_value_int64(env, args[0], &bytes);
    if (bytes < 0 || bytes > MAX_FILE_CACHE_SIZE) {
        ThrowParamError(env, "bytes out of range");
        return CreateUndefined(env);
    }

    CacheDownloadService::GetInstance().SetFileCacheSize(static_cast<uint64_t>(bytes));
    return CreateUndefined(env);
}

napi_value BuildNetworkNapiObject(napi_env env, const DownloadInfo& di)
{
    napi_value network;
    napi_create_object(env, &network);
    // network.ip: undefined when empty (e.g. DNS failure), else the IP string.
    if (di.network.ip.empty()) {
        napi_value undef;
        napi_get_undefined(env, &undef);
        napi_set_named_property(env, network, "ip", undef);
    } else {
        napi_set_named_property(env, network, "ip", CreateString(env, di.network.ip));
    }
    // network.dnsServers: array (possibly empty) — typeof === 'object'.
    napi_value dnsArr;
    napi_create_array_with_length(env, di.network.dnsServers.size(), &dnsArr);
    for (size_t i = 0; i < di.network.dnsServers.size(); ++i) {
        napi_value s = CreateString(env, di.network.dnsServers[i]);
        napi_set_element(env, dnsArr, static_cast<uint32_t>(i), s);
    }
    napi_set_named_property(env, network, "dnsServers", dnsArr);
    return network;
}

napi_value BuildPerformanceNapiObject(napi_env env, const DownloadInfo& di)
{
    napi_value performance;
    napi_create_object(env, &performance);
    napi_set_named_property(env, performance, "dnsTime", CreateDouble(env, di.performance.dnsTime));
    napi_set_named_property(env, performance, "connectTime", CreateDouble(env, di.performance.connectTime));
    napi_set_named_property(env, performance, "tlsTime", CreateDouble(env, di.performance.tlsTime));
    napi_set_named_property(env, performance, "firstSendTime", CreateDouble(env, di.performance.firstSendTime));
    napi_set_named_property(env, performance, "firstReceiveTime", CreateDouble(env, di.performance.firstRecvTime));
    napi_set_named_property(env, performance, "totalTime", CreateDouble(env, di.performance.totalTime));
    napi_set_named_property(env, performance, "redirectTime", CreateDouble(env, di.performance.redirectTime));
    return performance;
}

napi_value GetDownloadInfo(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1 || IsNullOrUndefined(env, args[0])) {
        ThrowParamError(env, "getDownloadInfo requires url");
        return CreateUndefined(env);
    }

    std::string url = GetStringArg(env, args[0]);
    if (url.size() > MAX_URL_LENGTH) {
        ThrowParamError(env, "invalid url");
        return CreateUndefined(env);
    }
    auto infoOpt = CacheDownloadService::GetInstance().GetDownloadInfo(url);
    if (!infoOpt.has_value()) {
        return CreateUndefined(env);
    }

    const DownloadInfo& di = *infoOpt;
    napi_value result;
    napi_create_object(env, &result);

    napi_value resource;
    napi_create_object(env, &resource);
    napi_set_named_property(env, resource, "size", CreateInt64(env, di.resource.size));
    napi_set_named_property(env, result, "resource", resource);

    napi_set_named_property(env, result, "network", BuildNetworkNapiObject(env, di));
    napi_set_named_property(env, result, "performance", BuildPerformanceNapiObject(env, di));

    return result;
}

napi_value SetDownloadInfoListSize(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1 || IsNullOrUndefined(env, args[0])) {
        ThrowParamError(env, "setDownloadInfoListSize requires size");
        return CreateUndefined(env);
    }

    int64_t size = 0;
    napi_get_value_int64(env, args[0], &size);
    if (size < 0 || size > MAX_INFO_LIST_SIZE) {
        ThrowParamError(env, "size out of range");
        return CreateUndefined(env);
    }

    CacheDownloadService::GetInstance().SetDownloadInfoListSize(static_cast<uint16_t>(size));
    return CreateUndefined(env);
}

napi_value ClearMemoryCache(napi_env env, napi_callback_info /* info */)
{
    CacheDownloadService::GetInstance().ClearMemoryCache();
    return CreateUndefined(env);
}

napi_value ClearFileCache(napi_env env, napi_callback_info /* info */)
{
    CacheDownloadService::GetInstance().ClearFileCache();
    return CreateUndefined(env);
}

napi_value OnDownloadSuccess(napi_env env, napi_callback_info info)
{
    size_t argc = NAPI_ARGC_TWO;
    napi_value args[NAPI_ARGC_TWO] = { nullptr };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NAPI_ARGC_TWO) {
        ThrowParamError(env, "onDownloadSuccess requires url and callback");
        return CreateUndefined(env);
    }

    if (IsNullOrUndefined(env, args[0])) {
        ThrowParamError(env, "invalid url");
        return CreateUndefined(env);
    }

    std::string url = GetStringArg(env, args[0]);
    if (url.empty() || url.size() > MAX_URL_LENGTH) {
        ThrowParamError(env, "invalid url");
        return CreateUndefined(env);
    }

    napi_valuetype cbType;
    napi_typeof(env, args[1], &cbType);
    if (cbType != napi_function) {
        ThrowParamError(env, "callback must be a function");
        return CreateUndefined(env);
    }

    napi_ref ref;
    napi_create_reference(env, args[1], 1, &ref);

    EnsureSuccessDispatcher(env, url);
    GetCallbackRegistry().success[url].entries.push_back({ref, false});

    return CreateUndefined(env);
}

napi_value OnDownloadError(napi_env env, napi_callback_info info)
{
    size_t argc = NAPI_ARGC_TWO;
    napi_value args[NAPI_ARGC_TWO] = { nullptr };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NAPI_ARGC_TWO) {
        ThrowParamError(env, "onDownloadError requires url and callback");
        return CreateUndefined(env);
    }

    if (IsNullOrUndefined(env, args[0])) {
        ThrowParamError(env, "invalid url");
        return CreateUndefined(env);
    }

    std::string url = GetStringArg(env, args[0]);
    if (url.empty() || url.size() > MAX_URL_LENGTH) {
        ThrowParamError(env, "invalid url");
        return CreateUndefined(env);
    }

    napi_valuetype cbType;
    napi_typeof(env, args[1], &cbType);
    if (cbType != napi_function) {
        ThrowParamError(env, "callback must be a function");
        return CreateUndefined(env);
    }

    napi_ref ref;
    napi_create_reference(env, args[1], 1, &ref);

    EnsureErrorDispatcher(env, url);
    GetCallbackRegistry().error[url].entries.push_back({ref, false});

    return CreateUndefined(env);
}

// Returns true if fnValue is null (match-all) or the stored JS function
// strictly equals fnValue.
bool CallbackMatches(napi_env env, napi_ref ref, napi_value fnValue)
{
    if (fnValue == nullptr) {
        return true;
    }
    if (!ref) {
        return false;
    }
    napi_value stored = nullptr;
    if (napi_get_reference_value(env, ref, &stored) != napi_ok || stored == nullptr) {
        return false;
    }
    bool eq = false;
    napi_strict_equals(env, stored, fnValue, &eq);
    return eq;
}

// Removes entries whose stored JS function strictly equals fnValue (or all
// entries for the url if fnValue is null). During dispatch entries are only
// marked removed (the running loop skips and erases them, keeping iterators
// valid); otherwise they are erased immediately.
void RemoveCallbacks(napi_env env, const std::string& url, napi_value fnValue,
                     bool isSuccess)
{
    auto& reg = GetCallbackRegistry();
    auto& map = isSuccess ? reg.success : reg.error;
    auto it = map.find(url);
    if (it == map.end()) {
        return;
    }
    UrlCallbacks& cbs = it->second;

    for (auto eit = cbs.entries.begin(); eit != cbs.entries.end();) {
        if (!CallbackMatches(env, eit->ref, fnValue)) {
            ++eit;
            continue;
        }
        if (cbs.dispatching > 0) {
            eit->removed = true;
            ++eit;
        } else {
            DeleteEntryRef(env, *eit);
            eit = cbs.entries.erase(eit);
        }
    }
    MaybeUnregisterDispatcher(map, url, isSuccess);
}

napi_value OffDownloadSuccess(napi_env env, napi_callback_info info)
{
    size_t argc = NAPI_ARGC_TWO;
    napi_value args[NAPI_ARGC_TWO] = { nullptr };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1 || IsNullOrUndefined(env, args[0])) {
        ThrowParamError(env, "offDownloadSuccess requires url");
        return CreateUndefined(env);
    }

    std::string url = GetStringArg(env, args[0]);
    if (url.empty() || url.size() > MAX_URL_LENGTH) {
        ThrowParamError(env, "invalid url");
        return CreateUndefined(env);
    }

    napi_value fnValue = nullptr;
    if (argc >= NAPI_ARGC_TWO && !IsNullOrUndefined(env, args[1])) {
        napi_valuetype t;
        napi_typeof(env, args[1], &t);
        if (t == napi_function) {
            fnValue = args[1];
        }
    }
    // isSuccess = true for success callback removal
    RemoveCallbacks(env, url, fnValue, true);
    return CreateUndefined(env);
}

napi_value OffDownloadError(napi_env env, napi_callback_info info)
{
    size_t argc = NAPI_ARGC_TWO;
    napi_value args[NAPI_ARGC_TWO] = { nullptr };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1 || IsNullOrUndefined(env, args[0])) {
        ThrowParamError(env, "offDownloadError requires url");
        return CreateUndefined(env);
    }

    std::string url = GetStringArg(env, args[0]);
    if (url.empty() || url.size() > MAX_URL_LENGTH) {
        ThrowParamError(env, "invalid url");
        return CreateUndefined(env);
    }

    napi_value fnValue = nullptr;
    if (argc >= NAPI_ARGC_TWO && !IsNullOrUndefined(env, args[1])) {
        napi_valuetype t;
        napi_typeof(env, args[1], &t);
        if (t == napi_function) {
            fnValue = args[1];
        }
    }
    // isSuccess = false for error callback removal
    RemoveCallbacks(env, url, fnValue, false);
    return CreateUndefined(env);
}

napi_value SetGlobalRetryOptions(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1 || IsNullOrUndefined(env, args[0])) {
        CacheDownloadService::GetInstance().SetGlobalRetryOptions(std::nullopt);
        return CreateUndefined(env);
    }

    napi_valuetype type;
    napi_typeof(env, args[0], &type);
    if (type != napi_object) {
        ThrowParamError(env, "options must be an object");
        return CreateUndefined(env);
    }

    int32_t maxRetry = DEFAULT_MAX_RETRY_COUNT;
    GetObjectInt32(env, args[0], "maxRetryCount", maxRetry);
    if (maxRetry < 0 || maxRetry > MAX_RETRY_COUNT) {
        ThrowParamError(env, "maxRetryCount out of range");
        return CreateUndefined(env);
    }
    CacheDownloadService::GetInstance().SetGlobalRetryOptions(
        static_cast<size_t>(std::max(0, maxRetry)));
    return CreateUndefined(env);
}

napi_value SetGlobalTimeoutOptions(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1 || IsNullOrUndefined(env, args[0])) {
        CacheDownloadService::GetInstance().SetGlobalTimeoutOptions(std::nullopt, std::nullopt);
        return CreateUndefined(env);
    }

    napi_valuetype type;
    napi_typeof(env, args[0], &type);
    if (type != napi_object) {
        ThrowParamError(env, "options must be an object");
        return CreateUndefined(env);
    }

    int32_t networkCheck = DEFAULT_NETWORK_CHECK_TIMEOUT;
    int32_t httpTotal = DEFAULT_HTTP_TOTAL_TIMEOUT;
    GetObjectInt32(env, args[0], "networkCheckTimeout", networkCheck);
    GetObjectInt32(env, args[0], "httpTotalTimeout", httpTotal);
    if (networkCheck < 0 || networkCheck > MAX_NETWORK_CHECK_TIMEOUT) {
        ThrowParamError(env, "networkCheckTimeout out of range");
        return CreateUndefined(env);
    }
    if (httpTotal < MIN_HTTP_TOTAL_TIMEOUT || httpTotal > MAX_HTTP_TOTAL_TIMEOUT) {
        ThrowParamError(env, "httpTotalTimeout out of range");
        return CreateUndefined(env);
    }
    CacheDownloadService::GetInstance().SetGlobalTimeoutOptions(
        static_cast<uint32_t>(std::max(0, networkCheck)),
        static_cast<uint32_t>(std::max(MIN_HTTP_TOTAL_TIMEOUT, httpTotal)));
    return CreateUndefined(env);
}

napi_value InitModule(napi_env env, napi_value exports)
{
    napi_value sslType;
    napi_create_object(env, &sslType);
    napi_set_named_property(env, sslType, "TLS", CreateString(env, "TLS"));
    napi_set_named_property(env, sslType, "TLCP", CreateString(env, "TLCP"));

    napi_value cacheStrategy;
    napi_create_object(env, &cacheStrategy);
    napi_set_named_property(env, cacheStrategy, "FORCE", CreateInt32(env, CACHE_STRATEGY_FORCE));
    napi_set_named_property(env, cacheStrategy, "LAZY", CreateInt32(env, CACHE_STRATEGY_LAZY));

    napi_value errorCode;
    napi_create_object(env, &errorCode);
    napi_set_named_property(env, errorCode, "DNS", CreateInt32(env, ERROR_CODE_DNS));
    napi_set_named_property(env, errorCode, "TCP", CreateInt32(env, ERROR_CODE_TCP));
    napi_set_named_property(env, errorCode, "SSL", CreateInt32(env, ERROR_CODE_SSL));
    napi_set_named_property(env, errorCode, "HTTP", CreateInt32(env, ERROR_CODE_HTTP));
    napi_set_named_property(env, errorCode, "OTHERS", CreateInt32(env, ERROR_CODE_OTHERS));

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_PROPERTY("SslType", sslType),
        DECLARE_NAPI_PROPERTY("CacheStrategy", cacheStrategy),
        DECLARE_NAPI_PROPERTY("ErrorCode", errorCode),

        DECLARE_NAPI_FUNCTION("download", Download),
        DECLARE_NAPI_FUNCTION("cancel", Cancel),
        DECLARE_NAPI_FUNCTION("setMemoryCacheSize", SetMemoryCacheSize),
        DECLARE_NAPI_FUNCTION("setFileCacheSize", SetFileCacheSize),
        DECLARE_NAPI_FUNCTION("getDownloadInfo", GetDownloadInfo),
        DECLARE_NAPI_FUNCTION("setDownloadInfoListSize", SetDownloadInfoListSize),
        DECLARE_NAPI_FUNCTION("clearMemoryCache", ClearMemoryCache),
        DECLARE_NAPI_FUNCTION("clearFileCache", ClearFileCache),
        DECLARE_NAPI_FUNCTION("onDownloadSuccess", OnDownloadSuccess),
        DECLARE_NAPI_FUNCTION("onDownloadError", OnDownloadError),
        DECLARE_NAPI_FUNCTION("offDownloadSuccess", OffDownloadSuccess),
        DECLARE_NAPI_FUNCTION("offDownloadError", OffDownloadError),
        DECLARE_NAPI_FUNCTION("setGlobalRetryOptions", SetGlobalRetryOptions),
        DECLARE_NAPI_FUNCTION("setGlobalTimeoutOptions", SetGlobalTimeoutOptions),
    };

    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}

} // namespace OHOS::Plugin::CacheDownload

#ifdef CACHE_DOWNLOAD_ENABLE_JNI
static void RegisterCacheDownloadJavaPlugin()
{
    const char className[] = "ohos.ace.plugin.cachedownloadplugin.CacheDownloadPlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(
        &OHOS::Plugin::CacheDownload::CacheDownloadJni::Register, className);
}
#endif

extern "C" __attribute__((constructor)) void CacheDownloadRegister()
{
    static napi_module module = {
        .nm_version = 1,
        .nm_flags = 0,
        .nm_filename = nullptr,
        .nm_register_func = OHOS::Plugin::CacheDownload::InitModule,
        .nm_modname = "request.cacheDownload",
        .nm_priv = nullptr,
        .reserved = { 0 }
    };
#ifdef CACHE_DOWNLOAD_ENABLE_JNI
    RegisterCacheDownloadJavaPlugin();
#endif
    napi_module_register(&module);
}
