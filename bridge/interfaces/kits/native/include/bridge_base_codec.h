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

#ifndef PLUGINS_BRIDGE_BRIDGE_BASE_CODEC_H
#define PLUGINS_BRIDGE_BRIDGE_BASE_CODEC_H

#include <memory>
#include <string>

namespace OHOS::Plugin::Bridge {
template <typename T, typename R>
class BridgeBaseCodec {
public:
    BridgeBaseCodec() = default;
    virtual ~BridgeBaseCodec() = default;

    BridgeBaseCodec(BridgeBaseCodec<T, R> const&) = delete;
    BridgeBaseCodec& operator=(BridgeBaseCodec<T, R> const&) = delete;

    std::unique_ptr<R> Encode(const T& data) const
    {
        return std::move(EncodeInner(data));
    }

    std::unique_ptr<T> Decode(const R& data) const
    {
        return std::move(DecodeInner(data));
    }

protected:
    virtual std::unique_ptr<R> EncodeInner(const T& data) const = 0;
    virtual std::unique_ptr<T> DecodeInner(const R& data) const = 0;
};
} // OHOS::Plugin::Bridge
#endif