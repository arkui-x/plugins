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

#ifndef PLUGINS_COMPONENT_PLATFORMVIEW_PLATFORMVIEW_MODEL_NG_H
#define PLUGINS_COMPONENT_PLATFORMVIEW_PLATFORMVIEW_MODEL_NG_H

#include "base/memory/referenced.h"
#include "core/components_ng/base/frame_node.h"

namespace OHOS::Ace::NG {
class FrameNode;

class ACE_EXPORT PlatformViewModelNG {
public:
    static PlatformViewModelNG* GetInstance();
    virtual ~PlatformViewModelNG() = default;
    void Create(const std::string& id);

private:
    static std::unique_ptr<PlatformViewModelNG> instance_;
    static std::mutex mutex_;
};
} // namespace OHOS::Ace::NG
#endif // PLUGINS_COMPONENT_PLATFORMVIEW_PLATFORMVIEW_MODEL_NG_H
