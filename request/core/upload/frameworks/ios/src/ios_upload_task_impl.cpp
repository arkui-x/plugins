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

#include "ios_upload_task_impl.h"
#include "upload_hilog_wrapper.h"


namespace OHOS::Plugin::Request::Upload {
std::shared_ptr<IUploadTask> IUploadTask::CreateUploadTask(std::shared_ptr<UploadConfig> &uploadConfig)
{
    return std::make_shared<IosUploadTaskImpl>(uploadConfig);
}

bool IUploadTask::CheckFilesValid(const std::vector<File> &files)
{
    return IosUploadAdp::IsRegularFiles(files);
}

IosUploadTaskImpl::IosUploadTaskImpl(std::shared_ptr<UploadConfig> &uploadConfig)
    : IUploadTask(uploadConfig)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "IosUploadTaskImpl allocated");
    uploadAdp_ = IosUploadAdp::Instance();
}

IosUploadTaskImpl::~IosUploadTaskImpl()
{
    uploadAdp_ = nullptr;
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "IosUploadTaskImpl freed");
}

// IUploadTask
void IosUploadTaskImpl::ExecuteTask()
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "ExecuteTask. In.");
    state_ = STATE_RUNNING;
    uploadAdp_->Upload(uploadConfig_, this);
}

bool IosUploadTaskImpl::Remove()
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "Remove. In.");
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    isRemoved_ = true;
    return uploadAdp_->Remove();
}

// IosUploadAdpCallback
void IosUploadTaskImpl::OnProgress(uint32_t uploadedSize, uint32_t totalSize)
{
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    if (isRemoved_) {
        return;
    }
    uploadedSize_ = uploadedSize;
    totalSize_ = totalSize;
    if (progressCallback_ != nullptr) {
        progressCallback_->Progress(uploadedSize, totalSize);
    }
}

void IosUploadTaskImpl::OnFail(const std::vector<TaskState>& taskStateList)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "OnFail taskStateList size:%{public}d", taskStateList.size());
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    if (isRemoved_) {
        return;
    }
    state_ = STATE_FAILURE;
    taskStates_ = taskStateList;
    if (failCallback_ != nullptr) {
        failCallback_->Fail(taskStateList);
    }
}

void IosUploadTaskImpl::OnComplete(const std::vector<TaskState>& taskStateList)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "OnComplete taskStateList size:%{public}d", taskStateList.size());
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    if (isRemoved_) {
        return;
    }
    state_ = STATE_SUCCESS;
    taskStates_ = taskStateList;
    if (completeCallback_ != nullptr) {
        completeCallback_->Complete(taskStateList);
        for (const auto& item : taskStates_) {
            UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK,
                "taskState: path:%{public}s, responseCode:%{public}d, message:%{public}s",
                item.path.c_str(), item.responseCode, item.message.c_str());
        }
    }
}

void IosUploadTaskImpl::SetRespHeader(const std::string &header)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "SetRespHeader");
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    if (isRemoved_) {
        return;
    }
    if (headerReceiveCallback_ != nullptr) {
        headerReceiveCallback_->HeaderReceive(header);
    } else {
        headerArray_.push_back(header);
    }
}
} // namespace OHOS::Plugin::Request::Upload