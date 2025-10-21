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

#ifndef COUNTRY_CODE_CALLBACK_NAPI_H
#define COUNTRY_CODE_CALLBACK_NAPI_H

#include "i_country_code_callback.h"
#include "iremote_stub.h"
#include "napi/native_api.h"
#include "uv.h"
#include "country_code.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
bool FindCountryCodeCallback(napi_ref cb);
void DeleteCountryCodeCallback(napi_ref cb);
class CountryCodeCallbackNapi : public IRemoteStub<ICountryCodeCallback> {
public:
    CountryCodeCallbackNapi();
    virtual ~CountryCodeCallbackNapi();
    void OnCountryCodeChange(const std::shared_ptr<CountryCode>& country) override;
    bool Send(const std::shared_ptr<CountryCode>& country);
    void SetEnv(napi_env env);
    void SetCallback(napi_ref cb);
    void DeleteHandler();
    void UvQueueWork(uv_loop_s* loop, uv_work_t* work);

    template <typename T>
    bool InitContext(T* context)
    {
        if (context == nullptr) {
            LBSLOGE(COUNTRY_CODE_CALLBACK, "context == nullptr.");
            return false;
        }
        context->env = env_;
        context->callback[SUCCESS_CALLBACK] = handlerCb_;
        return true;
    }
private:
    napi_env env_;
    napi_ref handlerCb_;
    std::mutex mutex_;
};
} // namespace Location
} // namespace OHOS
#endif // COUNTRY_CODE_CALLBACK_NAPI_H
