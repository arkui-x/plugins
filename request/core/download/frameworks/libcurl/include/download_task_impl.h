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

#ifndef PLUGINS_REQUEST_LIBCURL_DOWNLOAD_TASK_IMPL_H
#define PLUGINS_REQUEST_LIBCURL_DOWNLOAD_TASK_IMPL_H

#include <stddef.h>
#include <stdint.h>
#include <mutex>
#include <string>
#include <vector>
#include <iosfwd>
#include <thread>
#include "curl/curl.h"
#include "download_config.h"
#include "i_download_task.h"


namespace OHOS::Plugin::Request::Download {

class DownloadTaskImpl : public IDownloadTask {
public:
    DownloadTaskImpl(uint32_t taskId, const DownloadConfig &config);
    virtual ~DownloadTaskImpl(void);
    void ExecuteTask() override;
    bool Remove() override;
    bool Run();
    void GetRunResult(DownloadStatus &status, ErrorCode &code, PausedReason &reason);
    void SetRetryTime(uint32_t retryTime);
    void SetNetworkStatus(bool isOnline);
    bool IsSatisfiedConfiguration();

private:
    bool ExecHttp();
    CURLcode CurlPerformFileTransfer(CURL *handle) const;
    bool SetFileSizeOption(CURL *curl, struct curl_slist *requestHeader);
    bool SetOption(CURL *curl, struct curl_slist *requestHeader);
    struct curl_slist *MakeHeaders(const std::vector<std::string> &vec);
    void SetResumeFromLarge(CURL *curl, long long pos);
    bool GetFileSize(uint32_t &result);
    void HandleResponseCode(CURLcode code, int32_t httpCode);
    void HandleCleanup(DownloadStatus status);
    static size_t WriteCallback(void *buffer, size_t size, size_t num, void *param);
    static size_t HeaderCallback(void *buffer, size_t size, size_t num, void *param);
    static int ProgressCallback(void *param, double dltotal, double dlnow, double ultotal, double ulnow);
    void ForceStopRunning();
    bool HandleFileError();
    bool SetCertificationOption(CURL *curl);
    bool IsHttpsURL();
    bool SetHttpCertificationOption(CURL *curl);
    bool SetHttpsCertificationOption(CURL *curl);
    std::string ReadCertification();
    void PublishNotification(bool background, uint32_t percent);
    void PublishNotification(bool background, uint32_t prevSize, uint32_t downloadSize, uint32_t totalSize);
    std::time_t GetCurTimestamp();
    uint32_t ProgressNotification(uint32_t prevSize, uint32_t downloadSize, uint32_t totalSize);

private:
    std::string mimeType_;
    uint32_t totalSize_;
    uint32_t downloadSize_;
    bool isPartialMode_;
    bool forceStop_;
    bool isRemoved_;
    uint32_t retryTime_;
    bool hasFileSize_;
    uint32_t prevSize_;
    static constexpr uint32_t HUNDRED_PERCENT = 100;
    static constexpr uint32_t TEN_PERCENT_THRESHOLD = 10;
    static constexpr uint32_t NOTIFICATION_FREQUENCY = 200;
    std::time_t lastTimestamp_ = 0;
    std::unique_ptr<std::thread> thread_ = {};
    std::thread::native_handle_type thread_handle_ = {};
};
} // namespace OHOS::Plugin::Request::Download
#endif // PLUGINS_REQUEST_LIBCURL_DOWNLOAD_TASK_IMPL_H