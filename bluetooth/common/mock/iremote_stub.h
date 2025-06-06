/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_BLUETOOTH_COMMON_MOCK_I_REMOTE_STUB_H
#define PLUGINS_BLUETOOTH_COMMON_MOCK_I_REMOTE_STUB_H

#include <string>

#include "ipc_object_stub.h"
#include "iremote_broker.h"

namespace OHOS {
template<typename INTERFACE>
class IRemoteStub : public IPCObjectStub, public INTERFACE {
public:
    IRemoteStub();
    virtual ~IRemoteStub() override = default;
    sptr<IRemoteObject> AsObject() override;
    sptr<IRemoteBroker> AsInterface() override;
};

template<typename INTERFACE>
IRemoteStub<INTERFACE>::IRemoteStub() : IPCObjectStub(INTERFACE::GetDescriptor())
{}

template<typename INTERFACE>
sptr<IRemoteBroker> IRemoteStub<INTERFACE>::AsInterface()
{
    return this;
}

template<typename INTERFACE>
sptr<IRemoteObject> IRemoteStub<INTERFACE>::AsObject()
{
    return this;
}
} // namespace OHOS
#endif // PLUGINS_BLUETOOTH_COMMON_MOCK_I_REMOTE_STUB_H
