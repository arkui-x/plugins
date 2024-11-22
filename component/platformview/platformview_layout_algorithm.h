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

#ifndef PLUGINS_COMPONENT_PLATFORMVIEW_PLATFORMVIEW_LAYOUT_ALGORITHM_H
#define PLUGINS_COMPONENT_PLATFORMVIEW_PLATFORMVIEW_LAYOUT_ALGORITHM_H

#include "core/components_ng/layout/box_layout_algorithm.h"
#include "core/components_ng/layout/layout_wrapper.h"

namespace OHOS::Ace::NG {
class PipelineContext;

class ACE_EXPORT PlatformViewLayoutAlgorithm : public BoxLayoutAlgorithm {
    DECLARE_ACE_TYPE(PlatformViewLayoutAlgorithm, BoxLayoutAlgorithm);

public:
    PlatformViewLayoutAlgorithm() = default;

    ~PlatformViewLayoutAlgorithm() override = default;

    std::optional<SizeF> MeasureContent(
        const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper) override;

    void Measure(LayoutWrapper* layoutWrapper) override;

    void Layout(LayoutWrapper* layoutWrapper) override;

private:
    ACE_DISALLOW_COPY_AND_MOVE(PlatformViewLayoutAlgorithm);
};
} // namespace OHOS::Ace::NG
#endif // PLUGINS_COMPONENT_PLATFORMVIEW_PLATFORMVIEW_LAYOUT_ALGORITHM_H
