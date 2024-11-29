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

#include "napi_common_event.h"

#include <uv.h>

#include "errors.h"
#include "nlohmann/json.hpp"
namespace OHOS {
namespace Plugin {
static const int32_t CREATE_MAX_PARA = 2;
static const int32_t SUBSCRIBE_MAX_PARA = 2;
static const int32_t UNSUBSCRIBE_MAX_PARA = 2;
static const int32_t STR_MAX_SIZE = 256;
static const int32_t STR_DATA_MAX_SIZE = 64 * 1024;  // 64KB
static const int32_t ARGS_TWO_EVENT = 2;
static const int32_t ARGS_THREE = 3;
static const int32_t PARAM0_EVENT = 0;
static const int32_t PARAM1_EVENT = 1;
static const int8_t ERR_CES_FAILED = 1;
static const int32_t PUBLISH_MAX_PARA = 1;
static const int32_t ERROR_USERID = -1;
static const int32_t PUBLISH_MAX_PARA_BY_PUBLISHDATA = 2;
static const int32_t ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID = 401;
static const char* const DATA_PRORERTY = "data";
static const std::string COMMON_EVENT_PUBLISH_DATA = "data";
static thread_local napi_ref g_CommonEventSubscriber = nullptr;
static std::vector<std::shared_ptr<SubscriberInstance>> subscriberInstances;
static std::mutex subscriberInsMutex;

static const std::unordered_map<int32_t, std::string> ErrorCodeToMsg {
    { ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID, "Parameter error." },
};

napi_value NapiGetNull(napi_env env)
{
    napi_value result = nullptr;
    napi_get_null(env, &result);

    return result;
}

napi_value GetCallbackErrorValue(napi_env env, int8_t errorCode)
{
    napi_value result = NapiGetNull(env);
    napi_value eCode = NapiGetNull(env);
    if (errorCode == ERR_OK) {
        return result;
    }
    NAPI_CALL(env, napi_create_int32(env, errorCode, &eCode));
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_set_named_property(env, result, "code", eCode));

    auto iter = ErrorCodeToMsg.find(errorCode);
    std::string errMsg = iter != ErrorCodeToMsg.end() ? iter->second : "";
    napi_value message = nullptr;
    napi_create_string_utf8(env, errMsg.c_str(), NAPI_AUTO_LENGTH, &message);
    napi_set_named_property(env, result, "message", message);
    return result;
}

void SetPromise(const napi_env& env, const napi_deferred& deferred, const int8_t& errorCode, const napi_value& result)
{
    if (errorCode == 0) {
        napi_resolve_deferred(env, deferred, result);
    } else {
        napi_reject_deferred(env, deferred, GetCallbackErrorValue(env, errorCode));
    }
}

void SetCallback(const napi_env& env, const napi_ref& callbackIn, const int8_t& errorCode, const napi_value& result)
{
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value callback = nullptr;
    napi_value resultout = nullptr;
    napi_get_reference_value(env, callbackIn, &callback);

    napi_value results[ARGS_TWO_EVENT] = { nullptr };
    results[PARAM0_EVENT] = GetCallbackErrorValue(env, errorCode);
    results[PARAM1_EVENT] = result;

    NAPI_CALL_RETURN_VOID(
        env, napi_call_function(env, undefined, callback, ARGS_TWO_EVENT, &results[PARAM0_EVENT], &resultout));
}

void ReturnCallbackPromise(const napi_env& env, const CallbackPromiseInfo& info, const napi_value& result)
{
    if (info.isCallback) {
        SetCallback(env, info.callback, info.errorCode, result);
    } else {
        SetPromise(env, info.deferred, info.errorCode, result);
    }
}

napi_value ParseParametersByCreateSubscriber(
    const napi_env& env, const napi_value (&argv)[CREATE_MAX_PARA], const size_t& argc, napi_ref& callback)
{
    napi_valuetype valuetype;

    // argv[0]:CommonEventSubscribeInfo
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    if (valuetype != napi_object) {
        LOGE("Wrong argument type. object expected.");
        return nullptr;
    }

    // argv[1]:callback
    if (argc >= CREATE_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        if (valuetype != napi_function) {
            LOGE("Wrong argument type. Function expected.");
            return nullptr;
        }
        napi_create_reference(env, argv[1], 1, &callback);
    }

    return NapiGetNull(env);
}

void PaddingAsyncCallbackInfoCreateSubscriber(const napi_env& env, const size_t& argc,
    AsyncCallbackInfoCreate*& asyncCallbackInfo, const napi_ref& callback, napi_value& promise)
{
    if (argc >= CREATE_MAX_PARA) {
        asyncCallbackInfo->info.callback = callback;
        asyncCallbackInfo->info.isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        napi_create_promise(env, &deferred, &promise);
        asyncCallbackInfo->info.deferred = deferred;
        asyncCallbackInfo->info.isCallback = false;
    }
}

std::shared_ptr<SubscriberInstance> GetSubscriber(const napi_env& env, const napi_value& value)
{
    LOGI("Called.");
    SubscriberInstanceWrapper* wrapper = nullptr;
    napi_unwrap(env, value, (void**)&wrapper);
    if (wrapper == nullptr) {
        LOGW("Wrapper is nullptr.");
        return nullptr;
    }

    return wrapper->GetSubscriber();
}

napi_value GetSubscriberByUnsubscribe(
    const napi_env& env, const napi_value& value, std::shared_ptr<SubscriberInstance>& subscriber, bool& isFind)
{
    LOGI("Called.");
    isFind = false;
    subscriber = GetSubscriber(env, value);
    if (subscriber == nullptr) {
        LOGE("Subscriber is nullptr.");
        return NapiGetNull(env);
    }

    std::lock_guard<std::mutex> lock(subscriberInsMutex);
    for (auto subscriberInstance : subscriberInstances) {
        if (subscriberInstance.get() == subscriber.get()) {
            subscriber = subscriberInstance;
            isFind = true;
            break;
        }
    }

    return NapiGetNull(env);
}

napi_value ParseParametersByUnsubscribe(const napi_env& env, const size_t& argc,
    const napi_value (&argv)[UNSUBSCRIBE_MAX_PARA], std::shared_ptr<SubscriberInstance>& subscriber, napi_ref& callback)
{
    LOGI("Called.");
    napi_valuetype valuetype;
    napi_value result = nullptr;
    // argv[0]:subscriber
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    if (valuetype != napi_object) {
        LOGI("Wrong argument type for arg0. Subscribe expected.");
        return nullptr;
    }

    bool isFind = false;
    if (GetSubscriberByUnsubscribe(env, argv[0], subscriber, isFind) == nullptr) {
        return nullptr;
    }

    // argv[1]:callback
    if (argc >= UNSUBSCRIBE_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        if (valuetype != napi_function) {
            LOGI("Wrong argument type. Function expected.");
            return nullptr;
        }
        napi_create_reference(env, argv[1], 1, &callback);
    }

    napi_get_boolean(env, isFind, &result);

    return result;
}

SubscriberInstanceWrapper::SubscriberInstanceWrapper(const CommonEventSubscribeInfo& info)
{
    auto objectInfo = new (std::nothrow) SubscriberInstance(info);
    if (objectInfo == nullptr) {
        LOGE("Object info is nullptr.");
        return;
    }
    subscriber = std::shared_ptr<SubscriberInstance>(objectInfo);
}

std::shared_ptr<SubscriberInstance> SubscriberInstanceWrapper::GetSubscriber()
{
    return subscriber;
}

napi_value SetCommonEventData(const CommonEventDataWorker* commonEventDataWorkerData, napi_value& result)
{
    if (commonEventDataWorkerData == nullptr) {
        return nullptr;
    }

    napi_value value = nullptr;

    // event
    napi_create_string_utf8(
        commonEventDataWorkerData->env, commonEventDataWorkerData->event.c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(commonEventDataWorkerData->env, result, "event", value);

    // bundleName
    napi_create_string_utf8(
        commonEventDataWorkerData->env, commonEventDataWorkerData->bundleName.c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(commonEventDataWorkerData->env, result, "bundleName", value);

    // code
    napi_create_int32(commonEventDataWorkerData->env, commonEventDataWorkerData->code, &value);
    napi_set_named_property(commonEventDataWorkerData->env, result, "code", value);

    // data
    napi_create_string_utf8(
        commonEventDataWorkerData->env, commonEventDataWorkerData->data.c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(commonEventDataWorkerData->env, result, "data", value);
    return NapiGetNull(commonEventDataWorkerData->env);
}

napi_value ParseParametersBySubscribe(const napi_env& env, const napi_value (&argv)[SUBSCRIBE_MAX_PARA],
    std::shared_ptr<SubscriberInstance>& subscriber, napi_ref& callback)
{
    LOGI("Called.");
    napi_valuetype valuetype;
    // argv[0]:subscriber
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    // NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type for arg0. Subscribe expected.");
    if (valuetype != napi_object) {
        LOGE("Wrong argument type for arg0. Subscribe expected.");
        return nullptr;
    }
    subscriber = GetSubscriber(env, argv[0]);
    if (subscriber == nullptr) {
        LOGE("subscriber is nullptr.");
        return nullptr;
    }

    // argv[1]:callback
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
    if (valuetype != napi_function) {
        LOGE("Wrong argument type. Function expected.");
        return nullptr;
    }
    napi_create_reference(env, argv[1], 1, &callback);

    return NapiGetNull(env);
}

void UvQueueWorkOnReceiveEvent(uv_work_t* work, int status)
{
    if (work == nullptr || work->data == nullptr) {
        return;
    }
    CommonEventDataWorker* commonEventDataWorker = reinterpret_cast<CommonEventDataWorker*>(work->data);

    napi_handle_scope scope;
    napi_open_handle_scope(commonEventDataWorker->env, &scope);

    napi_value result = nullptr;
    napi_create_object(commonEventDataWorker->env, &result);
    if (SetCommonEventData(commonEventDataWorker, result) == nullptr) {
        delete work;
        work = nullptr;
        delete commonEventDataWorker;
        commonEventDataWorker = nullptr;
        return;
    }

    napi_value undefined = nullptr;
    napi_get_undefined(commonEventDataWorker->env, &undefined);

    napi_value callback = nullptr;
    napi_value resultout = nullptr;
    napi_get_reference_value(commonEventDataWorker->env, commonEventDataWorker->ref, &callback);

    napi_value results[ARGS_TWO_EVENT] = { nullptr };
    results[PARAM0_EVENT] = GetCallbackErrorValue(commonEventDataWorker->env, 0);
    results[PARAM1_EVENT] = result;
    napi_call_function(
        commonEventDataWorker->env, undefined, callback, ARGS_TWO_EVENT, &results[PARAM0_EVENT], &resultout);

    napi_close_handle_scope(commonEventDataWorker->env, scope);

    delete commonEventDataWorker;
    commonEventDataWorker = nullptr;
    delete work;
}
SubscriberInstance::SubscriberInstance(const CommonEventSubscribeInfo& subscribeInfo)
    : CommonEventSubscriber(subscribeInfo)
{}

SubscriberInstance::~SubscriberInstance() {}

void SubscriberInstance::SetEnv(const napi_env& env)
{
    env_ = env;
}

void SubscriberInstance::SetCallbackRef(const napi_ref& ref)
{
    ref_ = ref;
}

napi_value ParseParametersByGetSubscribeInfo(
    const napi_env& env, const size_t& argc, const napi_value (&argv)[1], napi_ref& callback)
{
    napi_valuetype valuetype;

    // argv[0]:callback
    if (argc >= 1) {
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        if (valuetype != napi_function) {
            LOGE("Wrong argument type. Function expected.");
            return nullptr;
        }

        napi_create_reference(env, argv[0], 1, &callback);
    }

    return NapiGetNull(env);
}

void PaddingAsyncCallbackInfoGetSubscribeInfo(const napi_env& env, const size_t& argc,
    AsyncCallbackInfoSubscribeInfo*& asyncCallbackInfo, const napi_ref& callback, napi_value& promise)
{
    LOGI("Called.");
    if (argc >= 1) {
        asyncCallbackInfo->info.callback = callback;
        asyncCallbackInfo->info.isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->info.deferred = deferred;
        asyncCallbackInfo->info.isCallback = false;
    }
}

void PaddingNapiCreateAsyncWorkCallbackInfo(AsyncCallbackInfoSubscribeInfo*& asyncCallbackInfo)
{
    LOGI("Called.");
    asyncCallbackInfo->events = asyncCallbackInfo->subscriber->GetSubscribeInfo().GetEvents();
    asyncCallbackInfo->permission = "";
    asyncCallbackInfo->deviceId = "";
    asyncCallbackInfo->userId = ERROR_USERID;
    asyncCallbackInfo->priority = 0;
}

void SetEventsResult(const napi_env& env, const std::vector<std::string>& events, napi_value& commonEventSubscribeInfo)
{
    LOGI("Called.");
    napi_value value = nullptr;
    if (events.size() > 0) {
        napi_value nEvents = nullptr;
        if (napi_create_array(env, &nEvents) != napi_ok) {
            return;
        }
        size_t index = 0;
        for (auto event : events) {
            napi_create_string_utf8(env, event.c_str(), NAPI_AUTO_LENGTH, &value);
            napi_set_element(env, nEvents, index, value);
            index++;
        }

        napi_set_named_property(env, commonEventSubscribeInfo, "events", nEvents);
    }
}

void SetNapiResult(const napi_env& env, const AsyncCallbackInfoSubscribeInfo* asyncCallbackInfo, napi_value& result)
{
    LOGI("Called.");
    SetEventsResult(env, asyncCallbackInfo->events, result);
}

napi_value GetSubscribeInfo(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    napi_ref callback = nullptr;
    if (ParseParametersByGetSubscribeInfo(env, argc, argv, callback) == nullptr) {
        return NapiGetNull(env);
    }
    AsyncCallbackInfoSubscribeInfo* asyncCallbackInfo =
        new (std::nothrow) AsyncCallbackInfoSubscribeInfo { .env = env, .asyncWork = nullptr, .userId = ERROR_USERID };
    if (asyncCallbackInfo == nullptr) {
        return NapiGetNull(env);
    }
    asyncCallbackInfo->subscriber = GetSubscriber(env, thisVar);
    if (asyncCallbackInfo->subscriber == nullptr) {
        return NapiGetNull(env);
    }
    napi_value promise = nullptr;
    PaddingAsyncCallbackInfoGetSubscribeInfo(env, argc, asyncCallbackInfo, callback, promise);
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "getSubscribeInfo", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        env, nullptr, resourceName,
        [](napi_env env, void* data) {
            AsyncCallbackInfoSubscribeInfo* asyncCallbackInfo = (AsyncCallbackInfoSubscribeInfo*)data;
            PaddingNapiCreateAsyncWorkCallbackInfo(asyncCallbackInfo);
        },
        [](napi_env env, napi_status status, void* data) {
            AsyncCallbackInfoSubscribeInfo* asyncCallbackInfo = (AsyncCallbackInfoSubscribeInfo*)data;
            if (asyncCallbackInfo) {
                napi_value result = nullptr;
                napi_create_object(env, &result);
                SetNapiResult(env, asyncCallbackInfo, result);
                ReturnCallbackPromise(env, asyncCallbackInfo->info, result);
                if (asyncCallbackInfo->info.callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->info.callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        },
        (void*)asyncCallbackInfo, &asyncCallbackInfo->asyncWork);
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    if (asyncCallbackInfo->info.isCallback) {
        return NapiGetNull(env);
    } else {
        return promise;
    }
}

void SubscriberInstance::OnReceiveEvent(const CommonEventData& data)
{
    uv_loop_s* loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        LOGE("Loop instance is nullptr.");
        return;
    }
    uv_work_t* work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("Work is null.");
        return;
    }
    CommonEventDataWorker* commonEventDataWorker = new (std::nothrow) CommonEventDataWorker();
    if (commonEventDataWorker == nullptr) {
        LOGE("Common event data worker is null.");
        delete work;
        work = nullptr;
        return;
    }
    commonEventDataWorker->event = data.GetEvent();
    commonEventDataWorker->code = data.GetCode();
    commonEventDataWorker->data = data.GetData();
    commonEventDataWorker->env = env_;
    commonEventDataWorker->ref = ref_;
    commonEventDataWorker->bundleName = data.GetBunduleName();

    work->data = (void*)commonEventDataWorker;
    uv_queue_work(
        loop, work, [](uv_work_t* work) {}, UvQueueWorkOnReceiveEvent);
}

napi_value GetEventsByCreateSubscriber(const napi_env& env, const napi_value& argv, std::vector<std::string>& events)
{
    napi_valuetype valuetype;
    bool hasProperty = false;
    bool isArray = false;
    napi_value eventsNapi = nullptr;
    size_t strLen = 0;
    uint32_t length = 0;
    // events
    NAPI_CALL(env, napi_has_named_property(env, argv, "events", &hasProperty));
    if (!hasProperty) {
        LOGE("Property events expected.");
        return nullptr;
    }
    napi_get_named_property(env, argv, "events", &eventsNapi);
    napi_is_array(env, eventsNapi, &isArray);
    if (!isArray) {
        LOGE("Wrong argument type. Array expected.");
        return nullptr;
    }
    napi_get_array_length(env, eventsNapi, &length);
    if (length <= 0) {
        LOGE("The array is empty.");
        return nullptr;
    }
    for (size_t i = 0; i < length; i++) {
        napi_value event = nullptr;
        napi_get_element(env, eventsNapi, i, &event);
        NAPI_CALL(env, napi_typeof(env, event, &valuetype));
        if (valuetype != napi_string) {
            LOGE("Wrong argument type. String expected.");
            return nullptr;
        }
        char str[STR_MAX_SIZE] = { 0 };
        NAPI_CALL(env, napi_get_value_string_utf8(env, event, str, STR_MAX_SIZE - 1, &strLen));
        LOGI("Event is %{public}s", str);
        events.emplace_back(str);
    }

    return NapiGetNull(env);
}

napi_value ParseParametersConstructor(
    const napi_env& env, const napi_callback_info& info, napi_value& thisVar, CommonEventSubscribeInfo& params)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    if (argc < 1) {
        LOGE("Wrong number of arguments.");
        return nullptr;
    }

    // events: Array<string>
    std::vector<std::string> events;
    if (!GetEventsByCreateSubscriber(env, argv[0], events)) {
        return nullptr;
    }
    CommonEventSubscribeInfo subscribeInfo(events);
    params = subscribeInfo;
    return NapiGetNull(env);
}

napi_value CommonEventSubscriberConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    CommonEventSubscribeInfo subscribeInfo;
    if (!ParseParametersConstructor(env, info, thisVar, subscribeInfo)) {
        return NapiGetNull(env);
    }
    auto wrapper = new (std::nothrow) SubscriberInstanceWrapper(subscribeInfo);
    if (wrapper == nullptr) {
        LOGE("Wrapper is nullptr.");
        return NapiGetNull(env);
    }

    napi_wrap(
        env, thisVar, wrapper,
        [](napi_env env, void* data, void* hint) {
            auto* wrapper = reinterpret_cast<SubscriberInstanceWrapper*>(data);
            std::lock_guard<std::mutex> lock(subscriberInsMutex);
            for (auto it = subscriberInstances.begin(); it != subscriberInstances.end(); it++) {
                if (it->get() == wrapper->GetSubscriber().get()) {
                    wrapper->GetSubscriber()->SetCallbackRef(nullptr);
                    CommonEventManager::GetInstance().UnSubscribeCommonEvent(*it, nullptr);
                    subscriberInstances.erase(it);
                    break;
                }
            }
            delete wrapper;
            wrapper = nullptr;
        },
        nullptr, nullptr);
    return thisVar;
}

void NapiThrow(napi_env env, int32_t errCode)
{
    napi_value code = nullptr;
    napi_create_int32(env, errCode, &code);

    auto iter = ErrorCodeToMsg.find(errCode);
    std::string errMsg = iter != ErrorCodeToMsg.end() ? iter->second : "";
    napi_value message = nullptr;
    napi_create_string_utf8(env, errMsg.c_str(), NAPI_AUTO_LENGTH, &message);

    napi_value error = nullptr;
    napi_create_error(env, nullptr, message, &error);
    napi_set_named_property(env, error, "code", code);
    napi_throw(env, error);
}

napi_value CreateSubscriber(napi_env env, napi_callback_info info)
{
    size_t argc = CREATE_MAX_PARA;
    napi_value argv[CREATE_MAX_PARA] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc < 1) {
        return NapiGetNull(env);
    }
    napi_ref callback = nullptr;
    if (ParseParametersByCreateSubscriber(env, argv, argc, callback) == nullptr) {
        NapiThrow(env, ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID);
        return NapiGetNull(env);
    }
    AsyncCallbackInfoCreate* asyncCallbackInfo =
        new (std::nothrow) AsyncCallbackInfoCreate { .env = env, .asyncWork = nullptr, .subscribeInfo = nullptr };
    if (asyncCallbackInfo == nullptr) {
        return NapiGetNull(env);
    }
    napi_value promise = nullptr;
    PaddingAsyncCallbackInfoCreateSubscriber(env, argc, asyncCallbackInfo, callback, promise);
    napi_create_reference(env, argv[0], 1, &asyncCallbackInfo->subscribeInfo);
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "CreateSubscriber", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        env, nullptr, resourceName,
        [](napi_env env, void* data) { LOGI("CreateSubscriber napi_create_async_work start"); },
        [](napi_env env, napi_status status, void* data) {
            AsyncCallbackInfoCreate* asyncCallbackInfo = (AsyncCallbackInfoCreate*)data;
            if (asyncCallbackInfo) {
                napi_value constructor = nullptr;
                napi_value subscribeInfoRefValue = nullptr;
                napi_get_reference_value(env, asyncCallbackInfo->subscribeInfo, &subscribeInfoRefValue);
                napi_get_reference_value(env, g_CommonEventSubscriber, &constructor);
                napi_new_instance(env, constructor, 1, &subscribeInfoRefValue, &asyncCallbackInfo->result);
                if (asyncCallbackInfo->result == nullptr) {
                    asyncCallbackInfo->info.errorCode = ERR_CES_FAILED;
                }
                ReturnCallbackPromise(env, asyncCallbackInfo->info, asyncCallbackInfo->result);
                if (asyncCallbackInfo->info.callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->info.callback);
                }
                if (asyncCallbackInfo->subscribeInfo != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->subscribeInfo);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        },
        (void*)asyncCallbackInfo, &asyncCallbackInfo->asyncWork);
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    if (asyncCallbackInfo->info.isCallback) {
        return NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value GetDataByPublish(const napi_env& env, const napi_value& value, std::string& data)
{
    napi_valuetype valuetype;
    napi_value result = nullptr;
    char str[STR_DATA_MAX_SIZE] = {0};
    bool hasProperty = false;
    size_t strLen = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, DATA_PRORERTY, &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, DATA_PRORERTY, &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_string) {
            LOGE("Parameter type error. String expected.");
            return nullptr;
        }
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_DATA_MAX_SIZE, &strLen));

        if (strLen > STR_DATA_MAX_SIZE - 1) {
            LOGE("Data exceed limit.");
            return nullptr;
        }
        nlohmann::json jsonValue;
        jsonValue[COMMON_EVENT_PUBLISH_DATA] = str;
        data = jsonValue.dump();
    }

    return NapiGetNull(env);
}

napi_value ParseParametersByPublish(const napi_env& env, const napi_value (&argv)[ARGS_THREE], const size_t& argc,
    std::string& event, std::string& commonEventPublishData, napi_ref& callback)
{
    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    if (valuetype != napi_string) {
        LOGE("Parameter type error. String expected.");
        return nullptr;
    }
    char str[STR_MAX_SIZE] = { 0 };
    size_t strLen = 0;
    napi_get_value_string_utf8(env, argv[0], str, STR_MAX_SIZE - 1, &strLen);
    event = str;

    if (argc == ARGS_THREE) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        if (valuetype != napi_object) {
            LOGE("Wrong argument type. Object expected.");
            return nullptr;
        }

        if (GetDataByPublish(env, argv[1], commonEventPublishData) == nullptr) {
            return nullptr;
        }

        NAPI_CALL(env, napi_typeof(env, argv[PUBLISH_MAX_PARA_BY_PUBLISHDATA], &valuetype));
        if (valuetype != napi_function) {
            LOGE("Wrong argument type. Function expected.");
            return nullptr;
        }
        napi_create_reference(env, argv[PUBLISH_MAX_PARA_BY_PUBLISHDATA], 1, &callback);
    } else {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        if (valuetype != napi_function) {
            LOGE("Wrong argument type. Function expected.");
            return nullptr;
        }
        napi_create_reference(env, argv[1], 1, &callback);
    }

    return NapiGetNull(env);
}

void CreateAsyncCallbackWork(napi_env env, AsyncCallbackInfo* asyncCallbackInfo)
{
    if (asyncCallbackInfo == nullptr) {
        LOGE("Async callback info is nullptr.");
        return;
    }
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "CreateAsyncCallbackWork", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        env, nullptr, resourceName, [](napi_env env, void* data) { LOGD("Napi create async work start."); },
        [](napi_env env, napi_status status, void* data) {
            LOGD("Napi async work complete.");
            AsyncCallbackInfo* asyncCallbackInfo = (AsyncCallbackInfo*)data;
            if (asyncCallbackInfo != nullptr) {
                if (asyncCallbackInfo->status < 0) {
                    asyncCallbackInfo->status = ERR_INVALID_VALUE;
                } else {
                    asyncCallbackInfo->status = ERR_OK;
                }
                SetCallback(env, asyncCallbackInfo->callback[0], asyncCallbackInfo->status, NapiGetNull(env));
                if (asyncCallbackInfo->callback != nullptr) {
                    LOGD("Delete napi async callback reference.");
                    napi_delete_reference(env, asyncCallbackInfo->callback[0]);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        },
        (void*)asyncCallbackInfo, &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
}

napi_value PublishCommonEvent(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_THREE;
    napi_value argv[ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc >= PUBLISH_MAX_PARA, "Wrong number of arguments");

    std::string event;
    std::string commonEventPublishData = "";
    napi_ref callback;
    if (ParseParametersByPublish(env, argv, argc, event, commonEventPublishData, callback) == nullptr) {
        NapiThrow(env, ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID);
        return NapiGetNull(env);
    }

    AsyncCallbackInfo* asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo { .env = env, .asyncWork = nullptr };
    asyncCallbackInfo->callback[0] = callback;
    CommonEventManager::GetInstance().PublishCommonEvent(event, commonEventPublishData, asyncCallbackInfo);
    CreateAsyncCallbackWork(env, asyncCallbackInfo);
    return NapiGetNull(env);
}

napi_value SubscribeCommonEvent(napi_env env, napi_callback_info info)
{
    size_t argc = SUBSCRIBE_MAX_PARA;
    napi_value argv[SUBSCRIBE_MAX_PARA] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc >= SUBSCRIBE_MAX_PARA, "Wrong number of arguments");

    napi_ref callback = nullptr;
    std::shared_ptr<SubscriberInstance> subscriber = nullptr;

    if (ParseParametersBySubscribe(env, argv, subscriber, callback) == nullptr) {
        NapiThrow(env, ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID);
        return NapiGetNull(env);
    }

    AsyncCallbackInfo* asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo { .env = env, .asyncWork = nullptr };
    if (asyncCallbackInfo == nullptr) {
        LOGE("Async callback is null");
        return NapiGetNull(env);
    }
    asyncCallbackInfo->callback[0] = callback;

    // Asynchronous function call
    subscriber->SetEnv(env);
    subscriber->SetCallbackRef(callback);
    {
        std::lock_guard<std::mutex> lock(subscriberInsMutex);
        subscriberInstances.push_back(subscriber);
    }
    CommonEventManager::GetInstance().SubscribeCommonEvent(subscriber);
    return NapiGetNull(env);
}

napi_value UnSubscribeCommonEvent(napi_env env, napi_callback_info info)
{
    // Argument parsing
    size_t argc = UNSUBSCRIBE_MAX_PARA;
    napi_value argv[UNSUBSCRIBE_MAX_PARA] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_ref callback = nullptr;
    std::shared_ptr<SubscriberInstance> subscriber = nullptr;
    napi_value result = nullptr;
    result = ParseParametersByUnsubscribe(env, argc, argv, subscriber, callback);
    if (result == nullptr) {
        NapiThrow(env, ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID);
        return NapiGetNull(env);
    }
    bool isFind = false;
    napi_get_value_bool(env, result, &isFind);
    if (!isFind) {
        LOGI("Unsubscribe failed. The current subscriber does not exist");
        return NapiGetNull(env);
    }

    AsyncCallbackInfo* asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo();
    if (asyncCallbackInfo == nullptr) {
        LOGE("Async callback is nullptr.");
        return NapiGetNull(env);
    }
    asyncCallbackInfo->env = env;
    if (argc >= UNSUBSCRIBE_MAX_PARA) {
        asyncCallbackInfo->callback[0] = callback;
    }
    CommonEventManager::GetInstance().UnSubscribeCommonEvent(subscriber, asyncCallbackInfo);
    CreateAsyncCallbackWork(env, asyncCallbackInfo);
    return NapiGetNull(env);
}

napi_value CommonEventSubscriberInit(napi_env env, napi_value exports)
{
    napi_value constructor = nullptr;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getSubscribeInfo", GetSubscribeInfo),
    };

    NAPI_CALL(env, napi_define_class(env, "commonEventSubscriber", NAPI_AUTO_LENGTH, CommonEventSubscriberConstructor,
                       nullptr, sizeof(properties) / sizeof(*properties), properties, &constructor));

    napi_create_reference(env, constructor, 1, &g_CommonEventSubscriber);
    if (g_CommonEventSubscriber == nullptr) {
        LOGI("g_CommonEventSubscriber is nullptr.");
    }
    napi_set_named_property(env, exports, "commonEventSubscriber", constructor);
    if (g_CommonEventSubscriber == nullptr) {
        LOGI("g_CommonEventSubscriber is nullptr.");
    }
    return exports;
}

napi_value CommonEventInit(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("createSubscriber", CreateSubscriber),
        DECLARE_NAPI_FUNCTION("publish", PublishCommonEvent),
        DECLARE_NAPI_FUNCTION("subscribe", SubscribeCommonEvent),
        DECLARE_NAPI_FUNCTION("unsubscribe", UnSubscribeCommonEvent),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}
} // namespace Plugin
} // namespace OHOS