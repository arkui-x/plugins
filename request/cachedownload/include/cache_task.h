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

#ifndef PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_TASK_H
#define PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_TASK_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include "cache_types.h"

namespace OHOS::Plugin::CacheDownload {

// Forward declarations
class CacheManager;

/** Task state constants (compatible with Rust implementation). */
constexpr uint8_t TASK_STATE_INIT    = 0;
constexpr uint8_t TASK_STATE_RUNNING = 1;
constexpr uint8_t TASK_STATE_SUCCESS = 2;
constexpr uint8_t TASK_STATE_FAIL    = 3;
constexpr uint8_t TASK_STATE_CANCEL  = 4;

/**
 * Handle to an active download task.
 *
 * Provides cancel/reset/state-query capabilities.
 * Thread-safe: uses atomic state transitions.
 *
 * Ported from: request_request/.../cache_download/src/download/task.rs → TaskHandle
 */
class TaskHandle {
    // DownloadContext (defined in cache_download_service.cpp) updates the
    // task state during execution.
    friend struct DownloadContext;
public:
    TaskHandle(const std::string& taskId,
               const std::string& url,
               CacheManager& cacheManager);
    ~TaskHandle() = default;

    /** Cancels the download task. Already-cached data is not affected. */
    void Cancel();

    /** Sets a callback to be invoked when Cancel() is called. */
    void SetCancelCallback(std::function<void()> callback);

    /** Clears the cancel callback to break reference cycles (e.g. when the
     *  task reaches a terminal state). */
    void ClearCancelCallback() { cancelCallback_ = nullptr; }

    /** Resets the task for retry. */
    void Reset();

    /** Returns the unique task ID. */
    const std::string& TaskId() const;

    /** Returns the download URL. */
    const std::string& Url() const;

    /** Returns true if the task has finished (success, fail, or cancel). */
    bool IsFinished() const;

    /** Returns the current task state. */
    TaskState GetState() const;

    /** Returns the raw state value (for internal use). */
    uint8_t StateValue() const;

    /** Increments the reference count. */
    void AddRef();

    /** Decrements the reference count. Returns true if count reaches 0. */
    bool ReleaseRef();

    /** Returns the current reference count. */
    int32_t RefCount() const;

private:
    friend class CacheDownloadService;

    std::string taskId_;
    std::string url_;
    std::atomic<uint8_t> state_;
    std::atomic<int32_t> refCount_;
    CacheManager& cacheManager_;
    std::function<void()> cancelCallback_;
};

using TaskHandlePtr = std::shared_ptr<TaskHandle>;

} // namespace OHOS::Plugin::CacheDownload

#endif // PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_TASK_H
