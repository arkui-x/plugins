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

#include "message_parcel.h"

#include "iremote_object.h"

namespace OHOS {
MessageParcel::MessageParcel()
    : Parcel(), writeRawDataFd_(-1), readRawDataFd_(-1), kernelMappedWrite_(nullptr), kernelMappedRead_(nullptr),
      rawData_(nullptr), rawDataSize_(0)
{}

MessageParcel::MessageParcel(Allocator* allocator)
    : Parcel(allocator), writeRawDataFd_(-1), readRawDataFd_(-1), kernelMappedWrite_(nullptr),
      kernelMappedRead_(nullptr), rawData_(nullptr), rawDataSize_(0)
{}

MessageParcel::~MessageParcel() {}

#ifndef CONFIG_IPC_SINGLE
bool MessageParcel::WriteDBinderProxy(const sptr<IRemoteObject>& object, uint32_t handle, uint64_t stubIndex)
{
    (void)object;
    (void)handle;
    (void)stubIndex;
    return false;
}
#endif

bool MessageParcel::WriteRemoteObject(const sptr<IRemoteObject>& object)
{
    (void)object;
    return false;
}

sptr<IRemoteObject> MessageParcel::ReadRemoteObject()
{
    return nullptr;
}

bool MessageParcel::WriteFileDescriptor(int fd)
{
    return false;
}

int MessageParcel::ReadFileDescriptor()
{
    return -1;
}

void MessageParcel::ClearFileDescriptor() {}

bool MessageParcel::ContainFileDescriptors() const
{
    return false;
}

bool MessageParcel::WriteInterfaceToken(std::u16string name)
{
    (void)name;
    return false;
}

std::u16string MessageParcel::ReadInterfaceToken()
{
    return u"";
}

bool MessageParcel::WriteRawData(const void* data, size_t size)
{
    (void)data;
    (void)size;
    return false;
}

bool MessageParcel::RestoreRawData(std::shared_ptr<char> rawData, size_t size)
{
    (void)rawData;
    (void)size;
    return false;
}

const void* MessageParcel::ReadRawData(size_t size)
{
    (void)size;
    return nullptr;
}

const void* MessageParcel::GetRawData() const
{
    return nullptr;
}

size_t MessageParcel::GetRawDataSize() const
{
    return -1;
}

size_t MessageParcel::GetRawDataCapacity() const
{
    return -1;
}

void MessageParcel::WriteNoException() {}

int32_t MessageParcel::ReadException()
{
    return -1;
}
} // namespace OHOS
