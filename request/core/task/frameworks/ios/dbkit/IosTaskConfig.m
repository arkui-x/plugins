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
        
        NSDictionary *jsonDict = (NSDictionary *)jsonObject;
        IosTaskConfig *config = [[IosTaskConfig alloc] init];
        [config initWithDictionary:jsonDict];
        return config;
    } @catch (NSException *exception) {
        NSLog(@"IosTaskConfig initWithJsonString has exception");
        return nil;
    }
}

- (void)initWithDictionary:(NSDictionary *)jsonDict {
    self.action = [jsonDict[@"action"] intValue];
    self.url = jsonDict[@"url"];
    self.version = [jsonDict[@"version"] intValue];
    self.mode = [jsonDict[@"mode"] intValue];
    self.network = [jsonDict[@"network"] intValue];
    self.index = [jsonDict[@"index"] intValue];
    self.begins = [jsonDict[@"begins"] intValue];
    self.ends = [jsonDict[@"ends"] intValue];
    self.priority = [jsonDict[@"priority"] intValue];
    self.overwrite = [jsonDict[@"overwrite"] boolValue];
    self.metered = [jsonDict[@"metered"] boolValue];
    self.roaming = [jsonDict[@"roaming"] boolValue];
    self.retry = [jsonDict[@"retry"] boolValue];
    self.redirect = [jsonDict[@"redirect"] boolValue];
    self.gauge = [jsonDict[@"gauge"] boolValue];
    self.precise = [jsonDict[@"precise"] boolValue];
    self.background = [jsonDict[@"background"] boolValue];
    self.title = jsonDict[@"title"];
    self.saveas = jsonDict[@"saveas"];
    self.realPath = jsonDict[@"realPath"];
    self.method = jsonDict[@"method"];
    self.token = jsonDict[@"token"];
    self.desc = jsonDict[@"desc"];
    self.data = jsonDict[@"data"];
    self.headers = jsonDict[@"headers"];
    self.forms = jsonDict[@"forms"];
    self.files = jsonDict[@"files"];
    self.bodyFds = jsonDict[@"bodyFds"];
    self.bodyFileNames = jsonDict[@"bodyFileNames"];
    self.extras = jsonDict[@"extras"];
}

- (IosTaskInfo *)getTaskInfo {
    IosTaskInfo *info = [[IosTaskInfo alloc] init];
    info.saveas = self.saveas;
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
    info.token = self.token;
    
    return info;
}

@end
