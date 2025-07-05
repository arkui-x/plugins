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

#include "config_json.h"
#include "file_spec_json.h"
#include "form_item_json.h"
#include "log.h"

namespace OHOS::Plugin::Request {
const std::string JSON_ACTION = "action";
const std::string JSON_URL = "url";
const std::string JSON_VERSION = "version";
const std::string JSON_MODE = "mode";
const std::string JSON_NETWORK = "network";
const std::string JSON_INDEX = "index";
const std::string JSON_BEGINS = "begins";
const std::string JSON_ENDS = "ends";
const std::string JSON_PRIORITY = "priority";
const std::string JSON_OVERWRITE = "overwrite";
const std::string JSON_METERED = "metered";
const std::string JSON_ROAMING = "roaming";
const std::string JSON_RETRY = "retry";
const std::string JSON_REDIRECT = "redirect";
const std::string JSON_GAUGE = "gauge";
const std::string JSON_PRECISE = "precise";
const std::string JSON_BACKGROUND = "background";
const std::string JSON_TITLE = "title";
const std::string JSON_SAVEAS = "saveas";
const std::string JSON_PROXY = "proxy";
const std::string JSON_METHOD = "method";
const std::string JSON_TOKEN = "token";
const std::string JSON_DESCRIPTION = "description";
const std::string JSON_DATA = "data";
const std::string JSON_HEADERS = "headers";
const std::string JSON_FORMS = "froms";
const std::string JSON_FILES = "files";
const std::string JSON_BODY_FDS = "bodyFds";
const std::string JSON_BODY_FILE_NAMES = "bodyFileNames";
const std::string JSON_EXTRAS = "extras";

void to_json(Json &json, const Config &config)
{
    json = Json({
        {JSON_ACTION, config.action},
        {JSON_URL, config.url},
        {JSON_VERSION, config.version},
        {JSON_MODE, config.mode},
        {JSON_NETWORK, config.network},
        {JSON_INDEX, config.index},
        {JSON_BEGINS, config.begins},
        {JSON_ENDS, config.ends},
        {JSON_OVERWRITE, config.overwrite},
        {JSON_METERED, config.metered},
        {JSON_ROAMING, config.roaming},
        {JSON_RETRY, config.retry},
        {JSON_REDIRECT, config.redirect},
        {JSON_GAUGE, config.gauge},
        {JSON_PRECISE, config.precise},
        {JSON_BACKGROUND, config.background},
        {JSON_TITLE, config.title},
        {JSON_SAVEAS, config.saveas},
        {JSON_PROXY, config.proxy},
        {JSON_METHOD, config.method},
        {JSON_TOKEN, config.token},
        {JSON_DESCRIPTION, config.description},
        {JSON_DATA, config.data},
        {JSON_HEADERS, config.headers},
        {JSON_FORMS, config.forms},
        {JSON_FILES, config.files},
        {JSON_BODY_FDS, config.bodyFds},
        {JSON_BODY_FILE_NAMES, config.bodyFileNames},
        {JSON_EXTRAS, config.extras},
    });
}

void from_json(const Json &json, Config &config)
{
    if (json.find(JSON_ACTION) != json.end() && json[JSON_ACTION].is_number()) {
        json.at(JSON_ACTION).get_to(config.action);
    }
    if (json.find(JSON_URL) != json.end() && json[JSON_URL].is_string()) {
        json.at(JSON_URL).get_to(config.url);
    }
    if (json.find(JSON_VERSION) != json.end() && json[JSON_VERSION].is_number()) {
        json.at(JSON_VERSION).get_to(config.version);
    }
    if (json.find(JSON_MODE) != json.end() && json[JSON_MODE].is_number()) {
        json.at(JSON_MODE).get_to(config.mode);
    }
    if (json.find(JSON_NETWORK) != json.end() && json[JSON_NETWORK].is_number()) {
        json.at(JSON_NETWORK).get_to(config.network);
    }
    if (json.find(JSON_INDEX) != json.end() && json[JSON_INDEX].is_number()) {
        json.at(JSON_INDEX).get_to(config.index);
    }
    if (json.find(JSON_BEGINS) != json.end() && json[JSON_BEGINS].is_number()) {
        json.at(JSON_BEGINS).get_to(config.begins);
    }
    if (json.find(JSON_ENDS) != json.end() && json[JSON_ENDS].is_number()) {
        json.at(JSON_ENDS).get_to(config.ends);
    }
    if (json.find(JSON_PRIORITY) != json.end() && json[JSON_PRIORITY].is_number()) {
        json.at(JSON_PRIORITY).get_to(config.priority);
    }
    if (json.find(JSON_OVERWRITE) != json.end() && json[JSON_OVERWRITE].is_boolean()) {
        json.at(JSON_OVERWRITE).get_to(config.overwrite);
    }
    if (json.find(JSON_METERED) != json.end() && json[JSON_METERED].is_boolean()) {
        json.at(JSON_METERED).get_to(config.metered);
    }
    if (json.find(JSON_ROAMING) != json.end() && json[JSON_ROAMING].is_boolean()) {
        json.at(JSON_ROAMING).get_to(config.roaming);
    }
    if (json.find(JSON_RETRY) != json.end() && json[JSON_RETRY].is_boolean()) {
        json.at(JSON_RETRY).get_to(config.retry);
    }
    if (json.find(JSON_REDIRECT) != json.end() && json[JSON_REDIRECT].is_boolean()) {
        json.at(JSON_REDIRECT).get_to(config.redirect);
    }
    if (json.find(JSON_GAUGE) != json.end() && json[JSON_GAUGE].is_boolean()) {
        json.at(JSON_GAUGE).get_to(config.gauge);
    }
    if (json.find(JSON_PRECISE) != json.end() && json[JSON_PRECISE].is_boolean()) {
        json.at(JSON_PRECISE).get_to(config.precise);
    }
    if (json.find(JSON_BACKGROUND) != json.end() && json[JSON_BACKGROUND].is_boolean()) {
        json.at(JSON_BACKGROUND).get_to(config.background);
    }
    if (json.find(JSON_TITLE) != json.end() && json[JSON_TITLE].is_string()) {
        json.at(JSON_TITLE).get_to(config.title);
    }
    if (json.find(JSON_SAVEAS) != json.end() && json[JSON_SAVEAS].is_string()) {
        json.at(JSON_SAVEAS).get_to(config.saveas);
    }
    if (json.find(JSON_PROXY) != json.end() && json[JSON_PROXY].is_string()) {
        json.at(JSON_PROXY).get_to(config.proxy);
    }
    if (json.find(JSON_METHOD) != json.end() && json[JSON_METHOD].is_string()) {
        json.at(JSON_METHOD).get_to(config.method);
    }
    if (json.find(JSON_TOKEN) != json.end() && json[JSON_TOKEN].is_string()) {
        json.at(JSON_TOKEN).get_to(config.token);
    }
    if (json.find(JSON_DESCRIPTION) != json.end() && json[JSON_DESCRIPTION].is_string()) {
        json.at(JSON_DESCRIPTION).get_to(config.description);
    }
    if (json.find(JSON_DATA) != json.end() && json[JSON_DATA].is_string()) {
        json.at(JSON_DATA).get_to(config.data);
    }
    if (json.find(JSON_HEADERS) != json.end() && json[JSON_HEADERS].is_object()) {
        json.at(JSON_HEADERS).get_to(config.headers);
    }
    if (json.find(JSON_FORMS) != json.end() && json[JSON_FORMS].is_array()) {
        json.at(JSON_FORMS).get_to(config.forms);
    }
    if (json.find(JSON_FILES) != json.end() && json[JSON_FILES].is_array()) {
        json.at(JSON_FILES).get_to(config.files);
    }
    if (json.find(JSON_BODY_FDS) != json.end() && json[JSON_BODY_FDS].is_array()) {
        json.at(JSON_BODY_FDS).get_to(config.bodyFds);
    }
    if (json.find(JSON_BODY_FILE_NAMES) != json.end() && json[JSON_BODY_FILE_NAMES].is_array()) {
        json.at(JSON_BODY_FILE_NAMES).get_to(config.bodyFileNames);
    }
    if (json.find(JSON_EXTRAS) != json.end() && json[JSON_EXTRAS].is_object()) {
        json.at(JSON_EXTRAS).get_to(config.extras);
    }
}
} // namespace OHOS::Plugin::Request