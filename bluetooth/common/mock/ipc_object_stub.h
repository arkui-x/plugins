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

#ifndef PLUGINS_BLUETOOTH_COMMON_MOCK_IPC_OBJECT_STUB_H
#define PLUGINS_BLUETOOTH_COMMON_MOCK_IPC_OBJECT_STUB_H

#include <list>

#include "ipc_object_proxy.h"
#include "iremote_object.h"

namespace OHOS {
struct RefCountNode {
    int remotePid;
    std::string deviceId;
};

class IPCObjectStub : public IRemoteObject {
public:
    enum {
        OBJECT_TYPE_NATIVE,
        OBJECT_TYPE_JAVA,
        OBJECT_TYPE_JAVASCRIPT,
    };

    explicit IPCObjectStub(std::u16string descriptor = std::u16string());
    ~IPCObjectStub() override;

    bool IsProxyObject() const override
    {
        return false;
    };

    int32_t GetObjectRefCount() override;

    int Dump(int fd, const std::vector<std::u16string>& args) override;

    virtual int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option);

    int SendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

    void OnFirstStrongRef(const void* objectId) override;

    void OnLastStrongRef(const void* objectId) override;

    bool AddDeathRecipient(const sptr<DeathRecipient>& recipient) override;

    bool RemoveDeathRecipient(const sptr<DeathRecipient>& recipient) override;

    int GetCallingPid();

    int GetCallingUid();

    uint32_t GetCallingTokenID();

    uint64_t GetCallingFullTokenID();

    uint32_t GetFirstTokenID();

    uint64_t GetFirstFullTokenID();

    virtual int OnRemoteDump(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option);

    virtual int32_t ProcessProto(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option);

    virtual int GetObjectType() const;

#ifndef CONFIG_IPC_SINGLE
    int32_t InvokerThread(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option);
    int32_t NoticeServiceDie(MessageParcel& data, MessageParcel& reply, MessageOption& option);
    int32_t InvokerDataBusThread(MessageParcel& data, MessageParcel& reply);
    int32_t AddAuthInfo(MessageParcel& data, MessageParcel& reply, uint32_t code);

private:
    int GetPidUid(MessageParcel& data, MessageParcel& reply);
    std::string GetSessionName();
    int32_t GetSessionNameForPidUid(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option);
    int32_t GetGrantedSessionName(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option);
    std::string CreateSessionName(int uid, int pid);
    bool IsSamgrCall();
#endif
private:
    bool IsDeviceIdIllegal(const std::string& deviceID);
};
} // namespace OHOS
#endif // PLUGINS_BLUETOOTH_COMMON_MOCK_IPC_OBJECT_STUB_H