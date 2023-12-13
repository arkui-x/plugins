/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#import "util_plugin.h"

@implementation UtilPlugin

+(instancetype)shareinstance
{
    static dispatch_once_t onceToken;
    static UtilPlugin *instance = nil;
    dispatch_once(&onceToken, ^{
        instance = [UtilPlugin new];
    });
    return instance;
}

-(NSStringEncoding)getNSEncoding:(NSString *)encoding
{
    NSString *encode = [encoding uppercaseString];
    NSArray *encodeArray = [NSArray arrayWithObjects:@"GBK",@"GB18030",@"GB2312",nil];
    int encodeIndex = [encodeArray indexOfObject:encode];
    NSStringEncoding result = nil;
    if(encodeIndex != NSNotFound)
    {
        // GB18030 is compatible with GB2312 and GBK
        result = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingGB_18030_2000);
    } else {
        NSLog(@"Unsopported encoding type");
    }
    return result;
}

-(NSString*)encodeIntoChinese:(NSString *)input encoding:(NSString *)encoding
{
    NSLog(@"Encoding to chinese m->input:%@,encoding:%@",input,encoding);
    NSString *result =@"";
    NSStringEncoding encode = [self getNSEncoding:encoding];
    NSData *responseData = [input dataUsingEncoding:encode];
    Byte *hexByte = (Byte *)[responseData bytes];

    NSString *hexString = @"";
    int dataLength = [responseData length];
    for(int i=0; i < dataLength; i++)
    {
        NSString *newHexStr = [NSString stringWithFormat:@"%x",hexByte[i]&0xff];
        if([newHexStr length] == 1)
            hexString = [NSString stringWithFormat:@"%@0%@",hexString,newHexStr];
        else
            hexString = [NSString stringWithFormat:@"%@%@",hexString,newHexStr];
    }

    if (hexString)
    {
        result = hexString;
    } else {
        NSLog(@"Encoding to chinese failed");
    }

    return result;
}

-(NSString*)decode:(NSString *)input encoding:(NSString *)encoding
{
    NSLog(@"decode m->input:%@,encoding:%@",input,encoding);
    NSString *result =@"";
    NSStringEncoding encodingType = [self getNSEncoding:encoding];
    //1.latin1 str -> latin1 bytes
    NSData *latin1Data = [input dataUsingEncoding:NSISOLatin1StringEncoding];
    //2.latin1 bytes -> encoding type str
    NSString *decodeStr = [[NSString alloc] initWithData:latin1Data encoding:encodingType];
    //3.encoding type str ->  utf8 bytes
    NSData *utf8Data = [decodeStr dataUsingEncoding:NSUTF8StringEncoding];
    //4. utf-8 bytes -> utf-8 str
    NSString *utf8Str = [[NSString alloc] initWithData:utf8Data encoding:NSUTF8StringEncoding];

    if (utf8Str)
    {
        result = utf8Str;
    } else {
        NSLog(@"Encoding to chinese failed");
    }

    return result;
}

@end