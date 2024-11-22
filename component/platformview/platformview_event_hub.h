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

#ifndef PLUGINS_COMPONENT_PLATFORMVIEW_PLATFORMVIEW_EVENT_HUB_H
#define PLUGINS_COMPONENT_PLATFORMVIEW_PLATFORMVIEW_EVENT_HUB_H

#include "base/memory/ace_type.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/event/gesture_event_hub.h"

namespace OHOS::Ace::NG {
using ExternalEvent = std::function<void(const std::string&, const uint32_t, const bool)>;
class PlatformViewEventHub : public EventHub {
    DECLARE_ACE_TYPE(PlatformViewEventHub, EventHub)

public:
    PlatformViewEventHub() = default;
    ~PlatformViewEventHub() override = default;

    void SetOnSurfaceInitEvent(ExternalEvent&& surfaceInitEvent)
    {
        surfaceInitEvent_ = std::move(surfaceInitEvent);
    }

    void FireSurfaceInitEvent(const std::string& componentId, const uint32_t nodeId) const
    {
        if (surfaceInitEvent_) {
            surfaceInitEvent_(componentId, nodeId, false);
        }
    }

private:
    ExternalEvent surfaceInitEvent_;
};
} // namespace OHOS::Ace::NG
#endif // PLUGINS_COMPONENT_PLATFORMVIEW_PLATFORMVIEW_EVENT_HUB_H
