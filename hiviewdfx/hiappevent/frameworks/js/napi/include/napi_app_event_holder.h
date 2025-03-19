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
#ifndef PLUGINS_HIVIEWDFX_HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_HIAPPEVENT_HOLDER_H
#define PLUGINS_HIVIEWDFX_HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_HIAPPEVENT_HOLDER_H

#include <memory>
#include <string>
#include <vector>

#include "hiappevent_base.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace HiviewDFX {
struct AppEventPackage {
    AppEventPackage() : packageId(0), row(0), size(0), data(0), events(0) {}
    ~AppEventPackage() {}
    int packageId;
    int row;
    int size;
    std::vector<std::string> data;
    std::vector<std::shared_ptr<AppEventPack>> events;
};

class NapiAppEventHolder {
public:
    NapiAppEventHolder(const std::string& name, int64_t observerSeq = -1);
    ~NapiAppEventHolder() {}
    static napi_value NapiConstructor(napi_env env, napi_callback_info info);
    static napi_value NapiExport(napi_env env, napi_value exports);
    static napi_value NapiSetRow(napi_env env, napi_callback_info info);
    static napi_value NapiSetSize(napi_env env, napi_callback_info info);
    static napi_value NapiTakeNext(napi_env env, napi_callback_info info);

    void SetRow(int row);
    void SetSize(int size);
    std::shared_ptr<AppEventPackage> TakeNext();

public:
    static thread_local napi_ref constructor_;

private:
    std::string name_;
    int takeRow_;
    int takeSize_;
    int packageId_;
    int64_t observerSeq_;
    bool hasSetRow_;
    bool hasSetSize_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // PLUGINS_HIVIEWDFX_HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_HIAPPEVENT_HOLDER_H
