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

#include "audio_utils.h"

using namespace std::chrono_literals;

namespace OHOS {
namespace AudioStandard {

bool PermissionUtil::VerifySelfPermission()
{
    return true;
}

bool VolumeUtils::IsPCVolumeEnable()
{
    return false;
}

bool PermissionUtil::VerifySystemPermission()
{
    return false;
}

uint32_t Util::GetSamplePerFrame(const AudioSampleFormat &format)
{
    uint32_t audioPerSampleLength = 2; // 2 byte
    switch (format) {
        case AudioSampleFormat::SAMPLE_U8:
            audioPerSampleLength = 1;
            break;
        case AudioSampleFormat::SAMPLE_S16LE:
            audioPerSampleLength = 2; // 2 byte
            break;
        case AudioSampleFormat::SAMPLE_S24LE:
            audioPerSampleLength = 3; // 3 byte
            break;
        case AudioSampleFormat::SAMPLE_S32LE:
        case AudioSampleFormat::SAMPLE_F32LE:
            audioPerSampleLength = 4; // 4 byte
            break;
        default:
            break;
    }
    return audioPerSampleLength;
}

size_t Util::CalculatePcmSizeFromDurationCeiling(std::chrono::nanoseconds duration,
    uint32_t sampleRate, uint32_t bytesPerSample)
{
    size_t sampleCount = duration * sampleRate / (1s);
    if (((duration * sampleRate) % (1s)) > (0ns)) {
        sampleCount++;
    }
    return sampleCount * bytesPerSample;
}
} // namespace AudioStandard
} // namespace OHOS
