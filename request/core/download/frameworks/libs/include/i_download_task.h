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

#ifndef PLUGINS_REQUEST_DOWNLOAD_TASK_H
#define PLUGINS_REQUEST_DOWNLOAD_TASK_H

#include <map>
#include <mutex>
#include <tuple>
#include <string>
#include "nocopyable.h"
#include "constant.h"
#include "download_config.h"
#include "download_notify_interface.h"
#include "download_info.h"


static constexpr const char *EVENT_COMPLETE = "complete";
static constexpr const char *EVENT_PAUSE = "pause";
static constexpr const char *EVENT_REMOVE = "remove";
static constexpr const char *EVENT_PROGRESS = "progress";
static constexpr const char *EVENT_FAIL = "fail";

namespace OHOS::Plugin::Request::Download {
using DownloadTaskCallback = void(*)(const std::string& type, uint32_t taskId, uint32_t argv1, uint32_t argv2);

enum ParamNumber {
    NO_PARAMETER,
    ONE_PARAMETER,
    TWO_PARAMETER,
};

class IDownloadTask : public NoCopyable {
public:
    explicit IDownloadTask(uint32_t taskId, const DownloadConfig &config);
    virtual ~IDownloadTask();
    static IDownloadTask* CreateDownloadTask(uint32_t taskId, const DownloadConfig &config);
    static bool CheckPathValid(const std::string &path);
    virtual void ExecuteTask() = 0;
    virtual bool Remove() = 0;
    virtual bool Suspend() = 0;
    virtual bool Restore() = 0;
    virtual bool GetTaskInfo(DownloadInfo &info) = 0;
    virtual bool GetTaskMimeType(std::string &mimeType) = 0;

public:
    void InstallCallback(DownloadTaskCallback cb);
    bool IsRunning();
    uint32_t GetId() const;
    bool AddListener(const std::string &type, std::shared_ptr<DownloadNotifyInterface> listener);
    void RemoveListener(const std::string &type, std::shared_ptr<DownloadNotifyInterface> listener);
    void RemoveListener(const std::string &type);
    bool IsSupportType(const std::string &type);
    void OnCallBack(const std::string &type, uint32_t argv1, uint32_t argv2);

protected:
    void SetStatus(DownloadStatus status, ErrorCode code, PausedReason reason);
    void SetStatus(DownloadStatus status);
    void SetError(ErrorCode code);
    void SetReason(PausedReason reason);
    void DumpStatus();
    void DumpErrorCode();
    void DumpPausedReason();
    void SetTaskReturned();
    bool IsTaskReturned();

protected:
    DownloadStatus status_ = SESSION_UNKNOWN;
    ErrorCode code_ = ERROR_UNKNOWN;
    PausedReason reason_ = PAUSED_UNKNOWN;
    int taskId_ = 0;
    DownloadConfig config_;
    std::mutex mutex_;
    DownloadTaskCallback eventCb_ = nullptr;
    std::map<std::string, std::shared_ptr<DownloadNotifyInterface>> listenerMap_;
    std::map<std::string, bool> supportEvents_;
    std::recursive_mutex rmutex_;
    bool isOnline_ = true;
    std::map<std::string, std::tuple<uint32_t, uint32_t>> eventBufMap_;
    bool isTaskReturn_ = false;
};
} // namespace OHOS::Plugin::Request::Download
#endif // PLUGINS_REQUEST_DOWNLOAD_TASK_H