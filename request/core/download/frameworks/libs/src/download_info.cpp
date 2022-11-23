/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "download_info.h"
#include "constant.h"
#include "log.h"

namespace OHOS::Plugin::Request::Download {
DownloadInfo::DownloadInfo()
    : description_(""), downloadedBytes_(0), downloadId_(0), failedReason_(ERROR_UNKNOWN), fileName_(""),
      filePath_(""), pausedReason_(PAUSED_UNKNOWN), status_(SESSION_UNKNOWN), targetURI_(""), downloadTitle_(""),
      taskType_("download"), downloadTotalBytes_(0) {
}

void DownloadInfo::SetDescription(const std::string &description)
{
    description_ = description;
}

void DownloadInfo::SetDownloadedBytes(uint32_t downloadedBytes)
{
    downloadedBytes_ = downloadedBytes;
}

void DownloadInfo::SetDownloadId(uint32_t downloadId)
{
    downloadId_ = downloadId;
}

void DownloadInfo::SetFailedReason(ErrorCode failedReason)
{
    failedReason_ = failedReason;
}

void DownloadInfo::SetFileName(const std::string &fileName)
{
    fileName_ = fileName;
}

void DownloadInfo::SetFilePath(const std::string &filePath)
{
    filePath_ = filePath;
}

void DownloadInfo::SetPausedReason(PausedReason pausedReason)
{
    pausedReason_ = pausedReason;
}

void DownloadInfo::SetStatus(DownloadStatus status)
{
    status_ = status;
}

void DownloadInfo::SetTargetURI(const std::string &targetURI)
{
    targetURI_ = targetURI;
}

void DownloadInfo::SetDownloadTitle(const std::string &downloadTitle)
{
    downloadTitle_ = downloadTitle;
}

void DownloadInfo::SetDownloadTotalBytes(uint32_t downloadTotalBytes)
{
    downloadTotalBytes_ = downloadTotalBytes;
}

void DownloadInfo::SetNetworkType(uint32_t networkType)
{
    networkType_ = networkType;
}

void DownloadInfo::SetMetered(bool enableMetered)
{
    enableMetered_ = enableMetered;
}

void DownloadInfo::SetRoaming(bool enableRoaming)
{
    enableRoaming_ = enableRoaming;
}

const std::string &DownloadInfo::GetDescription() const
{
    return description_;
}

uint32_t DownloadInfo::GetDownloadedBytes() const
{
    return downloadedBytes_;
}

uint32_t DownloadInfo::GetDownloadId() const
{
    return downloadId_;
}

ErrorCode DownloadInfo::GetFailedReason() const
{
    return failedReason_;
}

const std::string &DownloadInfo::GetFileName() const
{
    return fileName_;
}

const std::string &DownloadInfo::GetFilePath() const
{
    return filePath_;
}

PausedReason DownloadInfo::GetPausedReason() const
{
    return pausedReason_;
}

DownloadStatus DownloadInfo::GetStatus() const
{
    return status_;
}

const std::string &DownloadInfo::GetTargetURI() const
{
    return targetURI_;
}

const std::string &DownloadInfo::GetDownloadTitle() const
{
    return downloadTitle_;
}

std::string DownloadInfo::GetTaskType() const
{
    return taskType_;
}

uint32_t DownloadInfo::GetDownloadTotalBytes() const
{
    return downloadTotalBytes_;
}

uint32_t DownloadInfo::GetNetworkType() const
{
    return networkType_;
}

bool DownloadInfo::GetMetered() const
{
    return enableMetered_;
}

bool DownloadInfo::GetRoaming() const
{
    return enableRoaming_;
}

void DownloadInfo::Dump()
{
    DOWNLOAD_HILOGD("description: %{public}s", description_.c_str());
    DOWNLOAD_HILOGD("downloadedBytes: %{public}d", downloadedBytes_);
    DOWNLOAD_HILOGD("downloadId: %{public}d", downloadId_);
    DOWNLOAD_HILOGD("failedReason: %{public}d", failedReason_);
    DOWNLOAD_HILOGD("fileName: %{public}s", fileName_.c_str());
    DOWNLOAD_HILOGD("filePath: %{public}s", filePath_.c_str());
    DOWNLOAD_HILOGD("pausedReason: %{public}d", pausedReason_);
    DOWNLOAD_HILOGD("status: %{public}d", status_);
    DOWNLOAD_HILOGD("targetURI: %{public}s", targetURI_.c_str());
    DOWNLOAD_HILOGD("downloadTitle: %{public}s", downloadTitle_.c_str());
    DOWNLOAD_HILOGD("downloadTotalBytes: %{public}d", downloadTotalBytes_);
}
} // namespace OHOS::Plugin::Request::Download