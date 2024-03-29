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

#include "avmetadatahelper.h"
#include "nocopyable.h"
#include "i_avmetadatahelper_service.h"

#import <Foundation/Foundation.h>
#import <Photos/Photos.h>

@interface AVMetadataHelperAdapter : NSObject
{
    std::shared_ptr<OHOS::Media::HelperCallback> helperCallbacks_;
    NSURL *url_;
}

#define FIRST_FRAME_TIME 0.0
#define TRANSFORM_1 1.0
enum {
    FIRST_FRAME_TIME_SCALE = 600,
    MICROSECOND_CONVERSION = 1000,
    ORIENTAION_0 = 0,
    ORIENTAION_90 = 90,
    ORIENTAION_180 = 180,
    ORIENTAION_270 = 270,
};

- (void)setSource:(int32_t)fd;
- (std::unordered_map<int32_t, std::string>)resolveMetadata;
- (std::shared_ptr<OHOS::Media::AVSharedMemory>)fetchArtPicture;
- (void)releaseAVMetadataHelperAdapter;

- (void)notifyStateChange:(id)state;

- (void)setHelperCallback:(const std::shared_ptr<OHOS::Media::HelperCallback> &)callback;

- (NSString *)mimeTypeOfURL;
- (void)getVideoMetadata:(std::unordered_map<int32_t, std::string> &)videoMetadata;
- (void)getAudioMetadata:(std::unordered_map<int32_t, std::string> &)audioMetadata;
- (int)getVideoOrientation:(AVAssetTrack *)track;
@end