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

#include "i_download_task.h"
#include <vector>
#include "log.h"

namespace OHOS::Plugin::Request::Download {

IDownloadTask::IDownloadTask(uint32_t taskId, const DownloadConfig &config) : taskId_(taskId), config_(config)
{
    DOWNLOAD_HILOGD("alloc download task [%{public}d]", GetId());
    supportEvents_[EVENT_COMPLETE] = true;
    supportEvents_[EVENT_PAUSE] = true;
    supportEvents_[EVENT_REMOVE] = true;
    supportEvents_[EVENT_PROGRESS] = true;
    supportEvents_[EVENT_FAIL] = true;
}

IDownloadTask::~IDownloadTask()
{
    DOWNLOAD_HILOGD("free download task [%{public}d]", GetId());
    std::lock_guard<std::mutex> autoLock(mutex_);
    listenerMap_.clear();
    supportEvents_.clear();
    eventCb_ = nullptr;
}

uint32_t IDownloadTask::GetId() const
{
    return taskId_;
}

void IDownloadTask::InstallCallback(DownloadTaskCallback cb)
{
    eventCb_ = cb;
}

bool IDownloadTask::AddListener(const std::string &type, std::shared_ptr<DownloadNotifyInterface> listener)
{
    std::lock_guard<std::mutex> autoLock(mutex_);
    auto it = listenerMap_.find(type);
    if (it != listenerMap_.end()) {
        DOWNLOAD_HILOGE("replace event listener with %{public}s event", type.c_str());
    }
    listenerMap_[type] = listener;

    // event buf
    auto iter = eventBufMap_.find(type);
    if (iter != eventBufMap_.end()) {
        uint32_t argv1 = std::get<0>(iter->second);
        uint32_t argv2 = std::get<1>(iter->second);
        OnCallBack(type, argv1, argv2);
        eventBufMap_.erase(iter);
    }
    return true;
}

void IDownloadTask::RemoveListener(const std::string &type, std::shared_ptr<DownloadNotifyInterface> listener)
{
    std::lock_guard<std::mutex> autoLock(mutex_);
    auto it = listenerMap_.find(type);
    if (it == listenerMap_.end() || it->second != listener) {
        DOWNLOAD_HILOGE("no event listener");
        return;
    }
    listenerMap_.erase(it);
}

void IDownloadTask::RemoveListener(const std::string &type)
{
    std::lock_guard<std::mutex> autoLock(mutex_);
    auto it = listenerMap_.find(type);
    if (it == listenerMap_.end()) {
        DOWNLOAD_HILOGE("no event listener");
        return;
    }
    listenerMap_.erase(it);
}

void IDownloadTask::OnCallBack(const std::string &type, uint32_t argv1, uint32_t argv2)
{
    std::lock_guard<std::mutex> autoLock(mutex_);
    auto it = listenerMap_.find(type);
    if (it == listenerMap_.end()) {
        DOWNLOAD_HILOGE("no event listener, tmp store to event buf");
        eventBufMap_[type] = std::make_tuple(argv1, argv2);
        return;
    }
    std::vector<uint32_t> params;
    params.push_back(argv1);
    params.push_back(argv2);
    it->second->OnCallBack(params);
}

bool IDownloadTask::IsSupportType(const std::string &type)
{
    return supportEvents_.find(type) != supportEvents_.end();
}

void IDownloadTask::SetStatus(DownloadStatus status, ErrorCode code, PausedReason reason)
{
    auto stateChange = [this](DownloadStatus status, ErrorCode code, PausedReason reason) -> bool {
        std::lock_guard<std::recursive_mutex> autoLock(rmutex_);
        bool isChanged = false;
        if (status != this->status_) {
            this->status_ = status;
            isChanged = true;
        }
        if (code != this->code_) {
            this->code_ = code;
            isChanged = true;
        }
        if (this->reason_ != PAUSED_BY_USER) {
            if (!isOnline_ && reason == PAUSED_WAITING_TO_RETRY) {
                reason = PAUSED_WAITING_FOR_NETWORK;
            }
            if (reason != this->reason_) {
                this->reason_ = reason;
                isChanged = true;
            }
        }

        return isChanged;
    };
    DOWNLOAD_HILOGI("Status [%{public}d], Code [%{public}d], Reason [%{public}d]", status, code, reason);
    if (!stateChange(status, code, reason)) {
        return;
    }
    if (eventCb_ != nullptr) {
        std::lock_guard<std::recursive_mutex> autoLock(rmutex_);
        switch (status_) {
            case SESSION_SUCCESS:
                eventCb_("complete", GetId(), 0, 0);
                break;

            case SESSION_PAUSED:
                eventCb_("pause", GetId(), 0, 0);
                break;

            case SESSION_FAILED:
                eventCb_("fail", GetId(), code_, 0);
                break;

            default:
                break;
        }
    }
}

void IDownloadTask::SetStatus(DownloadStatus status)
{
    auto stateChange = [this](DownloadStatus status) -> bool {
        std::lock_guard<std::recursive_mutex> autoLock(rmutex_);
        if (status == this->status_) {
            DOWNLOAD_HILOGD("ignore same status");
            return false;
        }
        this->status_ = status;
        return true;
    };
    DOWNLOAD_HILOGI("Status [%{public}d]", status);
    if (!stateChange(status)) {
        return;
    }
    if (eventCb_ != nullptr) {
        std::lock_guard<std::recursive_mutex> autoLock(rmutex_);
        switch (status_) {
            case SESSION_SUCCESS:
                eventCb_("complete", GetId(), 0, 0);
                break;

            case SESSION_PAUSED:
                eventCb_("pause", GetId(), 0, 0);
                break;

            case SESSION_FAILED:
                eventCb_("fail", GetId(), code_, 0);
                break;

            default:
                break;
        }
    }
}

void IDownloadTask::SetError(ErrorCode code)
{
    DOWNLOAD_HILOGI("Code [%{public}d]", code);
    std::lock_guard<std::recursive_mutex> autoLock(rmutex_);
    if (code == code_) {
        DOWNLOAD_HILOGD("ignore same error code");
        return;
    }
    code_ = code;
}

void IDownloadTask::SetReason(PausedReason reason)
{
    DOWNLOAD_HILOGI("Reason [%{public}d]", reason);
    std::lock_guard<std::recursive_mutex> autoLock(rmutex_);

    if (reason_ != PAUSED_BY_USER) {
        if (!isOnline_ && reason == PAUSED_WAITING_TO_RETRY) {
            reason = PAUSED_WAITING_FOR_NETWORK;
        }
        if (reason == reason_) {
            DOWNLOAD_HILOGD("ignore same paused reason");
            return;
        }
        reason_ = reason;
    }
}

void IDownloadTask::DumpStatus()
{
    switch (status_) {
        case SESSION_SUCCESS:
            DOWNLOAD_HILOGD("status:    SESSION_SUCCESS");
            break;

        case SESSION_RUNNING:
            DOWNLOAD_HILOGD("status:    SESSION_RUNNING");
            break;

        case SESSION_PENDING:
            DOWNLOAD_HILOGD("status:    SESSION_PENDING");
            break;

        case SESSION_PAUSED:
            DOWNLOAD_HILOGD("status:    SESSION_PAUSED");
            break;

        case SESSION_FAILED:
            DOWNLOAD_HILOGD("status:    SESSION_FAILED");
            break;

        case SESSION_UNKNOWN:
            DOWNLOAD_HILOGD("status:    SESSION_UNKNOWN");
            break;

        default:
            DOWNLOAD_HILOGD("status:    SESSION_UNKNOWN");
            break;
    }
}

void IDownloadTask::DumpErrorCode()
{
    switch (code_) {
        case ERROR_CANNOT_RESUME:
            DOWNLOAD_HILOGD("error code:    ERROR_CANNOT_RESUME");
            break;

        case ERROR_DEVICE_NOT_FOUND:
            DOWNLOAD_HILOGD("error code:    ERROR_DEVICE_NOT_FOUND");
            break;

        case ERROR_INSUFFICIENT_SPACE:
            DOWNLOAD_HILOGD("error code:    ERROR_INSUFFICIENT_SPACE");
            break;

        case ERROR_FILE_ALREADY_EXISTS:
            DOWNLOAD_HILOGD("error code:    ERROR_FILE_ALREADY_EXISTS");
            break;

        case ERROR_FILE_ERROR:
            DOWNLOAD_HILOGD("error code:    ERROR_FILE_ERROR");
            break;

        case ERROR_HTTP_DATA_ERROR:
            DOWNLOAD_HILOGD("error code:    ERROR_HTTP_DATA_ERROR");
            break;

        case ERROR_TOO_MANY_REDIRECTS:
            DOWNLOAD_HILOGD("error code:    ERROR_TOO_MANY_REDIRECTS");
            break;

        case ERROR_UNHANDLED_HTTP_CODE:
            DOWNLOAD_HILOGD("error code:    ERROR_UNHANDLED_HTTP_CODE");
            break;

        case ERROR_UNKNOWN:
            DOWNLOAD_HILOGD("error code:    ERROR_UNKNOWN");
            break;

        default:
            DOWNLOAD_HILOGD("error code:    SESSION_UNKNOWN");
            break;
    }
}

void IDownloadTask::DumpPausedReason()
{
    switch (reason_) {
        case PAUSED_QUEUED_FOR_WIFI:
            DOWNLOAD_HILOGD("paused reason: PAUSED_QUEUED_FOR_WIFI");
            break;

        case PAUSED_WAITING_FOR_NETWORK:
            DOWNLOAD_HILOGD("paused reason: PAUSED_WAITING_FOR_NETWORK");
            break;

        case PAUSED_WAITING_TO_RETRY:
            DOWNLOAD_HILOGD("paused reason: PAUSED_WAITING_TO_RETRY");
            break;

        case PAUSED_BY_USER:
            DOWNLOAD_HILOGD("paused reason: PAUSED_BY_USER");
            break;

        case PAUSED_UNKNOWN:
            DOWNLOAD_HILOGD("paused reason: PAUSED_UNKNOWN");
            break;

        default:
            DOWNLOAD_HILOGD("paused reason: PAUSED_UNKNOWN");
            break;
    }
}

bool IDownloadTask::IsRunning()
{
    return (status_ == SESSION_RUNNING);
}

} // namespace OHOS::Plugin::Request::Download
