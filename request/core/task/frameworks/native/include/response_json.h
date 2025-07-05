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

#ifndef PLUGINS_REQUEST_ANDROID_RESPONSE_JSON_H
#define PLUGINS_REQUEST_ANDROID_RESPONSE_JSON_H

#include "constant.h"
#include "nlohmann/json.hpp"
using Json = nlohmann::json;

namespace OHOS::Plugin::Request {
void to_json(Json &jsonResponse, const Response &response);
void from_json(const Json &jsonResponse, Response &response);
} // namespace OHOS::Plugin::Request
#endif // PLUGINS_REQUEST_ANDROID_Response_JSON_H