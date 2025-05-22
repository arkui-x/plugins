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

#ifndef PLUGINS_COMPONENT_PLATFORMVIEW_PLATFORMVIEW_PATTERN_H
#define PLUGINS_COMPONENT_PLATFORMVIEW_PLATFORMVIEW_PATTERN_H

#include <string>

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/common/thread_checker.h"
#include "core/components_ng/event/focus_hub.h"
#include "core/components_ng/event/input_event.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/render_surface.h"
#include "platform_view_interface.h"
#include "platformview_event_hub.h"
#include "platformview_layout_algorithm.h"
#include "platformview_layout_property.h"

namespace OHOS::Ace::NG {
class PlatformViewPattern : public Pattern {
    DECLARE_ACE_TYPE(PlatformViewPattern, Pattern);

public:
    PlatformViewPattern() = default;
    PlatformViewPattern(const std::string& id);
    ~PlatformViewPattern() override = default;

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<PlatformViewLayoutProperty>();
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<PlatformViewEventHub>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<PlatformViewLayoutAlgorithm>();
    }

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::NODE, false };
    }

    void PlatformViewSizeInit();
    void PlatformViewSizeChange(const RectF& surfaceRect, bool needFireNativeEvent);
    void FireExternalEvent(RefPtr<NG::PipelineContext> context, const std::string& componentId, const uint32_t nodeId,
        const bool isDestroy);
    void UpdateSurfaceBounds(bool needForceRender, bool frameOffsetChange = false);

private:
    void OnAttachToFrameNode() override;
    void OnDetachFromFrameNode(FrameNode* frameNode) override;
    void BeforeSyncGeometryProperties(const DirtySwapConfig& config) override;
    void OnRebuildFrame() override;
    void OnAreaChangedInner() override {}
    void OnWindowHide() override {}
    void OnWindowShow() override {}
    void OnModifyDone() override;
    void DumpInfo() override;
    void DumpAdvanceInfo() override;
    void OnAttachContext(PipelineContext* context) override;
    void OnDetachContext(PipelineContext* context) override;

    void InitEvent();
    void InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub);
    void HandleTouchEvent(const TouchEventInfo& info);
    void InitFocusEvent(const RefPtr<FocusHub>& focusHub);
    ExternalEvent CreateExternalEvent();

    void RequestFocus();
    void PlatformViewInitialize();
    void PlatformViewAddCallBack();
    void* GetNativeWindow(int32_t instanceId, int64_t textureId);
    void OnTextureRefresh(void* surface);
    void PrepareSurface();
    void RegisterPlatformViewEvent();
    void PlatformViewDispatchTouchEvent(const TouchLocationInfo& changedPoint);
    void UpdatePlatformViewLayoutIfNeeded();

    std::string id_;
    RefPtr<RenderSurface> renderSurface_;
    WeakPtr<RenderSurface> renderSurfaceWeakPtr_;
    RefPtr<RenderContext> renderContextForPlatformView_;
    WeakPtr<RenderContext> renderContextForPlatformViewWeakPtr_;
    RefPtr<PlatformViewInterface> platformView_;
    WeakPtr<PlatformViewInterface> platformViewWeakPtr_;
    SizeF lastDrawSize_;
    OffsetF lastOffset_;
    bool hasPlatformViewInit_ = false;
    bool isTextureReady = false;
    RefPtr<TouchEventImpl> touchEvent_;
    OffsetF localPosition_;
    SizeF drawSize_;
    SizeF surfaceSize_;
};
} // namespace OHOS::Ace::NG
#endif // PLUGINS_COMPONENT_PLATFORMVIEW_PLATFORMVIEW_PATTERN_H
