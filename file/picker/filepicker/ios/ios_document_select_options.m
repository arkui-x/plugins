/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#import "ios_document_select_options.h"

const 

@implementation ios_document_select_options

- (instancetype)init {
    if (self = [super init]) {
        self.allowedUTIs = [NSArray array];
        self.maxSelectNumber = 0;
    }
    return self;
}

- (NSArray<NSString *> *)allowedUTIs {
    return [self transformFormat:self.fileSuffixFilters];
}

- (BOOL)allowsMultipleSelection {
    if (self.maxSelectNumber == 1) {
        return NO;
    }
    return YES;
}

- (NSMutableArray *)transformFormat:(NSArray *)types {
    if (!types || types.count == 0) {
        NSArray *documentTypes = @[@"public.content", @"public.text", @"public.source-code ", @"public.image", @"public.audiovisual-content", @"com.adobe.pdf", @"com.apple.keynote.key", @"com.microsoft.word.doc", @"com.microsoft.excel.xls", @"com.microsoft.powerpoint.ppt", @"public.folder"];
        return (NSMutableArray *)documentTypes;
    }
    NSMutableArray *iosFormat = [NSMutableArray array];
    for (NSString *type in types) {
        if ([type isEqualToString:@"png"] || [type isEqualToString:@"PNG"] || [type isEqualToString:@"jpg"] || [type isEqualToString:@"jpeg"]) {
            [iosFormat addObject:@"public.image"];
        } else if ([type isEqualToString:@"pdf"]) {
            [iosFormat addObject:@"com.adobe.pdf"];
        } else if ([type isEqualToString:@"doc"]) {
            [iosFormat addObject:@"com.microsoft.word.doc"];
            [iosFormat addObject:@"org.openxmlformats.wordprocessingml.document"];
        } else if ([type isEqualToString:@"xls"]) {
            [iosFormat addObject:@"org.openxmlformats.spreadsheetml.sheet"];
            [iosFormat addObject:@"com.microsoft.excel.xls"];
        } else if ([type isEqualToString:@"ppt"]) {
            [iosFormat addObject:@"com.microsoft.powerpoint.ppt"];
            [iosFormat addObject:@"org.openxmlformats.presentationml.presentation"];
        } else if ([type isEqualToString:@"txt"]) {
            [iosFormat addObject:@"public.text"];
        } else if ([type isEqualToString:@".gif"]) {
            [iosFormat addObject:@"com.compuserve.gif"];
        } else {
            [iosFormat addObject:@"public.content"];
        }
    }
    return iosFormat;
}

@end
