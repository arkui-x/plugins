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
 
#include "ios_upload_adp_impl.h"
#import <Foundation/Foundation.h>
#import "OHNetworkKit.h"
#include <stdio.h>
#include <numeric>
#include "i_upload_task.h"
#include "upload_hilog_wrapper.h"
#import "ios_certificate_utils.h"


namespace OHOS::Plugin::Request::Upload {

std::shared_ptr<IosUploadAdp> IosUploadAdp::Instance()
{
    return std::make_shared<IosUploadAdpImpl>();
}

bool IosUploadAdp::IsRegularFiles(const std::vector<File> &files)
{
    for (const auto &file : files) {
        std::string fileUri = file.uri;
        if (fileUri.empty()) {
            return false;
        }
        NSString *strFilePath = [NSString stringWithUTF8String:fileUri.c_str()];
        bool bExist = [[NSFileManager defaultManager] fileExistsAtPath:strFilePath];
        if (!bExist) {
            UPLOAD_HILOGE(UPLOAD_MODULE_FRAMEWORK, "invalid file path:%{public}s", fileUri.c_str());
            return false;
        }
    }
    return true;
}

IosUploadAdpImpl::IosUploadAdpImpl()
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "IosUploadAdpImpl allocated");
}

IosUploadAdpImpl::~IosUploadAdpImpl()
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "IosUploadAdpImpl freed");
    sessionCtrl_ = nil;
}

void IosUploadAdpImpl::Upload(std::shared_ptr<UploadConfig> &config, IosUploadAdpCallback *callback)
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        std::string method = !config->method.empty() ? config->method : "POST"; // default: POST
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "upload method:%{public}s", method.c_str());
        if (method == "POST") {
            PostUpdate(method, config, callback);
        } else if (method == "PUT") {
            PutUpdate(method, config, callback);
        } else {
            UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "unsupport method: %{public}s, accept: POST or PUT", method.c_str());
        }
    });
}

void IosUploadAdpImpl::PutUpdate(const std::string &method, std::shared_ptr<UploadConfig> &config, IosUploadAdpCallback *callback)
{
    NSString *url = [NSString stringWithUTF8String:config->url.c_str()];
    NSURL *baseUrl = [NSURL URLWithString:url];
    sessionCtrl_ = [[OHSessionManager alloc] initWithConfiguration:nil];
    
    NSMutableDictionary *headers = [[NSMutableDictionary alloc] init];
    [headers setValue:@"application/octet-stream" forKey:@"Content-Type"];

    for (auto it = config->header.begin(); it != config->header.end(); it++) {
        std::string key = it->first;
        std::string value = it->second;
        [headers setValue:[NSString stringWithUTF8String:value.c_str()] forKey:[NSString stringWithUTF8String:key.c_str()]];
    }
    NSString *methodStr = [NSString stringWithUTF8String:method.c_str()];
    if ([url hasPrefix:@"https"]) {
        OHOS::Plugin::Request::CertificateUtils::InstallCertificateChain(sessionCtrl_);
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "is https upload");
    } else {
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "is http upload");
    }

    for (const auto& file : config->files) {
        NSString *filePath = [NSString stringWithUTF8String:file.uri.c_str()];
        NSString *name = [NSString stringWithUTF8String:!file.name.empty() ? file.name.c_str() : "file"]; // default: file
        NSString *fileName = [NSString stringWithUTF8String:file.filename.c_str()];
        NSURL *localPath = [NSURL fileURLWithPath:filePath];
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "upload name:%{public}s, localPath:%{public}s", [name UTF8String], [[localPath description] UTF8String]);
        putTotalUnitCount_ += GetFileSize(localPath);
        putFileCount_++;
        
        NSURL *dstUrl = [baseUrl URLByAppendingPathComponent:fileName];
        NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:dstUrl];
        request.HTTPMethod = methodStr;
        [request setAllHTTPHeaderFields:headers];
        
        NSURLSessionUploadTask *task = [sessionCtrl_ uploadWithRequest:request
                                                              fromFile:localPath
                                                         progressBlock:^(NSProgress *uploadProgress) {
            if (callback != nullptr) {
                std::lock_guard<std::mutex> guard(mutex_);
                putCompletedUnitCount_ += uploadProgress.completedUnitCount;
                callback->OnProgress(putCompletedUnitCount_, putTotalUnitCount_);
            }
        } completion:^(NSURLResponse *response, id responseObject, NSError *error) {
            PutCompletionHandler(config, callback, response, responseObject, error);
        }];

        putUploadTaskList_.push_back(task);
    }
    ResumePutTask();
}

int64_t IosUploadAdpImpl::GetFileSize(NSURL *filePath)
{
    NSFileHandle *fileHandle = [NSFileHandle fileHandleForReadingFromURL:filePath error:nil];
    int64_t fileSize = 0;
    [fileHandle seekToEndReturningOffset:(unsigned long long *)&fileSize error:nil];
    return fileSize;
}

void IosUploadAdpImpl::PutCompletionHandler(std::shared_ptr<UploadConfig> &config, IosUploadAdpCallback *callback,
    NSURLResponse *response, id responseObject, NSError *error)
{
    std::lock_guard<std::mutex> guard(mutex_);
    if (error != nil) {
        putHasError_ = true;
    }
    putRspCount_++;
    // header
    if (callback != nullptr) {
        std::vector<std::string> headerList;
        NSHTTPURLResponse *resp = (NSHTTPURLResponse *)response;
        NSDictionary *allHeader = [resp allHeaderFields];
        for (NSString *key in allHeader.allKeys) {
            NSString *value = [allHeader objectForKey:key];
            std::string header;
            header = std::string([key UTF8String]) + ": " + std::string([value UTF8String]) + "\r\n";
            headerList.push_back(header);
        }
        std::string respHeader = std::accumulate(headerList.begin(), headerList.end(), std::string(""));
        if (!respHeader.empty()) {
            callback->SetRespHeader(respHeader);
        }
    }
    // all responded
    if (putRspCount_ == putFileCount_) {
        std::vector<TaskState> taskStateList;
        for (const auto& file: config->files) {
            TaskState taskState;
            taskState.path = file.filename;
            if (putHasError_) {
                taskState.responseCode = UPLOAD_ERRORCODE_UPLOAD_FAIL;
            } else {
                taskState.responseCode = UPLOAD_OK;
            }
            taskState.message = IUploadTask::GetCodeMessage(taskState.responseCode);
            taskStateList.push_back(taskState);
        }
        if (putHasError_) {
            UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "upload PUT failed. Error: %{public}s", [[error description] UTF8String]);
            if (callback != nullptr) {
                callback->OnFail(taskStateList);
            }
        } else {
            UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "upload, finished. resp: %{public}s, respObj: %{public}s",
                [[response description] UTF8String], [[responseObject description] UTF8String]);
            if (callback != nullptr) {
                callback->OnComplete(taskStateList);
            }
        }
    }
}

void IosUploadAdpImpl::ResumePutTask()
{
    for (auto &task : putUploadTaskList_) {
        if (task != nil) {
            [task resume];
        }
    }
}

void IosUploadAdpImpl::PostUpdate(const std::string &method, std::shared_ptr<UploadConfig> &config, IosUploadAdpCallback *callback)
{
    NSString *url = [NSString stringWithUTF8String:config->url.c_str()];
    NSURL *baseUrl = [NSURL URLWithString:url];
    sessionCtrl_ = [[OHSessionManager alloc] initWithConfiguration:nil];
    
    NSMutableDictionary *headers = [[NSMutableDictionary alloc] init];
    for (auto it = config->header.begin(); it != config->header.end(); it++) {
        std::string key = it->first;
        std::string value = it->second;
        [headers setValue:[NSString stringWithUTF8String:value.c_str()] forKey:[NSString stringWithUTF8String:key.c_str()]];
    }
    if ([url hasPrefix:@"https"]) {
        OHOS::Plugin::Request::CertificateUtils::InstallCertificateChain(sessionCtrl_);
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "is https upload");
    } else {
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "is http upload");
    }

    NSMutableURLRequest *request = [OHMultipartFormStream requestWithURL:baseUrl 
                                                                  method:[NSString stringWithUTF8String:method.c_str()] 
                                                              parameters:headers
                                                     multipartFormStream:^(OHMultipartFormStream * multipartStream) {
        // name-value of form data
        for (const auto& formDt : config->data) {
            [multipartStream appendWithFormData:[NSData dataWithBytes:formDt.value.c_str() length:formDt.value.size()] 
                                           name:[NSString stringWithUTF8String:formDt.name.c_str()]];
        }
        // files
        for (const auto& file : config->files) {
            NSString *filePath = [NSString stringWithUTF8String:file.uri.c_str()];
            NSString *name = [NSString stringWithUTF8String:!file.name.empty() ? file.name.c_str() : "file"]; // default: file
            NSString *fileName = [NSString stringWithUTF8String:file.filename.c_str()];
            NSURL *localPath = [NSURL fileURLWithPath:filePath];
            UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "upload name:%{public}s, localPath:%{public}s",
                    [name UTF8String], [[localPath description] UTF8String]);
            [multipartStream appendWithFilePath:localPath fileName:fileName fieldName:name mimeType:@"application/octet-stream"];
        }
    }];

    uploadTask_ = [sessionCtrl_ uploadWithStreamRequest:request
                                          progressBlock:^(NSProgress *progess) {
        if (callback != nullptr) {
            callback->OnProgress(progess.completedUnitCount, progess.totalUnitCount);
        }
    } completion:^(NSURLResponse *response, id responseObject, NSError *error) {
        PostCompletionHandler(config, callback, response, responseObject, error);
    }];
    [uploadTask_ resume];
}

void IosUploadAdpImpl::PostCompletionHandler(std::shared_ptr<UploadConfig> &config, IosUploadAdpCallback *callback,
    NSURLResponse *response, id responseObject, NSError *error)
{
    std::vector<TaskState> taskStateList;
    for (const auto& file: config->files) {
        TaskState taskState;
        taskState.path = file.filename;
        if (error != nil) {
            taskState.responseCode = UPLOAD_ERRORCODE_UPLOAD_FAIL;
        } else {
            taskState.responseCode = UPLOAD_OK;
        }
        taskState.message = IUploadTask::GetCodeMessage(taskState.responseCode);
        taskStateList.push_back(taskState);
    }

    // header
    if (callback != nullptr) {
        std::vector<std::string> headerList;
        NSHTTPURLResponse *resp = (NSHTTPURLResponse *)response;
        NSDictionary *allHeader = [resp allHeaderFields];
        for (NSString *key in allHeader.allKeys) {
            NSString *value = [allHeader objectForKey:key];
            std::string header;
            header = std::string([key UTF8String]) + ": " + std::string([value UTF8String]) + "\r\n";
            headerList.push_back(header);
        }
        std::string respHeader = std::accumulate(headerList.begin(), headerList.end(), std::string(""));
        if (!respHeader.empty()) {
            callback->SetRespHeader(respHeader);
        }
    }

    if (error != nil) {
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "upload failed. Error: %{public}s", [[error description] UTF8String]);
        if (callback != nullptr) {
            callback->OnFail(taskStateList);
        }
    } else {
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "upload, finished. resp:%{public}s, respObj:%{public}s",
            [[response description] UTF8String], [[responseObject description] UTF8String]);
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "upload completed.");
        if (callback != nullptr) {
            callback->OnComplete(taskStateList);
        }
    }
}

bool IosUploadAdpImpl::Remove()
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "Remove. In.");
    if (uploadTask_ != nil) {
        [uploadTask_ cancel];
    } else {
        // for PUT method
        for (auto &task : putUploadTaskList_) {
            if (task != nil) {
                [task cancel];
            }
        }
    }
    return true;
}

} // end of OHOS::Plugin::Request::Upload
