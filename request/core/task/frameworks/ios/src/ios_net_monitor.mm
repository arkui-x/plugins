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

#include "ios_net_monitor.h"
#include "log.h"
#import "OHNetworkKit.h"

namespace OHOS::Plugin::Request {
static IosNetMonitor *iosNetMonitor = IosNetMonitor::SharedInstance();
IosNetMonitor* IosNetMonitor::SharedInstance()
{
    static IosNetMonitor *instance = nullptr;
    std::once_flag onceFlag;
    std::call_once(onceFlag, [&] {
        if (instance == nullptr) {
            instance = new (std::nothrow) IosNetMonitor();
            instance->StartMonitoring();
        }
    });
    return instance;
}

IosNetMonitor::~IosNetMonitor()
{
    observerList_.clear();
}

void IosNetMonitor::AddObserver(IosNetMonitorObserver *observer)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (observer != nullptr) {
        observerList_.push_back(observer);
        observer->NetworkStatusChanged(networkType_);
    }
}

void IosNetMonitor::RemoveObserver(IosNetMonitorObserver *observer)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = observerList_.begin(); it != observerList_.end(); it++) {
        if (*it == observer) {
            observerList_.erase(it);
            break;
        }
    }
}

void IosNetMonitor::StartMonitoring()
{
    [[OHNetStatusMonitor sharedMonitor] setNetStatusChangeBlock:^(OHNetStatus status) {
        REQUEST_HILOGI("Reachability: %{public}s", OHStringFromNetworkReachabilityStatus(status).UTF8String);
        switch (status) {
            case OHNetStatusReachableViaWiFi: {
                networkType_ = NETWORK_WIFI;
                break;
            }
            case OHNetStatusReachableViaWWAN: {
                networkType_ = NETWORK_MOBILE;
                break;
            }
            case OHNetStatusNotReachable:
            case OHNetStatusUnknown:
            default: {
                networkType_ = NETWORK_INVALID;
                break;
            }
        }

        std::lock_guard<std::mutex> lock(mutex_);
        for (auto &observer : observerList_) {
            if (observer != nullptr) {
                observer->NetworkStatusChanged(networkType_);
            }
        }
    }];

    [[OHNetStatusMonitor sharedMonitor] startMonitoring];
}

} // namespace OHOS::Plugin::Request