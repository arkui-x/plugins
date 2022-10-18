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

#include <unistd.h>
#include <thread>

#include "curl/curl.h"
#include "curl/easy.h"
#include "upload_task.h"

namespace OHOS::Plugin::Request::Upload {
UploadTask::UploadTask(std::shared_ptr<UploadConfig> &uploadConfig)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "UploadTask. In.");
    uploadConfig_ = uploadConfig;
    curlAdp_ = nullptr;
    state_ = STATE_INIT;
    uploadedSize_ = 0;
    totalSize_ = 0;
    progressCallback_ = nullptr;
    failCallback_ = nullptr;
    completeCallback_ = nullptr;
}

UploadTask::~UploadTask()
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "~UploadTask. In.");
    std::lock_guard<std::mutex> guard(mutex_);
    SetCallback(TYPE_PROGRESS_CALLBACK, nullptr);
    SetCallback(TYPE_FAIL_CALLBACK, nullptr);
    SetCallback(TYPE_COMPLETE_CALLBACK, nullptr);
    Remove();
}

bool UploadTask::Remove()
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "Remove. In.");
    if (curlAdp_ != nullptr) {
        return curlAdp_->Remove();
    }
    ClearFileArray();
    return true;
}

void UploadTask::On(Type type, void *callback)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "On. In.");
    std::lock_guard<std::mutex> guard(mutex_);
    SetCallback(type, callback);
}

void UploadTask::Off(Type type, void *callback)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "Off. In.");

    std::lock_guard<std::mutex> guard(mutex_);
    if (callback == nullptr) {
        return;
    }

    if (type == TYPE_PROGRESS_CALLBACK && progressCallback_ != nullptr) {
        ((IProgressCallback*)callback)->Progress(uploadedSize_, totalSize_);
    }
    if (type == TYPE_FAIL_CALLBACK && failCallback_ != nullptr) {
        ((IFailCallback*)callback)->Fail(taskStates_);
    }
    if (type == TYPE_COMPLETE_CALLBACK && completeCallback_ != nullptr) {
        ((ICompleteCallback*)callback)->Complete(taskStates_);
    }
    SetCallback(type, nullptr);
}

void UploadTask::SetCallback(Type type, void *callback)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "SetCallback. In.");
    if (type == TYPE_PROGRESS_CALLBACK) {
        progressCallback_ = (IProgressCallback*)callback;
        if (progressCallback_ && uploadedSize_ > 0) {
            progressCallback_->Progress(uploadedSize_, totalSize_);
        }
    } else if (type == TYPE_FAIL_CALLBACK) {
        failCallback_ = (IFailCallback*)callback;
        if (failCallback_ && state_ == STATE_FAILURE) {
            failCallback_->Fail(taskStates_);
        }
    } else if (type == TYPE_COMPLETE_CALLBACK) {
        completeCallback_ = (ICompleteCallback*)callback;
        if (completeCallback_ && state_ == STATE_SUCCESS) {
            completeCallback_->Complete(taskStates_);
        }
    } else {
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "SetCallback. type[%{public}d] not match.", type);
    }
}

void UploadTask::Run(void *arg)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "Run. In.");
    usleep(USLEEP_INTERVEL_BEFOR_RUN);
    ((UploadTask*)arg)->OnRun();
    if (((UploadTask*)arg)->uploadConfig_->protocolVersion == "L5") {
        if (((UploadTask*)arg)->uploadConfig_->fcomplete) {
            ((UploadTask*)arg)->uploadConfig_->fcomplete();
            UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "Complete.");
        }
    }
}

uint32_t UploadTask::InitFileArray()
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "InitFileArray. In.");
    std::lock_guard<std::mutex> guard(mutex_);
    unsigned int fileSize = 0;
    FileData data;
    FILE *file;
    totalSize_ = 0;
    uint32_t initResult = UPLOAD_OK;
    ObtainFile obtainFile;
    uint32_t index = 1;
    for (auto f : uploadConfig_->files) {
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "filename is %{public}s", f.filename.c_str());
        data.result = UPLOAD_ERRORCODE_UPLOAD_FAIL;
        uint32_t ret = obtainFile.GetFile(&file, f.uri, fileSize);
        if (ret != UPLOAD_OK) {
            initResult = data.result;
            data.result = ret;
        }

        data.fp = file;
        std::size_t position = f.uri.find_last_of("/");
        if (position != std::string::npos) {
            data.filename = std::string(f.uri, position + 1);
            data.filename.erase(data.filename.find_last_not_of(" ") + 1);
        }
        data.name = f.name;
        data.type = f.type;
        data.fileIndex = index++;
        data.adp = nullptr;
        data.upsize = 0;
        data.totalsize = 0;
        data.list = nullptr;
        data.headSendFlag = 0;
        data.httpCode = 0;
        
        fileArray_.push_back(data);
        totalSize_ += fileSize;
    }
    lastTimestamp_ = GetCurTimestamp();

    return initResult;
}

uint32_t UploadTask::CheckConfig()
{
    if (uploadConfig_ == nullptr) {
        return UPLOAD_ERRORCODE_CONFIG_ERROR;
    }

    if (uploadConfig_->url.empty()) {
        return UPLOAD_ERRORCODE_CONFIG_ERROR;
    }

    if (uploadConfig_->files.empty()) {
        return UPLOAD_ERRORCODE_CONFIG_ERROR;
    }
    return UPLOAD_OK;
}

uint32_t UploadTask::StartUploadFile()
{
    uint32_t ret = CheckConfig();
    if (ret != UPLOAD_OK) {
        return ret;
    }

    ret = InitFileArray();
    if (ret != UPLOAD_OK) {
        return ret;
    }

    curlAdp_ = std::make_shared<CUrlAdp>(fileArray_, uploadConfig_);
    return curlAdp_->DoUpload(this);
}

std::string UploadTask::GetCodeMessage(uint32_t code)
{
    std::vector<std::pair<UploadErrorCode, std::string>> codeMap = {
        {UPLOAD_OK, "file uploaded successfully"},
        {UPLOAD_ERRORCODE_UNSUPPORT_URI, "file path error"},
        {UPLOAD_ERRORCODE_GET_FILE_ERROR, "failed to get file"},
        {UPLOAD_ERRORCODE_CONFIG_ERROR,  "upload configuration error"},
        {UPLOAD_ERRORCODE_UPLOAD_LIB_ERROR,  "libcurl return error"},
        {UPLOAD_ERRORCODE_UPLOAD_FAIL, "upload failed"},
        {UPLOAD_ERRORCODE_UPLOAD_OUTTIME, "upload timeout"},
    };

    for (const auto &it: codeMap) {
        if (it.first == code) {
            return it.second;
        }
    }
    return "unknown";
}

void UploadTask::OnRun()
{
    std::string traceParam = "url:" + uploadConfig_->url + "file num:" + std::to_string(uploadConfig_->files.size());
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "OnRun. In.");
    state_ = STATE_RUNNING;
    uint32_t ret = StartUploadFile();
    if (ret != UPLOAD_OK) {
        OnFail();
        ReportTaskFault(ret);
    } else {
        OnComplete();
    }

    ClearFileArray();
    totalSize_ = 0;
}

void UploadTask::ReportTaskFault(uint32_t ret) const
{
    uint32_t successCount = 0;
    uint32_t failCount = 0;
    for (auto &vmem : fileArray_) {
        if (vmem.result == UPLOAD_OK) {
            successCount++;
        } else {
            failCount++;
        }
    }
}

std::time_t UploadTask::GetCurTimestamp()
{
    auto tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    return tp.time_since_epoch().count();
}

void UploadTask::OnProgress(curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "OnProgress. In.");
    if (ulnow == uploadedSize_) {
        return;
    }

    std::lock_guard<std::mutex> guard(mutex_);
    uploadedSize_ = ulnow;
    if (uploadedSize_ == totalSize_) {
        state_ = STATE_SUCCESS;
    }

    std::time_t curTimestamp = GetCurTimestamp();
    if ((curTimestamp - lastTimestamp_) >= NOTIFICATION_FREQUENCY || uploadedSize_ == totalSize_) {
        if (progressCallback_) {
            progressCallback_->Progress(uploadedSize_, totalSize_);
        }
        lastTimestamp_ = GetCurTimestamp();
    }
}

std::vector<TaskState> UploadTask::GetTaskStates()
{
    std::vector<TaskState> taskStates;
    TaskState taskState;
    for (auto &vmem : fileArray_) {
        taskState = {vmem.filename, vmem.result, GetCodeMessage(vmem.result)};
        taskStates.push_back(taskState);
    }
    return taskStates;
}
void UploadTask::OnFail()
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "OnFail. In.");
    if (uploadConfig_->protocolVersion == "L5") {
        return;
    }
    std::lock_guard<std::mutex> guard(mutex_);
    std::vector<TaskState> taskStates = GetTaskStates();
    taskStates_ = taskStates;
    state_ = STATE_FAILURE;
    if (failCallback_) {
        failCallback_->Fail(taskStates);
    }
}

void UploadTask::OnComplete()
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "OnComplete. In.");
    std::lock_guard<std::mutex> guard(mutex_);
    std::vector<TaskState> taskStates = GetTaskStates();
    taskStates_ = taskStates;
    state_ = STATE_SUCCESS;
    if (completeCallback_) {
        completeCallback_->Complete(taskStates);
    }
}

void UploadTask::ExecuteTask()
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "ExecuteTask. In.");
    thread_ = std::make_unique<std::thread>(UploadTask::Run, this);
    thread_handle_ = thread_->native_handle();
    thread_->detach();
}

void UploadTask::ClearFileArray()
{
    std::lock_guard<std::mutex> guard(mutex_);
    for (auto &file : fileArray_) {
        if (file.fp != NULL) {
            fclose(file.fp);
        }
        file.name = "";
    }
    fileArray_.clear();
}

std::vector<std::string> UploadTask::StringSplit(const std::string &str, char delim)
{
    std::size_t previous = 0;
    std::size_t current = str.find(delim);
    std::vector<std::string> elems;
    while (current != std::string::npos) {
        if (current > previous) {
            elems.push_back(str.substr(previous, current - previous));
        }
        previous = current + 1;
        current = str.find(delim, previous);
    }
    if (previous != str.size()) {
        elems.push_back(str.substr(previous));
    }
    return elems;
}
} // namespace  OHOS::Plugin::Request::Upload