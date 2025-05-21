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

#ifndef MOCK_MOCK_SURFACE_H
#define MOCK_MOCK_SURFACE_H

#include <atomic>
#include <map>
#include <string>

#include <surface.h>
#include <ibuffer_producer.h>

namespace OHOS {
class MockSurface : public Surface {
public:
    MockSurface() {};

    GSError GetProducerInitInfo(ProducerInitInfo &info) override
    {
        return GSERROR_NOT_SUPPORT;
    }

    bool IsConsumer() const override
    {
        return true;
    }

    sptr<IBufferProducer> GetProducer() const override
    {
        return nullptr;
    }

    GSError RequestBuffer(sptr<SurfaceBuffer>& buffer,
                          int32_t &fence, BufferRequestConfig &config) override
    {
        return GSERROR_NOT_SUPPORT;
    }

    GSError RequestBuffers(std::vector<sptr<SurfaceBuffer>> &buffers,
        std::vector<sptr<SyncFence>> &fences, BufferRequestConfig &config)
    {
        return GSERROR_NOT_SUPPORT;
    }

    GSError CancelBuffer(sptr<SurfaceBuffer>& buffer) override
    {
        return GSERROR_NOT_SUPPORT;
    }

    GSError FlushBuffer(sptr<SurfaceBuffer>& buffer,
                        int32_t fence, BufferFlushConfig &config) override
    {
        return GSERROR_NOT_SUPPORT;
    }

    GSError AcquireBuffer(sptr<SurfaceBuffer>& buffer, int32_t &fence,
                          int64_t &timestamp, Rect &damage) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError ReleaseBuffer(sptr<SurfaceBuffer>& buffer, int32_t fence) override
    {
        return GSERROR_NOT_SUPPORT;
    }

    GSError RequestBuffer(sptr<SurfaceBuffer>& buffer,
                          sptr<SyncFence>& fence, BufferRequestConfig &config) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError FlushBuffer(sptr<SurfaceBuffer>& buffer,
                        const sptr<SyncFence>& fence, BufferFlushConfig &config) override
    {
        return GSERROR_NOT_SUPPORT;
    }

    GSError FlushBuffers(const std::vector<sptr<SurfaceBuffer>> &buffers,
        const std::vector<sptr<SyncFence>> &fences, const std::vector<BufferFlushConfigWithDamages> &configs) override
    {
        return GSERROR_NOT_SUPPORT;
    }

    GSError GetLastFlushedBuffer(sptr<SurfaceBuffer>& buffer,
                                  sptr<SyncFence>& fence, float matrix[16], bool isUseNewMatrix = false) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError FlushBuffer(sptr<SurfaceBuffer>& buffer, const sptr<SyncFence>& fence,
                        BufferFlushConfigWithDamages &config, bool needLock = true) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError AcquireBuffer(sptr<SurfaceBuffer>& buffer, sptr<SyncFence>& fence,
                          int64_t &timestamp, Rect &damage) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError ReleaseBuffer(sptr<SurfaceBuffer>& buffer, const sptr<SyncFence>& fence) override
    {
        return GSERROR_NOT_SUPPORT;
    }

    GSError AttachBuffer(sptr<SurfaceBuffer>& buffer) override
    {
        return GSERROR_NOT_SUPPORT;
    }

    GSError DetachBuffer(sptr<SurfaceBuffer>& buffer) override
    {
        return GSERROR_NOT_SUPPORT;
    }

    bool QueryIfBufferAvailable() override
    {
        return false;
    }

    uint32_t GetQueueSize() override
    {
        return 0;
    }
    GSError SetQueueSize(uint32_t queueSize) override
    {
        return GSERROR_NOT_SUPPORT;
    }

    const std::string& GetName() override
    {
        return notSupport;
    }
    uint64_t GetUniqueId() const override
    {
        return queueId_;
    }

    void SetUniqueId(uint64_t id)
    {
        queueId_ = id;
    }

    GSError SetDefaultWidthAndHeight(int32_t width, int32_t height) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    int32_t GetDefaultWidth() override
    {
        return -1;
    }
    int32_t GetDefaultHeight() override
    {
        return -1;
    }
    GSError SetDefaultUsage(uint64_t usage) override
    {
        return GSERROR_NOT_SUPPORT;
    }

    uint64_t GetDefaultUsage() override
    {
        return -1;
    }

    GSError SetUserData(const std::string &key, const std::string &val) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    std::string GetUserData(const std::string &key) override
    {
        return "";
    }

    GSError RegisterConsumerListener(sptr<IBufferConsumerListener>& listener) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError RegisterConsumerListener(IBufferConsumerListenerClazz *listener) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError RegisterReleaseListener(OnReleaseFunc func) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError UnRegisterReleaseListener() override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError RegisterDeleteBufferListener(OnDeleteBufferFunc func, bool isForUniRedraw = false) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError UnregisterConsumerListener() override
    {
        return GSERROR_NOT_SUPPORT;
    }

    void Dump(std::string &result) const override {};

    // Call carefully. This interface will empty all caches of the current process
    GSError CleanCache(bool cleanAll = false) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError GoBackground() override
    {
        return GSERROR_NOT_SUPPORT;
    }

    GSError SetTransform(GraphicTransformType transform) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GraphicTransformType GetTransform() const override
    {
        return GraphicTransformType::GRAPHIC_ROTATE_NONE;
    }

    GSError Connect() override
    {
        return GSERROR_NOT_SUPPORT;
    }
    
    GSError Disconnect() override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError SetScalingMode(uint32_t sequence, ScalingMode scalingMode) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError GetScalingMode(uint32_t sequence, ScalingMode &scalingMode) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError SetMetaData(uint32_t sequence, const std::vector<GraphicHDRMetaData> &metaData) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError SetMetaDataSet(uint32_t sequence, GraphicHDRMetadataKey key, const std::vector<uint8_t> &metaData) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError QueryMetaDataType(uint32_t sequence, HDRMetaDataType &type) const override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError GetMetaData(uint32_t sequence, std::vector<GraphicHDRMetaData> &metaData) const override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError GetMetaDataSet(uint32_t sequence, GraphicHDRMetadataKey &key,
                           std::vector<uint8_t> &metaData) const override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError SetTunnelHandle(const GraphicExtDataHandle *handle) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    sptr<SurfaceTunnelHandle> GetTunnelHandle() const override
    {
        return nullptr;
    }
    GSError SetPresentTimestamp(uint32_t sequence, const GraphicPresentTimestamp &timestamp) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError GetPresentTimestamp(uint32_t sequence, GraphicPresentTimestampType type, int64_t &time) const override
    {
        return GSERROR_NOT_SUPPORT;
    }

    void SetInstanceId(int32_t id)
    {
        instanceId_ = id;
    }

    int32_t GetDefaultFormat() override
    {
        return instanceId_;
    }
    GSError SetDefaultFormat(int32_t format) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    int32_t GetDefaultColorGamut() override
    {
        return -1;
    }
    GSError SetDefaultColorGamut(int32_t colorGamut) override
    {
        return GSERROR_NOT_SUPPORT;
    }

    sptr<NativeSurface> GetNativeSurface() override
    {
        return nullptr;
    }
    GSError SetWptrNativeWindowToPSurface(void* nativeWindow) override
    {
        return GSERROR_NOT_SUPPORT;
    }

    GSError AttachBuffer(sptr<SurfaceBuffer>& buffer, int32_t timeOut) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError RegisterSurfaceDelegator(sptr<IRemoteObject> client) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError RegisterReleaseListener(OnReleaseFuncWithFence func) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError RegisterUserDataChangeListener(const std::string &funcName, OnUserDataChangeFunc func) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError UnRegisterUserDataChangeListener(const std::string &funcName) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    GSError ClearUserDataChangeListener() override
    {
        return GSERROR_NOT_SUPPORT;
    }

    GSError AttachBufferToQueue(sptr<SurfaceBuffer> buffer) override
    {
        return GSERROR_NOT_SUPPORT;
    }

    GSError DetachBufferFromQueue(sptr<SurfaceBuffer> buffer, bool isReserveSlot = false) override
    {
        return GSERROR_NOT_SUPPORT;
    }

    GraphicTransformType GetTransformHint() const override
    {
        return GraphicTransformType::GRAPHIC_ROTATE_BUTT;
    }

    GSError SetTransformHint(GraphicTransformType transformHint) override
    {
        return GSERROR_NOT_SUPPORT;
    }

    void SetRequestWidthAndHeight(int32_t width, int32_t height) override {}

    int32_t GetRequestWidth() override
    {
        return 0;
    }

    int32_t GetRequestHeight() override
    {
        return 0;
    }

    GSError SetSurfaceSourceType(OHSurfaceSource sourceType) override
    {
        return GSERROR_NOT_SUPPORT;
    }

    OHSurfaceSource GetSurfaceSourceType() const  override
    {
        return OH_SURFACE_SOURCE_DEFAULT;
    }

    GSError SetSurfaceAppFrameworkType(std::string appFrameworkType)  override
    {
        return GSERROR_NOT_SUPPORT;
    }

    std::string GetSurfaceAppFrameworkType() const override
    {
        return "";
    }

    void SetBufferHold(bool hold) override {}

    GSError SetScalingMode(ScalingMode scalingMode)  override
    {
        return GSERROR_NOT_SUPPORT;
    }

    void SetWindowConfig(const BufferRequestConfig& config) override {}

    BufferRequestConfig GetWindowConfig() override
    {
        BufferRequestConfig config;
        return config;
    }
    
    GSError SetHdrWhitePointBrightness(float brightness) override
    {
        return GSERROR_NOT_SUPPORT;
    }
    
    GSError SetSdrWhitePointBrightness(float brightness) override
    {
        return GSERROR_NOT_SUPPORT;
    }

    GSError AcquireLastFlushedBuffer(sptr<SurfaceBuffer> &buffer, sptr<SyncFence> &fence,
        float matrix[16], uint32_t matrixSize, bool isUseNewMatrix) override
    {
        return GSERROR_NOT_SUPPORT;
    }

    GSError ReleaseLastFlushedBuffer(sptr<SurfaceBuffer> buffer)  override
    {
        return GSERROR_NOT_SUPPORT;
    }

    GSError SetGlobalAlpha(int32_t alpha)  override
    {
        return GSERROR_NOT_SUPPORT;
    }
private:
    uint64_t queueId_ = 0;
    int32_t instanceId_ = -1;
    std::string notSupport = "GSERROR_NOT_SUPPORT";
};
} // namespace OHOS
#endif // MOCK_MOCK_SURFACE_H
