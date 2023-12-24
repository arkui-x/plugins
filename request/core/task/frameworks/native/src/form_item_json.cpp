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

#include "form_item_json.h"
#include "log.h"

namespace OHOS::Plugin::Request {
const std::string JSON_NAME = "name";
const std::string JSON_VALUE = "value";

void to_json(Json &json, const FormItem &formItem)
{
    json = Json({{JSON_NAME, formItem.name},
                {JSON_VALUE, formItem.value}});
}

void from_json(const Json &json, FormItem &formItem)
{
    if (json.find(JSON_NAME) != json.end() && json[JSON_NAME].is_string()) {
        json.at(JSON_NAME).get_to(formItem.name);
    }
    if (json.find(JSON_VALUE) != json.end() && json[JSON_VALUE].is_string()) {
        json.at(JSON_VALUE).get_to(formItem.value);
    }
}
} // namespace OHOS::Plugin::Request