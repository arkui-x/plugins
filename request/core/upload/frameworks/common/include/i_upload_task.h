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

#ifndef PLUGINS_REQUEST_UPLOAD_IUPLOAD_TASK
#define PLUGINS_REQUEST_UPLOAD_IUPLOAD_TASK

#include <mutex>
#include "upload_common.h"
#include "upload_config.h"
#include "i_complete_callback.h"
#include "i_fail_callback.h"
#include "i_progress_callback.h"
#include "i_header_receive_callback.h"


namespace OHOS::Plugin::Request::Upload {
enum UploadTaskState {
    STATE_INIT,
    STATE_RUNNING,
    STATE_SUCCESS,
    STATE_FAILURE,
};

class CurlAdpCallback {
public:
    virtual void OnProgress(curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) = 0;
    virtual void OnFail() = 0;
    virtual void OnComplete() = 0;
    virtual void OnHeaderReceive(const std::string &header) = 0;
    virtual uint64_t GetTaskId() = 0;
};

class IUploadTaskRelease {
public:
    virtual void OnUploadTaskRelease(void *arg) = 0;
};

class IUploadTask {
public:
    explicit IUploadTask(std::shared_ptr<UploadConfig> &uploadConfig);
    virtual ~IUploadTask();
    static std::shared_ptr<IUploadTask> CreateUploadTask(std::shared_ptr<UploadConfig> &uploadConfig);
    static bool CheckFilesValid(const std::vector<File> &files);

public:
    virtual void On(Type type, void *callback);
    virtual void Off(Type type, void *callback);
    virtual void SetUploadTaskRelease(IUploadTaskRelease *task, void *arg);
    virtual bool IsRunning();
    virtual bool Remove() = 0;
    virtual void ExecuteTask() = 0;
    static std::string GetCodeMessage(uint32_t code);
    void SetCallback(Type type, void *callback);
    void UploadTaskRelease();

public:
    uint64_t taskId_ = 0;

protected:
    std::shared_ptr<UploadConfig> uploadConfig_;
    IProgressCallback *progressCallback_ = nullptr;
    IFailCallback *failCallback_ = nullptr;
    ICompleteCallback *completeCallback_ = nullptr;
    IHeaderReceiveCallback* headerReceiveCallback_ = nullptr;
    int64_t uploadedSize_ = 0;
    int64_t totalSize_ = 0;
    std::vector<TaskState> taskStates_;
    UploadTaskState state_ = STATE_INIT;
    std::recursive_mutex mutex_;
    IUploadTaskRelease *uploadTaskRelease_ = nullptr;
    void *uploadTaskData_ = nullptr;
    std::vector<std::string> headerArray_;
};
} // namespace OHOS::Plugin::Request::Upload
#endif // PLUGINS_REQUEST_UPLOAD_IUPLOAD_TASK