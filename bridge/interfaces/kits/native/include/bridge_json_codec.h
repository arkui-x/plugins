/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_BRIDGE_BRIDGE_JSON_CODEC_H
#define PLUGINS_BRIDGE_BRIDGE_JSON_CODEC_H

#include "bridge_base_codec.h"
#include "codecable_value.h"

#include "napi/native_api.h"
#include "nlohmann/json.hpp"

namespace OHOS::Plugin::Bridge {
using Json = nlohmann::json;

struct NapiRawValue {
    napi_env env = nullptr;
    napi_value value = nullptr;
    int argc = -1;
    napi_value* argValue = nullptr;
    int errorCode = -1;
    std::string errorMessage {};
    bool isForError = false;
};

struct DecodeValue {
    napi_env env = nullptr;
    std::string value {};
};

class BridgeJsonCodec final : public BridgeBaseCodec<NapiRawValue, DecodeValue> {
public:
    BridgeJsonCodec() = default;
    ~BridgeJsonCodec() override = default;

    BridgeJsonCodec(BridgeJsonCodec const&) = delete;
    BridgeJsonCodec& operator=(BridgeJsonCodec const&) = delete;

    static const BridgeJsonCodec& GetInstance();

    static std::string ParseNullParams(const std::string& data)
    {
        return Json::parse(data, nullptr, false).dump();
    }

private:
    virtual std::unique_ptr<DecodeValue> EncodeInner(const NapiRawValue& data) const override;
    virtual std::unique_ptr<NapiRawValue> DecodeInner(const DecodeValue& data) const override;
};
} // OHOS::Plugin::Bridge
#endif