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

#include "plugins/interfaces/native/plugin_c_utils.h"

#include <cstddef>

#include "base/thread/task_executor.h"
#include "frameworks/core/common/container.h"
#include "plugins/interfaces/native/plugin_utils.h"

#ifdef ANDROID_PLATFORM
#include "adapter/android/entrance/java/jni/jni_environment.h"

JNIEnv* OH_Plugin_GetJniEnv()
{
    return OHOS::Ace::Platform::JniEnvironment::GetInstance().GetJniEnv(nullptr, false).get();
}

void OH_Plugin_RegisterPlugin(bool (*func)(void*), const char* name)
{
    std::string className = name;
    OHOS::Plugin::PluginUtils::RegisterPlugin(func, className);
}
#endif

void OH_Plugin_RunTaskOnPlatform(OH_Plugin_Task task)
{
    auto taskExecutor = OHOS::Ace::Container::CurrentTaskExecutor();
    if (taskExecutor) {
        taskExecutor->PostTask([task]() { task(); }, OHOS::Ace::TaskExecutor::TaskType::PLATFORM);
    }
}

OH_Plugin_ReportEventType OH_Plugin_IsMatchType(napi_value value, napi_valuetype type, napi_env env)
{
    napi_valuetype paramType;
    napi_typeof(env, value, &paramType);
    if (paramType == type) {
        return TYPE_CHECK_SUCCESS;
    }
    return TYPE_CHECK_FAIL;
}

napi_value OH_Plugin_GetNapiInt32(int32_t number, napi_env env)
{
    napi_value value;
    napi_create_int32(env, number, &value);
    return value;
}

napi_value OH_Plugin_GetNamedProperty(napi_value jsonObject, std::string name, napi_env env)
{
    napi_value value;
    napi_get_named_property(env, jsonObject, name.c_str(), &value);
    return value;
}

int32_t OH_Plugin_GetCInt32(napi_value value, napi_env env)
{
    int32_t num;
    napi_get_value_int32(env, value, &num);
    return num;
}

size_t OH_Plugin_GetCString(napi_value value, napi_env env, char* buffer, size_t bufSize)
{
    size_t valueLength;
    napi_get_value_string_utf8(env, value, buffer, bufSize, &valueLength);
    return valueLength;
}

int64_t OH_Plugin_GetCInt64(napi_value value, napi_env env)
{
    int64_t num;
    napi_get_value_int64(env, value, &num);
    return num;
}

void OH_Plugin_EmitAsyncCallbackWork(OH_Plugin_AsyncCallbackInfo* asyncCallbackInfo)
{
    constexpr int ARGCOUNT = 2;
    if (asyncCallbackInfo == nullptr) {
        return;
    }
    napi_value resourceName;
    napi_create_string_latin1(asyncCallbackInfo->env, "AsyncCallback", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        asyncCallbackInfo->env, nullptr, resourceName, [](napi_env env, void* data) {},
        [](napi_env env, napi_status status, void* data) {
            OH_Plugin_AsyncCallbackInfo* asyncCallbackInfo = (OH_Plugin_AsyncCallbackInfo*)data;
            napi_value callback;
            napi_get_reference_value(env, asyncCallbackInfo->callback[0], &callback);
            napi_value result[ARGCOUNT] = {nullptr};
            napi_value callResult = nullptr;
            if (asyncCallbackInfo->status < 0) {
                napi_value code = nullptr;
                napi_value message = nullptr;
                napi_create_string_utf8(env, "-1", NAPI_AUTO_LENGTH, &code);
                napi_create_string_utf8(env, "failed", NAPI_AUTO_LENGTH, &message);
                napi_value result_err = nullptr;
                napi_create_error(env, code, message, &result_err);
                result[0] = result_err;
            } else {
                napi_value code = nullptr;
                napi_value message = nullptr;
                napi_create_string_utf8(env, "0", NAPI_AUTO_LENGTH, &code);
                napi_create_string_utf8(env, "success", NAPI_AUTO_LENGTH, &message);
                napi_value result_err = nullptr;
                napi_create_error(env, code, message, &result_err);
                result[0] = result_err;
                result[1] = asyncCallbackInfo->jsdata;
            }
            napi_call_function(env, nullptr, callback, ARGCOUNT, &result[0], &callResult);
            napi_delete_reference(env, asyncCallbackInfo->callback[0]);
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        asyncCallbackInfo, &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(asyncCallbackInfo->env, asyncCallbackInfo->asyncWork);
}

void OH_Plugin_EmitPromiseWork(OH_Plugin_AsyncCallbackInfo* asyncCallbackInfo)
{
    if (asyncCallbackInfo == nullptr) {
        return;
    }
    napi_value resourceName;
    napi_create_string_latin1(asyncCallbackInfo->env, "Promise", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        asyncCallbackInfo->env, nullptr, resourceName, [](napi_env env, void* data) {},
        [](napi_env env, napi_status status, void* data) {
            OH_Plugin_AsyncCallbackInfo* asyncCallbackInfo = (OH_Plugin_AsyncCallbackInfo*)data;
            napi_value result = nullptr;
            napi_get_undefined(env, &result);
            if (asyncCallbackInfo->status == 0) {
                napi_resolve_deferred(env, asyncCallbackInfo->deferred, asyncCallbackInfo->jsdata);
            } else {
                napi_value code = nullptr;
                napi_value message = nullptr;
                napi_create_string_utf8(env, "-1", NAPI_AUTO_LENGTH, &code);
                napi_create_string_utf8(env, "failed", NAPI_AUTO_LENGTH, &message);
                napi_create_error(env, code, message, &result);
                napi_reject_deferred(env, asyncCallbackInfo->deferred, result);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void*)asyncCallbackInfo, &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(asyncCallbackInfo->env, asyncCallbackInfo->asyncWork);
}
