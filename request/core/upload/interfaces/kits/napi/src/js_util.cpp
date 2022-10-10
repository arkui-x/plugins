/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "js_util.h"

using namespace  OHOS::Plugin::Request::Upload;
namespace  OHOS::Plugin::Request::UploadNapi {
std::string JSUtil::Convert2String(napi_env env, napi_value jsString)
{
    size_t maxLen = JSUtil::MAX_LEN;
    napi_status status = napi_get_value_string_utf8(env, jsString, NULL, 0, &maxLen);
    if (status != napi_ok) {
        GET_AND_THROW_LAST_ERROR((env));
        maxLen = JSUtil::MAX_LEN;
    }
    if (maxLen == 0) {
        return std::string();
    }
    char *buf = new char[maxLen + 1];
    if (buf == nullptr) {
        return std::string();
    }
    size_t len = 0;
    status = napi_get_value_string_utf8(env, jsString, buf, maxLen + 1, &len);
    if (status != napi_ok) {
        GET_AND_THROW_LAST_ERROR((env));
    }
    buf[len] = 0;
    std::string value(buf);
    delete[] buf;
    return value;
}

std::vector<std::string> JSUtil::Convert2StrVector(napi_env env, napi_value value)
{
    uint32_t arrLen = 0;
    napi_get_array_length(env, value, &arrLen);
    if (arrLen == 0) {
        return {};
    }
    std::vector<std::string> result;
    for (size_t i = 0; i < arrLen; ++i) {
        napi_value element;
        napi_get_element(env, value, i, &element);
        result.push_back(Convert2String(env, element));
    }
    return result;
}

std::vector<std::string> JSUtil::Convert2Header(napi_env env, napi_value value)
{
    std::vector<std::string> result;
    napi_value keyArr = nullptr;
    napi_status status = napi_get_property_names(env, value, &keyArr);
    if (status != napi_ok) {
        return result;
    }

    uint32_t len = 0;
    status = napi_get_array_length(env, keyArr, &len);
    if (status != napi_ok) {
        UPLOAD_HILOGE(UPLOAD_MODULE_JS_NAPI, "Convert2Header. napi_get_array_length statue Error");
        return result;
    }
    for (uint32_t i = 0; i < len; i++) {
        napi_value keyNapiValue = nullptr;
        napi_get_element(env, keyArr, i, &keyNapiValue);

        napi_valuetype valueType;
        napi_typeof(env, keyNapiValue, &valueType);
        if (valueType != napi_valuetype::napi_string) {
            continue;
        }

        char key[JSUtil::MAX_LEN] = { 0 };
        size_t cValueLength = 0;
        napi_get_value_string_utf8(env, keyNapiValue, key, JSUtil::MAX_LEN - 1, &cValueLength);

        napi_value jsvalue = nullptr;
        napi_get_named_property(env, value, key, &jsvalue);
        if (jsvalue != nullptr) {
            std::string item(key);
            item.append(SEPARATOR);
            item.append(Convert2String(env, jsvalue));
            result.push_back(item);
        }
    }
    return result;
}

napi_value JSUtil::Convert2JSStringVector(napi_env env, const std::vector<std::string> &cStrings)
{
    napi_value jsStrings = nullptr;
    napi_create_array_with_length(env, cStrings.size(), &jsStrings);
    int index = 0;
    for (const auto &cString : cStrings) {
        napi_value jsString = Convert2JSString(env, cString);
        napi_set_element(env, jsStrings, index++, jsString);
    }
    return jsStrings;
}

napi_value JSUtil::Convert2JSValue(napi_env env, const std::vector<int32_t> &cInts)
{
    napi_value jsInts = nullptr;
    napi_create_array_with_length(env, cInts.size(), &jsInts);
    int index = 0;
    for (const auto &cInt : cInts) {
        napi_value jsInt = Convert2JSValue(env, cInt);
        napi_set_element(env, jsInts, index++, jsInt);
    }
    return jsInts;
}

napi_value JSUtil::Convert2JSUploadResponse(napi_env env, const Upload::UploadResponse &response)
{
    napi_value jsResponse = nullptr;
    napi_create_object(env, &jsResponse);
    napi_set_named_property(env, jsResponse, "code", Convert2JSValue(env, response.code));
    napi_set_named_property(env, jsResponse, "data", Convert2JSString(env, response.data));
    napi_set_named_property(env, jsResponse, "headers", Convert2JSString(env, response.headers));
    return jsResponse;
}

napi_value JSUtil::Convert2JSValue(napi_env env, const std::vector<Upload::TaskState> &taskStates)
{
    napi_value jsTaskStates = nullptr;
    napi_create_array_with_length(env, taskStates.size(), &jsTaskStates);
    int index = 0;
    for (const auto &taskState : taskStates) {
        napi_value jsTaskState = nullptr;
        napi_create_object(env, &jsTaskState);
        napi_set_named_property(env, jsTaskState, "path", Convert2JSString(env, taskState.path));
        napi_set_named_property(env, jsTaskState, "responseCode ", Convert2JSValue(env, taskState.responseCode));
        napi_set_named_property(env, jsTaskState, "message", Convert2JSString(env, taskState.message));
        napi_set_element(env, jsTaskStates, index++, jsTaskState);
    }
    return jsTaskStates;
}

napi_value JSUtil::Convert2JSValue(napi_env env, int32_t value)
{
    napi_value jsValue;
    napi_status status = napi_create_int32(env, value, &jsValue);
    if (status != napi_ok) {
        return nullptr;
    }
    return jsValue;
}

void JSUtil::ParseFunction(napi_env env, napi_value &object, const char *name, bool &hasFunction, napi_ref &output)
{
    napi_value value = nullptr;
    auto ret = napi_get_named_property(env, object, name, &value);
    if ((ret == napi_ok) && (value != nullptr)) {
        napi_valuetype valueType = napi_null;
        NAPI_CALL_RETURN_VOID(env, napi_typeof(env, value, &valueType));
        if (valueType == napi_undefined) {
            return;
        }
        NAPI_ASSERT_RETURN_VOID(env, valueType == napi_function, "Wrong argument, function expected.");
        NAPI_CALL_RETURN_VOID(env, napi_create_reference(env, value, 1, &output));
        hasFunction = true;
    }
}

napi_value JSUtil::Convert2JSString(napi_env env, const std::string &cString)
{
    napi_value jsValue = nullptr;
    napi_create_string_utf8(env, cString.c_str(), cString.size(), &jsValue);
    return jsValue;
}

std::shared_ptr<Upload::UploadConfig> JSUtil::Convert2UploadConfig(napi_env env, napi_value jsConfig)
{
    Upload::UploadConfig config;
    napi_value value = nullptr;
    napi_get_named_property(env, jsConfig, "url", &value);
    if (value != nullptr) {
        config.url = Convert2String(env, value);
    }
    value = nullptr;
    napi_get_named_property(env, jsConfig, "header", &value);
    if (value != nullptr) {
        config.header = Convert2Header(env, value);
    }
    value = nullptr;
    napi_get_named_property(env, jsConfig, "method", &value);
    if (value != nullptr) {
        config.method = Convert2String(env, value);
    }
    value = nullptr;
    napi_get_named_property(env, jsConfig, "files", &value);
    if (value != nullptr) {
        config.files = Convert2FileVector(env, value);
    }
    value = nullptr;
    napi_get_named_property(env, jsConfig, "data", &value);
    if (value != nullptr) {
        config.data = Convert2RequestDataVector(env, value);
    }

    std::shared_ptr<Upload::UploadConfig> tmpConfig = std::make_shared<Upload::UploadConfig>(config);
    return tmpConfig;
}

napi_value JSUtil::Convert2JSUploadConfig(napi_env env, const Upload::UploadConfig &config)
{
    napi_value jsConfig = nullptr;
    napi_create_object(env, &jsConfig);
    napi_set_named_property(env, jsConfig, "url", Convert2JSString(env, config.url));
    napi_set_named_property(env, jsConfig, "header", Convert2JSStringVector(env, config.header));
    napi_set_named_property(env, jsConfig, "method", Convert2JSString(env, config.method));
    napi_set_named_property(env, jsConfig, "files", Convert2JSFileVector(env, config.files));
    napi_set_named_property(env, jsConfig, "data", Convert2JSRequestDataVector(env, config.data));
    return jsConfig;
}

Upload::File JSUtil::Convert2File(napi_env env, napi_value jsFile)
{
    Upload::File file;
    napi_value value = nullptr;
    napi_get_named_property(env, jsFile, "filename", &value);
    if (value != nullptr) {
        file.filename = Convert2String(env, value);
    }
    value = nullptr;
    napi_get_named_property(env, jsFile, "name", &value);
    if (value != nullptr) {
        file.name = Convert2String(env, value);
    }
    value = nullptr;
    napi_get_named_property(env, jsFile, "uri", &value);
    if (value != nullptr) {
        file.uri = Convert2String(env, value);
    }
    value = nullptr;
    napi_get_named_property(env, jsFile, "type", &value);
    if (value != nullptr) {
        file.type = Convert2String(env, value);
    }
    return file;
}

napi_value JSUtil::Convert2JSFile(napi_env env, const Upload::File &file)
{
    napi_value jsFile = nullptr;
    napi_create_object(env, &jsFile);
    napi_set_named_property(env, jsFile, "filename", Convert2JSString(env, file.filename));
    napi_set_named_property(env, jsFile, "name", Convert2JSString(env, file.name));
    napi_set_named_property(env, jsFile, "uri", Convert2JSString(env, file.uri));
    napi_set_named_property(env, jsFile, "type", Convert2JSString(env, file.type));
    return jsFile;
}

std::vector<Upload::File> JSUtil::Convert2FileVector(napi_env env, napi_value jsFiles)
{
    bool isArray = false;
    napi_is_array(env, jsFiles, &isArray);
    NAPI_ASSERT_BASE(env, isArray, "not array", { });
    uint32_t length = 0;
    napi_get_array_length(env, jsFiles, &length);
    std::vector<Upload::File> files;
    for (uint32_t i = 0; i < length; ++i) {
        napi_value file = nullptr;
        napi_get_element(env, jsFiles, i, &file);
        if (file == nullptr) {
            continue;
        }
        files.push_back(Convert2File(env, file));
    }
    return files;
}

napi_value JSUtil::Convert2JSFileVector(napi_env env, const std::vector<Upload::File> &files)
{
    napi_value jsFiles = nullptr;
    napi_create_array_with_length(env, files.size(), &jsFiles);
    int index = 0;
    for (const auto &file : files) {
        napi_value jsFile = Convert2JSFile(env, file);
        napi_set_element(env, jsFiles, index++, jsFile);
    }
    return jsFiles;
}

Upload::RequestData JSUtil::Convert2RequestData(napi_env env, napi_value jsRequestData)
{
    Upload::RequestData requestData;
    napi_value value = nullptr;
    napi_get_named_property(env, jsRequestData, "name", &value);
    if (value != nullptr) {
        requestData.name = Convert2String(env, value);
    }
    value = nullptr;
    napi_get_named_property(env, jsRequestData, "value", &value);
    if (value != nullptr) {
        requestData.value = Convert2String(env, value);
    }
    return requestData;
}

napi_value JSUtil::Convert2JSRequestData(napi_env env, const Upload::RequestData &requestData)
{
    napi_value jsRequestData = nullptr;
    napi_create_object(env, &jsRequestData);
    napi_set_named_property(env, jsRequestData, "name", Convert2JSString(env, requestData.name));
    napi_set_named_property(env, jsRequestData, "value", Convert2JSString(env, requestData.value));
    return jsRequestData;
}

std::vector<Upload::RequestData> JSUtil::Convert2RequestDataVector(napi_env env, napi_value jsRequestDatas)
{
    bool isArray = false;
    napi_is_array(env, jsRequestDatas, &isArray);
    NAPI_ASSERT_BASE(env, isArray, "not array", { });
    uint32_t length = 0;
    napi_get_array_length(env, jsRequestDatas, &length);
    std::vector<Upload::RequestData> requestDatas;
    for (uint32_t i = 0; i < length; ++i) {
        napi_value requestData = nullptr;
        napi_get_element(env, jsRequestDatas, i, &requestData);
        if (requestData == nullptr) {
            continue;
        }
        requestDatas.push_back(Convert2RequestData(env, requestData));
    }
    return requestDatas;
}

napi_value JSUtil::Convert2JSRequestDataVector(napi_env env, const std::vector<Upload::RequestData> &requestDatas)
{
    napi_value jsRequestDatas = nullptr;
    napi_create_array_with_length(env, requestDatas.size(), &jsRequestDatas);
    int index = 0;
    for (const auto &requestData : requestDatas) {
        napi_value jsRequestData = Convert2JSRequestData(env, requestData);
        napi_set_element(env, jsRequestData, index++, jsRequestDatas);
    }
    return jsRequestDatas;
}

bool JSUtil::Equals(napi_env env, napi_value value, napi_ref copy)
{
    if (copy == nullptr) {
        return (value == nullptr);
    }
    napi_value copyValue = nullptr;
    napi_get_reference_value(env, copy, &copyValue);

    bool isEquals = false;
    napi_strict_equals(env, value, copyValue, &isEquals);
    return isEquals;
}
} // namespace  OHOS::Plugin::Request::UploadNapi