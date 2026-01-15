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

#include "message_parcel_warp.h"

#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "api/visibility.h"
#include "ashmem.h"
#include "parcel.h"
#include "pasteboard_hilog.h"
#include "securec.h"

namespace OHOS {
namespace MiscServices {

constexpr int64_t SIZE_K = 1024;
constexpr int32_t KERNEL_MAX_SIZE = 2048;  // 2G
constexpr size_t MIN_RAW_SIZE = 32 * 1024; // 32k
constexpr int32_t DEFAULT_LOCAL_CAPACITY = 128;
constexpr size_t WRITE_SPLIT_CHUNK_SIZE = 256 * 1024 * 1024;

MessageParcelWarp::MessageParcelWarp()
{
    writeRawDataFd_ = -1;
    readRawDataFd_ = -1;
    kernelMappedWrite_ = nullptr;
    kernelMappedRead_ = nullptr;
    rawData_ = nullptr;
    rawDataSize_ = 0;
    canWrite_ = true;
    canRead_ = true;
    static int32_t paramMaxSize = MIN_RAW_SIZE;
    maxRawDataSize_ = paramMaxSize * SIZE_K * SIZE_K;
}

MessageParcelWarp::~MessageParcelWarp()
{
    if (kernelMappedWrite_ != nullptr) {
        ::munmap(kernelMappedWrite_, rawDataSize_);
        kernelMappedWrite_ = nullptr;
    }
    if (kernelMappedRead_ != nullptr) {
        ::munmap(kernelMappedRead_, rawDataSize_);
        kernelMappedRead_ = nullptr;
    }

    if (readRawDataFd_ >= 0) {
        ::close(readRawDataFd_);
        readRawDataFd_ = -1;
    }
    if (writeRawDataFd_ >= 0) {
        ::close(writeRawDataFd_);
        writeRawDataFd_ = -1;
    }
    rawData_ = nullptr;
    rawDataSize_ = 0;
    canWrite_ = false;
    canRead_ = false;
}

int64_t MessageParcelWarp::GetRawDataSize()
{
    return maxRawDataSize_;
}

bool MessageParcelWarp::MemcpyData(void* ptr, size_t size, const void* data, size_t count)
{
    if (size <= WRITE_SPLIT_CHUNK_SIZE) {
        if (memcpy_s(ptr, size, data, count) != EOK) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "memcpy_s failed, size:%{public}zu", size);
            return false;
        }
        return true;
    }
    char* ptrDest = static_cast<char*>(ptr);
    const char* ptrSrc = static_cast<const char*>(data);
    size_t remaining = size;
    size_t offset = 0;
    while (remaining > 0) {
        size_t currentChunkSize = (remaining > WRITE_SPLIT_CHUNK_SIZE) ? WRITE_SPLIT_CHUNK_SIZE : remaining;
        size_t destSize = size - offset;
        size_t destChunkCount = currentChunkSize;
        size_t copyCount = (destChunkCount <= destSize) ? destChunkCount : destSize;
        if (memcpy_s(ptrDest + offset, copyCount, ptrSrc + offset, currentChunkSize) != EOK) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "memcpy_s failed, size:%{public}zu", size);
            return false;
        }
        offset += currentChunkSize;
        remaining -= currentChunkSize;
    }
    return true;
}

bool MessageParcelWarp::WriteRawData(MessageParcel& parcelPata, const void* data, size_t size)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        canWrite_, false, PASTEBOARD_MODULE_COMMON, "is already write, size:%{public}zu", size);
    canWrite_ = false;
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(data != nullptr, false, PASTEBOARD_MODULE_COMMON, "data is null");
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(0 < size && static_cast<int64_t>(size) <= maxRawDataSize_, false,
        PASTEBOARD_MODULE_COMMON, "size invalid, size:%{public}zu", size);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        kernelMappedWrite_ == nullptr, false, PASTEBOARD_MODULE_COMMON, "kernelMappedWrite_ not null end.");
    if (!parcelPata.WriteInt64(size)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "data WriteInt64 failed end.");
        return false;
    }
    if (size <= MIN_RAW_SIZE) {
        rawDataSize_ = size;
        return parcelPata.WriteUnpadBuffer(data, size);
    }
    int fd = AshmemCreate("Pasteboard Ashmem", size);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(fd >= 0, false, PASTEBOARD_MODULE_COMMON, "ashmem create failed");

    writeRawDataFd_ = fd;
    int result = AshmemSetProt(fd, PROT_READ | PROT_WRITE);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(result >= 0, false, PASTEBOARD_MODULE_COMMON, "ashmem set port failed");

    void* ptr = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        ptr != MAP_FAILED, false, PASTEBOARD_MODULE_COMMON, "mmap failed, fd:%{public}d size:%{public}zu", fd, size);

    if (!parcelPata.WriteFileDescriptor(fd)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "write file descriptor failed, size:%{public}zu", size);
        ::munmap(ptr, size);
        return false;
    }
    if (!MemcpyData(ptr, size, data, size)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "memcpy_s failed, fd:%{public}d size:%{public}zu", fd, size);
        ::munmap(ptr, size);
        return false;
    }
    kernelMappedWrite_ = ptr;
    rawDataSize_ = size;
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_COMMON, "write ashmem end. fd:%{public}d size:%{public}zu", fd, size);
    return true;
}

const void* MessageParcelWarp::ReadRawData(MessageParcel& parcelPata, size_t size)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        canRead_, nullptr, PASTEBOARD_MODULE_COMMON, "is already write, size:%{public}zu", size);
    canRead_ = false;
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(0 < size && static_cast<int64_t>(size) <= maxRawDataSize_, nullptr,
        PASTEBOARD_MODULE_COMMON, "size invalid, size:%{public}zu", size);
    size_t bufferSize = static_cast<size_t>(parcelPata.ReadInt64());
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(bufferSize == size, nullptr, PASTEBOARD_MODULE_COMMON,
        "buffer size not equal size, bufferSize:%{public}zu size:%{public}zu", bufferSize, size);
    if (bufferSize <= MIN_RAW_SIZE) {
        rawDataSize_ = size;
        return parcelPata.ReadUnpadBuffer(size);
    }

    int fd = parcelPata.ReadFileDescriptor();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        fd >= 0, nullptr, PASTEBOARD_MODULE_COMMON, "read file descriptor failed fd:%{public}d", fd);
    readRawDataFd_ = fd;

    void* ptr = ::mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        ptr != MAP_FAILED, nullptr, PASTEBOARD_MODULE_COMMON, "mmap failed, fd:%{public}d size:%{public}zu", fd, size);

    kernelMappedRead_ = ptr;
    rawDataSize_ = size;
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_COMMON, "read ashmem end. fd:%{public}d size:%{public}zu", fd, size);
    return ptr;
}

int MessageParcelWarp::CreateTmpFd()
{
    int fd = AshmemCreate("PasteboardTmpAshmem", 1);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(fd >= 0, -1, PASTEBOARD_MODULE_COMMON, "ashmem create failed");
    writeRawDataFd_ = fd;
    return fd;
}

int MessageParcelWarp::GetWriteDataFd()
{
    return writeRawDataFd_;
}
} // namespace MiscServices
} // namespace OHOS