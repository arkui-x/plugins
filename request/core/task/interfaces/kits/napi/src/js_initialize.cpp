/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "js_initialize.h"

#include <cstring>
#include <regex>
#include <securec.h>
#include <fcntl.h>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

#include "constant.h"
#include "log.h"
#include "napi_utils.h"
#include "task_manager.h"

static constexpr const char *PARAM_KEY_DESCRIPTION = "description";
static constexpr const char *PARAM_KEY_NETWORKTYPE = "networkType";
static constexpr const char *PARAM_KEY_FILE_PATH = "filePath";
static constexpr const char *PARAM_KEY_BACKGROUND = "background";
constexpr const std::uint32_t CONFIG_PARAM_POS = 1;
static constexpr uint32_t FILE_PERMISSION = 0646;
static constexpr uint32_t TITLE_MAXIMUM = 256;
static constexpr uint32_t PROXY_MAXIMUM = 512;
static constexpr uint32_t DESCRIPTION_MAXIMUM = 1024;
static constexpr uint32_t URL_MAXIMUM = 2048;

namespace OHOS::Plugin::Request {
napi_value JsInitialize::Initialize(napi_env env, napi_callback_info info, Version version)
{
    REQUEST_HILOGI("constructor request task!");
    napi_value self = nullptr;
    size_t argc = NapiUtils::MAX_ARGC;
    napi_value argv[NapiUtils::MAX_ARGC] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &self, nullptr));
    int32_t number = NapiUtils::TWO_ARG;
    if (static_cast<int32_t>(argc) < number) {
        NapiUtils::ThrowError(env, E_PARAMETER_CHECK, "invalid parameter count");
        return nullptr;
    }

    Config config;
    config.version = version;
    ExceptionError err = InitParam(env, argv, config);
    if (err.code != E_OK) {
        NapiUtils::ThrowError(env, err.code, err.errInfo);
        return nullptr;
    }
    auto *task = TaskManager::Get().Create(config);
    if (task == nullptr) {
        REQUEST_HILOGE("Create task object failed");
        return nullptr;
    }
    auto finalize = [](napi_env env, void *data, void *hint) {
        REQUEST_HILOGI("destructed task");
        auto task = reinterpret_cast<ITask *>(data);
        delete task;
    };
    if (napi_wrap(env, self, task, finalize, nullptr, nullptr) != napi_ok) {
        finalize(env, task, nullptr);
        return nullptr;
    }
    return self;
}

ExceptionError JsInitialize::InitParam(napi_env env, napi_value* argv, Config &config)
{
    REQUEST_HILOGI("InitParam in");
    ExceptionError err = { .code = E_OK };

    if (!ParseConfig(env, argv[CONFIG_PARAM_POS], config, err.errInfo)) {
        err.code = E_PARAMETER_CHECK;
        return err;
    }

    REQUEST_HILOGI("config action: %{public}d", static_cast<int32_t>(config.action));
    REQUEST_HILOGI("config url: %{public}s", config.url.c_str());
    REQUEST_HILOGI("config version: %{public}d", static_cast<int32_t>(config.version));
    REQUEST_HILOGI("config mode: %{public}d", static_cast<int32_t>(config.mode));
    REQUEST_HILOGI("config network: %{public}d", static_cast<int32_t>(config.network));
    REQUEST_HILOGI("config index: %{public}d", static_cast<int32_t>(config.index));
    REQUEST_HILOGI("config begins: %{public}lld", config.begins);
    REQUEST_HILOGI("config ends: %{public}lld", config.ends);
    REQUEST_HILOGI("config overwrite: %{public}d", static_cast<bool>(config.overwrite));
    REQUEST_HILOGI("config title: %{public}s", config.title.c_str());
    REQUEST_HILOGI("config saveas: %{public}s", config.saveas.c_str());
    REQUEST_HILOGI("config proxy: %{public}s", config.proxy.c_str());
    REQUEST_HILOGI("config method: %{public}s", config.method.c_str());
    REQUEST_HILOGI("config token: %{public}s", config.token.c_str());
    REQUEST_HILOGI("config description: %{public}s", config.description.c_str());
    REQUEST_HILOGI("config data: %{public}s", config.data.c_str());

    CheckPartialDownload(config);

    return CheckFilePath(config);
}

void JsInitialize::CheckPartialDownload(Config &config)
{
    if (config.action == Action::DOWNLOAD) {
        if (config.begins > 0 || config.ends > 0) {
            std::string rangeValue = "bytes=" + std::to_string(config.begins) + "-";
            if (config.ends > 0) {
                rangeValue += std::to_string(config.ends);
            }
            config.headers.emplace("Range", rangeValue);
        }
    }
}

void JsInitialize::CheckFileUri(const Config &config, FileSpec &file)
{
    if (config.action == Action::UPLOAD) {
        REQUEST_HILOGI("CheckFileUri upload file.uri:%{public}s", file.uri.c_str());
        if (file.uri.length() > SANDBOX_CACHE_PATH.length()) {
            size_t pos = SANDBOX_CACHE_PATH.length();
            std::string prefix = file.uri.substr(0, pos);
            if (prefix == SANDBOX_CACHE_PATH) {
                std::string cacheDir;
                TaskManager::Get().GetDefaultStoragePath(cacheDir);
                file.uri = cacheDir + file.uri.substr(pos);
                REQUEST_HILOGI("CheckFileUri result file.uri:%{public}s", file.uri.c_str());
            }
        }
    }
}

ExceptionError JsInitialize::CheckFilePath(Config &config)
{
    ExceptionError err = { .code = E_OK };
    if (config.action == Action::DOWNLOAD) {
        FileSpec file = { .uri = config.saveas };
        config.files.emplace_back(file);
    }
    for (auto &file : config.files) {
        CheckFileUri(config, file);
        if (file.filename.empty()) {
            InterceptData("/", file.uri, file.filename);
        }
        if (file.type.empty()) {
            InterceptData(".", file.filename, file.type);
        }
        if (file.name.empty()) {
            file.name = "file";
        }
        err = GetFD(file.uri, config, file.fd);
        if (err.code != E_OK) {
            return err;
        }
    }
    return err;
}

ExceptionError JsInitialize::CheckUploadBodyFiles(Config &config, const std::string &filePath)
{
    ExceptionError error = { .code = E_OK };
    size_t len = config.files.size();

    for (size_t i = 0; i < len; i++) {
        auto now = std::chrono::high_resolution_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
        std::string fileName = filePath + "/tmp_body_" + std::to_string(i) + "_" + std::to_string(timestamp);
        REQUEST_HILOGI("Create upload body file, %{public}s", fileName.c_str());
        if (!NapiUtils::IsPathValid(fileName)) {
            REQUEST_HILOGE("IsPathValid error %{public}s", fileName.c_str());
            return { .code = E_PARAMETER_CHECK, .errInfo = "IsPathValid error fail path" };
        }

        int32_t bodyFd = open(fileName.c_str(), O_TRUNC | O_RDWR);
        if (bodyFd < 0) {
            bodyFd = open(fileName.c_str(), O_CREAT | O_RDWR, FILE_PERMISSION);
            if (bodyFd < 0) {
                return { .code = E_FILE_IO, .errInfo = "Failed to open file errno " + std::to_string(errno) };
            }
        }

        config.bodyFds.emplace_back(bodyFd);
        config.bodyFileNames.emplace_back(fileName);
    }
    return error;
}

ExceptionError JsInitialize::GetFD(const std::string &path, const Config &config, int32_t &fd)
{
    REQUEST_HILOGI("GetFD path: %{public}s", path.c_str());
    ExceptionError error = { .code = E_OK };
    fd = config.action == Action::UPLOAD ? open(path.c_str(), O_RDONLY) : open(path.c_str(), O_TRUNC | O_RDWR);
    if (fd >= 0) {
        REQUEST_HILOGI("File already exists");
        if (config.action == Action::UPLOAD) {
            chmod(path.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            return error;
        } else {
            chmod(path.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IWOTH);
        }

        if (config.version == Version::API10 && config.overwrite) {
            return error;
        }
        ExceptionErrorCode code = config.version == Version::API10 ? E_FILE_IO : E_FILE_PATH;
        return { .code = code, .errInfo = "Download File already exists" };
    } else {
        if (config.action == Action::UPLOAD) {
            ExceptionErrorCode code = config.version == Version::API10 ? E_FILE_IO : E_FILE_PATH;
            REQUEST_HILOGI("Failed to open file errno path: %{public}s", path.c_str());
            return { .code = code, .errInfo = "Failed to open file errno " + std::to_string(errno) };
        }
        fd = open(path.c_str(), O_CREAT | O_RDWR, FILE_PERMISSION);
        if (fd < 0) {
            REQUEST_HILOGI("GetFD download failed path: %{public}s", path.c_str());
            return { .code = E_FILE_IO, .errInfo = "Failed to open file errno " + std::to_string(errno) };
        }
        chmod(path.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IWOTH);
    }
    return error;
}

bool JsInitialize::ParseConfig(napi_env env, napi_value jsConfig, Config &config, std::string &errInfo)
{
    if (NapiUtils::GetValueType(env, jsConfig) != napi_object) {
        errInfo = "Wrong conf type, expected object";
        return false;
    }
    if (config.version != Version::API10) {
        return ParseConfigV9(env, jsConfig, config, errInfo);
    }

    if (!ParseAction(env, jsConfig, config.action)) {
        errInfo = "parse action error";
        return false;
    }
    if (!ParseProxy(env, jsConfig, config.proxy, errInfo)) {
        errInfo = "parse proxy error";
        return false;
    }
    if (!ParseUrl(env, jsConfig, config.url)) {
        errInfo = "parse url error";
        return false;
    }
    if (!ParseData(env, jsConfig, config)) {
        errInfo = "parse data error";
        return false;
    }
    if (!ParseIndex(env, jsConfig, config)) {
        errInfo = "Index exceeds file list";
        return false;
    }
    if (!ParseTitle(env, jsConfig, config) || !ParseToken(env, jsConfig, config) ||
        !ParseDescription(env, jsConfig, config.description)) {
        errInfo = "Exceeding maximum length";
        return false;
    }
    ParseMethod(env, jsConfig, config);
    ParseSaveas(env, jsConfig, config);
    ParseRoaming(env, jsConfig, config);
    ParseRedirect(env, jsConfig, config.redirect);
    ParseNetwork(env, jsConfig, config.network);
    ParseRetry(env, jsConfig, config.retry);

    config.overwrite = NapiUtils::Convert2Boolean(env, jsConfig, "overwrite");
    config.metered = NapiUtils::Convert2Boolean(env, jsConfig, "metered");
    config.gauge = NapiUtils::Convert2Boolean(env, jsConfig, "gauge");
    config.precise = NapiUtils::Convert2Boolean(env, jsConfig, "precise");
    config.priority = ParsePriority(env, jsConfig);
    config.begins = ParseBegins(env, jsConfig);
    config.ends = ParseEnds(env, jsConfig);
    config.mode = ParseMode(env, jsConfig);
    if (config.mode != Mode::FOREGROUND) {
        REQUEST_HILOGE("only support foreground task");
        return false;
    }
    REQUEST_HILOGI("config.overwrite: %{public}d", config.overwrite);
    config.headers = ParseMap(env, jsConfig, "headers");
    config.extras = ParseMap(env, jsConfig, "extras");
    return true;
}

void JsInitialize::ParseRoaming(napi_env env, napi_value jsConfig, Config &config)
{
    if (!NapiUtils::HasNamedProperty(env, jsConfig, "roaming")) {
        config.roaming = config.version == Version::API10;
    } else {
        config.roaming = NapiUtils::Convert2Boolean(env, jsConfig, "roaming");
    }
}

void JsInitialize::ParseNetwork(napi_env env, napi_value jsConfig, Network &network)
{
    network = static_cast<Network>(NapiUtils::Convert2Uint32(env, jsConfig, "network"));
    if (network != Network::ANY && network != Network::WIFI && network != Network::CELLULAR) {
        network = Network::ANY;
    }
}

bool JsInitialize::ParseToken(napi_env env, napi_value jsConfig, Config &config)
{
    if (!NapiUtils::HasNamedProperty(env, jsConfig, "token")) {
        return true;
    }
    napi_value value = NapiUtils::GetNamedProperty(env, jsConfig, "token");
    if (NapiUtils::GetValueType(env, value) != napi_string) {
        return true;
    }
    std::vector<char> token;
    token.resize(TOKEN_MAX_BYTES + 2);
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, value, token.data(), token.size(), &len);
    if (status != napi_ok || len < TOKEN_MIN_BYTES || len > TOKEN_MAX_BYTES) {
        return false;
    }
    config.token = token.data();
    return true;
}

bool JsInitialize::ParseIndex(napi_env env, napi_value jsConfig, Config &config)
{
    config.index = NapiUtils::Convert2Uint32(env, jsConfig, "index");
    if (config.action == Action::DOWNLOAD) {
        config.index = 0;
        return true;
    }
    if (config.files.size() <= config.index) {
        REQUEST_HILOGE("files.size is %{public}zu, index is %{public}d", config.files.size(), config.index);
        return false;
    }
    return true;
}

bool JsInitialize::ParseAction(napi_env env, napi_value jsConfig, Action &action)
{
    if (!NapiUtils::HasNamedProperty(env, jsConfig, "action")) {
        REQUEST_HILOGE("ParseAction err");
        return false;
    }
    napi_value value = NapiUtils::GetNamedProperty(env, jsConfig, "action");
    if (NapiUtils::GetValueType(env, value) != napi_number) {
        REQUEST_HILOGE("GetNamedProperty err");
        return false;
    }
    action = static_cast<Action>(NapiUtils::Convert2Uint32(env, value));
    if (action != Action::DOWNLOAD && action != Action::UPLOAD) {
        REQUEST_HILOGE("Must be UPLOAD or DOWNLOAD");
        return false;
    }
    return true;
}

void JsInitialize::ParseSaveas(napi_env env, napi_value jsConfig, Config &config)
{
    if (config.action == Action::UPLOAD) {
        REQUEST_HILOGI("action is upload");
        return;
    }
    config.saveas = NapiUtils::Convert2String(env, jsConfig, "saveas");
    std::string defaultStoragePath;
    TaskManager::Get().GetDefaultStoragePath(defaultStoragePath);
    if (config.saveas.empty() || config.saveas == "./" || config.saveas == ".") {
        std::string fileName;
        InterceptData("/", config.url, fileName);
        config.saveas = defaultStoragePath + "/" + fileName;
    }
    else {
        if ((config.saveas.length() > 2) && (config.saveas[0] == '.') && (config.saveas[1] == '/')) {
            config.saveas = defaultStoragePath + std::string(config.saveas, 1);
        }
        else if (config.saveas[0] != '/') {
        config.saveas = defaultStoragePath + "/" + config.saveas;
        }
    }

    REQUEST_HILOGI("ParseSaveas: %{public}s", config.saveas.c_str());
}


bool JsInitialize::ParseProxy(napi_env env, napi_value jsConfig, std::string &proxy, std::string &errInfo)
{
    proxy = NapiUtils::Convert2String(env, jsConfig, "proxy");
    if (proxy.empty()) {
        return true;
    }

    if (proxy.size() > PROXY_MAXIMUM) {
        REQUEST_HILOGE("The proxy exceeds the maximum length of 512");
        errInfo = "Parameter verification failed, the length of config.proxy exceeds 512";
        return false;
    }

    if (!regex_match(proxy, std::regex("^http:\\/\\/.+:\\d{1,5}$"))) {
        REQUEST_HILOGE("ParseProxy error");
        errInfo = "Parameter verification failed, the format of proxy is http(s)://<address or domain>:port";
        return false;
    }
    return true;
}

int64_t JsInitialize::ParseBegins(napi_env env, napi_value jsConfig)
{
    int64_t size = NapiUtils::Convert2Int64(env, jsConfig, "begins");
    return size >= 0 ? size : 0;
}

int64_t JsInitialize::ParseEnds(napi_env env, napi_value jsConfig)
{
    if (!NapiUtils::HasNamedProperty(env, jsConfig, "ends")) {
        return -1;
    }
    napi_value endsValue = NapiUtils::GetNamedProperty(env, jsConfig, "ends");
    napi_valuetype valueType;
    napi_status status = napi_typeof(env, endsValue, &valueType);
    if (status != napi_ok || valueType == napi_undefined) {
        return -1;
    }

    int64_t size = NapiUtils::Convert2Int64(env, jsConfig, "ends");
    return size < -1 ? -1 : size;
}

uint32_t JsInitialize::ParsePriority(napi_env env, napi_value jsConfig)
{
    if (!NapiUtils::HasNamedProperty(env, jsConfig, "priority")) {
        return 0;
    }
    return NapiUtils::Convert2Uint32(env, jsConfig, "priority");
}

Mode JsInitialize::ParseMode(napi_env env, napi_value jsConfig)
{
    if (!NapiUtils::HasNamedProperty(env, jsConfig, "mode")) {
        return Mode::FOREGROUND;
    }
    return static_cast<Mode>(NapiUtils::Convert2Uint32(env, jsConfig, "mode"));
}

bool JsInitialize::ParseDescription(napi_env env, napi_value jsConfig, std::string &description)
{
    description = NapiUtils::Convert2String(env, jsConfig, "description");
    if (description.size() > DESCRIPTION_MAXIMUM) {
        return false;
    }
    return true;
}

std::map<std::string, std::string> JsInitialize::ParseMap(napi_env env, napi_value jsConfig,
    const std::string &propertyName)
{
    std::map<std::string, std::string> result;
    napi_value jsValue = NapiUtils::GetNamedProperty(env, jsConfig, propertyName);
    if (jsValue == nullptr) {
        return result;
    }
    auto names = NapiUtils::GetPropertyNames(env, jsValue);
    for (auto iter = names.begin(); iter != names.end(); ++iter) {
        result[*iter] = NapiUtils::Convert2String(env, jsValue, *iter);
    }
    return result;
}

bool JsInitialize::ParseUrl(napi_env env, napi_value jsConfig, std::string &url)
{
    url = NapiUtils::Convert2String(env, jsConfig, "url");
    if (url.size() > URL_MAXIMUM) {
        REQUEST_HILOGE("The URL exceeds the maximum length of 2048");
        return false;
    }
    if (!regex_match(url, std::regex("^http(s)?:\\/\\/.+"))) {
        REQUEST_HILOGE("ParseUrl error");
        return false;
    }

    return true;
}

bool JsInitialize::ParseTitle(napi_env env, napi_value jsConfig, Config &config)
{
    config.title = NapiUtils::Convert2String(env, jsConfig, "title");
    if (config.version == Version::API10 && config.title.size() > TITLE_MAXIMUM) {
        return false;
    }
    if (config.title.empty()) {
        config.title = config.action == Action::UPLOAD ? "upload" : "download";
    }
    return true;
}

void JsInitialize::ParseMethod(napi_env env, napi_value jsConfig, Config &config)
{
    if (config.version == Version::API10) {
        config.method = config.action == Action::UPLOAD ? "PUT" : "GET";
    } else {
        config.method = "POST";
    }
    std::string method = NapiUtils::Convert2String(env, jsConfig, "method");
    if (!method.empty()) {
        transform(method.begin(), method.end(), method.begin(), ::toupper);
        if (config.action == Action::UPLOAD && (method == "POST" || method == "PUT")) {
            config.method = method;
        }
        if (config.action == Action::DOWNLOAD && (method == "POST" || method == "GET")) {
            config.method = method;
        }
    }
}

bool JsInitialize::ParseData(napi_env env, napi_value jsConfig, Config &config)
{
    napi_value value = NapiUtils::GetNamedProperty(env, jsConfig, "data");
    if (value == nullptr) {
        return true;
    }

    napi_valuetype valueType = NapiUtils::GetValueType(env, value);
    if (config.action == Action::UPLOAD && valueType == napi_object) {
        return Convert2FormItems(env, value, config.forms, config.files);
    } else if (config.action == Action::DOWNLOAD && valueType == napi_string) {
        config.data = NapiUtils::Convert2String(env, value);
    } else {
        REQUEST_HILOGE("data type is error");
        return false;
    }
    return true;
}

bool JsInitialize::ParseName(napi_env env, napi_value jsVal, std::string &name)
{
    napi_value value = NapiUtils::GetNamedProperty(env, jsVal, "name");
    if (NapiUtils::GetValueType(env, value) != napi_string) {
        return false;
    }
    name = NapiUtils::Convert2String(env, value);
    return true;
}

bool JsInitialize::GetFormItems(napi_env env, napi_value jsVal, std::vector<FormItem> &forms,
    std::vector<FileSpec> &files)
{
    if (!NapiUtils::HasNamedProperty(env, jsVal, "name") || !NapiUtils::HasNamedProperty(env, jsVal, "value")) {
        return false;
    }

    std::string name;
    if (!ParseName(env, jsVal, name)) {
        return false;
    }
    napi_value value = NapiUtils::GetNamedProperty(env, jsVal, "value");
    if (value == nullptr) {
        REQUEST_HILOGE("Get upload value failed");
        return false;
    }
    bool isArray = false;
    napi_is_array(env, value, &isArray);
    napi_valuetype valueType = NapiUtils::GetValueType(env, value);
    if (valueType == napi_string) {
        FormItem form;
        form.name = name;
        form.value = NapiUtils::Convert2String(env, value);
        forms.emplace_back(form);
    } else if (valueType == napi_object && !isArray) {
        FileSpec file;
        if (!Convert2FileSpec(env, value, name, file)) {
            REQUEST_HILOGE("Convert2FileSpec failed");
            return false;
        }
        files.emplace_back(file);
    } else if (isArray) {
        if (!Convert2FileSpecs(env, value, name, files)) {
            return false;
        }
    } else {
        REQUEST_HILOGE("value type is error");
        return false;
    }
    return true;
}

bool JsInitialize::Convert2FormItems(napi_env env, napi_value jsValue, std::vector<FormItem> &forms,
    std::vector<FileSpec> &files)
{
    bool isArray = false;
    napi_is_array(env, jsValue, &isArray);
    NAPI_ASSERT_BASE(env, isArray, "not array", false);
    uint32_t length = 0;
    napi_get_array_length(env, jsValue, &length);
    for (uint32_t i = 0; i < length; ++i) {
        napi_value jsVal = nullptr;
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env, &scope);
        napi_get_element(env, jsValue, i, &jsVal);
        if (jsVal == nullptr) {
            REQUEST_HILOGE("Get element jsVal failed");
            return false;
        }
        if (!GetFormItems(env, jsVal, forms, files)) {
            REQUEST_HILOGE("Get formItems failed");
            return false;
        }
        napi_close_handle_scope(env, scope);
    }
    if (files.empty()) {
        return false;
    }
    return true;
}

bool JsInitialize::Convert2FileSpecs(napi_env env, napi_value jsValue, const std::string &name,
    std::vector<FileSpec> &files)
{
    REQUEST_HILOGI("Convert2FileSpecs in");
    uint32_t length = 0;
    napi_get_array_length(env, jsValue, &length);
    for (uint32_t i = 0; i < length; ++i) {
        napi_value jsVal = nullptr;
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env, &scope);
        napi_get_element(env, jsValue, i, &jsVal);
        if (jsVal == nullptr) {
            return false;
        }
        FileSpec file;
        bool ret = Convert2FileSpec(env, jsVal, name, file);
        if (!ret) {
            return false;
        }
        files.emplace_back(file);
        napi_close_handle_scope(env, scope);
    }
    return true;
}

void JsInitialize::InterceptData(const std::string &str, const std::string &in, std::string &out)
{
    std::size_t position = in.find_last_of(str);
    if (position == std::string::npos) {
        return;
    }
    out = std::string(in, position + 1);
    out.erase(out.find_last_not_of(' ') + 1);
}

bool JsInitialize::Convert2FileSpec(napi_env env, napi_value jsValue, const std::string &name, FileSpec &file)
{
    REQUEST_HILOGI("Convert2FileSpec in");

    file.name = name;
    file.filename = NapiUtils::Convert2String(env, jsValue, "filename");
    file.type = NapiUtils::Convert2String(env, jsValue, "mimetype");

    std::string defaultStoragePath;
    TaskManager::Get().GetDefaultStoragePath(defaultStoragePath);
    file.uri = NapiUtils::Convert2String(env, jsValue, "path");
    if (file.uri.empty() || file.uri == "./" || file.uri == ".") {
        file.uri = defaultStoragePath + "/" + file.filename;
    }
    else {
        if ((file.uri.length() > 2) && (file.uri[0] == '.') && (file.uri[1] == '/')) {
            file.uri = defaultStoragePath + std::string(file.uri, 1);
        }
        else if (file.uri[0] != '/') {
        file.uri = defaultStoragePath + "/" + file.uri;
        }
    }

    REQUEST_HILOGI("Convert2FileSpec file name: %{public}s fileName: %{public}s uri: %{public}s", 
    file.name.c_str(), file.filename.c_str(), file.uri.c_str());
    return true;
}

void JsInitialize::ParseRedirect(napi_env env, napi_value jsConfig, bool &redirect)
{
    if (!NapiUtils::HasNamedProperty(env, jsConfig, "redirect")) {
        redirect = true;
    } else {
        redirect = NapiUtils::Convert2Boolean(env, jsConfig, "redirect");
    }
}

void JsInitialize::ParseRetry(napi_env env, napi_value jsConfig, bool &retry)
{
    if (!NapiUtils::HasNamedProperty(env, jsConfig, "retry")) {
        retry = true;
    } else {
        retry = NapiUtils::Convert2Boolean(env, jsConfig, "retry");
    }
}

bool JsInitialize::ParseConfigV9(napi_env env, napi_value jsConfig, Config &config, std::string &errInfo)
{
    REQUEST_HILOGI("ParseConfigV9 in");
    config.action = NapiUtils::GetRequestAction(env, jsConfig);
    config.headers = ParseMap(env, jsConfig, "header");
    if (!ParseUrl(env, jsConfig, config.url)) {
        errInfo = "Parse url error";
        return false;
    }
    auto func = config.action == Action::UPLOAD ? ParseUploadConfig : ParseDownloadConfig;
    if (!func(env, jsConfig, config, errInfo)) {
        return false;
    }
    ParseTitle(env, jsConfig, config);
    return true;
}

bool JsInitialize::ParseUploadConfig(napi_env env, napi_value jsConfig, Config &config, std::string &errInfo)
{
    REQUEST_HILOGI("ParseUploadConfig in");
    ParseMethod(env, jsConfig, config);
    napi_value jsFiles = NapiUtils::GetNamedProperty(env, jsConfig, PARAM_KEY_FILES);
    if (jsFiles == nullptr) {
        errInfo = "Parse config files error";
        return false;
    }

    config.files = NapiUtils::Convert2FileVector(env, jsFiles);
    if (config.files.empty()) {
        errInfo = "Parse config files error";
        return false;
    }

    napi_value jsData = NapiUtils::GetNamedProperty(env, jsConfig, PARAM_KEY_DATA);
    if (jsData == nullptr) {
        errInfo = "Parse config data error";
        return false;
    }
    config.forms = NapiUtils::Convert2RequestDataVector(env, jsData);

    if (!ParseIndex(env, jsConfig, config)) {
        errInfo = "Index exceeds file list";
        return false;
    }

    config.begins = ParseBegins(env, jsConfig);
    config.ends = ParseEnds(env, jsConfig);
    return true;
}

bool JsInitialize::ParseDownloadConfig(napi_env env, napi_value jsConfig, Config &config, std::string &errInfo)
{
    REQUEST_HILOGI("ParseDownloadConfig in");
    config.metered = NapiUtils::Convert2Boolean(env, jsConfig, "enableMetered");
    config.roaming = NapiUtils::Convert2Boolean(env, jsConfig, "enableRoaming");
    config.description = NapiUtils::Convert2String(env, jsConfig, PARAM_KEY_DESCRIPTION);
    uint32_t type = NapiUtils::Convert2Uint32(env, jsConfig, PARAM_KEY_NETWORKTYPE);
    if (type == NETWORK_MOBILE) {
        config.network = Network::CELLULAR;
    } else if (type == NETWORK_WIFI) {
        config.network = Network::WIFI;
    } else {
        config.network = Network::ANY;
    }
    config.saveas = NapiUtils::Convert2String(env, jsConfig, PARAM_KEY_FILE_PATH);
    if (config.saveas.empty()) {
        InterceptData("/", config.url, config.saveas);
    }
    config.background = NapiUtils::Convert2Boolean(env, jsConfig, PARAM_KEY_BACKGROUND);
    config.method = "GET";
    return true;
}

void JsInitialize::CreatProperties(napi_env env, napi_value &self, napi_value config, ITask *task)
{
    if (task->GetVersion() == Version::API10) {
        NapiUtils::SetStringPropertyUtf8(env, self, "tid", task->GetTid());
        napi_set_named_property(env, self, "config", config);
    }
}
} // namespace OHOS::Plugin::Request