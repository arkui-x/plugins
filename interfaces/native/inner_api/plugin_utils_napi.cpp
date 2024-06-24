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

#include "plugins/interfaces/native/inner_api/plugin_utils_napi.h"

#include "securec.h"

namespace OHOS::Plugin {
void PluginUtilsNApi::DefineProperties(
    napi_env env, napi_value object, const std::initializer_list<napi_property_descriptor>& properties)
{
    napi_property_descriptor descriptors[properties.size()];
    std::copy(properties.begin(), properties.end(), descriptors);

    (void)napi_define_properties(env, object, properties.size(), descriptors);
}

void PluginUtilsNApi::DefineClass(napi_env env, napi_value exports,
    const std::initializer_list<napi_property_descriptor>& properties, const std::string& className)
{
    auto constructor = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value thisVal = nullptr;
        NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVal, nullptr));

        return thisVal;
    };

    napi_value jsConstructor = nullptr;

    napi_property_descriptor descriptors[properties.size()];
    std::copy(properties.begin(), properties.end(), descriptors);

    NAPI_CALL_RETURN_VOID(env, napi_define_class(env, className.c_str(), NAPI_AUTO_LENGTH, constructor, nullptr,
                                   properties.size(), descriptors, &jsConstructor));

    SetNamedProperty(env, exports, className, jsConstructor);
}

void PluginUtilsNApi::SetNamedProperty(
    napi_env env, napi_value object, const std::string& propertyName, napi_value value)
{
    if (GetValueType(env, object) != napi_object) {
        return;
    }

    napi_set_named_property(env, object, propertyName.c_str(), value);
}

napi_value PluginUtilsNApi::GetNamedProperty(napi_env env, napi_value object, const std::string& propertyName)
{
    if (GetValueType(env, object) != napi_object) {
        return CreateUndefined(env);
    }

    napi_value value = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, object, propertyName.c_str(), &value));
    return value;
}

bool PluginUtilsNApi::HasNamedProperty(napi_env env, napi_value object, const std::string& propertyName)
{
    if (GetValueType(env, object) != napi_object) {
        return false;
    }

    bool hasProperty = false;
    NAPI_CALL_BASE(env, napi_has_named_property(env, object, propertyName.c_str(), &hasProperty), false);
    return hasProperty;
}

bool PluginUtilsNApi::GetPropertyNames(napi_env env, napi_value object, std::vector<std::string>& nameList)
{
    napi_value names = nullptr;
    NAPI_CALL_BASE(env, napi_get_property_names(env, object, &names), false);
    uint32_t length = 0;
    NAPI_CALL_BASE(env, napi_get_array_length(env, names, &length), false);
    for (uint32_t index = 0; index < length; ++index) {
        napi_value name = nullptr;
        if (napi_get_element(env, names, index, &name) != napi_ok) {
            continue;
        }
        if (GetValueType(env, name) != napi_string) {
            continue;
        }
        nameList.emplace_back(GetStringFromValueUtf8(env, name));
    }
    return true;
}

napi_value PluginUtilsNApi::NewInstance(napi_env env, napi_callback_info info,
    const std::string& className, size_t argc, const napi_value* argv)
{
    napi_value thisVal = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVal, nullptr));

    napi_value jsConstructor = GetNamedProperty(env, thisVal, className);
    if (GetValueType(env, jsConstructor) == napi_undefined) {
        return nullptr;
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_new_instance(env, jsConstructor, argc, argv, &result));
    return result;
}

napi_value PluginUtilsNApi::CreateErrorMessage(napi_env env, int errorCode, const std::string& errorMessage)
{
    napi_value result = CreateObject(env);
    SetNamedProperty(env, result, "code", GetNapiInt32(errorCode, env));
    SetNamedProperty(env, result, "message", CreateStringUtf8(env, errorMessage));
    return result;
}

napi_value PluginUtilsNApi::CreateStringUtf8(napi_env env, const std::string& str)
{
    napi_value value = nullptr;
    if (napi_create_string_utf8(env, str.c_str(), strlen(str.c_str()), &value) != napi_ok) {
        return nullptr;
    }
    return value;
}

bool PluginUtilsNApi::GetArrayBuffer(napi_env env, napi_value value, std::vector<uint8_t>& vector)
{
    bool bFlag = false;
    napi_is_arraybuffer(env, value, &bFlag);

    bool isDetached = false;
    napi_is_detached_arraybuffer(env, value, &isDetached);

    if (bFlag && !isDetached) {
        void* buffer = nullptr;
        size_t bufferSize = 0;
        if (napi_get_arraybuffer_info(env, value, &buffer, &bufferSize) == napi_ok) {
            vector.resize(bufferSize);
            if (memcpy_s(reinterpret_cast<uint8_t*>(vector.data()), bufferSize, buffer, bufferSize) == EOK) {
                return true;
            }
        }
    }
    return false;
}

napi_value PluginUtilsNApi::CreateArrayBuffer(napi_env env, const std::vector<uint8_t>& value)
{
   napi_handle_scope scope = nullptr;
   napi_open_handle_scope(env, &scope);
   napi_value arrayBuffer = nullptr;
   size_t bufferSize = value.size();
   void *data = nullptr;
   napi_create_arraybuffer(env, bufferSize, &data, &arrayBuffer);
    if (memcpy_s(data, bufferSize, reinterpret_cast<const uint8_t*>(value.data()), bufferSize) != EOK) {
       napi_close_handle_scope(env, scope);
       return nullptr;
   }
   napi_value result = nullptr;
   napi_create_typedarray(env, napi_uint8_array, bufferSize, arrayBuffer, 0, &result);
   napi_close_handle_scope(env, scope);
   return result;
}

std::string PluginUtilsNApi::GetStringFromValueUtf8(napi_env env, napi_value value)
{
    static constexpr size_t max_length = 2097152;
    if (GetValueType(env, value) != napi_string) {
        return {};
    }

    std::string result;
    size_t stringLength = 0;
    NAPI_CALL_BASE(env, napi_get_value_string_utf8(env, value, nullptr, 0, &stringLength), result);
    if (stringLength == 0 || stringLength > max_length) {
        return result;
    }

    auto deleter = [](char* s) { free(reinterpret_cast<void*>(s)); };
    char* strTmp = static_cast<char*>(malloc(stringLength + 1));
    if (strTmp == nullptr) {
        return result;
    }
    std::unique_ptr<char, decltype(deleter)> str(strTmp, deleter);
    if (memset_s(str.get(), stringLength + 1, 0, stringLength + 1) != EOK) {
        return result;
    }
    size_t length = 0;
    NAPI_CALL_BASE(env, napi_get_value_string_utf8(env, value, str.get(), stringLength + 1, &length), result);
    if (length > 0) {
        result.append(str.get(), length);
    }
    return result;
}

ReportEventType PluginUtilsNApi::IsMatchType(napi_value value, napi_valuetype type, napi_env env)
{
    napi_valuetype paramType;
    napi_typeof(env, value, &paramType);
    if (paramType == type) {
        return TYPE_CHECK_SUCCESS;
    }
    return TYPE_CHECK_FAIL;
}

napi_value PluginUtilsNApi::GetNapiInt32(int32_t number, napi_env env)
{
    napi_value value;
    napi_create_int32(env, number, &value);
    return value;
}

napi_value PluginUtilsNApi::GetNamedProperty(napi_value jsonObject, std::string name, napi_env env)
{
    napi_value value;
    napi_get_named_property(env, jsonObject, name.c_str(), &value);
    return value;
}

int32_t PluginUtilsNApi::GetCInt32(napi_value value, napi_env env)
{
    int32_t num;
    napi_get_value_int32(env, value, &num);
    return num;
}

size_t PluginUtilsNApi::GetCString(napi_value value, napi_env env, char* buffer, size_t bufSize)
{
    size_t valueLength;
    napi_get_value_string_utf8(env, value, buffer, bufSize, &valueLength);
    return valueLength;
}

int64_t PluginUtilsNApi::GetCInt64(napi_value value, napi_env env)
{
    int64_t num;
    napi_get_value_int64(env, value, &num);
    return num;
}

void PluginUtilsNApi::EmitAsyncCallbackWork(AsyncCallbackInfo* asyncCallbackInfo)
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
            AsyncCallbackInfo* asyncCallbackInfo = (AsyncCallbackInfo*)data;
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

void PluginUtilsNApi::EmitPromiseWork(AsyncCallbackInfo* asyncCallbackInfo)
{
    if (asyncCallbackInfo == nullptr) {
        return;
    }
    napi_value resourceName;
    napi_create_string_latin1(asyncCallbackInfo->env, "Promise", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        asyncCallbackInfo->env, nullptr, resourceName, [](napi_env env, void* data) {},
        [](napi_env env, napi_status status, void* data) {
            AsyncCallbackInfo* asyncCallbackInfo = (AsyncCallbackInfo*)data;
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

napi_value PluginUtilsNApi::CreateObject(napi_env env)
{
    napi_value object = nullptr;
    NAPI_CALL(env, napi_create_object(env, &object));
    return object;
}

napi_value PluginUtilsNApi::CreateUndefined(napi_env env)
{
    napi_value undefined = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undefined));
    return undefined;
}

napi_value PluginUtilsNApi::CreateNull(napi_env env)
{
    napi_value jsNull = nullptr;
    NAPI_CALL(env, napi_get_null(env, &jsNull));
    return jsNull;
}

napi_valuetype PluginUtilsNApi::GetValueType(napi_env env, napi_value value)
{
    if (value == nullptr) {
        return napi_undefined;
    }

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL_BASE(env, napi_typeof(env, value, &valueType), napi_undefined);
    return valueType;
}

napi_value PluginUtilsNApi::CreateInt32(napi_env env, int32_t code)
{
    napi_value value = nullptr;
    if (napi_create_int32(env, code, &value) != napi_ok) {
        return nullptr;
    }
    return value;
}

napi_value PluginUtilsNApi::CreateBoolean(napi_env env, bool value)
{
    napi_value jsValue = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, value, &jsValue));
    return jsValue;
}

bool PluginUtilsNApi::GetBool(napi_env env, napi_value value)
{
    bool boolValue = false;
    napi_status ret = napi_get_value_bool(env, value, &boolValue);
    if (ret == napi_ok) {
        return boolValue;
    }
    return false;
}

napi_value PluginUtilsNApi::CreateDouble(napi_env env, double value)
{
    napi_value jsValue = nullptr;
    NAPI_CALL(env, napi_create_double(env, value, &jsValue));
    return jsValue;
}

double PluginUtilsNApi::GetDouble(napi_env env, napi_value value)
{
    double numberValue = 0;
    napi_status ret = napi_get_value_double(env, value, &numberValue);
    if (ret == napi_ok) {
        return numberValue;
    }
    return 0;
}

bool PluginUtilsNApi::IsArray(napi_env env, napi_value value)
{
    bool isArray = false;
    napi_status ret = napi_is_array(env, value, &isArray);
    if (ret == napi_ok) {
        return isArray;
    }
    return false;
}

bool PluginUtilsNApi::IsArrayBuffer(napi_env env, napi_value value)
{
    bool isArrayBuffer = false;
    napi_status ret = napi_is_arraybuffer(env, value, &isArrayBuffer);
    if (ret == napi_ok) {
        return isArrayBuffer;
    }
    return false;
}

napi_value PluginUtilsNApi::CreateArray(napi_env env)
{
    napi_value value = nullptr;
    NAPI_CALL(env, napi_create_array(env, &value));
    return value;
}

void PluginUtilsNApi::SetSelementToArray(napi_env env, napi_value array, int index, napi_value value)
{
    napi_set_element(env, array, index, value);
}

napi_ref PluginUtilsNApi::CreateReference(napi_env env, napi_value callback)
{
    napi_ref callbackRef = nullptr;
    NAPI_CALL(env, napi_create_reference(env, callback, 1, &callbackRef));
    return callbackRef;
}

napi_value PluginUtilsNApi::GetReference(napi_env env, napi_ref callbackRef)
{
    napi_value callback = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, callbackRef, &callback));
    return callback;
}

void PluginUtilsNApi::DeleteReference(napi_env env, napi_ref callbackRef)
{
    (void)napi_delete_reference(env, callbackRef);
}

napi_value PluginUtilsNApi::CallFunction(napi_env env, napi_value recv, napi_value func,
    size_t argc, const napi_value* argv)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_call_function(env, recv, func, argc, argv, &result));
    return result;
}

napi_async_work PluginUtilsNApi::CreateAsyncWork(napi_env env, napi_value asyncWorkName,
    AsyncWorkExecutor executor, AsyncWorkComplete callback, void* data)
{
    napi_async_work asyncWork = nullptr;
    napi_status ret = napi_create_async_work(env, nullptr, asyncWorkName,
        executor, callback, data, &asyncWork);
    if (ret != napi_ok) {
        return nullptr;
    }

    if (napi_queue_async_work(env, asyncWork) != napi_ok) {
        napi_delete_async_work(env, asyncWork);
        asyncWork = nullptr;
    }
    return asyncWork;
}

void PluginUtilsNApi::DeleteAsyncWork(napi_env env, napi_async_work asyncWork)
{
    if (asyncWork != nullptr) {
        (void)napi_delete_async_work(env, asyncWork);
    }
}

napi_status PluginUtilsNApi::SetEnumItem(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_status status;
    napi_value itemName;
    napi_value itemValue;

    NAPI_CALL_BASE(env, status = napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &itemName), status);
    NAPI_CALL_BASE(env, status = napi_create_int32(env, value, &itemValue), status);

    NAPI_CALL_BASE(env, status = napi_set_property(env, object, itemName, itemValue), status);
    NAPI_CALL_BASE(env, status = napi_set_property(env, object, itemValue, itemName), status);

    return napi_ok;
}

bool PluginUtilsNApi::DetachArrayBufferFromTypedArray(napi_env env, napi_value value)
{
    napi_value arrayBuffer;
    bool isTypedArray = false;
    NAPI_CALL_BASE(env, napi_is_typedarray(env, value, &isTypedArray), false);
    if (isTypedArray) {
        NAPI_CALL_BASE(
            env, napi_get_typedarray_info(env, value, nullptr, nullptr, nullptr, &arrayBuffer, nullptr), false);
        NAPI_CALL_BASE(env, napi_detach_arraybuffer(env, arrayBuffer), false);
        return true;
    } else {
        return false;
    }
}
} // namespace OHOS::Plugin
