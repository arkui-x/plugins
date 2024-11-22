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

#include "napi_cancel.h"
#include "ans_inner_errors.h"

namespace OHOS {
namespace NotificationNapi {
constexpr int8_t CANCEL_MAX_PARA = 3;
constexpr int8_t CANCEL_GROUP_MAX_PARA = 2;
constexpr int8_t CANCEL_GROUP_MIN_PARA = 1;

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, ParametersInfoCancel &paras)
{
    LOGD("enter");

    size_t argc = CANCEL_MAX_PARA;
    napi_value argv[CANCEL_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    if (argc < 1) {
        LOGW("Wrong number of arguments");
        Common::NapiThrow(env, ERROR_PARAM_INVALID, MANDATORY_PARAMETER_ARE_LEFT_UNSPECIFIED);
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    // argv[0]: id: number / representativeBundle: BundleOption
    NAPI_CALL(env, napi_typeof(env, argv[PARAM0], &valuetype));
    if (valuetype != napi_number && valuetype != napi_object) {
        LOGW("Wrong argument type. Number object expected.");
        std::string msg = "Incorrect parameter types.The type of param must be number or object.";
        Common::NapiThrow(env, ERROR_PARAM_INVALID, msg);
        return nullptr;
    }

    if (valuetype == napi_number) {
        NAPI_CALL(env, napi_get_value_int32(env, argv[PARAM0], &paras.id));
    }

    // argv[1]: label: string / callback / id : number
    if (argc >= CANCEL_MAX_PARA - 1 && !paras.hasOption) {
        NAPI_CALL(env, napi_typeof(env, argv[PARAM1], &valuetype));
        if (valuetype == napi_undefined || valuetype == napi_null) {
            return Common::NapiGetNull(env);
        }
        if (valuetype != napi_number && valuetype != napi_boolean &&
            valuetype != napi_string && valuetype != napi_function) {
            LOGW("Wrong argument type. String or function expected.");
            std::string msg = "Incorrect parameter types.The type of param must be string or function.";
            Common::NapiThrow(env, ERROR_PARAM_INVALID, msg);
            return nullptr;
        }
        if (valuetype == napi_number) {
            int64_t number = 0;
            NAPI_CALL(env, napi_get_value_int64(env, argv[PARAM1], &number));
            paras.label = std::to_string(number);
        } else if (valuetype == napi_boolean) {
            bool result = false;
            NAPI_CALL(env, napi_get_value_bool(env, argv[PARAM1], &result));
            paras.label = std::to_string(result);
        } else if (valuetype == napi_string) {
            char str[STR_MAX_SIZE] = {0};
            size_t strLen = 0;
            NAPI_CALL(env, napi_get_value_string_utf8(env, argv[PARAM1], str, STR_MAX_SIZE - 1, &strLen));
            paras.label = str;
        } else {
            napi_create_reference(env, argv[PARAM1], 1, &paras.callback);
        }
    } else if (argc >= CANCEL_MAX_PARA - 1 && paras.hasOption) {
        NAPI_CALL(env, napi_typeof(env, argv[PARAM1], &valuetype));
        if (valuetype != napi_number) {
            LOGW("Wrong argument type. Number expected.");
            std::string msg = "Incorrect parameter types.The type of param must be number.";
            Common::NapiThrow(env, ERROR_PARAM_INVALID, msg);
            return nullptr;
        }
        NAPI_CALL(env, napi_get_value_int32(env, argv[PARAM1], &paras.id));
    }

    // argv[2]: callback
    if (argc >= CANCEL_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[PARAM2], &valuetype));
        if (valuetype != napi_function) {
            LOGW("Callback is not function excute promise.");
            return Common::NapiGetNull(env);
        }
        napi_create_reference(env, argv[PARAM2], 1, &paras.callback);
    }

    return Common::NapiGetNull(env);
}

napi_value NapiCancel(napi_env env, napi_callback_info info)
{
    LOGD("enter");
    ParametersInfoCancel paras;
    if (ParseParameters(env, info, paras) == nullptr) {
        Common::NapiThrow(env, ERROR_PARAM_INVALID);
        return Common::NapiGetUndefined(env);
    }

    AsyncCallbackInfoCancel *asynccallbackinfo = new (std::nothrow) AsyncCallbackInfoCancel {
        .env = env,
        .asyncWork = nullptr,
        .id = paras.id,
        .label = paras.label,
        // .option = paras.option,
        .hasOption = paras.hasOption
    };
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, paras.callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, paras.callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "cancel", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            LOGI("NapiCancel work excute.");
            AsyncCallbackInfoCancel *asynccallbackinfo = static_cast<AsyncCallbackInfoCancel *>(data);

            if (asynccallbackinfo) {
                asynccallbackinfo->info.errorCode =
                    NotificationHelper::CancelNotification(asynccallbackinfo->label, asynccallbackinfo->id);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            LOGI("NapiCancel work complete.");
            AsyncCallbackInfoCancel *asynccallbackinfo = static_cast<AsyncCallbackInfoCancel *>(data);
            if (asynccallbackinfo) {
                Common::CreateReturnValue(env, asynccallbackinfo->info, Common::NapiGetNull(env));
                if (asynccallbackinfo->info.callback != nullptr) {
                    LOGD("Delete napiCancel callback reference.");
                    napi_delete_reference(env, asynccallbackinfo->info.callback);
                }
                napi_delete_async_work(env, asynccallbackinfo->asyncWork);
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
            LOGD("NapiCancel work complete end.");
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    bool isCallback = asynccallbackinfo->info.isCallback;
    napi_queue_async_work_with_qos(env, asynccallbackinfo->asyncWork, napi_qos_user_initiated);

    if (isCallback) {
        LOGD("napiCancel callback is nullptr.");
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value NapiCancelAll(napi_env env, napi_callback_info info)
{
    LOGD("enter");
    napi_ref callback = nullptr;
    if (Common::ParseParaOnlyCallback(env, info, callback) == nullptr) {
        Common::NapiThrow(env, ERROR_PARAM_INVALID);
        return Common::NapiGetUndefined(env);
    }

    auto asynccallbackinfo = new (std::nothrow) AsyncCallbackInfoCancel {.env = env, .asyncWork = nullptr};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "cancelAll", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            LOGI("NapiCancelAll work excute.");
            AsyncCallbackInfoCancel *asynccallbackinfo = static_cast<AsyncCallbackInfoCancel *>(data);
            if (asynccallbackinfo) {
                asynccallbackinfo->info.errorCode = NotificationHelper::CancelAllNotifications();
            }
        },
        [](napi_env env, napi_status status, void *data) {
            LOGI("NapiCancelAll work complete.");
            AsyncCallbackInfoCancel *asynccallbackinfo = static_cast<AsyncCallbackInfoCancel *>(data);
            if (asynccallbackinfo) {
                Common::CreateReturnValue(env, asynccallbackinfo->info, Common::NapiGetNull(env));
                if (asynccallbackinfo->info.callback != nullptr) {
                    LOGD("Delete napiCancelAll callback reference.");
                    napi_delete_reference(env, asynccallbackinfo->info.callback);
                }
                napi_delete_async_work(env, asynccallbackinfo->asyncWork);
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
            LOGD("NapiCancelAll work complete end.");
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    bool isCallback = asynccallbackinfo->info.isCallback;
    napi_queue_async_work_with_qos(env, asynccallbackinfo->asyncWork, napi_qos_user_initiated);

    if (isCallback) {
        LOGD("napiCancelAll callback is nullptr.");
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}
}  // namespace NotificationNapi
}  // namespace OHOS