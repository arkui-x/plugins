/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#import <Foundation/Foundation.h>
#import <objc/runtime.h>
#include "ios_json_parser.h"
#include "raw_file_descriptor.h"
#include <iostream>
#include <sstream>
#include <unistd.h>

namespace OHOS {
namespace Sensors {
namespace {
constexpr int64_t READ_DATA_BUFF_SIZE = 256;
} // namespace
static std::string ReadFdToString(const RawFileDescriptor &rawFd)
{
    if (rawFd.fd < 0) {
        return {};
    }

    int dupFd = dup(rawFd.fd);
    if (dupFd < 0) {
        return {};
    }

    FILE *fp = fdopen(dupFd, "rb");
    if (fp == nullptr) {
        close(dupFd);
        return {};
    }

    if (fseek(fp, rawFd.offset, SEEK_SET) != 0) {
        fclose(fp);
        return {};
    }

    std::string dataStr = {};
    dataStr.resize(rawFd.length);
    char* buffer = &dataStr[0];
    int64_t totalRead = 0;
    while (totalRead < rawFd.length) {
        size_t toRead = static_cast<size_t>(std::min(rawFd.length - totalRead, READ_DATA_BUFF_SIZE));
        size_t bytesRead = fread(buffer + totalRead, 1, toRead, fp);
        if (bytesRead == 0) {
            break;
        }
        totalRead += bytesRead;
    }
    fclose(fp);
    if (totalRead != rawFd.length) {
        NSLog(@"Warning: Expected to read %lld bytes, but got %lld", rawFd.length, totalRead);
        return {};
    }
    return dataStr;
}

IosJsonParser::IosJsonParser(const std::string &jsonStr)
{
    ParseJsonString(jsonStr);
}

IosJsonParser::IosJsonParser(const RawFileDescriptor &rawFd)
{
    std::string jsonContent = ReadFdToString(rawFd);
    if (!jsonContent.empty()) {
        ParseJsonString(jsonContent);
    }
}

IosJsonParser::~IosJsonParser()
{
    @autoreleasepool {
        if (root_) {
            id obj = (__bridge_transfer id)root_;
            obj = nil;
            root_ = nullptr;
        }
    }
}

void IosJsonParser::ParseJsonString(const std::string &jsonStr)
{
    @autoreleasepool {
        NSString *jsonString = [NSString stringWithUTF8String:jsonStr.c_str()];
        if (!jsonString) {
            NSLog(@"Failed to convert JSON string to NSString");
            return;
        }
        NSData *jsonData = [jsonString dataUsingEncoding:NSUTF8StringEncoding];
        NSError *error = nil;
        id jsonObject = [NSJSONSerialization JSONObjectWithData:jsonData options:0 error:&error];
        if (error) {
            NSLog(@"Failed to parse JSON: %@", [error localizedDescription]);
            return;
        }
        root_ = (__bridge_retained void*)jsonObject;
    }
}

void* IosJsonParser::GetObjectItem(const std::string &key) const
{
    @autoreleasepool {
        if (!root_) {
            NSLog(@"root is null");
            return nullptr;
        }
        id rootObj = (__bridge id)root_;
        if (![rootObj isKindOfClass:[NSDictionary class]]) {
            NSLog(@"root is not a dictionary, it is: %@", [rootObj class]);
            return nullptr;
        }
        NSString *keyStr = [NSString stringWithUTF8String:key.c_str()];
        NSDictionary *dict = (NSDictionary *)rootObj;
        id result = [dict objectForKey:keyStr];
        if (!result) {
            NSLog(@"Key '%@' not found", keyStr);
            return nullptr;
        }
        return (__bridge void*)result;
    }
}

void* IosJsonParser::GetObjectItem(const void* parent, const std::string &key) const
{
    @autoreleasepool {
        if (!parent) {
            NSLog(@"GetObjectItem: parent is null");
            return nullptr;
        }
        id parentObj = (__bridge id)parent;
        if (![parentObj isKindOfClass:[NSDictionary class]]) {
            NSLog(@"GetObjectItem: parent is not a dictionary");
            return nullptr;
        }
        NSString *keyStr = [NSString stringWithUTF8String:key.c_str()];
        if (!keyStr) {
            NSLog(@"GetObjectItem: failed to convert key to NSString");
            return nullptr;
        }
        id result = [(NSDictionary *)parentObj objectForKey:keyStr];
        if (result == [NSNull null]) {
            NSLog(@"GetObjectItem: key '%@' exists but is NSNull", keyStr);
            return nullptr;
        }
        return (__bridge void*)result;
    }
}

bool IosJsonParser::IsArray(void* item) const
{
    @autoreleasepool {
        if (!item) {
            NSLog(@"IsArray: item is null");
            return false;
        }
        id itemObj = (__bridge id)item;
        return [itemObj isKindOfClass:[NSArray class]];
    }
}

int32_t IosJsonParser::GetArraySize(void* array) const
{
    @autoreleasepool {
        if (!array) {
            NSLog(@"GetArraySize: array is null");
            return 0;
        }
        id arrayObj = (__bridge id)array;
        if (![arrayObj isKindOfClass:[NSArray class]]) {
            return 0;
        }
        return (int32_t)[(NSArray *)arrayObj count];
    }
}

void* IosJsonParser::GetArrayItem(void* array, int32_t index) const
{
    @autoreleasepool {
        if (!array) {
            NSLog(@"GetArrayItem: array is null");
            return nullptr;
        }
        id arrayObj = (__bridge id)array;
        if (![arrayObj isKindOfClass:[NSArray class]]) {
            NSLog(@"GetArrayItem: item is not an array");
            return nullptr;
        }
        NSArray *arr = (NSArray *)arrayObj;
        if (index < 0 || index >= (int32_t)[arr count]) {
            NSLog(@"GetArrayItem: index %d out of bounds [0, %lu)", 
                index, (unsigned long)[arr count]);
            return nullptr;
        }
        id result = [arr objectAtIndex:index];
        if (result == [NSNull null]) {
            NSLog(@"GetArrayItem: item at index %d is NSNull", index);
            return nullptr;
        }
        return (__bridge void*)result;
    }
}

bool IosJsonParser::HasObjectItem(const std::string &key) const
{
    return GetObjectItem(key) != nullptr;
}

int32_t IosJsonParser::GetIntValue(void* item, int32_t defaultValue) const
{
    @autoreleasepool {
        if (!item) {
            return defaultValue;
        }
        id itemObj = (__bridge id)item;
        if ([itemObj isKindOfClass:[NSNumber class]]) {
            return [(NSNumber *)itemObj intValue];
        }
        return defaultValue;
    }
}

std::string IosJsonParser::GetStringValue(void* item, const std::string &defaultValue) const
{
    @autoreleasepool {
        if (!item) {
            return defaultValue;
        }
        id itemObj = (__bridge id)item;
        if ([itemObj isKindOfClass:[NSString class]]) {
            return [(NSString *)itemObj UTF8String];
        }
        return defaultValue;
    }
}

bool IosJsonParser::IsNumber(void* item) const
{
    @autoreleasepool {
        if (!item) {
            return false;
        }
        id itemObj = (__bridge id)item;
        return [itemObj isKindOfClass:[NSNumber class]];
    }
}

bool IosJsonParser::IsString(void* item) const
{
    @autoreleasepool {
        if (!item) {
            return false;
        }
        id itemObj = (__bridge id)item;
        return [itemObj isKindOfClass:[NSString class]];
    }
}

double IosJsonParser::GetDoubleValue(void* item, double defaultValue) const
{
    @autoreleasepool {
        if (!item) {
            return defaultValue;
        }
        id itemObj = (__bridge id)item;
        if ([itemObj isKindOfClass:[NSNumber class]]) {
            double value = [(NSNumber *)itemObj doubleValue];
            NSLog(@"GetDoubleValue: %f", value);
            return value;
        } else if ([itemObj isKindOfClass:[NSString class]]) {
            NSString *stringValue = (NSString *)itemObj;
            double value = [stringValue doubleValue];
            NSLog(@"GetDoubleValue from string '%@': %f", stringValue, value);
            return value;
        }
        return defaultValue;
    }
}
} // namespace Sensors
} // namespace OHOS