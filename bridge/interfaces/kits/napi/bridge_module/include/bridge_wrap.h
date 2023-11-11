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

#ifndef PLUGINS_BRIDGE_WRAP_H
#define PLUGINS_BRIDGE_WRAP_H

#include <memory>
#include <map>

#include "bridge.h"

namespace OHOS::Plugin::Bridge {
class BridgeWrap {
public:
    BridgeWrap() = default;
    ~BridgeWrap() = default;

    class Data {
    public:
        Data() = default;
        ~Data() = default;

        int32_t ref_ = 0;
        Bridge* bridge_ = nullptr;
    };
    static Bridge* CreateBridge(const std::string& bridgeName, const CodecType& codecType);
    static void DeleteBridge(const std::string& bridgeName, int32_t instanceId);
    static std::string GetBridgeNameWithID(const std::string& bridgeName, int32_t instanceId);
private:
    static std::map<std::string, std::shared_ptr<Data>> bridgeList_;
    static std::mutex bridgeListLock_;

    static std::shared_ptr<Data> findData(const std::string& bridgeNameWithID);
    static Bridge* BuildBridge(
        const std::string& bridgeName, const CodecType& codecType, const std::string& dataKey, int32_t instanceId);
    static Bridge* CopyBridge(std::shared_ptr<Data> data);
};
} // namespace OHOS::Plugin::Bridge
#endif