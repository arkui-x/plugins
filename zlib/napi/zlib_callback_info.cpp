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

#include "zlib_callback_info.h"

#include <unordered_map>

#include "bundle_errors.h"
#include "business_error.h"
#include "log.h"

namespace OHOS {
namespace AppExecFwk {
namespace LIBZIP {
namespace {
constexpr size_t ARGS_ONE = 1;
constexpr int32_t ERROR_CODE_ERRNO = -1;
static std::unordered_map<int32_t, int32_t> ERR_MAP = {
    { ERR_OK, SUCCESS },
    { ERR_ZLIB_SRC_FILE_DISABLED, ERR_ZLIB_SRC_FILE_INVALID },
    { ERR_ZLIB_DEST_FILE_DISABLED, ERR_ZLIB_DEST_FILE_INVALID },
    { ERR_ZLIB_SRC_FILE_FORMAT_ERROR, ERR_ZLIB_SRC_FILE_FORMAT_ERROR_OR_DAMAGED },
};
} // namespace

ZlibCallbackInfo::ZlibCallbackInfo(napi_env env, napi_ref callback, napi_deferred deferred, bool isCallback)
    : env_(env), callback_(callback), deferred_(deferred), isCallBack_(isCallback)
{}

ZlibCallbackInfo::~ZlibCallbackInfo() {}

int32_t ZlibCallbackInfo::ExcuteWork(uv_loop_s* loop, uv_work_t* work)
{
    int32_t ret = uv_queue_work(
        loop, work, [](uv_work_t* work) {},
        [](uv_work_t* work, int status) {
            if (work == nullptr) {
                return;
            }
            AsyncCallbackInfo* asyncCallbackInfo = reinterpret_cast<AsyncCallbackInfo*>(work->data);
            if (asyncCallbackInfo == nullptr) {
                return;
            }
            napi_handle_scope scope = nullptr;
            napi_open_handle_scope(asyncCallbackInfo->env, &scope);
            if (scope == nullptr) {
                return;
            }
            std::unique_ptr<AsyncCallbackInfo> callbackPtr { asyncCallbackInfo };
            napi_value result[ARGS_ONE] = { 0 };
            if (asyncCallbackInfo->deliverErrcode) {
                if (asyncCallbackInfo->callbackResult == ERR_OK) {
                    CHKRV_SCOPE(asyncCallbackInfo->env, napi_get_null(asyncCallbackInfo->env, &result[0]), scope);

                } else {
                    result[0] =
                        BusinessError::CreateCommonError(asyncCallbackInfo->env, asyncCallbackInfo->callbackResult, "");
                }
            } else {
                napi_create_int32(asyncCallbackInfo->env, asyncCallbackInfo->callbackResult, &result[0]);
            }
            if (asyncCallbackInfo->isCallBack) {
                napi_value callback = 0;
                napi_value placeHolder = nullptr;
                napi_get_reference_value(asyncCallbackInfo->env, asyncCallbackInfo->callback, &callback);
                CHKRV_SCOPE(asyncCallbackInfo->env,
                    napi_call_function(asyncCallbackInfo->env, nullptr, callback, sizeof(result) / sizeof(result[0]),
                        result, &placeHolder),
                    scope);
                if (asyncCallbackInfo->callback != nullptr) {
                    napi_delete_reference(asyncCallbackInfo->env, asyncCallbackInfo->callback);
                }
            } else {
                if (asyncCallbackInfo->callbackResult == ERR_OK) {
                    CHKRV_SCOPE(asyncCallbackInfo->env,
                        napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result[0]), scope);
                } else {
                    CHKRV_SCOPE(asyncCallbackInfo->env,
                        napi_reject_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result[0]), scope);
                }
            }
            if (work != nullptr) {
                delete work;
                work = nullptr;
            }
            napi_close_handle_scope(asyncCallbackInfo->env, scope);
        });
    return ret;
}

void ZlibCallbackInfo::OnZipUnZipFinish(ErrCode result)
{
    std::lock_guard<std::mutex> lock(validMutex_);
    if (!valid_) {
        LOGE("module exported object is invalid.");
        return;
    }

    // do callback or promise
    uv_loop_s* loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        LOGE("loop is nullptr.");
        return;
    }
    uv_work_t* work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("create work failed!");
        return;
    }
    ErrCode err = ERR_OK;
    if (!deliverErrcode_) {
        err = result == ERR_OK ? ERR_OK : ERROR_CODE_ERRNO;
    } else {
        err = ZlibCallbackInfo::ConvertErrCode(result);
    }

    AsyncCallbackInfo* asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo {
        .env = env_,
        .callback = callback_,
        .deferred = deferred_,
        .isCallBack = isCallBack_,
        .callbackResult = err,
        .deliverErrcode = deliverErrcode_,
    };
    std::unique_ptr<AsyncCallbackInfo> callbackPtr { asyncCallbackInfo };
    if (asyncCallbackInfo == nullptr) {
        delete work;
        return;
    }
    work->data = reinterpret_cast<void*>(asyncCallbackInfo);
    int32_t ret = ExcuteWork(loop, work);
    if (ret != 0) {
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
        }
        if (work != nullptr) {
            delete work;
        }
    }
    callbackPtr.release();
}

ErrCode ZlibCallbackInfo::ConvertErrCode(ErrCode nativeErrCode)
{
    if (ERR_MAP.find(nativeErrCode) != ERR_MAP.end()) {
        return ERR_MAP.at(nativeErrCode);
    }
    return ERROR_BUNDLE_SERVICE_EXCEPTION;
}

bool ZlibCallbackInfo::GetIsCallback() const
{
    return isCallBack_;
}

void ZlibCallbackInfo::SetIsCallback(bool isCallback)
{
    isCallBack_ = isCallback;
}

void ZlibCallbackInfo::SetCallback(napi_ref callback)
{
    callback_ = callback;
}

void ZlibCallbackInfo::SetDeferred(napi_deferred deferred)
{
    deferred_ = deferred;
}

void ZlibCallbackInfo::SetDeliverErrCode(bool isDeliverErrCode)
{
    deliverErrcode_ = isDeliverErrCode;
}

void ZlibCallbackInfo::SetValid(bool valid)
{
    std::lock_guard<std::mutex> lock(validMutex_);
    valid_ = valid;
}
} // namespace LIBZIP
} // namespace AppExecFwk
} // namespace OHOS