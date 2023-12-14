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

#ifndef PLUGINS_REQUEST_DOWNLOAD_INFO_H
#define PLUGINS_REQUEST_DOWNLOAD_INFO_H

#include <stdint.h>
#include <string>
#include <iosfwd>
#include "constant.h"

namespace OHOS::Plugin::Request::Download {
class DownloadInfo final {
public:
    DownloadInfo();

    void SetDescription(const std::string &description);

    void SetDownloadedBytes(int64_t downloadedBytes);

    void SetDownloadId(uint32_t downloadId);

    void SetFailedReason(ErrorCode failedReason);

    void SetFileName(const std::string &fileName);

    void SetFilePath(const std::string &filePath);

    void SetPausedReason(PausedReason pausedReason);

    void SetStatus(DownloadStatus status);

    void SetTargetURI(const std::string &targetURI);

    void SetDownloadTitle(const std::string & downloadTitle);

    void SetDownloadTotalBytes(int64_t downloadTotalBytes);

    void SetNetworkType(uint32_t networkType);
    void SetRoaming(bool enableRoaming);
    void SetMetered(bool enableMetered);

    [[nodiscard]] const std::string &GetDescription() const;

    [[nodiscard]] int64_t GetDownloadedBytes() const;

    [[nodiscard]] uint32_t GetDownloadId() const;

    [[nodiscard]] ErrorCode GetFailedReason() const;

    [[nodiscard]] const std::string &GetFileName() const;

    [[nodiscard]] const std::string &GetFilePath() const;

    [[nodiscard]] PausedReason GetPausedReason() const;

    [[nodiscard]] DownloadStatus GetStatus() const;

    [[nodiscard]] const std::string &GetTargetURI() const;

    [[nodiscard]] const std::string &GetDownloadTitle() const;

    [[nodiscard]] int64_t GetDownloadTotalBytes() const;

    [[nodiscard]] uint32_t GetNetworkType() const;
    [[nodiscard]] bool GetMetered() const;
    [[nodiscard]] bool GetRoaming() const;
    [[nodiscard]] std::string GetTaskType() const;
    void Dump();

private:
    std::string description_;
    int64_t downloadedBytes_;
    uint32_t downloadId_;
    ErrorCode failedReason_;
    std::string fileName_;
    std::string filePath_;
    PausedReason pausedReason_;
    DownloadStatus status_;
    std::string targetURI_;
    std::string downloadTitle_;
    std::string taskType_;
    int64_t downloadTotalBytes_;
    bool enableMetered_ {false};
    bool enableRoaming_ {false};
    uint32_t networkType_ {0};
};
} // namespace OHOS::Plugin::Request::Download
#endif /* PLUGINS_REQUEST_DOWNLOAD_INFO_H */