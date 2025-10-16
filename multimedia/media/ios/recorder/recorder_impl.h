/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef RECORDER_IMPL_H
#define RECORDER_IMPL_H

#include "recorder.h"
#include "nocopyable.h"
#import "recorder_adapter.h"

namespace OHOS {
namespace Media {
class RecorderImpl : public Recorder, public NoCopyable {
public:
    RecorderImpl();
    ~RecorderImpl();

    int32_t SetVideoSource(VideoSourceType source, int32_t &sourceId) override;
    int32_t SetVideoEncoder(int32_t sourceId, VideoCodecFormat encoder) override;
    int32_t SetVideoSize(int32_t sourceId, int32_t width, int32_t height) override;
    int32_t SetVideoFrameRate(int32_t sourceId, int32_t frameRate) override;
    int32_t SetVideoEncodingBitRate(int32_t sourceId, int32_t rate) override;
    int32_t SetVideoIsHdr(int32_t sourceId, bool isHdr) override;
    int32_t SetVideoEnableTemporalScale(int32_t sourceId, bool enableTemporalScale) override;
    int32_t SetVideoEnableStableQualityMode(int32_t sourceId, bool enableStableQualityMode) override;
    int32_t SetVideoEnableBFrame(int32_t sourceId, bool enableBFrame) override;
    int32_t SetCaptureRate(int32_t sourceId, double fps) override;
    sptr<OHOS::Surface> GetSurface(int32_t sourceId) override;
    sptr<OHOS::Surface> GetMetaSurface(int32_t sourceId) override;
    int32_t SetAudioSource(AudioSourceType source, int32_t &sourceId) override;
    int32_t SetAudioEncoder(int32_t sourceId, AudioCodecFormat encoder) override;
    int32_t SetAudioSampleRate(int32_t sourceId, int32_t rate) override;
    int32_t SetAudioChannels(int32_t sourceId, int32_t num) override;
    int32_t SetAudioEncodingBitRate(int32_t sourceId, int32_t bitRate) override;
    int32_t SetAudioAacProfile(int32_t sourceId, AacProfile aacProfile) override;
    int32_t SetDataSource(DataSourceType dataType, int32_t &sourceId) override;
    int32_t SetMetaSource(MetaSourceType source, int32_t &sourceId) override;
    int32_t SetMetaConfigs(int32_t sourceId) override;
    int32_t SetMaxDuration(int32_t duration) override;
    int32_t SetOutputFormat(OutputFormatType format) override;
    int32_t SetOutputFile(int32_t fd) override;
    int32_t SetFileGenerationMode(FileGenerationMode mode) override;
    int32_t SetNextOutputFile(int32_t fd) override;
    int32_t SetMaxFileSize(int64_t size) override;
    void SetLocation(float latitude, float longitude) override;
    void SetOrientationHint(int32_t rotation) override;
    int32_t SetRecorderCallback(const std::shared_ptr<RecorderCallback> &callback) override;
    int32_t SetUserCustomInfo(Meta &userCustomInfo) override;
    int32_t SetGenre(std::string &genre) override;
    int32_t Prepare() override;
    int32_t Start() override;
    int32_t Pause() override;
    int32_t Resume() override;
    int32_t Stop(bool block) override;
    int32_t Reset() override;
    int32_t Release() override;
    int32_t SetFileSplitDuration(FileSplitType type, int64_t timestamp, uint32_t duration) override;
    int32_t SetParameter(int32_t sourceId, const Format &format) override;
    int32_t Init();
    int32_t GetAVRecorderConfig(ConfigMap &configMap) override;
    int32_t GetLocation(Location &location) override;
    int32_t GetCurrentCapturerChangeInfo(AudioRecorderChangeInfo &changeInfo) override;
    int32_t GetAvailableEncoder(std::vector<EncoderCapabilityData> &encoderInfo) override;
    int32_t GetMaxAmplitude() override;
    int32_t IsWatermarkSupported(bool &isWatermarkSupported) override;
    int32_t SetWatermark(std::shared_ptr<AVBuffer> &waterMarkBuffer) override;
    int32_t SetUserMeta(const std::shared_ptr<Meta> &userMeta) override;
    int32_t SetWillMuteWhenInterrupted(bool muteWhenInterrupted) override;

private:
    RecorderAdapter *recorderAdapter_ = nil;
};
} // namespace Media
} // namespace OHOS
#endif // RECORDER_IMPL_H
