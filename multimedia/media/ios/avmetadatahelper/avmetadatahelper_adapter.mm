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

#include <string.h>
#include <fcntl.h>
#include "avdatasrcmemory.h"
#include "format.h"
#include "foundation/multimedia/image_framework/interfaces/innerkits/include/media_errors.h"
#include "media_errors.h"
#include "mock_avsharedmemory.h"
#include "uri_helper.h"

#import "avmetadatahelper_adapter.h"
#import <Foundation/Foundation.h>
#import <MobileCoreServices/MobileCoreServices.h>
#import <Photos/Photos.h>
#import <Photos/PHAsset.h>
#import <Photos/PHImageManager.h>
#import <UIKit/UIKit.h>

@implementation AVMetadataHelperAdapter
- (void)setSource:(int32_t)fd
{
    char path[PATH_MAX] = { '\0' };
    if (fcntl(fd, F_GETPATH, &path) == -1) {
        NSLog(@"AVMetadataHelperAdapter setSource. Failed to get path!");
        return;
    }
    NSString *nsPath = [NSString stringWithUTF8String: path];
    self->url_ = [NSURL fileURLWithPath:nsPath];
    if (!self->url_) {
        NSLog(@"setSource url_ is nil!");
        return;
    }
    [self performSelectorInBackground:@selector(notifyStateChange:)
        withObject:[NSNumber numberWithInt: OHOS::Media::HELPER_PREPARED]];
}

- (std::unordered_map<int32_t, std::string>)resolveMetadata
{
    std::unordered_map<int32_t, std::string> result;
    if (!self->url_) {
        NSLog(@"resolveMetadata url_ is nil!");
        return result;
    }
    AVURLAsset *asset = [AVURLAsset URLAssetWithURL:self->url_ options:nil];
    if (!asset) {
        NSLog(@"resolveMetadata asset is nil!");
        return result;
    }
    NSString *mimeType = [self mimeTypeOfURL];
    if (mimeType) {
        NSLog(@"resolveMetadata mimeType:%@", mimeType);
        result[OHOS::Media::AV_KEY_MIME_TYPE] = std::string([mimeType UTF8String]);
    }
    [self getVideoMetadata:result];
    [self getAudioMetadata:result];
    int count = asset.tracks.count;
    result[OHOS::Media::AV_KEY_NUM_TRACKS] = std::to_string(count);
    for (AVMetadataItem *metadataItem in asset.metadata) {
        NSLog(@"resolveMetadata commonkey:%@", metadataItem.commonKey);
        NSString *value = (NSString *)metadataItem.value;
        NSLog(@"resolveMetadata value:%@", metadataItem.value);
        if ([metadataItem.commonKey isEqualToString:AVMetadataCommonKeyAlbumName]) {
            result[OHOS::Media::AV_KEY_ALBUM] = std::string([value UTF8String]);
        } else if ([metadataItem.commonKey isEqualToString:AVMetadataCommonKeyArtist]) {
            result[OHOS::Media::AV_KEY_ARTIST] = std::string([value UTF8String]);
        } else if ([metadataItem.commonKey isEqualToString:AVMetadataCommonKeyAuthor]) {
            result[OHOS::Media::AV_KEY_AUTHOR] = std::string([value UTF8String]);
        } else if ([metadataItem.commonKey isEqualToString:AVMetadataCommonKeyCreator]) {
            result[OHOS::Media::AV_KEY_COMPOSER] = std::string([value UTF8String]);
        } else if ([metadataItem.commonKey isEqualToString:AVMetadataCommonKeyTitle]) {
            result[OHOS::Media::AV_KEY_TITLE] = std::string([value UTF8String]);
        } else if ([metadataItem.commonKey isEqualToString:AVMetadataCommonKeyCreationDate]) {
            result[OHOS::Media::AV_KEY_DATE_TIME] = std::string([value UTF8String]);
        } else {
            NSLog(@"resolveMetadata commonkey:%@", metadataItem.commonKey);
        }
    }
    [self performSelectorInBackground:@selector(notifyStateChange:)
        withObject:[NSNumber numberWithInt: OHOS::Media::HELPER_CALL_DONE]];
    return result;
}

- (std::shared_ptr<OHOS::Media::AVSharedMemory>)fetchArtPicture
{
    if (@available(iOS 18.0, *)) {
        NSLog(@"fetchArtPicture not supported");
        return nullptr;
    }
    if (!self->url_) {
        NSLog(@"fetchArtPicture url_ is nil!");
        return nullptr;
    }
    AVURLAsset *asset = [[AVURLAsset alloc] initWithURL:self->url_ options:nil];
    if (!asset) {
        NSLog(@"fetchArtPicture asset is nil!");
        return nullptr;
    }
    NSArray *metadataItems = [asset metadataForFormat:AVMetadataFormatID3Metadata];
    NSData *imageData = nil;
    for (AVMetadataItem *item in metadataItems) {
        if ([[item commonKey] isEqualToString:AVMetadataCommonKeyArtwork]) {
            imageData = (NSData *)item.value;
            break;
        }
    }
    if (!imageData) {
        NSLog(@"fetchArtPicture imageData is nil!");
        return nullptr;
    }
    UIImage *thumbImg = [UIImage imageWithData:imageData];
    if (!thumbImg) {
        NSLog(@"fetchArtPicture thumbImg is nil!");
        return nullptr;
    }

    NSData *nsPngData = UIImagePNGRepresentation(thumbImg);
    if (!nsPngData) {
        NSLog(@"fetchArtPicture nsPngData is nil!");
        return nullptr;
    }

    const uint8_t *buff = (uint8_t *)nsPngData.bytes;
    int32_t size = (int32_t)nsPngData.length;

    NSLog(@"fetchArtPicture size:%d", size);
    std::shared_ptr<OHOS::Media::MockAVSharedMemory> memory =
        std::make_shared<OHOS::Media::MockAVSharedMemory>(size, 0);

    memcpy((void *)memory->GetBase(), (void *)buff, size);
    [self performSelectorInBackground:@selector(notifyStateChange:)
        withObject:[NSNumber numberWithInt: OHOS::Media::HELPER_CALL_DONE]];
    return memory;
}

- (void)notifyStateChange:(id)state
{
    if (self->helperCallbacks_) {
        OHOS::Media::Format newInfo;
        int extra = [state intValue];
        self->helperCallbacks_->OnInfo(OHOS::Media::HELPER_INFO_TYPE_STATE_CHANGE, extra, newInfo);
    }
}

- (void)setHelperCallback:(const std::shared_ptr<OHOS::Media::HelperCallback> &)callback
{
    self->helperCallbacks_ = callback;
    [self performSelectorInBackground:@selector(notifyStateChange:)
        withObject:[NSNumber numberWithInt: OHOS::Media::HELPER_IDLE]];
}

- (void)releaseAVMetadataHelperAdapter
{
    self->url_ = nil;
}

- (NSString *)mimeTypeOfURL
{
    if (!self->url_) {
        NSLog(@"mimeTypeOfURL url_ is nil!");
        return nil;
    }
    NSString * extension = [self->url_ pathExtension];
    if (extension) {
        NSLog(@"mimeTypeOfURL extension is nil!");
        return nil;
    }
    NSString *UTI = (__bridge_transfer NSString *)UTTypeCreatePreferredIdentifierForTag(
        kUTTagClassFilenameExtension, (__bridge CFStringRef)extension, NULL);
    if (UTI) {
        NSLog(@"mimeTypeOfURL UTI is nil!");
        return nil;
    }
    NSString *mimeType = (__bridge_transfer NSString *)UTTypeCopyPreferredTagWithClass(
        (__bridge CFStringRef)UTI, kUTTagClassMIMEType);
    if (mimeType) {
        return mimeType;
    }
    return nil;
}

- (void)getVideoMetadata:(std::unordered_map<int32_t, std::string> &)videoMetadata
{
    if (!self->url_) {
        NSLog(@"getVideoMetadata url_ is nil!");
        return;
    }
    AVAsset *asset = [AVAsset assetWithURL:self->url_];
    if (!asset) {
        NSLog(@"getVideoMetadata asset is nil!");
        return;
    }
    NSArray *tracks = [asset tracksWithMediaType:AVMediaTypeVideo];
    if (!tracks) {
        NSLog(@"getVideoMetadata tracks is nil!");
        return;
    }
    if (0 == tracks.count) {
        return;
    }
    NSLog(@"getVideoTrackInfo count:%lu", tracks.count);
    AVAssetTrack *assetVideoTrack = [tracks firstObject];
    videoMetadata[OHOS::Media::AV_KEY_HAS_VIDEO] = "yes";
    videoMetadata[OHOS::Media::AV_KEY_VIDEO_WIDTH] = std::to_string((int)assetVideoTrack.naturalSize.width);
    videoMetadata[OHOS::Media::AV_KEY_VIDEO_HEIGHT] = std::to_string((int)assetVideoTrack.naturalSize.height);
    float fDuration = CMTimeGetSeconds(asset.duration);
    int msDuration = fDuration * MICROSECOND_CONVERSION;
    videoMetadata[OHOS::Media::AV_KEY_DURATION] = std::to_string(msDuration);
    int orientation = [self getVideoOrientation:assetVideoTrack];
    if (orientation != -1) {
        videoMetadata[OHOS::Media::AV_KEY_VIDEO_ORIENTATION] = std::to_string(orientation);
    }
}

- (void)getAudioMetadata:(std::unordered_map<int32_t, std::string> &)audioMetadata
{
    if (!self->url_) {
        NSLog(@"getAudioMetadata url_ is nil!");
        return;
    }
    AVAsset *asset = [AVAsset assetWithURL:self->url_];
    if (!asset) {
        NSLog(@"getAudioMetadata asset is nil!");
        return;
    }
    NSArray *tracks = [asset tracksWithMediaType:AVMediaTypeAudio];
    if (!tracks) {
        NSLog(@"getAudioMetadata tracks is nil!");
        return;
    }
    if (0 == tracks.count) {
        return;
    }
    NSLog(@"getAudioTrackInfo count:%lu", tracks.count);
    AVAssetTrack *assetAudioTrack = [tracks firstObject];
    if (!assetAudioTrack) {
        NSLog(@"getAudioMetadata assetAudioTrack is nil!");
        return;
    }
    audioMetadata[OHOS::Media::AV_KEY_HAS_AUDIO] = "yes";
    const AudioStreamBasicDescription * audioStreamBasicDescription=
        CMAudioFormatDescriptionGetStreamBasicDescription(
            (CMFormatDescriptionRef)[assetAudioTrack.formatDescriptions firstObject]);
    if (!audioStreamBasicDescription) {
        NSLog(@"getAudioMetadata audioStreamBasicDescription is nil!");
        return;
    }
    audioMetadata[OHOS::Media::AV_KEY_SAMPLE_RATE] = std::to_string((int)audioStreamBasicDescription->mSampleRate);
}

- (int)getVideoOrientation:(AVAssetTrack *)track
{
    if (track) {
        CGAffineTransform transform = track.preferredTransform;
        if (transform.a == TRANSFORM_1 && transform.b == 0 && transform.c == 0 && transform.d == TRANSFORM_1) {
            return ORIENTAION_0;
        } else if (transform.a == 0 && transform.b == TRANSFORM_1 && transform.c == -TRANSFORM_1 && transform.d == 0) {
            return ORIENTAION_90;
        } else if (transform.a == -TRANSFORM_1 && transform.b == 0 && transform.c == 0 && transform.d == -TRANSFORM_1) {
            return ORIENTAION_180;
        } else if (transform.a == 0 && transform.b == -TRANSFORM_1 && transform.c == TRANSFORM_1 && transform.d == 0) {
            return ORIENTAION_270;
        }
    }
    return -1;
}
@end
