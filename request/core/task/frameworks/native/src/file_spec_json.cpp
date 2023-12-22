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

#include "file_spec_json.h"
#include "log.h"

namespace OHOS::Plugin::Request {
const std::string JSON_NAME = "name";
const std::string JSON_URI = "uri";
const std::string JSON_FILENAME = "filename";
const std::string JSON_TYPE = "type";
const std::string JSON_FD = "fd";

void to_json(Json &json, const FileSpec &fileSpec)
{
    json = Json({{JSON_NAME, fileSpec.name},
                {JSON_URI, fileSpec.uri},
                {JSON_FILENAME, fileSpec.filename},
                {JSON_TYPE, fileSpec.type},
                {JSON_FD, fileSpec.fd}});
}

void from_json(const Json &json, FileSpec &fileSpec)
{
    if (json.find(JSON_NAME) != json.end() && json[JSON_NAME].is_string()) {
        json.at(JSON_NAME).get_to(fileSpec.name);
    }
    if (json.find(JSON_URI) != json.end() && json[JSON_URI].is_string()) {
        json.at(JSON_URI).get_to(fileSpec.uri);
    }
    if (json.find(JSON_FILENAME) != json.end() && json[JSON_FILENAME].is_string()) {
        json.at(JSON_FILENAME).get_to(fileSpec.filename);
    }
    if (json.find(JSON_TYPE) != json.end() && json[JSON_TYPE].is_string()) {
        json.at(JSON_TYPE).get_to(fileSpec.type);
    }
    if (json.find(JSON_FD) != json.end() && json[JSON_FD].is_number()) {
        json.at(JSON_FD).get_to(fileSpec.fd);
    }
}
} // namespace OHOS::Plugin::Request