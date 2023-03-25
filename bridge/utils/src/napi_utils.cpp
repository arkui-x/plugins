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

#include "napi_utils.h"

#include "plugins/interfaces/native/inner_utils/plugin_inner_napi_utils.h"
#include "plugins/interfaces/native/plugin_c_utils.h"

namespace OHOS::Plugin::Bridge {
static constexpr double double_min_value = 0.00001;

bool NAPIUtils::JsonStringToNapiValues(napi_env env, const std::string& str, size_t& argc, napi_value* argv)
{
    size_t argNum = 0;
    Json json = Json::parse(str, nullptr, false);
    while (json.find(std::to_string(argNum)) != json.end() && argNum < argc) {
        argv[argNum] = NAPI_GetPremers(env, json.at(std::to_string(argNum)));
        argNum++;
    }
    argc = argNum;
    return true;
}

bool NAPIUtils::NapiValuesToJsonString(napi_env env, const size_t& argc,
    const napi_value* argv, std::string& str)
{
    Json json = Json {};
    for (size_t i = 0; i < argc; i++) {
        json[std::to_string(i)] = PlatformPremers(env, argv[i]);
    }
    str = json.dump();
    return true;
}

napi_value NAPIUtils::NAPI_GetPremers(napi_env env, Json json)
{
    napi_value result = nullptr;
    if (json.is_string()) {
        result = PluginInnerNApiUtils::CreateStringUtf8(env, json.get<std::string>());
    } else if (json.is_boolean()) {
        result = PluginInnerNApiUtils::CreateBoolean(env, json.get<bool>());
    } else if (json.is_number()) {
        float okResultFloat = json.get<float>();
        int okResultInt = json.get<int>();
        if (okResultFloat - okResultInt < double_min_value) {
            result = PluginInnerNApiUtils::CreateInt32(env, okResultInt);
        } else {
            result = PluginInnerNApiUtils::CreateDouble(env, okResultFloat);
        }
    } else if (json.is_array()) {
        result = PluginInnerNApiUtils::CreateArray(env);
        int i = 0;
        for (auto& element : json) {
            napi_value value = NAPI_GetPremers(env, element);
            PluginInnerNApiUtils::SetSelementToArray(env, result, i, value);
            i++;
        }
    } else if (json.is_object()) {
        result = PluginInnerNApiUtils::CreateObject(env);
        for (const auto& [key, value] : json.items()) {
            PluginInnerNApiUtils::SetNamedProperty(env, result, key, NAPI_GetPremers(env, value));
        }
    } else {
        result = PluginInnerNApiUtils::CreateNull(env);
    }
    return result;
}

Json NAPIUtils::PlatformPremers(napi_env env, napi_value value)
{
    Json result;
    napi_valuetype valueType = PluginInnerNApiUtils::GetValueType(env, value);
    switch (valueType) {
        case napi_boolean: {
            result = PluginInnerNApiUtils::GetBool(env, value);
            break;
        }
        case napi_number: {
            int intValue = PluginInnerNApiUtils::GetCInt32(value, env);
            double numberValue = PluginInnerNApiUtils::GetDouble(env, value);
            if (numberValue - intValue > double_min_value) {
                result = numberValue;
            } else {
                result = intValue;
            }
            break;
        }
        case napi_string: {
            result = PluginInnerNApiUtils::GetStringFromValueUtf8(env, value);
            break;
        }
        case napi_object: {
            bool isArray = PluginInnerNApiUtils::IsArray(env, value);
            if (isArray) {
                uint32_t length;
                napi_get_array_length(env, value, &length);
                Json arrayValue = Json::array();
                for (uint32_t i = 0; i < length; i++) {
                    napi_value elementValue;
                    napi_get_element(env, value, i, &elementValue);
                    arrayValue.push_back(PlatformPremers(env, elementValue));
                }
                result = arrayValue;
                break;
            }
            std::vector<std::string> props;
            if (PluginInnerNApiUtils::GetPropertyNames(env, value, props)) {
                for (auto prop : props) {
                    result[prop] = PlatformPremers(env, PluginInnerNApiUtils::GetNamedProperty(env, value, prop));
                }
            }
            break;
        }
        default:
            break;
    }
    return result;
}

int NAPIUtils::NAPI_GetErrorCodeFromFson(Json json)
{
    int ret = -1;
    if (json.is_number()) {
        ret = json.get<int>();
    }
    return ret;
}
} // namespace OHOS::Plugin::Bridge