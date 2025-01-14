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

#ifndef PLUGINS_BLUETOOTH_COMMON_MOCK_I_SERVICE_REGISTRY_H
#define PLUGINS_BLUETOOTH_COMMON_MOCK_I_SERVICE_REGISTRY_H

#include "if_system_ability_manager.h"
#include "ipc_object_stub.h"
#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "mock_system_ability_manager.h"

namespace OHOS {
class IServiceRegistry : public IRemoteBroker {
public:
    /**
     * GetService, Retrieve an existing service, blocking for a few seconds.
     *
     * @param name, name of service
     * @return if it doesn't yet exist. return nulltpr.
     */
    virtual sptr<IRemoteObject> GetService(const std::u16string& name) = 0;

    /**
     * CheckService, Retrieve an existing service, non-blocking.
     *
     * @param name, name of service.
     * @return if it doesn't yet exist. return nulltpr.
     */
    virtual sptr<IRemoteObject> CheckService(const std::u16string& name) = 0;

    /**
     * AddService, Register a service.
     *
     * @param name, name of service.
     * @param service, remoteobject of service.
     * @return ERR_OK indicates that the add was successful.
     */
    virtual int AddService(const std::u16string& name, const sptr<IRemoteObject>& service, bool allowIsolated = false,
        int dumpsysFlags = 0) = 0;

    enum {
        GET_SERVICE_TRANSACTION = FIRST_CALL_TRANSACTION,
        CHECK_SERVICE_TRANSACTION,
        ADD_SERVICE_TRANSACTION,
    };

public:
    DECLARE_INTERFACE_DESCRIPTOR(u"");
};

class ServiceRegistry {
public:
    /**
     * GetInstance, get Service registry instance.
     *
     * @return Get Single Instance Object.
     */
    static sptr<IServiceRegistry> GetInstance();

private:
    static std::mutex serviceRegistryLock_;
};

class SystemAbilityManagerClient {
public:
    /**
     * GetInstance, get SystemAbilityManagerClient instance.
     *
     * @return Get Single Instance Object.
     */
    static SystemAbilityManagerClient& GetInstance();

    /**
     * GetSystemAbilityManager, get system ability manager.
     *
     * @return Get systemAbilityManager_.
     */
    sptr<MockSystemAbilityManager> GetSystemAbilityManager();

    /**
     * DestroySystemAbilityManagerObject, destroy remote object of samgr.
     *
     * @return destroy systemAbilityManager_.
     */
    void DestroySystemAbilityManagerObject();

private:
    SystemAbilityManagerClient() = default;
    ~SystemAbilityManagerClient() = default;

    sptr<MockSystemAbilityManager> systemAbilityManager_;
    std::mutex systemAbilityManagerLock_;
};
} // namespace OHOS
#endif // PLUGINS_BLUETOOTH_COMMON_MOCK_I_SERVICE_REGISTRY_H