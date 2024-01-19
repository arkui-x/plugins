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

#import "IosTaskFilter.h"

@implementation IosTaskFilter
@end

@implementation IosTaskProgress
- (NSString *)toJsonString {
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    [dict setValue:[NSNumber numberWithInt:self.state] forKey:@"state"];
    [dict setValue:[NSNumber numberWithInt:self.index] forKey:@"index"];
    [dict setValue:[NSNumber numberWithInteger:self.processed] forKey:@"processed"];
    [dict setValue:[NSNumber numberWithInteger:self.totalProcessed] forKey:@"totalProcessed"];
    [dict setObject:self.sizes forKey:@"sizes"];
    [dict setObject:self.extras forKey:@"extras"];
    [dict setObject:self.bodyBytes forKey:@"bodyBytes"];
    NSError *error;
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dict options:NSJSONWritingPrettyPrinted
        error:&error];
    if (!jsonData) {
        NSLog(@"Failed to translate IosTaskProgress dictionary to string");
        return nil;
    }
    NSString *jsonStr = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
    return jsonStr;
}
- (NSArray *)sizes {
    if (_sizes == nil) {
        _sizes = [NSArray array];
    }
    return _sizes;
}
- (NSDictionary *)extras {
    if (_extras == nil) {
        _extras = [NSDictionary dictionary];
    }
    return _extras;
}
- (NSArray *)bodyBytes {
    if (_bodyBytes == nil) {
        _bodyBytes = [NSArray array];
    }
    return _bodyBytes;
}
@end

@implementation IosTaskState
- (NSString *)toJsonString {
    NSDictionary *dict = @{
        @"path": self.path,
        @"responseCode": [NSNumber numberWithInt:self.responseCode],
        @"message": self.message
    };

    NSError *error;
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dict options:NSJSONWritingPrettyPrinted
        error:&error];
    if (!jsonData) {
        NSLog(@"Failed to translate dictionary to string");
        return nil;
    }
    NSString *jsonStr = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
    return jsonStr;
}
@end

