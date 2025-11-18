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
#include "location_gnss_geofence_callback_napi.h"

#include "napi/native_common.h"

#include "common_utils.h"
#include "location_log.h"
#include "napi_util.h"
#include "geofence_napi.h"
#include "location_async_context.h"
#include "geofence_async_context.h"
#include "beacon_fence.h"

namespace OHOS {
namespace Location {
static std::mutex g_regCallbackMutex;
static std::vector<napi_ref> g_registerCallbacks;
LocationGnssGeofenceCallbackNapi::LocationGnssGeofenceCallbackNapi()
{
    env_ = nullptr;
    handlerCb_ = nullptr;
    remoteDied_ = false;
    fenceId_ = -1;
    type_ = GNSS_GEOFENCE_OPT_TYPE_ADD;
    result_ = GNSS_GEOFENCE_OPERATION_SUCCESS;
    InitLatch();
}

LocationGnssGeofenceCallbackNapi::~LocationGnssGeofenceCallbackNapi()
{
    if (latch_ != nullptr) {
        delete latch_;
        latch_ = nullptr;
    }
    LBSLOGW(LOCATION_GNSS_GEOFENCE_CALLBACK, "~LocationGnssGeofenceCallbackNapi()");
}

void LocationGnssGeofenceCallbackNapi::InitLatch()
{
    latch_ = new CountDownLatch();
    latch_->SetCount(1);
}

napi_env LocationGnssGeofenceCallbackNapi::GetEnv()
{
    std::unique_lock<std::mutex> guard(mutex_);
    return env_;
}

void LocationGnssGeofenceCallbackNapi::SetEnv(const napi_env& env)
{
    std::unique_lock<std::mutex> guard(mutex_);
    env_ = env;
}

napi_ref LocationGnssGeofenceCallbackNapi::GetHandleCb()
{
    std::unique_lock<std::mutex> guard(mutex_);
    return handlerCb_;
}

void LocationGnssGeofenceCallbackNapi::SetHandleCb(const napi_ref& handlerCb)
{
    {
        std::unique_lock<std::mutex> guard(mutex_);
        handlerCb_ = handlerCb;
    }
    std::unique_lock<std::mutex> guard(g_regCallbackMutex);
    g_registerCallbacks.emplace_back(handlerCb);
}

bool FindGeofenceRegCallback(napi_ref cb)
{
    std::unique_lock<std::mutex> guard(g_regCallbackMutex);
    auto iter = std::find(g_registerCallbacks.begin(), g_registerCallbacks.end(), cb);
    if (iter == g_registerCallbacks.end()) {
        return false;
    }
    return true;
}

void DeleteGeofenceRegCallback(napi_ref cb)
{
    std::unique_lock<std::mutex> guard(g_regCallbackMutex);
    for (auto iter = g_registerCallbacks.begin(); iter != g_registerCallbacks.end(); iter++) {
        if (*iter == cb) {
            iter = g_registerCallbacks.erase(iter);
            break;
        }
    }
    LBSLOGW(LOCATION_GNSS_GEOFENCE_CALLBACK, "after DeleteGeofenceRegCallback, callback size %{public}s",
        std::to_string(g_registerCallbacks.size()).c_str());
}

void LocationGnssGeofenceCallbackNapi::OnTransitionStatusChange(
    GeofenceTransition transition)
{
    std::unique_lock<std::mutex> guard(mutex_);
    uv_loop_s *loop = nullptr;
    NAPI_CALL_RETURN_VOID(env_, napi_get_uv_event_loop(env_, &loop));
    if (loop == nullptr) {
        LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "loop == nullptr.");
        return;
    }
    if (handlerCb_ == nullptr) {
        LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "handler is nullptr.");
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "work == nullptr.");
        return;
    }
    GnssGeofenceAsyncContext *context = new (std::nothrow) GnssGeofenceAsyncContext(env_);
    if (context == nullptr) {
        LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "context == nullptr.");
        delete work;
        return;
    }
    if (!InitContext(context)) {
        LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "InitContext fail");
        delete work;
        delete context;
        return;
    }
    context->transition_ = transition;
    work->data = context;
    UvQueueWork(loop, work);
}

void LocationGnssGeofenceCallbackNapi::OnReportOperationResult(int fenceId, int type, int result)
{
    int addValue = static_cast<int>(GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_ADD);
    if ((type != addValue && fenceId == GetFenceId()) ||
        (type == addValue)) {
        GnssGeofenceOperateResult optResult = static_cast<GnssGeofenceOperateResult>(result);
        GnssGeofenceOperateType optType = static_cast<GnssGeofenceOperateType>(type);
        if (result == GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_SUCCESS &&
            optType == GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_ADD) {
            SetFenceId(fenceId);
        }
        SetGeofenceOperationType(optType);
        SetGeofenceOperationResult(optResult);
    }
}

bool LocationGnssGeofenceCallbackNapi::IsRemoteDied()
{
    return remoteDied_;
}

void LocationGnssGeofenceCallbackNapi::UvQueueWork(uv_loop_s* loop, uv_work_t* work)
{
    uv_queue_work(
        loop, work, [](uv_work_t *work) {}, [](uv_work_t *work, int status) {
            GnssGeofenceAsyncContext *context = nullptr;
            napi_handle_scope scope = nullptr;
            if (work == nullptr) {
                LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "work is nullptr");
                return;
            }
            context = static_cast<GnssGeofenceAsyncContext *>(work->data);
            if (context == nullptr || context->env == nullptr) {
                LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "context is nullptr");
                delete work;
                return;
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_open_handle_scope(context->env, &scope));
            if (scope == nullptr) {
                LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "scope is nullptr");
                delete context;
                delete work;
                return;
            }
            if (context->transition_.beaconFence == nullptr) {
                napi_value jsEvent[PARAM2];
                CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_create_object(context->env, &jsEvent[PARAM1]),
                    scope, context, work);
                CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_get_undefined(context->env, &jsEvent[PARAM0]),
                    scope, context, work);
                GeofenceTransitionToJs(context->env, context->transition_, jsEvent[PARAM1]);
                if (context->callback[SUCCESS_CALLBACK] != nullptr) {
                    napi_value undefine;
                    napi_value handler = nullptr;
                    napi_status ret = napi_ok;
                    CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_get_undefined(context->env, &undefine),
                        scope, context, work);
                    if (FindGeofenceRegCallback(context->callback[0])) {
                        CHK_NAPI_ERR_CLOSE_SCOPE(context->env,
                            napi_get_reference_value(context->env, context->callback[SUCCESS_CALLBACK], &handler),
                            scope, context, work);
                        ret = napi_call_function(context->env, nullptr, handler, RESULT_SIZE, jsEvent, &undefine);
                    } else {
                        LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "no valid callback");
                    }
                    if (ret != napi_ok) {
                        LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "Report event failed");
                    }
                }
            } else {
                napi_value jsEvent = nullptr;
                CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_create_object(context->env, &jsEvent),
                    scope, context, work);
                GeofenceTransitionToJs(context->env, context->transition_, jsEvent);
                if (context->callback[SUCCESS_CALLBACK] != nullptr) {
                    napi_value undefine;
                    napi_value handler = nullptr;
                    napi_status ret = napi_ok;
                    CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_get_undefined(context->env, &undefine),
                        scope, context, work);
                    if (FindGeofenceRegCallback(context->callback[0])) {
                        CHK_NAPI_ERR_CLOSE_SCOPE(context->env,
                            napi_get_reference_value(context->env, context->callback[SUCCESS_CALLBACK], &handler),
                            scope, context, work);
                        ret = napi_call_function(context->env, nullptr, handler, 1, &jsEvent, &undefine);
                    } else {
                        LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "no valid callback");
                    }
                    if (ret != napi_ok) {
                        LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "Report event failed");
                    }
                }
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_close_handle_scope(context->env, scope));
            delete context;
            delete work;
    });
}

void LocationGnssGeofenceCallbackNapi::DeleteHandler()
{
    std::unique_lock<std::mutex> guard(mutex_);
    if (handlerCb_ == nullptr || env_ == nullptr) {
        LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "handler or env is nullptr.");
        return;
    }
    DeleteGeofenceRegCallback(handlerCb_);
    NAPI_CALL_RETURN_VOID(env_, napi_delete_reference(env_, handlerCb_));
    handlerCb_ = nullptr;
}

void LocationGnssGeofenceCallbackNapi::CountDown()
{
    if (latch_ != nullptr) {
        latch_->CountDown();
    }
}

void LocationGnssGeofenceCallbackNapi::Wait(int time)
{
    if (latch_ != nullptr) {
        latch_->Wait(time);
    }
}

int LocationGnssGeofenceCallbackNapi::GetCount()
{
    if (latch_ != nullptr) {
        return latch_->GetCount();
    }
    return 0;
}

void LocationGnssGeofenceCallbackNapi::SetCount(int count)
{
    if (latch_ != nullptr) {
        return latch_->SetCount(count);
    }
}

void LocationGnssGeofenceCallbackNapi::ClearFenceId()
{
    std::unique_lock<std::mutex> guard(operationResultMutex_);
    fenceId_ = -1;
}

int LocationGnssGeofenceCallbackNapi::GetFenceId()
{
    std::unique_lock<std::mutex> guard(operationResultMutex_);
    return fenceId_;
}

void LocationGnssGeofenceCallbackNapi::SetFenceId(int fenceId)
{
    std::unique_lock<std::mutex> guard(operationResultMutex_);
    fenceId_ = fenceId;
}

GnssGeofenceOperateType LocationGnssGeofenceCallbackNapi::GetGeofenceOperationType()
{
    std::unique_lock<std::mutex> guard(operationResultMutex_);
    return type_;
}

void LocationGnssGeofenceCallbackNapi::SetGeofenceOperationType(GnssGeofenceOperateType type)
{
    std::unique_lock<std::mutex> guard(operationResultMutex_);
    type_ = type;
}

GnssGeofenceOperateResult LocationGnssGeofenceCallbackNapi::GetGeofenceOperationResult()
{
    std::unique_lock<std::mutex> guard(operationResultMutex_);
    return result_;
}

void LocationGnssGeofenceCallbackNapi::SetGeofenceOperationResult(GnssGeofenceOperateResult result)
{
    std::unique_lock<std::mutex> guard(operationResultMutex_);
    result_ = result;
}

LocationErrCode LocationGnssGeofenceCallbackNapi::DealGeofenceOperationResult()
{
    std::unique_lock<std::mutex> guard(operationResultMutex_);
    LocationErrCode errCode = ERRCODE_SUCCESS;
    GnssGeofenceOperateResult result = result_;
    switch (result) {
        case GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_SUCCESS:
            errCode = ERRCODE_SUCCESS;
            break;
        case GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_UNKNOWN:
            errCode = ERRCODE_SERVICE_UNAVAILABLE;
            break;
        case GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_TOO_MANY_GEOFENCES:
            errCode = ERRCODE_GEOFENCE_EXCEED_MAXIMUM;
            break;
        case GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_GEOFENCE_ID_EXISTS:
            errCode = ERRCODE_SERVICE_UNAVAILABLE;
            break;
        case GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_PARAMS_INVALID:
            errCode = ERRCODE_SERVICE_UNAVAILABLE;
            break;
        case GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_GEOFENCE_ID_UNKNOWN:
            errCode = ERRCODE_GEOFENCE_INCORRECT_ID;
            break;
        case GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_NOT_SUPPORTED:
            errCode = ERRCODE_NOT_SUPPORTED;
            break;
        case GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_OPERATE_FAILED:
            errCode = ERRCODE_GEOFENCE_FAIL;
            break;
        default:
            break;
    }
    return errCode;
}
}  // namespace Location
}  // namespace OHOS
