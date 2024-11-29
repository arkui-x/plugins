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

#include "napi_enable_notification.h"
#include "napi_base_context.h"
#include "ans_inner_errors.h"

namespace OHOS {
namespace NotificationNapi {
const int IS_NOTIFICATION_ENABLE_MAX_PARA = 1;
const int REQUEST_ENABLE_NOTIFICATION_MAX_PARA = 2;

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, IsEnableParams &params)
{
    LOGD("enter");

    size_t argc = IS_NOTIFICATION_ENABLE_MAX_PARA;
    napi_value argv[IS_NOTIFICATION_ENABLE_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    if (argc == 0) {
        return Common::NapiGetNull(env);
    }

    // argv[0]: bundle / userId / callback
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[PARAM0], &valuetype));
    if (valuetype != napi_function) {
        LOGW("Parameter type error. Function or object expected. Excute promise.");
        return Common::NapiGetNull(env);
    }

    napi_create_reference(env, argv[PARAM0], 1, &params.callback);
    return Common::NapiGetNull(env);
}

void AsyncCompleteCallbackNapiIsNotificationEnabled(napi_env env, void *data)
{
    LOGD("enter");
    if (!data) {
        LOGE("Invalid async callback data");
        return;
    }
    AsyncCallbackInfoIsEnable *asynccallbackinfo = static_cast<AsyncCallbackInfoIsEnable *>(data);
    if (asynccallbackinfo) {
        napi_value result = nullptr;
        napi_get_boolean(env, asynccallbackinfo->allowed, &result);
        Common::CreateReturnValue(env, asynccallbackinfo->info, result);
        if (asynccallbackinfo->info.callback != nullptr) {
            napi_delete_reference(env, asynccallbackinfo->info.callback);
        }
        napi_delete_async_work(env, asynccallbackinfo->asyncWork);
        delete asynccallbackinfo;
        asynccallbackinfo = nullptr;
    }
}

__attribute__((no_sanitize("cfi"))) napi_value NapiIsNotificationEnabled(napi_env env, napi_callback_info info)
{
    LOGD("enter");
    IsEnableParams params {};
    if (ParseParameters(env, info, params) == nullptr) {
        LOGD("ParseParameters is nullptr.");
        Common::NapiThrow(env, ERROR_PARAM_INVALID);
        return Common::NapiGetUndefined(env);
    }

    AsyncCallbackInfoIsEnable *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoIsEnable {.env = env, .asyncWork = nullptr, .params = params};
    if (!asynccallbackinfo) {
        LOGD("Asynccallbackinfo is nullptr.");
        return Common::JSParaError(env, params.callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, params.callback, asynccallbackinfo->info, promise);
    NotificationHelper::IsAllowedNotifySelf(static_cast<void*>(asynccallbackinfo), [](void* data, int32_t code, bool isEnable) {
        auto* asynccallbackinfo = static_cast<AsyncCallbackInfoIsEnable*>(data);
        asynccallbackinfo->info.errorCode = code;
        asynccallbackinfo->allowed = isEnable;
        AsyncCompleteCallbackNapiIsNotificationEnabled(
            asynccallbackinfo->env, static_cast<void*>(asynccallbackinfo));
    });

    bool isCallback = asynccallbackinfo->info.isCallback;
    if (isCallback) {
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}

void NapiAsyncCompleteCallbackRequestEnableNotification(napi_env env, void *data)
{
    LOGD("enter");
    if (data == nullptr) {
        LOGE("Invalid async callback data.");
        return;
    }
    auto* asynccallbackinfo = static_cast<AsyncCallbackInfoIsEnable*>(data);
    Common::CreateReturnValue(env, asynccallbackinfo->info, Common::NapiGetNull(env));
    if (asynccallbackinfo->info.callback != nullptr) {
        napi_delete_reference(env, asynccallbackinfo->info.callback);
    }
    napi_delete_async_work(env, asynccallbackinfo->asyncWork);
    delete asynccallbackinfo;
}

napi_value NapiRequestEnableNotification(napi_env env, napi_callback_info info)
{
    LOGD("enter");
    IsEnableParams params {};
    if (ParseRequestEnableParameters(env, info, params) == nullptr) {
        Common::NapiThrow(env, ERROR_PARAM_INVALID);
        return Common::NapiGetUndefined(env);
    }

    AsyncCallbackInfoIsEnable *asynccallbackinfo = new (std::nothrow) AsyncCallbackInfoIsEnable {
            .env = env, .params = params, .newInterface = true};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, params.callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, params.callback, asynccallbackinfo->info, promise);

    NotificationHelper::RequestEnableNotification(static_cast<void*>(asynccallbackinfo), [](void* data, int32_t code) {
        auto* asynccallbackinfo = static_cast<AsyncCallbackInfoIsEnable*>(data);
        asynccallbackinfo->info.errorCode = code;
        NapiAsyncCompleteCallbackRequestEnableNotification(
            asynccallbackinfo->env, static_cast<void*>(asynccallbackinfo));
    });

    bool isCallback = asynccallbackinfo->info.isCallback;
    if (isCallback) {
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value ParseRequestEnableParameters(const napi_env &env, const napi_callback_info &info, IsEnableParams &params)
{
    LOGD("enter");

    size_t argc = REQUEST_ENABLE_NOTIFICATION_MAX_PARA;
    napi_value argv[REQUEST_ENABLE_NOTIFICATION_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    if (argc == 0) {
        return Common::NapiGetNull(env);
    }

    // argv[0]: context / callback
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[PARAM0], &valuetype));
    if ((valuetype != napi_object) && (valuetype != napi_function)) {
        LOGW("Wrong argument type. Function or object expected. Excute promise.");
        return Common::NapiGetNull(env);
    }
    if (valuetype == napi_object) {
        bool stageMode = false;
        napi_status status = AbilityRuntime::Platform::IsStageContext(env, argv[PARAM0], stageMode);
        if (status != napi_ok || !stageMode) {
            LOGE("Only support stage mode");
            std::string msg = "Incorrect parameter types.Only support stage mode.";
            Common::NapiThrow(env, ERROR_PARAM_INVALID, msg);
            return nullptr;
        }
    } else {
        napi_create_reference(env, argv[PARAM0], 1, &params.callback);
    }
    // argv[1]:callback
    if (argc >= REQUEST_ENABLE_NOTIFICATION_MAX_PARA && valuetype == napi_object) {
        NAPI_CALL(env, napi_typeof(env, argv[PARAM1], &valuetype));
        if (valuetype != napi_function) {
            LOGW("Callback is not function excute promise.");
            return Common::NapiGetNull(env);
        }
        napi_create_reference(env, argv[PARAM1], 1, &params.callback);
    }

    return Common::NapiGetNull(env);
}
}  // namespace NotificationNapi
}  // namespace OHOS
