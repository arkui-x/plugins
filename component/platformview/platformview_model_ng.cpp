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

#include "platformview_model_ng.h"

#include "platformview_event_hub.h"
#include "platformview_layout_property.h"
#include "platformview_pattern.h"

#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_v2/inspector/inspector_constants.h"
namespace OHOS::Ace::NG {
const std::string PLATFORMVIEW_ETS_TAG = "PlatformView";
void PlatformViewModelNG::Create(const std::string& id, const std::int32_t type, const std::optional<std::string>& data)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", PLATFORMVIEW_ETS_TAG.c_str(), nodeId);
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        PLATFORMVIEW_ETS_TAG,
        nodeId,
        [id, type, data]() {
            return AceType::MakeRefPtr<PlatformViewPattern>(id, type, data);
        });
    stack->Push(frameNode);
}

void PlatformViewModelNG::SetScale(float x, float y, float z, const std::string& centerX, const std::string& centerY)
{
    auto frameNode = AceType::Claim(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<PlatformViewPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetScale(x, y, z, centerX, centerY);
}

void PlatformViewModelNG::SetRotation(float x, float y, float z, const std::string& angle, const std::string& centerX,
    const std::string& centerY, const std::string& centerZ, const std::string& perspective)
{
    auto frameNode = AceType::Claim(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<PlatformViewPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetRotation(x, y, z, angle, centerX, centerY, centerZ, perspective);
}

void PlatformViewModelNG::SetTranslate(const std::string& x, const std::string& y, const std::string& z)
    {
    auto frameNode = AceType::Claim(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<PlatformViewPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetTranslate(x, y, z);
}

void PlatformViewModelNG::SetTransformMatrix(const std::vector<float>& matrix)
{
    auto frameNode = AceType::Claim(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<PlatformViewPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetTransformMatrix(matrix);
}
} // namespace OHOS::Ace::NG
