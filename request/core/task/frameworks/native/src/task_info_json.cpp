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
#include "task_info_json.h"
#include "file_spec_json.h"
#include "form_item_json.h"
#include "log.h"
#include "progress_json.h"
#include "task_state_json.h"
#include "response_json.h"

namespace OHOS::Plugin::Request {
const std::string JSON_VERSION = "version";
const std::string JSON_URL = "url";
const std::string JSON_DATA = "data";
const std::string JSON_FILES = "files";
const std::string JSON_FORMS = "froms";
const std::string JSON_TID = "tid";
const std::string JSON_TITLE = "title";
const std::string JSON_DESCRIPTION = "description";
const std::string JSON_ACTION = "action";
const std::string JSON_MODE = "mode";
const std::string JSON_MIME_TYPE = "mimeType";
const std::string JSON_PROGRESS = "progress";
const std::string JSON_RESPONSE = "response";
const std::string JSON_GAUGE = "gauge";
const std::string JSON_CTIME = "ctime";
const std::string JSON_MTIME = "mtime";
const std::string JSON_RETRY = "retry";
const std::string JSON_TRIES = "tries";
const std::string JSON_FAULTS = "faults";
const std::string JSON_CODE = "code";
const std::string JSON_REASON = "reason";
const std::string JSON_WITH_SYSTEM = "withSystem";
const std::string JSON_EXTRAS = "extras";
const std::string JSON_TASK_STATES = "taskStates";

/* TaskInfo */
void to_json(Json &json, const TaskInfo &taskInfo)
{
    json = Json({{JSON_VERSION, taskInfo.version},
                {JSON_URL, taskInfo.url},
                {JSON_DATA, taskInfo.data},
                {JSON_FILES, taskInfo.files},
                {JSON_FORMS, taskInfo.forms},
                {JSON_TID, taskInfo.tid},
                {JSON_TITLE, taskInfo.title},
                {JSON_DESCRIPTION, taskInfo.description},
                {JSON_ACTION, taskInfo.action},
                {JSON_MODE, taskInfo.mode},
                {JSON_MIME_TYPE, taskInfo.mimeType},
                {JSON_PROGRESS, taskInfo.progress},
                {JSON_RESPONSE, taskInfo.response},
                {JSON_GAUGE, taskInfo.gauge},
                {JSON_CTIME, taskInfo.ctime},
                {JSON_MTIME, taskInfo.mtime},
                {JSON_RETRY, taskInfo.retry},
                {JSON_TRIES, taskInfo.tries},
                {JSON_FAULTS, taskInfo.faults},
                {JSON_CODE, taskInfo.code},
                {JSON_REASON, taskInfo.reason},
                {JSON_WITH_SYSTEM, taskInfo.withSystem},
                {JSON_EXTRAS, taskInfo.extras},
                {JSON_TASK_STATES, taskInfo.taskStates}});
}

void from_json(const Json &json, TaskInfo &taskInfo)
{
    if (json.find(JSON_VERSION) != json.end() && json[JSON_VERSION].is_string()) {
        json.at(JSON_VERSION).get_to(taskInfo.version);
    }
    if (json.find(JSON_URL) != json.end() && json[JSON_URL].is_string()) {
        json.at(JSON_URL).get_to(taskInfo.url);
    }
    if (json.find(JSON_DATA) != json.end() && json[JSON_DATA].is_string()) {
        json.at(JSON_DATA).get_to(taskInfo.data);
    }
    if (json.find(JSON_FILES) != json.end() && json[JSON_FILES].is_array()) {
        json.at(JSON_FILES).get_to(taskInfo.files);
    }
    if (json.find(JSON_FORMS) != json.end() && json[JSON_FORMS].is_array()) {
        json.at(JSON_FORMS).get_to(taskInfo.forms);
    }
    if (json.find(JSON_TID) != json.end() && json[JSON_TID].is_string()) {
        json.at(JSON_TID).get_to(taskInfo.tid);
    }
    if (json.find(JSON_TITLE) != json.end() && json[JSON_TITLE].is_string()) {
        json.at(JSON_TITLE).get_to(taskInfo.title);
    }
    if (json.find(JSON_DESCRIPTION) != json.end() && json[JSON_DESCRIPTION].is_string()) {
        json.at(JSON_DESCRIPTION).get_to(taskInfo.description);
    }
    if (json.find(JSON_ACTION) != json.end() && json[JSON_ACTION].is_number()) {
        json.at(JSON_ACTION).get_to(taskInfo.action);
    }
    if (json.find(JSON_MODE) != json.end() && json[JSON_MODE].is_number()) {
        json.at(JSON_MODE).get_to(taskInfo.mode);
    }
    if (json.find(JSON_MIME_TYPE) != json.end() && json[JSON_MIME_TYPE].is_string()) {
        json.at(JSON_MIME_TYPE).get_to(taskInfo.mimeType);
    }
    if (json.find(JSON_PROGRESS) != json.end() && json[JSON_PROGRESS].is_object()) {
        json.at(JSON_PROGRESS).get_to(taskInfo.progress);
    }
    if (json.find(JSON_RESPONSE) != json.end() && json[JSON_RESPONSE].is_object()) {
        json.at(JSON_RESPONSE).get_to(taskInfo.response);
    }
    if (json.find(JSON_GAUGE) != json.end() && json[JSON_GAUGE].is_boolean()) {
        json.at(JSON_GAUGE).get_to(taskInfo.gauge);
    }
    if (json.find(JSON_CTIME) != json.end() && json[JSON_CTIME].is_number()) {
        json.at(JSON_CTIME).get_to(taskInfo.ctime);
    }
    if (json.find(JSON_MTIME) != json.end() && json[JSON_MTIME].is_number()) {
        json.at(JSON_MTIME).get_to(taskInfo.mtime);
    }
    if (json.find(JSON_RETRY) != json.end() && json[JSON_RETRY].is_boolean()) {
        json.at(JSON_RETRY).get_to(taskInfo.retry);
    }
    if (json.find(JSON_TRIES) != json.end() && json[JSON_TRIES].is_number()) {
        json.at(JSON_TRIES).get_to(taskInfo.tries);
    }
    if (json.find(JSON_FAULTS) != json.end() && json[JSON_FAULTS].is_number()) {
        json.at(JSON_FAULTS).get_to(taskInfo.faults);
    }
    if (json.find(JSON_CODE) != json.end() && json[JSON_CODE].is_number()) {
        json.at(JSON_CODE).get_to(taskInfo.code);
    }
    if (json.find(JSON_REASON) != json.end() && json[JSON_REASON].is_number()) {
        json.at(JSON_REASON).get_to(taskInfo.reason);
    }
    if (json.find(JSON_WITH_SYSTEM) != json.end() && json[JSON_WITH_SYSTEM].is_boolean()) {
        json.at(JSON_WITH_SYSTEM).get_to(taskInfo.withSystem);
    }
    if (json.find(JSON_EXTRAS) != json.end() && json[JSON_EXTRAS].is_object()) {
        json.at(JSON_EXTRAS).get_to(taskInfo.extras);
    }
    if (json.find(JSON_TASK_STATES) != json.end() && json[JSON_TASK_STATES].is_array()) {
        json.at(JSON_TASK_STATES).get_to(taskInfo.taskStates);
    }
}
} // namespace OHOS::Plugin::Request