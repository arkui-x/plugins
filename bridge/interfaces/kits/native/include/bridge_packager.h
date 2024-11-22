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

#ifndef PLUGINS_BRIDGE_BRIDGE_PACKAGER_H
#define PLUGINS_BRIDGE_BRIDGE_PACKAGER_H

#include <functional>
#include <map>
#include <vector>

#include "bridge_binary_marshaller.h"
#include "bridge_binary_unmarshaller.h"
#include "codecable_value.h"

namespace OHOS::Plugin::Bridge {
class BridgePackager {
public:
    using UnMarshallFunc = std::function<CodecableValue(BridgeBinaryUnmarshaller*)>;
    using UnMarshallFuncMap = std::map<CodecableIndex, UnMarshallFunc>;
    using UnMarshallFuncValueType = std::map<CodecableIndex, UnMarshallFunc>::value_type;
    const static UnMarshallFuncMap unMarshallFuncMap;

    using MarshallFunc = std::function<void(const CodecableValue& value, BridgeBinaryMarshaller* buffer)>;
    using MarshallFuncMap = std::map<CodecableType, MarshallFunc>;
    using MarshallFuncValueType = std::map<CodecableType, MarshallFunc>::value_type;
    const static MarshallFuncMap marshallFuncMap;

    static CodecableIndex GetCodecableIndex(const CodecableValue& value);

    static CodecableValue UnMarshalling(BridgeBinaryUnmarshaller* pendingBuffer);
    static void Marshalling(const CodecableValue& value, BridgeBinaryMarshaller* pendingBuffer);

    static size_t UnMarshallingSize(BridgeBinaryUnmarshaller* pendingBuffer);
    static void MarshallingSize(size_t size, BridgeBinaryMarshaller* pendingBuffer);

    static CodecableValue UnMarshallingString(BridgeBinaryUnmarshaller* pendingBuffer);
    static void MarshallingString(const std::string& str, BridgeBinaryMarshaller* pendingBuffer);

    static void MarshallingListBool(const std::vector<bool>& vector, BridgeBinaryMarshaller* pendingBuffer);
    static CodecableValue UnMarshallingListBool(BridgeBinaryUnmarshaller* pendingBuffer);

    static void MarshallingListString(const std::vector<std::string>& vector, BridgeBinaryMarshaller* pendingBuffer);
    static CodecableValue UnMarshallingListString(BridgeBinaryUnmarshaller* pendingBuffer);

    static CodecableValue UnMarshallingMap(BridgeBinaryUnmarshaller* pendingBuffer);
    static void MarshallingMap(const CodecableMap& map, BridgeBinaryMarshaller* pendingBuffer);

    static CodecableValue UnMarshallingCompositeList(BridgeBinaryUnmarshaller* pendingBuffer);
    static void MarshallingCompositeList(const CodecableList& value, BridgeBinaryMarshaller* pendingBuffer);

    template <typename T>
    static CodecableValue UnMarshallingVector(BridgeBinaryUnmarshaller* pendingBuffer);
    template <typename T>
    static void MarshallingVector(const std::vector<T>& vector, BridgeBinaryMarshaller* pendingBuffer);
};
} // OHOS::Plugin::Bridge
#endif