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
    NSLog(@"UploadProxy allocated");
}

UploadProxy::~UploadProxy()
{
    NSLog(@"UploadProxy freed");
    if (uploadTask_ != nil) {
        [uploadTask_ cancel];
        uploadTask_ = nil;
    } else {
        // for PUT method
        for (auto &task : putUploadTaskList_) {
            if (task != nil) {
                [task cancel];
                task = nil;
            }
        }
        putUploadTaskList_.clear();
    }    
    sessionCtrl_ = nil;
}

int32_t UploadProxy::Start(int64_t taskId)
{
    NSLog(@"UploadProxy::Start enter");
    taskId_ = taskId;
    IosTaskDao::QueryTaskInfo(taskId, "", info_);
    InitTaskInfo(config_, info_);
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        string method = !config_.method.empty() ? config_.method : "POST"; // default: POST
        NSLog(@"upload method:%s", method.c_str());
        if (method == "POST") {
            PostUpload(method);
        } else if (method == "PUT") {
            PutUpload(method);
        } else {
            NSLog(@"unsupport method: %s, accept: POST or PUT", method.c_str());
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
    NSLog(@"UploadProxy::Pause enter");
    return E_OK;
}

int32_t UploadProxy::Resume(int64_t taskId)
{
    NSLog(@"UploadProxy::Resume enter");
    return E_OK;
}

int32_t UploadProxy::Stop(int64_t taskId)
{
    NSLog(@"UploadProxy::Stop enter");
    if (uploadTask_ != nil) {
        [uploadTask_ cancel];
    }
    ChangeState(State::STOPPED);
    return E_OK;
}

void UploadProxy::PutUpload(const string &method)
{
    NSLog(@"UploadProxy::PutUpload enter");
    NSString *url = JsonUtils::CStringToNSString(config_.url);
    if (!url) {
        NSLog(@"PutUpload invalid url");
        return;
    }
    NSURL *baseUrl = [NSURL URLWithString:url];
    sessionCtrl_ = [[OHSessionManager alloc] initWithConfiguration:nil];
    
    NSMutableDictionary *headers = [[NSMutableDictionary alloc] init];
    [headers setValue:@"application/octet-stream" forKey:@"Content-Type"];

    for (auto it = config_.headers.begin(); it != config_.headers.end(); it++) {
        [headers setValue:JsonUtils::CStringToNSString(it->first)
            forKey:JsonUtils::CStringToNSString(it->second)];
    }
    NSString *methodStr = JsonUtils::CStringToNSString(method);
    if ([url hasPrefix:@"https"]) {
        OHOS::Plugin::Request::CertificateUtils::InstallCertificateChain(sessionCtrl_);
        NSLog(@"is https upload");
    } else {
        NSLog(@"is http upload");
    }

    for (const auto& file : config_.files) {
        NSString *filePath = JsonUtils::CStringToNSString(file.uri);
        NSString *name = JsonUtils::CStringToNSString(!file.name.empty() ? file.name : "file"); // default: file
        NSString *fileName = JsonUtils::CStringToNSString(file.filename);
        if (!filePath) {
            continue;
        }
        NSURL *localPath = [NSURL fileURLWithPath:filePath];
        NSLog(@"upload name:%s, localPath:%s", name.UTF8String, [localPath description].UTF8String);
        putFileCount_++;
        NSURL *dstUrl = [baseUrl URLByAppendingPathComponent:fileName];
        NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:dstUrl];
        request.HTTPMethod = methodStr;
        [request setAllHTTPHeaderFields:headers];
        
        NSURLSessionUploadTask *task = [sessionCtrl_ uploadWithRequest:request
                                                              fromFile:localPath
                                                         progressBlock:^(NSProgress *progress) {
            NSLog(@"uploading, progress:%@", progress);
            OnProgressCallback(progress);
        } completion:^(NSURLResponse *response, id responseObject, NSError *error) {
            PutCompletionHandler(response, error);
        }];

        putUploadTaskList_.push_back(task);
    }
    ResumePutTask();
}

void UploadProxy::PutCompletionHandler(NSURLResponse *response, NSError *error)
{
    if (error != nil) {
        putHasError_ = true;
    }
    putRspCount_++;

    GetExtras(response);

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
            NSLog(@"upload PUT failed, error: %s", [error description].UTF8String);
            ChangeState(State::FAILED);
        } else {
            NSLog(@"upload completed, response: %s", [response description].UTF8String);
            ChangeState(State::COMPLETED);
        }
    }
}

void UploadProxy::ResumePutTask()
{
    NSLog(@"UploadProxy::ResumePutTask enter");
    for (auto &task : putUploadTaskList_) {
        if (task != nil) {
            [task resume];
        }
    }
}

NSString *UploadProxy::GetUploadPartFile(const FileSpec &file)
{
    NSString *cachesDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) objectAtIndex:0];
    NSString *partFilePath = [cachesDir stringByAppendingString:@"/temp.data"];
    NSString *filePath = JsonUtils::CStringToNSString(file.uri);  
    NSFileHandle *fileHandle = [NSFileHandle fileHandleForReadingAtPath:filePath];  
    if (fileHandle) {  
        NSUInteger fileLength = [fileHandle seekToEndOfFile];  
        NSUInteger readPosition = config_.begins;
        NSUInteger readLength = config_.ends - config_.begins; 
        if (readPosition + readLength <= fileLength) {  
            [fileHandle seekToFileOffset:readPosition];  
            NSData *data = [fileHandle readDataOfLength:readLength];
            NSLog(@"data:%@, data string:%@", data, [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding]);
            [data writeToFile:partFilePath atomically:YES];
        } else {  
            NSLog(@"invalid read position of file");  
        }     
        [fileHandle closeFile];  
    } else {  
        NSLog(@"failed to open file");  
    }
    return partFilePath;
}

void UploadProxy::SetMultipartStreamFilePath(const FileSpec &file, OHMultipartFormStream *multipartStream, NSString *filePath)
{
    NSString *name = JsonUtils::CStringToNSString(!file.name.empty() ? file.name : "file"); // default: file
    NSString *fileName = JsonUtils::CStringToNSString(file.filename);
    NSURL *localPath = [NSURL fileURLWithPath:filePath];
    NSLog(@"upload name:%s, localPath:%s", name.UTF8String, [localPath description].UTF8String);
    [multipartStream appendWithFilePath:localPath
                                fileName:fileName
                                fieldName:name
                                mimeType:@"application/octet-stream"];
}

void UploadProxy::PartUpload(const string &method)
{
    NSLog(@"UploadProxy::PartUpload enter");
    if (config_.files.size() < 1) {
        return;
    }
    NSString *partFilePath = GetUploadPartFile(config_.files[0]);
    if (!partFilePath) {
        NSLog(@"PartUpload invalid part file path");
        return;
    }

    NSString *url = JsonUtils::CStringToNSString(config_.url);
    if (!url) {
        NSLog(@"PartUpload invalid url");
        return;
    }
    NSURL *baseUrl = [NSURL URLWithString:url];
    sessionCtrl_ = [[OHSessionManager alloc] initWithConfiguration:nil];
    
    NSMutableDictionary *headers = [[NSMutableDictionary alloc] init];
    for (auto it = config_.headers.begin(); it != config_.headers.end(); it++) {
        [headers setValue:JsonUtils::CStringToNSString(it->second)
                   forKey:JsonUtils::CStringToNSString(it->first)];
    }
    if ([url hasPrefix:@"https"]) {
        OHOS::Plugin::Request::CertificateUtils::InstallCertificateChain(sessionCtrl_);
        NSLog(@"is https upload");
    } else {
        NSLog(@"is http upload");
    }
    NSLog(@"PartUpload headers:%@", headers);
    NSMutableURLRequest *request = [OHMultipartFormStream requestWithURL:baseUrl 
                                                                  method:JsonUtils::CStringToNSString(method) 
                                                              parameters:headers
                                                     multipartFormStream:^(OHMultipartFormStream *multipartStream) {
        // name-value of form data
        for (const auto& formDt : config_.forms) {
            NSData *data = [NSData dataWithBytes:formDt.value.c_str() length:formDt.value.size()];
            [multipartStream appendWithFormData:data name:JsonUtils::CStringToNSString(formDt.name)];
        }

        NSLog(@"PartUpload partFilePath:%@", partFilePath);
        FileSpec file = config_.files[0];
        SetMultipartStreamFilePath(file, multipartStream, partFilePath);
        for (int i = 1; i < config_.files.size(); ++i) {
            FileSpec file = config_.files[i];
            SetMultipartStreamFilePath(file, multipartStream, JsonUtils::CStringToNSString(file.uri));
        }
    }];

    uploadTask_ = [sessionCtrl_ uploadWithStreamRequest:request
                                          progressBlock:^(NSProgress *progress) {
        NSLog(@"uploading, progress:%@", progress);
        OnProgressCallback(progress);
    } completion:^(NSURLResponse *response, id responseObject, NSError *error) {
        [[NSFileManager defaultManager] removeItemAtPath:partFilePath error:nil]; 
        PostCompletionHandler(response, error);
    }];
    [uploadTask_ resume];
}

void UploadProxy::PostUpload(const string &method)
{
    NSLog(@"UploadProxy::PostUpload enter");
    if (config_.ends > 0) {
        PartUpload(method);
        return;
    }

    NSString *url = JsonUtils::CStringToNSString(config_.url);
    if (!url) {
        NSLog(@"PostUpload invalid url");
        return;
    }
    NSURL *baseUrl = [NSURL URLWithString:url];
    sessionCtrl_ = [[OHSessionManager alloc] initWithConfiguration:nil];
    
    NSMutableDictionary *headers = [[NSMutableDictionary alloc] init];
    for (auto it = config_.headers.begin(); it != config_.headers.end(); it++) {
        [headers setValue:JsonUtils::CStringToNSString(it->second)
                   forKey:JsonUtils::CStringToNSString(it->first)];
    }
    if ([url hasPrefix:@"https"]) {
        OHOS::Plugin::Request::CertificateUtils::InstallCertificateChain(sessionCtrl_);
        NSLog(@"is https upload");
    } else {
        NSLog(@"is http upload");
    }
    NSLog(@"PostUpload headers:%@", headers);
    NSMutableURLRequest *request = [OHMultipartFormStream requestWithURL:baseUrl 
                                                                  method:JsonUtils::CStringToNSString(method) 
                                                              parameters:headers
                                                     multipartFormStream:^(OHMultipartFormStream * multipartStream) {
        // name-value of form data
        for (const auto& formDt : config_.forms) {
            NSData *data = [NSData dataWithBytes:formDt.value.c_str() length:formDt.value.size()];
            [multipartStream appendWithFormData:data name:JsonUtils::CStringToNSString(formDt.name)];
        }
        for (const auto& file : config_.files) {
            NSString *filePath = JsonUtils::CStringToNSString(file.uri);
            if (!filePath) {
                NSLog(@"PostUpload invalid file path");
                continue;
            }
            NSLog(@"PostUpload filePath:%@", filePath);
            NSString *fieldName = JsonUtils::CStringToNSString(!file.name.empty() ? file.name : "file"); // default: file
            NSString *fileName = JsonUtils::CStringToNSString(file.filename);
            NSURL *localPath = [NSURL fileURLWithPath:filePath];
            NSLog(@"upload fieldName:%s, localPath:%s", fieldName.UTF8String, [localPath description].UTF8String);
            [multipartStream appendWithFilePath:localPath
                                       fileName:fileName
                                      fieldName:fieldName
                                       mimeType:@"application/octet-stream"];
        }
    }];

    uploadTask_ = [sessionCtrl_ uploadWithStreamRequest:request
                                          progressBlock:^(NSProgress *progress) {
        NSLog(@"uploading, progress:%@", progress);
        OnProgressCallback(progress);
    } completion:^(NSURLResponse *response, id responseObject, NSError *error) {
        PostCompletionHandler(response, error);
    }];
    [uploadTask_ resume];
    NSLog(@"upload task resume");
}

void UploadProxy::PostCompletionHandler(NSURLResponse *response, NSError *error)
{
    NSLog(@"UploadProxy::PostCompletionHandler enter");
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

    GetExtras(response);

    if (error != nil) {
        NSLog(@"upload failed. Error: %s", [error description].UTF8String);
        OnFailedCallback();
    } else {
        NSLog(@"upload completed. response:%s", [response description].UTF8String);
        OnCompletedCallback();
    }
}

void UploadProxy::InitTaskInfo(const Config &config, TaskInfo &info)
{
    NSLog(@"UploadProxy::InitTaskInfo enter");
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
    info.extras = config.extras;
    info.progress.index = config.index;
    if (info.progress.index == 0) {
        info.progress.index = 1;
    }
    info.progress.processed = 0;
    info.progress.totalProcessed = 0;
    for (auto &file : config.files) {
        TaskState state;
        state.path = file.filename;
        state.responseCode = E_OK;
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

int64_t UploadProxy::GetTotalFileSize() const
{
    int64_t totalFileSize = 0;
    for (auto &size : info_.progress.sizes) {
        totalFileSize += size;
    }
    return totalFileSize;
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
        callback_(taskId_, EVENT_PROGRESS, JsonUtils::TaskInfoToJsonString(info_));
        callback_(taskId_, EVENT_COMPLETED, JsonUtils::TaskInfoToJsonString(info_));
    }
}

void UploadProxy::OnProgressCallback(NSProgress *progress)
{
    NSLog(@"upload OnProgressCallback");
    if (progress == nil || callback_ == nullptr) {
        return;
    }
    info_.progress.processed = progress.completedUnitCount;
    info_.progress.state = State::RUNNING;
    info_.progress.totalProcessed = GetTotalFileSize();
    if (progress.fractionCompleted == 1.0) {
        info_.progress.state = State::COMPLETED;
        if (info_.progress.processed != GetTotalFileSize()) {
            info_.progress.processed = GetTotalFileSize();
        }
    }
    IosTaskDao::UpdateDB(info_, config_);

    int64_t now = RequestUtils::GetTimeNow();
    if (now - currentTime_ >= REPORT_INFO_INTERVAL) {
        callback_(taskId_, EVENT_PROGRESS, JsonUtils::TaskInfoToJsonString(info_));
        currentTime_ = now;
    }
}

void UploadProxy::OnCompletedCallback()
{
    NSLog(@"upload OnCompletedCallback");
    ChangeState(State::COMPLETED);
    IosTaskDao::UpdateDB(info_, config_);
}

void UploadProxy::OnFailedCallback()
{
    NSLog(@"upload OnFailedCallback");
    ChangeState(State::FAILED);
    IosTaskDao::UpdateDB(info_, config_);
}

void UploadProxy::GetExtras(NSURLResponse *response)
{
    NSLog(@"upload GetExtras");
    NSDictionary *allHeader = [(NSHTTPURLResponse *)response allHeaderFields];
    for (NSString *key in allHeader.allKeys) {
        NSString *value = [allHeader objectForKey:key];
        info_.progress.extras.emplace(key.UTF8String, value.UTF8String);
    }
}

} // end of OHOS::Plugin::Request
