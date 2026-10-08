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

#include "cache_download_ios_utils.h"

#import <Foundation/Foundation.h>

#include <string>

namespace OHOS::Plugin::CacheDownload {

std::string GetIosCacheDirectory()
{
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    if (paths == nil || [paths count] == 0) {
        return "/tmp/cacheDownload";
    }
    NSString* cacheDir = [paths objectAtIndex:0];
    // OH convention: cache files live under <cacheRoot>/preload_caches/<sha256(url)>_F.
    // arkui-x exposes this directory to ArkTS as context.cacheDir + "/preload_caches".
    NSString* result = [cacheDir stringByAppendingPathComponent:@"preload_caches"];
    return std::string([result UTF8String]);
}

} // namespace OHOS::Plugin::CacheDownload
