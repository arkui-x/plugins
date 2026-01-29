/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
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

#ifndef OHOS_IPC_MESSAGE_PARCEL_H
#define OHOS_IPC_MESSAGE_PARCEL_H

#include <string>

#include "parcel.h"
#include "refbase.h"

#if !defined(__APPLE__) && !defined(_WIN32) && !defined(__gnu_linux__)
#include <ashmem.h>
#else
#include <memory>
#include <mutex>
#endif

namespace OHOS {
class MessageOption;
class MessageParcel : public Parcel {
public:
    MessageParcel() {};
    ~MessageParcel() {};
    explicit MessageParcel(Allocator* allocator) {};
    bool WriteFileDescriptor(int fd)
    {
        return true;
    };
    int ReadFileDescriptor()
    {
        return 0;
    };
    bool ContainFileDescriptors() const
    {
        return true;
    };
    bool WriteInterfaceToken(std::u16string name)
    {
        return true;
    };
    std::u16string ReadInterfaceToken()
    {
        return u"";
    };
    bool WriteRawData(const void* data, size_t size)
    {
        return true;
    };
    const void* ReadRawData(size_t size)
    {
        return nullptr;
    };
    bool RestoreRawData(std::shared_ptr<char> rawData, size_t size)
    {
        return true;
    };
    const void* GetRawData() const
    {
        return nullptr;
    };
    size_t GetRawDataSize() const
    {
        return 0;
    };
    size_t GetRawDataCapacity() const
    {
        return 0;
    };
    void WriteNoException() {};
    int32_t ReadException()
    {
        return 0;
    };
    void ClearFileDescriptor() {};
    void SetClearFdFlag()
    {
        needCloseFd_ = true;
    };
    bool Append(MessageParcel& data)
    {
        return true;
    };

#if !defined(__APPLE__) && !defined(_WIN32) && !defined(__gnu_linux__)
    bool WriteAshmem(sptr<Ashmem> ashmem);
    sptr<Ashmem> ReadAshmem();
#endif

private:
    static constexpr size_t MAX_RAWDATA_SIZE = 128 * 1024 * 1024; // 128M
    static constexpr size_t MIN_RAWDATA_SIZE = 32 * 1024;         // 32k
    bool needCloseFd_ = false;
    std::vector<sptr<Parcelable>> holders_;
    int writeRawDataFd_;
    int readRawDataFd_;
    void* kernelMappedWrite_;
    void* kernelMappedRead_;
    std::shared_ptr<char> rawData_;
    size_t rawDataSize_;
};
} // namespace OHOS
#endif // OHOS_IPC_MESSAGE_PARCEL_H
