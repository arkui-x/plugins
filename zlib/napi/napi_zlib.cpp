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

#include "napi_zlib.h"

#include <cstring>
#include <uv.h>
#include <vector>

#include "bundle_errors.h"
#include "business_error.h"
#include "file_path.h"
#include "log.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "napi_arg.h"
#include "napi_constants.h"
#include "napi_zlib_common.h"
#include "zip.h"
#include "zip_utils.h"
#include "zlib_callback_info.h"

using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
namespace LIBZIP {
namespace {
constexpr size_t ARGS_MAX_COUNT = 10;
constexpr int32_t PARAM3 = 3;
constexpr int32_t PARAM2 = 2;
const char* WRONG_PARAM = "wrong param type";
} // namespace

#define COMPRESS_LEVE_CHECK(level, ret)                                                            \
    if (!(level == COMPRESS_LEVEL_NO_COMPRESSION || level == COMPRESS_LEVEL_DEFAULT_COMPRESSION || \
            level == COMPRESS_LEVEL_BEST_SPEED || level == COMPRESS_LEVEL_BEST_COMPRESSION)) {     \
        LOGE("level parameter =[%{public}d] value is incorrect", (int)level);                      \
        return ret;                                                                                \
    }

#define COMPRESS_STRATEGY_CHECK(strategy, false)                                                      \
    if (!(strategy == COMPRESS_STRATEGY_DEFAULT_STRATEGY || strategy == COMPRESS_STRATEGY_FILTERED || \
            strategy == COMPRESS_STRATEGY_HUFFMAN_ONLY || strategy == COMPRESS_STRATEGY_RLE ||        \
            strategy == COMPRESS_STRATEGY_FIXED)) {                                                   \
        LOGE("strategy parameter= [%{public}d] value is incorrect", (int)strategy);                   \
        return ret;                                                                                   \
    }

#define COMPRESS_MEM_CHECK(mem, false)                                                                            \
    if (!(mem == MEM_LEVEL_MIN_MEMLEVEL || mem == MEM_LEVEL_DEFAULT_MEMLEVEL || mem == MEM_LEVEL_MAX_MEMLEVEL)) { \
        LOGE("memLevel parameter =[%{public}d] value is incorrect", (int)mem);                                    \
        return ret;                                                                                               \
    }

void CompressExcute(napi_env env, AsyncZipCallbackInfo* asyncZipCallbackInfo);
void DecompressExcute(napi_env env, AsyncZipCallbackInfo* asyncZipCallbackInfo);
napi_value UnwrapStringParam(std::string& str, napi_env env, napi_value argv);
bool UnwrapOptionsParams(OPTIONS& options, napi_env env, napi_value arg);

/**
 * @brief FlushType data initialization.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value FlushTypeInit(napi_env env, napi_value exports)
{
    LOGD("called.");
    const int FLUSH_TYPE_NO_FLUSH = 0;
    const int FLUSH_TYPE_PARTIAL_FLUSH = 1;
    const int FLUSH_TYPE_SYNC_FLUSH = 2;
    const int FLUSH_TYPE_FULL_FLUSH = 3;
    const int FLUSH_TYPE_FINISH = 4;
    const int FLUSH_TYPE_BLOCK = 5;
    const int FLUSH_TYPE_TREES = 6;

    napi_value flushType = nullptr;
    napi_create_object(env, &flushType);
    SetNamedProperty(env, flushType, "FLUSH_TYPE_NO_FLUSH", FLUSH_TYPE_NO_FLUSH);
    SetNamedProperty(env, flushType, "FLUSH_TYPE_PARTIAL_FLUSH", FLUSH_TYPE_PARTIAL_FLUSH);
    SetNamedProperty(env, flushType, "FLUSH_TYPE_SYNC_FLUSH", FLUSH_TYPE_SYNC_FLUSH);
    SetNamedProperty(env, flushType, "FLUSH_TYPE_FULL_FLUSH", FLUSH_TYPE_FULL_FLUSH);
    SetNamedProperty(env, flushType, "FLUSH_TYPE_FINISH", FLUSH_TYPE_FINISH);
    SetNamedProperty(env, flushType, "FLUSH_TYPE_BLOCK", FLUSH_TYPE_BLOCK);
    SetNamedProperty(env, flushType, "FLUSH_TYPE_TREES", FLUSH_TYPE_TREES);

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_PROPERTY("FlushType", flushType),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties));

    return exports;
}
/**
 * @brief CompressLevel data initialization.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value CompressLevelInit(napi_env env, napi_value exports)
{
    LOGD("called.");
    const int COMPRESS_LEVEL_NO_COMPRESSION = 0;
    const int COMPRESS_LEVEL_BEST_SPEED = 1;
    const int COMPRESS_LEVEL_BEST_COMPRESSION = 9;
    const int COMPRESS_LEVEL_DEFAULT_COMPRESSION = -1;

    napi_value compressLevel = nullptr;
    napi_create_object(env, &compressLevel);
    SetNamedProperty(env, compressLevel, "COMPRESS_LEVEL_NO_COMPRESSION", COMPRESS_LEVEL_NO_COMPRESSION);
    SetNamedProperty(env, compressLevel, "COMPRESS_LEVEL_BEST_SPEED", COMPRESS_LEVEL_BEST_SPEED);
    SetNamedProperty(env, compressLevel, "COMPRESS_LEVEL_BEST_COMPRESSION", COMPRESS_LEVEL_BEST_COMPRESSION);
    SetNamedProperty(env, compressLevel, "COMPRESS_LEVEL_DEFAULT_COMPRESSION", COMPRESS_LEVEL_DEFAULT_COMPRESSION);

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_PROPERTY("CompressLevel", compressLevel),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties));

    return exports;
}
/**
 * @brief CompressStrategy data initialization.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value CompressStrategyInit(napi_env env, napi_value exports)
{
    LOGD("called.");
    const int COMPRESS_STRATEGY_DEFAULT_STRATEGY = 0;
    const int COMPRESS_STRATEGY_FILTERED = 1;
    const int COMPRESS_STRATEGY_HUFFMAN_ONLY = 2;
    const int COMPRESS_STRATEGY_RLE = 3;
    const int COMPRESS_STRATEGY_FIXED = 4;

    napi_value compressStrategy = nullptr;
    napi_create_object(env, &compressStrategy);
    SetNamedProperty(env, compressStrategy, "COMPRESS_STRATEGY_DEFAULT_STRATEGY", COMPRESS_STRATEGY_DEFAULT_STRATEGY);
    SetNamedProperty(env, compressStrategy, "COMPRESS_STRATEGY_FILTERED", COMPRESS_STRATEGY_FILTERED);
    SetNamedProperty(env, compressStrategy, "COMPRESS_STRATEGY_HUFFMAN_ONLY", COMPRESS_STRATEGY_HUFFMAN_ONLY);
    SetNamedProperty(env, compressStrategy, "COMPRESS_STRATEGY_RLE", COMPRESS_STRATEGY_RLE);
    SetNamedProperty(env, compressStrategy, "COMPRESS_STRATEGY_FIXED", COMPRESS_STRATEGY_FIXED);

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_PROPERTY("CompressStrategy", compressStrategy),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties));

    return exports;
}
/**
 * @brief MemLevel data initialization.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value MemLevelInit(napi_env env, napi_value exports)
{
    LOGD("called.");
    const int MEM_LEVEL_MIN = 1;
    const int MEM_LEVEL_DEFAULT = 8;
    const int MEM_LEVEL_MAX = 9;

    napi_value memLevel = nullptr;
    napi_create_object(env, &memLevel);
    SetNamedProperty(env, memLevel, "MEM_LEVEL_MIN", MEM_LEVEL_MIN);
    SetNamedProperty(env, memLevel, "MEM_LEVEL_DEFAULT", MEM_LEVEL_DEFAULT);
    SetNamedProperty(env, memLevel, "MEM_LEVEL_MAX", MEM_LEVEL_MAX);

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_PROPERTY("MemLevel", memLevel),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties));

    return exports;
}
/**
 * @brief FeatureAbility NAPI module registration.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value ZlibInit(napi_env env, napi_value exports)
{
    LOGD("called");

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("compressFile", CompressFile),
        DECLARE_NAPI_FUNCTION("decompressFile", DecompressFile),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties));

    return exports;
}

AsyncZipCallbackInfo* CreateZipAsyncCallbackInfo(napi_env env)
{
    LOGD("called.");
    napi_status ret;
    napi_value global = 0;
    const napi_extended_error_info* errorInfo = nullptr;
    ret = napi_get_global(env, &global);
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        if (errorInfo == nullptr) {
            LOGE("errorInfo is null");
            return nullptr;
        }
        LOGE("get_global=%{public}d err:%{public}s", ret, errorInfo->error_message);
    }

    AsyncZipCallbackInfo* asyncCallbackInfo = new (std::nothrow) AsyncZipCallbackInfo {
        .asyncWork = nullptr,
        .zlibCallbackInfo = nullptr,
    };
    if (asyncCallbackInfo == nullptr) {
        LOGE("asyncCallbackInfo is null");
        return nullptr;
    }
    LOGI("end.");
    return asyncCallbackInfo;
}

napi_value UnwrapStringParam(std::string& str, napi_env env, napi_value argv)
{
    LOGD("called");
    // unwrap the param[0]
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_status rev = napi_typeof(env, argv, &valueType);
    if (rev != napi_ok) {
        return nullptr;
    }

    if (valueType != napi_valuetype::napi_string) {
        LOGI("Parameter type does not match");
        return nullptr;
    }

    size_t len;
    napi_status status = napi_get_value_string_utf8(env, argv, nullptr, 0, &len);
    if (status != napi_ok) {
        LOGI("Get locale tag length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv, buf.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGI("Get locale tag failed");
        return nullptr;
    }
    str = std::string(buf.data());

    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, 1, &result));
    return result;
}

bool UnwrapOptionsParams(OPTIONS& options, napi_env env, napi_value arg)
{
    if (!IsTypeForNapiValue(env, arg, napi_object)) {
        return false;
    }
    napi_valuetype jsValueType = napi_undefined;
    napi_value jsProNameList = nullptr;
    uint32_t jsProCount = 0;

    NAPI_CALL_BASE(env, napi_get_property_names(env, arg, &jsProNameList), false);
    NAPI_CALL_BASE(env, napi_get_array_length(env, jsProNameList, &jsProCount), false);

    napi_value jsProName = nullptr;
    napi_value jsProValue = nullptr;

    for (uint32_t index = 0; index < jsProCount; index++) {
        NAPI_CALL_BASE(env, napi_get_element(env, jsProNameList, index, &jsProName), false);
        std::string strProName = UnwrapStringFromJS(env, jsProName, std::string());
        LOGI("Property name = %{public}s.", strProName.c_str());
        NAPI_CALL_BASE(env, napi_get_named_property(env, arg, strProName.c_str(), &jsProValue), false);
        NAPI_CALL_BASE(env, napi_typeof(env, jsProValue, &jsValueType), false);

        int ret = 0;
        if (strProName == std::string("flush")) {
            if (UnwrapIntValue(env, jsProValue, ret)) {
                options.flush = static_cast<FLUSH_TYPE>(ret);
            }
        } else if (strProName == std::string("finishFlush")) {
            if (UnwrapIntValue(env, jsProValue, ret)) {
                options.finishFlush = static_cast<FLUSH_TYPE>(ret);
            }
        } else if (strProName == std::string("chunkSize")) {
            if (UnwrapIntValue(env, jsProValue, ret)) {
                options.chunkSize = ret;
            }
        } else if (strProName == std::string("level")) {
            if (UnwrapIntValue(env, jsProValue, ret)) {
                COMPRESS_LEVE_CHECK(ret, false)
                options.level = static_cast<COMPRESS_LEVEL>(ret);
            }
        } else if (strProName == std::string("memLevel")) {
            if (UnwrapIntValue(env, jsProValue, ret)) {
                COMPRESS_MEM_CHECK(ret, false)
                options.memLevel = static_cast<MEMORY_LEVEL>(ret);
            }
        } else if (strProName == std::string("strategy")) {
            if (UnwrapIntValue(env, jsProValue, ret)) {
                COMPRESS_STRATEGY_CHECK(ret, false)
                options.strategy = static_cast<COMPRESS_STRATEGY>(ret);
            }
        }
    }
    return true;
}

// interface for v9
bool InitParam(CallZipUnzipParam& param, napi_env env, NapiArg& args, bool isZipFile)
{
    if (args.GetMaxArgc() < PARAM2) {
        return false;
    }
    for (size_t i = 0; i < args.GetMaxArgc(); ++i) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, args[i], &valueType);
        if (i == ARGS_POS_ZERO) {
            if (UnwrapStringParam(param.src, env, args[i]) == nullptr) {
                return false;
            }
        } else if (i == ARGS_POS_ONE) {
            if (UnwrapStringParam(param.dest, env, args[i]) == nullptr) {
                return false;
            }
        } else if (i == ARGS_POS_TWO) {
            if (isZipFile && valueType != napi_function && !UnwrapOptionsParams(param.options, env, args[i])) {
                return false;
            }
        } else if (i == ARGS_POS_THREE) {
            if (valueType == napi_function) {
                break;
            }
        } else {
            return false;
        }
    }
    return true;
}

void CompressExcute(napi_env env, AsyncZipCallbackInfo* asyncZipCallbackInfo)
{
    if (asyncZipCallbackInfo == nullptr) {
        LOGE("asyncZipCallbackInfo is nullptr");
        return;
    }
    std::unique_ptr<AsyncZipCallbackInfo> callbackPtr { asyncZipCallbackInfo };
    if (asyncZipCallbackInfo->zlibCallbackInfo == nullptr) {
        LOGE("zlibCallbackInfo is nullptr");
        return;
    }

    Zip(asyncZipCallbackInfo->param.src, asyncZipCallbackInfo->param.dest, false, asyncZipCallbackInfo->param.options,
        asyncZipCallbackInfo->zlibCallbackInfo);
}

napi_value CompressFile(napi_env env, napi_callback_info info)
{
    LOGD("napi begin CompressFile");
    NapiArg args(env, info);
    if (!args.Init(ARGS_SIZE_THREE, ARGS_SIZE_FOUR)) {
        LOGE("init args failed!");
        BusinessError::ThrowError(env, ERROR_PARAM_CHECK_ERROR, WRONG_PARAM);
        return nullptr;
    }
    CallZipUnzipParam param;
    if (!InitParam(param, env, args, true)) {
        LOGE("Init Param failed!");
        BusinessError::ThrowError(env, ERROR_PARAM_CHECK_ERROR, WRONG_PARAM);
        return nullptr;
    }
    AsyncZipCallbackInfo* asyncZipCallbackInfo = CreateZipAsyncCallbackInfo(env);
    if (asyncZipCallbackInfo == nullptr) {
        LOGE("asyncZipCallbackInfo nullptr!");
        return nullptr;
    }
    asyncZipCallbackInfo->param = param;
    std::unique_ptr<AsyncZipCallbackInfo> callbackPtr { asyncZipCallbackInfo };
    asyncZipCallbackInfo->zlibCallbackInfo = std::make_shared<ZlibCallbackInfo>(env, nullptr, nullptr, false);
    asyncZipCallbackInfo->zlibCallbackInfo->SetDeliverErrCode(true);
    if (args.GetMaxArgc() > PARAM3) {
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL_BASE(env, napi_typeof(env, args[PARAM3], &valuetype), nullptr);
        if (valuetype == napi_function) {
            napi_ref callbackRef = nullptr;
            NAPI_CALL(env, napi_create_reference(env, args[PARAM3], NAPI_RETURN_ONE, &callbackRef));
            asyncZipCallbackInfo->zlibCallbackInfo->SetCallback(callbackRef);
            asyncZipCallbackInfo->zlibCallbackInfo->SetIsCallback(true);
        }
    }
    napi_value promise = nullptr;
    if (!asyncZipCallbackInfo->zlibCallbackInfo->GetIsCallback()) {
        napi_deferred deferred;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncZipCallbackInfo->zlibCallbackInfo->SetDeferred(deferred);
    }
    CompressExcute(env, asyncZipCallbackInfo);
    callbackPtr.release();
    return promise;
}

void DecompressExcute(napi_env env, AsyncZipCallbackInfo* asyncZipCallbackInfo)
{
    if (asyncZipCallbackInfo == nullptr) {
        LOGE("asyncZipCallbackInfo is nullptr");
        return;
    }
    std::unique_ptr<AsyncZipCallbackInfo> callbackPtr { asyncZipCallbackInfo };
    if (asyncZipCallbackInfo->zlibCallbackInfo == nullptr) {
        LOGE("zlibCallbackInfo is nullptr");
        return;
    }
    Unzip(asyncZipCallbackInfo->param.src, asyncZipCallbackInfo->param.dest, asyncZipCallbackInfo->param.options,
        asyncZipCallbackInfo->zlibCallbackInfo);
}

napi_value DecompressFile(napi_env env, napi_callback_info info)
{
    LOGD("napi begin DecompressFile");
    NapiArg args(env, info);
    if (!args.Init(ARGS_SIZE_TWO, ARGS_SIZE_FOUR)) {
        BusinessError::ThrowError(env, ERROR_PARAM_CHECK_ERROR, WRONG_PARAM);
        return nullptr;
    }
    CallZipUnzipParam param;
    if (!InitParam(param, env, args, true)) {
        BusinessError::ThrowError(env, ERROR_PARAM_CHECK_ERROR, WRONG_PARAM);
        return nullptr;
    }
    AsyncZipCallbackInfo* asyncZipCallbackInfo = CreateZipAsyncCallbackInfo(env);
    if (asyncZipCallbackInfo == nullptr) {
        return nullptr;
    }
    asyncZipCallbackInfo->param = param;
    std::unique_ptr<AsyncZipCallbackInfo> callbackPtr { asyncZipCallbackInfo };
    asyncZipCallbackInfo->zlibCallbackInfo = std::make_shared<ZlibCallbackInfo>(env, nullptr, nullptr, false);
    asyncZipCallbackInfo->zlibCallbackInfo->SetDeliverErrCode(true);
    for (size_t i = PARAM2; i < args.GetMaxArgc(); i++) {
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL_BASE(env, napi_typeof(env, args[i], &valuetype), nullptr);
        if (valuetype == napi_function) {
            napi_ref callbackRef = nullptr;
            NAPI_CALL(env, napi_create_reference(env, args[i], NAPI_RETURN_ONE, &callbackRef));
            asyncZipCallbackInfo->zlibCallbackInfo->SetCallback(callbackRef);
            asyncZipCallbackInfo->zlibCallbackInfo->SetIsCallback(true);
            break;
        }
    }
    napi_value promise = nullptr;
    if (!asyncZipCallbackInfo->zlibCallbackInfo->GetIsCallback()) {
        napi_deferred deferred;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncZipCallbackInfo->zlibCallbackInfo->SetDeferred(deferred);
    }
    DecompressExcute(env, asyncZipCallbackInfo);
    callbackPtr.release();
    return promise;
}

} // namespace LIBZIP
} // namespace AppExecFwk
} // namespace OHOS