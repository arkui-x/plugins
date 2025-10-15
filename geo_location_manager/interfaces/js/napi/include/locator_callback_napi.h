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

#ifndef LOCATOR_CALLBACK_NAPI_H
#define LOCATOR_CALLBACK_NAPI_H

#include "iremote_stub.h"
#include "napi/native_api.h"
#include "uv.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "i_locator_callback.h"
#include "location.h"

namespace OHOS {
namespace Location {
bool FindLocationCallback(napi_ref cb);
void DeleteLocationCallback(napi_ref cb);
class LocatorCallbackNapi : public IRemoteStub<ILocatorCallback> {
public:
    LocatorCallbackNapi();
    virtual ~LocatorCallbackNapi();
    void DoSendWork(uv_loop_s *&loop, uv_work_t *&work);
    void DoSendErrorCode(uv_loop_s *&loop, uv_work_t *&work);
    bool SendErrorCode(const int& errorCode);

    void OnLocationReport(const std::unique_ptr<Location>& location) override;
    void OnLocatingStatusChange(const int status) override;
    void OnErrorReport(const int errorCode) override;
    void DeleteAllCallbacks();
    void DeleteHandler();
    void DeleteSuccessHandler();
    void DeleteFailHandler();
    void DeleteCompleteHandler();
    void InitLatch();
    bool IsSystemGeoLocationApi();
    bool IsSingleLocationRequest();
    void CountDown();
    void Wait(int time);
    int GetCount();
    void SetCount(int count);
    napi_ref GetHandleCb();
    void SetHandleCb(const napi_ref& handlerCb);
    napi_env GetEnv();
    void SetEnv(const napi_env& env);

    template <typename T>
    bool InitContext(T* context)
    {
        if (context == nullptr) {
            LBSLOGE(LOCATOR_CALLBACK, "context == nullptr.");
            return false;
        }
        context->env = env_;
        if (IsSystemGeoLocationApi()) {
            context->callback[SUCCESS_CALLBACK] = successHandlerCb_;
            context->callback[FAIL_CALLBACK] = failHandlerCb_;
            context->callback[COMPLETE_CALLBACK] = completeHandlerCb_;
        } else {
            context->callback[SUCCESS_CALLBACK] = handlerCb_;
        }
        return true;
    }

    inline napi_ref GetSuccHandleCb() const
    {
        return successHandlerCb_;
    }

    void SetSuccHandleCb(const napi_ref& successHandlerCb);

    inline napi_ref GetFailHandleCb() const
    {
        return failHandlerCb_;
    }

    inline void SetFailHandleCb(const napi_ref& failHandlerCb)
    {
        failHandlerCb_ = failHandlerCb;
    }

    inline napi_ref GetCompleteHandleCb() const
    {
        return completeHandlerCb_;
    }

    inline void SetCompleteHandleCb(const napi_ref& completeHandlerCb)
    {
        completeHandlerCb_ = completeHandlerCb;
    }

    inline int GetFixNumber() const
    {
        return fixNumber_;
    }

    inline void SetFixNumber(const int fixNumber)
    {
        fixNumber_ = fixNumber;
    }

    inline void SetLocationPriority(const int locationPriority)
    {
        locationPriority_ = locationPriority;
    }

    inline int GetLocationPriority()
    {
        return locationPriority_;
    }

    inline void SetErrorType(const int errorType)
    {
        std::unique_lock<std::mutex> guard(mutex_);
        errorType_ = errorType;
    }

    inline int GetErrorType()
    {
        std::unique_lock<std::mutex> guard(mutex_);
        return errorType_;
    }

    inline std::shared_ptr<Location> GetSingleLocation()
    {
        std::unique_lock<std::mutex> guard(mutex_);
        return singleLocation_;
    }
    bool NeedSetSingleLocation(const std::unique_ptr<Location>& location);
    bool IfReportAccuracyLocation();
    void SetSingleLocation(const std::unique_ptr<Location>& location);

private:
    napi_env env_;
    napi_ref handlerCb_;
    napi_ref successHandlerCb_;
    napi_ref failHandlerCb_;
    napi_ref completeHandlerCb_;
    int fixNumber_;
    std::mutex mutex_;
    CountDownLatch* latch_;
    std::shared_ptr<Location> singleLocation_;
    int locationPriority_;
    bool inHdArea_;
    int errorType_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATOR_CALLBACK_NAPI_H
