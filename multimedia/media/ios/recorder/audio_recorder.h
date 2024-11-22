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

#ifndef MULTIMEDIA_MEDIA_IOS_RECORDER_AUDIO_RECORDER_H
#define MULTIMEDIA_MEDIA_IOS_RECORDER_AUDIO_RECORDER_H

#import <AVFoundation/AVFoundation.h>

@interface AudioRecorder : NSObject<AVAudioRecorderDelegate>
{
    NSURL *strUrl_;
    AVAudioRecorder *avAudioRecorder_;
    bool isPause_;
    NSDictionary *dicSetting_;
    int audioEncoder_;
    float audioSampleRate_;
    int audioChannels_;
    int audioEncodingBitRate_;
    int audioOutputFormat_;
}

- (void)initDicSetting;
- (void)setAudioEncoder:(int)encoder;
- (void)setAudioSampleRate:(int)rate;
- (void)setAudioChannels:(int)num;
- (void)setAudioEncodingBitRate:(int)bitRate;
- (void)setOutputFormat:(int)format;
- (void)setOutputFile:(NSURL *)url;

- (void)setAudioSession;

- (void)prepare;
- (void)start;
- (void)pause;
- (void)resume;
- (void)stop;
- (void)reset;
- (void)releaseAudioRecorder;
@end
#endif //MULTIMEDIA_MEDIA_IOS_RECORDER_AUDIO_RECORDER_H
