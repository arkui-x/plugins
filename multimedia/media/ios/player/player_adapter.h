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
#ifndef MULTIMEDIA_AVPLAYER_IOS_PLAYER_ADAPTER_H
#define MULTIMEDIA_AVPLAYER_IOS_PLAYER_ADAPTER_H

#include "player.h"
#include "format.h"

#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIView.h>

@interface PlayerAdapter: NSObject
{
    std::shared_ptr<OHOS::Media::PlayerCallback> playerCb_;
    OHOS::Media::PlaybackRateMode avPlayerMode_;
    OHOS::Media::PlayerStates playerState_;
    UIView *surfaceView_;
    float speed_;
    bool isObserver_;
    bool isObserverSurface_;
}

#define MICROSECOND_CONVERSION_FLOAT 1000.0
#define PERIODIC_TIME 0.5
#define SPEED_FORWARD_0_75_F 0.75
#define SPEED_FORWARD_1_00_F 1.00
#define SPEED_FORWARD_1_25_F 1.25
#define SPEED_FORWARD_1_75_F 1.75
#define SPEED_FORWARD_2_00_F 2.00
enum {
    MICROSECOND_CONVERSION_INT = 1000
};

@property(nonatomic, strong)AVPlayer *avPlayer;
@property(nonatomic, strong)AVPlayerLayer *avPlayerLayer;
@property(nonatomic, strong)NSURL *url;
@property(nonatomic)bool isLoop;
@property(nonatomic, strong)id timeObserver;
@property(nonatomic)CMTime seekTime;

- (id)init;

- (void)setSource:(const std::string &)url;
- (void)setSourceURL:(NSURL *)url;
- (void)setSourceFileFd:(int)fd;

- (void)setVideoSurface:(long)layerId inceId:(long)inceId;

- (void)prepare;

- (void)play;

- (void)stop;

- (void)pause;

- (void)reset;

- (void)initPlayer;

- (void)releasePlayer:(bool)isNotify;

- (void)setVolume:(float)volume;
- (void)setSpeed:(OHOS::Media::PlaybackRateMode &) mode;

- (void)seekTime:(int)time;
- (void)getCurrentTime:(int32_t &)currentTime;
- (void)getDuration:(int32_t &)duration;

- (void)setLooping:(bool)loop;
- (bool)isLooping;

- (NSString *)mimeTypeOfURL;
- (void)getAudioTrackInfo:(std::vector<OHOS::Media::Format> &)audioTrack;
- (void)getVideoTrackInfo:(std::vector<OHOS::Media::Format> &)videoTrack;
- (void)getSubtitleTrackInfo:(std::vector<OHOS::Media::Format> &)subtitleTrack;

- (void)setPlayerCallback:(const std::shared_ptr<OHOS::Media::PlayerCallback> &)callback;

- (void)notifyStateChange:(id)isUser;
- (void)notifyVolumeChange:(id)volume;
- (void)notifySeekDone;
- (void)notifySpeedDone;
- (void)notifyDurationUpdate;
- (void)notifyVideoSize;
- (void)notifyEndOfStream;
- (void)notifyCurrentTime;
- (void)notifyError;

- (void)addObserverWithPlayToEndTimeNotification;
- (void)removeObserverWithPlayToEndTimeNotification;

- (void)addPeriodicTimeObserver;
- (void)removePeriodicTimeObserver;

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object
    change:(NSDictionary<NSKeyValueChangeKey,id> *)change context:(void *)context;
- (void)addObserverWithSurfaceFrame;
- (void)removeObserverWithSurfaceFrame;
@end
#endif // MULTIMEDIA_AVPLAYER_IOS_PLAYER_ADAPTER_H
