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

#ifndef PHOTO_CURSOR_ALBUM_H
#define PHOTO_CURSOR_ALBUM_H

#import <Foundation/Foundation.h>
#import <Photos/Photos.h>

NS_ASSUME_NONNULL_BEGIN

@interface photoCursorAlbum : NSObject

@property (nonatomic,strong) PHFetchResult* fetchResult;

@property (nonatomic,strong) PHAssetCollection* cursorAlbum;

- (id)initWithFetchResult:(PHFetchResult *)fetchResult;

- (int)getRowCount;

- (int)goToRow:(int) position;

- (NSString *)getString:(NSString *)field;

- (int)getInt:(NSString *)field;

- (int64_t)getLong:(NSString *)field;

- (int)goToFirstRow;

- (int)goToLastRow;

- (int)goToNextRow;

- (int)goToPreviousRow;

- (BOOL)isAtLastRow;

@end

NS_ASSUME_NONNULL_END

#endif /* PHOTO_CURSOR_ALBUM_H */
