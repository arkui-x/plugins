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
 
#include "upload_proxy.h"
#include <stdio.h>
#include <numeric>
#include "log.h"
#import "ios_certificate_utils.h"
#import "json_utils.h"
#include "request_utils.h"

namespace OHOS::Plugin::Request {
using namespace std;
static string GetCodeMessage(uint32_t code)
{
    vector<pair<ExceptionErrorCode, string>> codeMap = {
        {E_OK, "file uploaded successfully"},
        {E_FILE_PATH, "file path error"},
        {E_FILE_IO, "failed to get file"},
        {E_SERVICE_ERROR,  "upload failed"},
    };

    for (const auto &it : codeMap) {
        if (static_cast<uint32_t>(it.first) == code) {
            return it.second;
        }
    }
    return "unknown";
}

UploadProxy::UploadProxy(int64_t taskId, const Config &config, OnRequestCallback callback)
    : config_(config), callback_(callback)
{
    REQUEST_HILOGI("UploadProxy allocated");
}

UploadProxy::~UploadProxy()
{
    REQUEST_HILOGI("UploadProxy freed");
    REQUEST_HILOGI("Remove. In.");
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
    sessionCtrl_ = nil;
}

int32_t UploadProxy::Start(int64_t taskId)
{
    REQUEST_HILOGI("UploadProxy::Start enter");
    taskId_ = taskId;
    IosTaskDao::QueryTaskInfo(taskId, "", info_);
    InitTaskInfo(config_, info_);
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        string method = !config_.method.empty() ? config_.method : "POST"; // default: POST
        REQUEST_HILOGI("upload method:%{public}s", method.c_str());
        if (method == "POST") {
            PostUpdate(method);
        } else if (method == "PUT") {
            PutUpdate(method);
        } else {
            REQUEST_HILOGI("unsupport method: %{public}s, accept: POST or PUT", method.c_str());
        }
    });
    info_.progress.extras.clear();
    info_.progress.bodyBytes.clear();
    info_.progress.index++;
    info_.progress.totalProcessed = 0;
    return E_OK;
}

int32_t UploadProxy::Pause(int64_t taskId)
{
    REQUEST_HILOGI("UploadProxy::Pause enter");
    return E_OK;
}

int32_t UploadProxy::Resume(int64_t taskId)
{
    REQUEST_HILOGI("UploadProxy::Resume enter");
    return E_OK;
}

int32_t UploadProxy::Stop(int64_t taskId)
{
    REQUEST_HILOGI("UploadProxy::Stop enter");
    if (uploadTask_ != nil) {
        [uploadTask_ cancel];
    }
    return E_OK;
}

void UploadProxy::PutUpdate(const string &method)
{
    REQUEST_HILOGI("UploadProxy::PutUpdate enter");
    NSString *url = [NSString stringWithUTF8String:config_.url.c_str()];
    NSURL *baseUrl = [NSURL URLWithString:url];
    sessionCtrl_ = [[OHSessionManager alloc] initWithConfiguration:nil];
    
    NSMutableDictionary *headers = [[NSMutableDictionary alloc] init];
    [headers setValue:@"application/octet-stream" forKey:@"Content-Type"];

    for (auto it = config_.headers.begin(); it != config_.headers.end(); it++) {
        [headers setValue:[NSString stringWithUTF8String:it->first.c_str()]
            forKey:[NSString stringWithUTF8String:it->second.c_str()]];
    }
    NSString *methodStr = [NSString stringWithUTF8String:method.c_str()];
    if ([url hasPrefix:@"https"]) {
        OHOS::Plugin::Request::CertificateUtils::InstallCertificateChain(sessionCtrl_);
        REQUEST_HILOGI("is https upload");
    } else {
        REQUEST_HILOGI("is http upload");
    }

    for (const auto& file : config_.files) {
        NSString *filePath = [NSString stringWithUTF8String:file.uri.c_str()];
        NSString *name = [NSString stringWithUTF8String:!file.name.empty() ? file.name.c_str() : "file"]; // default: file
        NSString *fileName = [NSString stringWithUTF8String:file.filename.c_str()];
        NSURL *localPath = [NSURL fileURLWithPath:filePath];
        REQUEST_HILOGI("upload name:%{public}s, localPath:%{public}s", [name UTF8String], [[localPath description] UTF8String]);
        putTotalUnitCount_ += GetFileSize(localPath);
        putFileCount_++;
        NSURL *dstUrl = [baseUrl URLByAppendingPathComponent:fileName];
        NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:dstUrl];
        request.HTTPMethod = methodStr;
        [request setAllHTTPHeaderFields:headers];
        
        NSURLSessionUploadTask *task = [sessionCtrl_ uploadWithRequest:request
                                                              fromFile:localPath
                                                         progressBlock:^(NSProgress *uploadProgress) {
            REQUEST_HILOGI("uploading file...");
            OnProgressCallback(uploadProgress);
        } completion:^(NSURLResponse *response, id responseObject, NSError *error) {
            PutCompletionHandler(response, responseObject, error);
        }];

        putUploadTaskList_.push_back(task);
    }
    ResumePutTask();
}

int64_t UploadProxy::GetFileSize(NSURL *filePath)
{
    NSFileHandle *fileHandle = [NSFileHandle fileHandleForReadingFromURL:filePath error:nil];
    int64_t fileSize = 0;
    [fileHandle seekToEndReturningOffset:(unsigned long long *)&fileSize error:nil];
    return fileSize;
}

void UploadProxy::PutCompletionHandler(NSURLResponse *response, id responseObject, NSError *error)
{
    lock_guard<mutex> guard(mutex_);
    if (error != nil) {
        putHasError_ = true;
    }
    putRspCount_++;
    // header
    vector<string> headerList;
    NSHTTPURLResponse *resp = (NSHTTPURLResponse *)response;
    NSDictionary *allHeader = [resp allHeaderFields];
    for (NSString *key in allHeader.allKeys) {
        NSString *value = [allHeader objectForKey:key];
        string header;
        header = string([key UTF8String]) + ": " + string([value UTF8String]) + "\r\n";
        headerList.push_back(header);
    }
    string respHeader = accumulate(headerList.begin(), headerList.end(), string(""));
    if (!respHeader.empty()) {
        // callback->SetRespHeader(respHeader);
    }

    // all responded
    if (putRspCount_ == putFileCount_) {
        vector<TaskState> taskStateList;
        for (const auto& file: config_.files) {
            TaskState taskState;
            taskState.path = file.filename;
            if (putHasError_) {
                taskState.responseCode = E_SERVICE_ERROR;
            } else {
                taskState.responseCode = E_OK;
            }
            taskState.message = GetCodeMessage(taskState.responseCode);
            taskStateList.push_back(taskState);
        }
        if (putHasError_) {
            REQUEST_HILOGI("upload PUT failed. Error: %{public}s", [[error description] UTF8String]);
            ChangeState(State::FAILED);
        } else {
            REQUEST_HILOGI("upload, finished. resp: %{public}s, respObj: %{public}s",
                [[response description] UTF8String], [[responseObject description] UTF8String]);
            ChangeState(State::COMPLETED);
        }
    }
}

void UploadProxy::ResumePutTask()
{
    for (auto &task : putUploadTaskList_) {
        if (task != nil) {
            [task resume];
        }
    }
}

void UploadProxy::PostUpdate(const string &method)
{
    REQUEST_HILOGI("UploadProxy::PostUpdate enter");
    NSString *url = [NSString stringWithUTF8String:config_.url.c_str()];
    NSURL *baseUrl = [NSURL URLWithString:url];
    sessionCtrl_ = [[OHSessionManager alloc] initWithConfiguration:nil];
    
    NSMutableDictionary *headers = [[NSMutableDictionary alloc] init];
    for (auto it = config_.headers.begin(); it != config_.headers.end(); it++) {
        string key = it->first;
        string value = it->second;
        [headers setValue:[NSString stringWithUTF8String:value.c_str()] forKey:[NSString stringWithUTF8String:key.c_str()]];
    }
    if ([url hasPrefix:@"https"]) {
        OHOS::Plugin::Request::CertificateUtils::InstallCertificateChain(sessionCtrl_);
        REQUEST_HILOGI("is https upload");
    } else {
        REQUEST_HILOGI("is http upload");
    }

    NSMutableURLRequest *request = [OHMultipartFormStream requestWithURL:baseUrl 
                                                                  method:[NSString stringWithUTF8String:method.c_str()] 
                                                              parameters:headers
                                                     multipartFormStream:^(OHMultipartFormStream * multipartStream) {
        // name-value of form data
        for (const auto& formDt : config_.forms) {
            [multipartStream appendWithFormData:[NSData dataWithBytes:formDt.value.c_str() length:formDt.value.size()] 
                                           name:[NSString stringWithUTF8String:formDt.name.c_str()]];
        }
        // files
        for (const auto& file : config_.files) {
            NSString *filePath = [NSString stringWithUTF8String:file.uri.c_str()];
            NSString *name = [NSString stringWithUTF8String:!file.name.empty() ? file.name.c_str() : "file"]; // default: file
            NSString *fileName = [NSString stringWithUTF8String:file.filename.c_str()];
            NSURL *localPath = [NSURL fileURLWithPath:filePath];
            REQUEST_HILOGI("upload name:%{public}s, localPath:%{public}s", [name UTF8String], [[localPath description] UTF8String]);
            [multipartStream appendWithFilePath:localPath fileName:fileName fieldName:name mimeType:@"application/octet-stream"];
        }
    }];

    uploadTask_ = [sessionCtrl_ uploadWithStreamRequest:request
                                          progressBlock:^(NSProgress *progress) {
        REQUEST_HILOGI("uploading file...");
        OnProgressCallback(progress);
    } completion:^(NSURLResponse *response, id responseObject, NSError *error) {
        PostCompletionHandler(response, responseObject, error);
    }];
    [uploadTask_ resume];
}

void UploadProxy::PostCompletionHandler(NSURLResponse *response, id responseObject, NSError *error)
{
    vector<TaskState> taskStateList;
    for (const auto& file: config_.files) {
        TaskState taskState;
        taskState.path = file.filename;
        if (error != nil) {
            taskState.responseCode = E_SERVICE_ERROR;
        } else {
            taskState.responseCode = E_OK;
        }
        taskState.message = GetCodeMessage(taskState.responseCode);
        taskStateList.push_back(taskState);
    }

    // header
    vector<string> headerList;
    NSHTTPURLResponse *resp = (NSHTTPURLResponse *)response;
    NSDictionary *allHeader = [resp allHeaderFields];
    for (NSString *key in allHeader.allKeys) {
        NSString *value = [allHeader objectForKey:key];
        string header;
        header = string([key UTF8String]) + ": " + string([value UTF8String]) + "\r\n";
        headerList.push_back(header);
    }
    string respHeader = accumulate(headerList.begin(), headerList.end(), string(""));
    if (!respHeader.empty()) {
        // callback->SetRespHeader(respHeader);
    }

    if (error != nil) {
        REQUEST_HILOGI("upload failed. Error: %{public}s", [[error description] UTF8String]);
        ChangeState(State::FAILED);
    } else {
        REQUEST_HILOGI("upload, finished. resp:%{public}s, respObj:%{public}s",
            [[response description] UTF8String], [[responseObject description] UTF8String]);
        REQUEST_HILOGI("upload completed.");
        ChangeState(State::COMPLETED);
    }
}

void UploadProxy::InitTaskInfo(const Config &config, TaskInfo &info)
{
    info.version = config.version;
    info.url = config.url;
    info.data = config.data;
    info.files = config.files;
    info.forms = config.forms;
    info.tid = std::to_string(taskId_);
    info.title = config.title;
    info.description = config.description;
    info.action = config.action;
    info.mode = config.mode;
    info.mimeType = "";
    info.gauge = config.gauge;
    info.ctime = RequestUtils::GetTimeNow();
    info.mtime = RequestUtils::GetTimeNow();
    info.retry = false;
    info.tries = 0;
    info.faults = Faults::OTHERS;
    info.code = Reason::REASON_OK;
    info.reason = "";
    info.withSystem = false;
    info.priority = 0;
    info.extras = config.extras;
    info.progress.index = config.index;
    if (info.progress.index == 0) {
        info.progress.index = 1;
    }
    info.progress.processed = 0;
    info.progress.totalProcessed = 0;
    for (auto &file : config.files) {
        REQUEST_HILOGI("fileName = %{public}s", file.filename.c_str());
        TaskState state;
        state.path = file.filename;
        state.responseCode = E_SERVICE_ERROR;
        if (file.uri == "") {
            state.responseCode = E_FILE_PATH;
        }
        if (file.filename == "" || file.fd < 0) {
            state.responseCode = E_FILE_IO;
        }
        state.message = GetCodeMessage(state.responseCode);
        if (file.fd > 0) {
            int64_t fileSize = lseek(file.fd, 0, SEEK_END);
            if (config.begins > 0) {
                fileSize -= config.begins;
            }
            if (config.ends > 0) {
                fileSize = config.ends - config.begins;
            }
            info.progress.sizes.emplace_back(fileSize);
            int64_t offset = 0;
            if (info.progress.index == config.index && config.begins > 0) {
                offset = config.begins;
            }
            lseek(file.fd, offset, SEEK_SET);
        }
        info.taskStates.emplace_back(state);
    }
    ChangeState(State::INITIALIZED);
}

void UploadProxy::ChangeState(State state)
{
    info_.progress.state = state;
    info_.mtime = RequestUtils::GetTimeNow();
    if (callback_ == nullptr) {
        return;
    }
    if (state == State::FAILED) {
        callback_(taskId_, EVENT_FAILED, JsonUtils::TaskInfoToJsonString(info_));
    } else if (state == State::COMPLETED) {
        callback_(taskId_, EVENT_COMPLETED, JsonUtils::TaskInfoToJsonString(info_));
    }
}

void UploadProxy::OnProgressCallback(NSProgress *uploadProgress)
{
    if (uploadProgress != nil && callback_ != nullptr) {
        lock_guard<mutex> guard(mutex_);
        putCompletedUnitCount_ += uploadProgress.completedUnitCount;
        info_.progress.state = State::RUNNING;
        info_.progress.processed = putCompletedUnitCount_;
        info_.progress.totalProcessed = putTotalUnitCount_;
        callback_(taskId_, EVENT_PROGRESS, JsonUtils::TaskInfoToJsonString(info_));
        IosTaskDao::UpdateDB(info_, config_);
    }
}

} // end of OHOS::Plugin::Request
