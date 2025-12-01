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

#ifndef I_GNSS_GEOFENCE_CALLBACK_H
#define I_GNSS_GEOFENCE_CALLBACK_H

#include "iremote_broker.h"

#include "geofence_definition.h"

namespace OHOS {
namespace Location {
class IGnssGeofenceCallback : public IRemoteBroker {
public:
    enum {
        REPORT_OPERATION_RESULT_EVENT = 1,
        RECEIVE_TRANSITION_STATUS_EVENT
    };
    DECLARE_INTERFACE_DESCRIPTOR(u"location.IGnssGeofenceCallback");
    virtual void OnTransitionStatusChange(GeofenceTransition transition) = 0;
    virtual void OnReportOperationResult(int fenceId, int type, int result) = 0;
};
} // namespace Location
} // namespace OHOS
#endif // I_GNSS_GEOFENCE_CALLBACK_H
