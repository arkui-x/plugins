/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include <cstddef>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "constant.h"
#include "log.h"
#include "napi_task.h"
#include "napi_utils.h"
#include "plugin_utils.h"

#ifdef ANDROID_PLATFORM
#include "java/jni/task_manager_jni.h"
#endif

#define DECLARE_NAPI_METHOD(name, func) { name, 0, func, 0, 0, 0, napi_default, 0 }

static constexpr const char *BROADCAST_EVENT_COMPLETE = "ohos.request.event.COMPLETE";

namespace OHOS::Plugin::Request {
static napi_value exception_permission = nullptr;
static napi_value exception_parameter_check = nullptr;
static napi_value exception_unsupported = nullptr;
static napi_value exception_file_IO = nullptr;
static napi_value exception_file_path = nullptr;
static napi_value exception_service_error = nullptr;
static napi_value exception_other = nullptr;
static napi_value network_mobile = nullptr;
static napi_value network_wifi = nullptr;
static napi_value err_cannot_resume = nullptr;
static napi_value err_dev_not_found = nullptr;
static napi_value err_file_exist = nullptr;
static napi_value err_file_error = nullptr;
static napi_value err_http_data = nullptr;
static napi_value err_no_space = nullptr;
static napi_value err_many_redirect = nullptr;
static napi_value err_http_code = nullptr;
static napi_value err_unknown = nullptr;
static napi_value err_offline = nullptr;
static napi_value err_unsupported_network_type = nullptr;
static napi_value paused_queue_wifi = nullptr;
static napi_value paused_for_network = nullptr;
static napi_value paused_to_retry = nullptr;
static napi_value paused_by_user = nullptr;
static napi_value paused_unknown = nullptr;
static napi_value session_success = nullptr;
static napi_value session_running = nullptr;
static napi_value session_pending = nullptr;
static napi_value session_paused = nullptr;
static napi_value session_failed = nullptr;

static void NapiCreateInt32(napi_env env)
{
    /* create exception type const */
    napi_create_int32(env, static_cast<int32_t>(E_PERMISSION), &exception_permission);
    napi_create_int32(env, static_cast<int32_t>(E_PARAMETER_CHECK), &exception_parameter_check);
    napi_create_int32(env, static_cast<int32_t>(E_UNSUPPORTED), &exception_unsupported);
    napi_create_int32(env, static_cast<int32_t>(E_FILE_IO), &exception_file_IO);
    napi_create_int32(env, static_cast<int32_t>(E_FILE_PATH), &exception_file_path);
    napi_create_int32(env, static_cast<int32_t>(E_SERVICE_ERROR), &exception_service_error);
    napi_create_int32(env, static_cast<int32_t>(E_OTHER), &exception_other);

    /* Create Network Type Const */
    napi_create_int32(env, static_cast<int32_t>(NETWORK_MOBILE), &network_mobile);
    napi_create_int32(env, static_cast<int32_t>(NETWORK_WIFI), &network_wifi);

    /* Create error cause const */
    napi_create_int32(env, static_cast<int32_t>(ERROR_CANNOT_RESUME), &err_cannot_resume);
    napi_create_int32(env, static_cast<int32_t>(ERROR_DEVICE_NOT_FOUND), &err_dev_not_found);
    napi_create_int32(env, static_cast<int32_t>(ERROR_FILE_ALREADY_EXISTS), &err_file_exist);
    napi_create_int32(env, static_cast<int32_t>(ERROR_FILE_ERROR), &err_file_error);
    napi_create_int32(env, static_cast<int32_t>(ERROR_HTTP_DATA_ERROR), &err_http_data);
    napi_create_int32(env, static_cast<int32_t>(ERROR_INSUFFICIENT_SPACE), &err_no_space);
    napi_create_int32(env, static_cast<int32_t>(ERROR_TOO_MANY_REDIRECTS), &err_many_redirect);
    napi_create_int32(env, static_cast<int32_t>(ERROR_UNHANDLED_HTTP_CODE), &err_http_code);
    napi_create_int32(env, static_cast<int32_t>(ERROR_UNKNOWN), &err_unknown);
    napi_create_int32(env, static_cast<int32_t>(ERROR_OFFLINE), &err_offline);
    napi_create_int32(env, static_cast<int32_t>(ERROR_UNSUPPORTED_NETWORK_TYPE), &err_unsupported_network_type);

    /* Create paused reason Const */
    napi_create_int32(env, static_cast<int32_t>(PAUSED_QUEUED_FOR_WIFI), &paused_queue_wifi);
    napi_create_int32(env, static_cast<int32_t>(PAUSED_WAITING_FOR_NETWORK), &paused_for_network);
    napi_create_int32(env, static_cast<int32_t>(PAUSED_WAITING_TO_RETRY), &paused_to_retry);
    napi_create_int32(env, static_cast<int32_t>(PAUSED_BY_USER), &paused_by_user);
    napi_create_int32(env, static_cast<int32_t>(PAUSED_UNKNOWN), &paused_unknown);

    /* Create session status Const */
    napi_create_int32(env, static_cast<int32_t>(SESSION_SUCCESS), &session_success);
    napi_create_int32(env, static_cast<int32_t>(SESSION_RUNNING), &session_running);
    napi_create_int32(env, static_cast<int32_t>(SESSION_PENDING), &session_pending);
    napi_create_int32(env, static_cast<int32_t>(SESSION_PAUSED), &session_paused);
    napi_create_int32(env, static_cast<int32_t>(SESSION_FAILED), &session_failed);
}

static void NapiCreateAction(napi_env env, napi_value &action)
{
    napi_create_object(env, &action);
    NapiUtils::SetUint32Property(env, action, "DOWNLOAD", static_cast<uint32_t>(Action::DOWNLOAD));
    NapiUtils::SetUint32Property(env, action, "UPLOAD", static_cast<uint32_t>(Action::UPLOAD));
}

static void NapiCreateMode(napi_env env, napi_value &mode)
{
    napi_create_object(env, &mode);
    NapiUtils::SetUint32Property(env, mode, "FOREGROUND", static_cast<uint32_t>(Mode::FOREGROUND));
}

static void NapiCreateNetwork(napi_env env, napi_value &network)
{
    napi_create_object(env, &network);
    NapiUtils::SetUint32Property(env, network, "ANY", static_cast<uint32_t>(Network::ANY));
    NapiUtils::SetUint32Property(env, network, "WIFI", static_cast<uint32_t>(Network::WIFI));
    NapiUtils::SetUint32Property(env, network, "CELLULAR", static_cast<uint32_t>(Network::CELLULAR));
}

static void NapiCreateState(napi_env env, napi_value &state)
{
    napi_create_object(env, &state);
    NapiUtils::SetUint32Property(env, state, "INITIALIZED", static_cast<uint32_t>(State::INITIALIZED));
    NapiUtils::SetUint32Property(env, state, "WAITING", static_cast<uint32_t>(State::WAITING));
    NapiUtils::SetUint32Property(env, state, "RUNNING", static_cast<uint32_t>(State::RUNNING));
    NapiUtils::SetUint32Property(env, state, "RETRYING", static_cast<uint32_t>(State::RETRYING));
    NapiUtils::SetUint32Property(env, state, "PAUSED", static_cast<uint32_t>(State::PAUSED));
    NapiUtils::SetUint32Property(env, state, "STOPPED", static_cast<uint32_t>(State::STOPPED));
    NapiUtils::SetUint32Property(env, state, "COMPLETED", static_cast<uint32_t>(State::COMPLETED));
    NapiUtils::SetUint32Property(env, state, "FAILED", static_cast<uint32_t>(State::FAILED));
    NapiUtils::SetUint32Property(env, state, "REMOVED", static_cast<uint32_t>(State::REMOVED));
}

static void NapiCreateFaults(napi_env env, napi_value &faults)
{
    napi_create_object(env, &faults);
    NapiUtils::SetUint32Property(env, faults, "OTHERS", static_cast<uint32_t>(Faults::OTHERS));
    NapiUtils::SetUint32Property(env, faults, "DISCONNECTED", static_cast<uint32_t>(Faults::DISCONNECTED));
    NapiUtils::SetUint32Property(env, faults, "TIMEOUT", static_cast<uint32_t>(Faults::TIMEOUT));
    NapiUtils::SetUint32Property(env, faults, "PROTOCOL", static_cast<uint32_t>(Faults::PROTOCOL));
    NapiUtils::SetUint32Property(env, faults, "FSIO", static_cast<uint32_t>(Faults::FSIO));
}

static void NapiCreateBroadcastEvent(napi_env env, napi_value &broadcastEvent)
{
    napi_create_object(env, &broadcastEvent);
    NapiUtils::SetStringPropertyUtf8(
        env, broadcastEvent, "COMPLETE", static_cast<std::string>(BROADCAST_EVENT_COMPLETE));
}

static napi_value InitAgent(napi_env env, napi_value exports)
{
    REQUEST_HILOGI("InitAgent in");
    napi_value action = nullptr;
    NapiCreateAction(env, action);
    napi_value mode = nullptr;
    NapiCreateMode(env, mode);
    napi_value network = nullptr;
    NapiCreateNetwork(env, network);
    napi_value state = nullptr;
    NapiCreateState(env, state);
    napi_value faults = nullptr;
    NapiCreateFaults(env, faults);
    napi_value broadcastEvent = nullptr;
    NapiCreateBroadcastEvent(env, broadcastEvent);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_PROPERTY("Action", action),
        DECLARE_NAPI_PROPERTY("Mode", mode),
        DECLARE_NAPI_PROPERTY("Network", network),
        DECLARE_NAPI_PROPERTY("State", state),
        DECLARE_NAPI_PROPERTY("Faults", faults),
        DECLARE_NAPI_PROPERTY("BroadcastEvent", broadcastEvent),

        DECLARE_NAPI_METHOD("create", NapiTask::Create),
        DECLARE_NAPI_METHOD("remove", NapiTask::Remove),
        DECLARE_NAPI_METHOD("show", NapiTask::Show),
        DECLARE_NAPI_METHOD("touch", NapiTask::Touch),
        DECLARE_NAPI_METHOD("search", NapiTask::Search),
    };
    napi_status status = napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc);
    REQUEST_HILOGI("InitV10 end %{public}d", status);
    return exports;
}

static napi_value Init(napi_env env, napi_value exports)
{
    NapiCreateInt32(env);
    napi_value agent = nullptr;
    napi_create_object(env, &agent);
    InitAgent(env, agent);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("EXCEPTION_PERMISSION", exception_permission),
        DECLARE_NAPI_STATIC_PROPERTY("EXCEPTION_PARAMCHECK", exception_parameter_check),
        DECLARE_NAPI_STATIC_PROPERTY("EXCEPTION_UNSUPPORTED", exception_unsupported),
        DECLARE_NAPI_STATIC_PROPERTY("EXCEPTION_FILEIO", exception_file_IO),
        DECLARE_NAPI_STATIC_PROPERTY("EXCEPTION_FILEPATH", exception_file_path),
        DECLARE_NAPI_STATIC_PROPERTY("EXCEPTION_SERVICE", exception_service_error),
        DECLARE_NAPI_STATIC_PROPERTY("EXCEPTION_OTHERS", exception_other),

        DECLARE_NAPI_STATIC_PROPERTY("NETWORK_MOBILE", network_mobile),
        DECLARE_NAPI_STATIC_PROPERTY("NETWORK_WIFI", network_wifi),

        DECLARE_NAPI_STATIC_PROPERTY("ERROR_CANNOT_RESUME", err_cannot_resume),
        DECLARE_NAPI_STATIC_PROPERTY("ERROR_DEVICE_NOT_FOUND", err_dev_not_found),
        DECLARE_NAPI_STATIC_PROPERTY("ERROR_FILE_ALREADY_EXISTS", err_file_exist),
        DECLARE_NAPI_STATIC_PROPERTY("ERROR_FILE_ERROR", err_file_error),
        DECLARE_NAPI_STATIC_PROPERTY("ERROR_HTTP_DATA_ERROR", err_http_data),
        DECLARE_NAPI_STATIC_PROPERTY("ERROR_INSUFFICIENT_SPACE", err_no_space),
        DECLARE_NAPI_STATIC_PROPERTY("ERROR_TOO_MANY_REDIRECTS", err_many_redirect),
        DECLARE_NAPI_STATIC_PROPERTY("ERROR_UNHANDLED_HTTP_CODE", err_http_code),
        DECLARE_NAPI_STATIC_PROPERTY("ERROR_UNKNOWN", err_unknown),
        DECLARE_NAPI_STATIC_PROPERTY("ERROR_OFFLINE", err_offline),
        DECLARE_NAPI_STATIC_PROPERTY("ERROR_UNSUPPORTED_NETWORK_TYPE", err_unsupported_network_type),

        DECLARE_NAPI_STATIC_PROPERTY("PAUSED_QUEUED_FOR_WIFI", paused_queue_wifi),
        DECLARE_NAPI_STATIC_PROPERTY("PAUSED_WAITING_FOR_NETWORK", paused_for_network),
        DECLARE_NAPI_STATIC_PROPERTY("PAUSED_WAITING_TO_RETRY", paused_to_retry),
        DECLARE_NAPI_STATIC_PROPERTY("PAUSED_BY_USER", paused_by_user),
        DECLARE_NAPI_STATIC_PROPERTY("PAUSED_UNKNOWN", paused_unknown),

        DECLARE_NAPI_STATIC_PROPERTY("SESSION_SUCCESSFUL", session_success),
        DECLARE_NAPI_STATIC_PROPERTY("SESSION_RUNNING", session_running),
        DECLARE_NAPI_STATIC_PROPERTY("SESSION_PENDING", session_pending),
        DECLARE_NAPI_STATIC_PROPERTY("SESSION_PAUSED", session_paused),
        DECLARE_NAPI_STATIC_PROPERTY("SESSION_FAILED", session_failed),

        DECLARE_NAPI_PROPERTY("agent", agent),

        DECLARE_NAPI_METHOD("downloadFile", NapiTask::JsDownload),
        DECLARE_NAPI_METHOD("uploadFile", NapiTask::JsUpload)
    };

    napi_status status = napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc);
    REQUEST_HILOGI("init request %{public}d", status);
    return exports;
}

#ifdef ANDROID_PLATFORM
static void JSRegisterProgressResult()
{
    const char className[] = "ohos.ace.plugin.taskmanagerplugin.JavaTaskImpl";
    ARKUI_X_Plugin_RegisterJavaPlugin(&TaskManagerJni::Register, className);
}
#endif

extern "C" __attribute__((constructor)) void RequestRegister()
{
    static napi_module module = {
        .nm_version = 1,
        .nm_flags = 0,
        .nm_filename = nullptr,
        .nm_register_func = Init,
        .nm_modname = "request",
        .nm_priv = ((void *)0),
        .reserved = { 0 }
    };
#ifdef ANDROID_PLATFORM
    JSRegisterProgressResult();
#endif
    napi_module_register(&module);
    REQUEST_HILOGI("module register request");
}
} // namespace OHOS::Plugin::Request
