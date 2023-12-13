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

#include <string>

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
    ERROR_OFFLINE,
    ERROR_UNSUPPORTED_NETWORK_TYPE,
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
enum ExceptionErrorCode : uint32_t {
    EXCEPTION_OK = 0,
    EXCEPTION_PERMISSION = 201,
    EXCEPTION_PARAMETER_CHECK = 401,
    EXCEPTION_UNSUPPORTED = 801,
    EXCEPTION_FILE_IO = 13400001,
    EXCEPTION_FILE_PATH = 13400002,
    EXCEPTION_SERVICE_ERROR = 13400003,
    EXCEPTION_OTHER = 13499999,
};

struct ExceptionError {
    ExceptionErrorCode code;
    std::string errInfo;
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

static constexpr const char *EXCEPTION_OK_INFO = "check succeeded";
static constexpr const char *EXCEPTION_PERMISSION_INFO = "the permissions check fails";
static constexpr const char *EXCEPTION_PARAMETER_CHECK_INFO = "the parameters check fails";
static constexpr const char *EXCEPTION_UNSUPPORTED_INFO = "call unsupported api";
static constexpr const char *EXCEPTION_FILE_IO_INFO = " file operation error";
static constexpr const char *EXCEPTION_FILE_PATH_INFO = "bad file path";
static constexpr const char *EXCEPTION_SERVICE_ERROR_INFO = "task service ability error";
static constexpr const char *EXCEPTION_OTHER_INFO = "other error";

static constexpr const char *FUNCTION_SUSPEND = "suspend";
static constexpr const char *FUNCTION_GET_TASK_INFO = "getTaskInfo";
static constexpr const char *FUNCTION_GET_TASK_MIME_TYPE = "getTaskMimeType";
static constexpr const char *FUNCTION_DELETE = "delete";
static constexpr const char *FUNCTION_RESTORE = "restore";

static const std::string tlsVersion = "X-TLS-Version";
static const std::string cipherList = "X-Cipher-List";
static const std::string TLS_VERSION = "CURL_SSLVERSION_TLSv1_2";
static const std::string TLS_CIPHER = "TLS_DHE_RSA_WITH_AES_128_GCM_SHA256,TLS_DHE_RSA_WITH_AES_256_GCM_SHA384,"
                                      "TLS_DHE_DSS_WITH_AES_128_GCM_SHA256,TLS_DSS_RSA_WITH_AES_256_GCM_SHA384,"
                                      "TLS_PSK_WITH_AES_256_GCM_SHA384,TLS_DHE_PSK_WITH_AES_128_GCM_SHA256,"
                                      "TLS_DHE_PSK_WITH_AES_256_GCM_SHA384,"
                                      "TLS_DHE_PSK_WITH_CHACHA20_POLY1305_SHA256,"
                                      "TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,"
                                      "TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,"
                                      "TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,"
                                      "TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256,"
                                      "TLS_ECDHE_PSK_WITH_CHACHA20_POLY1305_SHA256,"
                                      "TLS_ECDHE_PSK_WITH_AES_128_GCM_SHA256,TLS_ECDHE_PSK_WITH_AES_256_GCM_SHA384,"
                                      "TLS_ECDHE_PSK_WITH_AES_128_GCM_SHA256,"
                                      "TLS_DHE_RSA_WITH_AES_128_CCM,TLS_DHE_RSA_WITH_AES_256_CCM,"
                                      "TLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256,TLS_PSK_WITH_AES_256_CCM,"
                                      "TLS_DHE_PSK_WITH_AES_128_CCM,TLS_DHE_PSK_WITH_AES_256_CCM,"
                                      "TLS_ECDHE_ECDSA_WITH_AES_128_CCM,TLS_ECDHE_ECDSA_WITH_AES_256_CCM,"
                                      "TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256,TLS_AES_128_GCM_SHA256,"
                                      "TLS_AES_256_GCM_SHA384,TLS_CHACHA20_POLY1305_SHA256,TLS_AES_128_CCM_SHA256,"
                                      "TLS_SM4_GCM_SM3,TLS_SM4_CCM_SM3";

static constexpr int RDB_EXECUTE_OK = 0;
static constexpr int RDB_EXECUTE_FAIL = -1;
static constexpr int OPERATION_OK = 0;
static constexpr int OPERATION_ERROR = -1;

static constexpr int RDB_OBJECT_EMPTY = -1;
static constexpr int PARAMETER_EMPTY = -1;

static constexpr int TRANSACTION_COUNT = 100;

constexpr int DOWNLOAD_INFO = 10000;

constexpr int REQUEST_PARAMS_NUM = 2;
constexpr size_t MIN_HEADER_LENGTH = sizeof(std::string) * 2;

static constexpr const char *EVENT_PROGRESS = "progress";
static constexpr const char *EVENT_COMPLETE = "complete";
static constexpr const char *EVENT_FAIL = "fail";
static constexpr size_t ASYNC_DEFAULT_POS = -1;
} // namespace OHOS::Plugin::Request::Download

#endif // PLUGINS_REQUEST_CONSTANT_H
