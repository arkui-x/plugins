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

#include "system_ability_load_callback_stub.h"

#include <cinttypes>

#include "datetime_ex.h"
#include "errors.h"
#include "ipc_object_stub.h"
#include "ipc_types.h"
#include "message_parcel.h"
#include "refbase.h"

namespace OHOS {
namespace {
constexpr int32_t FIRST_SYS_ABILITY_ID = 0x00000000;
constexpr int32_t LAST_SYS_ABILITY_ID = 0x00ffffff;
} // namespace
int32_t SystemAbilityLoadCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    return -1;
}

int32_t SystemAbilityLoadCallbackStub::OnLoadSystemAbilitySuccessInner(MessageParcel& data, MessageParcel& reply)
{
    return ERR_NONE;
}

int32_t SystemAbilityLoadCallbackStub::OnLoadSystemAbilityFailInner(MessageParcel& data, MessageParcel& reply)
{
    return ERR_NONE;
}

int32_t SystemAbilityLoadCallbackStub::OnLoadSACompleteForRemoteInner(MessageParcel& data, MessageParcel& reply)
{
    return ERR_NONE;
}

bool SystemAbilityLoadCallbackStub::CheckInputSystemAbilityId(int32_t systemAbilityId)
{
    return true;
}

bool SystemAbilityLoadCallbackStub::EnforceInterceToken(MessageParcel& data)
{
    return true;
}
} // namespace OHOS
