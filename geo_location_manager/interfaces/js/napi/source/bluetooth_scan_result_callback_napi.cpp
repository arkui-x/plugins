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

#include "bluetooth_scan_result_callback_napi.h"

#include "js_native_api.h"
#include "napi/native_common.h"
#include "uv.h"

#include "constant_definition.h"
#include "napi_util.h"

namespace OHOS {
namespace Location {
static std::mutex g_regCallbackMutex;
static std::vector<napi_ref> g_registerCallbacks;
BluetoothScanResultCallbackNapi::BluetoothScanResultCallbackNapi()
{
    env_ = nullptr;
    handlerCb_ = nullptr;
}

BluetoothScanResultCallbackNapi::~BluetoothScanResultCallbackNapi()
{
}

napi_env BluetoothScanResultCallbackNapi::GetEnv()
{
    std::unique_lock<std::mutex> guard(mutex_);
    return env_;
}

void BluetoothScanResultCallbackNapi::SetEnv(const napi_env& env)
{
    std::unique_lock<std::mutex> guard(mutex_);
    env_ = env;
}

napi_ref BluetoothScanResultCallbackNapi::GetHandleCb()
{
    std::unique_lock<std::mutex> guard(mutex_);
    return handlerCb_;
}

void BluetoothScanResultCallbackNapi::SetHandleCb(const napi_ref& handlerCb)
{
    {
        std::unique_lock<std::mutex> guard(mutex_);
        handlerCb_ = handlerCb;
    }
    std::unique_lock<std::mutex> guard(g_regCallbackMutex);
    g_registerCallbacks.emplace_back(handlerCb);
}

bool FindBlueToothCallback(napi_ref cb)
{
    std::unique_lock<std::mutex> guard(g_regCallbackMutex);
    auto iter = std::find(g_registerCallbacks.begin(), g_registerCallbacks.end(), cb);
    if (iter == g_registerCallbacks.end()) {
        return false;
    }
    return true;
}

void DeleteBlueToothCallback(napi_ref cb)
{
    std::unique_lock<std::mutex> guard(g_regCallbackMutex);
    for (auto iter = g_registerCallbacks.begin(); iter != g_registerCallbacks.end(); iter++) {
        if (*iter == cb) {
            iter = g_registerCallbacks.erase(iter);
            break;
        }
    }
}

void BluetoothScanResultCallbackNapi::DoSendWork(uv_loop_s*& loop, uv_work_t*& work)
{
    uv_queue_work(loop, work, [](uv_work_t* work) {}, [](uv_work_t* work, int status) {
        if (work == nullptr) {
            return;
        }
        napi_handle_scope scope = nullptr;
        auto context = static_cast<BluetoothScanResultAsyncContext*>(work->data);
        if (context == nullptr) {
            LBSLOGE(BLUETOOTH_CALLBACK, "context == nullptr");
            delete work;
            return;
        }
        if (context->env == nullptr || context->bluetoothScanResult == nullptr) {
            LBSLOGE(BLUETOOTH_CALLBACK, "bluetoothScanResult == nullptr");
            delete context;
            delete work;
            return;
        }
        if (!FindBlueToothCallback(context->callback[0])) {
            LBSLOGE(BLUETOOTH_CALLBACK, "no valid callback");
            delete context;
            delete work;
            return;
        }
        napi_open_handle_scope(context->env, &scope);
        if (scope == nullptr) {
            LBSLOGE(BLUETOOTH_CALLBACK, "scope == nullptr");
            DELETE_SCOPE_CONTEXT_WORK(context->env, scope, context, work);
            return;
        }
        napi_value jsEvent = nullptr;
        CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_create_object(context->env, &jsEvent), scope, context, work);
        BluetoothScanResultToJs(context->env, context->bluetoothScanResult, jsEvent);
        if (context->callback[0] != nullptr) {
            napi_value undefine = nullptr;
            napi_value handler = nullptr;
            CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_get_undefined(context->env, &undefine),
                scope, context, work);
            CHK_NAPI_ERR_CLOSE_SCOPE(context->env,
                napi_get_reference_value(context->env, context->callback[0], &handler), scope, context, work);
            if (napi_call_function(context->env, nullptr, handler, 1, &jsEvent, &undefine) != napi_ok) {
                LBSLOGE(BLUETOOTH_CALLBACK, "Report location failed");
            }
        }
        NAPI_CALL_RETURN_VOID(context->env, napi_close_handle_scope(context->env, scope));
        delete context;
        delete work;
    });
}

void BluetoothScanResultCallbackNapi::OnBluetoothScanResultChange(
    const std::unique_ptr<BluetoothScanResult>& bluetoothScanResult)
{
    std::unique_lock<std::mutex> guard(mutex_);
    uv_loop_s *loop = nullptr;
    if (env_ == nullptr) {
        LBSLOGI(BLUETOOTH_CALLBACK, "env_ is nullptr.");
        return;
    }
    if (handlerCb_ == nullptr) {
        LBSLOGE(BLUETOOTH_CALLBACK, "handler is nullptr.");
        return;
    }
    NAPI_CALL_RETURN_VOID(env_, napi_get_uv_event_loop(env_, &loop));
    if (loop == nullptr) {
        LBSLOGE(BLUETOOTH_CALLBACK, "loop == nullptr.");
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LBSLOGE(BLUETOOTH_CALLBACK, "work == nullptr.");
        return;
    }
    auto context = new (std::nothrow) BluetoothScanResultAsyncContext(env_);
    if (context == nullptr) {
        LBSLOGE(BLUETOOTH_CALLBACK, "context == nullptr.");
        delete work;
        return;
    }
    if (!InitContext(context)) {
        LBSLOGE(BLUETOOTH_CALLBACK, "InitContext fail");
        delete work;
        delete context;
        return;
    }
    context->bluetoothScanResult = std::make_unique<BluetoothScanResult>(*bluetoothScanResult);
    work->data = context;
    DoSendWork(loop, work);
}

void BluetoothScanResultCallbackNapi::DeleteHandler()
{
    LBSLOGI(BLUETOOTH_CALLBACK, "before DeleteHandler");
    std::unique_lock<std::mutex> guard(mutex_);
    if (env_ == nullptr) {
        LBSLOGE(BLUETOOTH_CALLBACK, "env is nullptr.");
        return;
    }
    DeleteBlueToothCallback(handlerCb_);
    if (handlerCb_ != nullptr) {
        NAPI_CALL_RETURN_VOID(env_, napi_delete_reference(env_, handlerCb_));
        handlerCb_ = nullptr;
    }
}
} // namespace Location
} // namespace OHOS
