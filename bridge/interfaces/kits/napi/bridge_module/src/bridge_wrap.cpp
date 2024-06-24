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

#include "bridge_wrap.h"

#include "bridge_manager.h"
#include "plugins/interfaces/native/inner_api/plugin_utils_inner.h"

namespace OHOS::Plugin::Bridge {
static constexpr const char* BRIDGE_NAME_ID_SEP = "$";

BridgeWrap& BridgeWrap::GetInstance()
{
    static BridgeWrap instance;
    return instance;
}

std::shared_ptr<BridgeWrap::Data> BridgeWrap::findData(const std::string& bridgeNameWithId)
{
    auto data = bridgeList_->find(bridgeNameWithId);
    if (data == bridgeList_->end()) {
        return nullptr;
    }

    return data->second;
}

Bridge* BridgeWrap::BuildBridge(
    const std::string& bridgeName, const CodecType& codecType, const std::string& dataKey, int32_t instanceId)
{
    auto bridge = new (std::nothrow) Bridge(bridgeName, instanceId, codecType);
    if (bridge == nullptr) {
        return nullptr;
    }

    std::shared_ptr<Data> data = std::make_shared<BridgeWrap::Data>();
    data->ref_++;
    data->bridge_ = bridge;
    (*bridgeList_)[dataKey] = data;
    return bridge;
}

Bridge* BridgeWrap::CopyBridge(std::shared_ptr<BridgeWrap::Data> data)
{
    if (data) {
        data->ref_++;
        return data->bridge_;
    }
    return nullptr;
}

Bridge* BridgeWrap::CreateBridge(const std::string& bridgeName, const CodecType& codecType)
{
    std::lock_guard<std::mutex> lock(*bridgeListLock_);
    int32_t instanceId = Ace::Platform::BridgeManager::GetCurrentInstanceId();
    std::string key(GetBridgeNameWithID(bridgeName, instanceId));
    auto data = findData(key);
    if (data == nullptr) {
        LOGI("BuildBridge instanceId is %{public}d bridgeName is %{public}s,", instanceId, bridgeName.c_str());
        return BuildBridge(bridgeName, codecType, key, instanceId);
    }
    LOGI("CopyBridge instanceId is %{public}d bridgeName is %{public}s,", instanceId, bridgeName.c_str());
    return CopyBridge(data);
}

void BridgeWrap::DeleteBridge(const std::string& bridgeName, int32_t instanceId)
{
    std::lock_guard<std::mutex> lock(*bridgeListLock_);
    LOGI("DeleteBridge instanceId is %{public}d bridgeName is %{public}s,", instanceId, bridgeName.c_str());
    std::string bridgeNameWithId = GetBridgeNameWithID(bridgeName, instanceId);
    auto data = findData(bridgeNameWithId);
    if (data == nullptr) {
        return;
    }

    data->ref_--;
    if (data->ref_ == 0) {
        if (data->bridge_) {
            data->bridge_->UnRegisterBridge();
            delete data->bridge_;
            data->bridge_ = nullptr;
        }

        auto it = bridgeList_->find(bridgeNameWithId);
        if (it != bridgeList_->end()) {
            bridgeList_->erase(it);
        }
    }
}

std::string BridgeWrap::GetBridgeNameWithID(const std::string& bridgeName, int32_t instanceId)
{
    std::string bridgeNameWithId(bridgeName);
    bridgeNameWithId.append(BRIDGE_NAME_ID_SEP);
    bridgeNameWithId.append(std::to_string(instanceId));
    return bridgeNameWithId;
}
} // namespace OHOS::Plugin::Bridge
