/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "app_event_stat.h"
#include "hiappevent_clean.h"
#include "hilog/log.h"
#include "napi_app_event_holder.h"
#include "napi_error.h"
#include "napi_hiappevent_builder.h"
#include "napi_hiappevent_config.h"
#include "napi_hiappevent_init.h"
#include "napi_hiappevent_userinfo.h"
#include "napi_hiappevent_watch.h"
#include "napi_hiappevent_write.h"
#include "napi_param_builder.h"
#include "napi_util.h"
#include "time_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "Napi"

using namespace OHOS::HiviewDFX;

namespace {
constexpr size_t MAX_PARAM_NUM = 4;
}

static napi_value Write(napi_env env, napi_callback_info info)
{
    napi_value params[MAX_PARAM_NUM] = { 0 };
    size_t paramNum = NapiUtil::GetCbInfo(env, info, params);
    NapiHiAppEventBuilder builder;
    auto appEventPack = builder.BuildV9(env, params, paramNum);
    if (appEventPack == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to build appEventPack.");
        return nullptr;
    }

    auto asyncContext = new (std::nothrow) NapiHiAppEventWrite::HiAppEventAsyncContext(env);
    if (asyncContext == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to new asyncContext.");
        return nullptr;
    }
    asyncContext->appEventPack = appEventPack;
    asyncContext->result = builder.GetResult();
    asyncContext->callback = builder.GetCallback();
    asyncContext->isV9 = true;

    // if the build is successful, the event verification is performed
    if (asyncContext->result >= 0) {
        if (auto ret = VerifyAppEvent(asyncContext->appEventPack); ret != 0) {
            asyncContext->result = ret;
        }
    }

    napi_value promise = nullptr;
    if (asyncContext->callback == nullptr && napi_create_promise(env, &asyncContext->deferred, &promise) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "callback is null, failed to create promise.");
        delete asyncContext;
        return nullptr;
    }
    NapiHiAppEventWrite::Write(env, asyncContext);
    return promise;
}

static napi_value Configure(napi_env env, napi_callback_info info)
{
    napi_value params[MAX_PARAM_NUM] = { 0 };
    if (NapiUtil::GetCbInfo(env, info, params) < 1) { // The min num of params for configure is 1
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("config"));
        return nullptr;
    }
    if (!NapiHiAppEventConfig::Configure(env, params[0], true)) {
        HILOG_ERROR(LOG_CORE, "failed to configure HiAppEvent");
    }
    return nullptr;
}

static napi_value SetUserId(napi_env env, napi_callback_info info)
{
    napi_value params[MAX_PARAM_NUM] = { 0 };
    if (NapiUtil::GetCbInfo(env, info, params) < 2) { // The min num of params for setUserId is 2
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("setUserId"));
        return nullptr;
    }
    if (!NapiHiAppEventUserInfo::SetUserId(env, params[0], params[1])) {
        HILOG_ERROR(LOG_CORE, "failed to set userId");
    }
    return nullptr;
}

static napi_value GetUserId(napi_env env, napi_callback_info info)
{
    napi_value params[MAX_PARAM_NUM] = { 0 };
    if (NapiUtil::GetCbInfo(env, info, params) < 1) { // The min num of params for getUserId is 1
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("getUserId"));
        return nullptr;
    }

    napi_value userId = nullptr;
    if (!NapiHiAppEventUserInfo::GetUserId(env, params[0], userId)) {
        HILOG_ERROR(LOG_CORE, "failed to get userId");
    }
    return userId;
}

static napi_value SetUserProperty(napi_env env, napi_callback_info info)
{
    napi_value params[MAX_PARAM_NUM] = { 0 };
    if (NapiUtil::GetCbInfo(env, info, params) < 2) { // The min num of params for setUserProperty is 2
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("setUserProperty"));
        return nullptr;
    }
    if (!NapiHiAppEventUserInfo::SetUserProperty(env, params[0], params[1])) {
        HILOG_ERROR(LOG_CORE, "failed to set userProperty");
    }
    return nullptr;
}

static napi_value GetUserProperty(napi_env env, napi_callback_info info)
{
    napi_value params[MAX_PARAM_NUM] = { 0 };
    if (NapiUtil::GetCbInfo(env, info, params) < 1) { // The min num of params for getUserProperty is 1
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("getUserProperty"));
        return nullptr;
    }

    napi_value userProperty = nullptr;
    if (!NapiHiAppEventUserInfo::GetUserProperty(env, params[0], userProperty)) {
        HILOG_ERROR(LOG_CORE, "failed to get userProperty");
    }
    return userProperty;
}

static napi_value ClearData(napi_env env, napi_callback_info info)
{
    uint64_t beginTime = TimeUtil::GetMilliseconds();
    HiAppEventClean::ClearData();
    AppEventStat::WriteApiEndEvent("clearData", beginTime, AppEventStat::SUCCESS, NapiError::ERR_OK);
    return NapiUtil::CreateUndefined(env);
}

static napi_value AddWatcher(napi_env env, napi_callback_info info)
{
    uint64_t beginTime = TimeUtil::GetMilliseconds();
    napi_value params[MAX_PARAM_NUM] = { 0 };
    if (NapiUtil::GetCbInfo(env, info, params) < 1) { // The min num of params for addWatcher is 1
        AppEventStat::WriteApiEndEvent("addWatcher", beginTime, AppEventStat::FAILED, NapiError::ERR_PARAM);
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("watcher"));
        return nullptr;
    }
    return NapiHiAppEventWatch::AddWatcher(env, params[0], beginTime);
}

static napi_value RemoveWatcher(napi_env env, napi_callback_info info)
{
    uint64_t beginTime = TimeUtil::GetMilliseconds();
    napi_value params[MAX_PARAM_NUM] = { 0 };
    if (NapiUtil::GetCbInfo(env, info, params) < 1) { // The min num of params for removeWatcher is 1
        AppEventStat::WriteApiEndEvent("removeWatcher", beginTime, AppEventStat::FAILED, NapiError::ERR_PARAM);
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("watcher"));
        return nullptr;
    }
    return NapiHiAppEventWatch::RemoveWatcher(env, params[0], beginTime);
}

static napi_value SetEventParam(napi_env env, napi_callback_info info)
{
    napi_value params[MAX_PARAM_NUM] = { 0 };
    size_t paramNum = NapiUtil::GetCbInfo(env, info, params);
    NapiParamBuilder builder;
    auto appEventPack = builder.BuildEventParam(env, params, paramNum);
    if (appEventPack == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to build appEventPack.");
        return nullptr;
    }

    auto asyncContext = new (std::nothrow) NapiHiAppEventWrite::HiAppEventAsyncContext(env);
    if (asyncContext == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to new asyncContext.");
        return nullptr;
    }
    asyncContext->appEventPack = appEventPack;
    asyncContext->result = builder.GetResult();

    napi_value promise = nullptr;
    if (napi_create_promise(env, &asyncContext->deferred, &promise) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to create promise.");
        delete asyncContext;
        return nullptr;
    }

    NapiHiAppEventWrite::SetEventParam(env, asyncContext);
    return promise;
}

// EXTERN_C_START
napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("setUserId", SetUserId),
        DECLARE_NAPI_FUNCTION("getUserId", GetUserId),
        DECLARE_NAPI_FUNCTION("setUserProperty", SetUserProperty),
        DECLARE_NAPI_FUNCTION("getUserProperty", GetUserProperty),
        DECLARE_NAPI_FUNCTION("write", Write),
        DECLARE_NAPI_FUNCTION("configure", Configure),
        DECLARE_NAPI_FUNCTION("clearData", ClearData),
        DECLARE_NAPI_FUNCTION("addWatcher", AddWatcher),
        DECLARE_NAPI_FUNCTION("removeWatcher", RemoveWatcher),
        DECLARE_NAPI_FUNCTION("setEventParam", SetEventParam)
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc));
    NapiHiAppEventInit::InitNapiClassV9(env, exports);
    NapiAppEventHolder::NapiExport(env, exports);
    return exports;
}
// EXTERN_C_END

static napi_module g_module_v9 = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "hiviewdfx.hiAppEvent",
    .nm_priv = ((void*)0),
    .reserved = { 0 } };

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&g_module_v9);
}
