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
#import "ios_certificate_utils.h"
#import "IosTaskDao.h"
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
    : taskId_(taskId), config_(config), callback_(callback)
{
    NSLog(@"UploadProxy allocated, taskId:%lld", taskId);
    InitTaskInfo(config_, info_);
}

UploadProxy::~UploadProxy()
{
    NSLog(@"UploadProxy freed, taskId:%lld", taskId_);
    for (auto &task : uploadTaskList_) {
        if (task != nil) {
            [task cancel];
            task = nil;
        }
    }
    uploadTaskList_.clear();
    sessionCtrl_ = nil;
}

int32_t UploadProxy::Start(int64_t taskId)
{
    NSLog(@"UploadProxy::Start, taskId:%lld", taskId);
    @autoreleasepool {
        string method = !config_.method.empty() ? config_.method : "POST"; // default: POST
        NSLog(@"upload method:%s", method.c_str());
        if (method == "POST") {
            PostUpload();
        } else if (method == "PUT") {
            PutUpload();
        } else {
            NSLog(@"unsupport method: %s, accept: POST or PUT", method.c_str());
        }
    }
    return E_OK;
}

int32_t UploadProxy::Pause(int64_t taskId)
{
    NSLog(@"UploadProxy::Pause taskId:%lld", taskId);
    bool isPause = false;
    for (auto &task : uploadTaskList_) {
        if (task == nil) {
            continue;
        }
        if (task.state == NSURLSessionTaskStateRunning) {
            isPause = true;
            [task suspend];
        }
    }
    if (isPause) {
        ChangeState(State::PAUSED);
    }
    return E_OK;
}

int32_t UploadProxy::Resume(int64_t taskId)
{
    NSLog(@"UploadProxy::Resume taskId:%lld", taskId);
    bool isResume = false;
    for (auto &task : uploadTaskList_) {
        if (task == nil) {
            continue;
        }
        if (task.state == NSURLSessionTaskStateSuspended) {
            isResume = true;
            [task resume];
        }
    }
    if (isResume) {
        ChangeState(State::RUNNING);
    }
    return E_OK;
}

int32_t UploadProxy::Stop(int64_t taskId)
{
    NSLog(@"UploadProxy::Stop, taskId:%lld", taskId);
    for (auto &task : uploadTaskList_) {
        if (task != nil) {
            [task cancel];
        }
    }
    ChangeState(State::STOPPED);
    isStopped_ = true;
    return E_OK;
}

void UploadProxy::PartPutUpload(NSURL *baseUrl, NSMutableDictionary *headers)
{
    NSLog(@"UploadProxy::PartPutUpload enter, config.index:%d", config_.index);
    uint32_t index = config_.index;
    if (config_.files.empty() || index >= config_.files.size()) {
        return;
    }
    NSString *partFilePath = GetUploadPartFile(config_.files[index]);
    if (!partFilePath) {
        NSLog(@"PartPutUpload invalid part file path");
        return;
    }

    for (int32_t i = 0; i < config_.files.size(); ++i) {
        if (i == index) {
            PartPutUploadFile(config_.files[i], i, partFilePath, baseUrl, headers);
        } else {
            PutUploadFile(config_.files[i], i, baseUrl, headers);
        }
        fileCount_++;
    }
}

void UploadProxy::PartPutUploadFile(const FileSpec &file, int32_t index, NSString *partFilePath, NSURL *baseUrl, NSMutableDictionary *headers)
{
    NSLog(@"UploadProxy::PartPutUploadFile enter, index:%d", index);
    NSString *fileName = JsonUtils::CStringToNSString(file.filename);
    NSURL *localPath = [NSURL fileURLWithPath:partFilePath];
    NSLog(@"PutUploadFile upload localPath:%s", [localPath description].UTF8String);
    NSURL *dstUrl = [baseUrl URLByAppendingPathComponent:fileName];
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:dstUrl];
    request.HTTPMethod = @"PUT";
    [request setAllHTTPHeaderFields:headers];

    NSURLSessionUploadTask *task = [sessionCtrl_ uploadWithRequest:request
                                                            fromFile:localPath
                                                        progressBlock:^(NSProgress *progress) {
        NSLog(@"PutUploadFile uploading, progress:%@", progress);
        OnProgressCallback(progress, index);
    } completion:^(NSURLResponse *response, id responseObject, NSError *error) {
        CompletionHandler(response, responseObject, error);
        RemoveFile(partFilePath);
    }];
    [task resume];
    uploadTaskList_.push_back(task);
}

void UploadProxy::PutUploadFile(const FileSpec &file, int32_t index, NSURL *baseUrl, NSMutableDictionary *headers)
{
    NSString *filePath = JsonUtils::CStringToNSString(file.uri);
    NSString *fileName = JsonUtils::CStringToNSString(file.filename);
    NSURL *localPath = [NSURL fileURLWithPath:filePath];
    NSLog(@"PutUploadFile upload localPath:%s", [localPath description].UTF8String);
    NSURL *dstUrl = [baseUrl URLByAppendingPathComponent:fileName];
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:dstUrl];
    request.HTTPMethod = @"PUT";
    [request setAllHTTPHeaderFields:headers];

    NSURLSessionUploadTask *task = [sessionCtrl_ uploadWithRequest:request
                                                            fromFile:localPath
                                                        progressBlock:^(NSProgress *progress) {
        NSLog(@"PutUploadFile uploading, progress:%@", progress);
        OnProgressCallback(progress, index);
    } completion:^(NSURLResponse *response, id responseObject, NSError *error) {
        CompletionHandler(response, responseObject, error);
    }];
    [task resume];
    uploadTaskList_.push_back(task);
}

void UploadProxy::PutUpload()
{
    NSLog(@"UploadProxy::PutUpload enter");
    NSString *url = JsonUtils::CStringToNSString(config_.url);
    if (!url) {
        NSLog(@"PutUpload invalid url");
        return;
    }
    NSURL *baseUrl = [NSURL URLWithString:url];
    NSURLSessionConfiguration *sessionConfig = [NSURLSessionConfiguration defaultSessionConfiguration];
    if (!config_.proxy.empty()) {
        NSString *proxyStr = JsonUtils::CStringToNSString(config_.proxy);
        NSURL *proxyUrl = [NSURL URLWithString:proxyStr];
        if (proxyUrl && proxyUrl.host && proxyUrl.port) {
            sessionConfig.requestCachePolicy = NSURLRequestReloadIgnoringLocalCacheData;
            sessionConfig.connectionProxyDictionary = @{
                @"HTTPProxy": proxyUrl.host,
                @"HTTPPort": proxyUrl.port,
                @"HTTPSProxy": proxyUrl.host,
                @"HTTPSPort": proxyUrl.port
            };
            NSLog(@"Proxy set: %@", proxyStr);
        } else {
            NSLog(@"Invalid proxy address: %@", proxyStr);
        }
    } else {
        NSLog(@"Proxy empty");
    }
    sessionCtrl_ = [[OHSessionManager alloc] initWithConfiguration:sessionConfig];

    NSMutableDictionary *headers = [[NSMutableDictionary alloc] init];
    [headers setValue:@"application/octet-stream" forKey:@"Content-Type"];

    for (auto it = config_.headers.begin(); it != config_.headers.end(); it++) {
        [headers setValue:JsonUtils::CStringToNSString(it->first)
            forKey:JsonUtils::CStringToNSString(it->second)];
    }
    if ([url hasPrefix:@"https"]) {
        OHOS::Plugin::Request::CertificateUtils::InstallCertificateChain(sessionCtrl_);
        NSLog(@"PutUpload it is https upload");
    } else {
        NSLog(@"PutUpload it is http upload");
    }

    if (config_.ends > 0) {
        PartPutUpload(baseUrl, headers);
        return;
    }
    for (int32_t i = 0; i < config_.files.size(); ++i) {
        PutUploadFile(config_.files[i], i, baseUrl, headers);
        fileCount_++;
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
        NSUInteger readPosition = 0;
        NSUInteger readLength = 0;
        int64_t beginPos = config_.begins;
        int64_t endPos = config_.ends;
        if (config_.begins >= fileLength) {
            beginPos = 0;
        }
        if (config_.ends >= fileLength) {
            endPos = -1;
        }
        if (config_.begins > config_.ends && config_.ends >= 0) {
            beginPos = 0;
            endPos = -1;
        }
        if (endPos == -1) {
            readLength = fileLength - beginPos;
            readPosition = beginPos;
        } else {
            readLength = endPos - beginPos + 1;
            readPosition = beginPos;
        }
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

void UploadProxy::PartPostUpload(NSURL *baseUrl, NSMutableDictionary *headers)
{
    NSLog(@"UploadProxy::PartPostUpload enter, config.index:%d", config_.index);
    uint32_t index = config_.index;
    if (config_.files.empty() || index >= config_.files.size()) {
        return;
    }
    NSString *partFilePath = GetUploadPartFile(config_.files[index]);
    if (!partFilePath) {
        NSLog(@"PartPostUpload invalid part file path");
        return;
    }

    for (int32_t i = 0; i < config_.files.size(); ++i) {
        if (i == index) {
            PartPostUploadFile(config_.files[i], i, partFilePath, baseUrl, headers);
        } else {
            PostUploadFile(config_.files[i], i, baseUrl, headers);
        }
        fileCount_++;
    }
}

void UploadProxy::PartPostUploadFile(const FileSpec &file, int32_t index, NSString *partFilePath, NSURL *baseUrl, NSMutableDictionary *headers)
{
     NSMutableURLRequest *request = [OHMultipartFormStream requestWithURL:baseUrl
                                                                    method:@"POST"
                                                                parameters:headers
                                                        multipartFormStream:^(OHMultipartFormStream * multipartStream) {
        // name-value of form data
        for (const auto &formDt : config_.forms) {
            NSData *data = [NSData dataWithBytes:formDt.value.c_str() length:formDt.value.size()];
            [multipartStream appendWithFormData:data name:JsonUtils::CStringToNSString(formDt.name)];
        }
        NSLog(@"PartPostUploadFile partFilePath:%@", partFilePath);
        NSString *fieldName = JsonUtils::CStringToNSString(!file.name.empty() ? file.name : "file"); // default: file
        NSString *fileName = JsonUtils::CStringToNSString(file.filename);
        NSURL *localPath = [NSURL fileURLWithPath:partFilePath];
        NSLog(@"PartPostUploadFile upload fieldName:%s, localPath:%s", fieldName.UTF8String, [localPath description].UTF8String);
        [multipartStream appendWithFilePath:localPath
                                fileName:fileName
                                fieldName:fieldName
                                mimeType:@"application/octet-stream"];
    }];

    NSURLSessionUploadTask *task = [sessionCtrl_ uploadWithStreamRequest:request
                                                           progressBlock:^(NSProgress *progress) {
        NSLog(@"PartPostUploadFile uploading, progress:%@", progress);
        OnProgressCallback(progress, index);
    } completion:^(NSURLResponse *response, id responseObject, NSError *error) {
        CompletionHandler(response, responseObject, error);
        RemoveFile(partFilePath);
    }];
    [task resume];
    uploadTaskList_.emplace_back(task);
    NSLog(@"PartPostUploadFile upload task resume");
}

void UploadProxy::PostUpload()
{
    NSLog(@"UploadProxy::PostUpload");
    NSString *url = JsonUtils::CStringToNSString(config_.url);
    if (!url) {
        NSLog(@"PostUpload invalid url");
        return;
    }
    NSURL *baseUrl = [NSURL URLWithString:url];
    NSURLSessionConfiguration *sessionConfig = [NSURLSessionConfiguration defaultSessionConfiguration];
    if (!config_.proxy.empty()) {
        NSString *proxyStr = JsonUtils::CStringToNSString(config_.proxy);
        NSURL *proxyUrl = [NSURL URLWithString:proxyStr];
        if (proxyUrl && proxyUrl.host && proxyUrl.port) {
            sessionConfig.requestCachePolicy = NSURLRequestReloadIgnoringLocalCacheData;
            sessionConfig.connectionProxyDictionary = @{
                @"HTTPProxy": proxyUrl.host,
                @"HTTPPort": proxyUrl.port,
                @"HTTPSProxy": proxyUrl.host,
                @"HTTPSPort": proxyUrl.port
            };
            NSLog(@"Proxy set: %@", proxyStr);
        } else {
            NSLog(@"Invalid proxy address: %@", proxyStr);
        }
    } else {
        NSLog(@"Proxy empty");
    }
    sessionCtrl_ = [[OHSessionManager alloc] initWithConfiguration:sessionConfig];

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

    if (config_.ends > 0) {
        PartPostUpload(baseUrl, headers);
        return;
    }
    for (int32_t i = 0; i < config_.files.size(); ++i) {
        PostUploadFile(config_.files[i], i, baseUrl, headers);
        fileCount_++;
    }
}

void UploadProxy::PostUploadFile(const FileSpec &file, int32_t index, NSURL *baseUrl, NSMutableDictionary *headers)
{
    NSLog(@"UploadProxy::PostUploadFile()");
    NSMutableURLRequest *request = [OHMultipartFormStream requestWithURL:baseUrl
                                                                    method:@"POST"
                                                                parameters:headers
                                                        multipartFormStream:^(OHMultipartFormStream *multipartStream) {
        // name-value of form data
        for (const auto& formDt : config_.forms) {
            NSData *data = [NSData dataWithBytes:formDt.value.c_str() length:formDt.value.size()];
            [multipartStream appendWithFormData:data name:JsonUtils::CStringToNSString(formDt.name)];
        }
        NSString *filePath = JsonUtils::CStringToNSString(file.uri);
        NSLog(@"PostUploadFile filePath:%@", filePath);
        NSString *fieldName = JsonUtils::CStringToNSString(!file.name.empty() ? file.name : "file"); // default: file
        NSString *fileName = JsonUtils::CStringToNSString(file.filename);
        NSURL *localPath = [NSURL fileURLWithPath:filePath];
        NSLog(@"PostUploadFile upload fieldName:%s, localPath:%s", fieldName.UTF8String, [localPath description].UTF8String);
        if (config_.begins > 0 && config_.ends == -1 && index == config_.index) {
            NSString *cachesDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) objectAtIndex:0];
            NSString *partFilePath = [cachesDir stringByAppendingString:@"/temp.data"];
            NSFileHandle *fileHandle = [NSFileHandle fileHandleForReadingAtPath:filePath];
            if (fileHandle) {
                NSUInteger fileLength = [fileHandle seekToEndOfFile];
                NSUInteger readPosition = config_.begins;
                NSUInteger readLength = fileLength - config_.begins;
                if (readPosition + readLength <= fileLength) {
                    [fileHandle seekToFileOffset:readPosition];
                    NSData *data = [fileHandle readDataOfLength:readLength];
                    [data writeToFile:partFilePath atomically:YES];
                }
                [fileHandle closeFile];
            }
            localPath = [NSURL fileURLWithPath:partFilePath];
        }
        [multipartStream appendWithFilePath:localPath
                                fileName:fileName
                                fieldName:fieldName
                                mimeType:@"application/octet-stream"];
    }];

    NSURLSessionUploadTask *task = [sessionCtrl_ uploadWithStreamRequest:request
                                                           progressBlock:^(NSProgress *progress) {
        NSLog(@"PostUploadFile uploading, progress:%@", progress);
        OnProgressCallback(progress, index);
    } completion:^(NSURLResponse *response, id responseObject, NSError *error) {
        CompletionHandler(response, responseObject, error);
    }];
    [task resume];
    uploadTaskList_.emplace_back(task);
    NSLog(@"PostUpload upload task resume");
}

void UploadProxy::CompletionHandler(NSURLResponse *response, id responseObject, NSError *error)
{
    NSLog(@"UploadProxy::CompletionHandler, response:%@", [response description]);
    respCount_++;

    vector<TaskState> taskStateList;
    for (const auto& file: config_.files) {
        TaskState taskState;
        taskState.path = file.filename;
        taskState.responseCode = (error != nil) ? E_SERVICE_ERROR : E_OK;
        taskState.message = GetCodeMessage(taskState.responseCode);
        taskStateList.push_back(taskState);
    }
    GetExtras(response);
    OnHeaderReceiveCallback(response,responseObject);
    OnResponseCallback(response);
    if ([response isKindOfClass:[NSHTTPURLResponse class]]) {
        NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
        NSLog(@"UploadProxy::CompletionHandler, statusCode:%d", httpResponse.statusCode);
        if (httpResponse.statusCode == 206) {
            ChangeState(State::FAILED);
            for (auto &task : uploadTaskList_) {
                if (task != nil) {
                    [task cancel];
                }
            }
            return;
        }
    }
    if (error != nil && info_.progress.state != State::PAUSED) {
        NSLog(@"upload failed, error: %@", [error description]);
        ChangeState(State::FAILED);
        for (auto &task : uploadTaskList_) {
            if (task != nil) {
                [task cancel];
            }
        }
    } else {
         // all responsed
        if (respCount_ == fileCount_) {
            if (error != nil && info_.progress.state != State::PAUSED) {
                NSLog(@"upload failed, error: %@", [error description]);
                ChangeState(State::FAILED);
            } else {
                NSLog(@"upload completed. response:%@", [response description]);
                ChangeState(State::COMPLETED);
            }
        }
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
    info.progress.index = 0;
    info.progress.processed = 0;
    info.progress.totalProcessed = 0;
    for(auto i = 0; i < config.files.size(); i++) {
        auto file = config.files[i];
        TaskState state;
        state.path = file.filename;
        state.responseCode = E_OK;
        if (file.uri.empty()) {
            state.responseCode = E_FILE_PATH;
        }
        if (file.filename.empty()) {
            state.responseCode = E_FILE_IO;
        }
        if (file.fd > 0) {
            const int64_t fileTotalSize = lseek(file.fd, 0, SEEK_END);
            int64_t fileSize = fileTotalSize;
            int64_t offset = 0;
            if (i == config.index) {
                int64_t beginPos = config.begins;
                int64_t endPos = config.ends;
                if (config.begins >= fileTotalSize) {
                    beginPos = 0;
                }
                if (config.ends >= fileTotalSize) {
                    endPos = -1;
                }
                if (config.begins > config.ends && config.ends >= 0) {
                    beginPos = 0;
                    endPos = -1;
                }
                if (endPos == -1) {
                    fileSize = fileTotalSize - beginPos;
                    offset = beginPos;
                } else {
                    fileSize = endPos - beginPos + 1;
                    offset = beginPos;
                }
            } else {
                NSLog(@"Full upload[%d]: size=%lld", i, fileSize);
            }
            info.progress.sizes.emplace_back(fileSize);
            lseek(file.fd, offset, SEEK_SET);
        } else {
            info.progress.sizes.emplace_back(0);
            if (state.responseCode == E_OK) {
                state.responseCode = E_FILE_IO;
            }
        }
        state.message = GetCodeMessage(state.responseCode);
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
    NSLog(@"UploadProxy::ChangeState, taskId:%lld, state:%d", taskId_, state);
    info_.progress.state = state;
    info_.mtime = RequestUtils::GetTimeNow();
    if (callback_ == nullptr) {
        return;
    }
    switch (state) {
        case State::FAILED: {
            callback_(taskId_, EVENT_FAILED, JsonUtils::TaskInfoToJsonString(info_));
            break;
        }
        case State::COMPLETED: {
            callback_(taskId_, EVENT_PROGRESS, JsonUtils::TaskInfoToJsonString(info_));
            callback_(taskId_, EVENT_COMPLETED, JsonUtils::TaskInfoToJsonString(info_));
            break;
        }
        case State::PAUSED: {
            callback_(taskId_, EVENT_PAUSE, JsonUtils::TaskInfoToJsonString(info_));
            IosTaskDao::UpdateDB(info_);
            break;
        }
        case State::RUNNING: {
            callback_(taskId_, EVENT_RESUME, JsonUtils::TaskInfoToJsonString(info_));
            IosTaskDao::UpdateDB(info_);
            break;
        }
        default: {
            NSLog(@"upload ChangeState, Other state");
            break;
        }
    }
}

NSString* UploadProxy::GetStandardHTTPReason(NSInteger statusCode) {
    static NSDictionary<NSNumber *, NSString *> *reasonPhrases;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        reasonPhrases = @{
            @100: @"Continue",
            @101: @"Switching Protocols",
            @200: @"OK",
            @201: @"Created",
            @202: @"Accepted",
            @203: @"Non-Authoritative Information",
            @204: @"No Content",
            @205: @"Reset Content",
            @206: @"Partial Content",
            @300: @"Multiple Choices",
            @301: @"Moved Permanently",
            @302: @"Found",
            @303: @"See Other",
            @304: @"Not Modified",
            @305: @"Use Proxy",
            @307: @"Temporary Redirect",
            @400: @"Bad Request",
            @401: @"Unauthorized",
            @402: @"Payment Required",
            @403: @"Forbidden",
            @404: @"Not Found",
            @405: @"Method Not Allowed",
            @406: @"Not Acceptable",
            @407: @"Proxy Authentication Required",
            @408: @"Request Timeout",
            @409: @"Conflict",
            @410: @"Gone",
            @411: @"Length Required",
            @412: @"Precondition Failed",
            @413: @"Payload Too Large",
            @414: @"URI Too Long",
            @415: @"Unsupported Media Type",
            @416: @"Range Not Satisfiable",
            @417: @"Expectation Failed",
            @426: @"Upgrade Required",
            @500: @"Internal Server Error",
            @501: @"Not Implemented",
            @502: @"Bad Gateway",
            @503: @"Service Unavailable",
            @504: @"Gateway Timeout",
            @505: @"HTTP Version Not Supported"
        };
    });

    return reasonPhrases[@(statusCode)] ?: [NSHTTPURLResponse localizedStringForStatusCode:statusCode];
}

void UploadProxy::OnHeaderReceiveCallback(NSURLResponse *response,id responseObject)
{
    if (callback_ == nullptr || response == nil || ![response isKindOfClass:[NSHTTPURLResponse class]]) {
        return;
    }
    NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
    NSString *responseBody = nil;
    NSStringEncoding encoding = NSUTF8StringEncoding;
    NSString *textEncodingName = httpResponse.textEncodingName;
    if (textEncodingName) {
        CFStringEncoding cfEncoding = CFStringConvertIANACharSetNameToEncoding((CFStringRef)textEncodingName);
        if (cfEncoding != kCFStringEncodingInvalidId) {
            encoding = CFStringConvertEncodingToNSStringEncoding(cfEncoding);
        }
    }
    NSData *bodyData = nil;
    if ([responseObject isKindOfClass:[NSData class]]) {
        bodyData = (NSData *)responseObject;
    } else if (responseObject != nil) {
        NSError *jsonError;
        bodyData = [NSJSONSerialization dataWithJSONObject:responseObject
                                                 options:0
                                                   error:&jsonError];
        if (jsonError) {
            NSLog(@"JSON serialization error: %@", jsonError);
        }
    }

    info_.progress.bodyBytes.clear();
    if (bodyData) {
        NSString *contentType = [httpResponse.allHeaderFields[@"Content-Type"] lowercaseString];
        if ([contentType containsString:@"image/"] ||
            [contentType containsString:@"application/octet-stream"]) {
            const uint8_t *bytes = static_cast<const uint8_t*>([bodyData bytes]);
            NSUInteger length = [bodyData length];
            info_.progress.bodyBytes.assign(bytes, bytes + length);
        } else {
            NSString *responseBody = [[NSString alloc] initWithData:bodyData encoding:encoding];
            if (responseBody) {
                std::string utf8String = [responseBody UTF8String];
                const uint8_t* rawData = reinterpret_cast<const uint8_t*>(utf8String.data());
                info_.progress.bodyBytes.assign(rawData, rawData + utf8String.size());
            }
        }
    }
    callback_(taskId_, EVENT_HEADERRECEIVE, JsonUtils::TaskInfoToJsonString(info_));
}

void UploadProxy::OnResponseCallback(NSURLResponse *response)
{
    if (callback_ == nullptr || response == nil) {
        return;
    }
     if ([response isKindOfClass:[NSHTTPURLResponse class]]) {
         NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
         NSInteger statusCode = httpResponse.statusCode;
         NSString *reason = GetStandardHTTPReason(statusCode);
         NSMutableDictionary<NSString *, NSArray<NSString *> *> *multiValueHeaders = [NSMutableDictionary dictionary];
         [httpResponse.allHeaderFields enumerateKeysAndObjectsUsingBlock:^(NSString *key, NSString *value, BOOL *stop) {
             if ([key.lowercaseString isEqualToString:@"set-cookie"]) {
                 multiValueHeaders[key] = @[value];
             } else {
                 NSArray<NSString *> *values = [value componentsSeparatedByString:@","];
                 multiValueHeaders[key] = values;
             }
         }];
         NSString *httpVersion = @"HTTP/1.1";
         NSString *description = [response description];
         NSRegularExpression *regex =
            [NSRegularExpression regularExpressionWithPattern:@"HTTP/(\\d\\.\\d) (\\d{3}) (.+?)\n" options:0 error:nil];
         NSTextCheckingResult *match =
            [regex firstMatchInString:description options:0 range:NSMakeRange(0, description.length)];
         if (match) {
             httpVersion = [description substringWithRange:match.range];
         }
         info_.response.version = httpVersion ? std::string([httpVersion UTF8String]) : "";
         info_.response.statusCode = httpResponse.statusCode;
         info_.response.reason = reason ? std::string([reason UTF8String]) : "";
         info_.response.headers.clear();
         for (NSString *ocKey in multiValueHeaders) {
            NSArray<NSString *> *ocValues = multiValueHeaders[ocKey];
            std::string cppKey = ocKey ? std::string([ocKey UTF8String]) : "";
            std::vector<std::string> cppValues;
            PushBackOcValues(ocValues, cppValues);
            info_.response.headers[cppKey] = cppValues;
         }
        callback_(taskId_, EVENT_RESPONSE, JsonUtils::TaskInfoToJsonString(info_));
     }
}

void UploadProxy::PushBackOcValues(NSArray<NSString *> *ocValues, std::vector<std::string> &cppValues)
{
    for (NSString *ocValue in ocValues) {
        if (ocValue) {
            cppValues.push_back(std::string([ocValue UTF8String]));
        }
    }
}

void UploadProxy::OnProgressCallback(NSProgress *progress, int32_t index)
{
    NSLog(@"upload OnProgressCallback, index:%d", index);
    if (progress == nil || callback_ == nullptr) {
        return;
    }
    info_.progress.index = index;
    info_.progress.processed = progress.completedUnitCount;
    info_.progress.totalProcessed = 0;
    for (int32_t i = 0; i < index - 1; ++i) {
        info_.progress.totalProcessed += info_.progress.sizes[i];
    }
    info_.progress.totalProcessed += progress.completedUnitCount;

    if (progress.fractionCompleted == 1.0) {
        info_.progress.state = State::COMPLETED;
        if (info_.progress.processed != GetTotalFileSize()) {
            info_.progress.processed = GetTotalFileSize();
        }
    }
    int64_t now = RequestUtils::GetTimeNow();
    if (now - currentTime_ >= REPORT_INFO_INTERVAL) {
        callback_(taskId_, EVENT_PROGRESS, JsonUtils::TaskInfoToJsonString(info_));
        currentTime_ = now;
        IosTaskDao::UpdateDB(info_);
    }
}

void UploadProxy::OnCompletedCallback()
{
    NSLog(@"upload OnCompletedCallback");
    info_.progress.index = info_.progress.sizes.size() - 1;
    ChangeState(State::COMPLETED);
    IosTaskDao::UpdateDB(info_);
}

void UploadProxy::OnFailedCallback()
{
    NSLog(@"upload OnFailedCallback");
    if (!isStopped_) {
        ChangeState(State::FAILED);
        IosTaskDao::UpdateDB(info_);
    }
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

void UploadProxy::RemoveFile(NSString *filePath)
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    BOOL fileExists = [fileManager fileExistsAtPath:filePath];
    if (fileExists) {
        [fileManager removeItemAtPath:filePath error:nil];
    }
}

} // end of OHOS::Plugin::Request
