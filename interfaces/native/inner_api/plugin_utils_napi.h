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

#ifndef PLUGIN_INTERFACE_NATIVE_INNER_UTILS_PLUGIN_INNER_NAPI_UTILS_H
#define PLUGIN_INTERFACE_NATIVE_INNER_UTILS_PLUGIN_INNER_NAPI_UTILS_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS::Plugin {
typedef void (*AsyncWorkExecutor)(napi_env env, void* data);
typedef void (*AsyncWorkComplete)(napi_env env, napi_status status, void* data);

typedef enum ReportEventType {
    TYPE_CHECK_SUCCESS = true,
    TYPE_CHECK_FAIL = false,
} ReportEventType;

struct AsyncCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback[1] = { 0 };
    napi_value jsdata;
    int32_t status;
};

class PluginUtilsNApi {
public:
    static constexpr const size_t MAX_ARG_NUM = 10;
    static constexpr const int ARG_NUM_0 = 0;
    static constexpr const int ARG_NUM_1 = 1;
    static constexpr const int ARG_NUM_2 = 2;
    static constexpr const int ARG_NUM_3 = 3;
    static constexpr const int ARG_NUM_4 = 4;
    static constexpr const int ARG_NUM_5 = 5;
    static constexpr const int ARG_NUM_6 = 6;
    static constexpr const int ARG_NUM_7 = 7;
    static constexpr const int ARG_NUM_8 = 8;
    static constexpr const int ARG_NUM_9 = 9;

    static void DefineProperties(napi_env env, napi_value object,
        const std::initializer_list<napi_property_descriptor>& properties);
    static void DefineClass(napi_env env, napi_value exports,
        const std::initializer_list<napi_property_descriptor>& properties, const std::string& className);
    static void SetNamedProperty(
        napi_env env, napi_value object, const std::string& propertyName, napi_value value);
    static napi_value GetNamedProperty(napi_env env, napi_value object, const std::string& propertyName);
    static bool HasNamedProperty(napi_env env, napi_value object, const std::string& propertyName);
    static bool GetPropertyNames(napi_env env, napi_value object, std::vector<std::string>& nameList);
    static napi_value NewInstance(napi_env env, napi_callback_info info,
        const std::string& className, size_t argc, const napi_value* argv);
    static napi_value CreateErrorMessage(napi_env env, int errorCode, const std::string& errorMessage);
    static napi_value CreateStringUtf8(napi_env env, const std::string& str);
    static std::string GetStringFromValueUtf8(napi_env env, napi_value value);
    static ReportEventType IsMatchType(napi_value value, napi_valuetype type, napi_env env);
    static napi_value GetNapiInt32(int32_t number, napi_env env);
    static napi_value GetNamedProperty(napi_value jsonObject, std::string name, napi_env env);
    static int32_t GetCInt32(napi_value value, napi_env env);
    static int64_t GetCInt64(napi_value value, napi_env env);
    static size_t GetCString(napi_value value, napi_env env, char* buf, size_t size);
    static void EmitPromiseWork(AsyncCallbackInfo* asyncCallbackInfo);
    static void EmitAsyncCallbackWork(AsyncCallbackInfo* asyncCallbackInfo);
    static napi_value CreateObject(napi_env env);
    static napi_value CreateUndefined(napi_env env);
    static napi_value CreateNull(napi_env env);
    static napi_valuetype GetValueType(napi_env env, napi_value value);
    static napi_value CreateBoolean(napi_env env, bool value);
    static napi_value CreateInt32(napi_env env, int32_t code);
    static bool GetBool(napi_env env, napi_value value);
    static napi_value CreateDouble(napi_env env, double value);
    static double GetDouble(napi_env env, napi_value value);
    static bool IsArray(napi_env env, napi_value value);
    static void SetSelementToArray(napi_env env, napi_value array, int index, napi_value value);
    static napi_value CreateArray(napi_env env);
    static napi_ref CreateReference(napi_env env, napi_value callback);
    static napi_value GetReference(napi_env env, napi_ref callbackRef);
    static void DeleteReference(napi_env env, napi_ref callbackRef);
    static napi_value CallFunction(napi_env env, napi_value recv, napi_value func,
        size_t argc, const napi_value* argv);
    static napi_async_work CreateAsyncWork(napi_env env, napi_value asyncWorkName,
        AsyncWorkExecutor executor, AsyncWorkComplete callback, void* data);
    static void DeleteAsyncWork(napi_env env, napi_async_work asyncWork);
    static bool IsArrayBuffer(napi_env env, napi_value value);
    static bool GetArrayBuffer(napi_env env, napi_value value, std::vector<uint8_t>& vector);
    static napi_value CreateArrayBuffer(napi_env env, const std::vector<uint8_t>& value);
    static napi_status SetEnumItem(napi_env env, napi_value object, const char* name, int32_t value);
    static bool DetachArrayBufferFromTypedArray(napi_env env, napi_value value);
};
} // namespace OHOS::Plugin
#endif // PLUGIN_INTERFACE_NATIVE_INNER_UTILS_PLUGIN_INNER_NAPI_UTILS_H
