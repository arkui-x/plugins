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

#ifndef OHOS_APPEXECFWK_LIBZIP_ZLIB_CALLBACK_H
#define OHOS_APPEXECFWK_LIBZIP_ZLIB_CALLBACK_H

#include <uv.h>

#include "appexecfwk_errors.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "nocopyable.h"
#include "zip_utils.h"

namespace OHOS {
namespace AppExecFwk {
namespace LIBZIP {
class ZlibCallbackInfo {
#define CHKRV_SCOPE(env, state, scope)           \
    do {                                         \
        if ((state) != napi_ok) {                \
            napi_close_handle_scope(env, scope); \
            return;                              \
        }                                        \
    } while (0)

public:
    ZlibCallbackInfo() = default;
    ZlibCallbackInfo(napi_env env, napi_ref callback, napi_deferred deferred, bool isCallback);
    virtual ~ZlibCallbackInfo();
    void OnZipUnZipFinish(ErrCode result);
    bool GetIsCallback() const;
    void SetIsCallback(bool isCallback);
    void SetCallback(napi_ref callback);
    void SetDeferred(napi_deferred deferred);
    void SetDeliverErrCode(bool isDeliverErrCode);
    void SetValid(bool valid);
    static ErrCode ConvertErrCode(ErrCode nativeErrCode);

private:
    int32_t ExcuteWork(uv_loop_s* loop, uv_work_t* work);

private:
    napi_env env_ = nullptr;
    napi_ref callback_ = nullptr;
    napi_deferred deferred_ = nullptr;
    bool isCallBack_ = false;
    bool deliverErrcode_ = false;
    bool valid_ = true;
    std::mutex validMutex_;
    DISALLOW_COPY_AND_MOVE(ZlibCallbackInfo);
};

struct AsyncCallbackInfo {
    napi_env env;
    napi_ref callback;
    napi_deferred deferred;
    bool isCallBack;
    ErrCode callbackResult;
    bool deliverErrcode;
};
} // namespace LIBZIP
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_APPEXECFWK_LIBZIP_ZLIB_CALLBACK_H