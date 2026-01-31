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

#ifndef PLUGINS_COMPONENT_PLATFORMVIEW_PLATFORM_VIEW_INTERFACE_H
#define PLUGINS_COMPONENT_PLATFORMVIEW_PLATFORM_VIEW_INTERFACE_H

#include <cstdint>

#include "base/memory/ace_type.h"
#include "core/components_ng/render/render_surface.h"

namespace OHOS::Ace::NG {
using CommonEvent = std::function<void()>;
using TextureRefreshEvent = std::function<void(uint32_t, uint64_t)>;
using PlatformViewReadyEvent = std::function<void()>;

enum class PlatformViewType {
    TEXTURE_TYPE = 0,
    SURFACE_TYPE = 1,
};

class PlatformViewInterface : public virtual AceType {
    DECLARE_ACE_TYPE(NG::PlatformViewInterface, AceType)

public:
    PlatformViewInterface() = default;
    ~PlatformViewInterface() override = default;

    static RefPtr<PlatformViewInterface> Create();

    virtual void InitPlatformView() {}
    virtual void SetScale(float x, float y, float z, const std::string& centerX, const std::string& centerY) {}
    virtual void SetRotation(float x, float y, float z, const std::string& angle, const std::string& centerX,
        const std::string& centerY, const std::string& centerZ, const std::string& perspective) {}
    virtual void SetTranslate(const std::string& x, const std::string& y, const std::string& z) {}
    virtual void SetTransformMatrix(const std::vector<float>& matrix) {}
    virtual void UpdatePlatformViewLayout(const NG::SizeF& drawSize, const NG::OffsetF& offset) {}
    virtual void ExchangeBind() {}
    virtual void GetPlatformViewType(int32_t& type) {}
    virtual void HandleTouchDown(const NG::OffsetF& offset) {}
    virtual void HandleTouchUp(const NG::OffsetF& offset) {}
    virtual void HandleTouchMove(const NG::OffsetF& offset) {}
    virtual void HandleTouchCancel(const NG::OffsetF& offset) {}

    virtual void SetRenderSurface(const RefPtr<RenderSurface>& renderSurface) {}
    virtual void RegisterTextureEvent(TextureRefreshEvent&& textureRefreshEvent) {}
    virtual void RegisterPlatformViewReadyEvent(PlatformViewReadyEvent&& platformViewReadyEvent) {}
    virtual void RegisterPlatformView() {}
    virtual int32_t SetSurface()
    {
        return -1;
    }
    virtual void Dispose() {}
protected:

    ACE_DISALLOW_COPY_AND_MOVE(PlatformViewInterface);
};
} // namespace OHOS::Ace::NG

#endif // PLUGINS_COMPONENT_PLATFORMVIEW_PLATFORM_VIEW_INTERFACE_H
