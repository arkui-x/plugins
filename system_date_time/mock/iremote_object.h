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

#ifndef PLUGINS_TIME_MOCK_I_REMOTE_OBJECT_H
#define PLUGINS_TIME_MOCK_I_REMOTE_OBJECT_H

#include <codecvt>
#include <locale>
#include <memory>
#include <string>

#include "refbase.h"

namespace OHOS {

class IRemoteObject : public virtual RefBase {
public:

    class DeathRecipient {
    public:
        virtual void OnRemoteDied(const wptr<IRemoteObject>& object) = 0;
        void DecStrongRef(const void *objectId) {}
        void IncStrongRef(const void *objectId) {}
        
    };

protected:
    explicit IRemoteObject(std::u16string descriptor = nullptr);
    virtual ~IRemoteObject() = default;
};
} // namespace OHOS
#endif // PLUGINS_TIME_MOCK_I_REMOTE_OBJECT_H
