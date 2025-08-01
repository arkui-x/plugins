/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#import "photo_cursor_ios.h"

#define SUCCESS_CODE 0
#define INVALID_POS -1
#ifndef dispatch_main_async_safe
#define dispatch_main_async_safe(block) \
if (dispatch_queue_get_label(DISPATCH_CURRENT_QUEUE_LABEL) == dispatch_queue_get_label(dispatch_get_main_queue())) { \
    block(); \
} else { \
    dispatch_async(dispatch_get_main_queue(), block); \
}
#endif

@interface photoCursorIos()

@property(nonatomic, strong)PHAssetResource * resource;
@property(nonatomic, assign)int index;

@end

@implementation photoCursorIos

- (id)initWithFetchResult:(PHFetchResult *)fetchResult {
    self = [super init];
    if (self) {
        self.fetchResult = fetchResult;
        self.cursorAsset = self.fetchResult.firstObject;
    }
    return self;
}

- (int)getRowCount {
    return (int)self.fetchResult.count;
}

- (int)goToRow:(int) position {
    if (position < self.fetchResult.count) {
        self.cursorAsset = self.fetchResult[position];
        [self clearResource];
        return SUCCESS_CODE;
    }
    return INVALID_POS;
}

- (NSString *)getString:(NSString *)field {
    if ([field isEqualToString:@"filename"]) {
        return [self returnString:[self.cursorAsset valueForKey:@"filename"]];
    } else if ([field isEqualToString:@"uri"]) {
        return [self returnString:[self parseUri]];
    } else if ([field isEqualToString:@"title"]) {
        return [self returnString:[self.cursorAsset valueForKey:@"filename"]];
    } else {
        return @"";
    }
}

- (NSString *)returnString:(NSString *)result {
    return result ? result : @"";
}

- (NSString *)parseUri {
    NSURL *currentURL = [[self.resource valueForKey:@"privateFileURL"]copy];
    BOOL fileUrlAuthozied = [currentURL startAccessingSecurityScopedResource];
    if (fileUrlAuthozied) {
        NSFileCoordinator *fileCoordinator = [[NSFileCoordinator alloc] init];
        NSError *error;
        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
        [fileCoordinator coordinateReadingItemAtURL:currentURL
                                            options:0
                                              error:&error
                                         byAccessor:^(NSURL *newURL) {
            if (!error) {
                NSLog(@"photoCursorIos coordinateReadingItemAtURL read url success");
            } else {
                NSLog(@"photoCursorIos coordinateReadingItemAtURL read url failed");
            }
            dispatch_semaphore_signal(semaphore);
        }];
        dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
        [currentURL stopAccessingSecurityScopedResource];
    } else {
        NSLog(@"photoCursorIos currentURL is not need to coordinateReadingItemAtURL");
    }
    return currentURL.absoluteString;
}

- (int)getInt:(NSString *)field {
    if ([field isEqualToString:@"mediaType"]) {
        return (int)self.cursorAsset.mediaType;
    } else if ([field isEqualToString:@"duration"]) {
        return (int)(self.cursorAsset.duration * 1000);
    } else if ([field isEqualToString:@"pixelHeight"]) {
        return (int)self.cursorAsset.pixelHeight;
    } else if ([field isEqualToString:@"pixelWidth"]) {
        return (int)self.cursorAsset.pixelWidth;
    } else if ([field isEqualToString:@"favorite"]) {
        return (int)self.cursorAsset.favorite;
    } else if ([field isEqualToString:@"hidden"]) {
        return (int)self.cursorAsset.hidden;
    } else if ([field isEqualToString:@"orientation"]) {
        __block NSInteger orientation = -1;
        PHImageManager *manager = [PHImageManager defaultManager];
        PHImageRequestOptions *options = [PHImageRequestOptions new];
        options.deliveryMode = PHImageRequestOptionsDeliveryModeFastFormat;
        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
        dispatch_main_async_safe(^{
            [manager requestImageForAsset:self.cursorAsset 
                               targetSize:PHImageManagerMaximumSize
                              contentMode:PHImageContentModeDefault
                                  options:options
                            resultHandler:^(UIImage * _Nullable result, NSDictionary * _Nullable info) {
                if (!result) {
                    NSLog(@"request image failed.");
                } else {
                    orientation = result.imageOrientation;
                }
                dispatch_semaphore_signal(semaphore);
            }];
        });
        dispatch_time_t timeout = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC));
        dispatch_semaphore_wait(semaphore, timeout);
        return orientation;
    } else {
        return 0;
    }
}

- (int64_t)getLong:(NSString *)field {
    if ([field isEqualToString:@"size"]) {
        return [[self.resource valueForKey:@"fileSize"]longValue];
    } else if ([field isEqualToString:@"modificationDate"]) {
        return [self dateToSec:self.cursorAsset.modificationDate];
    } else if ([field isEqualToString:@"creationDate"]) {
        return [self dateToSec:self.cursorAsset.creationDate];
    } else {
        return 0;
    }
}

- (int64_t)dateToSec:(NSDate *)date {
    NSTimeInterval time = [date timeIntervalSince1970];
    return (int64_t)time;
}

- (void)close {
    [self clearResource];
}

- (int)goToFirstRow {
    if (self.fetchResult.count == 0) {
        return INVALID_POS;
    }
    self.cursorAsset = self.fetchResult.firstObject;
    [self clearResource];
    return SUCCESS_CODE;
}

- (int)goToLastRow {
    if (self.fetchResult.count == 0) {
        return INVALID_POS;
    }
    self.cursorAsset = self.fetchResult.lastObject;
    [self clearResource];
    return SUCCESS_CODE;
}

- (int)goToNextRow {
    NSInteger index = [self.fetchResult indexOfObject:self.cursorAsset];
    if (index + 1 < self.fetchResult.count) {
        self.cursorAsset = self.fetchResult[index + 1];
        [self clearResource];
        return SUCCESS_CODE;
    }
    return INVALID_POS;
}

- (int)goToPreviousRow {
    NSInteger index = [self.fetchResult indexOfObject:self.cursorAsset];
    if (index - 1 >= 0) {
        self.cursorAsset = self.fetchResult[index - 1];
        [self clearResource];
        return SUCCESS_CODE;
    }
    return INVALID_POS;
}

- (BOOL)isAtLastRow {
    if (self.fetchResult.count == 0) {
        return true;
    }
    NSInteger index = [self.fetchResult indexOfObject:self.cursorAsset];
    return index == self.fetchResult.count - 1;
}

- (PHAssetResource *)resource {
    if (!_resource) {
        _resource = [[PHAssetResource assetResourcesForAsset:self.cursorAsset] firstObject];
    }
    return _resource;
}

- (void)clearResource {
    if (_resource) {
        _resource = nil;
    }
}

@end
