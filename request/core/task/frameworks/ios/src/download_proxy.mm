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
#include <thread>
#import  <UIKit/UIKit.h>
#include "log.h"
#import "ios_certificate_utils.h"
#import "json_utils.h"

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
    REQUEST_HILOGI("DownloadProxy allocated");
}

DownloadProxy::~DownloadProxy()
{
    sessionCtrl_ = nil;
    downloadTask_ = nil;
    resumeData_ = nil;
}

int32_t DownloadProxy::Start(int64_t taskId)
{
    REQUEST_HILOGI("Start, taskId:%{public}lld", taskId);
    taskId_ = taskId;
    IosTaskDao::QueryTaskInfo(taskId, "", info_);
    GetFileSize(config_.url);

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        REQUEST_HILOGI("Start download");
        NSString *urlStr = [NSString stringWithUTF8String:config_.url.c_str()];
        NSURL *url = [NSURL URLWithString:urlStr];
        NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];
        sessionCtrl_ = [[OHSessionManager alloc] initWithConfiguration:nil];
        if ([url.scheme compare:@"https"] == NSOrderedSame) {
            OHOS::Plugin::Request::CertificateUtils::InstallCertificateChain(sessionCtrl_);
        }
        auto headerMap = config_.headers;
        for (const auto &header: headerMap) {
            NSString *key = [NSString stringWithUTF8String:header.first.c_str()];
            NSString *value = [NSString stringWithUTF8String:header.second.c_str()];
            [request setValue:value forHTTPHeaderField:key];
        }

        downloadTask_ = [sessionCtrl_ downloadWithRequest:request
                                            progressBlock:^(NSProgress *downloadProgress) {
            OnProgressCallback(downloadProgress);
            if (!isMimeReported_) {
                ReportMimeType([downloadTask_ response]);
            }
        } destination:^NSURL * _Nullable(NSURLResponse * response, NSURL * temporaryURL) {
            NSString *filePath = [NSString stringWithUTF8String:config_.saveas.c_str()];
            NSURL *destPath = [NSURL fileURLWithPath:filePath];
            return destPath;
        } completion:^(NSURLResponse *response, NSURL *filePath, NSError *error) {
            REQUEST_HILOGI("downloadTask CompletionHandler start");           
            CompletionHandler(response, filePath, error);
        }];
        [downloadTask_ resume];
        REQUEST_HILOGI("downloadTask resume");
    });
    return E_OK;
}

int32_t DownloadProxy::Pause(int64_t taskId)
{
    REQUEST_HILOGI("Pause download, taskId:%{public}lld", taskId);
    // isSuspendByNetwork_ = isSuspendByNetwork;
    if (downloadTask_.state == NSURLSessionTaskStateRunning) {
        [downloadTask_ cancelByProducingResumeData:^(NSData *resumeData) {
            if (resumeData != nil) {
                resumeData_ = resumeData;
            }
        }];
    }
    return E_OK;
}

int32_t DownloadProxy::Resume(int64_t taskId)
{
    REQUEST_HILOGI("Restore download");
    if (resumeData_ == nil) {
        REQUEST_HILOGI("restore download, because resumeData is nil, download from begin.");
        return Start(taskId);
    }
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{

        [sessionCtrl_ downloadTaskWithResumeData:resumeData_
                                   progressBlock:^(NSProgress *downloadProgress) {
            OnProgressCallback(downloadProgress);
        } destination:^NSURL * _Nullable(NSURLResponse *response, NSURL *temporaryURL) {
            NSString *filePath = [NSString stringWithUTF8String:config_.saveas.c_str()];
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
    REQUEST_HILOGI("Stop enter");
    if (downloadTask_ != nil) {
        [downloadTask_ cancel];
    }
    return E_OK;
}

void DownloadProxy::PushNotification(BOOL isFailed)
{
    REQUEST_HILOGI("Push Notification");
    NSString *file = [NSString stringWithUTF8String:GetFileName(config_.url).c_str()];
    NSValue *result = [NSValue value:&isFailed withObjCType:@encode(BOOL)];
    PushNotification(file, result);
}

void DownloadProxy::CompletionHandler(NSURLResponse *response, NSURL *filePath, NSError *error)
{
    
    if (!isMimeReported_) {
        ReportMimeType(response);
    }
    if (error == nil) {
        REQUEST_HILOGI("success download file to: %{public}s", [[filePath description] UTF8String]);
        OnCompletedCallback();
    } else {
        REQUEST_HILOGI("failed download file to: %{public}s, error:%{public}s, error_code:%{public}ld",
            [[filePath description] UTF8String], [[error description] UTF8String], error.code);
        if (callback_ != nullptr) {
            if (error.code != -1011) { // NSURLErrorBadServerResponse
                isSuspendByNetwork_ = false;
            }
            callback_(taskId_, EVENT_FAILED, JsonUtils::TaskInfoToJsonString(info_));
        }

        // check can be resume?
        NSData *resumeData = [[error userInfo] objectForKey:NSURLSessionDownloadTaskResumeData];
        if (resumeData != nil) {
            REQUEST_HILOGI("Download failed, but has resumeData");
            resumeData_ = resumeData;
        }
    }

    REQUEST_HILOGI("isBackground: %{public}d", config_.background);
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
    NSString *mimeType = [response MIMEType];
    if ([mimeType length] == 0) {
        NSHTTPURLResponse *resp = (NSHTTPURLResponse *)response;
        mimeType = [resp valueForHTTPHeaderField:@"Content-Type"];
    }
    REQUEST_HILOGI("download, response:%{public}s, mimeType:%{public}s", [[response description] UTF8String], [mimeType UTF8String]);
    if ([mimeType length] > 0 && callback_ != nullptr) {
        // callback->SetMimeType([mimeType UTF8String]);
    }
}

void DownloadProxy::GetFileSize(const string &fileUrl)
{
    NSString *urlString = [NSString stringWithUTF8String:fileUrl.c_str()];
    NSURL *url = [NSURL URLWithString:urlString];
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];
    [request setHTTPMethod:@"HEAD"];

    NSURLResponse *response;
    NSError *error;
    [NSURLConnection sendSynchronousRequest:request returningResponse:&response error:&error];

    if (error) {
        REQUEST_HILOGE("Failed to get file size");
        return;
    }
    if ([response isKindOfClass:[NSHTTPURLResponse class]]) {
        NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
        NSDictionary *headers = [httpResponse allHeaderFields];
        NSString *contentLength = [headers objectForKey:@"Content-Length"];
        int64_t fileSize = [contentLength integerValue];
        REQUEST_HILOGE("download file size is %{public}lld", fileSize);
        info_.progress.sizes.emplace_back(fileSize);
    }
}

void DownloadProxy::OnCompletedCallback()
{
    if (callback_ != nullptr) {
        info_.progress.state = State::COMPLETED;
        callback_(taskId_, EVENT_COMPLETED, JsonUtils::TaskInfoToJsonString(info_));
        IosTaskDao::UpdateDB(info_, config_);
    }
}

void DownloadProxy::OnProgressCallback(NSProgress *downloadProgress)
{
    if (callback_ != nullptr && downloadProgress != nil) {
        info_.progress.state = State::RUNNING;
        info_.progress.processed = downloadProgress.completedUnitCount;
        info_.progress.totalProcessed = downloadProgress.totalUnitCount;
        callback_(taskId_, EVENT_PROGRESS, JsonUtils::TaskInfoToJsonString(info_));
        IosTaskDao::UpdateDB(info_, config_);
    }
}

} // namespace OHOS::Plugin::Request