/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "cache_task.h"
#include "cache_manager.h"

namespace OHOS::Plugin::CacheDownload {

TaskHandle::TaskHandle(const std::string& taskId,
                       const std::string& url,
                       CacheManager& cacheManager)
    : taskId_(taskId), url_(url), state_(TASK_STATE_INIT),
      refCount_(1), cacheManager_(cacheManager)
{
}

void TaskHandle::Cancel()
{
    uint8_t expected = TASK_STATE_RUNNING;
    uint8_t expectedInit = TASK_STATE_INIT;

    // Only cancel if currently running or init
    if (state_.compare_exchange_strong(expected, TASK_STATE_CANCEL) ||
        state_.compare_exchange_strong(expectedInit, TASK_STATE_CANCEL)) {
        if (cancelCallback_) {
            cancelCallback_();
        }
    }
}

void TaskHandle::SetCancelCallback(std::function<void()> callback)
{
    cancelCallback_ = std::move(callback);
}

void TaskHandle::Reset()
{
    uint8_t expected = TASK_STATE_CANCEL;
    // Allow reset from cancel back to init (for retry)
    state_.compare_exchange_strong(expected, TASK_STATE_INIT);
    // Also allow reset from fail
    expected = TASK_STATE_FAIL;
    state_.compare_exchange_strong(expected, TASK_STATE_INIT);
}

const std::string& TaskHandle::TaskId() const
{
    return taskId_;
}

const std::string& TaskHandle::Url() const
{
    return url_;
}

bool TaskHandle::IsFinished() const
{
    uint8_t s = state_.load();
    return s == TASK_STATE_SUCCESS || s == TASK_STATE_FAIL || s == TASK_STATE_CANCEL;
}

TaskState TaskHandle::GetState() const
{
    switch (state_.load()) {
        case TASK_STATE_INIT:    return TaskState::INIT;
        case TASK_STATE_RUNNING: return TaskState::RUNNING;
        case TASK_STATE_SUCCESS: return TaskState::SUCCESS;
        case TASK_STATE_FAIL:    return TaskState::FAIL;
        case TASK_STATE_CANCEL:  return TaskState::CANCEL;
        default:                return TaskState::INIT;
    }
}

uint8_t TaskHandle::StateValue() const
{
    return state_.load();
}

void TaskHandle::AddRef()
{
    refCount_.fetch_add(1);
}

bool TaskHandle::ReleaseRef()
{
    return refCount_.fetch_sub(1) == 1;
}

int32_t TaskHandle::RefCount() const
{
    return refCount_.load();
}

} // namespace OHOS::Plugin::CacheDownload
