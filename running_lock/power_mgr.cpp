/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "power_mgr.h"

#include "log.h"

namespace OHOS::Plugin {
namespace {
constexpr int MAX_NAME_LEN = 256;
}

std::mutex PowerMgr::mutex_;
std::shared_ptr<PowerMgr> PowerMgr::instance_ = nullptr;
std::shared_ptr<PowerMgr> PowerMgr::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock_l(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<PowerMgr>();
        }
    }
    return instance_;
}

std::shared_ptr<RunningLock> PowerMgr::CreateRunningLock(const std::string &name, RunningLockType type)
{
    uint32_t nameLen = (name.size() > MAX_NAME_LEN) ? MAX_NAME_LEN : name.size();
    std::shared_ptr<RunningLock> runningLock = RunningLock::Create(name.substr(0, nameLen), type);
    if (runningLock == nullptr) {
        LOGE("Failed to create RunningLock record");
        return nullptr;
    }
    LOGI("name: %{public}s, type = %{public}d", name.c_str(), type);
    return runningLock;
}
}  // namespace OHOS::Plugin
