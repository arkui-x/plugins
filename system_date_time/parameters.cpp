/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "parameters.h"
#include "time_service_client.h"
#include "time_common.h"

using namespace OHOS::MiscServices;

namespace OHOS {
namespace system {
std::string GetParameter(const std::string &key, const std::string &def)
{
    std::string timeZone = "";
    int ret = TimeServiceClient::GetInstance()->GetTimeZone(timeZone);
    if (ret != E_TIME_OK) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "GetTimeZone failed");
        return "";
    }
    return timeZone;
}
} // namespace system
} // namespace OHOS
