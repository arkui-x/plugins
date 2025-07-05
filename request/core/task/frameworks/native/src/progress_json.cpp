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

#include "progress_json.h"
#include "log.h"

namespace OHOS::Plugin::Request {
const std::string JSON_STATE = "state";
const std::string JSON_INDEX = "index";
const std::string JSON_PROCESSED = "processed";
const std::string JSON_LAST_PROCESSED = "lastProcessed";
const std::string JSON_TOTAL_PROCESSED = "totalProcessed";
const std::string JSON_SIZES = "sizes";
const std::string JSON_EXTRAS = "extras";
const std::string JSON_BODY_BYTES = "bodyBytes";

void to_json(Json &json, const Progress &progress)
{
    json = Json({{JSON_STATE, progress.state},
                {JSON_INDEX, progress.index},
                {JSON_PROCESSED, progress.processed},
                {JSON_LAST_PROCESSED, progress.lastProcessed},
                {JSON_TOTAL_PROCESSED, progress.totalProcessed},
                {JSON_SIZES, progress.sizes},
                {JSON_EXTRAS, progress.extras},
                {JSON_BODY_BYTES, progress.bodyBytes}});
}

void from_json(const Json &json, Progress &progress)
{
    if (json.find(JSON_STATE) != json.end() && json[JSON_STATE].is_number()) {
        json.at(JSON_STATE).get_to(progress.state);
    }
    if (json.find(JSON_INDEX) != json.end() && json[JSON_INDEX].is_number()) {
        json.at(JSON_INDEX).get_to(progress.index);
    }
    if (json.find(JSON_PROCESSED) != json.end() && json[JSON_PROCESSED].is_number()) {
        json.at(JSON_PROCESSED).get_to(progress.processed);
    }
    if (json.find(JSON_LAST_PROCESSED) != json.end() && json[JSON_LAST_PROCESSED].is_number()) {
        json.at(JSON_LAST_PROCESSED).get_to(progress.lastProcessed);
    }
    if (json.find(JSON_TOTAL_PROCESSED) != json.end() && json[JSON_TOTAL_PROCESSED].is_number()) {
        json.at(JSON_TOTAL_PROCESSED).get_to(progress.totalProcessed);
    }
    if (json.find(JSON_SIZES) != json.end() && json[JSON_SIZES].is_array()) {
        json.at(JSON_SIZES).get_to(progress.sizes);
    }
    if (json.find(JSON_EXTRAS) != json.end() && json[JSON_EXTRAS].is_object()) {
        json.at(JSON_EXTRAS).get_to(progress.extras);
    }
    if (json.find(JSON_BODY_BYTES) != json.end() && json[JSON_BODY_BYTES].is_array()) {
        json.at(JSON_BODY_BYTES).get_to(progress.bodyBytes);
    }
}
} // namespace OHOS::Plugin::Request