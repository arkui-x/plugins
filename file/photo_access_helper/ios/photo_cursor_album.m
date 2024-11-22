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

#import "photo_cursor_album.h"

#define SUCCESS_CODE 0
#define INVALID_POS -1

@interface photoCursorAlbum()

@property(nonatomic, assign)bool isNeedQuery;
@property(nonatomic, assign)int count;
@property(nonatomic, assign)int imageCount;
@property(nonatomic, assign)int videoCount;

@property(nonatomic, strong)NSDictionary *typeDic;
@property(nonatomic, strong)NSDictionary *subtypeDic;

@end

@implementation photoCursorAlbum

- (id)initWithFetchResult:(PHFetchResult *)fetchResult {
    self = [super init];
    if (self) {
        self.fetchResult = fetchResult;
        self.cursorAlbum = self.fetchResult.firstObject;
        [self initFilesCount];
    }
    return self;
}

- (int)getRowCount {
    return (int)self.fetchResult.count;
}

- (int)goToRow:(int)position {
    if (position < self.fetchResult.count) {
        self.cursorAlbum = self.fetchResult[position];
        [self initFilesCount];
        return SUCCESS_CODE;
    }
    return INVALID_POS;
}

- (NSString *)getString:(NSString *)field {
    if ([field isEqualToString:@"localizedTitle"]) {
        return [self returnString:self.cursorAlbum.localizedTitle];
    } else if ([field isEqualToString:@"localIdentifier"]) {
        return [self returnString:self.cursorAlbum.localIdentifier];
    }
    return @"";
}

- (NSString *)returnString:(NSString *)result {
    return result ? result : @"";
}

- (int)changType:(int)type typeDic:(NSDictionary *)typeDic {
    NSNumber *typeNumber = @(type);
    NSNumber *type_oh = [typeDic objectForKey:typeNumber];
    if (type_oh) {
        return [type_oh intValue];
    } else {
        return PHAssetCollectionSubtypeAlbumRegular;
    }
}

- (int)getInt:(NSString *)field {
    if ([field isEqualToString:@"assetCollectionType"]) {
        return [self changType:self.cursorAlbum.assetCollectionType typeDic:self.typeDic];
    } else if ([field isEqualToString:@"assetCollectionSubtype"]) {
        return [self changType:self.cursorAlbum.assetCollectionSubtype typeDic:self.subtypeDic];
    } else if ([field isEqualToString:@"hash"]) {
        return (int)self.cursorAlbum.hash;
    } else if ([field isEqualToString:@"count"]) {
        return self.count;
    } else if ([field isEqualToString:@"photosCount"]) {
        return self.imageCount;
    } else if ([field isEqualToString:@"videosCount"]) {
        return self.videoCount;
    } else {
        return 0;
    }
}

- (void)initFilesCount {
    self.count = 0;
    self.imageCount = 0;
    self.videoCount = 0;
    if (!self.cursorAlbum) {
        return;
    }
    PHFetchResult<PHAsset *> *result = [PHAsset fetchAssetsInAssetCollection:self.cursorAlbum options:nil];
    for (PHAsset *asset in result) {
        self.count++;
        if (asset.mediaType == PHAssetMediaTypeImage) {
            self.imageCount++;
        } else if (asset.mediaType == PHAssetMediaTypeVideo) {
            self.videoCount++;
        } else {
            NSLog(@"photoCursorAlbum invalid mediaType");
        }
    }
}

- (int64_t)getLong:(NSString *)field {
    return 0;
}

- (int)goToFirstRow {
    if (self.fetchResult.count == 0) {
        return INVALID_POS;
    }
    self.cursorAlbum = self.fetchResult.firstObject;
    [self initFilesCount];
    return SUCCESS_CODE;
}

- (int)goToLastRow {
    if (self.fetchResult.count == 0) {
        return INVALID_POS;
    }
    self.cursorAlbum = self.fetchResult.lastObject;
    [self initFilesCount];
    return SUCCESS_CODE;
}

- (int)goToNextRow {
    NSInteger index = [self.fetchResult indexOfObject:self.cursorAlbum];
    if (index + 1 < self.fetchResult.count) {
        self.cursorAlbum = self.fetchResult[index + 1];
        [self initFilesCount];
        return SUCCESS_CODE;
    }
    return INVALID_POS;
}

- (int)goToPreviousRow {
    NSInteger index = [self.fetchResult indexOfObject:self.cursorAlbum];
    if (index - 1 >= 0) {
        self.cursorAlbum = self.fetchResult[index - 1];
        [self initFilesCount];
        return SUCCESS_CODE;
    }
    return INVALID_POS;
}

- (BOOL)isAtLastRow {
    if (self.fetchResult.count == 0) {
        return true;
    }
    NSInteger index = [self.fetchResult indexOfObject:self.cursorAlbum];
    return index == self.fetchResult.count - 1;
}

- (NSDictionary *)typeDic {
    if (!_typeDic) {
        _typeDic = @{
            @1 : @0,
            @2 : @1024
        };
    }
    return _typeDic;
}

- (NSDictionary *)subtypeDic {
    if (!_subtypeDic) {
        _subtypeDic = @{
            @2 : @1,
            @203 : @1025,
            @202 : @1026
        };
    }
    return _subtypeDic;
}

@end
