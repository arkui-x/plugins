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

#include "js_task.h"

#include "complete_notify.h"
#include "fail_notify.h"
#include "header_notify.h"
#include "log.h"
#include "pause_notify.h"
#include "resume_notify.h"
#include "response_notify.h"
#include "progress_notify.h"
#include "remove_notify.h"
#include "request_utils.h"
#include "task.h"
#include "task_manager.h"

namespace OHOS::Plugin::Request {
constexpr const std::int32_t DECIMALISM = 10;
std::mutex JsTask::listenerMutex_;
std::map<std::string, std::vector<std::shared_ptr<JsNotify>>> JsTask::listenerMap_;

std::unordered_set<std::string> JsTask::supportEventsV9_ = {
    EVENT_COMPLETE,
    EVENT_PAUSE,
    EVENT_REMOVE,
    EVENT_PROGRESS,
    EVENT_HEADERRECEIVE,
    EVENT_FAIL,
};

std::unordered_set<std::string> JsTask::supportEventsV10_ = {
    EVENT_PAUSE,
    EVENT_RESUME,
    EVENT_REMOVE,
    EVENT_PROGRESS,
    EVENT_COMPLETED,
    EVENT_RESPONSE,
    EVENT_FAILED,
};

std::map<std::string, JsTask::Event> JsTask::functionMap_ = {
    {FUNCTION_PAUSE, JsTask::PauseExec},
    {FUNCTION_QUERY, JsTask::QueryExec},
    {FUNCTION_GET_TASK_MIME_TYPE, JsTask::QueryMimeTypeExec},
    {FUNCTION_DELETE, JsTask::RemoveExec},
    {FUNCTION_RESUME, JsTask::ResumeExec},
    {FUNCTION_START, JsTask::StartExec},
    {FUNCTION_STOP, JsTask::StopExec},
};

std::map<std::string, uint32_t> JsTask::resMap_ = {
    {FUNCTION_PAUSE, BOOL_RES},
    {FUNCTION_QUERY, INFO_RES},
    {FUNCTION_GET_TASK_MIME_TYPE, STR_RES},
    {FUNCTION_DELETE, BOOL_RES},
    {FUNCTION_RESUME, BOOL_RES},
    {FUNCTION_START, BOOL_RES},
};

std::map<State, DownloadStatus> JsTask::stateMap_ = {
    {State::INITIALIZED, SESSION_PENDING},
    {State::WAITING, SESSION_PAUSED},
    {State::RUNNING, SESSION_RUNNING},
    {State::RETRYING, SESSION_RUNNING},
    {State::PAUSED, SESSION_PAUSED},
    {State::COMPLETED, SESSION_SUCCESS},
    {State::STOPPED, SESSION_FAILED},
    {State::FAILED, SESSION_FAILED},
};

std::map<Reason, DownloadErrorCode> JsTask::failMap_ = {
    {REASON_OK, ERROR_FILE_ALREADY_EXISTS},
    {IO_ERROR, ERROR_FILE_ERROR},
    {REDIRECT_ERROR, ERROR_TOO_MANY_REDIRECTS},
    {OTHERS_ERROR, ERROR_UNKNOWN},
    {NETWORK_OFFLINE, ERROR_OFFLINE},
    {UNSUPPORTED_NETWORK_TYPE, ERROR_UNSUPPORTED_NETWORK_TYPE},
    {UNSUPPORT_RANGE_REQUEST, ERROR_UNKNOWN},
};

napi_value JsTask::Pause(napi_env env, napi_callback_info info)
{
    REQUEST_HILOGI("Pause in");
    return Exec(env, info, FUNCTION_PAUSE);
}

napi_value JsTask::Query(napi_env env, napi_callback_info info)
{
    REQUEST_HILOGI("QueryV8 in");
    return Exec(env, info, FUNCTION_QUERY);
}

napi_value JsTask::QueryMimeType(napi_env env, napi_callback_info info)
{
    REQUEST_HILOGI("QueryMimeType in");
    return Exec(env, info, FUNCTION_GET_TASK_MIME_TYPE);
}

napi_value JsTask::Remove(napi_env env, napi_callback_info info)
{
    REQUEST_HILOGI("RemoveV8 in");
    return Exec(env, info, FUNCTION_DELETE);
}

napi_value JsTask::Resume(napi_env env, napi_callback_info info)
{
    REQUEST_HILOGI("Resume in");
    return Exec(env, info, FUNCTION_RESUME);
}

napi_value JsTask::Start(napi_env env, napi_callback_info info)
{
    REQUEST_HILOGI("Start in");
    return Exec(env, info, FUNCTION_START);
}

napi_value JsTask::Stop(napi_env env, napi_callback_info info)
{
    REQUEST_HILOGI("Stop in");
    return Exec(env, info, FUNCTION_STOP);
}

napi_value JsTask::On(napi_env env, napi_callback_info info)
{
    REQUEST_HILOGI("On callback object");
    JsParam jsParam;
    ExceptionError err = ParseOnOffParameters(env, info, true, jsParam);
    if (err.code != E_OK) {
        NapiUtils::ThrowError(env, err.code, err.errInfo);
        return nullptr;
    }

    auto eventType = RequestUtils::GetEventType(jsParam.task->GetId(), jsParam.type);
    std::lock_guard<std::mutex> autoLock(listenerMutex_);
    auto it = listenerMap_.find(eventType);
    if (it != listenerMap_.end()) {
        for (auto item = it->second.begin(); item != it->second.end(); item++) {
            if (NapiUtils::Equals(env, jsParam.callback, (*item)->GetRef())) {
                REQUEST_HILOGE("Duplicate callback object");
                return nullptr;
            }
        }
    }

    auto listener = CreateNotify(env, jsParam.type, jsParam.callback,
                                 jsParam.task->GetVersion(), jsParam.task->GetAction());
    if (listener == nullptr) {
        REQUEST_HILOGE("Create callback object fail");
        return nullptr;
    }

    if (it != listenerMap_.end()) {
        it->second.emplace_back(listener);
    } else {
        std::vector<std::shared_ptr<JsNotify>> list {};
        list.emplace_back(listener);
        listenerMap_.emplace(eventType, list);
    }
    jsParam.task->AddListener(jsParam.type, listener);
    return nullptr;
}

napi_value JsTask::Off(napi_env env, napi_callback_info info)
{
    REQUEST_HILOGI("Off callback object");
    JsParam jsParam;
    ExceptionError err = ParseOnOffParameters(env, info, false, jsParam);
    if (err.code != E_OK) {
        NapiUtils::ThrowError(env, err.code, err.errInfo);
        return nullptr;
    }

    auto eventType = RequestUtils::GetEventType(jsParam.task->GetId(), jsParam.type);
    std::lock_guard<std::mutex> autoLock(listenerMutex_);
    auto it = listenerMap_.find(eventType);
    if (it == listenerMap_.end()) {
        REQUEST_HILOGE("no callback object");
        return nullptr;
    }
    
    if (jsParam.callback == nullptr) {
        jsParam.task->RemoveListener(jsParam.type, nullptr);
    } else {
        for (auto item = it->second.begin(); item != it->second.end(); item++) {
            if (NapiUtils::Equals(env, jsParam.callback, (*item)->GetRef())) {
                jsParam.task->RemoveListener(jsParam.type, *item);
                break;
            }
        }
    }
    listenerMap_.erase(it);
    return nullptr;
}

bool JsTask::IsSupportType(const std::string &type, Version version)
{
    if (version == Version::API10) {
        return supportEventsV10_.find(type) != supportEventsV10_.end();
    }
    return supportEventsV9_.find(type) != supportEventsV9_.end();
}

ExceptionError JsTask::ParseOnOffParameters(napi_env env, napi_callback_info info, bool isRequiredParam,
    JsParam &jsParam)
{
    ExceptionError err = { .code = E_OK };
    size_t argc = NapiUtils::MAX_ARGC;
    napi_value argv[NapiUtils::MAX_ARGC] = { nullptr };
    napi_value self = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &self, nullptr);
    if (status != napi_ok) {
        return { .code = E_PARAMETER_CHECK, .errInfo = "Failed to obtain parameters" };
    }
    napi_unwrap(env, self, reinterpret_cast<void **>(&jsParam.task));
    if (jsParam.task == nullptr) {
        return { .code = E_PARAMETER_CHECK, .errInfo = "Failed to obtain the current object" };
    }

    if ((isRequiredParam && argc < NapiUtils::TWO_ARG) || (!isRequiredParam && argc < NapiUtils::ONE_ARG)) {
        return { .code = E_PARAMETER_CHECK, .errInfo = "Wrong number of arguments" };
    }
    napi_valuetype valuetype;
    napi_typeof(env, argv[NapiUtils::FIRST_ARGV], &valuetype);
    if (valuetype != napi_string) {
        return { .code = E_PARAMETER_CHECK, .errInfo = "The first parameter is not of string type" };
    }
    jsParam.type = NapiUtils::Convert2String(env, argv[NapiUtils::FIRST_ARGV]);
    if (!IsSupportType(jsParam.type, jsParam.task->GetVersion())) {
        return { .code = E_PARAMETER_CHECK, .errInfo = "First parameter error" };
    }
    UnifiedType(jsParam.type);
    if (argc == NapiUtils::ONE_ARG) {
        return err;
    }

    if (argc == NapiUtils::TWO_ARG) {
        valuetype = napi_undefined;
        napi_typeof(env, argv[NapiUtils::SECOND_ARGV], &valuetype);
        if (valuetype != napi_function) {
            return { .code = E_PARAMETER_CHECK, .errInfo = "The second parameter is not of function type" };
        }
        jsParam.callback = argv[NapiUtils::SECOND_ARGV];
    }

    return err;
}

void JsTask::UnifiedType(std::string &type)
{
    if (type == EVENT_COMPLETE) {
        type = EVENT_COMPLETED;
    }
    if (type == EVENT_FAIL) {
        type = EVENT_FAILED;
    }
}

napi_value JsTask::Exec(napi_env env, napi_callback_info info, const std::string &execType)
{
    auto context = std::make_shared<ExecContext>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        return ParseInputParameters(env, argc, self, context);
    };
    auto output = [context, execType](napi_env env, napi_value *result) -> napi_status {
        if (context->retCode != E_OK) {
            return napi_generic_failure;
        }
        return GetResult(env, context, execType, *result);
    };
    auto exec = [context, execType](AsyncCall::Context *ctx) {
        auto handle = functionMap_.find(execType);
        if (handle != functionMap_.end()) {
            context->retCode = handle->second(context);
        }
    };

    context->SetAction(input, output);
    AsyncCall asyncCall(env, info, context);
    return asyncCall.Call(env, std::move(exec));
}

napi_status JsTask::ParseInputParameters(napi_env env, size_t argc, napi_value self,
    const std::shared_ptr<ExecContext> &context)
{
    NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
    NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&context->task)), napi_invalid_arg);
    NAPI_ASSERT_BASE(env, context->task != nullptr, "there is no native task", napi_invalid_arg);
    return napi_ok;
}

napi_status JsTask::GetResult(napi_env env, const std::shared_ptr<ExecContext> &context,
    const std::string &execType, napi_value &result)
{
    if (resMap_[execType] == BOOL_RES) {
        return NapiUtils::Convert2JSValue(env, context->boolRes, result);
    }
    if (resMap_[execType] == STR_RES) {
        return NapiUtils::Convert2JSValue(env, context->strRes, result);
    }
    if (resMap_[execType] == INFO_RES) {
        return NapiUtils::Convert2JSValue(env, context->infoRes, result);
    }
    return napi_generic_failure;
}

int32_t JsTask::StartExec(const std::shared_ptr<ExecContext> &context)
{
    if (context == nullptr || context->task == nullptr) {
        return E_PARAMETER_CHECK;
    }

    TaskInfo info;
    auto ret = TaskManager::Get().GetTaskInfo(context->task->GetId(), context->task->GetToken(), info);
    if (ret != E_OK || info.progress.state != State::INITIALIZED) {
        REQUEST_HILOGE("StartExec ret: %{public}d  state: %{public}d", ret, info.progress.state);
        return E_TASK_STATE;
    }

    ret = context->task->Start();
    if (ret == E_OK) {
        context->boolRes = true;
    }
    return ret;
}

int32_t JsTask::StopExec(const std::shared_ptr<ExecContext> &context)
{
    if (context == nullptr || context->task == nullptr) {
        return E_PARAMETER_CHECK;
    }

    TaskInfo info;
    auto ret = TaskManager::Get().GetTaskInfo(context->task->GetId(), "", info);
    bool isStarted = context->task->IsStarted();
    if (isStarted && info.progress.state == State::INITIALIZED) {
        info.progress.state = State::RUNNING;
    }
    if (ret != E_OK || 
        !(info.progress.state == State::WAITING || 
        info.progress.state == State::RUNNING ||
        info.progress.state == State::RETRYING)) {
        REQUEST_HILOGE("StopExec ret: %{public}d  state: %{public}d", ret, info.progress.state);
        return E_TASK_STATE;
    }

    ret = context->task->Stop();
    if (ret == E_OK) {
        context->boolRes = true;
    }
    return ret;
}

int32_t JsTask::PauseExec(const std::shared_ptr<ExecContext> &context)
{
    if (context == nullptr || context->task == nullptr) {
        return E_PARAMETER_CHECK;
    }

    TaskInfo info;
    auto ret = TaskManager::Get().GetTaskInfo(context->task->GetId(), "", info);
    if (ret != E_OK || !context->task->IsStarted() ||
     !(info.progress.state == State::INITIALIZED || 
     info.progress.state == State::WAITING || 
     info.progress.state == State::RUNNING)) {
        REQUEST_HILOGE("PauseExec ret: %{public}d  state: %{public}d", ret, info.progress.state);
        return E_TASK_STATE;
    }

    ret = context->task->Pause();
    if (ret == E_OK) {
        context->boolRes = true;
    }
    return ret;
}

int32_t JsTask::QueryExec(const std::shared_ptr<ExecContext> &context)
{
    if (context == nullptr || context->task == nullptr) {
        return E_PARAMETER_CHECK;
    }
    TaskInfo info;
    auto ret = TaskManager::Get().GetTaskInfo(context->task->GetId(), "", info);
    if (ret == E_OK) {
        context->boolRes = true;
    }
    GetDownloadInfo(info, context->infoRes);
    return ret;
}

int32_t JsTask::QueryMimeTypeExec(const std::shared_ptr<ExecContext> &context)
{
    if (context == nullptr || context->task == nullptr) {
        return E_PARAMETER_CHECK;
    }
    TaskInfo info;
    auto ret = TaskManager::Get().GetTaskInfo(context->task->GetId(), "", info);
    if (ret == E_OK) {
        context->boolRes = true;
        context->strRes = info.mimeType;
    }
    return E_OK;
}

void JsTask::GetDownloadInfo(const TaskInfo &infoRes, DownloadInfo &info)
{
    info.downloadId = strtoul(infoRes.tid.c_str(), NULL, DECIMALISM);
    if (infoRes.progress.state == State::FAILED) {
        auto it = failMap_.find(infoRes.code);
        if (it != failMap_.end()) {
            info.failedReason = it->second;
        } else {
            info.failedReason = ERROR_UNKNOWN;
        }
    }
    if (infoRes.progress.state == State::WAITING &&
        (infoRes.code == NETWORK_OFFLINE || infoRes.code == UNSUPPORTED_NETWORK_TYPE)) {
        info.pausedReason = PAUSED_WAITING_TO_RETRY;
    }
    if (infoRes.progress.state == State::PAUSED) {
        if (infoRes.code == USER_OPERATION) {
            info.pausedReason = PAUSED_BY_USER;
        }
    }
    if (!infoRes.files.empty()) {
        info.fileName = infoRes.files[0].filename;
        info.filePath = infoRes.files[0].uri;
    }
    auto it = stateMap_.find(infoRes.progress.state);
    if (it != stateMap_.end()) {
        info.status = it->second;
    }
    info.url = infoRes.url;
    info.downloadTitle = infoRes.title;
    if (!infoRes.progress.sizes.empty()) {
        info.downloadTotalBytes = infoRes.progress.sizes[0];
    }
    info.description = infoRes.description;
    info.downloadedBytes = infoRes.progress.processed;
}

int32_t JsTask::RemoveExec(const std::shared_ptr<ExecContext> &context)
{
    if (context == nullptr || context->task == nullptr) {
        return E_PARAMETER_CHECK;
    }
    auto ret = TaskManager::Get().Remove(context->task->GetId());
    if (ret == E_OK) {
        context->boolRes = true;
    }
    REQUEST_HILOGI("RemoveExec ret: %{public}d", ret);
    return ret;
}

int32_t JsTask::ResumeExec(const std::shared_ptr<ExecContext> &context)
{
    if (context == nullptr || context->task == nullptr) {
        return E_PARAMETER_CHECK;
    }
    TaskInfo info;
    auto ret = TaskManager::Get().GetTaskInfo(context->task->GetId(), "", info);
    if (ret != E_OK || info.progress.state != State::PAUSED) {
        REQUEST_HILOGE("ResumeExec %{public}d  state: %{public}d", ret, info.progress.state);
        return E_TASK_STATE;
    }
    ret = context->task->Resume();
    if (ret == E_OK) {
        context->boolRes = true;
    }
    return E_OK;
}

std::shared_ptr<JsNotify> JsTask::CreateNotify(napi_env env,
    const std::string &type, napi_value callback, Version version, Action action)
{
    REQUEST_HILOGI("CreateNotify type: %{public}s", type.c_str());
    if (type == EVENT_PROGRESS) {
        return std::make_shared<ProgressNotify>(env, callback, version, action);
    } else if (type == EVENT_COMPLETE || type == EVENT_COMPLETED) {
        return std::make_shared<CompleteNotify>(env, callback, version, action);
    } else if (type == EVENT_FAIL || type == EVENT_FAILED) {
        return std::make_shared<FailNotify>(env, callback, version, action);
    } else if (type == EVENT_HEADERRECEIVE) {
        return std::make_shared<HeaderNotify>(env, callback, version, action);
    } else if (type == EVENT_RESPONSE) {
        return std::make_shared<ResponseNotify>(env, callback, version, action);
    } else if (type == EVENT_PAUSE) {
        return std::make_shared<PauseNotify>(env, callback, version, action);
    } else if (type == EVENT_RESUME) {
        return std::make_shared<ResumeNotify>(env, callback, version, action);
    } else if (type == EVENT_REMOVE) {
        return std::make_shared<RemoveNotify>(env, callback, version, action);
    } else {
        REQUEST_HILOGE("not suppport event");
    }
    return nullptr;
}
} // namespace OHOS::Plugin::Request
