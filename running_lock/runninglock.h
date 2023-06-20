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

#ifndef PLUGINS_RUNNINGLOCK_RUNNINGLOCK_H
#define PLUGINS_RUNNINGLOCK_RUNNINGLOCK_H

#include <memory>
#include <string>

#include "inner_api/plugin_utils_napi.h"
#include "plugin_utils.h"

namespace OHOS::Plugin {
enum class RunningLockType : uint32_t {
    /**
     * RunningLock type: used to keep screen on.
     */
    RUNNINGLOCK_SCREEN,
    /**
     * RunningLock type: used to keep cpu running.
     */
    RUNNINGLOCK_BACKGROUND,
    /**
     * RunningLock type: used to keep the screen on/off when the proximity sensor is active.
     */
    RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL,

    RUNNINGLOCK_BUTT
};

struct RunningLockAsyncCallbackInfo;

class RunningLock {
public:
    RunningLock() = default;
    virtual ~RunningLock() = default;
    static std::shared_ptr<RunningLock> Create(const std::string &name, RunningLockType type);
    virtual void Init(RunningLockAsyncCallbackInfo *ptr) = 0;
    virtual bool IsUsed() = 0;
    /**
     * Acquires a runninglock.
     * The parameter of last called will replace that of the previous called, and the effective parameter will
     * be that of the last called. Eg : If call Lock(n) first then Lock(), a lasting lock will be hold, and
     * need UnLock() to be called to release it. If else call Lock() first and then Lock(n), the parameter 'n'
     * takes effect and it will be released in n ms automatiocly.
     * @param timeOutMs timeOutMs this runninglock will be released in timeOutMs milliseconds. If it is called without
     *                  parameter or parameter is 0 a lasting runninglock will be hold.
     */
    virtual void Lock(uint32_t timeOutMs = 0) = 0;

    /**
     * Release the runninglock, no matter how many times Lock() was called.
     * The release can be done by calling UnLock() only once.
     */
    virtual void UnLock() = 0;
};

namespace {
constexpr int RESULT_SIZE = 2;
constexpr int RUNNINGLOCK_NAME_MAX = 512;
constexpr int CREATRUNNINGLOCK_ARGC = 3;
}  // namespace

struct RunningLockAsyncCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_ref callbackRef = nullptr;
    napi_deferred deferred = nullptr;
    char name[RUNNINGLOCK_NAME_MAX] = {0};
    size_t nameLen = 0;
    RunningLockType type = RunningLockType::RUNNINGLOCK_BUTT;
    std::shared_ptr<RunningLock> runningLock;
};
}  // namespace OHOS::Plugin

#endif  // PLUGINS_RUNNINGLOCK_RUNNINGLOCK_H
