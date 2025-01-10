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

#include "ipc_object_stub.h"

namespace OHOS {
IPCObjectStub::IPCObjectStub(std::u16string descriptor) : IRemoteObject(descriptor) {}

IPCObjectStub::~IPCObjectStub() {}

bool IPCObjectStub::IsDeviceIdIllegal(const std::string& deviceID)
{
    return false;
}

int32_t IPCObjectStub::GetObjectRefCount()
{
    return 0;
}

int IPCObjectStub::Dump(int fd, const std::vector<std::u16string>& args)
{
    return -1;
}

int IPCObjectStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    return -1;
}

int IPCObjectStub::OnRemoteDump(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    return -1;
}

int IPCObjectStub::SendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    return -1;
}

void IPCObjectStub::OnFirstStrongRef(const void* objectId) {}

void IPCObjectStub::OnLastStrongRef(const void* objectId) {}

bool IPCObjectStub::AddDeathRecipient(const sptr<DeathRecipient>& recipient)
{
    (void)recipient;
    return false;
}

bool IPCObjectStub::RemoveDeathRecipient(const sptr<DeathRecipient>& recipient)
{
    return false;
}

int IPCObjectStub::GetCallingPid()
{
    return 0;
}

int IPCObjectStub::GetCallingUid()
{
    return 0;
}

uint32_t IPCObjectStub::GetCallingTokenID()
{
    return 0;
}

uint32_t IPCObjectStub::GetFirstTokenID()
{
    return 0;
}

int IPCObjectStub::GetObjectType() const
{
    return -1;
}

int32_t IPCObjectStub::ProcessProto(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    return -1;
}

#ifndef CONFIG_IPC_SINGLE
int32_t IPCObjectStub::InvokerThread(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    return -1;
}

int32_t IPCObjectStub::InvokerDataBusThread(MessageParcel& data, MessageParcel& reply)
{
    return -1;
}

int32_t IPCObjectStub::NoticeServiceDie(MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    return -1;
}

int32_t IPCObjectStub::AddAuthInfo(MessageParcel& data, MessageParcel& reply, uint32_t code)
{
    return -1;
}
#endif
} // namespace OHOS
