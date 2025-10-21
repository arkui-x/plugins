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
#include "json_parser.h"

#include "file_utils.h"
#include "sensors_errors.h"

#undef LOG_TAG
#define LOG_TAG "JsonParser"

namespace OHOS {
namespace Sensors {

JsonParser::JsonParser(const std::string &filePath)
{
    std::string jsonStr = ReadJsonFile(filePath);
    if (jsonStr.empty()) {
        MISC_HILOGE("Read json file fail");
        return;
    }
    cJson_ = cJSON_Parse(jsonStr.c_str());
}

JsonParser::JsonParser(const RawFileDescriptor &rawFd)
{
    std::string jsonStr = ReadFd(rawFd);
    if (jsonStr.empty()) {
        MISC_HILOGE("Read fd fail");
        return;
    }
    cJson_ = cJSON_Parse(jsonStr.c_str());
}

JsonParser::~JsonParser()
{
    if (cJson_ != nullptr) {
        cJSON_Delete(cJson_);
    }
}

bool JsonParser::HasObjectItem(cJSON *json, const std::string &key) const
{
    return cJSON_HasObjectItem(json, key.c_str());
}

bool JsonParser::HasObjectItem(const std::string &key) const
{
    return HasObjectItem(cJson_, key.c_str());
}

cJSON *JsonParser::GetObjectItem(cJSON *json, const std::string &key) const
{
    if (!cJSON_IsObject(json)) {
        MISC_HILOGE("The json is not object");
        return nullptr;
    }
    if (!cJSON_HasObjectItem(json, key.c_str())) {
        MISC_HILOGE("The json is not data:%{public}s", key.c_str());
        return nullptr;
    }
    return cJSON_GetObjectItem(json, key.c_str());
}

cJSON *JsonParser::GetObjectItem(const std::string &key) const
{
    return GetObjectItem(cJson_, key);
}

int32_t JsonParser::ParseJsonArray(cJSON *json, const std::string &key,
    std::vector<std::string> &vals) const
{
    cJSON *jsonArray = GetObjectItem(json, key);
    if (!cJSON_IsArray(jsonArray)) {
        MISC_HILOGE("The value of %{public}s is not array", key.c_str());
        return ERROR;
    }
    int32_t size = cJSON_GetArraySize(jsonArray);
    for (int32_t i = 0; i < size; ++i) {
        cJSON *val = cJSON_GetArrayItem(jsonArray, i);
        if ((!cJSON_IsString(val)) || (val->valuestring == nullptr)) {
            MISC_HILOGE("The value of index %{public}d is not string", i);
            return ERROR;
        }
        vals.push_back(val->valuestring);
    }
    return SUCCESS;
}

int32_t JsonParser::ParseJsonArray(const std::string &key, std::vector<std::string> &vals) const
{
    return ParseJsonArray(cJson_, key, vals);
}

bool JsonParser::IsArray(cJSON *json) const
{
    return cJSON_IsArray(json);
}

int32_t JsonParser::GetArraySize(cJSON *json) const
{
    return cJSON_GetArraySize(json);
}

cJSON *JsonParser::GetArrayItem(cJSON *json, int32_t index) const
{
    return cJSON_GetArrayItem(json, index);
}

int32_t JsonParser::GetIntValue(cJSON *json) const
{
    if (!cJSON_IsNumber(json)) {
        return static_cast<int32_t>(NAN);
    }
    return json->valueint;
}

double JsonParser::GetDoubleValue(cJSON *json) const
{
    if (!cJSON_IsNumber(json)) {
        return static_cast<double>(NAN);
    }
    return json->valuedouble;
}

std::string JsonParser::GetStringValue(cJSON *json) const
{
    if (!cJSON_IsString(json)) {
        return NULL;
    }
    return json->valuestring;
}
} // namespace Sensors
} // namespace OHOS