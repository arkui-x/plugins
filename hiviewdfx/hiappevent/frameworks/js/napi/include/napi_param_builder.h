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
#ifndef PLUGINS_HIVIEWDFX_HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_PARAM_BUILDER_H
#define PLUGINS_HIVIEWDFX_HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_PARAM_BUILDER_H

#include <memory>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_hiappevent_builder.h"

namespace OHOS {
namespace HiviewDFX {
class AppEventPack;

class NapiParamBuilder : public NapiHiAppEventBuilder {
public:
    NapiParamBuilder()
    {
        isV9_ = true;
    }
    ~NapiParamBuilder() {}
    std::shared_ptr<AppEventPack> BuildEventParam(const napi_env env, const napi_value params[], size_t len);

protected:
    virtual bool AddArrayParam2EventPack(napi_env env, const std::string& key, const napi_value arr) override;
    virtual bool AddParams2EventPack(napi_env env, const napi_value paramObj) override;

private:
    bool IsValidParams(const napi_env env, const napi_value params[], size_t len);
    bool BuildCustomEventParamPack(napi_env env, const napi_value params[], size_t len);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // PLUGINS_HIVIEWDFX_HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_PARAM_BUILDER_H
