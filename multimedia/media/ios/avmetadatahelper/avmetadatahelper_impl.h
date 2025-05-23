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
#ifndef AVMETADATAHELPER_IMPL_H
#define AVMETADATAHELPER_IMPL_H

#include "avmetadatahelper.h"
#include "nocopyable.h"
#import "avmetadatahelper_adapter.h"

namespace OHOS {
namespace Media {
class AVMetadataHelperImpl : public AVMetadataHelper, public NoCopyable {
public:
    AVMetadataHelperImpl();
    ~AVMetadataHelperImpl();

    int32_t SetSource(const std::string &uri, int32_t usage) override;
    int32_t SetSource(int32_t fd, int64_t offset, int64_t size, int32_t usage) override;
    int32_t SetSource(const std::shared_ptr<IMediaDataSource> &dataSrc) override;
    void SetScene(Scene scene) override;
    std::string ResolveMetadata(int32_t key) override;
    std::unordered_map<int32_t, std::string> ResolveMetadata() override;
    std::shared_ptr<AVSharedMemory> FetchArtPicture() override;
    std::shared_ptr<PixelMap> FetchFrameAtTime(int64_t timeUs, int32_t option, const PixelMapParams &param) override;
    std::shared_ptr<PixelMap> FetchFrameYuv(int64_t timeUs, int32_t option, const PixelMapParams &param) override;
    std::shared_ptr<PixelMap> FetchScaledFrameYuv(int64_t timeUs, int32_t option, const PixelMapParams &param) override;
    std::shared_ptr<Meta> GetAVMetadata() override;
    void Release() override;
    int32_t Init();
    int32_t SetHelperCallback(const std::shared_ptr<HelperCallback> &callback) override;
    int32_t GetTimeByFrameIndex(uint32_t index, uint64_t &time) override;
    int32_t GetFrameIndexByTime(uint64_t time, uint32_t &index) override;
private:
    AVMetadataHelperAdapter *avMetadataHelperAdapter_ = nil;
};
} // namespace Media
} // namespace OHOS
#endif // AVMETADATAHELPER_IMPL_H
