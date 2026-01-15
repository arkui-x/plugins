/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2026. All rights reserved.
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

#include <thread>

#include "napi_pasteboard_common.h"
#include "pasteboard_hilog.h"

using namespace OHOS::MiscServices;
using namespace OHOS::Media;

namespace OHOS {
namespace MiscServicesNapi {
static thread_local napi_ref g_systemPasteboard = nullptr;
static thread_local napi_ref g_systemPasteboard_instance = nullptr;
thread_local std::map<napi_ref, sptr<PasteboardObserverInstance>> SystemPasteboardNapi::localObservers_;
thread_local std::map<napi_ref, sptr<PasteboardObserverInstance>> SystemPasteboardNapi::remoteObservers_;
std::shared_ptr<PasteboardDelayGetterInstance> SystemPasteboardNapi::delayGetter_;
std::mutex SystemPasteboardNapi::delayMutex_;
constexpr int ARGC_TYPE_SET1 = 1;
constexpr size_t MAX_ARGS = 6;
constexpr size_t SYNC_TIMEOUT = 3500;
constexpr size_t DELAY_TIMEOUT = 2;
const std::string STRING_UPDATE = "update";
std::recursive_mutex SystemPasteboardNapi::listenerMutex_;
std::map<std::string, std::shared_ptr<ProgressListenerFn>> SystemPasteboardNapi::listenerMap_;

PasteboardDelayGetterInstance::PasteboardDelayGetterInstance(const napi_env& env, const napi_ref& ref)
    : env_(env), ref_(ref)
{
    stub_ = std::make_shared<PasteboardDelayGetterInstance::PasteboardDelayGetterImpl>();
}

PasteboardDelayGetterInstance::~PasteboardDelayGetterInstance()
{
    ref_ = nullptr;
}

void UvQueueWorkGetDelayPasteData(uv_work_t* work, int status)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "UvQueueWorkGetDelayPasteData start");
    if (UV_ECANCELED == status || work == nullptr || work->data == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "invalid parameter");
        return;
    }
    PasteboardDelayWorker* pasteboardDelayWorker = (PasteboardDelayWorker*)work->data;
    if (pasteboardDelayWorker == nullptr || pasteboardDelayWorker->delayGetter == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "pasteboardDataWorker or delayGetter is null");
        delete work;
        work = nullptr;
        return;
    }
    auto env = pasteboardDelayWorker->delayGetter->GetEnv();
    auto ref = pasteboardDelayWorker->delayGetter->GetRef();
    napi_handle_scope scope = nullptr;
    napi_status napiStatus = napi_open_handle_scope(env, &scope);
    if (napiStatus != napi_ok || scope == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "open handle scope failed, status = %{public}d", napiStatus);
        return;
    }
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    napi_value argv[1] = { CreateNapiString(env, pasteboardDelayWorker->dataType) };
    napi_value callback = nullptr;
    napi_value resultOut = nullptr;
    napi_get_reference_value(env, ref, &callback);
    {
        std::unique_lock<std::mutex> lock(pasteboardDelayWorker->mutex);
        auto ret = napi_call_function(env, undefined, callback, 1, argv, &resultOut);
        if (ret == napi_ok) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "get delay data success");
            UDMF::UnifiedDataNapi* unifiedDataNapi = nullptr;
            napi_unwrap(env, resultOut, reinterpret_cast<void**>(&unifiedDataNapi));
            if (unifiedDataNapi != nullptr) {
                pasteboardDelayWorker->unifiedData = unifiedDataNapi->value_;
            }
        }
        napi_close_handle_scope(env, scope);
        pasteboardDelayWorker->complete = true;
        if (!pasteboardDelayWorker->clean) {
            pasteboardDelayWorker->cv.notify_all();
            return;
        }
    }
    delete pasteboardDelayWorker;
    pasteboardDelayWorker = nullptr;
    delete work;
    work = nullptr;
}

static void ReleasePasteboardResource(PasteboardDelayWorker** pasteboardDelayWorker, uv_work_t** work)
{
    if (pasteboardDelayWorker == nullptr || work == nullptr || *pasteboardDelayWorker == nullptr || *work == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "invalid parameter");
        return;
    }
    delete *pasteboardDelayWorker;
    *pasteboardDelayWorker = nullptr;
    delete *work;
    *work = nullptr;
}

void PasteboardDelayGetterInstance::GetUnifiedData(const std::string& type, UDMF::UnifiedData& data)
{
    uv_loop_s* loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_JS_NAPI, "loop instance is nullptr");
        return;
    }
    uv_work_t* work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_JS_NAPI, "work is null");
        return;
    }
    PasteboardDelayWorker* pasteboardDelayWorker = new (std::nothrow) PasteboardDelayWorker();
    if (pasteboardDelayWorker == nullptr) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_JS_NAPI, "pasteboardDataWorker is null");
        delete work;
        work = nullptr;
        return;
    }
    pasteboardDelayWorker->delayGetter = shared_from_this();
    pasteboardDelayWorker->dataType = type;
    work->data = (void*)pasteboardDelayWorker;
    bool noNeedClean = false;
    {
        std::unique_lock<std::mutex> lock(pasteboardDelayWorker->mutex);
        int ret = uv_queue_work(
            loop, work,
            [](uv_work_t* work) { PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "GetUnifiedData callback"); },
            UvQueueWorkGetDelayPasteData);
        if (ret != 0) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "uv_queue_work ret is not 0");
            ReleasePasteboardResource(&pasteboardDelayWorker, &work);
            return;
        }
        if (pasteboardDelayWorker->cv.wait_for(lock, std::chrono::seconds(DELAY_TIMEOUT),
                [pasteboardDelayWorker] { return pasteboardDelayWorker->complete; }) &&
            pasteboardDelayWorker->unifiedData != nullptr) {
            data = *(pasteboardDelayWorker->unifiedData);
        }
        if (!pasteboardDelayWorker->complete && uv_cancel((uv_req_t*)work) != 0) {
            pasteboardDelayWorker->clean = true;
            noNeedClean = true;
        }
    }
    if (!noNeedClean) {
        ReleasePasteboardResource(&pasteboardDelayWorker, &work);
    }
}

bool SystemPasteboardNapi::CheckArgsOfOnAndOff(napi_env env, bool checkArgsCount, napi_value* argv, size_t argc)
{
    if (!CheckExpression(env, checkArgsCount, JSErrorCode::INVALID_PARAMETERS,
            "Parameter error. The number of arguments is wrong.") ||
        !CheckArgsType(env, argv[0], napi_string, "Parameter error. The type of mimeType must be string.")) {
        return false;
    }
    std::string mimeType;
    bool ret = GetValue(env, argv[0], mimeType);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "GetValue failed");
        return false;
    }
    if (!CheckExpression(env, mimeType == STRING_UPDATE, JSErrorCode::INVALID_PARAMETERS,
            "Parameter error. The value of type must be update")) {
        return false;
    }
    return true;
}

napi_value SystemPasteboardNapi::On(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi on() is called!");
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    size_t argc = MAX_ARGS;
    napi_value argv[MAX_ARGS] = { 0 };
    napi_value thisVar = 0;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    // on(type: 'update', callback: () => void) has 2 args
    if (!CheckArgsOfOnAndOff(env, argc >= 2, argv, argc) ||
        !CheckArgsType(env, argv[1], napi_function, "Parameter error. The type of callback must be function.")) {
        return result;
    }

    napi_value jsCallback = argv[1];
    auto observer = GetObserver(env, jsCallback, localObservers_);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(observer == nullptr, result, PASTEBOARD_MODULE_JS_NAPI, "observer exist");

    AddObserver(env, jsCallback, localObservers_, PasteboardObserverType::OBSERVER_LOCAL);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi on() is end!");
    return result;
}

napi_value SystemPasteboardNapi::Off(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi off () is called!");
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    size_t argc = MAX_ARGS;
    napi_value argv[MAX_ARGS] = { 0 };
    napi_value thisVar = 0;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    // off(type: 'update', callback?: () => void) has at least 1 arg
    if (!CheckArgsOfOnAndOff(env, argc >= 1, argv, argc)) {
        return result;
    }

    sptr<PasteboardObserverInstance> observer = nullptr;
    // 1: is the observer parameter
    if (argc > 1) {
        if (!CheckArgsType(env, argv[1], napi_function, "Parameter error. The type of callback must be function.")) {
            return result;
        }
        observer = GetObserver(env, argv[1], localObservers_);
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
            observer != nullptr, result, PASTEBOARD_MODULE_JS_NAPI, "observer not find");
    }

    DeleteObserver(env, observer, localObservers_, PasteboardObserverType::OBSERVER_LOCAL);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi off () is called!");
    return result;
}

napi_value SystemPasteboardNapi::OnRemoteUpdate(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi OnRemoteUpdate() is called!");
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    size_t argc = MAX_ARGS;
    napi_value argv[MAX_ARGS] = { 0 };
    napi_value thisVar = 0;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    if (argc != 1 ||
        !CheckArgsType(env, argv[0], napi_function, "Parameter error. The type of callback must be function.")) {
        return result;
    }
    napi_value jsCallback = argv[0];
    auto observer = GetObserver(env, jsCallback, remoteObservers_);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(observer == nullptr, result, PASTEBOARD_MODULE_JS_NAPI, "observer exist");

    AddObserver(env, jsCallback, remoteObservers_, PasteboardObserverType::OBSERVER_REMOTE);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi OnRemoteUpdate() is end!");
    return result;
}

napi_value SystemPasteboardNapi::OffRemoteUpdate(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi OffRemoteUpdate () is called!");
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    size_t argc = MAX_ARGS;
    napi_value argv[MAX_ARGS] = { 0 };
    napi_value thisVar = 0;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    sptr<PasteboardObserverInstance> observer = nullptr;
    if (argc > 1 || (argc == 1 && !CheckArgsType(env, argv[0], napi_function,
                                      "Parameter error. The type of callback must be function."))) {
        return result;
    }
    if (argc == 1) {
        observer = GetObserver(env, argv[0], remoteObservers_);
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
            observer != nullptr, result, PASTEBOARD_MODULE_JS_NAPI, "observer not find");
    }

    DeleteObserver(env, observer, remoteObservers_, PasteboardObserverType::OBSERVER_REMOTE);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi OffRemoteUpdate () is called!");
    return result;
}

napi_value SystemPasteboardNapi::Clear(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "Clear is called!");
    auto context = std::make_shared<AsyncCall::Context>();
    auto input = [context](napi_env env, size_t argc, napi_value* argv, napi_value self) -> napi_status {
        // clear has 0 or 1 args
        if (argc > 0 &&
            !CheckArgsType(env, argv[0], napi_function, "Parameter error. The type of callback must be function.")) {
            return napi_invalid_arg;
        }
        return napi_ok;
    };
    auto exec = [context](AsyncCall::Context* ctx) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "exec Clear");
        PasteboardClient::GetInstance()->Clear();
    };
    context->SetAction(std::move(input));
    // 0: the AsyncCall at the first position;
    AsyncCall asyncCall(env, info, context, 0);
    return asyncCall.Call(env, exec);
}

napi_value SystemPasteboardNapi::ClearData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "ClearData is called!");
    return Clear(env, info);
}

napi_value SystemPasteboardNapi::HasPasteData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "HasPasteData is called!");
    auto context = std::make_shared<HasContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value* argv, napi_value self) -> napi_status {
        // hasPasteData has 0 or 1 args
        if (argc > 0 &&
            !CheckArgsType(env, argv[0], napi_function, "Parameter error. The type of callback must be function.")) {
            return napi_invalid_arg;
        }
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value* result) -> napi_status {
        napi_status status = napi_get_boolean(env, context->hasPasteData, result);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "napi_get_boolean status = %{public}d", status);
        return status;
    };
    auto exec = [context](AsyncCall::Context* ctx) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "exec HasPasteData");
        context->hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "HasPasteData result = %{public}d", context->hasPasteData);
        context->status = napi_ok;
    };
    context->SetAction(std::move(input), std::move(output));
    // 0: the AsyncCall at the first position;
    AsyncCall asyncCall(env, info, context, 0);
    return asyncCall.Call(env, exec);
}

napi_value SystemPasteboardNapi::HasData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "HasData is called!");
    return HasPasteData(env, info);
}

void SystemPasteboardNapi::GetDataCommon(std::shared_ptr<GetContextInfo>& context)
{
    auto input = [](napi_env env, size_t argc, napi_value* argv, napi_value self) -> napi_status {
        // 1: GetPasteData has 0 or 1 args
        if (argc > 0 &&
            !CheckArgsType(env, argv[0], napi_function, "Parameter error. The type of callback must be function.")) {
            return napi_invalid_arg;
        }
        return napi_ok;
    };

    auto output = [context](napi_env env, napi_value* result) -> napi_status {
        napi_value instance = nullptr;
        napi_status status = PasteDataNapi::NewInstance(env, instance);
        if (status != napi_ok) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "NewInstance failed");
            return status;
        }
        PasteDataNapi* obj = nullptr;
        napi_status ret = napi_unwrap(env, instance, reinterpret_cast<void**>(&obj));
        if ((ret == napi_ok) && (obj != nullptr)) {
            obj->value_ = context->pasteData;
        } else {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "obj is null or ret not ok");
            return napi_generic_failure;
        }
        *result = instance;
        return napi_ok;
    };
    context->SetAction(std::move(input), std::move(output));
}

napi_value SystemPasteboardNapi::GetPasteData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "GetPasteData is called!");

    auto context = std::make_shared<GetContextInfo>();
    context->pasteData = std::make_shared<PasteData>();
    GetDataCommon(context);

    auto exec = [context](AsyncCall::Context* ctx) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "GetPasteData Begin");
        PasteboardClient::GetInstance()->GetPasteData(*context->pasteData);
        context->status = napi_ok;
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "GetPasteData End");
    };

    // 0: the AsyncCall at the first position;
    AsyncCall asyncCall(env, info, context, 0);
    return asyncCall.Call(env, exec);
}

napi_value SystemPasteboardNapi::GetData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "GetData is called!");

    auto context = std::make_shared<GetContextInfo>();
    context->pasteData = std::make_shared<PasteData>();
    GetDataCommon(context);

    auto exec = [context](AsyncCall::Context* ctx) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "GetData Begin");
        int32_t ret = PasteboardClient::GetInstance()->GetPasteData(*context->pasteData);
        if (ret == static_cast<int32_t>(PasteboardError::TASK_PROCESSING)) {
            context->SetErrInfo(ret, "Another getData is being processed");
        } else {
            context->status = napi_ok;
        }
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "GetData End");
    };
    // 0: the AsyncCall at the first position;
    AsyncCall asyncCall(env, info, context, 0);
    return asyncCall.Call(env, exec);
}

void SystemPasteboardNapi::SetDataCommon(std::shared_ptr<SetContextInfo>& context)
{
    auto input = [context](napi_env env, size_t argc, napi_value* argv, napi_value self) -> napi_status {
        // setData has 1 or 2 args
        if (!CheckExpression(env, argc > 0, JSErrorCode::INVALID_PARAMETERS,
                "Parameter error. The number of arguments must be greater than zero.") ||
            !CheckExpression(env, PasteDataNapi::IsPasteData(env, argv[0]), JSErrorCode::INVALID_PARAMETERS,
                "Parameter error. The Type of data must be pasteData.")) {
            return napi_invalid_arg;
        }
        if (argc > 1 &&
            !CheckArgsType(env, argv[1], napi_function, "Parameter error. The type of callback must be function.")) {
            return napi_invalid_arg;
        }
        PasteDataNapi* pasteData = nullptr;
        napi_unwrap(env, argv[0], reinterpret_cast<void**>(&pasteData));
        if (pasteData != nullptr) {
            context->obj = pasteData->value_;
        }
        return napi_ok;
    };
    context->SetAction(std::move(input));
}

napi_value SystemPasteboardNapi::SetPasteData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SetPasteData is called!");
    auto context = std::make_shared<SetContextInfo>();
    SetDataCommon(context);

    auto exec = [context](AsyncCall::Context* ctx) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "exec SetPasteData");
        if (context->obj != nullptr) {
            PasteboardClient::GetInstance()->SetPasteData(*(context->obj));
            context->obj = nullptr;
        }
        context->status = napi_ok;
    };
    // 1: the AsyncCall at the second position
    AsyncCall asyncCall(env, info, context, 1);
    return asyncCall.Call(env, exec);
}

napi_value SystemPasteboardNapi::SetData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SetData is called!");
    auto context = std::make_shared<SetContextInfo>();
    SetDataCommon(context);

    auto exec = [context](AsyncCall::Context* ctx) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "exec SetPasteData");
        int32_t ret = static_cast<int32_t>(PasteboardError::INVALID_DATA_ERROR);
        if (context->obj != nullptr) {
            std::map<uint32_t, std::shared_ptr<UDMF::EntryGetter>> entryGetters;
            for (auto record : context->obj->AllRecords()) {
                if (record != nullptr && record->GetEntryGetter() != nullptr) {
                    entryGetters.emplace(record->GetRecordId(), record->GetEntryGetter());
                }
            }
            ret = PasteboardClient::GetInstance()->SetPasteData(*(context->obj));
            context->obj = nullptr;
        }
        if (ret == static_cast<int32_t>(PasteboardError::E_OK)) {
            context->status = napi_ok;
        } else if (ret == static_cast<int32_t>(PasteboardError::PROHIBIT_COPY)) {
            context->SetErrInfo(ret, "The system prohibits copying");
        } else if (ret == static_cast<int32_t>(PasteboardError::TASK_PROCESSING)) {
            context->SetErrInfo(ret, "Another setData is being processed");
        }
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "exec context->status[%{public}d]", context->status);
    };
    // 1: the AsyncCall at the second position
    AsyncCall asyncCall(env, info, context, 1);
    return asyncCall.Call(env, exec);
}

napi_value SystemPasteboardNapi::SetUnifiedData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SetUnifiedData is called!");
    // std::shared_ptr<DfxAppEvent> processorEvent = std::make_shared<DfxAppEvent>();
    auto context = std::make_shared<SetUnifiedContextInfo>();
    SetDataCommon(context);

    auto exec = [context](AsyncCall::Context* ctx) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "exec SetPasteData");
        int32_t ret = static_cast<int32_t>(PasteboardError::INVALID_DATA_ERROR);
        if (context->obj != nullptr) {
            if (context->isDelay && context->delayGetter != nullptr && context->delayGetter->GetStub() != nullptr) {
                ret = PasteboardClient::GetInstance()->SetUnifiedData(*(context->obj), context->delayGetter->GetStub());
            } else {
                ret = PasteboardClient::GetInstance()->SetUnifiedData(*(context->obj));
            }
            context->obj = nullptr;
        }
        if (ret == static_cast<int32_t>(PasteboardError::E_OK)) {
            context->status = napi_ok;
        } else if (ret == static_cast<int32_t>(PasteboardError::PROHIBIT_COPY)) {
            context->SetErrInfo(ret, "The system prohibits copying");
        } else if (ret == static_cast<int32_t>(PasteboardError::TASK_PROCESSING)) {
            context->SetErrInfo(ret, "Another setData is being processed");
        }
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "exec context->status[%{public}d]", context->status);
    };
    // 1: the AsyncCall at the second position
    AsyncCall asyncCall(env, info, context, 1);
    return asyncCall.Call(env, exec);
}

napi_value SystemPasteboardNapi::GetUnifiedData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "GetUnifiedData is called!");

    auto context = std::make_shared<GetUnifiedContextInfo>();
    context->unifiedData = std::make_shared<UDMF::UnifiedData>();
    GetDataCommon(context);

    auto exec = [context](AsyncCall::Context* ctx) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "GetUnifiedData Begin");
        int32_t ret = PasteboardClient::GetInstance()->GetUnifiedData(*context->unifiedData);
        if (ret == static_cast<int32_t>(PasteboardError::TASK_PROCESSING)) {
            context->SetErrInfo(ret, "Another getData is being processed");
        } else {
            context->status = napi_ok;
        }
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "GetUnifiedData End");
    };
    // 0: the AsyncCall at the first position;
    AsyncCall asyncCall(env, info, context, 0);
    return asyncCall.Call(env, exec);
}

napi_value SystemPasteboardNapi::GetUnifiedDataSync(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi GetUnifiedDataSync is called!");
    napi_value instance = nullptr;
    std::shared_ptr<UDMF::UnifiedData> unifiedData = std::make_shared<UDMF::UnifiedData>();

    NAPI_CALL(env, UDMF::UnifiedDataNapi::NewInstance(env, unifiedData, instance));
    UDMF::UnifiedDataNapi* obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void**>(&obj));
    if ((status != napi_ok) || (obj == nullptr) || obj->value_ == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "GetUnifiedDataSync get obj failed.");
        return nullptr;
    }
    auto block = std::make_shared<BlockObject<std::shared_ptr<int32_t>>>(SYNC_TIMEOUT);
    std::thread thread([block, unifiedData = obj->value_]() mutable {
        auto ret = PasteboardClient::GetInstance()->GetUnifiedData(*unifiedData);
        std::shared_ptr<int32_t> value = std::make_shared<int32_t>(ret);
        block->SetValue(value);
    });
    thread.detach();
    auto value = block->GetValue();
    if (!CheckExpression(
            env, value != nullptr, JSErrorCode::REQUEST_TIME_OUT, "Excessive processing time for internal data.")) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "time out, GetUnifiedDataSync failed.");
    }
    int32_t ret = (value != nullptr) ? *value : static_cast<int32_t>(PasteboardError::TIMEOUT_ERROR);

    return instance;
}

napi_value SystemPasteboardNapi::SetUnifiedDataSync(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi SetUnifiedDataSync is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = { 0 };
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    if (!CheckExpression(
            env, argc > 0, JSErrorCode::INVALID_PARAMETERS, "Parameter error. Wrong number of arguments.")) {
        return nullptr;
    }
    UDMF::UnifiedDataNapi* unifiedDataNapi = nullptr;
    napi_unwrap(env, argv[0], reinterpret_cast<void**>(&unifiedDataNapi));
    if (!CheckExpression(env, (unifiedDataNapi != nullptr && unifiedDataNapi->value_ != nullptr),
            JSErrorCode::INVALID_PARAMETERS, "Parameter error. The Type of data must be unifiedData.")) {
        return nullptr;
    }
    auto properties = unifiedDataNapi->GetPropertiesNapi(env);
    bool isDelay = false;
    std::shared_ptr<PasteboardDelayGetterInstance> delayGetter = nullptr;
    if (properties != nullptr && properties->delayDataRef_ != nullptr) {
        delayGetter = std::make_shared<PasteboardDelayGetterInstance>(env, properties->delayDataRef_);
        delayGetter->GetStub()->SetDelayGetterWrapper(delayGetter);
        isDelay = true;
    }
    auto block = std::make_shared<BlockObject<std::shared_ptr<int32_t>>>(SYNC_TIMEOUT);
    std::shared_ptr<UDMF::UnifiedData> unifiedData = unifiedDataNapi->value_;
    std::thread thread([block, unifiedData, isDelay, delayGetter]() mutable {
        int32_t ret = isDelay ? PasteboardClient::GetInstance()->SetUnifiedData(*unifiedData, delayGetter->GetStub())
                              : PasteboardClient::GetInstance()->SetUnifiedData(*unifiedData);
        std::shared_ptr<int32_t> value = std::make_shared<int32_t>(ret);
        block->SetValue(value);
    });
    thread.detach();
    auto value = block->GetValue();
    if (!CheckExpression(
            env, value != nullptr, JSErrorCode::REQUEST_TIME_OUT, "Excessive processing time for internal data.")) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "time out, SetUnifiedDataSync failed.");
        return nullptr;
    }
    if (*value != static_cast<int32_t>(PasteboardError::E_OK)) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "operate invalid, SetUnifiedDataSync failed");
        return nullptr;
    }
    {
        std::lock_guard<std::mutex> lck(delayMutex_);
        delayGetter_ = delayGetter;
    }
    return nullptr;
}

napi_value SystemPasteboardNapi::SetAppShareOptions(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = { 0 };
    napi_value thisArg = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));
    if (!CheckExpression(env, argc > 0, JSErrorCode::INVALID_PARAMETERS,
            "Parameter error. Mandatory parameters are left unspecified.")) {
        return nullptr;
    }
    int32_t shareOptions;
    auto status = napi_get_value_int32(env, argv[0], &shareOptions);
    if (!CheckExpression(
            env, status == napi_ok, JSErrorCode::INVALID_PARAMETERS, "Parameter error. Incorrect parameter types.")) {
        return nullptr;
    }
    auto result = PasteboardClient::GetInstance()->SetAppShareOptions(static_cast<ShareOption>(shareOptions));
    if (!CheckExpression(env, result != static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR),
            JSErrorCode::INVALID_PARAMETERS, "Parameter error. Parameter verification failed.")) {
        return nullptr;
    }
    if (!CheckExpression(env, result != static_cast<int32_t>(PasteboardError::PERMISSION_VERIFICATION_ERROR),
            JSErrorCode::NO_PERMISSION, "Permission verification failed. A non-permission application calls a API.")) {
        return nullptr;
    }
    if (!CheckExpression(env, result != static_cast<int32_t>(PasteboardError::INVALID_OPERATION_ERROR),
            JSErrorCode::SETTINGS_ALREADY_EXIST, "Settings already exist.")) {
        return nullptr;
    }
    return nullptr;
}

napi_value SystemPasteboardNapi::RemoveAppShareOptions(napi_env env, napi_callback_info info)
{
    auto result = PasteboardClient::GetInstance()->RemoveAppShareOptions();
    CheckExpression(env, result != static_cast<int32_t>(PasteboardError::PERMISSION_VERIFICATION_ERROR),
        JSErrorCode::NO_PERMISSION, "Permission verification failed. A non-permission application calls a API.");
    return nullptr;
}

void SystemPasteboardNapi::SetDataCommon(std::shared_ptr<SetUnifiedContextInfo>& context)
{
    auto input = [context](napi_env env, size_t argc, napi_value* argv, napi_value self) -> napi_status {
        // setData has 1 arg
        if (!CheckExpression(
                env, argc > 0, JSErrorCode::INVALID_PARAMETERS, "Parameter error. Wrong number of arguments.")) {
            return napi_invalid_arg;
        }
        UDMF::UnifiedDataNapi* unifiedDataNapi = nullptr;
        context->status = napi_unwrap(env, argv[0], reinterpret_cast<void**>(&unifiedDataNapi));
        if (!CheckExpression(env, unifiedDataNapi != nullptr, JSErrorCode::INVALID_PARAMETERS,
                "Parameter error. The Type of data must be unifiedData.")) {
            return napi_invalid_arg;
        }
        context->obj = unifiedDataNapi->value_;
        auto properties = unifiedDataNapi->GetPropertiesNapi(env);
        if (properties != nullptr && properties->delayDataRef_ != nullptr) {
            context->delayGetter = std::make_shared<PasteboardDelayGetterInstance>(env, properties->delayDataRef_);
            if (context->delayGetter == nullptr || context->delayGetter->GetStub() == nullptr) {
                PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "SetDataCommon delayGetter is null");
                return napi_invalid_arg;
            }
            context->delayGetter->GetStub()->SetDelayGetterWrapper(context->delayGetter);
            context->isDelay = true;
        }
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value* result) -> napi_status {
        if (context->status == napi_ok) {
            std::lock_guard<std::mutex> lck(delayMutex_);
            delayGetter_ = std::move(context->delayGetter);
        }
        return napi_ok;
    };
    context->SetAction(std::move(input), std::move(output));
}

void SystemPasteboardNapi::GetDataCommon(std::shared_ptr<GetUnifiedContextInfo>& context)
{
    auto input = [](napi_env env, size_t argc, napi_value* argv, napi_value self) -> napi_status {
        // 1: GetPasteData has 0 or 1 args
        if (argc > 0 &&
            !CheckArgsType(env, argv[0], napi_function, "Parameter error. The type of callback must be function.")) {
            return napi_invalid_arg;
        }
        return napi_ok;
    };

    auto output = [context](napi_env env, napi_value* result) -> napi_status {
        napi_value instance = nullptr;
        std::shared_ptr<UDMF::UnifiedData> unifiedData = std::make_shared<UDMF::UnifiedData>();
        napi_status status = UDMF::UnifiedDataNapi::NewInstance(env, unifiedData, instance);
        if (status != napi_ok) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "NewInstance failed");
            return status;
        }

        UDMF::UnifiedDataNapi* obj = nullptr;
        napi_status ret = napi_unwrap(env, instance, reinterpret_cast<void**>(&obj));
        if ((ret == napi_ok) && (obj != nullptr)) {
            obj->value_ = context->unifiedData;
        } else {
            return napi_generic_failure;
        }
        *result = instance;
        return napi_ok;
    };
    context->SetAction(std::move(input), std::move(output));
}

napi_value SystemPasteboardNapi::IsRemoteData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi IsRemoteData() is called!");
    auto block = std::make_shared<BlockObject<std::shared_ptr<int32_t>>>(SYNC_TIMEOUT);
    std::thread thread([block]() {
        auto ret = PasteboardClient::GetInstance()->IsRemoteData();
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "value=%{public}d", ret);
        std::shared_ptr<int32_t> value = std::make_shared<int32_t>(static_cast<int32_t>(ret));
        block->SetValue(value);
    });
    thread.detach();
    auto value = block->GetValue();
    if (!CheckExpression(
            env, value != nullptr, JSErrorCode::REQUEST_TIME_OUT, "Excessive processing time for internal data.")) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "time out, IsRemoteData failed.");
        return nullptr;
    }
    napi_value result = nullptr;
    napi_get_boolean(env, *value, &result);
    return result;
}

napi_value SystemPasteboardNapi::GetDataSource(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi GetDataSource() is called!");
    auto block = std::make_shared<BlockObject<std::shared_ptr<std::pair<int32_t, std::string>>>>(SYNC_TIMEOUT);
    std::thread thread([block]() mutable {
        std::string bundleName;
        int32_t ret = PasteboardClient::GetInstance()->GetDataSource(bundleName);
        auto value = std::make_shared<std::pair<int32_t, std::string>>(ret, bundleName);
        block->SetValue(value);
    });
    thread.detach();
    auto value = block->GetValue();
    if (!CheckExpression(
            env, value != nullptr, JSErrorCode::REQUEST_TIME_OUT, "Excessive processing time for internal data.")) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "time out, GetDataSource failed.");
        return nullptr;
    }

    if (value->first != static_cast<int32_t>(PasteboardError::E_OK)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "GetDataSource, failed, ret = %{public}d", value->first);
        return nullptr;
    }
    napi_value result = nullptr;
    napi_create_string_utf8(env, value->second.c_str(), NAPI_AUTO_LENGTH, &result);
    return result;
}

napi_value SystemPasteboardNapi::GetMimeTypes(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi GetMimeTypes() is called!");
    auto context = std::make_shared<GetMimeTypesContextInfo>();
    auto input = [](napi_env env, size_t argc, napi_value* argv, napi_value self) -> napi_status {
        if (argc > 0 &&
            !CheckArgsType(env, argv[0], napi_function, "Parameter error. The type of callback must be function.")) {
            return napi_invalid_arg;
        }
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value* result) -> napi_status {
        napi_status status = NapiDataUtils::SetValue(env, context->mimeTypes, *result);
        return status;
    };
    auto exec = [context](AsyncCall::Context* ctx) {
        context->mimeTypes = PasteboardClient::GetInstance()->GetMimeTypes();
        context->status = napi_ok;
    };
    context->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, context, 1);
    return asyncCall.Call(env, exec);
}

napi_value SystemPasteboardNapi::GetChangeCount(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi GetChangeCount() is called!");
    uint32_t changeCount = 0;
    PasteboardClient::GetInstance()->GetChangeCount(changeCount);
    napi_value result = nullptr;
    napi_create_uint32(env, changeCount, &result);
    return result;
}

napi_value SystemPasteboardNapi::HasDataType(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi HasDataType() is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = { 0 };
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    if ((!CheckExpression(env, argc >= ARGC_TYPE_SET1, JSErrorCode::INVALID_PARAMETERS,
            "Parameter error. The number of arguments must be grater than zero.")) ||
        (!CheckArgsType(env, argv[0], napi_string, "Parameter error. The type of mimeType must be string."))) {
        return nullptr;
    }

    std::string mimeType;
    if (!GetValue(env, argv[0], mimeType)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Failed to GetValue!");
        return nullptr;
    }
    auto block = std::make_shared<BlockObject<std::shared_ptr<int32_t>>>(SYNC_TIMEOUT);
    std::thread thread([block, mimeType]() {
        auto ret = PasteboardClient::GetInstance()->HasDataType(mimeType);
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "ret = %{public}d", ret);
        std::shared_ptr<int32_t> value = std::make_shared<int32_t>(static_cast<int32_t>(ret));
        block->SetValue(value);
    });
    thread.detach();
    auto value = block->GetValue();
    if (!CheckExpression(
            env, value != nullptr, JSErrorCode::REQUEST_TIME_OUT, "Excessive processing time for internal data.")) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "time out, HasDataType failed.");
        return nullptr;
    }
    napi_value result = nullptr;
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "value = %{public}d", *value);
    napi_get_boolean(env, *value, &result);
    return result;
}

napi_value SystemPasteboardNapi::DetectPatterns(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<DetectPatternsContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value* argv, napi_value self) -> napi_status {
        if (!CheckExpression(env, argc == ARGC_TYPE_SET1, JSErrorCode::INVALID_PARAMETERS,
                "Parameter error. The number of arguments must be one.")) {
            return napi_invalid_arg;
        }
        bool getValueRes = GetValue(env, argv[0], context->patternsToCheck);
        if (!CheckExpression(
                env, getValueRes, JSErrorCode::INVALID_PARAMETERS, "Parameter error. Array<Pattern> expected.")) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Failed to GetValue.");
            return napi_invalid_arg;
        }
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value* result) -> napi_status {
        napi_status status = SetValue(env, context->patternsDetect, *result);
        return status;
    };
    auto exec = [context](AsyncCall::Context* ctx) {
        context->patternsDetect = PasteboardClient::GetInstance()->DetectPatterns(context->patternsToCheck);
        context->status = napi_ok;
    };
    context->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, context, 1);
    return asyncCall.Call(env, exec);
}

napi_value SystemPasteboardNapi::ClearDataSync(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi ClearDataSync() is called!");
    auto block = std::make_shared<BlockObject<std::shared_ptr<int32_t>>>(SYNC_TIMEOUT);
    std::thread thread([block]() {
        PasteboardClient::GetInstance()->Clear();
        std::shared_ptr<int32_t> value = std::make_shared<int32_t>(0);
        block->SetValue(value);
    });
    thread.detach();
    auto value = block->GetValue();
    if (!CheckExpression(
            env, value != nullptr, JSErrorCode::REQUEST_TIME_OUT, "Excessive processing time for internal data.")) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "time out, ClearDataSync failed.");
    }
    return nullptr;
}

napi_value SystemPasteboardNapi::GetDataSync(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi GetDataSync() is called!");
    napi_value instance = nullptr;
    NAPI_CALL(env, PasteDataNapi::NewInstance(env, instance));
    PasteDataNapi* obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void**>(&obj));
    if ((status != napi_ok) || (obj == nullptr) || obj->value_ == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "GetDataSync napi_unwrap failed");
        return nullptr;
    }
    auto block = std::make_shared<BlockObject<std::shared_ptr<int32_t>>>(SYNC_TIMEOUT);
    std::thread thread([block, pasteData = obj->value_]() mutable {
        auto ret = PasteboardClient::GetInstance()->GetPasteData(*pasteData);
        std::shared_ptr<int32_t> value = std::make_shared<int32_t>(ret);
        block->SetValue(value);
    });
    thread.detach();
    auto value = block->GetValue();
    if (!CheckExpression(
            env, value != nullptr, JSErrorCode::REQUEST_TIME_OUT, "Excessive processing time for internal data.")) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "time out, GetDataSync failed.");
    }
    int32_t ret = (value != nullptr) ? *value : static_cast<int32_t>(PasteboardError::TIMEOUT_ERROR);
    return instance;
}

napi_value SystemPasteboardNapi::SetDataSync(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi SetDataSync() is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = { 0 };
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    if (!CheckExpression(
            env, argc > 0, JSErrorCode::INVALID_PARAMETERS, "Parameter error. The number of arguments must be one.") ||
        !CheckExpression(env, PasteDataNapi::IsPasteData(env, argv[0]), JSErrorCode::INVALID_PARAMETERS,
            "Parameter error. The Type of data must be pasteData.")) {
        return nullptr;
    }

    PasteDataNapi* pasteData = nullptr;
    napi_unwrap(env, argv[0], reinterpret_cast<void**>(&pasteData));
    if (pasteData == nullptr || pasteData->value_ == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Failed to GetValue!");
        return nullptr;
    }
    auto block = std::make_shared<BlockObject<std::shared_ptr<int32_t>>>(SYNC_TIMEOUT);
    std::shared_ptr<PasteData> data = pasteData->value_;
    std::thread thread([block, data]() {
        auto ret = PasteboardClient::GetInstance()->SetPasteData(*data);
        std::shared_ptr<int32_t> value = std::make_shared<int32_t>(ret);
        block->SetValue(value);
    });
    thread.detach();
    auto value = block->GetValue();
    if (!CheckExpression(
            env, value != nullptr, JSErrorCode::REQUEST_TIME_OUT, "Excessive processing time for internal data.")) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "time out, SetDataSync failed.");
        return nullptr;
    }

    if (*value != static_cast<int32_t>(PasteboardError::E_OK)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "operate invalid, SetDataSync failed");
        return nullptr;
    }
    return nullptr;
}

napi_value SystemPasteboardNapi::HasDataSync(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi HasDataSync() is called!");
    auto block = std::make_shared<BlockObject<std::shared_ptr<int32_t>>>(SYNC_TIMEOUT);
    std::thread thread([block]() {
        auto ret = PasteboardClient::GetInstance()->HasPasteData();
        std::shared_ptr<int32_t> value = std::make_shared<int32_t>(static_cast<int32_t>(ret));
        block->SetValue(value);
    });
    thread.detach();
    auto value = block->GetValue();
    if (!CheckExpression(
            env, value != nullptr, JSErrorCode::REQUEST_TIME_OUT, "Excessive processing time for internal data.")) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "time out, HasDataSync failed.");
        return nullptr;
    }
    napi_value result = nullptr;
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "value=%{public}d", *value);
    napi_get_boolean(env, *value, &result);
    return result;
}

void SystemPasteboardNapi::ProgressNotify(std::shared_ptr<GetDataParams> params)
{
    if (params == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "params is null!");
        return;
    }

    if (params->info == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "params->info is null!");
        return;
    }
    std::shared_ptr<ProgressListenerFn> listenerFn = nullptr;
    std::lock_guard<std::recursive_mutex> lock(listenerMutex_);
    auto it = listenerMap_.find("progressNotify");
    if (it != listenerMap_.end()) {
        listenerFn = it->second;
    }

    if (listenerFn == nullptr || listenerFn->tsFunction == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "thread safe function is nullptr!");
        return;
    }

    napi_status status = napi_acquire_threadsafe_function(listenerFn->tsFunction);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "acquire progressNotify failed!");
        return;
    }

    MiscServices::ProgressInfo* progress = new ProgressInfo();
    progress->percentage = params->info->percentage;
    status = napi_call_threadsafe_function(listenerFn->tsFunction, static_cast<void*>(progress), napi_tsfn_blocking);
    if (status != napi_ok) {
        delete progress;
        progress = nullptr;
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "call progressNotify failed!");
        return;
    }

    status = napi_release_threadsafe_function(listenerFn->tsFunction, napi_tsfn_release);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "release progressNotify failed!");
        return;
    }
}

void SystemPasteboardNapi::CallJsProgressNotify(napi_env env, napi_value jsFunction, void* context, void* data)
{
#define AGR_COUNT 1
#define DEVICE_NAME_LEN 512
    (void)context;
    ProgressInfo* info = (ProgressInfo*)data;
    if (info == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "progressInfo is nullptr!");
        return;
    }

    int progress = info->percentage;
    delete info;
    info = nullptr;

    napi_value percentage;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, progress, &percentage));

    napi_value progressInfo = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &progressInfo));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, progressInfo, "progress", percentage));

    napi_value argv[AGR_COUNT] = { progressInfo };
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, NULL, jsFunction, AGR_COUNT, argv, NULL));
}

bool SystemPasteboardNapi::CreateThreadSafeFunc(napi_env env, const std::shared_ptr<ProgressListenerFn> listenerFn)
{
#define MAX_LISTENER_LEN 20
    napi_value name = nullptr;
    if (listenerFn->jsCallback == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "jsCallback is null!");
        return false;
    }

    NAPI_CALL_BASE(env, napi_create_string_utf8(env, "progressNotify", MAX_LISTENER_LEN, &name), false);
    napi_status status = napi_create_threadsafe_function(
        env, listenerFn->jsCallback, NULL, name, 0, 1, NULL, NULL, NULL, CallJsProgressNotify, &listenerFn->tsFunction);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "create threadsafe failed!");
        return false;
    }
    return true;
}

bool SystemPasteboardNapi::AddProgressListener(napi_env env, std::shared_ptr<MiscServices::GetDataParams> getDataParam,
    const std::shared_ptr<ProgressListenerFn> listenerFn)
{
    std::lock_guard<std::recursive_mutex> lock(listenerMutex_);
    auto it = listenerMap_.find("progressNotify");
    if (it == listenerMap_.end()) {
        listenerMap_.insert({ "progressNotify", listenerFn });
    }

    if (!CreateThreadSafeFunc(env, listenerFn)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "CreateThreadSafeFunc failed!");
        listenerMap_.erase("progressNotify");
        return false;
    }

    getDataParam->listener.ProgressNotify = ProgressNotify;
    return true;
}

bool SystemPasteboardNapi::CheckParamsType(napi_env env, napi_value in, napi_valuetype expectedType)
{
    napi_valuetype type = napi_undefined;
    NAPI_CALL_BASE(env, napi_typeof(env, in, &type), false);
    if (type != expectedType) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "invalid parameters!");
        return false;
    }
    return true;
}

bool SystemPasteboardNapi::ParseJsGetDataWithProgress(
    napi_env env, napi_value in, std::shared_ptr<MiscServices::GetDataParams>& getDataParam)
{
#define MAX_DESTURI_LEN 250
    napi_value destUri = nullptr;
    NAPI_CALL_BASE(env, napi_get_named_property(env, in, "destUri", &destUri), false);
    if (CheckParamsType(env, destUri, napi_string)) {
        size_t destUriLen = 0;
        NAPI_CALL_BASE(env, napi_get_value_string_utf8(env, destUri, nullptr, 0, &destUriLen), false);
        if (destUriLen <= 0 || destUriLen > MAX_DESTURI_LEN) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "destUriLen check failed!");
            return false;
        }
        char* uri = (char*)malloc(destUriLen + 1);
        if (uri == nullptr) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "malloc failed, uri is nullptr.");
            return false;
        }
        NAPI_CALL_BASE(env, napi_get_value_string_utf8(env, destUri, uri, destUriLen + 1, &destUriLen), false);
        getDataParam->destUri = uri;
        free(uri);
    }
    napi_value fileConflictOption;
    NAPI_CALL_BASE(env, napi_get_named_property(env, in, "fileConflictOptions", &fileConflictOption), false);
    getDataParam->fileConflictOption = FILE_OVERWRITE;
    if (CheckParamsType(env, fileConflictOption, napi_number)) {
        NAPI_CALL_BASE(env,
            napi_get_value_int32(env, fileConflictOption, reinterpret_cast<int*>(&getDataParam->fileConflictOption)),
            false);
    }
    napi_value progressIndicator;
    NAPI_CALL_BASE(env, napi_get_named_property(env, in, "progressIndicator", &progressIndicator), false);
    NAPI_CALL_BASE(env,
        napi_get_value_int32(env, progressIndicator, reinterpret_cast<int*>(&getDataParam->progressIndicator)), false);

    std::shared_ptr<ProgressListenerFn> listenerFn = std::make_shared<ProgressListenerFn>();
    NAPI_CALL_BASE(env, napi_get_named_property(env, in, "progressListener", &listenerFn->jsCallback), false);
    if (CheckParamsType(env, listenerFn->jsCallback, napi_function)) {
        if (!AddProgressListener(env, getDataParam, listenerFn)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "add listener failed!");
            return false;
        }
    }
    return true;
}

void SystemPasteboardNapi::GetDataWithProgressParam(std::shared_ptr<GetDataParamsContextInfo>& context)
{
    auto input = [context](napi_env env, size_t argc, napi_value* argv, napi_value self) -> napi_status {
        // 1: GetPasteData has 0 or 1 args
        if (argc > 0 &&
            !CheckArgsType(env, argv[0], napi_object, "Parameter error. The type of object must be function.")) {
            return napi_invalid_arg;
        }
        if (!CheckExpression(env, ParseJsGetDataWithProgress(env, argv[0], context->getDataParams),
                JSErrorCode::INVALID_PARAMETERS, "Parameter error. parse param failed!")) {
            return napi_invalid_arg;
        }
        return napi_ok;
    };

    auto output = [context](napi_env env, napi_value* result) -> napi_status {
        napi_value instance = nullptr;
        napi_status status = PasteDataNapi::NewInstance(env, instance);
        if (status != napi_ok) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "NewInstance failed!");
            return napi_invalid_arg;
        }
        PasteDataNapi* obj = nullptr;
        napi_status ret = napi_unwrap(env, instance, reinterpret_cast<void**>(&obj));
        if ((ret == napi_ok) && (obj != nullptr)) {
            obj->value_ = context->pasteData;
        } else {
            return napi_generic_failure;
        }
        *result = instance;
        return napi_ok;
    };
    context->SetAction(std::move(input), std::move(output));
}

napi_value SystemPasteboardNapi::GetDataWithProgress(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "GetDataWithProgress is called!");

    auto context = std::make_shared<GetDataParamsContextInfo>();
    context->pasteData = std::make_shared<PasteData>();
    context->getDataParams = std::make_shared<GetDataParams>();
    GetDataWithProgressParam(context);

    auto exec = [context](AsyncCall::Context* ctx) {
        context->getDataParams->info = new (std::nothrow) ProgressInfo();
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "GetDataWithProgress Begin");
        int32_t ret = PasteboardClient::GetInstance()->GetDataWithProgress(*context->pasteData, context->getDataParams);
        auto it = ErrorCodeMap.find(PasteboardError(ret));
        if (it != ErrorCodeMap.end()) {
            context->SetErrInfo(static_cast<int32_t>(it->second.first), it->second.second);
        } else {
            if (ret != static_cast<int32_t>(PasteboardError::E_OK)) {
                context->SetErrInfo(static_cast<int32_t>(JSErrorCode::ERR_GET_DATA_FAILED),
                    "System error occurred during paste execution.");
            } else {
                context->status = napi_ok;
            }
        }
        listenerMap_.erase("progressNotify");
        if (context->getDataParams->info != nullptr) {
            delete context->getDataParams->info;
            context->getDataParams->info = nullptr;
        }
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "GetDataWithProgress End");
    };
    // 0: the AsyncCall at the first position;
    AsyncCall asyncCall(env, info, context, 0);
    return asyncCall.Call(env, exec);
}

napi_value SystemPasteboardNapi::SystemPasteboardInit(napi_env env, napi_value exports)
{
    napi_status status = napi_ok;
    napi_property_descriptor descriptors[] = {
        DECLARE_NAPI_WRITABLE_FUNCTION("on", On),
        DECLARE_NAPI_WRITABLE_FUNCTION("off", Off),
        DECLARE_NAPI_WRITABLE_FUNCTION("onRemoteUpdate", OnRemoteUpdate),
        DECLARE_NAPI_WRITABLE_FUNCTION("offRemoteUpdate", OffRemoteUpdate),
        DECLARE_NAPI_WRITABLE_FUNCTION("clear", Clear),
        DECLARE_NAPI_WRITABLE_FUNCTION("getPasteData", GetPasteData),
        DECLARE_NAPI_WRITABLE_FUNCTION("hasPasteData", HasPasteData),
        DECLARE_NAPI_WRITABLE_FUNCTION("setPasteData", SetPasteData),
        DECLARE_NAPI_WRITABLE_FUNCTION("clearData", ClearData),
        DECLARE_NAPI_WRITABLE_FUNCTION("getData", GetData),
        DECLARE_NAPI_WRITABLE_FUNCTION("hasData", HasData),
        DECLARE_NAPI_WRITABLE_FUNCTION("setData", SetData),
        DECLARE_NAPI_WRITABLE_FUNCTION("isRemoteData", IsRemoteData),
        DECLARE_NAPI_WRITABLE_FUNCTION("getDataSource", GetDataSource),
        DECLARE_NAPI_WRITABLE_FUNCTION("getMimeTypes", GetMimeTypes),
        DECLARE_NAPI_WRITABLE_FUNCTION("hasDataType", HasDataType),
        DECLARE_NAPI_WRITABLE_FUNCTION("detectPatterns", DetectPatterns),
        DECLARE_NAPI_WRITABLE_FUNCTION("clearDataSync", ClearDataSync),
        DECLARE_NAPI_WRITABLE_FUNCTION("getDataSync", GetDataSync),
        DECLARE_NAPI_WRITABLE_FUNCTION("hasDataSync", HasDataSync),
        DECLARE_NAPI_WRITABLE_FUNCTION("setDataSync", SetDataSync),
        DECLARE_NAPI_WRITABLE_FUNCTION("setUnifiedData", SetUnifiedData),
        DECLARE_NAPI_WRITABLE_FUNCTION("getUnifiedData", GetUnifiedData),
        DECLARE_NAPI_WRITABLE_FUNCTION("setUnifiedDataSync", SetUnifiedDataSync),
        DECLARE_NAPI_WRITABLE_FUNCTION("getUnifiedDataSync", GetUnifiedDataSync),
        DECLARE_NAPI_WRITABLE_FUNCTION("setAppShareOptions", SetAppShareOptions),
        DECLARE_NAPI_WRITABLE_FUNCTION("removeAppShareOptions", RemoveAppShareOptions),
        DECLARE_NAPI_WRITABLE_FUNCTION("getDataWithProgress", GetDataWithProgress),
        DECLARE_NAPI_WRITABLE_FUNCTION("getChangeCount", GetChangeCount),
    };
    napi_value constructor;
    napi_define_class(env, "SystemPasteboard", NAPI_AUTO_LENGTH, New, nullptr,
        sizeof(descriptors) / sizeof(napi_property_descriptor), descriptors, &constructor);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Failed to define class at SystemPasteboardInit");
        return nullptr;
    }
    napi_create_reference(env, constructor, 1, &g_systemPasteboard);
    status = napi_set_named_property(env, exports, "SystemPasteboard", constructor);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Set property failed when SystemPasteboardInit");
        return nullptr;
    }
    return exports;
}

SystemPasteboardNapi::SystemPasteboardNapi() : env_(nullptr)
{
    value_ = std::make_shared<PasteDataNapi>();
}

SystemPasteboardNapi::~SystemPasteboardNapi()
{
    value_ = nullptr;
}

void SystemPasteboardNapi::Destructor(napi_env env, void* nativeObject, void* finalize_hint)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "Destructor");
    (void)nativeObject;
    SystemPasteboardNapi* obj = static_cast<SystemPasteboardNapi*>(nativeObject);
    delete obj;
}

napi_value SystemPasteboardNapi::New(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_ARGS;
    napi_value argv[MAX_ARGS] = { 0 };
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "proc.");
    // get native object
    SystemPasteboardNapi* obj = new (std::nothrow) SystemPasteboardNapi();
    if (!obj) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "New obj is null");
        return nullptr;
    }
    obj->env_ = env;
    ASSERT_CALL(env,
        napi_wrap(env, thisVar, obj, SystemPasteboardNapi::Destructor,
            nullptr, // finalize_hint
            nullptr),
        obj);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "end.");
    return thisVar;
}

napi_status SystemPasteboardNapi::NewInstance(napi_env env, napi_value& instance)
{
    napi_status status;
    if (g_systemPasteboard_instance != nullptr) {
        status = napi_get_reference_value(env, g_systemPasteboard_instance, &instance);
        if (status != napi_ok) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "get instance failed");
            return status;
        }
        return napi_ok;
    }

    napi_value constructor;
    status = napi_get_reference_value(env, g_systemPasteboard, &constructor);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "get reference failed");
        return status;
    }

    status = napi_new_instance(env, constructor, 0, nullptr, &instance);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "new instance failed");
        return status;
    }
    napi_create_reference(env, instance, 1, &g_systemPasteboard_instance);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "new instance ok");

    return napi_ok;
}

sptr<PasteboardObserverInstance> SystemPasteboardNapi::GetObserver(
    napi_env env, napi_value jsCallback, std::map<napi_ref, sptr<PasteboardObserverInstance>>& observerMap)
{
    for (const auto& [refKey, observerValue] : observerMap) {
        napi_value callback = nullptr;
        napi_get_reference_value(env, refKey, &callback);
        bool isEqual = false;
        napi_strict_equals(env, jsCallback, callback, &isEqual);
        if (isEqual) {
            return observerValue;
        }
    }
    return nullptr;
}

void SystemPasteboardNapi::AddObserver(napi_env env, napi_value jsCallback,
    std::map<napi_ref, sptr<PasteboardObserverInstance>>& observerMap, MiscServices::PasteboardObserverType type)
{
    PasteboardNapiScope scope(env);

    napi_value name = nullptr;
    napi_status status = napi_create_string_utf8(env, "pasteboardChanged", NAPI_AUTO_LENGTH, &name);
    PASTEBOARD_CHECK_AND_RETURN_LOGE(
        status == napi_ok, PASTEBOARD_MODULE_JS_NAPI, "create string failed, status=%{public}d", status);

    napi_ref ref = nullptr;
    status = napi_create_reference(env, jsCallback, 1, &ref);
    PASTEBOARD_CHECK_AND_RETURN_LOGE(
        status == napi_ok, PASTEBOARD_MODULE_JS_NAPI, "create reference failed, status=%{public}d", status);

    napi_threadsafe_function napiCallback = nullptr;
    status = napi_create_threadsafe_function(
        env, jsCallback, nullptr, name, 0, 1, nullptr, nullptr, nullptr, nullptr, &napiCallback);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "create callback failed, status=%{public}d", status);
        napi_delete_reference(env, ref);
        return;
    }

    auto observer = sptr<PasteboardObserverInstance>::MakeSptr(napiCallback, env);
    if (observer == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "malloc observer failed");
        napi_release_threadsafe_function(napiCallback, napi_tsfn_release);
        napi_delete_reference(env, ref);
        return;
    }

    bool ret = PasteboardClient::GetInstance()->Subscribe(type, observer);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "subscribe observer failed");
    }

    observerMap[ref] = observer;
}

void SystemPasteboardNapi::DeleteObserver(napi_env env, const sptr<PasteboardObserverInstance>& observer,
    std::map<napi_ref, sptr<PasteboardObserverInstance>>& observerMap, MiscServices::PasteboardObserverType type)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "observer == null: %{public}d, size: %{public}zu", observer == nullptr,
        observerMap.size());
    std::vector<sptr<PasteboardObserverInstance>> observers;
    std::vector<napi_ref> refs;
    for (auto it = observerMap.begin(); it != observerMap.end();) {
        if (it->second == observer) {
            refs.push_back(it->first);
            observers.push_back(observer);
            it = observerMap.erase(it);
            break;
        }
        if (observer == nullptr) {
            refs.push_back(it->first);
            observers.push_back(it->second);
            it = observerMap.erase(it);
        } else {
            it++;
        }
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "delete observer size: %{public}zu", observers.size());
    for (auto& delObserver : observers) {
        PasteboardClient::GetInstance()->Unsubscribe(type, delObserver);
    }
    for (auto& ref : refs) {
        napi_delete_reference(env, ref);
    }
}

void PasteboardDelayGetterInstance::PasteboardDelayGetterImpl::GetUnifiedData(
    const std::string& type, UDMF::UnifiedData& data)
{
    std::shared_ptr<PasteboardDelayGetterInstance> delayGetterInstance(wrapper_.lock());
    if (delayGetterInstance == nullptr) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_JS_NAPI, "no delay getter");
        return;
    }
    delayGetterInstance->GetUnifiedData(type, data);
}

void PasteboardDelayGetterInstance::PasteboardDelayGetterImpl::GetPasteData(
    const std::string& type, MiscServices::PasteData& data)
{}

void PasteboardDelayGetterInstance::PasteboardDelayGetterImpl::SetDelayGetterWrapper(
    const std::shared_ptr<PasteboardDelayGetterInstance> delayGetterInstance)
{
    wrapper_ = delayGetterInstance;
}
} // namespace MiscServicesNapi
} // namespace OHOS