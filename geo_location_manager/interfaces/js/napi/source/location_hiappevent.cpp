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
 
#include <string>
#include "location_hiappevent.h"
#include "location_log.h"
#include "common_utils.h"
#ifdef LOCATION_HIAPPEVENT_ENABLE
#include "app_event.h"
#include "app_event_processor_mgr.h"
#endif
 
namespace OHOS {
namespace Location {
LocationHiAppEvent::LocationHiAppEvent()
{
    LBSLOGI(LOCATION_HIAPPEVENT, "ability constructed.");
}
 
LocationHiAppEvent::~LocationHiAppEvent()
{
    LBSLOGI(LOCATION_HIAPPEVENT, "ability destructed.");
}
 
LocationHiAppEvent* LocationHiAppEvent::GetInstance()
{
    static LocationHiAppEvent data;
    return &data;
}
 
void LocationHiAppEvent::AddProcessor()
{
}
 
void LocationHiAppEvent::WriteEndEvent(const int64_t beginTime, const int result, const int errCode,
    const std::string& apiName)
{
#ifdef LOCATION_HIAPPEVENT_ENABLE
    AddProcessor();
    // 非应用不支持打点
    if (processorId_ == HA_NOT_SUPPORT_PROCESS_ID) {
        LBSLOGI(LOCATION_HIAPPEVENT, "not support");
        return;
    }
    HiviewDFX::HiAppEvent::Event event("api_diagnostic", "api_exec_end", OHOS::HiviewDFX::HiAppEvent::BEHAVIOR);
    event.AddParam("trans_id", CommonUtils::GenerateUuid());
    event.AddParam("api_name", apiName);
    event.AddParam("sdk_name", std::string("LocationKit"));
    event.AddParam("begin_time", beginTime);
    event.AddParam("end_time", CommonUtils::GetCurrentTimeMilSec());
    event.AddParam("result", result);
    event.AddParam("error_code", errCode);
    OHOS::HiviewDFX::HiAppEvent::Write(event);
    LBSLOGI(LOCATION_HIAPPEVENT, "WriteEndEvent end, apiName:%{public}s", apiName.c_str());
#endif
}
}  // namespace Location
}  // namespace OHOS