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

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace OHOS::Plugin::Request {

enum class Action : uint32_t {
    DOWNLOAD = 0,
    UPLOAD,
    ANY,
};

enum class Mode : uint32_t {
    FOREGROUND = 1,
    ANY,
};

enum class Network : uint32_t {
    ANY = 0,
    WIFI,
    CELLULAR,
};

enum class Version : uint32_t {
    API9 = 0,
    API10,
    UNKNOWN,
};

enum Reason : uint32_t {
    REASON_OK = 0,
    TASK_SURVIVAL_ONE_MONTH,
    WAITTING_NETWORK_ONE_DAY,
    STOPPED_NEW_FRONT_TASK,
    RUNNING_TASK_MEET_LIMITS,
    USER_OPERATION,
    APP_BACKGROUND_OR_TERMINATE,
    NETWORK_OFFLINE,
    UNSUPPORTED_NETWORK_TYPE,
    BUILD_CLIENT_FAILED,
    BUILD_REQUEST_FAILED,
    GET_FILESIZE_FAILED,
    CONTINUOUS_TASK_TIMEOUT,
    CONNECT_ERROR,
    REQUEST_ERROR,
    UPLOAD_FILE_ERROR,
    REDIRECT_ERROR,
    PROTOCOL_ERROR,
    IO_ERROR,
    UNSUPPORT_RANGE_REQUEST,
    OTHERS_ERROR,
};

enum class State : uint32_t {
    INITIALIZED = 0x00,
    WAITING = 0x10,
    RUNNING = 0x20,
    RETRYING = 0x21,
    PAUSED = 0x30,
    STOPPED = 0x31,
    COMPLETED = 0x40,
    FAILED = 0x41,
    REMOVED = 0x50,
    DEFAULT = 0x60,
    ANY = 0x61,
};

enum class Faults : uint32_t {
    OTHERS = 0xFF,
    DISCONNECTED = 0x00,
    TIMEOUT = 0x10,
    PROTOCOL = 0x20,
    FSIO = 0x40,
};

enum class EventType : uint32_t {
    DATA_CALLBACK = 0,
    HEADER_CALLBACK,
    TASK_STATE_CALLBACK,
    PROGRESS_CALLBACK,
    BUTT,
};

enum NetworkType {
    NETWORK_INVALID = 0x00000000,
    NETWORK_MOBILE = 0x00000001,
    NETWORK_WIFI = 0x00010000,
    NETWORK_MASK = 0x00010001,
};

enum DownloadErrorCode {
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

enum ExceptionErrorCode : int32_t {
    E_OK = 0,
    E_UNLOADING_SA,
    E_IPC_SIZE_TOO_LARGE,
    E_MIMETYPE_NOT_FOUND,
    E_TASK_INDEX_TOO_LARGE,
    E_PERMISSION = 201,
    E_NOT_SYSTEM_APP = 202,
    E_PARAMETER_CHECK = 401,
    E_UNSUPPORTED = 801,
    E_FILE_IO = 13400001,
    E_FILE_PATH = 13400002,
    E_SERVICE_ERROR = 13400003,
    E_OTHER = 13499999,
    E_TASK_QUEUE = 21900004,
    E_TASK_MODE = 21900005,
    E_TASK_NOT_FOUND = 21900006,
    E_TASK_STATE = 21900007,
};

struct Progress {
    State state {State::ANY};
    uint32_t index {0};
    int64_t processed {0};
    int64_t lastProcessed {0};
    int64_t totalProcessed {0};
    std::vector<int64_t> sizes {};
    std::map<std::string, std::string> extras {};
    std::vector<uint8_t> bodyBytes {};
};

struct Response {
    std::string version;
    int32_t statusCode;
    std::string reason;
    std::map<std::string, std::vector<std::string>> headers;
};

struct TaskState {
    std::string path {""};
    uint32_t responseCode {REASON_OK};
    std::string message {""};
};

struct UploadResponse {
    int32_t code {0};
    std::string data {""};
    std::string headers {""};
};

struct FormItem {
    std::string name {""};
    std::string value {""};
};

struct FileSpec {
    std::string name {""};
    std::string uri {""};
    std::string filename {""};
    std::string type {""};
    int32_t fd {-1};
};

struct Config {
    Action action {Action::ANY};
    std::string url {""};
    Version version {Version::UNKNOWN};
    Mode mode {Mode::ANY};
    Network network {Network::ANY};
    uint32_t index {0};
    int64_t begins {0};
    int64_t ends {-1};
    uint32_t priority {0};
    bool overwrite {false};
    bool metered {false};
    bool roaming {false};
    bool retry {true};
    bool redirect {true};
    bool gauge {false};
    bool precise {false};
    bool background {false};
    std::string title {""};
    std::string saveas {""};
    std::string proxy {""};
    std::string method {""};
    std::string token {"null"};
    std::string description {""};
    std::string data {""};
    std::map<std::string, std::string> headers {};
    std::vector<FormItem> forms {};
    std::vector<FileSpec> files {};
    std::vector<int32_t> bodyFds {};
    std::vector<std::string> bodyFileNames {};
    std::map<std::string, std::string> extras {};
};

struct TaskInfo {
    Version version {Version::UNKNOWN};
    std::string url {""};
    std::string data {""};
    std::string proxy {""};
    std::vector<FileSpec> files {};
    std::vector<FormItem> forms {};
    std::string tid {""};
    std::string title {""};
    std::string description {""};
    Action action {Action::ANY};
    Mode mode {Mode::ANY};
    std::string mimeType {""};
    Progress progress {};
    Response response {};
    bool gauge {false};
    uint64_t ctime {0};
    uint64_t mtime {0};
    bool retry {false};
    uint32_t tries {0};
    Faults faults {Faults::OTHERS};
    Reason code {Reason::REASON_OK};
    std::string reason {""};
    bool withSystem {false};
    uint32_t priority {0};
    std::map<std::string, std::string> extras {};
    std::vector<TaskState> taskStates {};
};

struct Filter {
    int64_t before {0};
    int64_t after {0};
    State state {State::ANY};
    Action action {Action::ANY};
    Mode mode {Mode::ANY};
};

struct ExceptionError {
    ExceptionErrorCode code {ExceptionErrorCode::E_OK};
    std::string errInfo {""};
};

struct DownloadInfo {
    uint32_t downloadId {0};
    DownloadErrorCode failedReason {DownloadErrorCode::ERROR_CANNOT_RESUME};
    std::string fileName {""};
    std::string filePath {""};
    PausedReason pausedReason {PausedReason::PAUSED_QUEUED_FOR_WIFI};
    DownloadStatus status {DownloadStatus::SESSION_UNKNOWN};
    std::string url {""};
    std::string downloadTitle {""};
    int64_t downloadTotalBytes {0};
    std::string description {""};
    int64_t downloadedBytes {0};
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

static constexpr const char *E_OK_INFO = "check succeeded";
static constexpr const char *E_PERMISSION_INFO = "the permissions check fails";
static constexpr const char *E_PARAMETER_CHECK_INFO = "the parameters check fails";
static constexpr const char *E_UNSUPPORTED_INFO = "call unsupported api";
static constexpr const char *E_FILE_IO_INFO = " file operation error";
static constexpr const char *E_FILE_PATH_INFO = "bad file path";
static constexpr const char *E_SERVICE_ERROR_INFO = "task service ability error";
static constexpr const char *E_OTHER_INFO = "other error";
static constexpr const char *E_TASK_QUEUE_INFO = "application task queue full error";
static constexpr const char *E_TASK_MODE_INFO = "task mode error";
static constexpr const char *E_TASK_NOT_FOUND_INFO = "task not found error";
static constexpr const char *E_TASK_STATE_INFO = "task state error";

static constexpr const char *FUNCTION_ON = "on";
static constexpr const char *FUNCTION_OFF = "off";
static constexpr const char *FUNCTION_START = "start";
static constexpr const char *FUNCTION_STOP = "stop";
static constexpr const char *FUNCTION_PAUSE = "pause";
static constexpr const char *FUNCTION_RESUME = "resume";
static constexpr const char *FUNCTION_QUERY = "query";
static constexpr const char *FUNCTION_SUSPEND = "suspend";
static constexpr const char *FUNCTION_GET_TASK_INFO = "getTaskInfo";
static constexpr const char *FUNCTION_GET_TASK_MIME_TYPE = "getTaskMimeType";
static constexpr const char *FUNCTION_DELETE = "delete";
static constexpr const char *FUNCTION_RESTORE = "restore";

static constexpr const char *PARAM_KEY_METHOD = "method";
static constexpr const char *PARAM_KEY_FILES = "files";
static constexpr const char *PARAM_KEY_DATA = "data";

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

static constexpr int64_t INVALID_TASK_ID = -1;
static constexpr int32_t REPORT_INFO_INTERVAL = 1000;

static constexpr const char *EVENT_COMPLETED = "completed";
static constexpr const char *EVENT_FAILED = "failed";
static constexpr const char *EVENT_PAUSE = "pause";
static constexpr const char *EVENT_RESUME = "resume";
static constexpr const char *EVENT_REMOVE = "remove";
static constexpr const char *EVENT_HEADERRECEIVE = "headerReceive";
static constexpr const char *EVENT_RESPONSE = "response";
static constexpr const char *EVENT_PROGRESS = "progress";
static constexpr const char *EVENT_COMPLETE = "complete";
static constexpr const char *EVENT_FAIL = "fail";

static constexpr const char *METHOD_GET = "GET";
static constexpr const char *METHOD_PUT = "PUT";
static constexpr const char *METHOD_POST = "POST";

static const std::string SANDBOX_CACHE_PATH = "internal://cache";
} // namespace OHOS::Plugin::Request

#endif // PLUGINS_REQUEST_CONSTANT_H
