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

#ifndef PLUGINS_REQUEST_CONSTANT_H
#define PLUGINS_REQUEST_CONSTANT_H

namespace OHOS::Plugin::Request::Download {
enum NetworkType {
    NETWORK_INVALID = 0x00000000,
    NETWORK_MOBILE = 0x00000001,
    NETWORK_WIFI = 0x00010000,
    NETWORK_MASK = 0x00010001,
};

enum ErrorCode {
    ERROR_CANNOT_RESUME,
    ERROR_DEVICE_NOT_FOUND,
    ERROR_FILE_ALREADY_EXISTS,
    ERROR_FILE_ERROR,
    ERROR_HTTP_DATA_ERROR,
    ERROR_INSUFFICIENT_SPACE,
    ERROR_TOO_MANY_REDIRECTS,
    ERROR_UNHANDLED_HTTP_CODE,
    ERROR_UNKNOWN,
    ERROR_NETWORK_FAIL,
};

enum PausedReason {
    PAUSED_QUEUED_FOR_WIFI,
    PAUSED_WAITING_FOR_NETWORK,
    PAUSED_WAITING_TO_RETRY,
    PAUSED_BY_USER,
    PAUSED_UNKNOWN,
};

enum DownloadStatus {
    SESSION_SUCCESS,
    SESSION_RUNNING,
    SESSION_PENDING,
    SESSION_PAUSED,
    SESSION_FAILED,
    SESSION_UNKNOWN,
};

enum HttpErrorCode {
    HTTP_OK = 200,
    HTTP_PARIAL_FILE = 206,
};

const uint32_t DEFAULT_READ_TIMEOUT = 60;
const uint32_t DEFAULT_CONNECT_TIMEOUT = 60;
const uint32_t HTTP_FORCE_STOP = 1;
const uint32_t DEFAULT_LOW_SPEED_LIMIT = 30;

static constexpr const char *HTTP_METHOD_GET = "GET";
static constexpr const char *HTTP_URL_PARAM_START = "?";
static constexpr const char *HTTP_URL_PARAM_SEPARATOR = "&";
static constexpr const char *HTTP_URL_NAME_VALUE_SEPARATOR = "=";
static constexpr const char *HTTP_HEADER_SEPARATOR = ":";
static constexpr const char *HTTP_LINE_SEPARATOR = "\r\n";

static constexpr const char *HTTP_DEFAULT_USER_AGENT = "libcurl-agent/1.0";
static constexpr const char *HTTP_DEFAULT_CA_PATH = "cacert.pem";

static constexpr const char *HTTP_CONTENT_TYPE = "content-type";
static constexpr const char *HTTP_CONTENT_TYPE_TEXT = "text/plain";
static constexpr const char *HTTP_CONTENT_TYPE_URL_ENCODE = "application/x-www-form-urlencoded";
static constexpr const char *HTTP_CONTENT_TYPE_JSON = "application/json";

static constexpr int RDB_EXECUTE_OK = 0;
static constexpr int RDB_EXECUTE_FAIL = -1;
static constexpr int OPERATION_OK = 0;
static constexpr int OPERATION_ERROR = -1;

static constexpr int RDB_OBJECT_EMPTY = -1;
static constexpr int PARAMETER_EMPTY = -1;

static constexpr int TRANSACTION_COUNT = 100;

constexpr int DOWNLOAD_INFO = 10000;

constexpr int REQUEST_PARAMS_NUM = 2;
} // namespace OHOS::Plugin::Request::Download

#endif // PLUGINS_REQUEST_CONSTANT_H
