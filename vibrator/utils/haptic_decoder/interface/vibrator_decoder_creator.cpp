/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "vibrator_decoder_creator.h"

#include "default_vibrator_decoder_factory.h"
#include "he_vibrator_decoder_factory.h"
#include "sensors_errors.h"

#undef LOG_TAG
#define LOG_TAG "VibratorDecoderCreator"

namespace OHOS {
namespace Sensors {
namespace {
const std::string JSON_TAG = "Channels";
} // namespace
IVibratorDecoder *VibratorDecoderCreator::CreateDecoder(const JsonParser &parser)
{
    CALL_LOG_ENTER;
    if (CheckJsonMetadata(parser)) {
        MISC_HILOGD("Get oh_json tag");
        DefaultVibratorDecoderFactory factory;
        return factory.CreateDecoder();
    } else {
        MISC_HILOGD("Get he tag");
        HEVibratorDecoderFactory factory;
        return factory.CreateDecoder();
    }
}

bool VibratorDecoderCreator::CheckJsonMetadata(const JsonParser &parser)
{
    return parser.HasObjectItem(JSON_TAG);
}

extern "C" IVibratorDecoder *Create(const JsonParser &parser)
{
    VibratorDecoderCreator creator;
    return creator.CreateDecoder(parser);
}

extern "C" void Destroy(IVibratorDecoder *decoder)
{
    if (decoder != nullptr) {
        delete decoder;
        decoder = nullptr;
    }
}
} // namespace Sensors
} // namespace OHOS