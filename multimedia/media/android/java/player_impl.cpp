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

#include "player_impl.h"
#include "media_log.h"
#include "media_errors.h"
#include "player_jni.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN_PLAYER, "PlayerImpl"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<Player> PlayerFactory::CreatePlayer(const PlayerProducer producer)
{
    MEDIA_LOGD("PlayerImpl: CreatePlayer in");
    std::shared_ptr<PlayerImpl> impl = std::make_shared<PlayerImpl>();
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, nullptr, "failed to new PlayerImpl");

    int32_t ret = impl->Init();
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, nullptr, "failed to init PlayerImpl");

    return impl;
}

int32_t PlayerImpl::Init()
{
    MEDIA_LOGD("PlayerImpl:0x%{public}06" PRIXPTR " Init in", FAKE_POINTER(this));
    return Plugin::PlayerJni::CallVoidFunc(long(this), "createPlayer");
}

PlayerImpl::PlayerImpl()
{
    MEDIA_LOGD("PlayerImpl:0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

PlayerImpl::~PlayerImpl()
{
    MEDIA_LOGD("PlayerImpl:0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
    Plugin::PlayerJni::CallVoidFunc(long(this), "releasePlayer");
}

int32_t PlayerImpl::SetSource(const std::shared_ptr<IMediaDataSource> &dataSrc)
{
    MEDIA_LOGD("PlayerImpl:0x%{public}06" PRIXPTR " SetSource in(dataSrc)", FAKE_POINTER(this));
    CHECK_AND_RETURN_RET_LOG(dataSrc != nullptr, MSERR_INVALID_VAL, "failed to create data source");

    return Plugin::PlayerJni::setDataSource(long(this), dataSrc);
}

int32_t PlayerImpl::SetSource(const std::string &url)
{
    return Plugin::PlayerJni::setDataSource(long(this), url);
}

int32_t PlayerImpl::SetSource(int32_t fd, int64_t offset, int64_t size)
{
    return Plugin::PlayerJni::setDataSource(long(this), fd, offset, size);
}

int32_t PlayerImpl::AddSubSource(const std::string &url)
{
    return MSERR_OK;
}

int32_t PlayerImpl::AddSubSource(int32_t fd, int64_t offset, int64_t size)
{
    MEDIA_LOGD("PlayerImpl:0x%{public}06" PRIXPTR " AddSubSource in(fd)", FAKE_POINTER(this));

    return MSERR_OK;
}

int32_t PlayerImpl::Play()
{
    return Plugin::PlayerJni::CallVoidFunc(long(this), "play");
}

int32_t PlayerImpl::Prepare()
{
    return Plugin::PlayerJni::CallVoidFunc(long(this), "prepare");
}

int32_t PlayerImpl::PrepareAsync()
{
    return Plugin::PlayerJni::CallVoidFunc(long(this), "prepareAsync");
}

int32_t PlayerImpl::Pause()
{
    return Plugin::PlayerJni::CallVoidFunc(long(this), "pause");
}

int32_t PlayerImpl::Stop()
{
    return Plugin::PlayerJni::CallVoidFunc(long(this), "stop");
}

int32_t PlayerImpl::Reset()
{
    return Plugin::PlayerJni::CallVoidFunc(long(this), "reset");
}

int32_t PlayerImpl::Release()
{
    return Plugin::PlayerJni::CallVoidFunc(long(this), "release");
}

int32_t PlayerImpl::ReleaseSync()
{
    return Plugin::PlayerJni::CallVoidFunc(long(this), "release");
}

int32_t PlayerImpl::SetVolume(float leftVolume, float rightVolume)
{
    return Plugin::PlayerJni::SetVolume(long(this), leftVolume, rightVolume);
}

int32_t PlayerImpl::SetVolumeMode(int32_t mode)
{
    return MSERR_UNSUPPORT;
}

int32_t PlayerImpl::Seek(int32_t mSeconds, PlayerSeekMode mode)
{
    return Plugin::PlayerJni::Seek(long(this), mSeconds, mode);
}

int32_t PlayerImpl::GetCurrentTime(int32_t &currentTime)
{
    return Plugin::PlayerJni::GetCurrentPosition(long(this), currentTime);
}

int32_t PlayerImpl::GetVideoTrackInfo(std::vector<Format> &videoTrack)
{
    return Plugin::PlayerJni::GetVideoTrackInfo(long(this), videoTrack);
}

int32_t PlayerImpl::GetPlaybackInfo(Format& playbackInfo)
{
    return MSERR_UNSUPPORT;
}

int32_t PlayerImpl::GetAudioTrackInfo(std::vector<Format> &audioTrack)
{
    return Plugin::PlayerJni::GetAudioTrackInfo(long(this), audioTrack);
}

int32_t PlayerImpl::GetSubtitleTrackInfo(std::vector<Format> &subtitleTrack)
{
    return Plugin::PlayerJni::GetSubtitleTrackInfo(long(this), subtitleTrack);
}

int32_t PlayerImpl::GetVideoWidth()
{
    return Plugin::PlayerJni::GetVideoWidth(long(this));
}

int32_t PlayerImpl::GetVideoHeight()
{
    return Plugin::PlayerJni::GetVideoHeight(long(this));
}

int32_t PlayerImpl::SetPlaybackSpeed(PlaybackRateMode mode)
{
    return Plugin::PlayerJni::SetSpeed(long(this), mode);
}

int32_t PlayerImpl::SetPlaybackRate(float rate)
{
    return MSERR_UNSUPPORT;
}

int32_t PlayerImpl::GetPlaybackSpeed(PlaybackRateMode &mode)
{
    return Plugin::PlayerJni::GetSpeed(long(this), mode);
}

int32_t PlayerImpl::SelectBitRate(uint32_t bitRate)
{
    return Plugin::PlayerJni::SelectBitRate(long(this), bitRate);
}

int32_t PlayerImpl::GetDuration(int32_t &duration)
{
    return Plugin::PlayerJni::GetDuration(long(this), duration);
}

#ifdef SUPPORT_VIDEO
int32_t PlayerImpl::SetVideoSurface(sptr<Surface> surface)
{
    return Plugin::PlayerJni::SetSurface(long(this), surface->GetDefaultFormat(), (long)surface->GetUniqueId());
}
#endif

bool PlayerImpl::IsPlaying()
{
    MEDIA_LOGD("PlayerImpl:0x%{public}06" PRIXPTR " IsPlaying in", FAKE_POINTER(this));

    return false;
}

bool PlayerImpl::IsLooping()
{
    return Plugin::PlayerJni::IsLooping(long(this));
}

int32_t PlayerImpl::SetLooping(bool loop)
{
    return Plugin::PlayerJni::SetLooping(long(this), loop);
}

int32_t PlayerImpl::SetPlayerCallback(const std::shared_ptr<PlayerCallback> &callback)
{
    return Plugin::PlayerJni::SetCallback(long(this), callback);
}

int32_t PlayerImpl::SetParameter(const Format &param)
{
    int32_t value = 0;
    if (param.GetIntValue(PlayerKeys::VIDEO_SCALE_TYPE, value)) {
        return Plugin::PlayerJni::SetVideoScalingMode(long(this), value);
    }
    return MSERR_OK;
}

int32_t PlayerImpl::SelectTrack(int32_t index, PlayerSwitchMode mode)
{
    return MSERR_UNSUPPORT;
}

int32_t PlayerImpl::DeselectTrack(int32_t index)
{
    return MSERR_UNSUPPORT;
}

int32_t PlayerImpl::GetCurrentTrack(int32_t trackType, int32_t &index)
{
    return MSERR_UNSUPPORT;
}

#ifdef SUPPORT_DRM
int32_t PlayerImpl::SetDecryptConfig(const sptr<DrmStandard::IMediaKeySessionService> &keySessionProxy, bool svp)
{
    return MSERR_UNSUPPORT;
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
