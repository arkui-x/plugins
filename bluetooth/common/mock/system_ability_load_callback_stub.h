/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_BLUETOOTH_COMMON_MOCK_SYSTEM_ABILITY_LOAD_CALLBACK_STUB_H
#define PLUGINS_BLUETOOTH_COMMON_MOCK_SYSTEM_ABILITY_LOAD_CALLBACK_STUB_H

#include "iremote_stub.h"
#include "isystem_ability_load_callback.h"

namespace OHOS {
class SystemAbilityLoadCallbackStub : public IRemoteStub<ISystemAbilityLoadCallback> {
public:
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

private:
    static bool CheckInputSystemAbilityId(int32_t systemAbilityId);
    static bool EnforceInterceToken(MessageParcel& data);

    int32_t OnLoadSystemAbilitySuccessInner(MessageParcel& data, MessageParcel& reply);
    int32_t OnLoadSystemAbilityFailInner(MessageParcel& data, MessageParcel& reply);
    int32_t OnLoadSACompleteForRemoteInner(MessageParcel& data, MessageParcel& reply);
};
} // namespace OHOS
#endif // PLUGINS_BLUETOOTH_COMMON_MOCK_SYSTEM_ABILITY_LOAD_CALLBACK_STUB_H
