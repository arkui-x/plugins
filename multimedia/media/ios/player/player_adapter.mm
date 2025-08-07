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

#include "media_description.h"
#include <fcntl.h>

#import "player_adapter.h"
#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#import <CoreMedia/CMTime.h>
#import <CoreMedia/CMFormatDescription.h>
#import <MobileCoreServices/MobileCoreServices.h>
#import "AceSurfaceHolder.h"

@implementation PlayerAdapter
- (id)init
{
    NSLog(@"PlayerAdapter init!");
    self = [super init];
    if(self) {
       self->playerCb_ = nullptr;
       self->playerState_ = OHOS::Media::PLAYER_IDLE;
    }
    return self;
}

- (void)setSource:(const std::string &)url
{
    NSString *str= [NSString stringWithCString:url.c_str() encoding:[NSString defaultCStringEncoding]];
    if (!str) {
        NSLog(@"setSource str is nil!");
        return;
    }
    NSURL *nsUrl = [NSURL URLWithString:str];
    if (!nsUrl) {
        NSLog(@"setSource nsUrl is nil!");
        return;
    }
    [self setSourceURL:nsUrl];
}

- (void)setSourceURL:(NSURL *)url
{
    self.url = url;
    [self initPlayer];
}

- (void)initPlayer
{
    AVAudioSession *audioSession = [AVAudioSession sharedInstance];
    if (!audioSession) {
        NSLog(@"initPlayer audioSession is nil!");
        return;
    }
    [audioSession setCategory:AVAudioSessionCategoryPlayback withOptions:AVAudioSessionCategoryOptionMixWithOthers
        error:nil];
    [audioSession setActive:YES error:nil];

    AVPlayerItem * playerItem = [AVPlayerItem playerItemWithURL:self.url];
    if (self.avPlayer) {
        [self.avPlayer replaceCurrentItemWithPlayerItem:playerItem];
    } else {
        self.avPlayer = [AVPlayer playerWithPlayerItem: playerItem];
    }
    [playerItem addObserver:self forKeyPath:@"status" options:NSKeyValueObservingOptionNew context:nil];
    if (self.avPlayer) {
        NSLog(@"Successfully created a AVPlayer!");
        if (@available(iOS 15.0, *)) {
            self.avPlayer.audiovisualBackgroundPlaybackPolicy = 
                AVPlayerAudiovisualBackgroundPlaybackPolicyContinuesIfPossible;
        }
    }
    self->speed_ = 0.0;
    self->playerState_ = OHOS::Media::PLAYER_INITIALIZED;
    [self performSelectorInBackground:@selector(notifyStateChange:) withObject:[NSNumber numberWithBool:YES]];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object
    change:(NSDictionary<NSKeyValueChangeKey,id> *)change context:(void *)context {
    if ([keyPath isEqualToString:@"frame"] && [object isKindOfClass:[UIView class]]) {
        UIView * view = (UIView *)object;
        if (view) {
            self.avPlayerLayer.frame = view.bounds;
            [view.layer setNeedsDisplay];
        }
    }

    if ([keyPath isEqualToString:@"status"] && [object isKindOfClass:[AVPlayerItem class]]) {
        AVPlayerItemStatus status = (AVPlayerItemStatus)[change[NSKeyValueChangeNewKey] integerValue];
        AVPlayerItem * item = (AVPlayerItem *)object;
        if (item && status == AVPlayerItemStatusReadyToPlay) {
            [self notifyDurationUpdate];
            [item removeObserver:self forKeyPath:@"status"];
        }
    }
}

- (void)setSourceFileFd:(int)fd
{
    char path[PATH_MAX] = { '\0' };
    if (fcntl(fd, F_GETPATH, &path) == -1) {
        NSLog(@"RecorderAdapter setSourceFileFd. Failed to get path!");
        return;
    }
    NSString *nsPath = [NSString stringWithUTF8String: path];
    if (!nsPath) {
        NSLog(@"setSourceFileFd nsPath is nil!");
        return;
    }
    NSURL *url = [NSURL fileURLWithPath:nsPath];
    if (!url) {
        NSLog(@"setSourceFileFd url is nil!");
        return;
    }
    [self setSourceURL:url];
}

- (void)setVideoSurface:(long)layerId inceId:(long)inceId
{
    Class aceSurfaceHolderClass = NSClassFromString(@"AceSurfaceHolder");
    if (aceSurfaceHolderClass) {
        CALayer *caLayer = [aceSurfaceHolderClass getLayerWithId:layerId inceId:inceId];
        if (caLayer) {
            self.avPlayerLayer = [AVPlayerLayer playerLayerWithPlayer:self.avPlayer];
            self.avPlayerLayer.videoGravity = AVLayerVideoGravityResize;
            [caLayer addSublayer: self.avPlayerLayer];
            self->surfaceView_ = caLayer.delegate;
            self->surfaceView_.backgroundColor = UIColor.blackColor;
            [self addObserverWithSurfaceFrame];
            self->isObserverSurface_ = YES;
        }
    }
}

- (void)addObserverWithSurfaceFrame
{
    if (self->surfaceView_) {
        [self->surfaceView_ addObserver:self forKeyPath:@"frame"
            options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld context:nil];
        [self->surfaceView_.layer setNeedsDisplay];
    }
}

- (void)removeObserverWithSurfaceFrame
{
    if (self->surfaceView_) {
        [self->surfaceView_ removeObserver:self forKeyPath:@"frame"];
    }
}

- (void)prepare
{
    if (self->playerState_ != OHOS::Media::PLAYER_STOPPED &&
        self->playerState_ != OHOS::Media::PLAYER_INITIALIZED) {
        return;
    }
    if (self->playerState_ == OHOS::Media::PLAYER_STOPPED) {
        [self initPlayer];
    }
    if (!self->isObserver_) {
        [self addObserverWithPlayToEndTimeNotification];
        [self addPeriodicTimeObserver];
        self->isObserver_ = YES;
    }
    [self notifyVideoSize];
    [self notifyCurrentTime];
    self->playerState_ = OHOS::Media::PLAYER_PREPARED;
    [self performSelectorInBackground:@selector(notifyStateChange:) withObject:[NSNumber numberWithBool:YES]];
}

- (void)play
{
    if (self->playerState_ < OHOS::Media::PLAYER_PREPARED ||
        self->playerState_ == OHOS::Media::PLAYER_STOPPED ||
        self->playerState_ == OHOS::Media::PLAYER_RELEASED) {
        NSLog(@"play the current state of the player is error!");
        return;
    }
    if(self->playerState_ == OHOS::Media::PLAYER_PLAYBACK_COMPLETE) {
        [self.avPlayer seekToTime:kCMTimeZero];
    }
    if(self->speed_ != 0.0) {
        self.avPlayer.rate = self->speed_;
    } else {
        [self.avPlayer play];
    }
    self->playerState_ = OHOS::Media::PLAYER_STARTED;
    [self notifyVideoSize];
    [self performSelectorInBackground:@selector(notifyStateChange:) withObject:[NSNumber numberWithBool:YES]];
}

- (void)stop
{
    if (self->playerState_ == OHOS::Media::PLAYER_PAUSED ||
        self->playerState_ == OHOS::Media::PLAYER_PLAYBACK_COMPLETE ||
        self->playerState_ == OHOS::Media::PLAYER_STARTED ||
        self->playerState_ == OHOS::Media::PLAYER_PREPARED) {
        [self.avPlayer pause];
        [self releasePlayer:NO];
        self->playerState_ = OHOS::Media::PLAYER_STOPPED;
        [self performSelectorInBackground:@selector(notifyStateChange:) withObject:[NSNumber numberWithBool:YES]];
    }
}

- (void)pause
{
    if (self->playerState_ == OHOS::Media::PLAYER_STARTED ||
        self->playerState_ == OHOS::Media::PLAYER_PAUSED) {
        if (self.avPlayer.rate > 0.0) {
            [self.avPlayer pause];
        }
        self->playerState_ = OHOS::Media::PLAYER_PAUSED;
        [self performSelectorInBackground:@selector(notifyStateChange:) withObject:[NSNumber numberWithBool:YES]];
    }
}

- (void)reset
{
    if (self->playerState_ == OHOS::Media::PLAYER_PREPARED ||
        self->playerState_ == OHOS::Media::PLAYER_STARTED ||
        self->playerState_ == OHOS::Media::PLAYER_PLAYBACK_COMPLETE ||
        self->playerState_ == OHOS::Media::PLAYER_PAUSED) {
        [self stop];
    } else {
        if (self->playerState_ != OHOS::Media::PLAYER_IDLE) {
            [self releasePlayer:NO];
        }
    }
    if (self.avPlayer) {
        [self.avPlayer replaceCurrentItemWithPlayerItem:nil];
        self.avPlayer = nil;
    }
    self->playerState_ = OHOS::Media::PLAYER_IDLE;
    [self performSelectorInBackground:@selector(notifyStateChange:) withObject:[NSNumber numberWithBool:YES]];
}

- (void)releasePlayer:(bool)isNotify
{
    if (self->isObserver_) {
        [self removeObserverWithPlayToEndTimeNotification];
        [self removePeriodicTimeObserver];
        self->isObserver_ = NO;
    }
    if (self->isObserverSurface_) {
        [self removeObserverWithSurfaceFrame];
        self->isObserverSurface_ = NO;
    }
    if (isNotify) {
        if (self.avPlayer) {
            [self stop];
            [self.avPlayer replaceCurrentItemWithPlayerItem:nil];
            self.avPlayer = nil;
        }
        self->playerState_ = OHOS::Media::PLAYER_RELEASED;
    }
}

- (void)setVolume:(float)volume
{
    if (self.avPlayer) {
        self.avPlayer.volume = volume;
        [self performSelectorInBackground:@selector(notifyVolumeChange:) withObject:[NSNumber numberWithFloat:volume]];
    }
}

- (void)setSpeed:(OHOS::Media::PlaybackRateMode &) mode;
{
    avPlayerMode_ = mode;
    float multiple = 0.0;
    if (mode == OHOS::Media::SPEED_FORWARD_0_75_X) {
        multiple = SPEED_FORWARD_0_75_F;
    } else if (mode == OHOS::Media::SPEED_FORWARD_1_00_X) {
        multiple = SPEED_FORWARD_1_00_F;
    } else if (mode == OHOS::Media::SPEED_FORWARD_1_25_X) {
        multiple = SPEED_FORWARD_1_25_F;
    } else if (mode == OHOS::Media::SPEED_FORWARD_1_75_X) {
        multiple = SPEED_FORWARD_1_75_F;
    } else if (mode == OHOS::Media::SPEED_FORWARD_2_00_X) {
        multiple = SPEED_FORWARD_2_00_F;
    } else {
        NSLog(@"setSpeed The rate mode is error!");
    }
    if(multiple != self->speed_) {
        self->speed_ = multiple;
        if(self->playerState_ == OHOS::Media::PLAYER_STARTED) {
            self.avPlayer.rate = multiple;
        }
        [self performSelectorInBackground:@selector(notifySpeedDone) withObject:nil];
    } else {
        return;
    }
}

- (void)seekTime:(int)time
{
    Float64 times = time / MICROSECOND_CONVERSION_FLOAT;
    CMTime pos = CMTimeMakeWithSeconds(times, NSEC_PER_SEC);
    [self.avPlayer seekToTime:pos toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero
        completionHandler:^(BOOL finished) {
            if (finished) {
                if (self->playerState_ == OHOS::Media::PLAYER_STARTED) {
                    [self.avPlayer play];
                }
                [self notifySeekDone];
            }
        }];
    self.seekTime = pos;
}

- (void)getCurrentTime:(int32_t &)currentTime
{
    if (self->playerState_ == OHOS::Media::PLAYER_PREPARED ||
        self->playerState_ == OHOS::Media::PLAYER_STARTED ||
        self->playerState_ == OHOS::Media::PLAYER_PAUSED ||
        self->playerState_ == OHOS::Media::PLAYER_PLAYBACK_COMPLETE) {
        Float64 seconds = CMTimeGetSeconds(self.avPlayer.currentTime);
        currentTime = seconds * MICROSECOND_CONVERSION_INT;
    } else {
        NSLog(@"getCurrentTime the state of the player is error!");
        currentTime = -1;
    }
}

- (void)getDuration:(int32_t &)duration
{
    if (self->playerState_ == OHOS::Media::PLAYER_PREPARED ||
        self->playerState_ == OHOS::Media::PLAYER_STARTED ||
        self->playerState_ == OHOS::Media::PLAYER_PAUSED ||
        self->playerState_ == OHOS::Media::PLAYER_PLAYBACK_COMPLETE) {
        Float64 seconds = CMTimeGetSeconds(self.avPlayer.currentItem.duration);
        duration = seconds * MICROSECOND_CONVERSION_INT;
    } else {
        NSLog(@"getDuration the state of the player is error!");
        duration = -1;
    }
}

- (void)setLooping:(bool)loop
{
    self.isLoop = loop;
}

- (bool)isLooping
{
    return self.isLoop;
}

- (NSString *)mimeTypeOfURL
{
    if (!self.url) {
        NSLog(@"mimeTypeOfURL url is nil!");
        return nil;
    }
    NSString * extension = [self.url pathExtension];
    if (!extension) {
        NSLog(@"mimeTypeOfURL extension is nil!");
        return nil;
    }
    NSString *UTIType = (__bridge_transfer NSString *)UTTypeCreatePreferredIdentifierForTag(
        kUTTagClassFilenameExtension, (__bridge CFStringRef)extension, NULL);
    if (!UTIType) {
        NSLog(@"mimeTypeOfURL UTIType is nil!");
        return nil;
    }
    NSString *mimeType = (__bridge_transfer NSString *)UTTypeCopyPreferredTagWithClass(
        (__bridge CFStringRef)UTIType, kUTTagClassMIMEType);
    if (mimeType) {
        return mimeType;
    }
    return nil;
}

- (void)getAudioTrackInfo:(std::vector<OHOS::Media::Format> &)audioTrack
{
    if (!self.url) {
        NSLog(@"getAudioTrackInfo url is nil!");
        return;
    }
    AVAsset *asset = [AVAsset assetWithURL:self.url];
    if (!asset) {
        NSLog(@"getAudioTrackInfo asset is nil!");
        return;
    }
    NSArray *tracks = [asset tracksWithMediaType:AVMediaTypeAudio];
    if (!tracks) {
        NSLog(@"getAudioTrackInfo tracks is nil!");
        return;
    }
    if (0 == tracks.count) {
        return;
    }
    NSLog(@"getAudioTrackInfo count:%lu", tracks.count);
    AVAssetTrack *assetAudioTrack = [tracks firstObject];
    if (!assetAudioTrack) {
        NSLog(@"getVideoTrackInfo assetAudioTrack is nil!");
        return;
    }
    OHOS::Media::Format audioMeta;
    audioMeta.PutIntValue(OHOS::Media::MediaDescriptionKey::MD_KEY_TRACK_INDEX, 0);
    audioMeta.PutIntValue(OHOS::Media::MediaDescriptionKey::MD_KEY_TRACK_TYPE, 0);
    float bps = [assetAudioTrack estimatedDataRate];
    audioMeta.PutIntValue(OHOS::Media::MediaDescriptionKey::MD_KEY_BITRATE, (int)bps);
    NSString *format = [self mimeTypeOfURL];
    NSLog(@"getAudioTrackInfo format:%@", format);
    std::string strFormat = std::string([format UTF8String]);
    audioMeta.PutStringValue(OHOS::Media::MediaDescriptionKey::MD_KEY_CODEC_MIME, strFormat);
    const AudioStreamBasicDescription * audioStreamBasicDescription=
        CMAudioFormatDescriptionGetStreamBasicDescription(
            (CMFormatDescriptionRef)[assetAudioTrack.formatDescriptions firstObject]);
    audioMeta.PutIntValue(OHOS::Media::MediaDescriptionKey::MD_KEY_SAMPLE_RATE,
        (int)audioStreamBasicDescription->mSampleRate);
    std::string language = std::string([assetAudioTrack.languageCode UTF8String]);
    audioMeta.PutStringValue(OHOS::Media::MediaDescriptionKey::MD_KEY_LANGUAGE, language);
    audioTrack.push_back(audioMeta);
}

- (void)getVideoTrackInfo:(std::vector<OHOS::Media::Format> &)videoTrack
{
    if (!self.url) {
        NSLog(@"getVideoTrackInfo url is nil!");
        return;
    }
    AVAsset *asset = [AVAsset assetWithURL:self.url];
    if (!asset) {
        NSLog(@"getVideoTrackInfo asset is nil!");
        return;
    }
    NSArray *tracks = [asset tracksWithMediaType:AVMediaTypeVideo];
    if (!tracks) {
        NSLog(@"getVideoTrackInfo tracks is nil!");
        return;
    }
    if (0 == tracks.count) {
        return;
    }
    NSLog(@"getVideoTrackInfo count:%lu", tracks.count);
    AVAssetTrack *assetVideoTrack = [tracks firstObject];
    if (!assetVideoTrack) {
        NSLog(@"getVideoTrackInfo assetVideoTrack is nil!");
        return;
    }
    OHOS::Media::Format videoMeta;
    videoMeta.PutIntValue(OHOS::Media::MediaDescriptionKey::MD_KEY_TRACK_INDEX, 1);
    videoMeta.PutIntValue(OHOS::Media::MediaDescriptionKey::MD_KEY_TRACK_TYPE, 1);
    videoMeta.PutIntValue(OHOS::Media::MediaDescriptionKey::MD_KEY_WIDTH, (int)assetVideoTrack.naturalSize.width);
    videoMeta.PutIntValue(OHOS::Media::MediaDescriptionKey::MD_KEY_HEIGHT, (int)assetVideoTrack.naturalSize.height);
    float bps = [assetVideoTrack estimatedDataRate];
    videoMeta.PutIntValue(OHOS::Media::MediaDescriptionKey::MD_KEY_BITRATE, (int)bps);
    float fps = [assetVideoTrack nominalFrameRate];
    videoMeta.PutIntValue(OHOS::Media::MediaDescriptionKey::MD_KEY_FRAME_RATE, (int)fps);
    float fDuration = CMTimeGetSeconds(asset.duration);
    int msDuration = fDuration * MICROSECOND_CONVERSION_INT;
    videoMeta.PutIntValue(OHOS::Media::MediaDescriptionKey::MD_KEY_DURATION, msDuration);
    NSString *format = [self mimeTypeOfURL];
    NSLog(@"getVideoTrackInfo format:%@", format);
    std::string strFormat = std::string([format UTF8String]);
    videoMeta.PutStringValue(OHOS::Media::MediaDescriptionKey::MD_KEY_CODEC_MIME, strFormat);
    videoTrack.push_back(videoMeta);
}

- (void)getSubtitleTrackInfo:(std::vector<OHOS::Media::Format> &)subtitleTrack
{
    if (!self.url) {
        NSLog(@"getSubtitleTrackInfo url is nil!");
        return;
    }
    AVAsset *asset = [AVAsset assetWithURL:self.url];
    if (!asset) {
        NSLog(@"getSubtitleTrackInfo asset is nil!");
        return;
    }
    NSArray *tracks = [asset tracksWithMediaType:AVMediaTypeSubtitle];
    if (!tracks) {
        NSLog(@"getSubtitleTrackInfo tracks is nil!");
        return;
    }
    if (0 == tracks.count) {
        return;
    }
    NSLog(@"GetSubtitleTrackInfo count:%lu", tracks.count);
    AVAssetTrack *assetSubttileTrack = [tracks firstObject];
    if (!assetSubttileTrack) {
        NSLog(@"getSubtitleTrackInfo assetSubttileTrack is nil!");
        return;
    }
    OHOS::Media::Format subttileMeta;
    subttileMeta.PutIntValue(OHOS::Media::MediaDescriptionKey::MD_KEY_TRACK_INDEX, 3);
    subttileMeta.PutIntValue(OHOS::Media::MediaDescriptionKey::MD_KEY_TRACK_TYPE, 3);
    std::string language = std::string([assetSubttileTrack.languageCode UTF8String]);
    subttileMeta.PutStringValue(OHOS::Media::MediaDescriptionKey::MD_KEY_LANGUAGE, language);
    NSString *format = [self mimeTypeOfURL];
    NSLog(@"GetSubtitleTrackInfo format:%@", format);
    std::string strFormat = std::string([format UTF8String]);
    subttileMeta.PutStringValue(OHOS::Media::MediaDescriptionKey::MD_KEY_CODEC_MIME, strFormat);
    subtitleTrack.push_back(subttileMeta);
}

- (void)setPlayerCallback:(const std::shared_ptr<OHOS::Media::PlayerCallback> &)callback
{
    self->playerCb_ = callback;
}

- (void)notifyStateChange:(id)isUser
{
    int32_t extra = self->playerState_;
    OHOS::Media::Format newInfo;
    if ([isUser boolValue]) {
        newInfo.PutIntValue(OHOS::Media::PlayerKeys::PLAYER_STATE_CHANGED_REASON,
            OHOS::Media::StateChangeReason::USER);
    } else {
        newInfo.PutIntValue(OHOS::Media::PlayerKeys::PLAYER_STATE_CHANGED_REASON,
            OHOS::Media::StateChangeReason::BACKGROUND);
    }

    if (self->playerCb_) {
        self->playerCb_->OnInfo(OHOS::Media::INFO_TYPE_STATE_CHANGE, extra, newInfo);
    }
}

- (void)notifyVolumeChange:(id)volume
{
    OHOS::Media::Format format;
    float fvolume = [volume floatValue];
    (void)format.PutFloatValue(OHOS::Media::PlayerKeys::PLAYER_VOLUME_LEVEL, fvolume);
    if (self->playerCb_) {
        self->playerCb_->OnInfo(OHOS::Media::INFO_TYPE_VOLUME_CHANGE, 0, format);
    }
}

- (void)notifySeekDone
{
    OHOS::Media::Format format;
    Float64 seconds = CMTimeGetSeconds(self.seekTime);
    int32_t time = seconds * MICROSECOND_CONVERSION_INT;
    if (self->playerCb_) {
        self->playerCb_->OnInfo(OHOS::Media::INFO_TYPE_SEEKDONE, time, format);
    }
}

- (void)notifySpeedDone
{
    OHOS::Media::Format format;
    if (self->playerCb_) {
        self->playerCb_->OnInfo(OHOS::Media::INFO_TYPE_SPEEDDONE, avPlayerMode_, format);
    }
}

- (void)notifyDurationUpdate
{
    OHOS::Media::Format format;
    Float64 seconds = CMTimeGetSeconds(self.avPlayer.currentItem.duration);
    int32_t duration = seconds * MICROSECOND_CONVERSION_INT;
    if (self->playerCb_) {
        self->playerCb_->OnInfo(OHOS::Media::INFO_TYPE_DURATION_UPDATE, duration, format);
    }
}

- (void)notifyVideoSize
{
    OHOS::Media::Format format;
    CGSize size = self.avPlayer.currentItem.presentationSize;
    int width = (int)size.width;
    int height = (int)size.height;
    (void)format.PutIntValue(OHOS::Media::PlayerKeys::PLAYER_WIDTH, width);
    (void)format.PutIntValue(OHOS::Media::PlayerKeys::PLAYER_HEIGHT, height);
    if (self->playerCb_) {
        self->playerCb_->OnInfo(OHOS::Media::INFO_TYPE_RESOLUTION_CHANGE, 0, format);
    }
}

- (void)notifyEndOfStream
{
    OHOS::Media::Format format;
    self->playerState_ = OHOS::Media::PLAYER_PLAYBACK_COMPLETE;
    [self performSelectorInBackground:@selector(notifyStateChange:) withObject:[NSNumber numberWithBool:NO]];
    int32_t hasLoop = 0;
    if (self.isLoop) {
        hasLoop = 1;
        [self play];
    }
    if (self->playerCb_) {
        self->playerCb_->OnInfo(OHOS::Media::INFO_TYPE_EOS, hasLoop, format);
    }
}

- (void)notifyCurrentTime
{
    OHOS::Media::Format format;
    Float64 seconds = CMTimeGetSeconds(self.avPlayer.currentTime);
    int32_t time = seconds * MICROSECOND_CONVERSION_INT;
    if (time < 0) {
        time = 0;
    }
    if (self->playerCb_) {
        self->playerCb_->OnInfo(OHOS::Media::INFO_TYPE_POSITION_UPDATE, time, format);
    }
}

- (void)notifyError
{
    NSError *error = self.avPlayer.error;
    int32_t errorCode = error.code;
    NSString *errorDomain = error.domain;
    std::string errorMsg =  [errorDomain UTF8String];
    if (self->playerCb_) {
        self->playerCb_->OnError(errorCode, errorMsg);
    }
}

- (void)addObserverWithPlayToEndTimeNotification
{
    [[NSNotificationCenter defaultCenter] addObserver:self
                                        selector:@selector(notifyEndOfStream)
                                        name:AVPlayerItemDidPlayToEndTimeNotification
                                        object:nil];
}

- (void)removeObserverWithPlayToEndTimeNotification
{
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                        name:AVPlayerItemDidPlayToEndTimeNotification
                                        object:nil];
}

- (void)addPeriodicTimeObserver
{
    // Invoke callback every half second
    CMTime interval = CMTimeMakeWithSeconds(PERIODIC_TIME, NSEC_PER_SEC);
    // Queue on which to invoke the callback
    dispatch_queue_t mainQueue = dispatch_get_main_queue();
    // Add time observer
    self.timeObserver =
        [self.avPlayer addPeriodicTimeObserverForInterval:interval
                                                  queue:mainQueue
                                             usingBlock:^(CMTime time) {
            [self notifyCurrentTime];
            if (self.avPlayer.status == AVPlayerStatusFailed) {
                [self notifyError];
            }
        }];
}

- (void)removePeriodicTimeObserver
{
    if (self.timeObserver) {
        [self.avPlayer removeTimeObserver:self.timeObserver];
        self.timeObserver = nil;
    }
}
@end
