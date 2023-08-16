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

#include "android_download_task_impl.h"

#include <jni.h>

#include "log.h"
#include "inner_api/plugin_utils_inner.h"
#include "java/jni/download_manager_jni.h"

namespace OHOS::Plugin::Request::Download {
IDownloadTask* IDownloadTask::CreateDownloadTask(uint32_t taskId, const DownloadConfig &config)
{
    return new (std::nothrow) AndroidDownloadTaskImpl(taskId, config);
}

bool IDownloadTask::CheckPathValid(const std::string &path)
{
    std::string parentPath = path.substr(0, path.rfind('/'));
    return AndroidDownloadAdp::IsDirectory(parentPath);
}

AndroidDownloadTaskImpl::AndroidDownloadTaskImpl(uint32_t taskId, const DownloadConfig &config)
    : IDownloadTask(taskId, config)
{
    DOWNLOAD_HILOGD("start to execute AndroidDownloadTaskImpl");
}

AndroidDownloadTaskImpl::~AndroidDownloadTaskImpl()
{
    DOWNLOAD_HILOGI("Destructed download service task [%{public}d]", GetId());
    AndroidDownloadAdp_ = nullptr;
}

void AndroidDownloadTaskImpl::ExecuteTask()
{
    DOWNLOAD_HILOGI("start to execute ExecuteTask.");
    AndroidDownloadAdp_ = AndroidDownloadAdp::Instance();
    AndroidDownloadAdp_->GetNetworkType(reinterpret_cast<void *>(this));
    if (networkType_ == NETWORK_INVALID) {
        SetStatus(SESSION_FAILED, ERROR_OFFLINE, PAUSED_UNKNOWN); // off line
        SetTaskReturned();
        return;
    }

    if (config_.GetNetworkType() == NETWORK_INVALID ||
       (config_.GetNetworkType() & networkType_) != NETWORK_INVALID) {
        if (!config_.IsMetered() && networkType_ == NETWORK_MOBILE) {
            DOWNLOAD_HILOGD("unsupported meter network, enableMetered: %{public}d, real: %{public}d",
                config_.IsMetered(), networkType_);
            SetStatus(SESSION_FAILED, ERROR_UNSUPPORTED_NETWORK_TYPE, PAUSED_UNKNOWN); // unsupported network type
            SetTaskReturned();
            return;
        }

        auto filePath = config_.GetFilePath();
        if (filePath.find("/data") == 0) { // 沙箱路径
            sendBoxPath_ = filePath;

            // 获取absPath
            int pos1 = filePath.find("com");
            std::string tempAbsPath = filePath.substr(pos1);
            int pos2 = tempAbsPath.rfind('/');
            std::string absPath = "/storage/emulated/0/Android/data/" + tempAbsPath; // 外部存储路径

            if (!CheckPathValid(absPath)) {
                SetStatus(SESSION_FAILED, ERROR_FILE_ERROR, PAUSED_UNKNOWN); // invalid file path
                SetTaskReturned();
                return;
            }
            config_.SetFilePath(absPath);
        } else {
            DOWNLOAD_HILOGI("config_ filePath is not sandbox path");
        }

        AndroidDownloadAdp_->Download(config_, reinterpret_cast<void *>(this));
        SetStatus(SESSION_RUNNING);
    } else {
        DOWNLOAD_HILOGD("unsupported network, required: %{public}d, real: %{public}d",
            config_.GetNetworkType(), networkType_);
        SetStatus(SESSION_FAILED, ERROR_UNSUPPORTED_NETWORK_TYPE, PAUSED_UNKNOWN); // unsupported network type
    }
    SetTaskReturned();
}

bool AndroidDownloadTaskImpl::Remove()
{
    DOWNLOAD_HILOGI("remove download task.");
    isRemoved_ = true;
    AndroidDownloadAdp_->Remove();
    if (eventCb_ != nullptr) {
        eventCb_("remove", GetId(), 0, 0);
    }
    return true;
}

bool AndroidDownloadTaskImpl::Suspend()
{
    DOWNLOAD_HILOGI("suspend download task");
    AndroidDownloadAdp_->Suspend();
    SetStatus(SESSION_PAUSED, ERROR_UNKNOWN, PAUSED_BY_USER);
    return true;
}

bool AndroidDownloadTaskImpl::Restore()
{
    DOWNLOAD_HILOGI("restore download task");
    bool isRestore = AndroidDownloadAdp_->Restore();
    if (isRestore) {
        SetStatus(SESSION_RUNNING);
    } else {
        SetStatus(SESSION_FAILED, ERROR_CANNOT_RESUME, PAUSED_UNKNOWN);
    }
    return isRestore;
}

bool AndroidDownloadTaskImpl::GetTaskInfo(DownloadInfo &info)
{
    std::lock_guard<std::mutex> guard(queryMutex_);
    DOWNLOAD_HILOGD("getTaskInfo, recvSize:%{public}d, totalSize:%{public}d", receivedSize_, totalSize_);
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

bool AndroidDownloadTaskImpl::GetTaskMimeType(std::string &mimeType)
{
    std::lock_guard<std::mutex> guard(queryMutex_);
    mimeType = mimeType_;
    DOWNLOAD_HILOGI("get task mimeType: %{public}s", mimeType.c_str());
    return true;
}

void AndroidDownloadTaskImpl::OnProgress(uint32_t receivedSize, uint32_t totalSize)
{
    std::lock_guard<std::mutex> guard(queryMutex_);
    DOWNLOAD_HILOGI("download progress: task: %d, filepath: %s, receivedSize: %d, totalSize: %d, eventCb_: %p",
        GetId(), config_.GetFilePath().c_str(), receivedSize, totalSize, eventCb_);

    receivedSize_ = receivedSize;
    totalSize_ = totalSize;
    if (eventCb_ != nullptr) {
        DOWNLOAD_HILOGI("OnProgress: eventCb_:%p", eventCb_);
        eventCb_("progress", GetId(), receivedSize, totalSize);
    }
}

void AndroidDownloadTaskImpl::OnComplete()
{
    DOWNLOAD_HILOGI("success to download file to %s", config_.GetFilePath().c_str());
    SetStatus(SESSION_SUCCESS);

    std::string copyFile = "cp " + config_.GetFilePath() + " " + sendBoxPath_;
    if (system(copyFile.c_str()) != 0) {
        DOWNLOAD_HILOGI("%s failed", copyFile.c_str());
        return;
    }
    DOWNLOAD_HILOGI("%s completed", copyFile.c_str());

    std::string removeFile = "rm -rf " + config_.GetFilePath();
    if (system(removeFile.c_str()) != 0) {
        DOWNLOAD_HILOGI("%s failed", removeFile.c_str());
    }
}

void AndroidDownloadTaskImpl::OnFail(ErrorCode errorCode)
{
    std::lock_guard<std::mutex> guard(queryMutex_);
    if (status_ == SESSION_PAUSED || isRemoved_) {
        DOWNLOAD_HILOGI("download task is paused or removed");
        return;
    }
    SetStatus(SESSION_FAILED, errorCode, PAUSED_UNKNOWN);
}

void AndroidDownloadTaskImpl::SetMimeType(const std::string &mimeType)
{
    DOWNLOAD_HILOGI("mimeType:%{public}s", mimeType.c_str());
    std::lock_guard<std::mutex> guard(queryMutex_);
    mimeType_ = mimeType;
}

void AndroidDownloadTaskImpl::SetNetworkType(NetworkType networkType)
{
    DOWNLOAD_HILOGI("get network type from mobile: %{public}d", networkType);
    std::lock_guard<std::mutex> guard(queryMutex_);
    networkType_ = networkType;
}
} // namespace OHOS::Plugin::Request::Download