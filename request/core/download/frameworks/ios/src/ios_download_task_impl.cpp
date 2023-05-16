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

#include "log.h"
#include "ios_download_task_impl.h"

namespace OHOS::Plugin::Request::Download {
IDownloadTask* IDownloadTask::CreateDownloadTask(uint32_t taskId, const DownloadConfig &config)
{
    return new (std::nothrow) IosDownloadTaskImpl(taskId, config);
}

bool IDownloadTask::CheckPathValid(const std::string &path)
{
    std::string parentPath = path.substr(0, path.rfind('/'));
    return IosDownloadAdp::IsDirectory(parentPath);
}

IosDownloadTaskImpl::IosDownloadTaskImpl(uint32_t taskId, const DownloadConfig &config)
    : IDownloadTask(taskId, config)
{
    DOWNLOAD_HILOGD("IosDownloadTaskImpl allocated");
    IosNetMonitor::SharedInstance()->AddObserver(this);
}

IosDownloadTaskImpl::~IosDownloadTaskImpl()
{
    DOWNLOAD_HILOGD("IosDownloadTaskImpl freed");
    IosNetMonitor::SharedInstance()->RemoveObserver(this);
    eventCb_ = nullptr;
    thread_ = nullptr;
    if (downloadAdp_ == nullptr) {
        return;
    }
    downloadAdp_->Remove();
    while (!downloadAdp_->IsCompleted()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 100: 100ms
    }
    downloadAdp_ = nullptr;
}

// IDownloadTask
void IosDownloadTaskImpl::ExecuteTask()
{
    DOWNLOAD_HILOGD("execute download task");
    thread_ = std::make_unique<std::thread>(&IosDownloadTaskImpl::RunTask, this);
    if (thread_ == nullptr) {
        DOWNLOAD_HILOGE("create download thread failed, task:[%{public}d]", GetId());
        SetStatus(SESSION_FAILED, ERROR_UNKNOWN, PAUSED_UNKNOWN);
        return;
    }
    thread_->detach();
}

void IosDownloadTaskImpl::RunTask(IosDownloadTaskImpl *task)
{
    if (task != nullptr) {
        task->RunTaskImpl();
    }
}

void IosDownloadTaskImpl::RunTaskImpl()
{
    if (networkType_ == NETWORK_INVALID) {
        SetStatus(SESSION_FAILED, ERROR_OFFLINE, PAUSED_UNKNOWN); // off line
        return;
    }

    if (config_.GetNetworkType() == NETWORK_INVALID ||  // network type not set
        (config_.GetNetworkType() & networkType_) != NETWORK_INVALID) {     // allowed, NETWORK_MOBILE、NETWORK_WIFI
        if (!config_.IsMetered() && networkType_ == NETWORK_MOBILE) {
            DOWNLOAD_HILOGD("unsupported meter network, enableMetered:%{public}d, real:%{public}d",
                config_.IsMetered(), networkType_);
            SetStatus(SESSION_FAILED, ERROR_UNSUPPORTED_NETWORK_TYPE, PAUSED_UNKNOWN); // unsupported network type
            return;
        }
        downloadAdp_ = IosDownloadAdp::Instance();
        downloadAdp_->Download(config_, this);
        SetStatus(SESSION_RUNNING);
    } else {
        DOWNLOAD_HILOGD("unsupported network, required:%{public}d, real:%{public}d",
            config_.GetNetworkType(), networkType_);
        SetStatus(SESSION_FAILED, ERROR_UNSUPPORTED_NETWORK_TYPE, PAUSED_UNKNOWN); // unsupported network type
    }
}

bool IosDownloadTaskImpl::Remove()
{
    DOWNLOAD_HILOGD("remove download task");
    std::lock_guard<std::mutex> guard(queryMutex_);
    isRemoved_ = true;
    downloadAdp_->Remove();
    if (eventCb_ != nullptr) {
        eventCb_("remove", GetId(), 0, 0);
    }
    return true;
}

bool IosDownloadTaskImpl::Suspend()
{
    DOWNLOAD_HILOGD("suspend download task");
    bool res = downloadAdp_->Suspend(false);
    SetStatus(SESSION_PAUSED, ERROR_UNKNOWN, PAUSED_BY_USER);
    return res;
}

bool IosDownloadTaskImpl::Restore()
{
    DOWNLOAD_HILOGD("restore download task");
    bool res = downloadAdp_->Restore(this);
    if (res) {
        SetStatus(SESSION_RUNNING);
    } else {
        SetStatus(SESSION_FAILED, ERROR_CANNOT_RESUME, PAUSED_UNKNOWN);
    }
    return res;
}

bool IosDownloadTaskImpl::GetTaskInfo(DownloadInfo &info)
{
    std::lock_guard<std::mutex> guard(queryMutex_);
    DOWNLOAD_HILOGD("getTaskInfo, recvSize: %{public}d, totalSize: %{public}d", receivedSize_, totalSize_);
    info.SetDescription(config_.GetDescription());
    info.SetDownloadedBytes(receivedSize_);
    info.SetDownloadId(taskId_);
    info.SetFailedReason(code_);
    std::string fileName = config_.GetFilePath().substr(config_.GetFilePath().rfind('/') + 1);
    std::string filePath = config_.GetFilePath().substr(0, config_.GetFilePath().rfind('/'));
    info.SetFileName(fileName);
    info.SetFilePath(filePath);
    info.SetPausedReason(reason_);
    info.SetStatus(status_);
    info.SetTargetURI(config_.GetUrl());
    info.SetDownloadTitle(config_.GetTitle());
    info.SetDownloadTotalBytes(totalSize_);
    return true;
}

bool IosDownloadTaskImpl::GetTaskMimeType(std::string &mimeType)
{
    std::lock_guard<std::mutex> guard(queryMutex_);
    mimeType = mimeType_;
    DOWNLOAD_HILOGD("mimeType: %{public}s", mimeType.c_str());
    return true;
}

// IosDownloadAdpCallback
void IosDownloadTaskImpl::OnProgress(uint32_t receivedSize, uint32_t totalSize)
{
    std::lock_guard<std::mutex> guard(queryMutex_);
    receivedSize_ = receivedSize;
    totalSize_ = totalSize;
    if (eventCb_ != nullptr) {
        eventCb_("progress", GetId(), receivedSize, totalSize);
    }
}

void IosDownloadTaskImpl::OnComplete()
{
    SetStatus(SESSION_SUCCESS);
}

void IosDownloadTaskImpl::OnFail(ErrorCode errorCode)
{
    std::lock_guard<std::mutex> guard(queryMutex_);
    if ((isOnline_ && status_ == SESSION_PAUSED) || isRemoved_) {
        return;
    }
    SetStatus(SESSION_FAILED, errorCode, PAUSED_UNKNOWN);
}

void IosDownloadTaskImpl::SetMimeType(const std::string &mimeType)
{
    std::lock_guard<std::mutex> guard(queryMutex_);
    DOWNLOAD_HILOGD("mimeType: %{public}s", mimeType.c_str());
    mimeType_ = mimeType;
}

// IosNetMonitorObserver
void IosDownloadTaskImpl::NetworkStatusChanged(NetworkType netType)
{
    if (netType == NETWORK_WIFI) {
        ReachableViaWiFi();
    } else if (netType == NETWORK_MOBILE) {
        ReachableViaWWAN();
    } else if (netType == NETWORK_INVALID) {
        NotReachable();
    }
}

void IosDownloadTaskImpl::ReachableViaWiFi()
{
    DOWNLOAD_HILOGD("NetStatus: WiFi");
    isOnline_ = true;
    networkType_ = NETWORK_WIFI;
    DOWNLOAD_HILOGD("ReachableViaWiFi isSuspendByNetwork_: %{public}d", isSuspendByNetwork_);
    if (isSuspendByNetwork_ && downloadAdp_ != nullptr) {
        DOWNLOAD_HILOGD("ReachableViaWiFi reset SuspendByNetwork to false");
        isSuspendByNetwork_ = false;
        downloadAdp_->Restore(this);
    }
}

void IosDownloadTaskImpl::ReachableViaWWAN()
{
    DOWNLOAD_HILOGD("NetStatus: WWAN");
    isOnline_ = true;
    networkType_ = NETWORK_MOBILE;
    DOWNLOAD_HILOGD("ReachableViaWWAN isSuspendByNetwork_: %{public}d", isSuspendByNetwork_);
    if (isSuspendByNetwork_ && downloadAdp_ != nullptr) {
        DOWNLOAD_HILOGD("ReachableViaWWAN reset SuspendByNetwork to false");
        isSuspendByNetwork_ = false;
        downloadAdp_->Restore(this);
    }
}

void IosDownloadTaskImpl::NotReachable()
{
    DOWNLOAD_HILOGD("NetStatus: NotReachable");
    isOnline_ = false;
    networkType_ = NETWORK_INVALID;
    DOWNLOAD_HILOGD("NotReachable status: %{public}d", status_);
    if (status_ == SESSION_RUNNING && downloadAdp_ != nullptr) {
        DOWNLOAD_HILOGD("NotReachable set SuspendByNetwork to true");
        isSuspendByNetwork_ = true;
        downloadAdp_->Suspend(true);
        SetStatus(SESSION_PAUSED, ERROR_OFFLINE, PAUSED_WAITING_TO_RETRY);
    }
}
} // namespace OHOS::Plugin::Request::Download