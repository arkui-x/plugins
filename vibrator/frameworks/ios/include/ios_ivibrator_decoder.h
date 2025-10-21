/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef IOS_IVIBRATOR_DECODER_H
#define IOS_IVIBRATOR_DECODER_H

#include "raw_file_descriptor.h"
#include "vibrator_infos.h"

namespace OHOS {
namespace Sensors {

class IosJsonParser;

class IosIVibratorDecoder {
public:
    virtual ~IosIVibratorDecoder() = default;
    virtual int32_t DecodeEffect(const RawFileDescriptor &rawFd, const IosJsonParser &parser, VibratePackage &pkg) = 0;
};

} // namespace Sensors
} // namespace OHOS

#endif //IOS_IVIBRATOR_DECODER_H
