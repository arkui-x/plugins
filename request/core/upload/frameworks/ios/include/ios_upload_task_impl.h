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

#ifndef PLUGINS_REQUEST_UPLOAD_IOS_UPLOAD_TASK_IMPL
#define PLUGINS_REQUEST_UPLOAD_IOS_UPLOAD_TASK_IMPL

#include "i_upload_task.h"
#include "ios_upload_adp.h"

namespace OHOS::Plugin::Request::Upload {
class IosUploadTaskImpl final : public IUploadTask, public IosUploadAdpCallback {
public:
    explicit IosUploadTaskImpl(std::shared_ptr<UploadConfig> &uploadConfig);
    ~IosUploadTaskImpl();

public:
    // IUploadTask
    bool Remove() override;
    void ExecuteTask() override;

public:
    // IosUploadAdpCallback
    void OnProgress(int64_t uploadedSize, int64_t totalSize) override;
    void OnFail(const std::vector<TaskState>& taskStateList) override;
    void OnComplete(const std::vector<TaskState>& taskStateList) override;
    void SetRespHeader(const std::string &header) override;

private:
    std::shared_ptr<IosUploadAdp> uploadAdp_ = nullptr;
    bool isRemoved_ = false;
};
} // namespace OHOS::Plugin::Request::Upload
#endif // PLUGINS_REQUEST_UPLOAD_IOS_UPLOAD_TASK_IMPL