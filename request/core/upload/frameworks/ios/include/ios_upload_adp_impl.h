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

#ifndef PLUGINS_REQUEST_UPLOAD_IOS_UPLOAD_ADP_IMPL_H
#define PLUGINS_REQUEST_UPLOAD_IOS_UPLOAD_ADP_IMPL_H

#include "ios_upload_adp.h"
#import "OHNetworkKit.h"
#include <mutex>


namespace OHOS::Plugin::Request::Upload {

class IosUploadAdpImpl final : public IosUploadAdp {
public:
    IosUploadAdpImpl();
    ~IosUploadAdpImpl();
    static int64_t GetFileSize(NSURL *filePath);
    void Upload(std::shared_ptr<UploadConfig> &config, IosUploadAdpCallback *callback) override;
    bool Remove() override;

private:
    void PutUpdate(const std::string &method, std::shared_ptr<UploadConfig> &config, IosUploadAdpCallback *callback);
    void PostUpdate(const std::string &method, std::shared_ptr<UploadConfig> &config, IosUploadAdpCallback *callback);
    void ParseHeader(const std::string &header, std::string &key, std::string &value);
    void PutCompletionHandler(std::shared_ptr<UploadConfig> &config, IosUploadAdpCallback *callback,
        NSURLResponse *response, id responseObject, NSError *error);
    void PostCompletionHandler(std::shared_ptr<UploadConfig> &config, IosUploadAdpCallback *callback,
        NSURLResponse *response, id responseObject, NSError *error);
    void ResumePutTask();

private:
    OHHttpSessionController *sessionCtrl_ = nil;
    NSURLSessionUploadTask *uploadTask_ = nil;
    std::vector<NSURLSessionUploadTask *> putUploadTaskList_;
    int64_t putCompletedUnitCount_ = 0;
    int64_t putTotalUnitCount_ = 0;
    bool putHasError_ = false;
    int putRspCount_ = 0;
    int putFileCount_ = 0;
    std::mutex mutex_;
};

} // namespace OHOS::Plugin::Request::Upload

#endif // PLUGINS_REQUEST_UPLOAD_IOS_UPLOAD_ADP_IMPL_H