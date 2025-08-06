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

#ifndef PLUGINS_REQUEST_UPLOAD_PROXY_H
#define PLUGINS_REQUEST_UPLOAD_PROXY_H

#include "constant.h"
#include "i_task_adp.h"
#import "OHNetworkKit.h"

namespace OHOS::Plugin::Request {
class UploadProxy final : public ITaskAdp {
public:
    UploadProxy(int64_t taskId, const Config &config, OnRequestCallback callback);
    virtual ~UploadProxy();

    int32_t Start(int64_t taskId) override;
    int32_t Pause(int64_t taskId) override;
    int32_t Resume(int64_t taskId) override;
    int32_t Stop(int64_t taskId) override;

private:
    void InitTaskInfo(const Config &config, TaskInfo &info);
    void PutUpload();
    void PostUpload();
    void CompletionHandler(NSURLResponse *response, id responseObject, NSError *error);
    void OnResponseCallback(NSURLResponse *response);
    void OnHeaderReceiveCallback(NSURLResponse *response, id responseObject);
    void OnProgressCallback(NSProgress *progress, int32_t index);
    void OnCompletedCallback();
    void OnFailedCallback();
    void ChangeState(State state);
    int64_t GetTotalFileSize() const;
    NSString *GetUploadPartFile(const FileSpec &file);
    void GetExtras(NSURLResponse *response);
    void PartPostUpload(NSURL *baseUrl, NSMutableDictionary *headers);
    void PostUploadFile(const FileSpec &file, int32_t index, NSURL *baseUrl, NSMutableDictionary *headers);
    void PartPostUploadFile(const FileSpec &file, int32_t index, NSString *partFilePath, NSURL *baseUrl, NSMutableDictionary *headers);
    void RemoveFile(NSString *filePath);
    void PartPutUpload(NSURL *baseUrl, NSMutableDictionary *headers);
    void PutUploadFile(const FileSpec &file, int32_t index, NSURL *baseUrl, NSMutableDictionary *headers);
    void PartPutUploadFile(const FileSpec &file, int32_t index, NSString *partFilePath, NSURL *baseUrl, NSMutableDictionary *headers);
    void PushBackOcValues(NSArray<NSString *> *ocValues, std::vector<std::string> &cppValues);
    NSString* GetStandardHTTPReason(NSInteger statusCode);

private:
    Config config_;
    TaskInfo info_;
    OHSessionManager *sessionCtrl_ = nil;
    std::vector<NSURLSessionUploadTask *> uploadTaskList_ {};
    int respCount_ = 0;
    int fileCount_ = 0;
    OnRequestCallback callback_ = nullptr;
    int64_t taskId_ = INVALID_TASK_ID;
    int64_t currentTime_ = 0;
    bool isStopped_ = false;
};
} // namespace OHOS::Plugin::Request

#endif // PLUGINS_REQUEST_UPLOAD_PROXY_H