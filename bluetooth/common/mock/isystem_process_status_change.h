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

#ifndef PLUGINS_BLUETOOTH_COMMON_MOCK_I_SYSTEM_PROCESS_STATUS_CHANGE_H
#define PLUGINS_BLUETOOTH_COMMON_MOCK_I_SYSTEM_PROCESS_STATUS_CHANGE_H

#include <string>

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"

namespace OHOS {
struct SystemProcessInfo {
    std::string processName;
    int32_t pid = -1;
    int32_t uid = -1;
};

class ISystemProcessStatusChange : public IRemoteBroker {
public:
    virtual ~ISystemProcessStatusChange() = default;

    /**
     * OnSystemProcessStarted, OnSystemProcessStarted will be called when subscribe process Started.
     *
     * @param systemProcessInfo, Process related status information.
     * @return void.
     */
    virtual void OnSystemProcessStarted(SystemProcessInfo& systemProcessInfo) = 0;

    /**
     * OnSystemProcessStopped, OnSystemProcessStopped will be called when subscribe process Stopped.
     *
     * @param systemProcessInfo, Process related status information.
     * @return void.
     */
    virtual void OnSystemProcessStopped(SystemProcessInfo& systemProcessInfo) = 0;
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.ISystemProcessStatusChange");
    enum {
        ON_SYSTEM_PROCESS_STARTED = 1,
        ON_SYSTEM_PROCESS_STOPPED = 2,
    };
};
} // namespace OHOS
#endif // PLUGINS_BLUETOOTH_COMMON_MOCK_I_SYSTEM_PROCESS_STATUS_CHANGE_H
