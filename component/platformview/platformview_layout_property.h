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

#ifndef PLUGINS_COMPONENT_PLATFORMVIEW_PLATFORMVIEW_LAYOUT_PROPERTY_H
#define PLUGINS_COMPONENT_PLATFORMVIEW_PLATFORMVIEW_LAYOUT_PROPERTY_H

#include "core/components_ng/layout/layout_property.h"
#include "core/components_ng/property/property.h"

namespace OHOS::Ace::NG {
class ACE_EXPORT PlatformViewLayoutProperty : public LayoutProperty {
    DECLARE_ACE_TYPE(PlatformViewLayoutProperty, LayoutProperty);

public:
    PlatformViewLayoutProperty() = default;

    ~PlatformViewLayoutProperty() override = default;

    RefPtr<LayoutProperty> Clone() const override
    {
        auto value = MakeRefPtr<PlatformViewLayoutProperty>();
        value->LayoutProperty::UpdateLayoutProperty(DynamicCast<LayoutProperty>(this));
        return value;
    }

    void Reset() override
    {
        LayoutProperty::Reset();
    }
    
private:
    ACE_DISALLOW_COPY_AND_MOVE(PlatformViewLayoutProperty);
};
} // namespace OHOS::Ace::NG
#endif // PLUGINS_COMPONENT_PLATFORMVIEW_PLATFORMVIEW_LAYOUT_PROPERTY_H
