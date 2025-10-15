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

#ifndef LOCATION_SWITCH_CALLBACK_NAPI_H
#define LOCATION_SWITCH_CALLBACK_NAPI_H
#include "i_switch_callback.h"

#include "iremote_stub.h"
#include "napi_util.h"
#include "napi/native_api.h"
#include "uv.h"

namespace OHOS {
namespace Location {
bool FindSwitchCallback(napi_ref cb);
void DeleteSwitchCallback(napi_ref cb);
class LocationSwitchCallbackNapi : public IRemoteStub<ISwitchCallback> {
public:
    LocationSwitchCallbackNapi();
    virtual ~LocationSwitchCallbackNapi();
    bool IsRemoteDied();
    napi_value PackResult(bool switchState);
    bool Send(int switchState);
    void OnSwitchChange(int switchState) override;
    void DeleteHandler();
    void UvQueueWork(uv_loop_s* loop, uv_work_t* work);
    napi_ref GetHandleCb();
    void SetHandleCb(const napi_ref& handlerCb);
    napi_env GetEnv();
    void SetEnv(const napi_env& env);

    template <typename T>
    bool InitContext(T* context)
    {
        if (context == nullptr) {
            LBSLOGE(SWITCH_CALLBACK, "context == nullptr.");
            return false;
        }
        context->env = env_;
        context->callback[SUCCESS_CALLBACK] = handlerCb_;
        return true;
    }

    inline bool GetRemoteDied() const
    {
        return remoteDied_;
    }

    inline void SetRemoteDied(const bool remoteDied)
    {
        remoteDied_ = remoteDied;
    }

private:
    napi_env env_;
    napi_ref handlerCb_;
    bool remoteDied_;
    std::mutex mutex_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_SWITCH_CALLBACK_NAPI_H
