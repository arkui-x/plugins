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

#ifndef I_NMEA_MESSAGE_CALLBACK_H
#define I_NMEA_MESSAGE_CALLBACK_H

#include <string>

#include "iremote_broker.h"

namespace OHOS {
namespace Location {
class INmeaMessageCallback : public IRemoteBroker {
public:
    enum {
        RECEIVE_NMEA_MESSAGE_EVENT = 1,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"location.INmeaMessageCallback");
    /*
     * message changed report to kits
     */
    virtual void OnMessageChange(int64_t timestamp, const std::string msg) = 0;
};
} // namespace Location
} // namespace OHOS
#endif // I_NMEA_MESSAGE_CALLBACK_H
