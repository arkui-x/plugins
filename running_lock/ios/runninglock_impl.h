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

#ifndef PLUGINS_RUNNINGLOCK_IOS_RUNNINGLOCK_IMPL_H
#define PLUGINS_RUNNINGLOCK_IOS_RUNNINGLOCK_IMPL_H

#include <memory>

#include "plugins/running_lock/runninglock.h"

namespace OHOS::Plugin {
class RunningLockImpl final : public RunningLock {
public:
    RunningLockImpl(const std::string &name, RunningLockType type);
    ~RunningLockImpl() override = default;
    void Init(RunningLockAsyncCallbackInfo *ptr) override;
    bool IsUsed() override;
    void Lock(uint32_t timeOutMs = 0) override;
    void UnLock() override;
};
}  // namespace OHOS::Plugin
#endif  // PLUGINS_RUNNINGLOCK_IOS_RUNNINGLOCK_IMPL_H
