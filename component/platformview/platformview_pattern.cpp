/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "platformview_pattern.h"

#include <cmath>
#include <cstdlib>

#include "platform_view_impl.h"
#include "base/geometry/ng/point_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/log/dump_log.h"
#include "base/log/frame_report.h"
#include "base/log/log_wrapper.h"
#include "base/memory/ace_type.h"
#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/common/ace_engine.h"
#include "core/common/ace_view.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/event/touch_event.h"
#include "core/pipeline_ng/pipeline_context.h"
#ifdef NG_BUILD
#include "bridge/declarative_frontend/ng/declarative_frontend_ng.h"
#else
#include "bridge/declarative_frontend/declarative_frontend.h"
#endif
#include "platformview_event_hub.h"

namespace OHOS::Ace::NG {
PlatformViewPattern::PlatformViewPattern(const std::string& id, const std::optional<std::string>& data)
    : id_(id), data_(data) {}
void PlatformViewPattern::RequestFocus()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<PlatformViewEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto focusHub = eventHub->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);

    focusHub->RequestFocusImmediately();
}

void* PlatformViewPattern::GetNativeWindow(int32_t instanceId, int64_t textureId)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    CHECK_NULL_RETURN(container, nullptr);
    auto nativeView = container->GetAceView();
    CHECK_NULL_RETURN(nativeView, nullptr);
    return const_cast<void*>(nativeView->GetNativeWindowById(textureId));
}

void PlatformViewPattern::OnTextureRefresh(void* surface)
{
    CHECK_NULL_VOID(surface);
    auto renderContextForPlatformView = renderContextForPlatformViewWeakPtr_.Upgrade();
    CHECK_NULL_VOID(renderContextForPlatformView);
    renderContextForPlatformView->MarkNewFrameAvailable(surface);
    UpdatePlatformViewLayoutIfNeeded();
#ifdef IOS_PLATFORM
    isTextureReady = true;
#endif
}

void PlatformViewPattern::RegisterPlatformViewEvent()
{
    CHECK_NULL_VOID(platformView_);
    ContainerScope scope(GetHostInstanceId());
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);

    auto uiTaskExecutor = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::UI);
    auto pvPattern = WeakClaim(this);

    auto&& textureRefreshEvent = [pvPattern, uiTaskExecutor](int32_t instanceId, int64_t textureId) {
        uiTaskExecutor.PostSyncTask(
            [&pvPattern, instanceId, textureId] {
                auto platformViewPattern = pvPattern.Upgrade();
                CHECK_NULL_VOID(platformViewPattern);
                void* nativeWindow = platformViewPattern->GetNativeWindow(instanceId, textureId);
                if (!nativeWindow) {
                    LOGE("the native window is nullptr.");
                    return;
                }
                platformViewPattern->OnTextureRefresh(nativeWindow);
            },
            "ArkUIPlatformViewPatternTextureRefreshEvent");
    };
    platformView_->RegisterTextureEvent(textureRefreshEvent);

    auto&& platformViewReadyEvent = [pvPattern, uiTaskExecutor]() {
        uiTaskExecutor.PostSyncTask(
            [&pvPattern] {
                auto platformViewPattern = pvPattern.Upgrade();
                CHECK_NULL_VOID(platformViewPattern);
                auto host = platformViewPattern->GetHost();
                CHECK_NULL_VOID(host);
                host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
            },
            "ArkUIPlatformViewPatternPlatformViewReadyEvent");
    };
    platformView_->RegisterPlatformViewReadyEvent(platformViewReadyEvent);
}

void PlatformViewPattern::PrepareSurface()
{
    if (!platformView_ || renderSurface_->IsSurfaceValid()) {
        return;
    }
    if (!SystemProperties::GetExtSurfaceEnabled()) {
        renderSurface_->SetRenderContext(renderContextForPlatformView_);
    }
    renderSurface_->InitSurface();
    platformView_->SetRenderSurface(renderSurface_);
}

void PlatformViewPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    pipeline->AddWindowStateChangedCallback(host->GetId());
    if (FrameReport::GetInstance().GetEnable()) {
        FrameReport::GetInstance().EnableSelfRender();
    }
    PlatformViewInitialize();
}

void PlatformViewPattern::PlatformViewInitialize()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    platformView_ = AceType::MakeRefPtr<NG::PlatformViewImpl>(id_, data_);
    platformView_->InitPlatformView();
    platformViewWeakPtr_ = platformView_;
    renderSurface_ = RenderSurface::Create();
    renderSurface_->SetInstanceId(GetHostInstanceId());
    renderContextForPlatformView_ = RenderContext::Create();
    InitEvent();
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    static RenderContext::ContextParam param = { RenderContext::ContextType::HARDWARE_TEXTURE, "PlatformViewSurface",
        RenderContext::PatternType::PLATFORM_VIEW };
    renderContextForPlatformView_->InitContext(false, param);
    renderSurfaceWeakPtr_ = renderSurface_;
    renderContextForPlatformViewWeakPtr_ = renderContextForPlatformView_;

    PlatformViewAddCallBack();
 
    renderContext->UpdateBackgroundColor(Color::WHITE);
    renderContextForPlatformView_->UpdateBackgroundColor(Color::WHITE);
    renderContext->SetClipToBounds(true);
}

void PlatformViewPattern::PlatformViewAddCallBack()
{
    auto OnAttachCallBack = [weak = WeakClaim(this)](int64_t textureId, bool isAttach) mutable {
        auto platformViewPattern = weak.Upgrade();
        CHECK_NULL_VOID(platformViewPattern);
        if (auto renderSurface = platformViewPattern->renderSurfaceWeakPtr_.Upgrade(); renderSurface) {
            renderSurface->AttachToGLContext(textureId, isAttach);
        }
    };
    renderContextForPlatformView_->AddAttachCallBack(OnAttachCallBack);
    auto OnUpdateCallBack = [weak = WeakClaim(this)](std::vector<float>& matrix) mutable {
        auto platformViewPattern = weak.Upgrade();
        CHECK_NULL_VOID(platformViewPattern);
#if defined(ANDROID_PLATFORM)
        if (auto renderSurface = platformViewPattern->renderSurfaceWeakPtr_.Upgrade(); renderSurface) {
            renderSurface->UpdateTextureImage(matrix);
        }
#endif
#if defined(IOS_PLATFORM)
        if (auto platformView = platformViewPattern->platformViewWeakPtr_.Upgrade(); platformView) {
            platformView->ExchangeBind();
        }
#endif
    };
    renderContextForPlatformView_->AddUpdateCallBack(OnUpdateCallBack);
#if defined(IOS_PLATFORM)
    auto OnInitTypeCallback = [weak = WeakClaim(this)](int32_t& type) mutable {
        auto platformViewPattern = weak.Upgrade();
        CHECK_NULL_VOID(platformViewPattern);
        if (auto platformView = platformViewPattern->platformViewWeakPtr_.Upgrade(); platformView) {
            platformView->GetPlatformViewType(type);
        }
    };
    renderContextForPlatformView_->AddInitTypeCallBack(OnInitTypeCallback);
#endif
}

void PlatformViewPattern::OnModifyDone()
{
    ContainerScope scope(GetHostInstanceId());
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto platformTask = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::BACKGROUND);
    platformTask.PostTask(
        [weak = WeakClaim(this)] {
            auto platformViewPattern = weak.Upgrade();
            CHECK_NULL_VOID(platformViewPattern);
            platformViewPattern->RegisterPlatformViewEvent();
            platformViewPattern->PrepareSurface();
        },
        "ArkUIPlatformViewPatternOnModifyDone");
}

void PlatformViewPattern::OnRebuildFrame()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    CHECK_NULL_VOID(renderContextForPlatformView_);
    renderContext->AddChild(renderContextForPlatformView_, 0);
}

void PlatformViewPattern::OnDetachFromFrameNode(FrameNode* frameNode)
{
    CHECK_NULL_VOID(frameNode);
    if (!hasPlatformViewInit_) {
        return;
    }
    auto id = frameNode->GetId();
    auto pipeline = AceType::DynamicCast<PipelineContext>(PipelineBase::GetCurrentContext());
    CHECK_NULL_VOID(pipeline);
    pipeline->RemoveWindowStateChangedCallback(id);
    if (FrameReport::GetInstance().GetEnable()) {
        FrameReport::GetInstance().DisableSelfRender();
    }
}

void PlatformViewPattern::OnAttachContext(PipelineContext* context)
{
    CHECK_NULL_VOID(context);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    context->AddWindowStateChangedCallback(host->GetId());
    CHECK_NULL_VOID(renderSurface_);
    renderSurface_->SetInstanceId(context->GetInstanceId());
}

void PlatformViewPattern::OnDetachContext(PipelineContext* context)
{
    CHECK_NULL_VOID(context);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    context->RemoveWindowStateChangedCallback(host->GetId());
}

void PlatformViewPattern::BeforeSyncGeometryProperties(const DirtySwapConfig& config)
{
    if (config.skipMeasure) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    drawSize_ = geometryNode->GetContentSize();
    if (!drawSize_.IsPositive()) {
        LOGW("PlatformView[%{public}s]'s size is not positive", id_.c_str());
        return;
    }
    localPosition_ = geometryNode->GetContentOffset();
    if (!hasPlatformViewInit_) {
        hasPlatformViewInit_ = true;
    }
    UpdateSurfaceBounds(false, config.frameOffsetChange);
#ifdef IOS_PLATFORM
    isTextureReady = false;
#endif
    UpdatePlatformViewLayoutIfNeeded();
    host->MarkNeedSyncRenderTree();
}

void PlatformViewPattern::UpdatePlatformViewLayoutIfNeeded()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto transformRelativeOffset = host->GetTransformRelativeOffset();
    OffsetF offset = localPosition_ + transformRelativeOffset;
    if (lastDrawSize_ != drawSize_ || lastOffset_ != offset) {
        platformView_->UpdatePlatformViewLayout(drawSize_, offset);
#ifdef IOS_PLATFORM
        if (!isTextureReady) {
            return;
        }
#endif
        if (renderContextForPlatformView_) {
            renderContextForPlatformView_->SetBounds(
                localPosition_.GetX(), localPosition_.GetY(), drawSize_.Width(), drawSize_.Height());
        }
        if (SystemProperties::GetExtSurfaceEnabled()) {
            renderSurface_->SetExtSurfaceBounds(transformRelativeOffset.GetX() + localPosition_.GetX(),
                transformRelativeOffset.GetY() + localPosition_.GetY(), drawSize_.Width(), drawSize_.Height());
        }
        lastDrawSize_ = drawSize_;
        lastOffset_ = offset;
    }
}

void PlatformViewPattern::DumpInfo()
{
    DumpLog::GetInstance().AddDesc(std::string("paltformviewId: ").append(id_));
}

void PlatformViewPattern::DumpAdvanceInfo()
{
    DumpLog::GetInstance().AddDesc(
        std::string("surfaceRect: ").append(RectF { localPosition_, surfaceSize_ }.ToString()));
    if (renderSurface_) {
        renderSurface_->DumpInfo();
    }
}

void PlatformViewPattern::PlatformViewSizeChange(const RectF& surfaceRect, bool needFireNativeEvent)
{
    // do not trigger when the size is first initialized
    if (needFireNativeEvent) {
        ContainerScope scope(GetHostInstanceId());
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        auto viewScale = context->GetViewScale();
        renderSurface_->AdjustNativeWindowSize(static_cast<uint32_t>(surfaceRect.Width() * viewScale),
            static_cast<uint32_t>(surfaceRect.Height() * viewScale));
    }
    renderSurface_->UpdateSurfaceSizeInUserData(
        static_cast<uint32_t>(surfaceRect.Width()), static_cast<uint32_t>(surfaceRect.Height()));
}

void PlatformViewPattern::InitEvent()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<PlatformViewEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto gestureHub = eventHub->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    InitTouchEvent(gestureHub);
    auto focusHub = host->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    InitFocusEvent(focusHub);
}

void PlatformViewPattern::InitFocusEvent(const RefPtr<FocusHub>& focusHub)
{
    focusHub->SetFocusable(true);
}

void PlatformViewPattern::InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    CHECK_NULL_VOID(!touchEvent_);

    auto touchTask = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleTouchEvent(info);
    };

    touchEvent_ = MakeRefPtr<TouchEventImpl>(std::move(touchTask));
    gestureHub->AddTouchEvent(touchEvent_);
}

void PlatformViewPattern::HandleTouchEvent(const TouchEventInfo& info)
{
    auto touchInfoList = info.GetChangedTouches();
    if (touchInfoList.empty()) {
        return;
    }
    auto touchType = touchInfoList.front().GetTouchType();
    if (touchType == TouchType::DOWN) {
        RequestFocus();
    }
    const auto& changedPoint = touchInfoList.front();
    PlatformViewDispatchTouchEvent(changedPoint);
}

void PlatformViewPattern::PlatformViewDispatchTouchEvent(const TouchLocationInfo& changedPoint)
{
    CHECK_NULL_VOID(platformView_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto transformRelativeOffset = host->GetTransformRelativeOffset();
    NG::OffsetF pointOffset = localPosition_ + transformRelativeOffset;
    if (changedPoint.GetTouchType() == TouchType::DOWN) {
        platformView_->HandleTouchDown(pointOffset);
    } else if (changedPoint.GetTouchType() == TouchType::MOVE) {
        platformView_->HandleTouchMove(pointOffset);
    } else if (changedPoint.GetTouchType() == TouchType::UP) {
        platformView_->HandleTouchUp(pointOffset);
    } else if (changedPoint.GetTouchType() == TouchType::CANCEL) {
        platformView_->HandleTouchCancel(pointOffset);
    }
}

void PlatformViewPattern::UpdateSurfaceBounds(bool needForceRender, bool frameOffsetChange)
{
    if (!drawSize_.IsPositive()) {
        return;
    }
    auto preSurfaceSize = surfaceSize_;

    surfaceSize_ = drawSize_;

    if (preSurfaceSize != surfaceSize_) {
        PlatformViewSizeChange({ localPosition_, surfaceSize_ }, preSurfaceSize.IsPositive());
    }

    if (renderSurface_) {
        renderSurface_->SetSurfaceDefaultSize(
            static_cast<int32_t>(surfaceSize_.Width()), static_cast<int32_t>(surfaceSize_.Height()));
    }
    if (needForceRender) {
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        host->MarkNeedRenderOnly();
    }
}
} // namespace OHOS::Ace::NG
