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

#include "player_impl.h"
#include "media_log.h"
#include "media_errors.h"
#include "plugins/interfaces/native/inner_api/plugin_utils_inner.h"
#include "foundation/multimedia/image_framework/interfaces/innerkits/include/media_errors.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN_PLAYER, "PlayerImpl"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<Player> PlayerFactory::CreatePlayer(const PlayerProducer producer)
{
    std::shared_ptr<PlayerImpl> impl = std::make_shared<PlayerImpl>();
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, nullptr, "failed to new PlayerImpl");

    int32_t ret = impl->Init();
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, nullptr, "failed to init PlayerImpl");

    return impl;
}

int32_t PlayerImpl::Init()
{
    return MSERR_OK;
}

PlayerImpl::PlayerImpl()
{
    playerAdapter_ = [[PlayerAdapter alloc]init];
}

PlayerImpl::~PlayerImpl()
{
    [playerAdapter_ releasePlayer:NO];
    playerAdapter_ = nil;
}

int32_t PlayerImpl::SetSource(const std::shared_ptr<IMediaDataSource> &dataSrc)
{
    return MSERR_OK;
}

int32_t PlayerImpl::SetSource(const std::string &url)
{
    CHECK_AND_RETURN_RET_LOG(!url.empty(), MSERR_INVALID_VAL, "url is empty..");
    [playerAdapter_ setSource:url];
    return MSERR_OK;
}

int32_t PlayerImpl::SetSource(int32_t fd, int64_t offset, int64_t size)
{
    [playerAdapter_ setSourceFileFd:fd];
    return MSERR_OK;
}

int32_t PlayerImpl::AddSubSource(const std::string &url)
{
    return MSERR_OK;
}

int32_t PlayerImpl::AddSubSource(int32_t fd, int64_t offset, int64_t size)
{
    return MSERR_OK;
}

int32_t PlayerImpl::Play()
{
    [playerAdapter_ play];
    return MSERR_OK;
}

int32_t PlayerImpl::Prepare()
{
    [playerAdapter_ prepare];
    return MSERR_OK;
}

int32_t PlayerImpl::PrepareAsync()
{
    [playerAdapter_ prepare];
    return MSERR_OK;
}

int32_t PlayerImpl::Pause()
{
    [playerAdapter_ pause];
    return MSERR_OK;
}

int32_t PlayerImpl::Stop()
{
    [playerAdapter_ stop];
    return MSERR_OK;
}

int32_t PlayerImpl::Reset()
{
    [playerAdapter_ reset];
    return MSERR_OK;
}

int32_t PlayerImpl::Release()
{
    [playerAdapter_ releasePlayer:YES];
    return MSERR_OK;
}

int32_t PlayerImpl::ReleaseSync()
{
    [playerAdapter_ releasePlayer:YES];
    return MSERR_OK;
}

int32_t PlayerImpl::SetVolume(float leftVolume, float rightVolume)
{
    [playerAdapter_ setVolume:leftVolume];
    return MSERR_OK;
}

int32_t PlayerImpl::SetVolumeMode(int32_t mode)
{
    return MSERR_UNSUPPORT;
}

int32_t PlayerImpl::Seek(int32_t mSeconds, PlayerSeekMode mode)
{
    [playerAdapter_ seekTime: mSeconds];
    return MSERR_OK;
}

int32_t PlayerImpl::GetCurrentTime(int32_t &currentTime)
{
    [playerAdapter_ getCurrentTime: currentTime];
    return MSERR_OK;
}

int32_t PlayerImpl::GetVideoTrackInfo(std::vector<Format> &videoTrack)
{
    CHECK_AND_RETURN_RET_LOG(playerAdapter_ != nullptr, MSERR_INVALID_VAL, "playerAdapter_ is nullptr");
    [playerAdapter_ getVideoTrackInfo:videoTrack];
    return MSERR_OK;
}

int32_t PlayerImpl::GetPlaybackInfo(Format& playbackInfo)
{
    return MSERR_UNSUPPORT;
}

int32_t PlayerImpl::GetAudioTrackInfo(std::vector<Format> &audioTrack)
{
    CHECK_AND_RETURN_RET_LOG(playerAdapter_ != nullptr, MSERR_INVALID_VAL, "playerAdapter_ is nullptr");
    [playerAdapter_ getAudioTrackInfo:audioTrack];
    return MSERR_OK;
}

int32_t PlayerImpl::GetSubtitleTrackInfo(std::vector<Format> &subtitleTrack)
{
    CHECK_AND_RETURN_RET_LOG(playerAdapter_ != nullptr, MSERR_INVALID_VAL, "playerAdapter_ is nullptr");
    [playerAdapter_ getSubtitleTrackInfo:subtitleTrack];
    return MSERR_OK;
}

int32_t PlayerImpl::GetVideoWidth()
{
    return 0;
}

int32_t PlayerImpl::GetVideoHeight()
{
    return 0;
}

int32_t PlayerImpl::SetPlaybackSpeed(PlaybackRateMode mode)
{
    [playerAdapter_ setSpeed:mode];
    return MSERR_OK;
}

int32_t PlayerImpl::SetPlaybackRate(float rate)
{
    return MSERR_UNSUPPORT;
}

int32_t PlayerImpl::GetPlaybackSpeed(PlaybackRateMode &mode)
{
    return 0;
}

int32_t PlayerImpl::SelectBitRate(uint32_t bitRate)
{
    return 0;
}

int32_t PlayerImpl::GetDuration(int32_t &duration)
{
    [playerAdapter_ getDuration: duration];
    return 0;
}

int32_t PlayerImpl::SetVideoSurface(sptr<Surface> surface)
{
    CHECK_AND_RETURN_RET_LOG(surface != nullptr, MSERR_INVALID_VAL, "surface is nullptr");
    [playerAdapter_ setVideoSurface:(long)surface->GetUniqueId()
        inceId:(long)surface->GetDefaultFormat()];
    return MSERR_OK;
}

bool PlayerImpl::IsPlaying()
{
    return false;
}

bool PlayerImpl::IsLooping()
{
    [playerAdapter_ isLooping];
    return false;
}

int32_t PlayerImpl::SetLooping(bool loop)
{
    [playerAdapter_ setLooping:loop];
    return MSERR_OK;
}

int32_t PlayerImpl::SetPlayerCallback(const std::shared_ptr<PlayerCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, MSERR_INVALID_VAL, "callback is nullptr");
    [playerAdapter_ setPlayerCallback:callback];
    return MSERR_OK;
}

int32_t PlayerImpl::SetParameter(const Format &param)
{
    return MSERR_OK;
}

int32_t PlayerImpl::SelectTrack(int32_t index, PlayerSwitchMode mode)
{
    return MSERR_OK;
}

int32_t PlayerImpl::DeselectTrack(int32_t index)
{
    return MSERR_OK;
}

int32_t PlayerImpl::GetCurrentTrack(int32_t trackType, int32_t &index)
{
    return MSERR_OK;
}

#ifdef SUPPORT_DRM
int32_t PlayerImpl::SetDecryptConfig(const sptr<DrmStandard::IMediaKeySessionService> &keySessionProxy, bool svp)
{
    return MSERR_OK;
}
#endif

int32_t PlayerImpl::SetMediaSource(const std::shared_ptr<AVMediaSource> &mediaSource, AVPlayStrategy strategy)
{
    return MSERR_UNSUPPORT;
}

int32_t PlayerImpl::SetPlaybackStrategy(AVPlayStrategy playbackStrategy)
{
    return MSERR_UNSUPPORT;
}

int32_t PlayerImpl::SetMediaMuted(OHOS::Media::MediaType mediaType, bool isMuted)
{
    return MSERR_UNSUPPORT;
}

bool PlayerImpl::ReleaseClientListener()
{
    return MSERR_UNSUPPORT;
}
} // namespace Media
} // namespace OHOS
