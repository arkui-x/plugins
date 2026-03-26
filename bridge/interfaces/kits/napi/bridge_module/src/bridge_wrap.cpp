/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

BridgeWrap& BridgeWrap::GetInstance()
{
    static BridgeWrap instance;
    return instance;
}

std::shared_ptr<BridgeWrap::Data> BridgeWrap::findData(const std::string& bridgeName)
{
    auto data = bridgeList_->find(bridgeName);
    if (data == bridgeList_->end()) {
        return nullptr;
    }
    return data->second;
}

std::shared_ptr<Bridge> BridgeWrap::BuildBridge(const std::string& bridgeName, const CodecType& codecType)
{
    auto bridge = std::make_shared<Bridge>(bridgeName, codecType);
    if (bridge == nullptr) {
        return nullptr;
    }

    std::shared_ptr<Data> data = std::make_shared<BridgeWrap::Data>();
    data->ref_++;
    data->bridge_ = bridge;
    (*bridgeList_)[bridgeName] = data;
    return bridge;
}

std::shared_ptr<Bridge> BridgeWrap::CopyBridge(std::shared_ptr<BridgeWrap::Data> data, const CodecType& codecType)
{
    if (data) {
        data->ref_++;
        if (data->bridge_) {
            data->bridge_->SetCodecType(codecType);
        }
        return data->bridge_;
    }
    return nullptr;
}

std::shared_ptr<Bridge> BridgeWrap::CreateBridge(const std::string& bridgeName, const CodecType& codecType)
{
    std::lock_guard<std::mutex> lock(*bridgeListLock_);
    auto data = findData(bridgeName);
    if (data == nullptr || data->bridge_ == nullptr) {
        LOGI("BuildBridge bridgeName is %{public}s,", bridgeName.c_str());
        return BuildBridge(bridgeName, codecType);
    }
    LOGI("CopyBridge bridgeName is %{public}s,", bridgeName.c_str());
    return CopyBridge(data, codecType);
}

std::shared_ptr<Bridge> BridgeWrap::GetBridge(const std::string& bridgeName)
{
    std::lock_guard<std::mutex> lock(*bridgeListLock_);
    auto data = findData(bridgeName);
    if (data == nullptr) {
        LOGE("GetBridge: The bridge with name %{public}s does not exist.", bridgeName.c_str());
        return nullptr;
    }
    return data->bridge_;
}

void BridgeWrap::DeleteBridge(const std::string& bridgeName)
{
    std::lock_guard<std::mutex> lock(*bridgeListLock_);
    auto data = findData(bridgeName);
    if (data == nullptr) {
        return;
    }
    LOGI("DeleteBridge bridgeName is %{public}s,", bridgeName.c_str());
    data->ref_--;
    if (data->ref_ == 0) {
        if (data->bridge_) {
            data->bridge_->UnRegisterBridge();
            data->bridge_.reset();
        }
        auto it = bridgeList_->find(bridgeName);
        if (it != bridgeList_->end()) {
            bridgeList_->erase(it);
        }
    }
}
} // namespace OHOS::Plugin::Bridge
