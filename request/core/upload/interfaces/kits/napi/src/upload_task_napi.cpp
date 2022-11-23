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

#include "upload_task_napi.h"

#include <mutex>
#include <unistd.h>
#include <uv.h>

#include "async_call.h"
#include "js_util.h"
#include "upload_task.h"

using namespace OHOS::Plugin::Request::Upload;
namespace OHOS::Plugin::Request::UploadNapi {
std::vector<UploadTaskNapi *> UploadTaskNapi::uploadTaskList_;
std::mutex UploadTaskNapi::uploadTaskListLock_;
static constexpr int UploadTaskLimitSize = 4;
static constexpr long UploadTaskTimeout = 500000;
std::mutex mutex_;
int taskId_ = 0;
std::map<std::string, UploadTaskNapi::Exec> UploadTaskNapi::onTypeHandlers_ = {
    {"progress", UploadTaskNapi::OnProgress},
    {"fail", UploadTaskNapi::OnFail},
    {"complete", UploadTaskNapi::OnComplete},
};
std::map<std::string, UploadTaskNapi::Exec> UploadTaskNapi::offTypeHandlers_ = {
    {"progress", UploadTaskNapi::OffProgress},
    {"fail", UploadTaskNapi::OffFail},
    {"complete", UploadTaskNapi::OffComplete},
};

napi_value UploadTaskNapi::JsUpload(napi_env env, napi_callback_info info)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Enter JsUpload.");
    struct ContextInfo {
        napi_ref ref = nullptr;
    };
    auto ctxInfo = std::make_shared<ContextInfo>();
    auto input = [ctxInfo](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Upload parser to native params %{public}d!", static_cast<int>(argc));
        NAPI_ASSERT_BASE(env, (argc > 0) && (argc <= 2), " need 1 or 2 parameters!", napi_invalid_arg);
        napi_value uploadProxy = nullptr;
        napi_status status = napi_new_instance(env, GetCtor(env), argc, argv, &uploadProxy);
        if ((uploadProxy == nullptr) || (status != napi_ok)) {
            return napi_generic_failure;
        }
        napi_create_reference(env, uploadProxy, 1, &(ctxInfo->ref));
        return napi_ok;
    };
    auto output = [ctxInfo](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_reference_value(env, ctxInfo->ref, result);
        napi_delete_reference(env, ctxInfo->ref);
        return status;
    };
    auto context = std::make_shared<AsyncCall::Context>(input, output);
    AsyncCall asyncCall(env, info, context);
    return asyncCall.Call(env);
}

napi_status UploadTaskNapi::ParseParam(napi_env env, napi_callback_info info, bool IsRequiredParam,
    JsParam &jsParam)
{
    size_t argc = JSUtil::MAX_ARGC;
    napi_value argv[JSUtil::MAX_ARGC] = {nullptr};
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &jsParam.self, nullptr);
    if (status != napi_ok) {
        return napi_invalid_arg;
    }
    if (jsParam.self == nullptr) {
        return napi_invalid_arg;
    }

    if (!JSUtil::CheckParamNumber(argc, IsRequiredParam)) {
        return napi_invalid_arg;
    }
    if (!JSUtil::CheckParamType(env, argv[0], napi_string)) {
        return napi_invalid_arg;
    }
    jsParam.type = JSUtil::Convert2String(env, argv[0]);
    if (onTypeHandlers_.find(jsParam.type) == onTypeHandlers_.end()) {
        return napi_invalid_arg;
    }
    if (argc == TWO_ARG) {
        if (!JSUtil::CheckParamType(env, argv[1], napi_function)) {
            return napi_invalid_arg;
        }
        jsParam.callback = argv[1];
    }
    return napi_ok;
}
napi_value UploadTaskNapi::JsOn(napi_env env, napi_callback_info info)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Enter JsOn.");
    bool IsRequiredParam = true;
    JsParam jsParam;
    napi_status status = ParseParam(env, info, IsRequiredParam, jsParam);
    NAPI_ASSERT(env, status == napi_ok, "ParseParam fail");
    auto handle = onTypeHandlers_.find(jsParam.type);
    handle->second(env, jsParam.callback, jsParam.self);
    return nullptr;
}

napi_value UploadTaskNapi::JsOff(napi_env env, napi_callback_info info)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Enter JsOff.");
    bool IsRequiredParam = false;
    JsParam jsParam;
    napi_status status = ParseParam(env, info, IsRequiredParam, jsParam);
    NAPI_ASSERT(env, status == napi_ok, "ParseParam fail");
    auto handle = offTypeHandlers_.find(jsParam.type);
    handle->second(env, jsParam.callback, jsParam.self);
    return nullptr;
}

napi_value UploadTaskNapi::JsRemove(napi_env env, napi_callback_info info)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Enter JsRemove.");
    auto context = std::make_shared<RemoveContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        NAPI_ASSERT_BASE(env, argc == 0, " should 0 parameter!", napi_invalid_arg);
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, context->removeStatus, result);
        return status;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        context->removeStatus = context->proxy->napiUploadTask_->Remove();
        if (context->removeStatus == true) {
            context->status = napi_ok;
        }
    };
    context->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context));
    return asyncCall.Call(env, exec);
}

napi_status UploadTaskNapi::OnProgress(napi_env env, napi_value callback, napi_value self)
{
    UploadTaskNapi *proxy = nullptr;
    NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&proxy)), napi_invalid_arg);
    NAPI_ASSERT_BASE(env, proxy != nullptr, "there is no native upload task", napi_invalid_arg);

    std::shared_ptr<IProgressCallback> progressCallback = std::make_shared<ProgressCallback>(proxy, env, callback);
    if (!progressCallback) {
        UPLOAD_HILOGE(UPLOAD_MODULE_JS_NAPI, "create failed!");
        return napi_generic_failure;
    }
    if (JSUtil::Equals(env, callback, progressCallback->GetCallback()) && proxy->onFail_ != nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "OnProgress callback already register!");
        return napi_generic_failure;
    }
    proxy->offProgress_ = std::move(proxy->onProgress_);
    proxy->napiUploadTask_->On(TYPE_PROGRESS_CALLBACK, (void *)(progressCallback.get()));
    proxy->onProgress_ = std::move(progressCallback);
    return napi_ok;
}

napi_status UploadTaskNapi::OnFail(napi_env env, napi_value callback, napi_value self)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Enter OnFail.");
    UploadTaskNapi *proxy = nullptr;
    NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&proxy)), napi_invalid_arg);
    NAPI_ASSERT_BASE(env, proxy != nullptr, "there is no native upload task", napi_invalid_arg);

    std::shared_ptr<IFailCallback> failCallback = std::make_shared<FailCallback>(proxy, env, callback);
    if (!failCallback) {
        UPLOAD_HILOGE(UPLOAD_MODULE_JS_NAPI, "create failed!");
        return napi_generic_failure;
    }
    if (JSUtil::Equals(env, callback, failCallback->GetCallback()) && proxy->onFail_ != nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "OnFail callback already register!");
        return napi_generic_failure;
    }
    proxy->offFail_ = std::move(proxy->onFail_);
    proxy->napiUploadTask_->On(TYPE_FAIL_CALLBACK, (void *)(failCallback.get()));
    proxy->onFail_ = std::move(failCallback);
    return napi_ok;
}

napi_status UploadTaskNapi::OnComplete(napi_env env, napi_value callback, napi_value self)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Enter OnComplete.");
    UploadTaskNapi *proxy = nullptr;
    NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&proxy)), napi_invalid_arg);
    NAPI_ASSERT_BASE(env, proxy != nullptr, "there is no native upload task", napi_invalid_arg);

    std::shared_ptr<ICompleteCallback> completeCallback = std::make_shared<CompleteCallback>(proxy, env, callback);
    if (!completeCallback) {
        UPLOAD_HILOGE(UPLOAD_MODULE_JS_NAPI, "create failed!");
        return napi_generic_failure;
    }
    if (JSUtil::Equals(env, callback, completeCallback->GetCallback()) && proxy->onComplete_ != nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "OnComplete callback already register!");
        return napi_generic_failure;
    }
    proxy->offComplete_ = std::move(proxy->onComplete_);
    proxy->napiUploadTask_->On(TYPE_COMPLETE_CALLBACK, (void *)(completeCallback.get()));
    proxy->onComplete_ = std::move(completeCallback);
    return napi_ok;
}

napi_status UploadTaskNapi::OffProgress(napi_env env, napi_value callback, napi_value self)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Enter OffProgress.");
    UploadTaskNapi *proxy = nullptr;
    NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&proxy)), napi_invalid_arg);
    NAPI_ASSERT_BASE(env, proxy != nullptr, "there is no native upload task", napi_invalid_arg);

    if (proxy->onProgress_ == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Progress. proxy->onProgress_ == nullptr.");
        return napi_generic_failure;
    } else {
        std::shared_ptr<IProgressCallback> progressCallback = std::make_shared<ProgressCallback>(proxy, env, callback);
        proxy->napiUploadTask_->Off(TYPE_PROGRESS_CALLBACK, (void *)(progressCallback.get()));
        proxy->onProgress_ = nullptr;
    }
    return napi_ok;
}

napi_status UploadTaskNapi::OffFail(napi_env env, napi_value callback, napi_value self)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Enter OffFail.");
    UploadTaskNapi *proxy = nullptr;
    NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&proxy)), napi_invalid_arg);
    NAPI_ASSERT_BASE(env, proxy != nullptr, "there is no native upload task", napi_invalid_arg);

    if (proxy->onFail_ == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Fail. proxy->onFail_ == nullptr.");
        return napi_generic_failure;
    } else {
        std::shared_ptr<IFailCallback> failCallback = std::make_shared<FailCallback>(proxy, env, callback);
        proxy->napiUploadTask_->Off(TYPE_FAIL_CALLBACK, failCallback.get());
        proxy->onFail_ = nullptr;
        proxy->offFail_ = std::move(failCallback);
    }
    return napi_ok;
}


napi_status UploadTaskNapi::OffComplete(napi_env env, napi_value callback, napi_value self)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Enter OffComplete.");
    UploadTaskNapi *proxy = nullptr;
    NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&proxy)), napi_invalid_arg);
    NAPI_ASSERT_BASE(env, proxy != nullptr, "there is no native upload task", napi_invalid_arg);
    if (proxy->onComplete_ == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "CompleteCallback. proxy->OffComplete_ == nullptr.");
        return napi_generic_failure;
    } else {
        std::shared_ptr<ICompleteCallback> completeCallback = std::make_shared<CompleteCallback>(proxy, env, callback);
        proxy->napiUploadTask_->Off(TYPE_COMPLETE_CALLBACK, completeCallback.get());
        proxy->onComplete_ = nullptr;
    }
    return napi_ok;
}

void AddCallbackToConfig(std::shared_ptr<Upload::UploadConfig> &config, napi_env env, napi_value jsConfig,
    UploadTaskNapi *proxy)
{
    JSUtil::ParseFunction(env, jsConfig, "success", proxy->success_);
    JSUtil::ParseFunction(env, jsConfig, "fail", proxy->fail_);
    JSUtil::ParseFunction(env, jsConfig, "complete", proxy->complete_);

    config->fsuccess = std::bind(&UploadTaskNapi::OnSystemSuccess, proxy->env_, proxy->success_,
        std::placeholders::_1);
    config->ffail = std::bind(&UploadTaskNapi::OnSystemFail, proxy->env_, proxy->fail_,
        std::placeholders::_1, std::placeholders::_2);
    config->fcomplete = std::bind(&UploadTaskNapi::OnSystemComplete, proxy->env_, proxy->complete_);
}

napi_value UploadTaskNapi::GetCtor(napi_env env)
{
    napi_value cons = nullptr;
    napi_property_descriptor clzDes[] = {
        {"on", 0, JsOn, 0, 0, 0, napi_default, 0 },
        {"off", 0, JsOff, 0, 0, 0, napi_default, 0 },
        {"remove", 0, JsRemove, 0, 0, 0, napi_default, 0 },
    };
    NAPI_CALL(env, napi_define_class(env, "UploadTaskNapi", NAPI_AUTO_LENGTH, Initialize, nullptr,
                                    sizeof(clzDes) / sizeof(napi_property_descriptor), clzDes, &cons));
    return cons;
}

std::shared_ptr<Upload::UploadTask> UploadTaskNapi::GetTaskPtr()
{
    return napiUploadTask_;
}

napi_value UploadTaskNapi::Initialize(napi_env env, napi_callback_info info)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "constructor upload task!");
    napi_value self = nullptr;
    if (UploadTaskNapi::UploadTaskSize() >= UploadTaskLimitSize) {
        napi_get_undefined(env, &self);
        return self;
    }

    size_t argc = JSUtil::MAX_ARGC;
    int parametersPosition = 0;
    napi_value argv[JSUtil::MAX_ARGC] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &self, nullptr));
    auto *proxy = new (std::nothrow) UploadTaskNapi();
    if (proxy == nullptr) {
        UPLOAD_HILOGE(UPLOAD_MODULE_JS_NAPI, "Failed to create UploadTaskNapi");
        return nullptr;
    }
    proxy->env_ = env;
    proxy->napiUploadConfig_ = JSUtil::ParseUploadConfig(env, argv[parametersPosition]);
    if (proxy->napiUploadConfig_ == nullptr) {
        UPLOAD_HILOGE(UPLOAD_MODULE_JS_NAPI, "Initialize. ParseUploadConfig fail.");
        delete proxy;
        return nullptr;
    }
    if (proxy->napiUploadConfig_->protocolVersion == "L5") {
        AddCallbackToConfig(proxy->napiUploadConfig_, env, argv[parametersPosition], proxy);
    }
    proxy->napiUploadTask_ = std::make_shared<Upload::UploadTask>(proxy->napiUploadConfig_);
    {
        std::lock_guard<std::mutex> autoLock(mutex_);
        proxy->napiUploadTask_->taskId_ = taskId_++;
    }
    UploadTaskNapi::RegisterUploadTask(proxy);
    proxy->napiUploadTask_->SetUploadTaskRelease(proxy, proxy);
    proxy->napiUploadTask_->ExecuteTask();

    auto finalize = [](napi_env env, void *data, void *hint) {
        UPLOAD_HILOGE(UPLOAD_MODULE_JS_NAPI, "UploadTask. delete");
    };
    if (napi_wrap(env, self, proxy, finalize, nullptr, nullptr) != napi_ok) {
        return nullptr;
    }
    return self;
}

void UploadTaskNapi::OnUploadTaskRelease(void *arg)
{
    UploadTaskNapi *proxy = reinterpret_cast<UploadTaskNapi *>(arg);
    UPLOAD_HILOGE(UPLOAD_MODULE_JS_NAPI, "UploadTask. delete. start");
    if (proxy) {
        auto task = proxy->GetTaskPtr();
        do {
            usleep(UploadTaskTimeout);
        } while (task->IsRunning());
    }
    UploadTaskNapi::UnRegisterUploadTask(proxy);
}

void UploadTaskNapi::OnSystemSuccess(napi_env env, napi_ref ref, Upload::UploadResponse &response)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "OnSystemSuccess enter");
    uv_loop_s *loop_ = nullptr;
    napi_get_uv_event_loop(env, &loop_);
    if (loop_ == nullptr) {
        UPLOAD_HILOGE(UPLOAD_MODULE_JS_NAPI, "Failed to get uv event loop");
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Failed to create uv work");
        return;
    }
    SystemSuccessCallback *successCallback = new (std::nothrow)SystemSuccessCallback;
    if (successCallback == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Failed to create successCallback");
        delete work;
        return;
    }
    successCallback->env = env;
    successCallback->ref = ref;
    successCallback->response = response;
    work->data = (void *)successCallback;
    int ret = uv_queue_work(loop_, work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            SystemSuccessCallback *successCallback = reinterpret_cast<SystemSuccessCallback *>(work->data);
            if (successCallback != nullptr) {
                napi_value callback = nullptr;
                napi_value global = nullptr;
                napi_value result = nullptr;
                napi_value jsResponse = JSUtil::Convert2JSUploadResponse(successCallback->env,
                    successCallback->response);
                napi_value args[1] = { jsResponse };
                napi_get_reference_value(successCallback->env, successCallback->ref, &callback);
                napi_get_global(successCallback->env, &global);
                napi_call_function(successCallback->env, global, callback, 1, args, &result);
                delete successCallback;
                successCallback = nullptr;
            }
            delete work;
            work = nullptr;
        });
    if (ret != 0) {
        UPLOAD_HILOGE(UPLOAD_MODULE_JS_NAPI, "OnSystemSuccess. uv_queue_work Failed");
        delete successCallback;
        delete work;
    }
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "OnSystemSuccess end");
}

void UploadTaskNapi::OnSystemFail(napi_env env, napi_ref ref, std::string &data, int32_t &code)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "OnSystemFail enter");
    uv_loop_s *loop_ = nullptr;
    napi_get_uv_event_loop(env, &loop_);
    if (loop_ == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Failed to get uv event loop");
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Failed to create uv work");
        return;
    }
    SystemFailCallback *failCallback = new (std::nothrow) SystemFailCallback;
    if (failCallback == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Failed to create SystemFailCallback");
        delete work;
        return;
    }
    failCallback->data = data;
    failCallback->code = code;
    failCallback->env = env;
    failCallback->ref = ref;
    work->data = (void *)failCallback;
    int ret = uv_queue_work(loop_, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            SystemFailCallback *failCallback = reinterpret_cast<SystemFailCallback *>(work->data);
            if (failCallback != nullptr) {
                napi_value callback = nullptr;
                napi_value global = nullptr;
                napi_value result = nullptr;
                napi_value jsData = nullptr;
                napi_create_string_utf8(failCallback->env, failCallback->data.c_str(),
                    failCallback->data.size(), &jsData);
                napi_value jsCode = nullptr;
                napi_create_int32(failCallback->env, failCallback->code, &jsCode);
                napi_value args[2] = { jsData, jsCode };
                napi_get_reference_value(failCallback->env, failCallback->ref, &callback);
                napi_get_global(failCallback->env, &global);
                napi_call_function(failCallback->env, global, callback, sizeof(args) / sizeof(args[0]), args, &result);
                delete failCallback;
                failCallback = nullptr;
            }
            delete work;
            work = nullptr;
        });
    if (ret != 0) {
        delete failCallback;
        delete work;
    }
}

void UploadTaskNapi::OnSystemComplete(napi_env env, napi_ref ref)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "OnSystemComplete enter");
    uv_loop_s *loop_ = nullptr;
    napi_get_uv_event_loop(env, &loop_);
    if (loop_ == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Failed to get uv event loop");
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Failed to create uv work");
        return;
    }
    SystemCompleteCallback *completeCallback = new (std::nothrow)SystemCompleteCallback;
    if (completeCallback == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Failed to create SystemCompleteCallback");
        delete work;
        return;
    }
    completeCallback->env = env;
    completeCallback->ref = ref;
    work->data = (void *)completeCallback;
    int ret = uv_queue_work(loop_, work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            SystemCompleteCallback *completeCallback = reinterpret_cast<SystemCompleteCallback *>(work->data);
            if (completeCallback != nullptr) {
                napi_value callback = nullptr;
                napi_value global = nullptr;
                napi_value result = nullptr;

                napi_get_reference_value(completeCallback->env, completeCallback->ref, &callback);
                napi_get_global(completeCallback->env, &global);
                napi_call_function(completeCallback->env, global, callback, 0, nullptr, &result);
                delete completeCallback;
                completeCallback = nullptr;
            }
            delete work;
            work = nullptr;
        });
    if (ret != 0) {
        delete completeCallback;
        delete work;
    }
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "OnSystemComplete end");
}

bool UploadTaskNapi::JudgeFail(const IFailCallback *target)
{
    if ((this->onFail_ != nullptr && this->onFail_.get() == target) ||
        (this->offFail_ != nullptr && this->offFail_.get() == target)) {
        return true;
    }
    return false;
}
bool UploadTaskNapi::JudgeComplete(const ICompleteCallback *target)
{
    if ((this->onComplete_ != nullptr && this->onComplete_.get() == target) ||
       (this->offComplete_ != nullptr && this->offComplete_.get() == target)) {
        return true;
    }
    return false;
}
bool UploadTaskNapi::JudgeProgress(const IProgressCallback *target)
{
    if ((this->onProgress_ != nullptr && this->onProgress_.get() == target) ||
       (this->offProgress_ != nullptr && this->offProgress_.get() == target)) {
        return true;
    }
    return false;
}

void UploadTaskNapi::RegisterUploadTask(UploadTaskNapi *task)
{
    if (task) {
        std::lock_guard<std::mutex> lock(uploadTaskListLock_);
        for (size_t i = 0; i<uploadTaskList_.size(); i++) {
            if (uploadTaskList_[i] == task) {
                return;
            }
        }
        uploadTaskList_.push_back(task);
    }
}

void UploadTaskNapi::UnRegisterUploadTask(UploadTaskNapi *task)
{
    if (task) {
        std::lock_guard<std::mutex> lock(uploadTaskListLock_);
        for (size_t i = 0; i<uploadTaskList_.size(); i++) {
            if (uploadTaskList_[i] == task && uploadTaskList_[i]) {
                delete uploadTaskList_[i];
                uploadTaskList_[i] = nullptr;
                uploadTaskList_.erase(uploadTaskList_.begin() + i);
                return;
            }
        }
    }
}

size_t UploadTaskNapi::UploadTaskSize(void)
{
    return uploadTaskList_.size();
}
} // namespace  OHOS::Plugin::Request::UploadNapi