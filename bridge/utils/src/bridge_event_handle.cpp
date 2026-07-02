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

#include "bridge_event_handle.h"

#include "log.h"

namespace OHOS::Plugin::Bridge {
std::shared_ptr<BridgeEventHandle> BridgeEventHandle::instance_ = nullptr;
std::mutex BridgeEventHandle::mutex_;
std::mutex BridgeEventHandle::envMutex_;
std::map<napi_env, std::shared_ptr<BridgeEventHandle>> BridgeEventHandle::envInstances_;

BridgeEventHandle::BridgeEventHandle()
{
    auto eventRunner = AppExecFwk::EventRunner::Current();
    mainEventHandler_ = std::make_shared<AppExecFwk::EventHandler>(eventRunner);
    auto bridgeEventRunner = AppExecFwk::EventRunner::Create("bridgeEventRunner");
    bridgeEventHandler_ = std::make_shared<AppExecFwk::EventHandler>(bridgeEventRunner);
}

BridgeEventHandle::BridgeEventHandle(napi_env env) : env_(env)
{
    napi_value resourceName = nullptr;
    napi_status status = napi_create_string_utf8(env, "BridgeTsfn", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        LOGE("BridgeEventHandle(env): create resourceName failed");
        return;
    }
    status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 0, 1, nullptr,
        Finalize, this, CallJs, &tsfn_);
    if (status != napi_ok) {
        LOGE("BridgeEventHandle(env): create tsfn failed");
        tsfn_ = nullptr;
        return;
    }
    napi_add_env_cleanup_hook(env, EnvCleanup, this);
}

BridgeEventHandle::~BridgeEventHandle()
{
    if (tsfn_ != nullptr) {
        napi_release_threadsafe_function(tsfn_, napi_tsfn_release);
        tsfn_ = nullptr;
    }
}

std::shared_ptr<BridgeEventHandle> BridgeEventHandle::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<BridgeEventHandle>();
        }
    }

    return instance_;
}

std::shared_ptr<BridgeEventHandle> BridgeEventHandle::GetInstance(napi_env env)
{
    if (env == nullptr) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(envMutex_);
    auto it = envInstances_.find(env);
    if (it != envInstances_.end()) {
        return it->second;
    }
    return nullptr;
}

void BridgeEventHandle::CreateInstance(napi_env env)
{
    if (env == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(envMutex_);
    auto it = envInstances_.find(env);
    if (it != envInstances_.end()) {
        return;
    }
    auto sp = std::shared_ptr<BridgeEventHandle>(new BridgeEventHandle(env));
    envInstances_[env] = sp;
}

void BridgeEventHandle::RunTaskOnBridgeThread(const Task& task)
{
    if (task == nullptr) {
        return;
    }
    if (bridgeEventHandler_ != nullptr) {
        bridgeEventHandler_->PostTask(task);
    } else {
        LOGE("BridgeEventHandle bridgeEventHandler is nullptr");
    }
}

void BridgeEventHandle::RunTaskOnMainThread(const Task& task)
{
    if (task == nullptr) {
        return;
    }
    if (mainEventHandler_ != nullptr) {
        mainEventHandler_->PostTask(task);
    } else {
        LOGE("BridgeEventHandle mainEventHandler is nullptr");
    }
}

void BridgeEventHandle::RunTaskOnJsThread(const Task& task)
{
    if (task == nullptr) {
        return;
    }
    if (tsfn_ == nullptr) {
        LOGE("RunTaskOnJsThread: tsfn is null, callback dropped");
        return;
    }
    Task* p = new Task(task);
    napi_status status = napi_call_threadsafe_function(tsfn_, p, napi_tsfn_nonblocking);
    if (status != napi_ok) {
        delete p;
        LOGE("RunTaskOnJsThread: call tsfn failed");
    }
}

void BridgeEventHandle::CallJs(napi_env env, napi_value jsCallback, void* context, void* data)
{
    (void)jsCallback;
    Task* p = static_cast<Task*>(data);
    if (p == nullptr) {
        return;
    }
    if (*p) {
        (*p)();
    }
    delete p;
}

void BridgeEventHandle::Finalize(napi_env env, void* data, void* hint)
{
    (void)env;
    (void)data;
    (void)hint;
}

void BridgeEventHandle::EnvCleanup(void* data)
{
    auto* self = static_cast<BridgeEventHandle*>(data);
    if (self == nullptr) {
        return;
    }
    if (self->tsfn_ != nullptr) {
        napi_release_threadsafe_function(self->tsfn_, napi_tsfn_release);
        self->tsfn_ = nullptr;
    }
    std::lock_guard<std::mutex> lock(envMutex_);
    envInstances_.erase(self->env_);
}
} // namespace OHOS::Plugin::Bridge
