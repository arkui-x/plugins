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

#ifndef MESSAGE_PARCEL_WARP_H
#define MESSAGE_PARCEL_WARP_H

#include "api/visibility.h"
#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {

constexpr int64_t DEFAULT_MAX_RAW_DATA_SIZE = 128 * 1024 * 1024; // 128M

class API_EXPORT MessageParcelWarp {
public:
    MessageParcelWarp();
    ~MessageParcelWarp();

    bool WriteRawData(MessageParcel& parcelPata, const void* data, size_t size);
    const void* ReadRawData(MessageParcel& parcelData, size_t size);
    static int64_t GetRawDataSize();
    int CreateTmpFd();
    int GetWriteDataFd();
    bool MemcpyData(void* ptr, size_t size, const void* data, size_t count);

private:
    std::shared_ptr<char> rawData_ = nullptr;
    int writeRawDataFd_ = -1;
    int readRawDataFd_ = -1;
    void* kernelMappedWrite_ = nullptr;
    void* kernelMappedRead_ = nullptr;
    size_t rawDataSize_ = 0;
    bool canWrite_ = true;
    bool canRead_ = true;
    static inline int64_t maxRawDataSize_ = DEFAULT_MAX_RAW_DATA_SIZE;
};
} // namespace MiscServices
} // namespace OHOS
#endif // MESSAGE_PARCEL_WARP_H