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

#ifndef PLUGINS_REQUEST_UPLOAD_IOS_UPLOAD_ADP_H
#define PLUGINS_REQUEST_UPLOAD_IOS_UPLOAD_ADP_H

#include <memory>
#include "upload_config.h"
#include "upload_common.h"

namespace OHOS::Plugin::Request::Upload {
class IosUploadAdpCallback;
class IosUploadAdp {
public:
    IosUploadAdp() = default;
    virtual ~IosUploadAdp() {}
    static std::shared_ptr<IosUploadAdp> Instance();
    static bool IsRegularFiles(const std::vector<File> &files);
    virtual void Upload(std::shared_ptr<UploadConfig> &config, IosUploadAdpCallback *callback) = 0;
    virtual bool Remove() = 0;
};

class IosUploadAdpCallback {
public:
    IosUploadAdpCallback() = default;
    virtual ~IosUploadAdpCallback() {}
    virtual void OnProgress(uint32_t uploadedSize, uint32_t totalSize) = 0;
    virtual void OnFail(const std::vector<TaskState>& taskStateList) = 0;
    virtual void OnComplete(const std::vector<TaskState>& taskStateList) = 0;
    virtual void SetRespHeader(const std::string &header) = 0;
};
} // namespace OHOS::Plugin::Request::Upload

#endif // PLUGINS_REQUEST_UPLOAD_IOS_UPLOAD_ADP_H