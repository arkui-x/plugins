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

#include "plugins/running_lock/android/java/jni/runninglock_receiver.h"

#include "inner_api/plugin_utils_inner.h"
#include "inner_api/plugin_utils_napi.h"
#include "log.h"
#include "plugin_utils.h"
#include "plugins/running_lock/js_runninglock.h"

namespace OHOS {
namespace Plugin {
void RunningLockReceiver::ReceiveCallBack(RunningLockAsyncCallbackInfo *ptr)
{
    LOGI("RunningLockReceiver::ReceiveCallBack ptr->env[%p] ptr->callbackRef[%p] ", ptr->env, ptr->callbackRef);
    PluginUtilsInner::RunTaskOnJS([ptr]() {
        AsyncCallbackInfo *pptr =
            new (std::nothrow) AsyncCallbackInfo {.env = ptr->env, .asyncWork = nullptr};
        pptr->callback[0] = ptr->callbackRef;
        pptr->deferred = ptr->deferred;
        pptr->jsdata = CreateInstanceForRunningLock(ptr->env, ptr);
        if (pptr->jsdata) {
            pptr->status = 0;
        } else {
            pptr->status = -1;
        }
        if (pptr->callback[0] == nullptr) {
            LOGI("PluginUtilsNApi::EmitPromiseWork called.");
            PluginUtilsNApi::EmitPromiseWork(pptr);
        } else {
            LOGI("PluginUtilsNApi::EmitAsyncCallbackWork called.");
            PluginUtilsNApi::EmitAsyncCallbackWork(pptr);
        }
    });
}
}  // namespace Plugin
}  // namespace OHOS