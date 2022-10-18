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

#include "download_task_napi.h"

#include <fcntl.h>
#include <mutex>
#include <uv.h>
#include <unistd.h>

#include "async_call.h"
#include "download_event.h"
#include "download_manager.h"
#include "download_remove.h"
#include "log.h"
#include "napi_utils.h"

static constexpr const char *FUNCTION_ON = "on";
static constexpr const char *FUNCTION_OFF = "off";
static constexpr const char *FUNCTION_REMOVE = "remove";

static constexpr const char *PARAM_KEY_URI = "url";
static constexpr const char *PARAM_KEY_HEADER = "header";
static constexpr const char *PARAM_KEY_METERED = "enableMetered";
static constexpr const char *PARAM_KEY_ROAMING = "enableRoaming";
static constexpr const char *PARAM_KEY_DESCRIPTION = "description";
static constexpr const char *PARAM_KEY_NETWORKTYPE = "networkType";
static constexpr const char *PARAM_KEY_FILE_PATH = "filePath";
static constexpr const char *PARAM_KEY_TITLE = "title";
static constexpr const char *PARAM_KEY_BACKGROUND = "background";

static constexpr uint32_t FILE_PERMISSION = 0644;

namespace OHOS::Plugin::Request::Download {
constexpr const std::uint32_t CONFIG_PARAM_AT_FIRST = 0;
__thread napi_ref DownloadTaskNapi::globalCtor = nullptr;
std::mutex mutex_;
napi_value DownloadTaskNapi::JsMain(napi_env env, napi_callback_info info)
{
    DOWNLOAD_HILOGD("Enter download JsMain.");
    struct ContextInfo {
        napi_ref ref = nullptr;
    };
    auto ctxInfo = std::make_shared<ContextInfo>();
    auto input = [ctxInfo](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        DOWNLOAD_HILOGD("download parser to native params %{public}d!", static_cast<int>(argc));
        NAPI_ASSERT_BASE(env, (argc > 0) && (argc < 3), " need 2 parameters!", napi_invalid_arg);
        napi_value proxy = nullptr;
        napi_status status = napi_new_instance(env, GetCtor(env), argc, argv, &proxy);
        if ((proxy == nullptr) || (status != napi_ok)) {
            DOWNLOAD_HILOGE("Failed to create download task");
            return napi_generic_failure;
        }
        napi_create_reference(env, proxy, 1, &(ctxInfo->ref));
        return napi_ok;
    };
    auto output = [ctxInfo](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_reference_value(env, ctxInfo->ref, result);
        napi_delete_reference(env, ctxInfo->ref);
        return status;
    };
    auto context = std::make_shared<AsyncCall::Context>(input, output);
    AsyncCall asyncCall(env, info, context, 1);
    return asyncCall.Call(env);
}

napi_value DownloadTaskNapi::GetCtor(napi_env env)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_value cons;
    if (globalCtor != nullptr) {
        NAPI_CALL(env, napi_get_reference_value(env, globalCtor, &cons));
        return cons;
    }

    napi_property_descriptor clzDes[] = {
        {FUNCTION_ON, 0, DownloadEvent::On, 0, 0, 0, napi_default, 0},
        {FUNCTION_OFF, 0, DownloadEvent::Off, 0, 0, 0, napi_default, 0},
        {FUNCTION_REMOVE, 0, DownloadRemove::Exec, 0, 0, 0, napi_default, 0},
    };
    NAPI_CALL(env, napi_define_class(env, "DownloadTaskNapi", NAPI_AUTO_LENGTH, Initialize, nullptr,
                       sizeof(clzDes) / sizeof(napi_property_descriptor), clzDes, &cons));
    NAPI_CALL(env, napi_create_reference(env, cons, 1, &globalCtor));
    return cons;
}

napi_value DownloadTaskNapi::Initialize(napi_env env, napi_callback_info info)
{
    DOWNLOAD_HILOGD("constructor download task!");
    napi_value self = nullptr;
    int parametersPosition = CONFIG_PARAM_AT_FIRST;
    size_t argc = NapiUtils::MAX_ARGC;
    napi_value argv[NapiUtils::MAX_ARGC] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &self, nullptr));

    DownloadConfig config;
    if (!ParseConfig(env, argv[parametersPosition], config)) {
        DOWNLOAD_HILOGE("download config has wrong type");
        return nullptr;
    }
    
    auto *task = DownloadManager::GetInstance().Download(config);
    if (task == nullptr) {
        DOWNLOAD_HILOGE("download task fail");
        return nullptr;
    }
    auto finalize = [](napi_env env, void *data, void *hint) {
        DOWNLOAD_HILOGD("destructed download task start.");
        DownloadTask *task = reinterpret_cast<DownloadTask *>(data);
        if (task) {
            auto ret = DownloadManager::GetInstance().Remove(task->GetId());
            DOWNLOAD_HILOGD("destructed download task, ret: %d", ret);
            delete task;
        }
        DOWNLOAD_HILOGD("destructed download task end.");
    };
    if (napi_wrap(env, self, task, finalize, nullptr, nullptr) != napi_ok) {
        finalize(env, task, nullptr);
        return nullptr;
    }
    DOWNLOAD_HILOGD("Succeed to allocate download task");
    return self;
}

bool DownloadTaskNapi::ParseConfig(napi_env env, napi_value configValue, DownloadConfig &config)
{
    if (!ParseHeader(env, configValue, config)) {
        return false;
    }
    config.SetUrl(NapiUtils::GetStringPropertyUtf8(env, configValue, PARAM_KEY_URI));
    config.SetMetered(NapiUtils::GetBooleanProperty(env, configValue, PARAM_KEY_METERED));
    config.SetRoaming(NapiUtils::GetBooleanProperty(env, configValue, PARAM_KEY_ROAMING));
    config.SetDescription(NapiUtils::GetStringPropertyUtf8(env, configValue, PARAM_KEY_DESCRIPTION));
    config.SetFilePath(NapiUtils::GetStringPropertyUtf8(env, configValue, PARAM_KEY_FILE_PATH));
    config.SetTitle(NapiUtils::GetStringPropertyUtf8(env, configValue, PARAM_KEY_TITLE));
    config.SetBackground(NapiUtils::GetBooleanProperty(env, configValue, PARAM_KEY_BACKGROUND));

    if (!IsPathValid(config.GetFilePath())) {
        return false;
    }

    int32_t fd = -1;
    int32_t err = 0;
    fd = open(config.GetFilePath().c_str(), O_RDWR);
    if (fd > 0) {
        DOWNLOAD_HILOGD("Download File already exists");
        close(fd);
        fd = -1;
    } else {
        fd = open(config.GetFilePath().c_str(), O_CREAT | O_RDWR, FILE_PERMISSION);
        if (fd < 0) {
            DOWNLOAD_HILOGE("Failed to open file errno [%{public}d]", errno);
        }
        err = errno;
    }
    config.SetFD(fd);
    config.SetFDError(err);
    config.SetApplicationInfoUid(static_cast<int32_t>(getuid()));

    return true;
}

bool DownloadTaskNapi::IsPathValid(const std::string &filePath)
{
    auto path = filePath.substr(0, filePath.rfind('/'));
    char resolvedPath[PATH_MAX + 1] = { 0 };
    if (path.length() > PATH_MAX || realpath(path.c_str(), resolvedPath) == nullptr
        || strncmp(resolvedPath, path.c_str(), path.length()) != 0) {
        DOWNLOAD_HILOGE("invalid file path!");
        return false;
    }
    return true;
}

bool DownloadTaskNapi::ParseHeader(napi_env env, napi_value configValue, DownloadConfig &config)
{
    if (!NapiUtils::HasNamedProperty(env, configValue, PARAM_KEY_HEADER)) {
        DOWNLOAD_HILOGD("No header present, ignore it");
        return true;
    }
    napi_value header = NapiUtils::GetNamedProperty(env, configValue, PARAM_KEY_HEADER);
    if (NapiUtils::GetValueType(env, header) != napi_object) {
        return false;
    }
    auto names = NapiUtils::GetPropertyNames(env, header);
    std::vector<std::string>::iterator iter;
    DOWNLOAD_HILOGD("current name list size = %{public}zu", names.size());
    for (iter = names.begin(); iter != names.end(); ++iter) {
        auto value = NapiUtils::GetStringPropertyUtf8(env, header, *iter);
        if (!value.empty()) {
            config.SetHeader(NapiUtils::ToLower(*iter), value);
        }
    }
    return true;
}
} // namespace OHOS::Plugin::Request::Download
