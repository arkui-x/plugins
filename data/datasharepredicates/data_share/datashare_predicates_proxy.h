/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DATASHARE_PREDICATES_PROXY_H
#define DATASHARE_PREDICATES_PROXY_H

#include "datashare_abs_predicates.h"
#include "js_proxy.h"

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace DataShare {
class DataSharePredicatesProxy : public JSProxy::JSProxy<DataShareAbsPredicates>{
public:
    static void Init(napi_env env, napi_value exports);
    static std::shared_ptr<DataShareAbsPredicates> GetNativePredicates(
        const napi_env env, const napi_value arg);
    static void Destructor(napi_env env, void *nativeObject, void *finalize_hint);

    DataSharePredicatesProxy();
    static napi_value GetConstructor(napi_env env);

private:
    ~DataSharePredicatesProxy();

    static napi_value New(napi_env env, napi_callback_info info);
    static std::shared_ptr<DataShareAbsPredicates> GetNativePredicates(napi_env env, napi_callback_info info);

    static napi_value EqualTo(napi_env env, napi_callback_info info);
    static napi_value NotEqualTo(napi_env env, napi_callback_info info);
    static napi_value BeginWrap(napi_env env, napi_callback_info info);
    static napi_value EndWrap(napi_env env, napi_callback_info info);
    static napi_value Or(napi_env env, napi_callback_info info);
    static napi_value And(napi_env env, napi_callback_info info);
    static napi_value Contains(napi_env env, napi_callback_info info);
    static napi_value BeginsWith(napi_env env, napi_callback_info info);
    static napi_value EndsWith(napi_env env, napi_callback_info info);
    static napi_value IsNull(napi_env env, napi_callback_info info);
    static napi_value IsNotNull(napi_env env, napi_callback_info info);
    static napi_value Like(napi_env env, napi_callback_info info);
    static napi_value Unlike(napi_env env, napi_callback_info info);
    static napi_value Glob(napi_env env, napi_callback_info info);
    static napi_value Between(napi_env env, napi_callback_info info);
    static napi_value NotBetween(napi_env env, napi_callback_info info);
    static napi_value GreaterThan(napi_env env, napi_callback_info info);
    static napi_value LessThan(napi_env env, napi_callback_info info);
    static napi_value GreaterThanOrEqualTo(napi_env env, napi_callback_info info);
    static napi_value LessThanOrEqualTo(napi_env env, napi_callback_info info);
    static napi_value OrderByAsc(napi_env env, napi_callback_info info);
    static napi_value OrderByDesc(napi_env env, napi_callback_info info);
    static napi_value Distinct(napi_env env, napi_callback_info info);
    static napi_value Limit(napi_env env, napi_callback_info info);
    static napi_value GroupBy(napi_env env, napi_callback_info info);
    static napi_value IndexedBy(napi_env env, napi_callback_info info);
    static napi_value In(napi_env env, napi_callback_info info);
    static napi_value NotIn(napi_env env, napi_callback_info info);
    static napi_value PrefixKey(napi_env env, napi_callback_info info);
    static napi_value InKeys(napi_env env, napi_callback_info info);
};
} // namespace DataShare
} // namespace OHOS
#endif // DATASHARE_PREDICATES_PROXY_H
