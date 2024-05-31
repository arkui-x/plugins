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

#ifndef PLUGINS_BRIDGE_METHOD_DATA_CONVERTER_H
#define PLUGINS_BRIDGE_METHOD_DATA_CONVERTER_H

#include "napi/native_api.h"
#include "codecable_value.h"

namespace OHOS::Plugin::Bridge {
class MethodDataConverter {
public:
    MethodDataConverter() = delete;
    virtual ~MethodDataConverter() = delete;

    static CodecableValue ConvertToCodecableValue(napi_env env, napi_value value);
    static CodecableValue ConvertToCodecableValue(napi_env env, const size_t& argc, const napi_value* argv);
    
    static napi_value ConvertToNapiValue(napi_env env, const CodecableValue& value);
    static void ConvertToNapiValues(napi_env env, const CodecableValue& value, size_t& argc, napi_value* argv);

private:
    static CodecableValue GainListValue(napi_env env, napi_value value);
    static CodecableValue GainMapValue(napi_env env, napi_value value);

    static CodecableValue GainListBoolValue(napi_env env, napi_value value, uint32_t length);
    static CodecableValue GainListStringValue(napi_env env, napi_value value, uint32_t length);
    static CodecableValue GainListDoubleValue(napi_env env, napi_value value, uint32_t length);
    static CodecableValue GainListInt32Value(napi_env env, napi_value value, uint32_t length);

    static napi_value CreateMapValue(napi_env env, const CodecableValue& value);
    static napi_value CreateListBoolValue(napi_env env, const CodecableValue& value);
    static napi_value CreateListInt32Value(napi_env env, const CodecableValue& value);
    static napi_value CreateListInt64Value(napi_env env, const CodecableValue& value);
    static napi_value CreateListDoubleValue(napi_env env, const CodecableValue& value);
    static napi_value CreateListStringValue(napi_env env, const CodecableValue& value);
};
} // namespace OHOS::Plugin::Bridge
#endif