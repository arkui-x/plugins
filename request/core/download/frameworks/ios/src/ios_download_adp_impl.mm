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

#include "ios_download_adp_impl.h"
#include <stdio.h>
#include <thread>
#import <UIKit/UIKit.h>
#include "log.h"
#import "ios_certificate_utils.h"


namespace OHOS::Plugin::Request::Download {

std::shared_ptr<IosDownloadAdp> IosDownloadAdp::Instance()
{
    return std::make_shared<IosDownloadAdpImpl>();
}

bool IosDownloadAdp::IsDirectory(const std::string &path)
{
    NSURL *url = [NSURL URLWithString:[NSString stringWithUTF8String:path.c_str()]];
    NSFileWrapper *fileWrapper = [[NSFileWrapper alloc] initWithURL:url options:NSFileWrapperReadingImmediate|NSFileWrapperReadingWithoutMapping error:nil];
    return fileWrapper.isDirectory;
}

IosDownloadAdpImpl::IosDownloadAdpImpl()
{
}

IosDownloadAdpImpl::~IosDownloadAdpImpl()
{
    sessionCtrl_ = nil;
    downloadTask_ = nil;
    resumeData_ = nil;
}

void IosDownloadAdpImpl::Download(const DownloadConfig &config, IosDownloadAdpCallback *callback)
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        DOWNLOAD_HILOGD("Download Enter");
        config_ = config;
        NSUUID *uuid = [NSUUID UUID];
        NSURLSessionConfiguration *configuration = [NSURLSessionConfiguration backgroundSessionConfigurationWithIdentifier:[uuid UUIDString]];
        configuration.discretionary = NO;
        configuration.sessionSendsLaunchEvents = YES;
        fileName_ = GetFileName(config.GetUrl());
        NSString *urlStr = [NSString stringWithUTF8String:config.GetUrl().c_str()];
        NSURL *url = [NSURL URLWithString:urlStr];
        NSURLRequest *request = [NSURLRequest requestWithURL:url];
        sessionCtrl_ = [[OHHttpSessionController alloc] initWithBaseURL:url sessionConfiguration:configuration];
        [sessionCtrl_ setDidFinishEventsForBackgroundURLSessionBlock:(^(NSURLSession *session) {
            DOWNLOAD_HILOGD("resume from background.");
        })];
        if ([url.scheme compare:@"https"] == NSOrderedSame) {
            OHOS::Plugin::Request::CertificateUtils::InstallCertificateChain(sessionCtrl_);
        }
        auto headerMap = config.GetHeader();
        for (const auto &header: headerMap) {
            NSString *key = [NSString stringWithUTF8String:header.first.c_str()];
            NSString *value = [NSString stringWithUTF8String:header.second.c_str()];
            [sessionCtrl_.requestHandler setValue:value forHeaderField:key];
        }
        NSURLRequest *serializedRequest = [sessionCtrl_.requestHandler requestBySeriReq:request withParameters:nil error:nil];
        downloadTask_ = [sessionCtrl_ downloadTaskWithRequest:serializedRequest
            progress:^(NSProgress *downloadProgress) {
                if (callback != nullptr) {
                    callback->OnProgress(downloadProgress.completedUnitCount, downloadProgress.totalUnitCount);
                }
                if (!isMimeReported_) {
                    ReportMimeType(callback, [downloadTask_ response]);
                }
            }
            destination:^NSURL *(NSURL *targetPath, NSURLResponse *response) {
                NSString *filePath = [NSString stringWithUTF8String:config.GetFilePath().c_str()];
                NSURL *destPath = [NSURL URLWithString:filePath];
                return destPath;
            } completionHandler:^(NSURLResponse *response, NSURL *filePath, NSError *error) {
                CompletionHandler(callback, response, filePath, error);
        }];
        [downloadTask_ resume];
        DOWNLOAD_HILOGD("downloadTask resume");
    });
}

std::string IosDownloadAdpImpl::GetFileName(const std::string &fileName)
{
    std::size_t pos = fileName.rfind('/');
    if (pos != std::string::npos) {
        return fileName.substr(pos+1);
    }
    return fileName;
}

void IosDownloadAdpImpl::PushNotification(BOOL isFailed)
{
    DOWNLOAD_HILOGD("Push Notification");
    NSString *file = [NSString stringWithUTF8String:fileName_.c_str()];
    NSValue *result = [NSValue value:&isFailed withObjCType:@encode(BOOL)];
    PushNotification(file, result);
}

void IosDownloadAdpImpl::CompletionHandler(IosDownloadAdpCallback *callback, NSURLResponse *response, NSURL *filePath, NSError *error)
{
    if (!isMimeReported_) {
        ReportMimeType(callback, response);
    }
    if (error == nil) {
        DOWNLOAD_HILOGD("success download file to: %{public}s", [[filePath description] UTF8String]);
        if (callback != nullptr) {
            callback->OnComplete();
        }
    } else {
        DOWNLOAD_HILOGD("failed download file to: %{public}s, error:%{public}s, error_code:%{public}ld", [[filePath description] UTF8String], [[error description] UTF8String], error.code);
        if (callback != nullptr) {
            if (error.code == -1011) { // NSURLErrorBadServerResponse
                callback->OnFail(ERROR_FILE_ERROR);
            } else {
                callback->OnFail(isSuspendByNetwork_ ? ERROR_OFFLINE : ERROR_UNKNOWN);
                isSuspendByNetwork_ = false;
            }
        }

        // check can be resume?
        NSData *resumeData = [[error userInfo] objectForKey:NSURLSessionDownloadTaskResumeData];
        if (resumeData != nil) {
            DOWNLOAD_HILOGD("Download failed, but has resumeData");
            resumeData_ = resumeData;
        }
    }

    DOWNLOAD_HILOGD("isBackground: %{public}d", config_.IsBackground());
    // Allow download background task notifications.
    if (config_.IsBackground()) {
        PushNotification(error != nil);
    }

    std::lock_guard<std::mutex> guard(mutex_);
    isCompleted_ = true;
}

bool IosDownloadAdpImpl::Remove()
{
    DOWNLOAD_HILOGD("Remove download");
    Suspend(false);
    return true;
}

bool IosDownloadAdpImpl::Suspend(bool isSuspendByNetwork)
{
    DOWNLOAD_HILOGD("Suspend download");
    isSuspendByNetwork_ = isSuspendByNetwork;
    if (downloadTask_.state == NSURLSessionTaskStateRunning) {
        [downloadTask_ cancelByProducingResumeData:^(NSData *resumeData) {
            if (resumeData != nil) {
                resumeData_ = resumeData;
            }
        }];
    }
    return true;
}

bool IosDownloadAdpImpl::Restore(IosDownloadAdpCallback *callback)
{
    DOWNLOAD_HILOGD("Restore download");
    if (resumeData_ == nil) {
        DOWNLOAD_HILOGD("restore download, because resumeData is nil, download from begin.");
        Download(config_, callback);
        return true;
    }
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        downloadTask_ = [sessionCtrl_ downloadTaskWithResumeData:resumeData_
            progress:^(NSProgress *downloadProgress) {
                if (callback != nullptr) {
                    callback->OnProgress(downloadProgress.completedUnitCount, downloadProgress.totalUnitCount);
                }
            }
            destination:^NSURL *(NSURL *targetPath, NSURLResponse *response) {
                NSString *filePath = [NSString stringWithUTF8String:config_.GetFilePath().c_str()];
                NSURL *destPath = [NSURL URLWithString:filePath];
                return destPath;
            } completionHandler:^(NSURLResponse *response, NSURL *filePath, NSError *error) {
                CompletionHandler(callback, response, filePath, error);
        }];
        [downloadTask_ resume];
    });
    return true;
}

void IosDownloadAdpImpl::PushNotification(NSString *fileName, NSValue *result)
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

bool IosDownloadAdpImpl::IsCompleted()
{
    std::lock_guard<std::mutex> guard(mutex_);
    return isCompleted_;
}

void IosDownloadAdpImpl::ReportMimeType(IosDownloadAdpCallback *callback, NSURLResponse *response)
{
    isMimeReported_ = true;
    NSString *mimeType = [response MIMEType];
    if ([mimeType length] == 0) {
        NSHTTPURLResponse *resp = (NSHTTPURLResponse *)response;
        mimeType = [resp valueForHTTPHeaderField:@"Content-Type"];
    }
    DOWNLOAD_HILOGD("download, response:%{public}s, mimeType:%{public}s", [[response description] UTF8String], [mimeType UTF8String]);
    if ([mimeType length] > 0 && callback != nullptr) {
        callback->SetMimeType([mimeType UTF8String]);
    }
}
} // namespace OHOS::Plugin::Request::Download