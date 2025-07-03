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

#include "response_json.h"
#include "log.h"

namespace OHOS::Plugin::Request {
const std::string JSON_VERSION = "version";
const std::string JSON_STATUS_CODE = "statusCode";
const std::string JSON_REASON = "reason";
const std::string JSON_HEADERS = "headers";

void to_json(Json &json, const Response &response)
{
    Json headersJson;
    for (const auto &header : response.headers) {
        Json valuesJson;
        for (const auto &value : header.second) {
            valuesJson.push_back(value);
        }
        headersJson[header.first] = valuesJson;
    }

    json = Json {
        {JSON_VERSION, response.version},
        {JSON_STATUS_CODE, response.statusCode},
        {JSON_REASON, response.reason},
        {JSON_HEADERS, headersJson}
    };
}

void from_json(const Json &json, Response &response)
{
    if (json.find(JSON_VERSION) != json.end() && json[JSON_VERSION].is_string()) {
        json.at(JSON_VERSION).get_to(response.version);
    }
    
    if (json.find(JSON_STATUS_CODE) != json.end() && json[JSON_STATUS_CODE].is_number()) {
        json.at(JSON_STATUS_CODE).get_to(response.statusCode);
    }
    
    if (json.find(JSON_REASON) != json.end() && json[JSON_REASON].is_string()) {
        json.at(JSON_REASON).get_to(response.reason);
    }
    
    if (json.find(JSON_HEADERS) != json.end() && json[JSON_HEADERS].is_object()) {
        const auto &headersJson = json[JSON_HEADERS];
        for (auto it = headersJson.begin(); it != headersJson.end(); ++it) {
            const std::string &key = it.key();
            const Json &valuesJson = it.value();
            
            if (valuesJson.is_array()) {
                std::vector<std::string> values;
                for (const auto &value : valuesJson) {
                    if (value.is_string()) {
                        values.push_back(value.get<std::string>());
                    }
                }
                response.headers[key] = values;
            }
        }
    }
}
} // namespace OHOS::Plugin::Request