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

#include "napi_publish.h"

#include "ans_inner_errors.h"

namespace OHOS {
namespace NotificationNapi {
namespace {
constexpr int8_t PUBLISH_NOTIFICATION_MAX = 3;
}

napi_value GetCallback(const napi_env &env, const napi_value &value, ParametersInfoPublish &params)
{
    LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    if (valuetype != napi_function) {
        LOGW("Callback is not function excute promise.");
        return Common::NapiGetNull(env);
    }
    napi_create_reference(env, value, 1, &params.callback);
    LOGD("end");
    return Common::NapiGetNull(env);
}

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, ParametersInfoPublish &params)
{
    LOGD("enter");

    size_t argc = PUBLISH_NOTIFICATION_MAX;
    napi_value argv[PUBLISH_NOTIFICATION_MAX] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc < 1) {
        LOGW("Wrong number of arguments.");
        Common::NapiThrow(env, ERROR_PARAM_INVALID, MANDATORY_PARAMETER_ARE_LEFT_UNSPECIFIED);
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[PARAM0], &valuetype));
    if (valuetype != napi_object) {
        LOGW("Argument type error. Object expected.");
        std::string msg = "Incorrect parameter types.The type of param must be object.";
        Common::NapiThrow(env, ERROR_PARAM_INVALID, msg);
        return nullptr;
    }

    // argv[0] : NotificationRequest
    if (Common::GetNotificationRequest(env, argv[PARAM0], params.request) == nullptr) {
        return nullptr;
    }

    // argv[1] : userId / callback
    if (argc >= PUBLISH_NOTIFICATION_MAX - 1) {
        NAPI_CALL(env, napi_typeof(env, argv[PARAM1], &valuetype));
        if ((valuetype != napi_number) && (valuetype != napi_function)) {
            LOGW("Wrong argument type. Function or object expected. Execute promise");
            return Common::NapiGetNull(env);
        }

        if (valuetype != napi_number) {
            napi_create_reference(env, argv[PARAM1], 1, &params.callback);
        }
    }

    // argv[2] : callback
    if (argc >= PUBLISH_NOTIFICATION_MAX) {
        if (GetCallback(env, argv[PARAM2], params) == nullptr) {
            return nullptr;
        }
    }

    LOGD("end");
    return Common::NapiGetNull(env);
}

napi_value NapiPublish(napi_env env, napi_callback_info info)
{
    LOGD("enter");
    ParametersInfoPublish params;
    if (ParseParameters(env, info, params) == nullptr) {
        Common::NapiThrow(env, ERROR_PARAM_INVALID);
        return Common::NapiGetUndefined(env);
    }

    napi_value promise = nullptr;
    auto asynccallbackinfo = new (std::nothrow) AsyncCallbackInfoPublish {.env = env, .asyncWork = nullptr};
    if (!asynccallbackinfo) {
       LOGD("asynccallbackinfo is nullptr.");
        return Common::JSParaError(env, params.callback);
    }
    asynccallbackinfo->request = params.request;
    Common::PaddingCallbackPromiseInfo(env, params.callback, asynccallbackinfo->info, promise);
    NotificationHelper::PublishNotification(asynccallbackinfo->request, static_cast<void*>(asynccallbackinfo), [](void* data, int32_t code) {
        AsyncCallbackInfoPublish *asynccallbackinfo = static_cast<AsyncCallbackInfoPublish *>(data);
        if (asynccallbackinfo) {
            napi_env infoEnv = asynccallbackinfo->env;
            asynccallbackinfo->info.errorCode = code;
            Common::CreateReturnValue(infoEnv, asynccallbackinfo->info, Common::NapiGetNull(infoEnv));
            if (asynccallbackinfo->info.callback != nullptr) {
                LOGD("Delete napiPublish callback reference.");
                napi_delete_reference(infoEnv, asynccallbackinfo->info.callback);
            }
            napi_delete_async_work(infoEnv, asynccallbackinfo->asyncWork);
            delete asynccallbackinfo;
            asynccallbackinfo = nullptr;
        }
    });

    bool isCallback = asynccallbackinfo->info.isCallback;
    if (isCallback) {
        LOGD("napiPublish callback is nullptr.");
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}

}  // namespace NotificationNapi
}  // namespace OHOS