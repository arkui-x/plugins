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

#include "avmetadatahelper_impl.h"
#include "securec.h"
#include "media_log.h"
#include "media_errors.h"
#include "scope_guard.h"
#include "foundation/multimedia/image_framework/interfaces/innerkits/include/media_errors.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN_PLAYER, "AVMetadatahelperImpl"};
}

namespace OHOS {
namespace Media {
struct PixelMapMemHolder {
    bool isShmem;
    std::shared_ptr<AVSharedMemory> shmem;
    uint8_t *heap;
};

static void FreePixelMapData(void *addr, void *context, uint32_t size)
{
    (void)size;

    MEDIA_LOGD("free pixel map data");

    CHECK_AND_RETURN_LOG(context != nullptr, "context is nullptr");
    PixelMapMemHolder *holder = reinterpret_cast<PixelMapMemHolder *>(context);
    if (holder->isShmem) {
        if (holder->shmem == nullptr) {
            MEDIA_LOGE("shmem is nullptr");
        }
        holder->shmem = nullptr;
        holder->heap = nullptr;
    } else {
        if (holder->heap == nullptr || holder->heap != addr) {
            MEDIA_LOGE("heap is invalid");
        } else {
            delete [] holder->heap;
            holder->heap = nullptr;
        }
    }
    delete holder;
}

static PixelMapMemHolder *CreatePixelMapData(const std::shared_ptr<AVSharedMemory> &mem, const OutputFrame &frame)
{
    PixelMapMemHolder *holder = new (std::nothrow) PixelMapMemHolder;
    CHECK_AND_RETURN_RET_LOG(holder != nullptr, nullptr, "alloc pixelmap mem holder failed");

    ON_SCOPE_EXIT(0) { delete holder; };

    int32_t minStride = frame.width_ * frame.bytesPerPixel_;
    CHECK_AND_RETURN_RET_LOG(minStride <= frame.stride_, nullptr, "stride info wrong");

    if (frame.stride_ == minStride) {
        CANCEL_SCOPE_EXIT_GUARD(0);
        holder->isShmem = true;
        holder->shmem = mem;
        holder->heap = frame.GetFlattenedData();
        return holder;
    }

    static constexpr int64_t maxAllowedSize = 100 * 1024 * 1024;
    int64_t memSize = static_cast<int64_t>(minStride) * frame.height_;
    CHECK_AND_RETURN_RET_LOG(memSize <= maxAllowedSize, nullptr, "alloc heap size too large");

    uint8_t *heap = new (std::nothrow) uint8_t[memSize];
    CHECK_AND_RETURN_RET_LOG(heap != nullptr, nullptr, "alloc heap failed");

    ON_SCOPE_EXIT(1) { delete [] heap; };

    uint8_t *currDstPos = heap;
    uint8_t *currSrcPos = frame.GetFlattenedData();
    for (int32_t row = 0; row < frame.height_; ++row) {
        errno_t rc = memcpy_s(currDstPos, static_cast<size_t>(memSize), currSrcPos, static_cast<size_t>(minStride));
        CHECK_AND_RETURN_RET_LOG(rc == EOK, nullptr, "memcpy_s failed");

        currDstPos += minStride;
        currSrcPos += frame.stride_;
        memSize -= minStride;
    }

    holder->isShmem = false;
    holder->heap = heap;

    CANCEL_SCOPE_EXIT_GUARD(0);
    CANCEL_SCOPE_EXIT_GUARD(1);
    return holder;
}

static std::shared_ptr<PixelMap> CreatePixelMap(const std::shared_ptr<AVSharedMemory> &mem, PixelFormat color)
{
    CHECK_AND_RETURN_RET_LOG(mem != nullptr, nullptr, "Fetch frame failed");
    CHECK_AND_RETURN_RET_LOG(mem->GetBase() != nullptr, nullptr, "Addr is nullptr");
    CHECK_AND_RETURN_RET_LOG(mem->GetSize() > 0, nullptr, "size is incorrect");
    CHECK_AND_RETURN_RET_LOG(static_cast<uint32_t>(mem->GetSize()) >= sizeof(OutputFrame),
                             nullptr, "size is incorrect");

    OutputFrame *frame = reinterpret_cast<OutputFrame *>(mem->GetBase());
    MEDIA_LOGD("width: %{public}d, stride : %{public}d, height: %{public}d, size: %{public}d, format: %{public}d",
        frame->width_, frame->stride_, frame->height_, frame->size_, color);

    InitializationOptions opts;
    opts.size.width = frame->width_;
    opts.size.height = frame->height_;
    opts.pixelFormat = color;
    opts.editable = true;
    std::shared_ptr<PixelMap> pixelMap = PixelMap::Create(opts);

    CHECK_AND_RETURN_RET_LOG(pixelMap != nullptr, nullptr, "pixelMap create failed");
    CHECK_AND_RETURN_RET_LOG(pixelMap->GetByteCount() <= frame->size_, nullptr, "Size inconsistent !");

    PixelMapMemHolder *holder = CreatePixelMapData(mem, *frame);
    CHECK_AND_RETURN_RET_LOG(holder != nullptr, nullptr, "create pixel map data failed");

    pixelMap->SetPixelsAddr(holder->heap, holder, static_cast<uint32_t>(pixelMap->GetByteCount()),
        AllocatorType::CUSTOM_ALLOC, FreePixelMapData);
    return pixelMap;
}

std::shared_ptr<AVMetadataHelper> AVMetadataHelperFactory::CreateAVMetadataHelper()
{
    std::shared_ptr<AVMetadataHelperImpl> impl = std::make_shared<AVMetadataHelperImpl>();
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, nullptr, "failed to new AVMetadataHelperImpl");

    int32_t ret = impl->Init();
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, nullptr, "failed to init AVMetadataHelperImpl");

    return impl;
}

int32_t AVMetadataHelperImpl::Init()
{
    avMetadataHelperAdapter_ = [[AVMetadataHelperAdapter alloc] init];
    CHECK_AND_RETURN_RET_LOG(avMetadataHelperAdapter_ != nullptr, MSERR_NO_MEMORY,
        "failed to create avmetadatahelper service");
    return MSERR_OK;
}

AVMetadataHelperImpl::AVMetadataHelperImpl()
{
    MEDIA_LOGD("AVMetadataHelperImpl:0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVMetadataHelperImpl::~AVMetadataHelperImpl()
{
    MEDIA_LOGD("AVMetadataHelperImpl:0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
    avMetadataHelperAdapter_ = nil;
}

int32_t AVMetadataHelperImpl::SetHelperCallback(const std::shared_ptr<HelperCallback> &callback)
{
    MEDIA_LOGD("AVMetadataHelperImpl:0x%{public}06" PRIXPTR " SetHelperCallback in", FAKE_POINTER(this));
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, MSERR_INVALID_VAL, "callback is nullptr");
    CHECK_AND_RETURN_RET_LOG(avMetadataHelperAdapter_ != nullptr, MSERR_INVALID_VAL,
        "avMetadataHelperAdapter_ is nullptr");
    [avMetadataHelperAdapter_ setHelperCallback:callback];
    return MSERR_OK;
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
    CHECK_AND_RETURN_RET_LOG(avMetadataHelperAdapter_ != nullptr, MSERR_INVALID_VAL,
        "avMetadataHelperAdapter_ is nullptr");
    [avMetadataHelperAdapter_ setSource:fd];
    return MSERR_OK;
}

int32_t AVMetadataHelperImpl::SetSource(const std::shared_ptr<IMediaDataSource> &dataSrc)
{
    MEDIA_LOGD("AVMetadataHelperImpl:0x%{public}06" PRIXPTR " SetSource in(dataSrc)", FAKE_POINTER(this));
    CHECK_AND_RETURN_RET_LOG(dataSrc != nullptr, MSERR_INVALID_VAL, "failed to create data source");

    return MSERR_OK;
}

void  AVMetadataHelperImpl::SetScene(Scene scene) {}

std::string AVMetadataHelperImpl::ResolveMetadata(int32_t key)
{
    return "";
}

std::unordered_map<int32_t, std::string> AVMetadataHelperImpl::ResolveMetadata()
{
    CHECK_AND_RETURN_RET_LOG(avMetadataHelperAdapter_ != nullptr, {},
        "avmetadatahelperadapter does not exist.");
    return [avMetadataHelperAdapter_ resolveMetadata];
}

std::shared_ptr<AVSharedMemory> AVMetadataHelperImpl::FetchArtPicture()
{
    CHECK_AND_RETURN_RET_LOG(avMetadataHelperAdapter_ != nullptr, nullptr,
        "avmetadatahelperadapter does not exist.");
    return [avMetadataHelperAdapter_ fetchArtPicture];
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
    CHECK_AND_RETURN_LOG(avMetadataHelperAdapter_ != nullptr, "avmetadatahelper service does not exist.");
    [avMetadataHelperAdapter_ releaseAVMetadataHelperAdapter];
}
} // namespace Media
} // namespace OHOS
