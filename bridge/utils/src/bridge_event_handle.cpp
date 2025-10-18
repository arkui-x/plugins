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

#include "bridge_event_handle.h"

#include "log.h"

namespace OHOS::Plugin::Bridge {
std::shared_ptr<BridgeEventHandle> BridgeEventHandle::instance_ = nullptr;
std::mutex BridgeEventHandle::mutex_;
BridgeEventHandle::BridgeEventHandle()
{
    auto eventRunner = AppExecFwk::EventRunner::Current();
    mainEventHandler_ = std::make_shared<AppExecFwk::EventHandler>(eventRunner);
    auto bridgeEventRunner = AppExecFwk::EventRunner::Create("bridgeEventRunner");
    bridgeEventHandler_ = std::make_shared<AppExecFwk::EventHandler>(bridgeEventRunner);
}

BridgeEventHandle::~BridgeEventHandle() {}

std::shared_ptr<BridgeEventHandle> BridgeEventHandle::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<BridgeEventHandle>();
        }
    }

    return instance_;
}

void BridgeEventHandle::RunTaskOnBridgeThread(const Task& task)
{
    if (task == nullptr) {
        return;
    }
    if (bridgeEventHandler_ != nullptr) {
        bridgeEventHandler_->PostTask(task);
    } else {
        LOGE("BridgeEventHandle bridgeEventHandler is nullptr");
    }
}

void BridgeEventHandle::RunTaskOnMainThread(const Task& task)
{
    if (task == nullptr) {
        return;
    }
    if (mainEventHandler_ != nullptr) {
        mainEventHandler_->PostTask(task);
    } else {
        LOGE("BridgeEventHandle mainEventHandler is nullptr");
    }
}
}