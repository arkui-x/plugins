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
 
#ifndef LOCATION_HIAPPEVENT_H
#define LOCATION_HIAPPEVENT_H
 
#include <mutex>
#include <string>
 
namespace OHOS {
namespace Location {
const int32_t HA_CONFIG_TIMEOUT = 90;
const int32_t HA_CONFIG_ROW = 30;
const int32_t HA_NOT_SUPPORT_PROCESS_ID = -200;

static const std::string KEY_KIT_REPORT_APPID = "kit_report_appId";
 
class LocationHiAppEvent {
public:
    LocationHiAppEvent();
    ~LocationHiAppEvent();
    static LocationHiAppEvent* GetInstance();
    void AddProcessor();
    void WriteEndEvent(const int64_t beginTime, const int result, const int errCode, const std::string& apiName);
private:
    bool Init();
    int64_t processorId_{-1};
    std::mutex processorIdMutex_;
};
 
}  // namespace Location
}  // namespace OHOS
#endif // LOCATION_HIAPPEVENT_H