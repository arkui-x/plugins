/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "napi_utils.h"

#include <cstring>
#include <initializer_list>
#include <memory>

#include "log.h"
#include "securec.h"

namespace OHOS::Plugin::Request::NapiUtils {
static constexpr const int MAX_STRING_LENGTH = 65536;
static constexpr int64_t JS_NUMBER_MAX_VALUE = (1LL << 53) - 1;
static constexpr size_t UTF8_MULTIBYTE_MIN_LEN = 2;
static constexpr const char *REASON_OK_INFO = "Task successful";
static constexpr const char *TASK_SURVIVAL_ONE_MONTH_INFO = "The task has not been completed for a month yet";
static constexpr const char *WAITTING_NETWORK_ONE_DAY_INFO = "The task waiting for network recovery has not been "
                                                             "completed for a day yet";
static constexpr const char *STOPPED_NEW_FRONT_TASK_INFO = "Stopped by a new front task";
static constexpr const char *RUNNING_TASK_MEET_LIMITS_INFO = "Too many task in running state";
static constexpr const char *USER_OPERATION_INFO = "User operation";
static constexpr const char *APP_BACKGROUND_OR_TERMINATE_INFO = "The app is background or terminate";
static constexpr const char *NETWORK_OFFLINE_INFO = "NetWork is offline";
static constexpr const char *UNSUPPORTED_NETWORK_TYPE_INFO = "NetWork type not meet the task config";
static constexpr const char *BUILD_CLIENT_FAILED_INFO = "Build client error";
static constexpr const char *BUILD_REQUEST_FAILED_INFO = "Build request error";
static constexpr const char *GET_FILESIZE_FAILED_INFO = "Failed because cannot get the file size from the server and "
                                                        "the precise is setted true by user";
static constexpr const char *CONTINUOUS_TASK_TIMEOUT_INFO = "Continuous processing task time out";
static constexpr const char *CONNECT_ERROR_INFO = "Connect error";
static constexpr const char *REQUEST_ERROR_INFO = "Request error";
static constexpr const char *UPLOAD_FILE_ERROR_INFO = "There are some files upload failed";
static constexpr const char *REDIRECT_ERROR_INFO = "Redirect error";
static constexpr const char *PROTOCOL_ERROR_INFO = "Http protocol error";
static constexpr const char *IO_ERROR_INFO = "Io Error";
static constexpr const char *UNSUPPORT_RANGE_REQUEST_INFO = "Range request not supported";
static constexpr const char *OTHERS_ERROR_INFO = "Some other error occured";
static const std::map<ExceptionErrorCode, std::string> ErrorCodeToMsg {
    {E_OK, E_OK_INFO },
    {E_PERMISSION, E_PERMISSION_INFO },
    {E_PARAMETER_CHECK, E_PARAMETER_CHECK_INFO },
    {E_UNSUPPORTED, E_UNSUPPORTED_INFO },
    {E_FILE_IO, E_FILE_IO_INFO },
    {E_FILE_PATH, E_FILE_PATH_INFO },
    {E_SERVICE_ERROR, E_SERVICE_ERROR_INFO },
    {E_TASK_QUEUE, E_TASK_QUEUE_INFO },
    {E_TASK_MODE, E_TASK_MODE_INFO },
    {E_TASK_NOT_FOUND, E_TASK_NOT_FOUND_INFO },
    {E_TASK_STATE, E_TASK_STATE_INFO },
    {E_OTHER, E_OTHER_INFO },
};

napi_status Convert2JSValue(napi_env env, const DownloadInfo &in, napi_value &out)
{
    napi_create_object(env, &out);
    SetStringPropertyUtf8(env, out, "description", in.description);
    SetUint32Property(env, out, "downloadedBytes", in.downloadedBytes);
    SetUint32Property(env, out, "downloadId", in.downloadId);
    SetUint32Property(env, out, "failedReason", in.failedReason);
    SetStringPropertyUtf8(env, out, "fileName", in.fileName);
    SetStringPropertyUtf8(env, out, "filePath", in.filePath);
    SetUint32Property(env, out, "pausedReason", in.pausedReason);
    SetUint32Property(env, out, "status", in.status);
    SetStringPropertyUtf8(env, out, "targetURI", in.url);
    SetStringPropertyUtf8(env, out, "downloadTitle", in.downloadTitle);
    SetInt64Property(env, out, "downloadTotalBytes", in.downloadTotalBytes);
    return napi_ok;
}

napi_status Convert2JSValue(napi_env env, std::string &in, napi_value &out)
{
    return napi_create_string_utf8(env, in.c_str(), strlen(in.c_str()), &out);
}

napi_status Convert2JSValue(napi_env env, bool in, napi_value &out)
{
    return napi_get_boolean(env, in, &out);
}

napi_value Convert2JSValue(napi_env env, bool code)
{
    napi_value value = nullptr;
    if (napi_get_boolean(env, code, &value) != napi_ok) {
        return nullptr;
    }
    return value;
}

napi_value Convert2JSValue(napi_env env, int32_t code)
{
    napi_value value = nullptr;
    if (napi_create_int32(env, code, &value) != napi_ok) {
        return nullptr;
    }
    return value;
}

napi_value Convert2JSValue(napi_env env, uint32_t code)
{
    napi_value value = nullptr;
    if (napi_create_uint32(env, code, &value) != napi_ok) {
        return nullptr;
    }
    return value;
}

napi_value Convert2JSValue(napi_env env, int64_t code)
{
    napi_value value = nullptr;
    if (napi_create_int64(env, code, &value) != napi_ok) {
        return nullptr;
    }
    return value;
}

napi_value Convert2JSValue(napi_env env, uint64_t code)
{
    if (code > JS_NUMBER_MAX_VALUE) {
        return nullptr;
    }
    napi_value value = nullptr;
    if (napi_create_int64(env, static_cast<int64_t>(code), &value) != napi_ok) {
        return nullptr;
    }
    return value;
}

napi_value Convert2JSValue(napi_env env, const std::vector<int64_t> &code)
{
    napi_value value = nullptr;
    napi_create_array_with_length(env, code.size(), &value);
    int index = 0;
    for (const auto &cInt : code) {
        napi_value jsInt = Convert2JSValue(env, cInt);
        napi_set_element(env, value, index++, jsInt);
    }
    return value;
}

napi_value Convert2JSValue(napi_env env, const std::vector<std::string> &ids)
{
    napi_value value = nullptr;
    napi_create_array_with_length(env, ids.size(), &value);
    int index = 0;
    for (const auto &id : ids) {
        napi_set_element(env, value, index++, Convert2JSValue(env, id));
    }
    return value;
}

napi_value Convert2JSHeaders(napi_env env, const Progress &progress)
{
    std::vector<uint8_t> bodyBytes = progress.bodyBytes;
    napi_value headers = nullptr;
    napi_create_object(env, &headers);
    for (const auto &cInt : progress.extras) {
        napi_set_named_property(env, headers, cInt.first.c_str(), Convert2JSValue(env, cInt.second));
    }
    napi_value body = nullptr;
    if (IsTextUTF8(bodyBytes)) {
        napi_create_string_utf8(env, reinterpret_cast<const char *>(bodyBytes.data()), bodyBytes.size(), &body);
    } else {
        uint8_t *data = nullptr;
        napi_create_arraybuffer(env, bodyBytes.size(), reinterpret_cast<void **>(&data), &body);
        if (memcpy_s(data, bodyBytes.size(), bodyBytes.data(), bodyBytes.size()) == 0) {
            REQUEST_HILOGW("Body data memcpy_s error");
        }
    }

    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_set_named_property(env, object, "headers", headers);
    napi_set_named_property(env, object, "body", body);
    return object;
}

napi_value Convert2JSHeadersAndBody(napi_env env, const std::map<std::string, std::string> &header,
    const std::vector<uint8_t> &bodyBytes, bool isSeparate)
{
    napi_value headers = nullptr;
    napi_create_object(env, &headers);
    for (const auto &cInt : header) {
        napi_set_named_property(env, headers, cInt.first.c_str(), Convert2JSValue(env, cInt.second));
    }
    napi_value body = nullptr;
    if (IsTextUTF8(bodyBytes)) {
        napi_create_string_utf8(env, reinterpret_cast<const char *>(bodyBytes.data()), bodyBytes.size(), &body);
    } else {
        uint8_t *data = nullptr;
        napi_create_arraybuffer(env, bodyBytes.size(), reinterpret_cast<void **>(&data), &body);
        if (memcpy_s(data, bodyBytes.size(), bodyBytes.data(), bodyBytes.size()) == 0) {
            REQUEST_HILOGW("Body data memcpy_s error");
        }
    }

    if (isSeparate) {
        napi_value object = nullptr;
        napi_create_object(env, &object);
        napi_set_named_property(env, object, "headers", headers);
        napi_set_named_property(env, object, "body", body);
        return object;
    } else {
        napi_set_named_property(env, headers, "body", body);
        return headers;
    }
}

napi_value Convert2JSValue(napi_env env, const std::map<std::string, std::string> &code)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    for (const auto &cInt : code) {
        napi_set_named_property(env, object, cInt.first.c_str(), Convert2JSValue(env, cInt.second));
    }
    return object;
}

napi_value Convert2JSValue(napi_env env, const std::string &str)
{
    napi_value value = nullptr;
    if (napi_create_string_utf8(env, str.c_str(), strlen(str.c_str()), &value) != napi_ok) {
        return nullptr;
    }
    return value;
}

napi_value Convert2JSValue(napi_env env, const std::vector<TaskState> &taskStates)
{
    napi_value value = nullptr;
    napi_create_array_with_length(env, taskStates.size(), &value);
    int index = 0;
    for (const auto &taskState : taskStates) {
        napi_value jsTaskState = nullptr;
        napi_create_object(env, &jsTaskState);
        napi_set_named_property(env, jsTaskState, "path", Convert2JSValue(env, taskState.path));
        napi_set_named_property(env, jsTaskState, "responseCode", Convert2JSValue(env, taskState.responseCode));
        napi_set_named_property(env, jsTaskState, "message", Convert2JSValue(env, taskState.message));
        napi_set_element(env, value, index++, jsTaskState);
    }
    return value;
}

napi_value Convert2JSValue(napi_env env, const Progress &progress)
{
    napi_value value = nullptr;
    napi_create_object(env, &value);
    napi_set_named_property(env, value, "state", Convert2JSValue(env, static_cast<uint32_t>(progress.state)));
    napi_set_named_property(env, value, "index", Convert2JSValue(env, progress.index));
    napi_set_named_property(env, value, "processed", Convert2JSValue(env, progress.processed));
    napi_set_named_property(env, value, "sizes", Convert2JSValue(env, progress.sizes));
    napi_set_named_property(env, value, "extras",
        Convert2JSHeadersAndBody(env, progress.extras, progress.bodyBytes, false));
    return value;
}

napi_value Convert2JSValue(napi_env env, const Response &response)
{
    napi_value value = nullptr;
    napi_create_object(env, &value);

    napi_set_named_property(env, value, "version", Convert2JSValue(env, response.version));
    napi_set_named_property(env, value, "statusCode", Convert2JSValue(env, response.statusCode));
    napi_set_named_property(env, value, "reason", Convert2JSValue(env, response.reason));

    napi_value headers = nullptr;
    napi_create_object(env, &headers);
    
    for (const auto &header : response.headers) {
        const std::string &key = header.first;
        const std::vector<std::string> &values = header.second;
        napi_value jsValues = nullptr;
        napi_create_array_with_length(env, values.size(), &jsValues);
        for (size_t i = 0; i < values.size(); i++) {
            napi_set_element(env, jsValues, i, Convert2JSValue(env, values[i]));
        }
        napi_set_named_property(env, headers, key.c_str(), jsValues);
    }
    napi_set_named_property(env, value, "headers", headers);
    
    return value;
}

napi_value Convert2JSValue(napi_env env, const std::vector<FileSpec> &files, const std::vector<FormItem> &forms)
{
    napi_value data = nullptr;
    size_t filesLen = files.size();
    size_t formsLen = forms.size();
    napi_create_array_with_length(env, filesLen + formsLen, &data);
    size_t i = 0;
    for (; i < formsLen; i++) {
        napi_value object = nullptr;
        napi_create_object(env, &object);
        napi_set_named_property(env, object, "name", Convert2JSValue(env, forms[i].name));
        napi_set_named_property(env, object, "value", Convert2JSValue(env, forms[i].value));
        napi_set_element(env, data, i, object);
    }
    for (size_t j = 0; j < filesLen; j++) {
        napi_value fileSpec = nullptr;
        napi_create_object(env, &fileSpec);
        napi_set_named_property(env, fileSpec, "path", Convert2JSValue(env, files[j].uri));
        napi_set_named_property(env, fileSpec, "mimeType", Convert2JSValue(env, files[j].type));
        napi_set_named_property(env, fileSpec, "filename", Convert2JSValue(env, files[j].filename));
        napi_value object = nullptr;
        napi_create_object(env, &object);
        napi_set_named_property(env, object, "name", Convert2JSValue(env, files[j].name));
        napi_set_named_property(env, object, "value", fileSpec);
        napi_set_element(env, data, i, object);
        i++;
    }
    return data;
}

uint32_t Convert2Broken(Reason code)
{
    static std::map<Reason, Faults> InnerCodeToBroken = {
        { REASON_OK, Faults::OTHERS },
        { TASK_SURVIVAL_ONE_MONTH, Faults::OTHERS },
        { WAITTING_NETWORK_ONE_DAY, Faults::OTHERS },
        { STOPPED_NEW_FRONT_TASK, Faults::OTHERS },
        { RUNNING_TASK_MEET_LIMITS, Faults::OTHERS },
        { USER_OPERATION, Faults::OTHERS },
        { APP_BACKGROUND_OR_TERMINATE, Faults::OTHERS },
        { NETWORK_OFFLINE, Faults::DISCONNECTED },
        { UNSUPPORTED_NETWORK_TYPE, Faults::OTHERS },
        { BUILD_CLIENT_FAILED, Faults::OTHERS },
        { BUILD_REQUEST_FAILED, Faults::OTHERS },
        { GET_FILESIZE_FAILED, Faults::FSIO },
        { CONTINUOUS_TASK_TIMEOUT, Faults::TIMEOUT },
        { CONNECT_ERROR, Faults::PROTOCOL },
        { REQUEST_ERROR, Faults::PROTOCOL },
        { UPLOAD_FILE_ERROR, Faults::OTHERS },
        { REDIRECT_ERROR, Faults::PROTOCOL },
        { PROTOCOL_ERROR, Faults::PROTOCOL },
        { IO_ERROR, Faults::FSIO },
        { UNSUPPORT_RANGE_REQUEST, Faults::PROTOCOL },
        { OTHERS_ERROR, Faults::OTHERS },
    };
    auto iter = InnerCodeToBroken.find(code);
    if (iter != InnerCodeToBroken.end()) {
        return static_cast<uint32_t>(iter->second);
    }
    return 0;
}

std::string Convert2ReasonMsg(Reason code)
{
    static std::map<Reason, std::string> ReasonMsg = {
        { REASON_OK, REASON_OK_INFO },
        { TASK_SURVIVAL_ONE_MONTH, TASK_SURVIVAL_ONE_MONTH_INFO },
        { WAITTING_NETWORK_ONE_DAY, WAITTING_NETWORK_ONE_DAY_INFO },
        { STOPPED_NEW_FRONT_TASK, STOPPED_NEW_FRONT_TASK_INFO },
        { RUNNING_TASK_MEET_LIMITS, RUNNING_TASK_MEET_LIMITS_INFO },
        { USER_OPERATION, USER_OPERATION_INFO },
        { APP_BACKGROUND_OR_TERMINATE, APP_BACKGROUND_OR_TERMINATE_INFO },
        { NETWORK_OFFLINE, NETWORK_OFFLINE_INFO },
        { UNSUPPORTED_NETWORK_TYPE, UNSUPPORTED_NETWORK_TYPE_INFO },
        { BUILD_CLIENT_FAILED, BUILD_CLIENT_FAILED_INFO },
        { BUILD_REQUEST_FAILED, BUILD_REQUEST_FAILED_INFO },
        { GET_FILESIZE_FAILED, GET_FILESIZE_FAILED_INFO },
        { CONTINUOUS_TASK_TIMEOUT, CONTINUOUS_TASK_TIMEOUT_INFO },
        { CONNECT_ERROR, CONNECT_ERROR_INFO },
        { REQUEST_ERROR, REQUEST_ERROR_INFO },
        { UPLOAD_FILE_ERROR, UPLOAD_FILE_ERROR_INFO },
        { REDIRECT_ERROR, REDIRECT_ERROR_INFO },
        { PROTOCOL_ERROR, PROTOCOL_ERROR_INFO },
        { IO_ERROR, IO_ERROR_INFO },
        { UNSUPPORT_RANGE_REQUEST, UNSUPPORT_RANGE_REQUEST_INFO },
        { OTHERS_ERROR, OTHERS_ERROR_INFO },
    };
    auto iter = ReasonMsg.find(code);
    if (iter != ReasonMsg.end()) {
        return iter->second;
    }
    return "unknown";
}

napi_value Convert2JSValue(napi_env env, TaskInfo &taskInfo)
{
    napi_value value = nullptr;
    napi_create_object(env, &value);
    if (taskInfo.withSystem) {
        taskInfo.url = "";
        taskInfo.data = "";
        if (taskInfo.action == Action::UPLOAD) {
            taskInfo.files.clear();
            taskInfo.forms.clear();
        }
    }
    napi_set_named_property(env, value, "url", Convert2JSValue(env, taskInfo.url));
    napi_set_named_property(env, value, "saveas", Convert2JSValue(env, GetSaveas(taskInfo.files, taskInfo.action)));
    if (taskInfo.action == Action::DOWNLOAD) {
        napi_set_named_property(env, value, "data", Convert2JSValue(env, taskInfo.data));
    } else {
        napi_set_named_property(env, value, "data", Convert2JSValue(env, taskInfo.files, taskInfo.forms));
    }
    napi_set_named_property(env, value, "tid", Convert2JSValue(env, taskInfo.tid));
    napi_set_named_property(env, value, "title", Convert2JSValue(env, taskInfo.title));
    napi_set_named_property(env, value, "description", Convert2JSValue(env, taskInfo.description));
    napi_set_named_property(env, value, "action", Convert2JSValue(env, static_cast<uint32_t>(taskInfo.action)));
    napi_set_named_property(env, value, "mode", Convert2JSValue(env, static_cast<uint32_t>(taskInfo.mode)));
    napi_set_named_property(env, value, "mimeType", Convert2JSValue(env, taskInfo.mimeType));
    napi_set_named_property(env, value, "progress", Convert2JSValue(env, taskInfo.progress));
    napi_set_named_property(env, value, "response", Convert2JSValue(env, taskInfo.response));
    napi_set_named_property(env, value, "gauge", Convert2JSValue(env, taskInfo.gauge));
    napi_set_named_property(env, value, "ctime", Convert2JSValue(env, taskInfo.ctime));
    napi_set_named_property(env, value, "mtime", Convert2JSValue(env, taskInfo.mtime));
    napi_set_named_property(env, value, "retry", Convert2JSValue(env, taskInfo.retry));
    napi_set_named_property(env, value, "tries", Convert2JSValue(env, taskInfo.tries));
    if (taskInfo.code == Reason::REASON_OK) {
        napi_value value1 = nullptr;
        napi_get_null(env, &value1);
        napi_set_named_property(env, value, "faults", value1);
    } else {
        napi_set_named_property(env, value, "faults", Convert2JSValue(env, Convert2Broken(taskInfo.code)));
    }
    napi_set_named_property(env, value, "reason", Convert2JSValue(env, Convert2ReasonMsg(taskInfo.code)));
    napi_set_named_property(env, value, "extras", Convert2JSValue(env, taskInfo.extras));
    return value;
}

std::string GetSaveas(const std::vector<FileSpec> &files, Action action)
{
    if (action == Action::UPLOAD) {
        return "";
    }
    if (files.empty()) {
        return "";
    }
    return files[0].uri;
}

bool Convert2Boolean(napi_env env, napi_value object, const std::string &propertyName)
{
    if (!HasNamedProperty(env, object, propertyName)) {
        return false;
    }
    napi_value value = GetNamedProperty(env, object, propertyName);
    if (GetValueType(env, value) != napi_boolean) {
        return false;
    }
    bool ret = false;
    NAPI_CALL_BASE(env, napi_get_value_bool(env, value, &ret), false);
    return ret;
}

uint32_t Convert2Uint32(napi_env env, napi_value value)
{
    uint32_t ret = 0;
    NAPI_CALL_BASE(env, napi_get_value_uint32(env, value, &ret), 0);
    return ret;
}

uint32_t Convert2Uint32(napi_env env, napi_value object, const std::string &propertyName)
{
    if (!HasNamedProperty(env, object, propertyName)) {
        return 0;
    }
    napi_value value = GetNamedProperty(env, object, propertyName);
    if (GetValueType(env, value) != napi_number) {
        return 0;
    }
    return Convert2Uint32(env, value);
}

int64_t Convert2Int64(napi_env env, napi_value value)
{
    int64_t ret = 0;
    NAPI_CALL_BASE(env, napi_get_value_int64(env, value, &ret), 0);
    return ret;
}

int64_t Convert2Int64(napi_env env, napi_value object, const std::string &propertyName)
{
    if (!HasNamedProperty(env, object, propertyName)) {
        return 0;
    }
    napi_value value = GetNamedProperty(env, object, propertyName);
    if (GetValueType(env, value) != napi_number) {
        return 0;
    }
    return Convert2Int64(env, value);
}

std::string Convert2String(napi_env env, napi_value value)
{
    std::string result;
    std::vector<char> str(MAX_STRING_LENGTH + 1, '\0');
    size_t length = 0;
    NAPI_CALL_BASE(env, napi_get_value_string_utf8(env, value, &str[0], MAX_STRING_LENGTH, &length), result);
    if (length > 0) {
        return result.append(&str[0], length);
    }
    return result;
}

std::string Convert2String(napi_env env, napi_value object, const std::string &propertyName)
{
    if (!HasNamedProperty(env, object, propertyName)) {
        return "";
    }
    napi_value value = GetNamedProperty(env, object, propertyName);
    if (GetValueType(env, value) != napi_string) {
        return "";
    }
    return Convert2String(env, value);
}

void ThrowError(napi_env env, ExceptionErrorCode errorCode, const std::string &errMsg)
{
    napi_value businessError = nullptr;
    napi_value codeValue = nullptr;
    napi_value message = nullptr;
    napi_create_int32(env, errorCode, &codeValue);
    napi_create_string_utf8(env, errMsg.c_str(), errMsg.length(), &message);
    napi_create_error(env, nullptr, message, &businessError);
    napi_set_named_property(env, businessError, "code", codeValue);
    napi_status throwStatus = napi_throw(env, businessError);
    if (throwStatus != napi_ok) {
        REQUEST_HILOGE("Failed to throw an exception, %{public}d, code = %{public}s", throwStatus, errMsg.c_str());
    }
}

napi_value CreateBusinessError(napi_env env, ExceptionErrorCode errorCode, const std::string &msg)
{
    napi_value result = nullptr;
    napi_value codeValue = nullptr;
    napi_value message = nullptr;
    auto iter = ErrorCodeToMsg.find(errorCode);
    std::string strMsg = iter != ErrorCodeToMsg.end() ? iter->second : "general error";
    NAPI_CALL(env, napi_create_string_utf8(env, strMsg.c_str(), strMsg.length(), &message));
    NAPI_CALL(env, napi_create_int32(env, errorCode, &codeValue));
    NAPI_CALL(env, napi_create_error(env, nullptr, message, &result));
    napi_set_named_property(env, result, "code", codeValue);
    return result;
}

napi_valuetype GetValueType(napi_env env, napi_value value)
{
    if (value == nullptr) {
        return napi_undefined;
    }

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL_BASE(env, napi_typeof(env, value, &valueType), napi_undefined);
    return valueType;
}

bool HasNamedProperty(napi_env env, napi_value object, const std::string &propertyName)
{
    bool hasProperty = false;
    NAPI_CALL_BASE(env, napi_has_named_property(env, object, propertyName.c_str(), &hasProperty), false);
    return hasProperty;
}

napi_value GetNamedProperty(napi_env env, napi_value object, const std::string &propertyName)
{
    napi_value value = nullptr;
    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, object, propertyName.c_str(), &hasProperty));
    if (!hasProperty) {
        return value;
    }
    NAPI_CALL(env, napi_get_named_property(env, object, propertyName.c_str(), &value));
    return value;
}

std::vector<std::string> GetPropertyNames(napi_env env, napi_value object)
{
    std::vector<std::string> ret;
    napi_value names = nullptr;
    NAPI_CALL_BASE(env, napi_get_property_names(env, object, &names), ret);
    uint32_t length = 0;
    NAPI_CALL_BASE(env, napi_get_array_length(env, names, &length), ret);
    for (uint32_t index = 0; index < length; ++index) {
        napi_value name = nullptr;
        if (napi_get_element(env, names, index, &name) != napi_ok) {
            continue;
        }
        if (GetValueType(env, name) != napi_string) {
            continue;
        }
        ret.emplace_back(Convert2String(env, name));
    }
    return ret;
}

void SetUint32Property(napi_env env, napi_value object, const std::string &name, uint32_t value)
{
    napi_value jsValue = Convert2JSValue(env, value);
    if (GetValueType(env, jsValue) != napi_number) {
        return;
    }

    napi_set_named_property(env, object, name.c_str(), jsValue);
}

void SetInt64Property(napi_env env, napi_value object, const std::string &name, int64_t value)
{
    napi_value jsValue = Convert2JSValue(env, value);
    if (GetValueType(env, jsValue) != napi_number) {
        return;
    }

    napi_set_named_property(env, object, name.c_str(), jsValue);
}

void SetStringPropertyUtf8(napi_env env, napi_value object, const std::string &name, const std::string &value)
{
    napi_value jsValue = Convert2JSValue(env, value);
    if (GetValueType(env, jsValue) != napi_string) {
        return;
    }
    napi_set_named_property(env, object, name.c_str(), jsValue);
}

Action GetRequestAction(napi_env env, napi_value configValue)
{
    if (HasNamedProperty(env, configValue, PARAM_KEY_METHOD) || HasNamedProperty(env, configValue, PARAM_KEY_FILES) ||
        HasNamedProperty(env, configValue, PARAM_KEY_DATA)) {
        return Action::UPLOAD;
    }
    return Action::DOWNLOAD;
}

std::vector<FileSpec> Convert2FileVector(napi_env env, napi_value jsFiles)
{
    bool isArray = false;
    napi_is_array(env, jsFiles, &isArray);
    NAPI_ASSERT_BASE(env, isArray, "not array", { });
    uint32_t length = 0;
    napi_get_array_length(env, jsFiles, &length);
    std::vector<FileSpec> files;
    for (uint32_t i = 0; i < length; ++i) {
        napi_value jsFile = nullptr;
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env, &scope);
        napi_get_element(env, jsFiles, i, &jsFile);
        if (jsFile == nullptr) {
            continue;
        }
        FileSpec file;
        bool ret = Convert2File(env, jsFile, file);
        if (!ret) {
            continue;
        }
        files.emplace_back(file);
        napi_close_handle_scope(env, scope);
    }
    return files;
}

bool Convert2File(napi_env env, napi_value jsFile, FileSpec &file)
{
    napi_value filename = GetNamedProperty(env, jsFile, "filename");
    if (filename == nullptr) {
        return false;
    }
    file.filename = Convert2String(env, filename);

    napi_value name = GetNamedProperty(env, jsFile, "name");
    if (name == nullptr) {
        return false;
    }
    file.name = Convert2String(env, name);

    napi_value uri = GetNamedProperty(env, jsFile, "uri");
    if (uri == nullptr) {
        return false;
    }
    file.uri = Convert2String(env, uri);

    napi_value type = GetNamedProperty(env, jsFile, "type");
    if (type == nullptr) {
        return false;
    }
    file.type = Convert2String(env, type);
    return true;
}

std::vector<FormItem> Convert2RequestDataVector(napi_env env, napi_value jsRequestDatas)
{
    bool isArray = false;
    napi_is_array(env, jsRequestDatas, &isArray);
    NAPI_ASSERT_BASE(env, isArray, "not array", { });
    uint32_t length = 0;
    napi_get_array_length(env, jsRequestDatas, &length);
    std::vector<FormItem> requestDatas;
    for (uint32_t i = 0; i < length; ++i) {
        napi_value requestData = nullptr;
        napi_get_element(env, jsRequestDatas, i, &requestData);
        if (requestData == nullptr) {
            continue;
        }
        requestDatas.emplace_back(Convert2RequestData(env, requestData));
    }
    return requestDatas;
}

FormItem Convert2RequestData(napi_env env, napi_value jsRequestData)
{
    FormItem requestData;
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

bool IsPathValid(const std::string &filePath)
{
    auto path = filePath.substr(0, filePath.rfind('/'));
    char resolvedPath[PATH_MAX + 1] = { 0 };
    if (path.length() > PATH_MAX || realpath(path.c_str(), resolvedPath) == nullptr ||
        strncmp(resolvedPath, path.c_str(), path.length()) != 0) {
        REQUEST_HILOGE("invalid file path!");
        return false;
    }
    return true;
}

bool IsTextUTF8(const std::vector<uint8_t> &bytes)
{
    if (bytes.size() == 0) {
        return false;
    }
    auto getMultibyteLength = [](uint8_t chr) -> size_t {
        size_t num = 0;
        uint8_t mask = 0x80;
        for (size_t i = 0; i < 8; i++) {
            if ((chr & mask) == mask) {
                mask = mask >> 1;
                num++;
            } else {
                break;
            }
        }
        return num;
    };

    auto checkLength = [](const size_t count, const std::vector<uint8_t> &bytes, size_t &index) -> bool {
        for (size_t j = 0; j < count - 1; j++) {
            if ((bytes[index] & 0xc0) != 0x80) {
                return false;
            }
            index++;
        }
        return true;
    };

    size_t count = 0;
    size_t i = 0;
    while (i < bytes.size()) {
        if ((bytes[i] & 0x80) == 0x00) {
            i++;
            continue;
        } else if ((count = getMultibyteLength(bytes[i])) > UTF8_MULTIBYTE_MIN_LEN) {
            i++;
            bool isLengthOk = checkLength(count, bytes, i);
            if (!isLengthOk) {
                return false;
            }
        } else {
            return false;
        }
    }
    return true;
}

napi_status GetFunctionRef(napi_env env, napi_value value, napi_ref &ref)
{
    napi_status result = napi_ok;
    if (GetValueType(env, value) != napi_function) {
        REQUEST_HILOGE("CheckNapiType fail");
        return napi_generic_failure;
    }
    result = napi_create_reference(env, value, 1, &ref);
    if (result != napi_ok) {
        REQUEST_HILOGE("napi_create_reference fail");
    }
    return result;
}
bool Equals(napi_env env, napi_value value, napi_ref copy)
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

} // namespace OHOS::Plugin::Request::Download::NapiUtils
