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

#include "task_state_json.h"
#include "log.h"

namespace OHOS::Plugin::Request {
const std::string JSON_PATH = "path";
const std::string JSON_RESPONSE_CODE = "responseCode";
const std::string JSON_MESSAGE = "message";

void to_json(Json &json, const TaskState &taskState)
{
    json = Json({{JSON_PATH, taskState.path},
                {JSON_RESPONSE_CODE, taskState.responseCode},
                {JSON_MESSAGE, taskState.message}});
}

void from_json(const Json &json, TaskState &taskState)
{
    if (json.find(JSON_PATH) != json.end() && json[JSON_PATH].is_string()) {
        json.at(JSON_PATH).get_to(taskState.path);
    }
    if (json.find(JSON_RESPONSE_CODE) != json.end() && json[JSON_RESPONSE_CODE].is_number()) {
        json.at(JSON_RESPONSE_CODE).get_to(taskState.responseCode);
    }
    if (json.find(JSON_MESSAGE) != json.end() && json[JSON_MESSAGE].is_string()) {
        json.at(JSON_MESSAGE).get_to(taskState.message);
    }
}
} // namespace OHOS::Plugin::Request