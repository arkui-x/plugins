/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_FENCEEXTENSION_IREMOTE_FENCE_EXTENSION_H
#define OHOS_FENCEEXTENSION_IREMOTE_FENCE_EXTENSION_H

#include "iremote_broker.h"
#include "constant_definition.h"
#include <map>

namespace OHOS {
namespace Location {
class IRemoteFenceExtension : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.FenceExtension.IRemoteFenceExtension");
    enum { ON_FENCE_STATUS_CHANGE = 0 };

    /**
     * @brief called back when geofence status is change.
     *
     * @param extraData other extra data
     */
    virtual FenceExtensionErrCode OnFenceStatusChange(std::map<std::string, std::string> extraData) = 0;
};
}  // namespace Location
}  // namespace OHOS
#endif  // OHOS_FENCEEXTENSION_IREMOTE_FENCE_EXTENSION_H
