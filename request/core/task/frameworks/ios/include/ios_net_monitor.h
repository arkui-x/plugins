/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_REQUEST_IOS_NET_MONITOR_H
#define PLUGINS_REQUEST_IOS_NET_MONITOR_H

#include <mutex>
#include <vector>
#include "constant.h"

namespace OHOS::Plugin::Request {
class IosNetMonitorObserver {
public:
    IosNetMonitorObserver() = default;
    virtual ~IosNetMonitorObserver() {}

public:
    virtual void NetworkStatusChanged(NetworkType netType) = 0;
};

class IosNetMonitor final {
public:
    static IosNetMonitor* SharedInstance();
    ~IosNetMonitor();
    void AddObserver(IosNetMonitorObserver *observer);
    void RemoveObserver(IosNetMonitorObserver *observer);
    void StartMonitoring();

private:
    IosNetMonitor() = default;

private:
    std::vector<IosNetMonitorObserver*> observerList_;
    NetworkType networkType_ = NETWORK_INVALID;
    std::mutex mutex_;
};
} // namespace OHOS::Plugin::Request 
#endif // PLUGINS_REQUEST_IOS_NET_MONITOR_H