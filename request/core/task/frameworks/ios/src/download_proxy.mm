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

#include "download_proxy.h"
#import  <UIKit/UIKit.h>
#import "ios_certificate_utils.h"
#import "json_utils.h"
#include "request_utils.h"

namespace OHOS::Plugin::Request {
using namespace std;
static string GetFileName(const string &fileName)
{
    size_t pos = fileName.rfind('/');
    if (pos != string::npos) {
        return fileName.substr(pos+1);
    }
    return fileName;
}

DownloadProxy::DownloadProxy(int64_t taskId, const Config &config, OnRequestCallback callback)
    : config_(config), callback_(callback)
{
    NSLog(@"DownloadProxy allocated, taskId:%lld", taskId);
    InitTaskInfo(config, info_);
    IosNetMonitor::SharedInstance()->AddObserver(this);
}

DownloadProxy::~DownloadProxy()
{
    NSLog(@"DownloadProxy freed, taskId:%lld", taskId_);
    IosNetMonitor::SharedInstance()->RemoveObserver(this);
    sessionCtrl_ = nil;
    downloadTask_ = nil;
    resumeData_ = nil;
}

void DownloadProxy::InitTaskInfo(const Config &config, TaskInfo &info)
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
        if (file.fd > 0) {
            int64_t fileSize = lseek(file.fd, 0, SEEK_END);
            if (config.begins > 0) {
                fileSize -= config.begins;
            }
            if (config.ends > 0) {
                fileSize = config.ends - config.begins;
            }
            SetSizes(fileSize);
            int64_t offset = 0;
            if (info.progress.index == config.index && config.begins > 0) {
                offset = config.begins;
            }
            lseek(file.fd, offset, SEEK_SET);
        }
        info.taskStates.emplace_back(state);
    }
}

bool DownloadProxy::CheckUrl(const string &strUrl)
{
    NSURL *url = [NSURL URLWithString:JsonUtils::CStringToNSString(strUrl)];
    NSURLRequest *request = [NSURLRequest requestWithURL:url];  
    NSURLResponse *response = nil;  
    NSError *error = nil;  
    [NSURLConnection sendSynchronousRequest:request returningResponse:&response error:&error];  
    if ([response isKindOfClass:[NSHTTPURLResponse class]]) {  
        NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;  
        return httpResponse.statusCode == 200;
    }
    return false;
}

int32_t DownloadProxy::Start(int64_t taskId)
{
    NSLog(@"start download, taskId:%lld, config_.saveas:%s", taskId, config_.saveas.c_str());
    taskId_ = taskId;
    IosTaskDao::QueryTaskInfo(taskId, "", info_);

    GetFileSize(config_.url);
    if (!CheckUrl(config_.url)) {
        NSLog(@"invalid download url:%s", config_.url.c_str());
        info_.progress.totalProcessed = -1;
        downloadTotalBytes_ = -1;
        OnFailedCallback();
        return E_OK;
    }

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSLog(@"start download");
        NSString *urlStr = JsonUtils::CStringToNSString(config_.url);
        NSURL *url = [NSURL URLWithString:urlStr];
        NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];
        sessionCtrl_ = [[OHSessionManager alloc] initWithConfiguration:nil];
        if ([url.scheme compare:@"https"] == NSOrderedSame) {
            OHOS::Plugin::Request::CertificateUtils::InstallCertificateChain(sessionCtrl_);
        }
        auto headerMap = config_.headers;
        for (const auto &header: headerMap) {
            NSString *key = JsonUtils::CStringToNSString(header.first);
            NSString *value = JsonUtils::CStringToNSString(header.second);
            [request setValue:value forHTTPHeaderField:key];
        }
        downloadTask_ = [sessionCtrl_ downloadWithRequest:request
                                            progressBlock:^(NSProgress *progress) {
            NSLog(@"downloading, progress:%@", progress);
            OnProgressCallback(progress);
            if (!isMimeReported_) {
                ReportMimeType([downloadTask_ response]);
            }
        } destination:^NSURL * _Nullable(NSURLResponse *response, NSURL *temporaryURL) {
            NSString *filePath = JsonUtils::CStringToNSString(config_.saveas);
            NSLog(@"download filePath:%@", filePath);
            NSURL *destPath = [NSURL fileURLWithPath:filePath];
            NSLog(@"download destPath:%@", destPath);
            return destPath;
        } completion:^(NSURLResponse *response, NSURL *filePath, NSError *error) {        
            CompletionHandler(response, filePath, error);
        }];
        [downloadTask_ resume];
        NSLog(@"downloadTask resume");
    });
    return E_OK;
}

int32_t DownloadProxy::Pause(int64_t taskId)
{
    NSLog(@"Pause download, taskId:%lld", taskId);
    if (downloadTask_.state == NSURLSessionTaskStateRunning) {
        [downloadTask_ cancelByProducingResumeData:^(NSData *resumeData) {
            if (resumeData != nil) {
                resumeData_ = resumeData;
            }
        }];
        OnPauseCallback();
    }
    return E_OK;
}

int32_t DownloadProxy::Resume(int64_t taskId)
{
    NSLog(@"Resume download, taskId:%lld", taskId);
    if (resumeData_ == nil) {
        NSLog(@"restore download, because resumeData is nil, download from begin.");
        return Start(taskId);
    }
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [sessionCtrl_ downloadTaskWithResumeData:resumeData_
                                   progressBlock:^(NSProgress *downloadProgress) {
            OnProgressCallback(downloadProgress);
        } destination:^NSURL * _Nullable(NSURLResponse *response, NSURL *temporaryURL) {
            NSString *filePath = JsonUtils::CStringToNSString(config_.saveas);
            NSURL *destPath = [NSURL URLWithString:filePath];
            return destPath;
        } completion:^(NSURLResponse *response, NSURL *filePath, NSError *error) {
            CompletionHandler(response, filePath, error);
        }];
        [downloadTask_ resume];
    });
    return E_OK;
}

int32_t DownloadProxy::Stop(int64_t taskId)
{
    NSLog(@"Stop download, taskId:%lld", taskId);
    if (downloadTask_ != nil) {
        [downloadTask_ cancel];
    }
    return E_OK;
}

void DownloadProxy::PushNotification(BOOL isFailed)
{
    NSLog(@"push notification");
    NSString *file = JsonUtils::CStringToNSString(GetFileName(config_.url));
    NSValue *result = [NSValue value:&isFailed withObjCType:@encode(BOOL)];
    PushNotification(file, result);
}

void DownloadProxy::CompletionHandler(NSURLResponse *response, NSURL *filePath, NSError *error)
{
    NSLog(@"CompletionHandler, filePath:%@", filePath);
    if (!isMimeReported_) {
        ReportMimeType(response);
    }
    if (error == nil) {
        NSLog(@"success download file to: %@", [filePath description]);
        OnCompletedCallback();
    } else {        
        NSLog(@"failed download file to: %s, error:%s, error_code:%ld",
            [filePath description].UTF8String, [error description].UTF8String, error.code);
        OnFailedCallback();

        // check can be resume?
        NSData *resumeData = [[error userInfo] objectForKey:NSURLSessionDownloadTaskResumeData];
        if (resumeData != nil) {
            NSLog(@"download failed, but has resumeData");
            resumeData_ = resumeData;
        }
    }

    NSLog(@"isBackground: %d", config_.background);
    // Allow download background task notifications.
    if (config_.background) {
        PushNotification(error != nil);
    }
}

void DownloadProxy::PushNotification(NSString *fileName, NSValue *result)
{
    if ([UIDevice currentDevice].systemVersion.floatValue >= 8.0) {
        UIUserNotificationSettings *settings = [UIUserNotificationSettings settingsForTypes:
            UIUserNotificationTypeBadge | UIUserNotificationTypeAlert | UIUserNotificationTypeSound
            categories:nil];
        [[UIApplication sharedApplication] registerUserNotificationSettings:settings];
    }

    BOOL isFailed = FALSE;
    [result getValue:&isFailed];
    NSString *res = !isFailed ? @"Completed." : @"Failed.";
    NSString *msg = [NSString stringWithFormat:@"Download %@ %@", fileName, res];
    UILocalNotification *notification = [[UILocalNotification alloc] init]; 
    notification.alertBody = msg;
    notification.alertAction = msg;
    notification.soundName = UILocalNotificationDefaultSoundName;
    [[UIApplication sharedApplication] presentLocalNotificationNow:notification];
}

void DownloadProxy::ReportMimeType(NSURLResponse *response)
{
    isMimeReported_ = true;
    NSString *mimeType = response.MIMEType;
    if (mimeType.length == 0) {
        mimeType = [(NSHTTPURLResponse *)response valueForHTTPHeaderField:@"Content-Type"];
    }
    NSLog(@"download, response:%@, mimeType:%@", response.description, mimeType);
    if (mimeType.length > 0) {
        info_.mimeType = mimeType.UTF8String;
        IosTaskDao::UpdateDB(info_, config_);
    }
}

void DownloadProxy::OnCompletedCallback()
{
    NSLog(@"download OnCompletedCallback");
    if (callback_ != nullptr) {
        info_.progress.state = State::COMPLETED;
        callback_(taskId_, EVENT_PROGRESS, JsonUtils::TaskInfoToJsonString(info_));
        callback_(taskId_, EVENT_COMPLETED, JsonUtils::TaskInfoToJsonString(info_));
        IosTaskDao::UpdateDB(info_, config_);
    }
}

void DownloadProxy::OnProgressCallback(NSProgress *progress)
{
    NSLog(@"download OnProgressCallback");
    if (callback_ == nullptr || progress == nil) {
        return;
    }
    info_.progress.state = State::RUNNING;
    info_.progress.processed = progress.completedUnitCount;
    info_.progress.totalProcessed = downloadTotalBytes_;
    if (progress.fractionCompleted == 1.0) {
        info_.progress.state = State::COMPLETED;
    }
    IosTaskDao::UpdateDB(info_, config_);

    int64_t now = RequestUtils::GetTimeNow();
    if (now - currentTime_ >= REPORT_INFO_INTERVAL) {
        callback_(taskId_, EVENT_PROGRESS, JsonUtils::TaskInfoToJsonString(info_));
        currentTime_ = now;
    }
}

void DownloadProxy::OnFailedCallback()
{
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 2.0 * NSEC_PER_SEC), dispatch_get_main_queue(), ^(void) {  
        if (callback_ != nullptr) {
            info_.progress.state = State::FAILED;
            info_.code = Reason::OTHERS_ERROR;
            SetSizes(downloadTotalBytes_);
            NSLog(@"download OnFailedCallback start");
            callback_(taskId_, EVENT_FAILED, JsonUtils::TaskInfoToJsonString(info_));
            IosTaskDao::UpdateDB(info_, config_);
        }
    });
}

void DownloadProxy::OnPauseCallback()
{
    NSLog(@"download OnPauseCallback");
    if (callback_ != nullptr) {
        info_.progress.state = State::PAUSED;
        SetSizes(downloadTotalBytes_);
        callback_(taskId_, EVENT_PAUSE, JsonUtils::TaskInfoToJsonString(info_));
        IosTaskDao::UpdateDB(info_, config_);
    }
}

void DownloadProxy::SetSizes(int64_t fileSize)
{
    NSLog(@"SetSizes fileSize:%lld", fileSize);
    info_.progress.sizes.clear();
    info_.progress.sizes.emplace_back(fileSize);
}

void DownloadProxy::GetFileSize(const string &downloadUrl)
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSURL *url = [NSURL URLWithString:JsonUtils::CStringToNSString(downloadUrl)];
        NSData *data = [NSData dataWithContentsOfURL:url];
        downloadTotalBytes_ = data.length;
        SetSizes(data.length);
        NSLog(@"download file size is %lld", data.length);
    });
}

// IosNetMonitorObserver
void DownloadProxy::NetworkStatusChanged(NetworkType netType)
{
    NSLog(@"DownloadProxy::NetworkStatusChanged, netType:%d", netType);
    if (netType == NETWORK_WIFI) {
        ReachableViaWiFi();
    } else if (netType == NETWORK_MOBILE) {
        ReachableViaWWAN();
    } else if (netType == NETWORK_INVALID) {
        NotReachable();
    }
}

void DownloadProxy::ReachableViaWiFi()
{
    NSLog(@"DownloadProxy::ReachableViaWiFi enter");
}

void DownloadProxy::ReachableViaWWAN()
{
    NSLog(@"DownloadProxy::ReachableViaWWAN enter");
}

void DownloadProxy::NotReachable()
{
    NSLog(@"DownloadProxy::NotReachable enter");
}

} // namespace OHOS::Plugin::Request