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

#include "recorder_impl.h"
#include <map>
#include "media_log.h"
#include "media_errors.h"
#include "foundation/multimedia/image_framework/interfaces/innerkits/include/media_errors.h"

#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN_PLAYER, "RecorderImpl"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<Recorder> RecorderFactory::CreateRecorder()
{
    std::shared_ptr<RecorderImpl> impl = std::make_shared<RecorderImpl>();
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, nullptr, "failed to new RecorderImpl");

    int32_t ret = impl->Init();
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, nullptr, "failed to init RecorderImpl");
    return impl;
}

int32_t RecorderImpl::Init()
{
    return MSERR_OK;
}

int32_t RecorderImpl::GetAVRecorderConfig(ConfigMap &configMap)
{
    return MSERR_OK;
}

int32_t RecorderImpl::GetLocation(Location &location)
{
    return MSERR_OK;
}

RecorderImpl::RecorderImpl()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
    recorderAdapter_ = [[RecorderAdapter alloc]init];
}

RecorderImpl::~RecorderImpl()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
    recorderAdapter_ = nil;
}

int32_t RecorderImpl::SetVideoSource(VideoSourceType source, int32_t &sourceId)
{
    if (Media::VIDEO_SOURCE_SURFACE_YUV == source) {
        [recorderAdapter_ setVideoSource:@[
            AVCaptureDeviceTypeBuiltInLiDARDepthCamera,
            AVCaptureDeviceTypeBuiltInTrueDepthCamera]];
    } else {
        MEDIA_LOGI("Invalid video source!");
    }
    return MSERR_OK;
}

int32_t RecorderImpl::SetVideoEncoder(int32_t sourceId, VideoCodecFormat encoder)
{
    CHECK_AND_RETURN_RET_LOG(recorderAdapter_ != nullptr, MSERR_INVALID_OPERATION, "recorder adapter does not exist..");
    if (encoder == Media::H264) {
        [recorderAdapter_ setVideoEncoder:AVVideoCodecTypeH264];
    } else {
        MEDIA_LOGI("Not surpport video format!");
    }

    return MSERR_OK;
}

int32_t RecorderImpl::SetVideoSize(int32_t sourceId, int32_t width, int32_t height)
{
    CHECK_AND_RETURN_RET_LOG(recorderAdapter_ != nullptr, MSERR_INVALID_OPERATION, "recorder adapter does not exist..");
    [recorderAdapter_ setVideoWidth:width];
    [recorderAdapter_ setVideoHeigh:height];
    return MSERR_OK;
}

int32_t RecorderImpl::SetVideoFrameRate(int32_t sourceId, int32_t frameRate)
{
    CHECK_AND_RETURN_RET_LOG(recorderAdapter_ != nullptr, MSERR_INVALID_OPERATION, "recorder adapter does not exist..");
    [recorderAdapter_ setVideoFrameRate:frameRate];
    return MSERR_OK;
}

int32_t RecorderImpl::RecorderImpl::SetVideoEncodingBitRate(int32_t sourceId, int32_t rate)
{
    CHECK_AND_RETURN_RET_LOG(recorderAdapter_ != nullptr, MSERR_INVALID_OPERATION, "recorder adapter does not exist..");
    [recorderAdapter_ setVideoBitRate:rate];
    return MSERR_OK;
}

int32_t RecorderImpl::RecorderImpl::SetVideoIsHdr(int32_t sourceId, bool isHdr)
{
    CHECK_AND_RETURN_RET_LOG(recorderAdapter_ != nullptr, MSERR_INVALID_OPERATION, "recorder adapter does not exist..");
    return MSERR_OK;
}

int32_t RecorderImpl::RecorderImpl::SetVideoEnableTemporalScale(int32_t sourceId, bool enableTemporalScale)
{
    return MSERR_OK;
}

int32_t RecorderImpl::RecorderImpl::SetVideoEnableStableQualityMode(int32_t sourceId, bool enableStableQualityMode)
{
    return MSERR_OK;
}

int32_t RecorderImpl::RecorderImpl::SetVideoEnableBFrame(int32_t sourceId, bool enableBFrame)
{
    return MSERR_OK;
}

int32_t RecorderImpl::SetCaptureRate(int32_t sourceId, double fps)
{

    return MSERR_OK;
}

sptr<OHOS::Surface> RecorderImpl::GetSurface(int32_t sourceId)
{
    return nullptr;
}

sptr<OHOS::Surface> RecorderImpl::GetMetaSurface(int32_t sourceId)
{
    return nullptr;
}

int32_t RecorderImpl::SetAudioSource(AudioSourceType source, int32_t &sourceId)
{
    CHECK_AND_RETURN_RET_LOG(recorderAdapter_ != nullptr, MSERR_INVALID_OPERATION, "recorder adapter does not exist..");
    [recorderAdapter_ setAudioSource:@[AVCaptureDeviceTypeBuiltInMicrophone]];
    return MSERR_OK;
}

int32_t RecorderImpl::SetAudioEncoder(int32_t sourceId, AudioCodecFormat encoder)
{
    CHECK_AND_RETURN_RET_LOG(recorderAdapter_ != nullptr, MSERR_INVALID_OPERATION, "recorder adapter does not exist..");
    [recorderAdapter_ setAudioEncoder: AVAudioQualityLow];
    return MSERR_OK;
}

int32_t RecorderImpl::SetAudioSampleRate(int32_t sourceId, int32_t rate)
{
    CHECK_AND_RETURN_RET_LOG(recorderAdapter_ != nullptr, MSERR_INVALID_OPERATION, "recorder adapter does not exist..");
    [recorderAdapter_ setAudioSampleRate:rate];
    return MSERR_OK;
}

int32_t RecorderImpl::SetAudioChannels(int32_t sourceId, int32_t num)
{
    CHECK_AND_RETURN_RET_LOG(recorderAdapter_ != nullptr, MSERR_INVALID_OPERATION, "recorder adapter does not exist..");
    [recorderAdapter_ setAudioChannels:num];
    return MSERR_OK;
}

int32_t RecorderImpl::SetAudioEncodingBitRate(int32_t sourceId, int32_t bitRate)
{
    CHECK_AND_RETURN_RET_LOG(recorderAdapter_ != nullptr, MSERR_INVALID_OPERATION, "recorder adapter does not exist..");
    [recorderAdapter_ setAudioEncodingBitRate:bitRate];
    return MSERR_OK;
}

int32_t RecorderImpl::SetDataSource(DataSourceType dataType, int32_t &sourceId)
{
    return MSERR_OK;
}

int32_t RecorderImpl::SetMetaSource(MetaSourceType source, int32_t &sourceId)
{
    return MSERR_OK;
}

int32_t RecorderImpl::SetMetaConfigs(int32_t sourceId)
{
    return MSERR_OK;
}

int32_t RecorderImpl::SetMaxDuration(int32_t duration)
{
    return MSERR_OK;
}

int32_t RecorderImpl::SetOutputFormat(OutputFormatType format)
{
    CHECK_AND_RETURN_RET_LOG(recorderAdapter_ != nullptr, MSERR_INVALID_OPERATION, "recorder adapter does not exist..");
    [recorderAdapter_ setOutputFormat:format];
    return Media::MSERR_OK;
}

int32_t RecorderImpl::SetOutputFile(int32_t fd)
{
    CHECK_AND_RETURN_RET_LOG(recorderAdapter_ != nullptr, MSERR_INVALID_OPERATION, "recorder adapter does not exist..");
    [recorderAdapter_ setOutputFile:fd];
    return MSERR_OK;
}

int32_t RecorderImpl::SetFileGenerationMode(FileGenerationMode mode)
{
    return MSERR_OK;
}

int32_t RecorderImpl::SetNextOutputFile(int32_t fd)
{

    return MSERR_OK;
}

int32_t RecorderImpl::SetMaxFileSize(int64_t size)
{

    return MSERR_OK;
}

void RecorderImpl::SetLocation(float latitude, float longitude)
{
    CHECK_AND_RETURN_LOG(recorderAdapter_ != nullptr, "recorder adapter does not exist..");
    [recorderAdapter_ setLocation:latitude longitude:longitude];
}

void RecorderImpl::SetOrientationHint(int32_t rotation)
{
    CHECK_AND_RETURN_LOG(recorderAdapter_ != nullptr, "recorder adapter does not exist..");
    [recorderAdapter_ setOrientationHint:rotation];
}

int32_t RecorderImpl::SetRecorderCallback(const std::shared_ptr<RecorderCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, MSERR_INVALID_VAL, "input callback is nullptr.");
    CHECK_AND_RETURN_RET_LOG(recorderAdapter_ != nullptr, MSERR_INVALID_OPERATION, "recorder adapter does not exist..");
    [recorderAdapter_ SetRecorderCallback:callback];
    return MSERR_OK;
}

int32_t RecorderImpl::SetUserCustomInfo(Meta &userCustomInfo)
{
    return MSERR_OK;
}

int32_t RecorderImpl::SetGenre(std::string &genre)
{
    return MSERR_OK;
}

int32_t RecorderImpl::Prepare()
{
    CHECK_AND_RETURN_RET_LOG(recorderAdapter_ != nullptr, MSERR_INVALID_OPERATION, "recorder adapter does not exist..");
    [recorderAdapter_ prepare];
    return MSERR_OK;
}

int32_t RecorderImpl::Start()
{
    CHECK_AND_RETURN_RET_LOG(recorderAdapter_ != nullptr, MSERR_INVALID_OPERATION, "recorder adapter does not exist..");
    [recorderAdapter_ start];
    return MSERR_OK;
}

int32_t RecorderImpl::Pause()
{
    CHECK_AND_RETURN_RET_LOG(recorderAdapter_ != nullptr, MSERR_INVALID_OPERATION, "recorder adapter does not exist..");
    [recorderAdapter_ pause];
    return MSERR_OK;
}

int32_t RecorderImpl::Resume()
{
    CHECK_AND_RETURN_RET_LOG(recorderAdapter_ != nullptr, MSERR_INVALID_OPERATION, "recorder adapter does not exist..");
    [recorderAdapter_ resume];
    return MSERR_OK;
}

int32_t RecorderImpl::Stop(bool block)
{
    CHECK_AND_RETURN_RET_LOG(recorderAdapter_ != nullptr, MSERR_INVALID_OPERATION, "recorder adapter does not exist..");
    [recorderAdapter_ stop];
    return MSERR_OK;
}

int32_t RecorderImpl::Reset()
{
    CHECK_AND_RETURN_RET_LOG(recorderAdapter_ != nullptr, MSERR_INVALID_OPERATION, "recorder adapter does not exist..");
    [recorderAdapter_ reset];
    return MSERR_OK;
}

int32_t RecorderImpl::Release()
{
    CHECK_AND_RETURN_RET_LOG(recorderAdapter_ != nullptr, MSERR_INVALID_OPERATION, "recorder adapter does not exist..");
    [recorderAdapter_ releaseRecorder];
    return MSERR_OK;
}

int32_t RecorderImpl::SetFileSplitDuration(FileSplitType type, int64_t timestamp, uint32_t duration)
{

    return MSERR_OK;
}

int32_t RecorderImpl::SetParameter(int32_t sourceId, const Format &format)
{

    return MSERR_OK;
}

int32_t RecorderImpl::GetCurrentCapturerChangeInfo(AudioRecorderChangeInfo &changeInfo)
{
    return MSERR_OK;
}

int32_t RecorderImpl::GetAvailableEncoder(std::vector<EncoderCapabilityData> &encoderInfo)
{

    return MSERR_OK;
}

int32_t RecorderImpl::GetMaxAmplitude()
{

    return MSERR_OK;
}

int32_t RecorderImpl::IsWatermarkSupported(bool &isWatermarkSupported)
{
    return MSERR_OK;
}

int32_t RecorderImpl::SetWatermark(std::shared_ptr<AVBuffer> &waterMarkBuffer)
{
    return MSERR_OK;
}

int32_t RecorderImpl::SetUserMeta(const std::shared_ptr<Meta> &userMeta)
{
    return MSERR_OK;
}

int32_t RecorderImpl::SetWillMuteWhenInterrupted(bool muteWhenInterrupted)
{
    return MSERR_OK;
}
} // namespace Media
} // namespace OHOS
