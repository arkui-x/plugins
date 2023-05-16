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

#include "i_upload_task.h"
#include <numeric>
#include "upload_hilog_wrapper.h"


namespace OHOS::Plugin::Request::Upload {
IUploadTask::IUploadTask(std::shared_ptr<UploadConfig> &uploadConfig)
    : uploadConfig_(uploadConfig)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "IUploadTask. alloc.");
}

IUploadTask::~IUploadTask()
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "~IUploadTask. free.");
}

void IUploadTask::On(Type type, void *callback)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "On. In.");
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    SetCallback(type, callback);
}

void IUploadTask::Off(Type type, void *callback)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "Off. In.");
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    if (callback == nullptr) {
        return;
    }

    if (type == TYPE_PROGRESS_CALLBACK && callback != nullptr) {
        ((IProgressCallback*)callback)->Progress(uploadedSize_, totalSize_);
    }
    if (type == TYPE_HEADER_RECEIVE_CALLBACK && callback != nullptr) {
        std::string header = std::accumulate(headerArray_.begin(), headerArray_.end(), std::string(""));
        (static_cast<IHeaderReceiveCallback *>(callback))->HeaderReceive(header);
    }
    if (type == TYPE_FAIL_CALLBACK && callback != nullptr) {
        ((IFailCallback*)callback)->Fail(taskStates_);
    }
    if (type == TYPE_COMPLETE_CALLBACK && callback != nullptr) {
        ((ICompleteCallback*)callback)->Complete(taskStates_);
    }
    SetCallback(type, nullptr);
}

void IUploadTask::SetCallback(Type type, void *callback)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "SetCallback. In.");
    if (type == TYPE_PROGRESS_CALLBACK) {
        progressCallback_ = (IProgressCallback*)callback;
        if (progressCallback_ && uploadedSize_ > 0) {
            progressCallback_->Progress(uploadedSize_, totalSize_);
        }
    } else if (type == TYPE_HEADER_RECEIVE_CALLBACK) {
        headerReceiveCallback_ = (IHeaderReceiveCallback *)callback;
        if (headerReceiveCallback_ && headerArray_.empty() == false) {
            for (auto header : headerArray_) {
                if (header.length() > 0) {
                    headerReceiveCallback_->HeaderReceive(header);
                }
            }
            headerArray_.clear();
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

void IUploadTask::SetUploadTaskRelease(IUploadTaskRelease *task, void *arg)
{
    uploadTaskRelease_ = task;
    uploadTaskData_ = arg;
}

bool IUploadTask::IsRunning()
{
    return (state_ == STATE_RUNNING);
}

std::string IUploadTask::GetCodeMessage(uint32_t code)
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

    for (const auto &it : codeMap) {
        if (it.first == code) {
            return it.second;
        }
    }
    return "unknown";
}

void IUploadTask::UploadTaskRelease()
{
    if (uploadTaskRelease_) {
        uploadTaskRelease_->OnUploadTaskRelease(uploadTaskData_);
    }
}
} // namespace OHOS::Plugin::Request::Upload