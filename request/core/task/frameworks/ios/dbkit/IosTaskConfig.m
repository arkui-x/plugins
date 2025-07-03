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

#import "IosTaskConfig.h"
#include <time.h>
#import "IosTaskFilter.h"
#import "Constants.h"

@implementation IosTaskConfig

+ (instancetype)initWithJsonString:(NSString *)jsonString {
    @try {
        if (jsonString == nil) {
            return nil;
        }
        NSData *jsonData = [jsonString dataUsingEncoding:NSUTF8StringEncoding];
        NSError *error = nil;
        id jsonObject = [NSJSONSerialization JSONObjectWithData:jsonData options:kNilOptions error:&error];
        if (error) {
            NSLog(@"failed to translate string to json object");
            return nil;
        }
        if (![jsonObject isKindOfClass:[NSDictionary class]]) {
            NSLog(@"failed to translate string to dictionary");
            return nil;
        }
        
        NSDictionary *dict = (NSDictionary *)jsonObject;
        IosTaskConfig *config = [[IosTaskConfig alloc] init];
        [config initWithDictionary:dict];
        return config;
    } @catch (NSException *exception) {
        NSLog(@"IosTaskConfig initWithJsonString has exception");
        return nil;
    }
}

- (void)initWithDictionary:(NSDictionary *)dict {
    self.action = [dict[@"action"] intValue];
    self.url = dict[@"url"];
    self.version = [dict[@"version"] intValue];
    self.mode = [dict[@"mode"] intValue];
    self.network = [dict[@"network"] intValue];
    self.index = [dict[@"index"] intValue];
    self.begins = [dict[@"begins"] intValue];
    self.ends = [dict[@"ends"] intValue];
    self.overwrite = [dict[@"overwrite"] boolValue];
    self.metered = [dict[@"metered"] boolValue];
    self.roaming = [dict[@"roaming"] boolValue];
    self.retry = [dict[@"retry"] boolValue];
    self.redirect = [dict[@"redirect"] boolValue];
    self.gauge = [dict[@"gauge"] boolValue];
    self.precise = [dict[@"precise"] boolValue];
    self.background = [dict[@"background"] boolValue];
    self.title = dict[@"title"];
    self.saveas = dict[@"saveas"];
    self.proxy = dict[@"proxy"];
    self.realPath = dict[@"realPath"];
    self.method = dict[@"method"];
    self.token = dict[@"token"];
    self.desc = dict[@"desc"];
    self.data = dict[@"data"];
    self.headers = dict[@"headers"];
    self.forms = dict[@"forms"];
    self.files = dict[@"files"];
    self.bodyFds = dict[@"bodyFds"];
    self.bodyFileNames = dict[@"bodyFileNames"];
    self.extras = dict[@"extras"];
}

- (IosTaskInfo *)getTaskInfo {
    IosTaskInfo *info = [[IosTaskInfo alloc] init];
    info.saveas = self.saveas;
    info.proxy = self.proxy;
    info.url = self.url;
    info.data = @"";
    info.title = self.title;
    info.desc = self.desc;
    info.action = self.action;
    info.mode = self.mode;
    info.mimeType = @"";
    IosTaskProgress *progress = [[IosTaskProgress alloc] init];
    progress.state = StateInitialized;
    info.progress = [progress toJsonString];
    struct timespec ts;  
    clock_gettime(CLOCK_REALTIME, &ts);  
    long long milliseconds = (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    info.ctime = milliseconds;
    info.mtime = milliseconds;
    info.faults = FaultsOthers;
    info.reason = @"";
    info.taskStates = @"";
    info.downloadId = 0;
    info.token = self.token;
    info.roaming = self.roaming;
    info.metered = self.metered;
    info.network = self.network;
    info.headers = [self objToJsonString:self.headers];
    info.version = self.version;
    info.index = self.index;
    info.begins = self.begins;
    info.ends = self.ends;
    info.overwrite = self.overwrite;
    info.retry = self.retry;
    info.redirect = self.redirect;
    info.gauge = self.gauge;
    info.precise = self.precise;
    info.background = self.background;
    info.method = self.method;
    info.forms = [self objToJsonString:self.forms];
    info.files = [self objToJsonString:self.files];
    info.bodyFds = [self objToJsonString:self.bodyFds];
    info.bodyFileNames = [self objToJsonString:self.bodyFileNames];
    info.tries = @"";
    info.code = 0;
    info.withSystem = false;
    info.extras = [self objToJsonString:self.extras];
    return info;
}

- (NSString *)objToJsonString:(id)obj {
    if (obj == nil) {
        return @"";
    }
    
    NSError *error = nil;
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:obj options:NSJSONWritingPrettyPrinted error:&error];
    if (error) {
        NSLog(@"objToJsonString error:%@", error.localizedDescription);
        return @"";
    }
    NSString *jsonString = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
    return jsonString;
}

@end
