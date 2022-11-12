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

#ifndef PLUGINS_REQUEST_UPLOAD_UPLOAD_TASK
#define PLUGINS_REQUEST_UPLOAD_UPLOAD_TASK

#include <cstdio>
#include <pthread.h>
#include <thread>
#include <vector>
#include <curl/curl.h>
#include <curl/easy.h>

#include "curl_adp.h"
#include "i_complete_callback.h"
#include "i_fail_callback.h"
#include "i_progress_callback.h"
#include "i_upload_task.h"
#include "obtain_file.h"
#include "upload_config.h"
#include "upload_common.h"
#include "upload_hilog_wrapper.h"

namespace OHOS::Plugin::Request::Upload {
enum UploadTaskState {
    STATE_INIT,
    STATE_RUNNING,
    STATE_SUCCESS,
    STATE_FAILURE,
};

class UploadTask : public IUploadTask {
public:
    UPLOAD_API UploadTask(std::shared_ptr<UploadConfig> &uploadConfig);
    UPLOAD_API virtual ~UploadTask();
    UPLOAD_API virtual bool Remove();
    UPLOAD_API virtual void On(Type type, void *callback);
    UPLOAD_API virtual void Off(Type type, void *callback);
    UPLOAD_API void ExecuteTask();
    static void Run(void *arg);
    virtual void OnRun();

    UPLOAD_API virtual void SetCallback(Type type, void *callback);
    virtual void OnProgress(curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

    virtual void OnFail();
    virtual void OnComplete();
    std::vector<std::string> StringSplit(const std::string &str, char delim);
    std::time_t GetCurTimestamp();
    bool IsRunning();
    void SetUploadTaskRelease(IUploadTaskRelease *task, void *arg)
    {
        uploadTaskRelease_ = task;
        uploadTaskData_ = arg;
    }
    void UploadTaskRelease();
protected:
    uint32_t InitFileArray();
    void ClearFileArray();
    uint32_t CheckConfig();
    std::string GetCodeMessage(uint32_t code);
    std::vector<TaskState> GetTaskStates();
    IUploadTaskRelease *uploadTaskRelease_ = nullptr;
    void *uploadTaskData_ = nullptr;

private:
    void ReportTaskFault(uint32_t ret) const;
    uint32_t StartUploadFile();

    std::shared_ptr<UploadConfig> uploadConfig_;
    std::unique_ptr<std::thread> thread_;
    static constexpr const char *REQUEST_TASK_FAULT = "REQUEST_TASK_FAULT";
    static constexpr const char *TASKS_TYPE = "TASKS_TYPE";
    static constexpr const char *UPLOAD = "UPLOAD";
    static constexpr const char *TOTAL_FILE_NUM = "TOTAL_FILE_NUM";
    static constexpr const char *FAIL_FILE_NUM = "FAIL_FILE_NUM";
    static constexpr const char *SUCCESS_FILE_NUM = "SUCCESS_FILE_NUM";
    static constexpr const char *ERROR_INFO = "ERROR_INFO";

    IProgressCallback *progressCallback_ = nullptr;
    IFailCallback *failCallback_ = nullptr;
    ICompleteCallback *completeCallback_ = nullptr;

    std::shared_ptr<CUrlAdp> curlAdp_;
    int64_t uploadedSize_;
    int64_t totalSize_;
    std::vector<std::string> headerArray_;
    std::string header_;
    std::vector<FileData> fileDatas_;
    std::vector<TaskState> taskStates_;
    UploadTaskState state_;
    std::recursive_mutex mutex_;
    std::thread::native_handle_type thread_handle_;
    static constexpr int USLEEP_INTERVEL_BEFOR_RUN = 50 * 1000;
    std::time_t lastTimestamp_ = 0;
    static constexpr uint32_t NOTIFICATION_FREQUENCY = 200;
};
} // end of  OHOS::Plugin::Request::Upload
#endif