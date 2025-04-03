/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <unistd.h>

#include "log.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "plugins/ability_access_ctrl/ability_access_ctrl.h"
#include "plugins/ability_access_ctrl/napi_common.h"

#ifdef ANDROID_PLATFORM
#include "plugins/ability_access_ctrl/android/java/jni/ability_access_ctrl_jni.h"
#endif

#include "uv.h"

namespace OHOS::Plugin {
static thread_local napi_ref g_abilityTokenManagerRef;
static const std::string ATMANAGER_CLASS_NAME = "atManager";
static std::unique_ptr<AbilityAccessCtrl> g_acPlugin = nullptr;
enum class JsReturnCode {
    SUCCESS = 0,
    PARAM_ILLEGAL = 401,
    PARAM_INVALID = 12100001,
    INNER_ERROR = 12100009
};

enum class InputParams {
    VERIFY_MAX = 2,
    REQUEST_MAX = 3,
};

static constexpr int32_t ASYNC_CALL_BACK_VALUES_NUM = 2;
static constexpr int32_t MAX_LENGTH = 256;
static constexpr int32_t RET_SUCCESS = 0;

static inline bool IsTokenIDValid(uint32_t id) { return id != 0; }

static bool IsPermissionNameValid(const std::string& permissionName)
{
    return !permissionName.empty() && (permissionName.length() <= MAX_LENGTH);
}

napi_value GetNapiNull(napi_env env)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}

static const std::map<uint32_t, std::string> g_errorStringMap = {
    {static_cast<uint32_t>(JsReturnCode::PARAM_ILLEGAL), "The parameter is invalid."},
    {static_cast<uint32_t>(JsReturnCode::INNER_ERROR), "Common inner error."},
};

std::string GetParamErrorMsg(const std::string& param, const std::string& type)
{
    std::string msg = "Parameter Error. The type of \"" + param + "\" must be " + type + ".";
    return msg;
}

std::string GetErrorMessage(uint32_t errCode)
{
    auto iter = g_errorStringMap.find(errCode);
    if (iter != g_errorStringMap.end()) {
        return iter->second;
    }
    std::string errMsg = "Unknown error, errCode + " + std::to_string(errCode) + ".";
    return errMsg;
}

static napi_value GenerateBusinessError(napi_env env, int32_t errCode, const std::string& errMsg)
{
    napi_value businessError = nullptr;

    napi_value code = nullptr;
    NAPI_CALL(env, napi_create_uint32(env, errCode, &code));

    napi_value msg = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, errMsg.c_str(), NAPI_AUTO_LENGTH, &msg));

    NAPI_CALL(env, napi_create_error(env, nullptr, msg, &businessError));
    NAPI_CALL(env, napi_set_named_property(env, businessError, "code", code));
    NAPI_CALL(env, napi_set_named_property(env, businessError, "message", msg));
    return businessError;
}

static void ReturnPromiseResult(napi_env env, int32_t contextResult, napi_deferred deferred, napi_value result)
{
    if (contextResult != RET_SUCCESS) {
        napi_value businessError = GenerateBusinessError(env, contextResult, GetErrorMessage(contextResult));
        NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, deferred, businessError));
    } else {
        NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, deferred, result));
    }
}

static void ReturnCallbackResult(napi_env env, int32_t contextResult, napi_ref &callbackRef, napi_value result)
{
    napi_value businessError = GetNapiNull(env);
    if (contextResult != RET_SUCCESS) {
        businessError = GenerateBusinessError(env, contextResult, GetErrorMessage(contextResult));
    }
    napi_value results[ASYNC_CALL_BACK_VALUES_NUM] = { businessError, result };

    napi_value callback = nullptr;
    napi_value thisValue = nullptr;
    napi_value thatValue = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &thisValue));
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, 0, &thatValue));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, callbackRef, &callback));
    NAPI_CALL_RETURN_VOID(env,
        napi_call_function(env, thisValue, callback, ASYNC_CALL_BACK_VALUES_NUM, results, &thatValue));
}

void SetNamedProperty(napi_env env, napi_value dstObj, const int32_t objValue, const char* propName)
{
    napi_value prop = nullptr;
    napi_create_int32(env, objValue, &prop);
    napi_set_named_property(env, dstObj, propName, prop);
}

static bool ParseInputCheckPermission(
    const napi_env env, const napi_callback_info info, AtManagerAsyncContext& asyncContext)
{
    size_t argc = static_cast<size_t>(InputParams::VERIFY_MAX);

    napi_value argv[static_cast<size_t>(InputParams::VERIFY_MAX)] = { nullptr };
    napi_value thisVar = nullptr;
    std::string errMsg;
    void *data = nullptr;
    NAPI_CALL_BASE(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data), false);
    if (argc < static_cast<size_t>(InputParams::VERIFY_MAX)) {
        NAPI_CALL_BASE(env, napi_throw(env, GenerateBusinessError(env,
            static_cast<int32_t>(JsReturnCode::PARAM_ILLEGAL), "Parameter is missing.")), false);
        return false;
    }
    // 0: the first parameter of argv
    if (!ParseUint32(env, argv[0], asyncContext.tokenId)) {
        errMsg = GetParamErrorMsg("tokenId", "number");
        NAPI_CALL_BASE(env,
            napi_throw(env, GenerateBusinessError(env, static_cast<int32_t>(JsReturnCode::PARAM_ILLEGAL), errMsg)), false);
        return false;
    }
    // 1: the second parameter of argv
    if (!ParseString(env, argv[1], asyncContext.permission)) {
        errMsg = GetParamErrorMsg("permissionName", "string");
        NAPI_CALL_BASE(env,
            napi_throw(env, GenerateBusinessError(env, static_cast<int32_t>(JsReturnCode::PARAM_ILLEGAL), errMsg)), false);
        return false;
    }
    if (!IsTokenIDValid(asyncContext.tokenId) || !IsPermissionNameValid(asyncContext.permission)) {
        asyncContext.jsCode = static_cast<int32_t>(JsReturnCode::PARAM_INVALID); // -1: faile
    }
    return true;
}

static bool ParseInputRequestPermission(
    const napi_env env, const napi_callback_info info, RequestAsyncContext& asyncContext)
{
    size_t argc = static_cast<size_t>(InputParams::REQUEST_MAX);

    napi_value argv[static_cast<size_t>(InputParams::REQUEST_MAX)] = { nullptr };
    napi_value thisVar = nullptr;
    std::string errMsg;
    void *data = nullptr;
    NAPI_CALL_BASE(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data), false);
    if (argc < static_cast<size_t>(InputParams::REQUEST_MAX) - 1) {
        NAPI_CALL_BASE(env, napi_throw(env, GenerateBusinessError(env,
            static_cast<int32_t>(JsReturnCode::PARAM_ILLEGAL), "Parameter is missing.")), false);
        return false;
    }

    // argv[1] : permissionList
    if (!ParseStringArray(env, argv[1], asyncContext.permissionList) ||
        (asyncContext.permissionList.empty())) {
        errMsg = GetParamErrorMsg("permissions", "Array<string>");
        NAPI_CALL_BASE(
            env, napi_throw(env, GenerateBusinessError(env, static_cast<int32_t>(JsReturnCode::PARAM_ILLEGAL), errMsg)), false);
        return false;
    }

    // argv[2] : callback
    if (argc == static_cast<size_t>(InputParams::REQUEST_MAX)) {
        if (!ParseCallback(env, argv[2], asyncContext.callbackRef)) {    // argv[2] : callback
            errMsg = GetParamErrorMsg("callback", "Callback<PermissionRequestResult>");
            napi_throw(env, GenerateBusinessError(env, static_cast<int32_t>(JsReturnCode::PARAM_ILLEGAL), errMsg));
            return false;
        }
    }

    return true;
}

napi_value JsConstructor(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, nullptr));
    return thisVar;
}

static napi_value CreateAtManager(napi_env env, napi_callback_info cbInfo)
{
    napi_value instance = nullptr;
    napi_value cons = nullptr;

    NAPI_CALL(env, napi_get_reference_value(env, g_abilityTokenManagerRef, &cons));
    LOGW("Get a reference to the global variable g_abilityTokenManagerRef complete");

    NAPI_CALL(env, napi_new_instance(env, cons, 0, nullptr, &instance));
    LOGW("New js instance complete");
    return instance;
}

static napi_value JSCheckAccessTokenSync(napi_env env, napi_callback_info info)
{
    if (!g_acPlugin) {
        g_acPlugin = AbilityAccessCtrl::Create();
        if (!g_acPlugin) {
            LOGE("JSCheckAccessTokenSync: null return");
            return nullptr;
        }
    }
    auto* asyncContext = new (std::nothrow) AtManagerAsyncContext(env);
    if (asyncContext == nullptr) {
        return nullptr;
    }

    std::unique_ptr<AtManagerAsyncContext> context {asyncContext};
    if (!ParseInputCheckPermission(env, info, *asyncContext)) {
        return nullptr;
    }

    if (asyncContext->jsCode != static_cast<int32_t>(JsReturnCode::SUCCESS)) {
        NAPI_CALL(env, napi_throw(env, GenerateBusinessError(env,
            asyncContext->jsCode, GetErrorMessage(asyncContext->jsCode))));
        return nullptr;
    }

    bool isGranted = g_acPlugin->CheckPermission(asyncContext->permission);
    asyncContext->status = isGranted ? PERMISSION_GRANTED : PERMISSION_DENIED;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, asyncContext->status, &result));
    return result;
}

void CheckAccessTokenExecute(napi_env env, void *data)
{
    if (!g_acPlugin) {
        g_acPlugin = AbilityAccessCtrl::Create();
        if (!g_acPlugin) {
            LOGE("CheckAccessTokenExecute: null return");
            return;
        }
    }
    AtManagerAsyncContext* asyncContext = reinterpret_cast<AtManagerAsyncContext *>(data);
    if (asyncContext == nullptr) {
        return;
    }
    bool isGranted = g_acPlugin->CheckPermission(asyncContext->permission);
    asyncContext->status = isGranted ? PERMISSION_GRANTED : PERMISSION_DENIED;
}

void CheckAccessTokenComplete(napi_env env, napi_status status, void *data)
{
    AtManagerAsyncContext* asyncContext = reinterpret_cast<AtManagerAsyncContext *>(data);
    if (asyncContext == nullptr) {
        return;
    }
    std::unique_ptr<AtManagerAsyncContext> context {asyncContext};
    if (asyncContext->jsCode != static_cast<int32_t>(JsReturnCode::SUCCESS)) {
        napi_value error = GenerateBusinessError(env, asyncContext->jsCode, GetErrorMessage(asyncContext->jsCode));
        NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, asyncContext->deferred, error));
    } else {
        napi_value result = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, asyncContext->status, &result));
        NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, asyncContext->deferred, result));
    }
}

static napi_value JSCheckAccessToken(napi_env env, napi_callback_info info)
{
    auto* asyncContext = new (std::nothrow) AtManagerAsyncContext(env);
    if (asyncContext == nullptr) {
        return nullptr;
    }

    std::unique_ptr<AtManagerAsyncContext> context {asyncContext};
    if (!ParseInputCheckPermission(env, info, *asyncContext)) {
        return nullptr;
    }

    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &(asyncContext->deferred), &promise));

    napi_value resource = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "CheckAccessToken", NAPI_AUTO_LENGTH, &resource));

    NAPI_CALL(env, napi_create_async_work(
        env, nullptr, resource,
        CheckAccessTokenExecute, CheckAccessTokenComplete,
        reinterpret_cast<void *>(asyncContext), &(asyncContext->work)));
    NAPI_CALL(env, napi_queue_async_work(env, asyncContext->work));
    context.release();
    return promise;
}

static napi_value WrapRequestResult(const napi_env& env,
    const std::vector<std::string>& permissions, const std::vector<int>& grantResults)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value objPermissions;
    NAPI_CALL(env, napi_create_array(env, &objPermissions));
    for (size_t i = 0; i < permissions.size(); i++) {
        napi_value nPerm = nullptr;
        NAPI_CALL(env, napi_create_string_utf8(env, permissions[i].c_str(), NAPI_AUTO_LENGTH, &nPerm));
        NAPI_CALL(env, napi_set_element(env, objPermissions, i, nPerm));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "permissions", objPermissions));

    napi_value objGrantResults;
    NAPI_CALL(env, napi_create_array(env, &objGrantResults));
    for (size_t i = 0; i < grantResults.size(); i++) {
        napi_value nGrantResult = nullptr;
        NAPI_CALL(env, napi_create_int32(env, grantResults[i], &nGrantResult));
        NAPI_CALL(env, napi_set_element(env, objGrantResults, i, nGrantResult));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "authResults", objGrantResults));
    return result;
}

static void ResultCallbackJSThreadWorker(uv_work_t* work, int32_t status)
{
    (void)status;
    if (work == nullptr) {
        LOGE("uv_queue_work input work is nullptr");
        return;
    }
    std::unique_ptr<uv_work_t> uvWorkPtr {work};
    ResultCallback *retCB = reinterpret_cast<ResultCallback*>(work->data);
    if (retCB == nullptr) {
        LOGE("retCB is nullptr");
        return;
    }
    std::unique_ptr<ResultCallback> callbackPtr {retCB};

    RequestAsyncContext* context = reinterpret_cast<RequestAsyncContext*>(retCB->data);
    if (context == nullptr) {
        return;
    }
    std::unique_ptr<RequestAsyncContext> contextPtr {context};

    int32_t result = static_cast<int32_t>(JsReturnCode::SUCCESS);
    if (retCB->grantResults.empty()) {
        LOGE("grantResults empty");
        result = static_cast<int32_t>(JsReturnCode::INNER_ERROR);
    }
    napi_handle_scope scope = nullptr;
    NAPI_CALL_RETURN_VOID(context->env, napi_open_handle_scope(context->env, &scope));
    napi_value requestResult = WrapRequestResult(context->env, retCB->permissions, retCB->grantResults);
    if (requestResult == nullptr) {
        LOGE("wrap requestResult failed");
        result = static_cast<int32_t>(JsReturnCode::INNER_ERROR);
    }

    if (context->deferred != nullptr) {
        ReturnPromiseResult(context->env, result,  context->deferred, requestResult);
    } else {
        ReturnCallbackResult(context->env, result, context->callbackRef, requestResult);
    }
    NAPI_CALL_RETURN_VOID(context->env, napi_close_handle_scope(context->env, scope));
    LOGI("OnRequestPermissionsFromUser async callback is called end");
}

void OnRequestPermission(
    void* data, const std::vector<std::string>& permissions, const std::vector<int> grantResults)
{
    LOGI("AbilityAccessCtrlImpl OnRequest called");
    auto* retCB = new (std::nothrow) ResultCallback();
    if (retCB == nullptr) {
        LOGE("insufficient memory for work!");
        return;
    }
    std::unique_ptr<ResultCallback> callbackPtr {retCB};
    retCB->permissions = permissions;
    retCB->grantResults = grantResults;
    retCB->data = data;

    RequestAsyncContext* asyncContext = reinterpret_cast<RequestAsyncContext*>(data);
    if (asyncContext == nullptr) {
        LOGE("asyncContext is nullptr!");
        return;
    }
    std::unique_ptr<RequestAsyncContext> contextPtr {asyncContext};

    uv_loop_s* loop = nullptr;
    NAPI_CALL_RETURN_VOID(asyncContext->env, napi_get_uv_event_loop(asyncContext->env, &loop));
    if (loop == nullptr) {
        LOGE("loop instance is nullptr");
        return;
    }
    uv_work_t* work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("insufficient memory for work!");
        return;
    }
    std::unique_ptr<uv_work_t> uvWorkPtr {work};
    work->data = reinterpret_cast<void *>(retCB);
    NAPI_CALL_RETURN_VOID(asyncContext->env,
        uv_queue_work(loop, work, [](uv_work_t* work) {}, ResultCallbackJSThreadWorker));

    uvWorkPtr.release();
    callbackPtr.release();
    contextPtr.release();
}

static napi_value JSRequestPermissionsFromUser(napi_env env, napi_callback_info info)
{
    LOGE("JSRequestPermissionsFromUser enter");
    if (!g_acPlugin) {
        g_acPlugin = AbilityAccessCtrl::Create();
        if (!g_acPlugin) {
            LOGE("RequestPermissionsFromUserExecute: null return");
            return nullptr;
        }
    }
    auto* asyncContext = new (std::nothrow) RequestAsyncContext(env);
    if (asyncContext == nullptr) {
        return nullptr;
    }

    std::unique_ptr<RequestAsyncContext> context {asyncContext};
    if (!ParseInputRequestPermission(env, info, *asyncContext)) {
        return nullptr;
    }
    napi_value result = nullptr;
    if (asyncContext->callbackRef == nullptr) {
        NAPI_CALL(env, napi_create_promise(env, &(asyncContext->deferred), &result));
    } else {
        NAPI_CALL(env, napi_get_undefined(env, &result));
    }
    g_acPlugin->RequestPermissions(
        asyncContext->permissionList, OnRequestPermission, reinterpret_cast<void *>(asyncContext));
    context.release();
    LOGE("JSRequestPermissionsFromUser leave");
    return result;
}

static napi_value AbilityAccessCtrlExport(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptor[] = { DECLARE_NAPI_FUNCTION("createAtManager", CreateAtManager) };

    NAPI_CALL(env, napi_define_properties(env,
        exports, sizeof(descriptor) / sizeof(napi_property_descriptor), descriptor));

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("checkAccessTokenSync", JSCheckAccessTokenSync),
        DECLARE_NAPI_FUNCTION("checkAccessToken", JSCheckAccessToken),
        DECLARE_NAPI_FUNCTION("requestPermissionsFromUser", JSRequestPermissionsFromUser),
    };

    napi_value cons = nullptr;
    NAPI_CALL(env, napi_define_class(env, ATMANAGER_CLASS_NAME.c_str(), ATMANAGER_CLASS_NAME.size(),
        JsConstructor, nullptr, sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));

    NAPI_CALL(env, napi_create_reference(env, cons, 1, &g_abilityTokenManagerRef));
    NAPI_CALL(env, napi_set_named_property(env, exports, ATMANAGER_CLASS_NAME.c_str(), cons));

    napi_value grantStatus = nullptr;
    napi_create_object(env, &grantStatus);

    SetNamedProperty(env, grantStatus, PERMISSION_DENIED, "PERMISSION_DENIED");
    SetNamedProperty(env, grantStatus, PERMISSION_GRANTED, "PERMISSION_GRANTED");

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("GrantStatus", grantStatus),
    };
    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    return exports;
}

static napi_module abilityAccessCtrlModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = AbilityAccessCtrlExport,
    .nm_modname = "abilityAccessCtrl",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

#ifdef ANDROID_PLATFORM
static void AbilityAccessCtrlJniRegister()
{
    const char className[] = "ohos.ace.plugin.abilityaccessctrl.AbilityAccessCtrl";
    ARKUI_X_Plugin_RegisterJavaPlugin(&AbilityAccessCtrlJni::Register, className);
}
#endif

extern "C" __attribute__((constructor)) void AbilityAccessCtrlRegister()
{
#ifdef ANDROID_PLATFORM
    AbilityAccessCtrlJniRegister();
#endif
    napi_module_register(&abilityAccessCtrlModule);
}
} // namespace OHOS::Plugin
