/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
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

#include "plugins/interfaces/native/log.h"
#include "storage_statistics_platform.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleStorageStatistics {
namespace {
constexpr NSInteger MAX_TRAVERSAL_DEPTH = 64;

const char* ToCString(NSString* value)
{
    return value == nil ? "<null>" : [value UTF8String];
}

bool IsValidPath(NSString* path)
{
    return path != nil && path.length > 0;
}

int32_t ValidateRequiredPaths(NSArray<NSString*>* paths)
{
    for (NSString* path in paths) {
        if (!IsValidPath(path)) {
            LOGE("required path is invalid: %s", ToCString(path));
            return PLATFORM_ERROR;
        }
    }
    return PLATFORM_SUCCESS;
}

NSString* GetDocumentsDir()
{
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    return [paths firstObject];
}

NSString* GetApplicationSupportDir()
{
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    return [paths firstObject];
}

NSString* GetCacheDir()
{
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    return [paths firstObject];
}

NSString* GetTempDir()
{
    NSString* tempDirectory = NSTemporaryDirectory();
    if ([tempDirectory hasSuffix:@"/"]) {
        tempDirectory = [tempDirectory substringToIndex:tempDirectory.length - 1];
    }
    return tempDirectory;
}

NSString* GetPreferencesDir()
{
    return [[NSHomeDirectory() stringByAppendingPathComponent:@"Library"] stringByAppendingPathComponent:@"Preferences"];
}

NSString* GetInstallPath()
{
    return [[NSBundle mainBundle] bundlePath];
}

NSArray<NSString*>* GetChildPaths(NSString* path, NSFileManager* fileManager)
{
    NSError* error = nil;
    NSArray<NSString*>* children = [fileManager contentsOfDirectoryAtPath:path error:&error];
    if (children == nil && error != nil) {
        LOGE("failed to enumerate directory, path=%s reason=%s",
            ToCString(path), [[error localizedDescription] UTF8String]);
    }
    return children == nil ? @[] : children;
}

int64_t GetFileSize(NSString* path)
{
    NSURL* url = [NSURL fileURLWithPath:path];
    NSError* error = nil;
    NSArray* keys = @[ NSURLTotalFileAllocatedSizeKey, NSURLFileAllocatedSizeKey ];
    NSDictionary<NSURLResourceKey, id>* values = [url resourceValuesForKeys:keys error:&error];
    if (values == nil) {
        if (error != nil) {
            LOGE("failed to query file size, path=%s reason=%s",
                ToCString(path), [[error localizedDescription] UTF8String]);
        }
        return 0;
    }
    NSNumber* totalAllocated = values[NSURLTotalFileAllocatedSizeKey];
    if (totalAllocated != nil) {
        return [totalAllocated longLongValue];
    }
    NSNumber* fileSize = values[NSURLFileAllocatedSizeKey];
    return fileSize == nil ? 0 : [fileSize longLongValue];
}

int64_t CalculatePathSize(NSString* path)
{
    NSFileManager* fileManager = [NSFileManager defaultManager];
    BOOL isDirectory = NO;
    if (![fileManager fileExistsAtPath:path isDirectory:&isDirectory]) {
        return 0;
    }
    if (!isDirectory) {
        return GetFileSize(path);
    }
    int64_t totalSize = 0;
    NSMutableArray<NSString*>* pendingDirs = [NSMutableArray arrayWithObject:path];
    NSMutableArray<NSNumber*>* pendingDepths = [NSMutableArray arrayWithObject:@0];
    while ([pendingDirs count] > 0) {
        NSString* currentDir = [pendingDirs lastObject];
        [pendingDirs removeLastObject];
        NSInteger currentDepth = [[pendingDepths lastObject] integerValue];
        [pendingDepths removeLastObject];
        if (currentDepth >= MAX_TRAVERSAL_DEPTH) {
            LOGE("calculate path size reached max traversal depth, path=%s", ToCString(currentDir));
            continue;
        }
        for (NSString* child in GetChildPaths(currentDir, fileManager)) {
            NSString* childPath = [currentDir stringByAppendingPathComponent:child];
            BOOL childIsDirectory = NO;
            if (![fileManager fileExistsAtPath:childPath isDirectory:&childIsDirectory]) {
                continue;
            }
            if (childIsDirectory) {
                [pendingDirs addObject:childPath];
                [pendingDepths addObject:@(currentDepth + 1)];
                continue;
            }
            totalSize += GetFileSize(childPath);
        }
    }
    return totalSize;
}

int64_t CalculatePathsSize(NSArray<NSString*>* paths)
{
    int64_t totalSize = 0;
    for (NSString* path in paths) {
        totalSize += CalculatePathSize(path);
    }
    return totalSize;
}

int64_t CountPathInodes(NSString* path)
{
    NSFileManager* fileManager = [NSFileManager defaultManager];
    BOOL isDirectory = NO;
    if (![fileManager fileExistsAtPath:path isDirectory:&isDirectory]) {
        return 0;
    }
    int64_t inodes = 0;
    NSMutableArray<NSString*>* pendingPaths = [NSMutableArray arrayWithObject:path];
    NSMutableArray<NSNumber*>* pendingDepths = [NSMutableArray arrayWithObject:@0];
    while ([pendingPaths count] > 0) {
        NSString* currentPath = [pendingPaths lastObject];
        [pendingPaths removeLastObject];
        NSInteger currentDepth = [[pendingDepths lastObject] integerValue];
        [pendingDepths removeLastObject];
        BOOL currentIsDirectory = NO;
        if (![fileManager fileExistsAtPath:currentPath isDirectory:&currentIsDirectory]) {
            continue;
        }
        inodes++;
        if (!currentIsDirectory) {
            continue;
        }
        if (currentDepth >= MAX_TRAVERSAL_DEPTH) {
            LOGE("count path inodes reached max traversal depth, path=%s", ToCString(currentPath));
            continue;
        }
        for (NSString* child in GetChildPaths(currentPath, fileManager)) {
            [pendingPaths addObject:[currentPath stringByAppendingPathComponent:child]];
            [pendingDepths addObject:@(currentDepth + 1)];
        }
    }
    return inodes;
}
} // namespace

int32_t GetCurrentBundleStatsByPlatform(BundleStatsValue& stats)
{
    NSString* installPath = GetInstallPath();
    NSArray<NSString*>* dataPaths = @[ GetDocumentsDir(), GetApplicationSupportDir(), GetPreferencesDir() ];
    NSArray<NSString*>* cachePaths = @[ GetCacheDir(), GetTempDir() ];
    if (ValidateRequiredPaths(@[ installPath ]) != PLATFORM_SUCCESS) {
        LOGE("failed to query bundle stats, install path is invalid: %s", ToCString(installPath));
        return PLATFORM_ERROR;
    }
    stats = BundleStatsValue {};
    stats.appSize = CalculatePathSize(installPath);
    stats.dataSize = CalculatePathsSize(dataPaths);
    stats.cacheSize = CalculatePathsSize(cachePaths);
    return PLATFORM_SUCCESS;
}

int32_t GetCurrentBundleInodesByPlatform(int64_t& inodes)
{
    NSString* installPath = GetInstallPath();
    NSString* homePath = NSHomeDirectory();
    if (ValidateRequiredPaths(@[ installPath, homePath ]) != PLATFORM_SUCCESS) {
        LOGE("failed to query bundle inodes, installPath=%s homePath=%s",
            ToCString(installPath), ToCString(homePath));
        return PLATFORM_ERROR;
    }
    inodes = CountPathInodes(installPath) + CountPathInodes(homePath);
    return PLATFORM_SUCCESS;
}
} // namespace ModuleStorageStatistics
} // namespace FileManagement
} // namespace OHOS
