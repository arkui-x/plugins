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

#ifndef PLUGINS_BLUETOOTH_COMMON_MOCK_IF_SYSTEM_ABILITY_MANAGER_H
#define PLUGINS_BLUETOOTH_COMMON_MOCK_IF_SYSTEM_ABILITY_MANAGER_H

#include <list>
#include <string>

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "isystem_ability_load_callback.h"
#include "isystem_ability_status_change.h"
#include "isystem_process_status_change.h"
#include "samgr_ipc_interface_code.h"
#include "system_ability_on_demand_event.h"


namespace OHOS {
class ISystemAbilityManager : public IRemoteBroker {
public:
    /**
     * ListSystemAbilities, Return list of all existing abilities.
     *
     * @param dumpFlags, dump all.
     * @return Returns the sa where the current samgr exists.
     */
    virtual std::vector<std::u16string> ListSystemAbilities(unsigned int dumpFlags = DUMP_FLAG_PRIORITY_ALL) = 0;

    enum {
        SHEEFT_CRITICAL = 0,
        SHEEFT_HIGH,
        SHEEFT_NORMAL,
        SHEEFT_DEFAULT,
        SHEEFT_PROTO,
    };

    static const unsigned int DUMP_FLAG_PRIORITY_CRITICAL = 1 << SHEEFT_CRITICAL;
    static const unsigned int DUMP_FLAG_PRIORITY_HIGH = 1 << SHEEFT_HIGH;
    static const unsigned int DUMP_FLAG_PRIORITY_NORMAL = 1 << SHEEFT_NORMAL;

    static const unsigned int DUMP_FLAG_PRIORITY_DEFAULT = 1 << SHEEFT_DEFAULT;
    static const unsigned int DUMP_FLAG_PRIORITY_ALL =
        DUMP_FLAG_PRIORITY_CRITICAL | DUMP_FLAG_PRIORITY_HIGH | DUMP_FLAG_PRIORITY_NORMAL | DUMP_FLAG_PRIORITY_DEFAULT;
    static const unsigned int DUMP_FLAG_PROTO = 1 << SHEEFT_PROTO;

    /**
     * GetSystemAbility, Retrieve an existing ability, retrying and blocking for a few seconds if it doesn't exist.
     *
     * @param systemAbilityId, Need to obtain the said of sa.
     * @return nullptr indicates acquisition failure.
     */
    virtual sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId) = 0;

    /**
     * CheckSystemAbility, Retrieve an existing ability, no-blocking.
     *
     * @param systemAbilityId, Need to obtain the said of sa.
     * @return nullptr indicates acquisition failure.
     */
    virtual sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId) = 0;

    /**
     * RemoveSystemAbility, Remove an ability.
     *
     * @param systemAbilityId, Need to remove the said of sa.
     * @return ERR_OK indicates remove success.
     */
    virtual int32_t RemoveSystemAbility(int32_t systemAbilityId) = 0;

    /**
     * SubscribeSystemAbility, Subscribe a system ability status.
     *
     * @param systemAbilityId, Need to subscribe the said of sa.
     * @param listener, Need to implement OnAddSystemAbility, OnRemoveSystemAbility.
     * @return ERR_OK indicates SubscribeSystemAbility success.
     */
    virtual int32_t SubscribeSystemAbility(
        int32_t systemAbilityId, const sptr<ISystemAbilityStatusChange>& listener) = 0;

    /**
     * UnSubscribeSystemAbility, UnSubscribe a system ability status.
     *
     * @param systemAbilityId, Need to UnSubscribe the said of sa.
     * @param listener, Need to implement OnAddSystemAbility, OnRemoveSystemAbility.
     * @return ERR_OK indicates SubscribeSystemAbility success.
     */
    virtual int32_t UnSubscribeSystemAbility(
        int32_t systemAbilityId, const sptr<ISystemAbilityStatusChange>& listener) = 0;

    /**
     * GetSystemAbility, Retrieve an existing ability, blocking for a few seconds if it doesn't exist.
     *
     * @param systemAbilityId, Need to get the said of sa.
     * @param deviceId, If the device id is empty, it indicates that it is a local get.
     * @return nullptr indicates acquisition failure.
     */
    virtual sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId, const std::string& deviceId) = 0;

    /**
     * CheckSystemAbility, Retrieve an existing ability, no-blocking.
     *
     * @param systemAbilityId, Need to get the said of sa.
     * @param deviceId, If the device id is empty, it indicates that it is a local get.
     * @return nullptr indicates acquisition failure.
     */
    virtual sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, const std::string& deviceId) = 0;

    /**
     * AddOnDemandSystemAbilityInfo, Add ondemand ability info.
     *
     * @param systemAbilityId, Need to add info the said of sa.
     * @param localAbilityManagerName, Process Name.
     * @return ERR_OK indicates AddOnDemandSystemAbilityInfo success.
     */
    virtual int32_t AddOnDemandSystemAbilityInfo(
        int32_t systemAbilityId, const std::u16string& localAbilityManagerName) = 0;

    /**
     * CheckSystemAbility, Retrieve an ability, no-blocking.
     *
     * @param systemAbilityId, Need to check the said of sa.
     * @param isExist, Issue parameters, and a result of true indicates success.
     * @return nullptr indicates acquisition failure.
     */
    virtual sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, bool& isExist) = 0;

    struct SAExtraProp {
        SAExtraProp() = default;
        SAExtraProp(bool isDistributed, unsigned int dumpFlags, const std::u16string& capability,
            const std::u16string& permission)
        {
            this->isDistributed = isDistributed;
            this->dumpFlags = dumpFlags;
            this->capability = capability;
            this->permission = permission;
        }

        bool isDistributed = false;
        unsigned int dumpFlags = DUMP_FLAG_PRIORITY_DEFAULT;
        std::u16string capability;
        std::u16string permission;
    };

    /**
     * AddSystemAbility, add an ability to samgr.
     *
     * @param systemAbilityId, Need to add the said of sa.
     * @param ability, SA to be added.
     * @param extraProp, Additional parameters for sa, such as whether it is distributed.
     * @return ERR_OK indicates successful add.
     */
    virtual int32_t AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
        const SAExtraProp& extraProp = SAExtraProp(false, DUMP_FLAG_PRIORITY_DEFAULT, u"", u"")) = 0;

    /**
     * AddSystemProcess, add an process.
     *
     * @param procName, Need to add the procName of process.
     * @param procObject, Remoteobject of procName.
     * @return ERR_OK indicates successful add.
     */
    virtual int32_t AddSystemProcess(const std::u16string& procName, const sptr<IRemoteObject>& procObject) = 0;

    /**
     * LoadSystemAbility, Load sa.
     *
     * @param systemAbilityId, Need to load the said of sa.
     * @param timeout, limited time to load sa.
     * @return remote object means that the load was successful.
     */
    virtual sptr<IRemoteObject> LoadSystemAbility(int32_t systemAbilityId, int32_t timeout) = 0;

    /**
     * LoadSystemAbility, Load sa.
     *
     * @param systemAbilityId, Need to load the said of sa.
     * @param callback, OnLoadSystemAbilityFail and OnLoadSystemAbilitySuccess need be rewritten.
     * @return ERR_OK It does not mean that the load was successful.
     */
    virtual int32_t LoadSystemAbility(int32_t systemAbilityId, const sptr<ISystemAbilityLoadCallback>& callback) = 0;

    /**
     * LoadSystemAbility, Load sa.
     *
     * @param systemAbilityId, Need to load the said of sa.
     * @param deviceId, if deviceId is empty, it indicates local load.
     * @param callback, OnLoadSystemAbilityFail and OnLoadSystemAbilitySuccess need be rewritten.
     * @return ERR_OK It does not mean that the load was successful.
     */
    virtual int32_t LoadSystemAbility(
        int32_t systemAbilityId, const std::string& deviceId, const sptr<ISystemAbilityLoadCallback>& callback) = 0;

    /**
     * UnloadSystemAbility, UnLoad sa.
     *
     * @param systemAbilityId, Need to UnLoad the said of sa.
     * @return ERR_OK It does not mean that the unload was successful, but sa entered an idle state.
     */
    virtual int32_t UnloadSystemAbility(int32_t systemAbilityId) = 0;

    /**
     * CancelUnloadSystemAbility, CancelUnload sa.
     *
     * @param systemAbilityId, Need to CancelUnload the said of sa.
     * @return ERR_OK indicates that the uninstall was canceled successfully.
     */
    virtual int32_t CancelUnloadSystemAbility(int32_t systemAbilityId) = 0;

    /**
     * UnloadAllIdleSystemAbility, unload all idle sa.
     * only support for memmgrservice
     *
     * @return ERR_OK It means unload all idle sa success.
     */
    virtual int32_t UnloadAllIdleSystemAbility() = 0;

    /**
     * GetSystemProcessInfo, Get process info by said.
     *
     * @param systemAbilityId, Need the said of sa which wants to get process info.
     * @param systemProcessInfo, Issue a parameter and return it as a result.
     * @return ERR_OK indicates that the get successfully.
     */
    virtual int32_t GetSystemProcessInfo(int32_t systemAbilityId, SystemProcessInfo& systemProcessInfo) = 0;

    /**
     * GetRunningSystemProcess, Get all processes currently running.
     *
     * @param systemProcessInfos, Issue a parameter and return it as a result.
     * @return ERR_OK indicates that the get successfully.
     */
    virtual int32_t GetRunningSystemProcess(std::list<SystemProcessInfo>& systemProcessInfos) = 0;

    /**
     * SubscribeSystemProcess, Subscribe the status of process.
     *
     * @param listener, callback
     * @return ERR_OK indicates that the Subscribe successfully.
     */
    virtual int32_t SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener) = 0;

    /**
     * SendStrategy, Send strategy to SA.
     *
     * @param type, type is a certain device status type.
     * @param systemAbilityIds, Need the vector of said which wants to send strategy.
     * @param level, level is level of a certain device status type.
     * @param action, action is scheduling strategy.
     * @return ERR_OK indicates that the Subscribe successfully.
     */
    virtual int32_t SendStrategy(
        int32_t type, std::vector<int32_t>& systemAbilityIds, int32_t level, std::string& action) = 0;

    /**
     * UnSubscribeSystemProcess, UnSubscribe the status of process.
     *
     * @param listener, callback
     * @return ERR_OK indicates that the UnSubscribe successfully.
     */
    virtual int32_t UnSubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener) = 0;

    /**
     * GetExtensionSaIds, Return list of saId that match extension.
     *
     * @param extension, extension, match with profile extension.
     * @param saIds, list of saId that match extension
     * @return ERR_OK indicates that the list of saId that match extension success.
     */
    virtual int32_t GetExtensionSaIds(const std::string& extension, std::vector<int32_t>& saIds) = 0;

    /**
     * GetExtensionRunningSaList, Return started list of hanlde that match extension.
     *
     * @param extension, extension, match with profile extension.
     * @param saList, started list of remote obj that match extension
     * @return ERR_OK indicates that the list of hanlde that match extension success.
     */
    virtual int32_t GetExtensionRunningSaList(
        const std::string& extension, std::vector<sptr<IRemoteObject>>& saList) = 0;

    /**
     * GetRunningSaExtensionInfoList, Return list of started said and process hanlde that match extension.
     *
     * @param extension, extension, match with profile extension.
     * @param infoList, list of started said and sa process remote obj that match extension
     * @return ERR_OK indicates that the list of hanlde that match extension success.
     */
    struct SaExtensionInfo {
        int32_t saId = -1;
        sptr<IRemoteObject> processObj = nullptr;
    };
    virtual int32_t GetRunningSaExtensionInfoList(
        const std::string& extension, std::vector<SaExtensionInfo>& infoList) = 0;

    virtual int32_t GetOnDemandReasonExtraData(int64_t extraDataId, MessageParcel& extraDataParcel) = 0;
    virtual int32_t GetOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
        std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents) = 0;
    virtual int32_t UpdateOnDemandPolicy(int32_t systemAbilityId, OnDemandPolicyType type,
        const std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents) = 0;
    virtual int32_t GetOnDemandSystemAbilityIds(std::vector<int32_t>& systemAbilityIds) = 0;

public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.ISystemAbilityManager");

protected:
    static constexpr int32_t FIRST_SYS_ABILITY_ID = 0x00000000;
    static constexpr int32_t LAST_SYS_ABILITY_ID = 0x00ffffff;
    bool CheckInputSysAbilityId(int32_t sysAbilityId) const
    {
        if (sysAbilityId >= FIRST_SYS_ABILITY_ID && sysAbilityId <= LAST_SYS_ABILITY_ID) {
            return true;
        }
        return false;
    }
    static inline const std::u16string SAMANAGER_INTERFACE_TOKEN = u"ohos.samgr.accessToken";
};
} // namespace OHOS
#endif // defined(PLUGINS_BLUETOOTH_COMMON_MOCK_IF_SYSTEM_ABILITY_MANAGER_H