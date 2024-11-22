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

#include "plugins/running_lock/ios/runninglock_impl.h"

#include <memory>

namespace OHOS::Plugin {
std::shared_ptr<RunningLock> RunningLock::Create(const std::string &name, RunningLockType type)
{
    return std::make_shared<RunningLockImpl>(name, type);
}

void RunningLockImpl::Init(RunningLockAsyncCallbackInfo *ptr){}

RunningLockImpl::RunningLockImpl(const std::string &name, RunningLockType type) {}

bool RunningLockImpl::IsUsed()
{
    return true;
}

void RunningLockImpl::Lock(uint32_t timeOutMs) {}

void RunningLockImpl::UnLock() {}
} // namespace OHOS::Plugin
