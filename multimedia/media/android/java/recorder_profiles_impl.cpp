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

#include "recorder_profiles_impl.h"
#include "media_log.h"
#include "media_errors.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "RecorderProfilesImpl"};
}

namespace OHOS {
namespace Media {
RecorderProfiles& RecorderProfilesFactory::CreateRecorderProfiles()
{
    return RecorderProfilesImpl::GetInstance();
}

RecorderProfiles& RecorderProfilesImpl::GetInstance()
{
    static RecorderProfilesImpl instance;
    instance.Init();
    return instance;
}

int32_t RecorderProfilesImpl::Init()
{
    return MSERR_OK;
}

RecorderProfilesImpl::RecorderProfilesImpl()
{
    MEDIA_LOGD("RecorderProfilesImpl:0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

RecorderProfilesImpl::~RecorderProfilesImpl()
{
    MEDIA_LOGD("RecorderProfilesImpl:0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

bool RecorderProfilesImpl::IsAudioRecorderConfigSupported(const AudioRecorderProfile &profile)
{
    return false;
}

bool RecorderProfilesImpl::HasVideoRecorderProfile(int32_t sourceId, int32_t qualityLevel)
{
    return false;
}

std::vector<std::shared_ptr<AudioRecorderCaps>> RecorderProfilesImpl::GetAudioRecorderCaps()
{
    std::vector<std::shared_ptr<AudioRecorderCaps>> audioRecorderCapsArray;
    return audioRecorderCapsArray;
}

std::vector<std::shared_ptr<VideoRecorderCaps>> RecorderProfilesImpl::GetVideoRecorderCaps()
{
    std::vector<std::shared_ptr<VideoRecorderCaps>> videoRecorderCapsArray;
    return videoRecorderCapsArray;
}

std::shared_ptr<VideoRecorderProfile> RecorderProfilesImpl::GetVideoRecorderProfile(int32_t sourceId,
    int32_t qualityLevel)
{
    return nullptr;
}
}  // namespace Media
}  // namespace OHOS
