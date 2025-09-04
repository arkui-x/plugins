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
#import "IosTaskDao.h"
#import "json_utils.h"
#include "request_utils.h"
#import <UserNotifications/UserNotifications.h>

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
    : taskId_(taskId), config_(config), callback_(callback)
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
    info.progress.index = 0;
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

void DownloadProxy::SetProxy(const Config &config, NSURLSessionConfiguration **sessionConfig)
{
    if (sessionConfig == nil) {
        return;
    }
    if (config.proxy.empty()) {
        NSLog(@"Proxy empty");
        return;
    }
    NSURLSessionConfiguration *newSessionConfig = [NSURLSessionConfiguration defaultSessionConfiguration];
    newSessionConfig.HTTPCookieStorage = [NSHTTPCookieStorage sharedHTTPCookieStorage];
    newSessionConfig.HTTPShouldSetCookies = YES;
    NSString *proxyStr = JsonUtils::CStringToNSString(config.proxy);
    NSURL *proxyUrl = [NSURL URLWithString:proxyStr];
    if (proxyUrl && proxyUrl.host && proxyUrl.port) {
        newSessionConfig.requestCachePolicy = NSURLRequestReloadIgnoringLocalCacheData;
        newSessionConfig.connectionProxyDictionary = @{
            @"HTTPEnable": @(YES),
            @"HTTPProxy": proxyUrl.host,
            @"HTTPPort": proxyUrl.port,
            @"HTTPSEnable": @(YES),
            @"HTTPSProxy": proxyUrl.host,
            @"HTTPSPort": proxyUrl.port
        };
        *sessionConfig = newSessionConfig;
        NSLog(@"Proxy set: %@", proxyStr);
    } else {
        NSLog(@"Invalid proxy address: %@", proxyStr);
    }
}

int32_t DownloadProxy::Start(int64_t taskId)
{
    NSLog(@"start download, taskId:%lld, config_.saveas:%s", taskId, config_.saveas.c_str());
    @autoreleasepool {
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            NSLog(@"start download");
            NSString *urlStr = JsonUtils::CStringToNSString(config_.url);
            NSURL *url = [NSURL URLWithString:urlStr];
            NSURLSessionConfiguration *sessionConfig = nil;
            SetProxy(config_, &sessionConfig);
            NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];
            sessionCtrl_ = [[OHSessionManager alloc] initWithConfiguration:sessionConfig];
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
                OnResponseCallback(response);
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
        info_.progress.state = State::RUNNING;
        IosTaskDao::UpdateDB(info_);
    }
    return E_OK;
}

int32_t DownloadProxy::Pause(int64_t taskId)
{
    NSLog(@"Pause download, taskId:%lld", taskId);
    if (downloadTask_.state == NSURLSessionTaskStateRunning) {
        [downloadTask_ cancelByProducingResumeData:^(NSData *resumeData) {
            NSLog(@"DownloadProxy::Pause resumeData=%@", resumeData);
            if (resumeData != nil) {
                resumeData_ = resumeData;
            } else {
                resumeData_ = nil;
            }
        }];
        if (info_.progress.lastProcessed < info_.progress.processed ) {
            info_.progress.lastProcessed = info_.progress.processed;
        }
        isPause_ = true;
        OnPauseCallback();
    }
    return E_OK;
}

int32_t DownloadProxy::Resume(int64_t taskId)
{
    NSLog(@"Resume download, taskId:%lld", taskId);
    if (resumeData_ == nil) {
        int32_t ret = Start(taskId);
        if (ret == E_OK) {
            info_.progress.state = State::RUNNING;
            callback_(taskId_, EVENT_RESUME, JsonUtils::TaskInfoToJsonString(info_));
            IosTaskDao::UpdateDB(info_);
        }
        return ret;
    }
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [sessionCtrl_ downloadTaskWithResumeData:resumeData_
                                   progressBlock:^(NSProgress *downloadProgress) {
            OnProgressCallback(downloadProgress);
        } destination:^NSURL * _Nullable(NSURLResponse *response, NSURL *temporaryURL) {
            NSString *filePath = JsonUtils::CStringToNSString(config_.saveas);
            NSURL *destPath = [NSURL fileURLWithPath:filePath];
            return destPath;
        } completion:^(NSURLResponse *response, NSURL *filePath, NSError *error) {
            CompletionHandler(response, filePath, error);
        }];
        [downloadTask_ resume];
    });
    info_.progress.state = State::RUNNING;
    callback_(taskId_, EVENT_RESUME, JsonUtils::TaskInfoToJsonString(info_));
    IosTaskDao::UpdateDB(info_);
    return E_OK;
}

int32_t DownloadProxy::Stop(int64_t taskId)
{
    NSLog(@"Stop download, taskId:%lld", taskId);
    if (downloadTask_ != nil) {
        [downloadTask_ cancel];
    }
    isStopped_ = true;
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
    BOOL isFailed = FALSE;
    [result getValue:&isFailed];
    NSString *res = !isFailed ? @"成功" : @"失败";
    NSString *msg = [NSString stringWithFormat:@"下载%@", res];

    if (@available(iOS 10.0, *)) {
        UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
        [center requestAuthorizationWithOptions:UNAuthorizationOptionBadge|UNAuthorizationOptionSound|
            UNAuthorizationOptionAlert|
            UNAuthorizationOptionCarPlay completionHandler:^(BOOL granted, NSError * _Nullable error) {
            if (granted) {
                UNMutableNotificationContent *content = [[UNMutableNotificationContent alloc] init];
                content.title = msg;
                content.body = fileName;
                content.sound = [UNNotificationSound defaultSound];

                UNTimeIntervalNotificationTrigger *trigger =
                    [UNTimeIntervalNotificationTrigger triggerWithTimeInterval:0.1 repeats:NO];

                NSString *identifier = [NSString stringWithFormat:@"download_%@_%@", fileName, [[NSUUID UUID] UUIDString]];
                UNNotificationRequest *request = [UNNotificationRequest requestWithIdentifier:identifier
                                                                                      content:content
                                                                                      trigger:trigger];
                [center addNotificationRequest:request withCompletionHandler:^(NSError * _Nullable error) {
                    if (error) {
                        NSLog(@"Failed to show notification: %@", error);
                    }
                }];
            }
        }];
    } else if ([UIDevice currentDevice].systemVersion.floatValue >= 8.0) {
        UIUserNotificationSettings *settings = [UIUserNotificationSettings settingsForTypes:
            UIUserNotificationTypeBadge | UIUserNotificationTypeAlert | UIUserNotificationTypeSound
            categories:nil];
        [[UIApplication sharedApplication] registerUserNotificationSettings:settings];
        UILocalNotification *notification = [[UILocalNotification alloc] init];
        notification.alertBody = msg;
        notification.alertAction = @"View";
        notification.soundName = UILocalNotificationDefaultSoundName;
        [[UIApplication sharedApplication] presentLocalNotificationNow:notification];
    }
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
        IosTaskDao::UpdateDB(info_);
    }
}

void DownloadProxy::OnCompletedCallback()
{
    NSLog(@"download OnCompletedCallback");
    if (callback_ != nullptr) {
        info_.progress.state = State::COMPLETED;
        if (downloadTotalBytes_ == -1) {
            info_.progress.totalProcessed = info_.progress.processed;
            downloadTotalBytes_ = info_.progress.processed;
            SetSizes(downloadTotalBytes_);
        }
        callback_(taskId_, EVENT_PROGRESS, JsonUtils::TaskInfoToJsonString(info_));
        callback_(taskId_, EVENT_COMPLETED, JsonUtils::TaskInfoToJsonString(info_));
        IosTaskDao::UpdateDB(info_);
    }
}

void DownloadProxy::OnProgressCallback(NSProgress *progress)
{
    NSLog(@"download OnProgressCallback");
    if (callback_ == nullptr || progress == nil) {
        return;
    }
    info_.progress.processed = progress.completedUnitCount;
    info_.progress.totalProcessed = progress.totalUnitCount;
    downloadTotalBytes_ = progress.totalUnitCount;
    SetSizes(downloadTotalBytes_);
    if (progress.fractionCompleted == 1.0) {
        info_.progress.state = State::COMPLETED;
    }
    int64_t now = RequestUtils::GetTimeNow();
    if (now - currentTime_ >= REPORT_INFO_INTERVAL) {
        callback_(taskId_, EVENT_PROGRESS, JsonUtils::TaskInfoToJsonString(info_));
        currentTime_ = now;
        IosTaskDao::UpdateDB(info_);
    }
}

NSString* DownloadProxy::GetStandardHTTPReason(NSInteger statusCode) {
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

void DownloadProxy::OnResponseCallback(NSURLResponse *response)
{
    NSLog(@"download OnResponseCallback");
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
            [NSRegularExpression regularExpressionWithPattern:@"HTTP/(\\d\\.\\d) (\\d{3})\n" options:0 error:nil];
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

void DownloadProxy::PushBackOcValues(NSArray<NSString *> *ocValues, std::vector<std::string> &cppValues)
{
    for (NSString *ocValue in ocValues) {
        if (ocValue) {
            cppValues.push_back(std::string([ocValue UTF8String]));
        }
    }
}

void DownloadProxy::OnFailedCallback()
{
    if (isStopped_) {
        NSLog(@"download OnFailedCallback task is stopped");
        return;
    }
    if (isPause_) {
        NSLog(@"download OnFailedCallback task is pause");
        return;
    }

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 2.0 * NSEC_PER_SEC), dispatch_get_main_queue(), ^(void) {
        if (callback_ != nullptr) {
            info_.progress.state = State::FAILED;
            info_.progress.processed = 0;
            info_.code = Reason::OTHERS_ERROR;
            SetSizes(downloadTotalBytes_);
            NSLog(@"download OnFailedCallback start");
            callback_(taskId_, EVENT_FAILED, JsonUtils::TaskInfoToJsonString(info_));
            IosTaskDao::UpdateDB(info_);
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
        IosTaskDao::UpdateDB(info_);
    }
}

void DownloadProxy::SetSizes(int64_t fileSize)
{
    NSLog(@"SetSizes fileSize:%lld", fileSize);
    info_.progress.sizes.clear();
    info_.progress.sizes.emplace_back(fileSize);
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