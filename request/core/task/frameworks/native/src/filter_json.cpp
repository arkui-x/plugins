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

#include "filter_json.h"
#include "log.h"

namespace OHOS::Plugin::Request {
const std::string JSON_BEFORE = "before";
const std::string JSON_AFTER = "after";
const std::string JSON_STATE = "state";
const std::string JSON_ACTION = "action";
const std::string JSON_MODE = "mode";

/* Filter */
void to_json(Json &json, const Filter &filter)
{
    json = Json({{JSON_BEFORE, filter.before},
                {JSON_AFTER, filter.after},
                {JSON_STATE, filter.state},
                {JSON_ACTION, filter.action}});
}

void from_json(const Json &json, Filter &filter)
{
    if (json.find(JSON_BEFORE) != json.end() && json[JSON_BEFORE].is_number()) {
        json.at(JSON_BEFORE).get_to(filter.before);
    }
    if (json.find(JSON_AFTER) != json.end() && json[JSON_AFTER].is_number()) {
        json.at(JSON_AFTER).get_to(filter.after);
    }
    if (json.find(JSON_STATE) != json.end() && json[JSON_STATE].is_number()) {
        json.at(JSON_STATE).get_to(filter.state);
    }
    if (json.find(JSON_ACTION) != json.end() && json[JSON_ACTION].is_number()) {
        json.at(JSON_ACTION).get_to(filter.action);
    }
}
} // namespace OHOS::Plugin::Request