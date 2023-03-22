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

using namespace OHOS::Plugin::Request::UploadNapi;
using namespace OHOS::Plugin::Request::Upload;
using namespace OHOS::Plugin::Request::Download;

namespace OHOS::Plugin::Request {
// fix code rule issue
static napi_value err_file_exist = nullptr;
static napi_value err_file_error = nullptr;
static napi_value err_http_data = nullptr;
static napi_value err_no_space = nullptr;
static napi_value err_http_code = nullptr;
static napi_value err_unknown = nullptr;

static void NapiCreateInt32(napi_env env)
{
    /* Create Network Type Const */
    napi_create_int32(env, static_cast<int32_t>(ERROR_FILE_ALREADY_EXISTS), &err_file_exist);
    napi_create_int32(env, static_cast<int32_t>(ERROR_FILE_ERROR), &err_file_error);
    napi_create_int32(env, static_cast<int32_t>(ERROR_HTTP_DATA_ERROR), &err_http_data);
    napi_create_int32(env, static_cast<int32_t>(ERROR_INSUFFICIENT_SPACE), &err_no_space);
    napi_create_int32(env, static_cast<int32_t>(ERROR_UNHANDLED_HTTP_CODE), &err_http_code);
    napi_create_int32(env, static_cast<int32_t>(ERROR_UNKNOWN), &err_unknown);
}

static napi_value Init(napi_env env, napi_value exports)
{
    NapiCreateInt32(env);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("ERROR_FILE_ALREADY_EXISTS", err_file_exist),
        DECLARE_NAPI_STATIC_PROPERTY("ERROR_FILE_ERROR", err_file_error),
        DECLARE_NAPI_STATIC_PROPERTY("ERROR_HTTP_DATA_ERROR", err_http_data),
        DECLARE_NAPI_STATIC_PROPERTY("ERROR_INSUFFICIENT_SPACE", err_no_space),
        DECLARE_NAPI_STATIC_PROPERTY("ERROR_UNHANDLED_HTTP_CODE", err_http_code),
        DECLARE_NAPI_STATIC_PROPERTY("ERROR_UNKNOWN", err_unknown),

        {"download", 0, DownloadTaskNapi::JsMain, 0, 0, 0, napi_default, 0 },
        {"upload", 0, UploadTaskNapi::JsUpload, 0, 0, 0, napi_default, 0 }
    };

    napi_status status = napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc);
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "init request %{public}d", status);
    return exports;
}

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
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "module register request");
}
} // namespace OHOS::Plugin
