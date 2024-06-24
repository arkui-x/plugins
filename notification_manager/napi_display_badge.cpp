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

#include "napi_display_badge.h"

#include "ans_inner_errors.h"

namespace OHOS {
namespace NotificationNapi {
const int32_t SET_BADGE_NUMBER_MAX_PARA = 2;
const int32_t SET_BADGE_NUMBER_MIN_PARA = 1;

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, SetBadgeNumberParams &params)
{
    LOGD("enter");

    size_t argc = SET_BADGE_NUMBER_MAX_PARA;
    napi_value argv[SET_BADGE_NUMBER_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    if (argc < SET_BADGE_NUMBER_MIN_PARA) {
        LOGW("Wrong number of arguments.");
        Common::NapiThrow(env, ERROR_PARAM_INVALID, MANDATORY_PARAMETER_ARE_LEFT_UNSPECIFIED);
        return nullptr;
    }

    // argv[0]: badgeNumber
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[PARAM0], &valuetype));

    // setBadgeNumber(badgeNumber)
    if (valuetype != napi_number) {
        LOGW("Wrong argument type. Number expected.");
        std::string msg = "Incorrect parameter types.The type of param must be number.";
        Common::NapiThrow(env, ERROR_PARAM_INVALID, msg);
        return nullptr;
    }
    napi_get_value_int32(env, argv[PARAM0], &params.badgeNumber);

    // argv[1]:callback
    if (argc >= SET_BADGE_NUMBER_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[PARAM1], &valuetype));
        if (valuetype != napi_function) {
            LOGW("Callback is not function excute promise.");
            return Common::NapiGetNull(env);
        }
        napi_create_reference(env, argv[PARAM1], 1, &params.callback);
    }

    return Common::NapiGetNull(env);
}

void AsyncCompleteCallbackNapiSetBadgeNumber(napi_env env, napi_status status, void *data)
{
    LOGD("enter");
    if (!data) {
        LOGE("Invalid async callback data");
        return;
    }
    AsyncCallbackSetBadgeNumber *asynccallbackinfo = static_cast<AsyncCallbackSetBadgeNumber *>(data);
    Common::CreateReturnValue(env, asynccallbackinfo->info, Common::NapiGetNull(env));
    if (asynccallbackinfo->info.callback != nullptr) {
        napi_delete_reference(env, asynccallbackinfo->info.callback);
    }
    napi_delete_async_work(env, asynccallbackinfo->asyncWork);
    delete asynccallbackinfo;
    asynccallbackinfo = nullptr;
}

napi_value NapiSetBadgeNumber(napi_env env, napi_callback_info info)
{
    LOGD("enter");
    SetBadgeNumberParams params {};
    if (ParseParameters(env, info, params) == nullptr) {
        Common::NapiThrow(env, ERROR_PARAM_INVALID);
        return Common::NapiGetUndefined(env);
    }

    AsyncCallbackSetBadgeNumber *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackSetBadgeNumber {.env = env, .asyncWork = nullptr, .params = params};
    if (!asynccallbackinfo) {
        Common::NapiThrow(env, ERROR_PARAM_INVALID);
        return Common::NapiGetUndefined(env);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, params.callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "setBadgeNumber", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            LOGI("NapiSetBadgeNumber work excute.");
            AsyncCallbackSetBadgeNumber *asynccallbackinfo = static_cast<AsyncCallbackSetBadgeNumber *>(data);
            if (asynccallbackinfo) {
                LOGI("option.badgeNumber: %{public}d", asynccallbackinfo->params.badgeNumber);
                asynccallbackinfo->info.errorCode = NotificationHelper::SetBadgeNumber(
                    asynccallbackinfo->params.badgeNumber);
            }
        },
        AsyncCompleteCallbackNapiSetBadgeNumber,
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    bool isCallback = asynccallbackinfo->info.isCallback;
    napi_queue_async_work_with_qos(env, asynccallbackinfo->asyncWork, napi_qos_user_initiated);

    if (isCallback) {
        LOGD("napiSetBadgeNumber callback is nullptr.");
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}
}  // namespace NotificationNapi
}  // namespace OHOS