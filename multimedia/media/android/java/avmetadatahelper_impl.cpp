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

#include "avmetadatahelper_impl.h"
#include "securec.h"
#include "media_log.h"
#include "media_errors.h"
#include "scope_guard.h"
#include "avmetadatahelper_jni.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN_PLAYER, "AVMetadatahelperImpl"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<AVMetadataHelper> AVMetadataHelperFactory::CreateAVMetadataHelper()
{
    std::shared_ptr<AVMetadataHelperImpl> impl = std::make_shared<AVMetadataHelperImpl>();
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, nullptr, "failed to new AVMetadataHelperImpl");

    return impl;
}

AVMetadataHelperImpl::AVMetadataHelperImpl()
{
    Plugin::AVMetadataHelperJni::CreateMetadataRetriever((long)this);
    MEDIA_LOGD("AVMetadataHelperImpl:0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVMetadataHelperImpl::~AVMetadataHelperImpl()
{
    Plugin::AVMetadataHelperJni::ReleaseMetadataRetriever((long)this);
    MEDIA_LOGD("AVMetadataHelperImpl:0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t AVMetadataHelperImpl::SetHelperCallback(const std::shared_ptr<HelperCallback> &callback)
{
    MEDIA_LOGD("AVMetadataHelperImpl:0x%{public}06" PRIXPTR " SetHelperCallback in", FAKE_POINTER(this));

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, MSERR_INVALID_VAL, "callback is nullptr");
    return Plugin::AVMetadataHelperJni::SetHelperCallback((long)this, callback);
}

int32_t AVMetadataHelperImpl::GetTimeByFrameIndex(uint32_t index, uint64_t &time)
{
    return MSERR_OK;
}

int32_t AVMetadataHelperImpl::GetFrameIndexByTime(uint64_t time, uint32_t &index)
{
    return MSERR_OK;
}

int32_t AVMetadataHelperImpl::SetSource(const std::string &uri, int32_t usage)
{
    CHECK_AND_RETURN_RET_LOG(!uri.empty(), MSERR_INVALID_VAL, "uri is empty.");

    return MSERR_OK;
}

int32_t AVMetadataHelperImpl::SetSource(int32_t fd, int64_t offset, int64_t size, int32_t usage)
{
    MEDIA_LOGI("Set file source fd: %{public}d, offset: %{public}" PRIu64 ", size: %{public}" PRIu64,
        fd, offset, size);
    CHECK_AND_RETURN_RET_LOG(fd > 0 && offset >= 0 && size >= -1, MSERR_INVALID_VAL,
        "invalid param");

    return Plugin::AVMetadataHelperJni::SetSource((long)this, fd, offset, size, usage);
}

int32_t AVMetadataHelperImpl::SetSource(const std::shared_ptr<IMediaDataSource> &dataSrc)
{
    MEDIA_LOGD("AVMetadataHelperImpl:0x%{public}06" PRIXPTR " SetSource in(dataSrc)", FAKE_POINTER(this));
    CHECK_AND_RETURN_RET_LOG(dataSrc != nullptr, MSERR_INVALID_VAL, "failed to create data source");
    return Plugin::AVMetadataHelperJni::SetSource((long)this, dataSrc);
}

void AVMetadataHelperImpl::SetScene(Scene scene) {}

std::string AVMetadataHelperImpl::ResolveMetadata(int32_t key)
{
    return "";
}

std::unordered_map<int32_t, std::string> AVMetadataHelperImpl::ResolveMetadata()
{
    MEDIA_LOGD("AVMetadataHelperImpl:0x%{public}06" PRIXPTR " ResolveMetadata in()", FAKE_POINTER(this));
    return Plugin::AVMetadataHelperJni::ResolveMetadata((long)this);
}

std::shared_ptr<AVSharedMemory> AVMetadataHelperImpl::FetchArtPicture()
{
    return Plugin::AVMetadataHelperJni::FetchArtPicture((long)this);
}

std::shared_ptr<PixelMap> AVMetadataHelperImpl::FetchFrameAtTime(
    int64_t timeUs, int32_t option, const PixelMapParams &param)
{
    return nullptr;
}

std::shared_ptr<PixelMap> AVMetadataHelperImpl::FetchFrameYuv(
    int64_t timeUs, int32_t option, const PixelMapParams& param)
{
    return nullptr;
}

std::shared_ptr<PixelMap> AVMetadataHelperImpl::FetchScaledFrameYuv(
    int64_t timeUs, int32_t option, const PixelMapParams& param)
{
    return nullptr;
}

std::shared_ptr<Meta> AVMetadataHelperImpl::GetAVMetadata()
{
    auto meta = std::make_shared<Meta>();
    auto metaMap = ResolveMetadata();
    for (auto iter = metaMap.begin(); iter != metaMap.end(); iter++) {
        auto it = g_MetadataCodeMap.find(iter->first);
        const char* key = nullptr;
        if (it != g_MetadataCodeMap.end()) {
            key = it->second;
        }
        if (key == nullptr) {
            continue;
        }
        std::string tag(key);
        meta->SetData(tag, iter->second);
    }
    return meta;
}

void AVMetadataHelperImpl::Release()
{
    Plugin::AVMetadataHelperJni::Release((long)this);
}
} // namespace Media
} // namespace OHOS
