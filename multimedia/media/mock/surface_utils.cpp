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

#include <surface_utils.h>
#include <cinttypes>
#include <array>
#include "securec.h"
#include "buffer_log.h"
#include "mock_surface.h"
#include "plugins/interfaces/native/inner_api/plugin_utils_inner.h"

namespace OHOS {
using namespace HiviewDFX;
constexpr int64_t TRANSFORM_MATRIX_ELE_COUNT = 16;

SurfaceUtils* SurfaceUtils::GetInstance()
{
    static SurfaceUtils instance;
    return &instance;
}

SurfaceUtils::~SurfaceUtils()
{
    surfaceCache_.clear();
}

sptr<Surface> SurfaceUtils::GetSurface(uint64_t uniqueId)
{
    auto surface = sptr(new MockSurface());
    surface->SetUniqueId(uniqueId);
    surface->SetInstanceId(Plugin::PluginUtilsInner::GetInstanceId());
    return surface;
}

SurfaceError SurfaceUtils::Add(uint64_t uniqueId, const wptr<Surface> &surface)
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    if (surface == nullptr) {
        BLOGE(" surface is nullptr.");
        return GSERROR_INVALID_ARGUMENTS;
    }
    if (surfaceCache_.count(uniqueId) == 0) {
        surfaceCache_[uniqueId] = surface;
        return GSERROR_OK;
    }
    BLOGW("the surface by uniqueId %" PRIu64 " already existed", uniqueId);
    return GSERROR_OK;
}

SurfaceError SurfaceUtils::Remove(uint64_t uniqueId)
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    if (surfaceCache_.count(uniqueId) == 0) {
        BLOGE("Delete failed without surface by uniqueId %" PRIu64, uniqueId);
        return GSERROR_INVALID_OPERATING;
    }
    surfaceCache_.erase(uniqueId);
    return GSERROR_OK;
}

std::array<float, 16> SurfaceUtils::MatrixProduct(const std::array<float, 16>& lMat, const std::array<float, 16>& rMat)
{
    // Product matrix 4 * 4 = 16
    return std::array<float, 16> {lMat[0] * rMat[0] + lMat[4] * rMat[1] + lMat[8] * rMat[2] + lMat[12] * rMat[3],
                                  lMat[1] * rMat[0] + lMat[5] * rMat[1] + lMat[9] * rMat[2] + lMat[13] * rMat[3],
                                  lMat[2] * rMat[0] + lMat[6] * rMat[1] + lMat[10] * rMat[2] + lMat[14] * rMat[3],
                                  lMat[3] * rMat[0] + lMat[7] * rMat[1] + lMat[11] * rMat[2] + lMat[15] * rMat[3],

                                  lMat[0] * rMat[4] + lMat[4] * rMat[5] + lMat[8] * rMat[6] + lMat[12] * rMat[7],
                                  lMat[1] * rMat[4] + lMat[5] * rMat[5] + lMat[9] * rMat[6] + lMat[13] * rMat[7],
                                  lMat[2] * rMat[4] + lMat[6] * rMat[5] + lMat[10] * rMat[6] + lMat[14] * rMat[7],
                                  lMat[3] * rMat[4] + lMat[7] * rMat[5] + lMat[11] * rMat[6] + lMat[15] * rMat[7],

                                  lMat[0] * rMat[8] + lMat[4] * rMat[9] + lMat[8] * rMat[10] + lMat[12] * rMat[11],
                                  lMat[1] * rMat[8] + lMat[5] * rMat[9] + lMat[9] * rMat[10] + lMat[13] * rMat[11],
                                  lMat[2] * rMat[8] + lMat[6] * rMat[9] + lMat[10] * rMat[10] + lMat[14] * rMat[11],
                                  lMat[3] * rMat[8] + lMat[7] * rMat[9] + lMat[11] * rMat[10] + lMat[15] * rMat[11],

                                  lMat[0] * rMat[12] + lMat[4] * rMat[13] + lMat[8] * rMat[14] + lMat[12] * rMat[15],
                                  lMat[1] * rMat[12] + lMat[5] * rMat[13] + lMat[9] * rMat[14] + lMat[13] * rMat[15],
                                  lMat[2] * rMat[12] + lMat[6] * rMat[13] + lMat[10] * rMat[14] + lMat[14] * rMat[15],
                                  lMat[3] * rMat[12] + lMat[7] * rMat[13] + lMat[11] * rMat[14] + lMat[15] * rMat[15]};
}

void SurfaceUtils::ComputeTransformMatrix(float matrix[16], uint32_t matrixSize,
    sptr<SurfaceBuffer>& buffer, GraphicTransformType& transform, const Rect& crop)
{
    const std::array<float, TRANSFORM_MATRIX_ELE_COUNT> rotate90 = {0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1};
    std::array<float, TRANSFORM_MATRIX_ELE_COUNT> transformMatrix = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    float tx = 0.f;
    float ty = 0.f;
    float sx = 1.f;
    float sy = 1.f;
    if (transform == GraphicTransformType::GRAPHIC_ROTATE_90) {
        transformMatrix = MatrixProduct(transformMatrix, rotate90);
    }
    float bufferWidth = buffer->GetWidth();
    float bufferHeight = buffer->GetHeight();
    if (crop.w < bufferWidth && bufferWidth != 0) {
        tx = (float(crop.x) / bufferWidth);
        sx = (float(crop.w) / bufferWidth);
    }
    if (crop.h < bufferHeight && bufferHeight != 0) {
        ty = (float(bufferHeight - crop.y) / bufferHeight);
        sy = (float(crop.h) / bufferHeight);
    }
    static const std::array<float, 16> cropMatrix = {sx, 0, 0, 0, 0, sy, 0, 0, 0, 0, 1, 0, tx, ty, 0, 1};
    transformMatrix = MatrixProduct(cropMatrix, transformMatrix);

    auto ret = memcpy_s(matrix, sizeof(transformMatrix),
                        transformMatrix.data(), sizeof(transformMatrix));
    if (ret != EOK) {
        BLOGE("ComputeTransformMatrix: transformMatrix memcpy_s failed");
    }
}
} // namespace OHOS
