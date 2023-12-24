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
    self.action = jsonDict[@"action"];
    self.url = jsonDict[@"url"];
    self.version = jsonDict[@"version"];
    self.mode = jsonDict[@"mode"];
    self.network = jsonDict[@"network"];
    self.index = jsonDict[@"index"];
    self.begins = jsonDict[@"begins"];
    self.ends = jsonDict[@"ends"];
    self.priority = jsonDict[@"priority"];
    self.overwrite = jsonDict[@"overwrite"];
    self.metered = jsonDict[@"metered"];
    self.roaming = jsonDict[@"roaming"];
    self.retry = jsonDict[@"retry"];
    self.redirect = jsonDict[@"redirect"];
    self.gauge = jsonDict[@"gauge"];
    self.precise = jsonDict[@"precise"];
    self.background = jsonDict[@"background"];
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
    NSTimeInterval current = [[NSDate date] timeIntervalSince1970];
    info.ctime = current;
    info.mtime = current;
    info.faults = FaultsOthers;
    info.reason = @"";
    info.taskStates = @"";
    info.token = self.token;
    
    return info;
}

@end
