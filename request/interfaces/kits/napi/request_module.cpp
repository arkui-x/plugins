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
#include "download_task_napi.h"
#include "js_util.h"
#include "inner_utils/plugin_inner_napi_utils.h"
#include "plugin_c_utils.h"
#include "upload_task_napi.h"

#ifdef ANDROID_PLATFORM
#include "java/jni/download_manager_jni.h"
#endif

using namespace OHOS::Plugin::Request::UploadNapi;
using namespace OHOS::Plugin::Request::Upload;
using namespace OHOS::Plugin::Request::Download;

namespace OHOS::Plugin::Request {
// fix code rule issue
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
    napi_create_int32(env, static_cast<int32_t>(EXCEPTION_PERMISSION), &exception_permission);
    napi_create_int32(env, static_cast<int32_t>(EXCEPTION_PARAMETER_CHECK), &exception_parameter_check);
    napi_create_int32(env, static_cast<int32_t>(EXCEPTION_UNSUPPORTED), &exception_unsupported);
    napi_create_int32(env, static_cast<int32_t>(EXCEPTION_FILE_IO), &exception_file_IO);
    napi_create_int32(env, static_cast<int32_t>(EXCEPTION_FILE_PATH), &exception_file_path);
    napi_create_int32(env, static_cast<int32_t>(EXCEPTION_SERVICE_ERROR), &exception_service_error);
    napi_create_int32(env, static_cast<int32_t>(EXCEPTION_OTHER), &exception_other);

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

static napi_value Init(napi_env env, napi_value exports)
{
    NapiCreateInt32(env);

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

        {"downloadFile", 0, DownloadTaskNapi::JsMain, 0, 0, 0, napi_default, 0 },
        {"uploadFile", 0, UploadTaskNapi::JsUploadFile, 0, 0, 0, napi_default, 0 }
    };

    napi_status status = napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc);
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "init request %{public}d", status);
    return exports;
}

#ifdef ANDROID_PLATFORM
static void DownloadPluginJniRegister()
{
    const char className[] = "ohos.ace.plugin.downloadmanagerplugin.DownloadManagerPlugin";
    OH_Plugin_RegisterJavaPlugin(&DownloadManagerJni::Register, className);
}

static void JSRegisterProgressResult()
{
    const char className[] = "ohos.ace.plugin.downloadmanagerplugin.DownloadManagerPlugin";
    OH_Plugin_RegisterJavaPlugin(&DownloadManagerJni::Register, className);
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
    napi_module_register(&module);

#ifdef ANDROID_PLATFORM
    OH_Plugin_RunAsyncTask(&DownloadPluginJniRegister, OH_PLUGIN_PLATFORM_THREAD);
    OH_Plugin_RunAsyncTask(&JSRegisterProgressResult, OH_PLUGIN_PLATFORM_THREAD);
#endif

    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "module register request");
}
} // namespace OHOS::Plugin::Request
