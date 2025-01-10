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

#ifndef PLUGINS_TIME_MOCK_I_REMOTE_BROKER_H
#define PLUGINS_TIME_MOCK_I_REMOTE_BROKER_H

#include <functional>
#include <unordered_map>
#include <vector>

#include "iremote_object.h"

namespace OHOS {
template<typename T>
class BrokerCreator {
public:
    BrokerCreator() = default;
    ~BrokerCreator() = default;
};

class IRemoteBroker : public virtual RefBase {
public:
    IRemoteBroker() = default;
    virtual ~IRemoteBroker() override = default;
};

#define DECLARE_INTERFACE_DESCRIPTOR(DESCRIPTOR)                   \
    static constexpr const char16_t* metaDescriptor_ = DESCRIPTOR; \
    static inline const std::u16string GetDescriptor()             \
    {                                                              \
        return metaDescriptor_;                                    \
    }
} // namespace OHOS
#endif // PLUGINS_TIME_MOCK_I_REMOTE_BROKER_H
