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

#ifndef BLUETOOTH_SCAN_RESULT_CALLBACK_NAPI_H
#define BLUETOOTH_SCAN_RESULT_CALLBACK_NAPI_H

#include "iremote_stub.h"
#include "napi/native_api.h"
#include "uv.h"

#include "constant_definition.h"
#include "location_log.h"
#include "ibluetooth_scan_result_callback.h"
#include "bluetooth_scan_result.h"

namespace OHOS {
namespace Location {
bool FindBlueToothCallback(napi_ref cb);
void DeleteBlueToothCallback(napi_ref cb);
class BluetoothScanResultCallbackNapi : public IRemoteStub<IBluetoothScanResultCallback> {
public:
    BluetoothScanResultCallbackNapi();
    virtual ~BluetoothScanResultCallbackNapi();

    void DoSendWork(uv_loop_s *&loop, uv_work_t *&work);
    void OnBluetoothScanResultChange(const std::unique_ptr<BluetoothScanResult>& bluetoothScanResult) override;
    void DeleteHandler();
    bool FindCallback();
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
#endif // BLUETOOTH_SCAN_RESULT_CALLBACK_NAPI_H
