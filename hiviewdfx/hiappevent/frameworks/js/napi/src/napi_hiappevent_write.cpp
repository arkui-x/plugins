/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "napi_hiappevent_write.h"

#include <map>

#include "hiappevent_base.h"
#include "hiappevent_write.h"
#include "hilog/log.h"
#include "napi_error.h"
#include "napi_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace NapiHiAppEventWrite {
namespace {
constexpr size_t ERR_INDEX = 0;
constexpr size_t VALUE_INDEX = 1;
constexpr size_t RESULT_SIZE = 2;

napi_value BuildErrorByResult(const napi_env env, int result)
{
    const std::map<int, std::pair<int, std::string>> errMap = {
        { ErrorCode::ERROR_INVALID_EVENT_NAME, { NapiError::ERR_INVALID_NAME, "Invalid event name." } },
        { ErrorCode::ERROR_INVALID_EVENT_DOMAIN, { NapiError::ERR_INVALID_DOMAIN, "Invalid event domain." } },
        { ErrorCode::ERROR_HIAPPEVENT_DISABLE, { NapiError::ERR_DISABLE, "Function disabled." } },
        { ErrorCode::ERROR_INVALID_PARAM_NAME, { NapiError::ERR_INVALID_KEY, "Invalid event parameter name." } },
        { ErrorCode::ERROR_INVALID_PARAM_VALUE_LENGTH,
            { NapiError::ERR_INVALID_STR_LEN, "Invalid string length of the event parameter." } },
        { ErrorCode::ERROR_INVALID_PARAM_NUM,
            { NapiError::ERR_INVALID_PARAM_NUM, "Invalid number of event parameters." } },
        { ErrorCode::ERROR_INVALID_LIST_PARAM_SIZE,
            { NapiError::ERR_INVALID_ARR_LEN, "Invalid array length of the event parameter." } },
        { ErrorCode::ERROR_INVALID_CUSTOM_PARAM_NUM,
            { NapiError::ERR_INVALID_CUSTOM_PARAM_NUM, "The number of parameter keys exceeds the limit." } },
    };
    return errMap.find(result) == errMap.end()
               ? NapiUtil::CreateNull(env)
               : NapiUtil::CreateError(env, errMap.at(result).first, errMap.at(result).second);
}
} // namespace

void Write(const napi_env env, HiAppEventAsyncContext* asyncContext)
{
    if (asyncContext == nullptr) {
        HILOG_ERROR(LOG_CORE, "asyncContext is null");
        return;
    }
    napi_value resource = NapiUtil::CreateString(env, "NapiHiAppEventWriter");
    napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void* data) {
            HiAppEventAsyncContext* asyncContext = (HiAppEventAsyncContext*)data;
            if (asyncContext->appEventPack != nullptr && asyncContext->result >= 0) {
                WriteEvent(asyncContext->appEventPack);
            }
        },
        [](napi_env env, napi_status status, void* data) {
            HiAppEventAsyncContext* asyncContext = (HiAppEventAsyncContext*)data;
            napi_value results[RESULT_SIZE] = { 0 };
            if (asyncContext->result == 0) {
                results[ERR_INDEX] = NapiUtil::CreateNull(env);
                results[VALUE_INDEX] = NapiUtil::CreateInt32(env, asyncContext->result);
            } else {
                if (asyncContext->isV9) {
                    results[ERR_INDEX] = BuildErrorByResult(env, asyncContext->result);
                } else {
                    results[ERR_INDEX] =
                        NapiUtil::CreateObject(env, "code", NapiUtil::CreateInt32(env, asyncContext->result));
                }
                results[VALUE_INDEX] = NapiUtil::CreateNull(env);
            }

            if (asyncContext->deferred != nullptr) { // promise
                if (asyncContext->result == 0) {
                    napi_resolve_deferred(env, asyncContext->deferred, results[VALUE_INDEX]);
                } else {
                    napi_reject_deferred(env, asyncContext->deferred, results[ERR_INDEX]);
                }
            } else { // callback
                napi_value callback = nullptr;
                napi_get_reference_value(env, asyncContext->callback, &callback);
                napi_value retValue = nullptr;
                napi_call_function(env, nullptr, callback, RESULT_SIZE, results, &retValue);
                napi_delete_reference(env, asyncContext->callback);
            }
            napi_delete_async_work(env, asyncContext->asyncWork);
            delete asyncContext;
        },
        (void*)asyncContext, &asyncContext->asyncWork);
    napi_queue_async_work_with_qos(env, asyncContext->asyncWork, napi_qos_default);
}

void SetEventParam(const napi_env env, HiAppEventAsyncContext* asyncContext)
{
    if (asyncContext == nullptr) {
        HILOG_ERROR(LOG_CORE, "asyncContext is null");
        return;
    }
    napi_value resource = NapiUtil::CreateString(env, "NapiHiAppEventSetEventParam");
    napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void* data) {
            HiAppEventAsyncContext* asyncContext = (HiAppEventAsyncContext*)data;
            if (asyncContext->appEventPack != nullptr && asyncContext->result == 0) {
                if (auto ret = SetEventParam(asyncContext->appEventPack); ret > 0) {
                    asyncContext->result = ret;
                }
            }
        },
        [](napi_env env, napi_status status, void* data) {
            HiAppEventAsyncContext* asyncContext = (HiAppEventAsyncContext*)data;
            napi_value results[RESULT_SIZE] = { 0 };
            if (asyncContext != nullptr && asyncContext->deferred != nullptr) { // promise
                if (asyncContext->result == 0) {
                    results[ERR_INDEX] = NapiUtil::CreateNull(env);
                    results[VALUE_INDEX] = NapiUtil::CreateInt32(env, asyncContext->result);
                    napi_resolve_deferred(env, asyncContext->deferred, results[VALUE_INDEX]);
                } else {
                    results[ERR_INDEX] = BuildErrorByResult(env, asyncContext->result);
                    results[VALUE_INDEX] = NapiUtil::CreateNull(env);
                    napi_reject_deferred(env, asyncContext->deferred, results[ERR_INDEX]);
                }
            }
            napi_delete_async_work(env, asyncContext->asyncWork);
            delete asyncContext;
        },
        (void*)asyncContext, &asyncContext->asyncWork);
    napi_queue_async_work_with_qos(env, asyncContext->asyncWork, napi_qos_default);
}
} // namespace NapiHiAppEventWrite
} // namespace HiviewDFX
} // namespace OHOS
