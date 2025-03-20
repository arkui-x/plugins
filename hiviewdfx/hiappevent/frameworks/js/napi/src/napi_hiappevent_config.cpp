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
#include "napi_hiappevent_config.h"

#include <map>
#include <string>

#include "hiappevent_config.h"
#include "napi_error.h"
#include "napi_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace NapiHiAppEventConfig {
namespace {
const std::map<std::string, napi_valuetype> CONFIG_OPTION_MAP = {
    { "disable", napi_boolean },
    { "maxStorage", napi_string },
};
}
bool Configure(const napi_env env, const napi_value configObj, bool isThrow)
{
    if (!NapiUtil::IsObject(env, configObj)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("config", "ConfigOption"), isThrow);
        return false;
    }

    std::vector<std::string> keys;
    NapiUtil::GetPropertyNames(env, configObj, keys);
    for (auto key : keys) {
        if (CONFIG_OPTION_MAP.find(key) == CONFIG_OPTION_MAP.end()) {
            continue;
        }
        napi_value value = NapiUtil::GetProperty(env, configObj, key);
        if (CONFIG_OPTION_MAP.at(key) != NapiUtil::GetType(env, value)) {
            std::string errMsg = NapiUtil::CreateErrMsg(key, CONFIG_OPTION_MAP.at(key));
            NapiUtil::ThrowError(env, NapiError::ERR_PARAM, errMsg, isThrow);
            return false;
        }
        if (!HiAppEventConfig::GetInstance().SetConfigurationItem(key, NapiUtil::ConvertToString(env, value))) {
            std::string errMsg = "Invalid max storage quota value.";
            NapiUtil::ThrowError(env, NapiError::ERR_INVALID_MAX_STORAGE, errMsg, isThrow);
            return false;
        }
    }
    return true;
}
} // namespace NapiHiAppEventConfig
} // namespace HiviewDFX
} // namespace OHOS
