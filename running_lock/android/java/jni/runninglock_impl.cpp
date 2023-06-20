/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "plugins/running_lock/android/java/jni/runninglock_impl.h"

#include "inner_api/plugin_utils_inner.h"
#include "inner_api/plugin_utils_napi.h"
#include "log.h"
#include "plugin_utils.h"
#include "plugins/running_lock/android/java/jni/runninglock_jni.h"

namespace {
constexpr int WAIT_TIME = 2000;
}  // namespace

namespace OHOS::Plugin {
std::shared_ptr<RunningLock> RunningLock::Create(const std::string &name, RunningLockType type)
{
    return std::make_shared<RunningLockImpl>(name, type);
}

void RunningLockImpl::Init(RunningLockAsyncCallbackInfo *ptr)
{
    std::string name = name_;
    RunningLockType type = type_;
    LOGI("RunningLockImpl::Init name[%s], type[%d] ptr->env[%p] ptr->callbackRef[%p]",
        name_.c_str(), type_, ptr->env, ptr->callbackRef);
    PluginUtilsInner::RunTaskOnPlatform([name, type, ptr]() { RunningLockJni::Init(name, type, ptr); });
}

RunningLockImpl::RunningLockImpl(const std::string &name, RunningLockType type)
{
    name_ = name;
    type_ = type;
    LOGI("RunningLockImpl created! name[%s], type[%d]", name_.c_str(), type_);
}

bool RunningLockImpl::IsUsed()
{
    bool isused = false;
    LOGI("RunningLockImpl IsUsed called");
    std::chrono::milliseconds span(WAIT_TIME);
    PluginUtilsInner::RunSyncTaskOnLocal([&isused]() mutable { isused = RunningLockJni::IsUsed(); }, span);
    return isused;
}

void RunningLockImpl::Lock(uint32_t timeOutMs)
{
    LOGI("RunningLockImpl Lock called");
    PluginUtilsInner::RunTaskOnPlatform([timeOutMs]() { RunningLockJni::Lock(timeOutMs); });
}

void RunningLockImpl::UnLock()
{
    LOGI("RunningLockImpl UnLock called");
    PluginUtilsInner::RunTaskOnPlatform([]() { RunningLockJni::UnLock(); });
}
}  // namespace OHOS::Plugin
