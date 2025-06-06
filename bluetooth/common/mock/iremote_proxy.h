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

#ifndef PLUGINS_BLUETOOTH_COMMON_MOCK_I_REMOTE_PROXY_H
#define PLUGINS_BLUETOOTH_COMMON_MOCK_I_REMOTE_PROXY_H

#include "iremote_broker.h"
#include "iremote_object.h"
#include "peer_holder.h"

namespace OHOS {
template<typename INTERFACE>
class IRemoteProxy : public PeerHolder, public INTERFACE {
public:
    explicit IRemoteProxy(const sptr<IRemoteObject>& object);
    ~IRemoteProxy() override = default;

protected:
    sptr<IRemoteObject> AsObject() override;
};

template<typename INTERFACE>
IRemoteProxy<INTERFACE>::IRemoteProxy(const sptr<IRemoteObject>& object) : PeerHolder(object)
{}

template<typename INTERFACE>
sptr<IRemoteObject> IRemoteProxy<INTERFACE>::AsObject()
{
    return Remote();
}
} // namespace OHOS
#endif // PLUGINS_BLUETOOTH_COMMON_MOCK_I_REMOTE_PROXY_H
