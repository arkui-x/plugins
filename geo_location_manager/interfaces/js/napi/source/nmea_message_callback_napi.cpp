/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include "nmea_message_callback_napi.h"

#include "common_utils.h"
#include "location_log.h"
#include "napi_util.h"
#include "constant_definition.h"
#include "napi/native_api.h"
#include "napi/native_common.h"

namespace OHOS {
namespace Location {
static std::mutex g_regCallbackMutex;
static std::vector<napi_ref> g_registerCallbacks;
NmeaMessageCallbackNapi::NmeaMessageCallbackNapi()
{
    env_ = nullptr;
    handlerCb_ = nullptr;
    remoteDied_ = false;
}

NmeaMessageCallbackNapi::~NmeaMessageCallbackNapi()
{
    LBSLOGW(NMEA_MESSAGE_CALLBACK, "~NmeaMessageCallbackNapi()");
}

napi_env NmeaMessageCallbackNapi::GetEnv()
{
    std::unique_lock<std::mutex> guard(mutex_);
    return env_;
}

void NmeaMessageCallbackNapi::SetEnv(const napi_env& env)
{
    std::unique_lock<std::mutex> guard(mutex_);
    env_ = env;
}

napi_ref NmeaMessageCallbackNapi::GetHandleCb()
{
    std::unique_lock<std::mutex> guard(mutex_);
    return handlerCb_;
}

void NmeaMessageCallbackNapi::SetHandleCb(const napi_ref& handlerCb)
{
    {
        std::unique_lock<std::mutex> guard(mutex_);
        handlerCb_ = handlerCb;
    }
    std::unique_lock<std::mutex> guard(g_regCallbackMutex);
    g_registerCallbacks.emplace_back(handlerCb);
}

bool FindNmeaCallback(napi_ref cb)
{
    std::unique_lock<std::mutex> guard(g_regCallbackMutex);
    auto iter = std::find(g_registerCallbacks.begin(), g_registerCallbacks.end(), cb);
    if (iter == g_registerCallbacks.end()) {
        return false;
    }
    return true;
}

void DeleteNmeaCallback(napi_ref cb)
{
    std::unique_lock<std::mutex> guard(g_regCallbackMutex);
    for (auto iter = g_registerCallbacks.begin(); iter != g_registerCallbacks.end(); iter++) {
        if (*iter == cb) {
            iter = g_registerCallbacks.erase(iter);
            break;
        }
    }
    LBSLOGW(NMEA_MESSAGE_CALLBACK, "after DeleteNmeaCallback, callback size %{public}s",
        std::to_string(g_registerCallbacks.size()).c_str());
}

bool NmeaMessageCallbackNapi::IsRemoteDied()
{
    return remoteDied_;
}

napi_value NmeaMessageCallbackNapi::PackResult(const std::string msg)
{
    napi_value result;
    NAPI_CALL(env_, napi_create_string_utf8(env_, msg.c_str(), NAPI_AUTO_LENGTH, &result));
    return result;
}

bool NmeaMessageCallbackNapi::Send(const std::string msg)
{
    std::unique_lock<std::mutex> guard(mutex_);
    uv_loop_s *loop = nullptr;
    NAPI_CALL_BASE(env_, napi_get_uv_event_loop(env_, &loop), false);
    if (loop == nullptr) {
        LBSLOGE(NMEA_MESSAGE_CALLBACK, "loop == nullptr.");
        return false;
    }
    if (handlerCb_ == nullptr) {
        LBSLOGE(NMEA_MESSAGE_CALLBACK, "handler is nullptr.");
        return false;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LBSLOGE(NMEA_MESSAGE_CALLBACK, "work == nullptr.");
        return false;
    }
    NmeaAsyncContext *context = new (std::nothrow) NmeaAsyncContext(env_);
    if (context == nullptr) {
        LBSLOGE(NMEA_MESSAGE_CALLBACK, "context == nullptr.");
        delete work;
        return false;
    }
    if (!InitContext(context)) {
        LBSLOGE(NMEA_MESSAGE_CALLBACK, "InitContext fail");
        delete work;
        delete context;
        return false;
    }
    context->msg = msg;
    work->data = context;
    UvQueueWork(loop, work);
    return true;
}


void NmeaMessageCallbackNapi::UvQueueWork(uv_loop_s* loop, uv_work_t* work)
{
    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            NmeaAsyncContext *context = nullptr;
            napi_handle_scope scope = nullptr;
            if (work == nullptr) {
                LBSLOGE(NMEA_MESSAGE_CALLBACK, "work is nullptr!");
                return;
            }
            context = static_cast<NmeaAsyncContext *>(work->data);
            if (context == nullptr || context->env == nullptr) {
                LBSLOGE(NMEA_MESSAGE_CALLBACK, "context is nullptr!");
                delete work;
                return;
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_open_handle_scope(context->env, &scope));
            if (scope == nullptr) {
                delete context;
                delete work;
                return;
            }
            napi_value jsEvent;
            CHK_NAPI_ERR_CLOSE_SCOPE(context->env,
                napi_create_string_utf8(context->env, context->msg.c_str(), NAPI_AUTO_LENGTH, &jsEvent),
                scope, context, work);
            if (context->callback[0] != nullptr) {
                napi_value undefine;
                napi_value handler = nullptr;
                napi_status ret = napi_ok;
                CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_get_undefined(context->env, &undefine),
                    scope, context, work);
                if (FindNmeaCallback(context->callback[0])) {
                    CHK_NAPI_ERR_CLOSE_SCOPE(context->env,
                        napi_get_reference_value(context->env, context->callback[0], &handler),
                        scope, context, work);
                    ret = napi_call_function(context->env, nullptr, handler, 1, &jsEvent, &undefine);
                }
                if (ret != napi_ok) {
                    LBSLOGE(NMEA_MESSAGE_CALLBACK, "Report event failed");
                }
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_close_handle_scope(context->env, scope));
            delete context;
            delete work;
    });
}

void NmeaMessageCallbackNapi::OnMessageChange(int64_t timestamp, const std::string msg)
{
    LBSLOGI(NMEA_MESSAGE_CALLBACK, "NmeaMessageCallbackNapi::OnMessageChange");
    Send(msg);
}

void NmeaMessageCallbackNapi::DeleteHandler()
{
    std::unique_lock<std::mutex> guard(mutex_);
    if (handlerCb_ == nullptr || env_ == nullptr) {
        LBSLOGE(NMEA_MESSAGE_CALLBACK, "handler or env is nullptr.");
        return;
    }
    DeleteNmeaCallback(handlerCb_);
    NAPI_CALL_RETURN_VOID(env_, napi_delete_reference(env_, handlerCb_));
    handlerCb_ = nullptr;
}
}  // namespace Location
}  // namespace OHOS
