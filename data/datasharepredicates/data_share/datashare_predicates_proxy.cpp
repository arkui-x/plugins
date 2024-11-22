/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "datashare_predicates_proxy.h"

#include "datashare_js_utils.h"
#include "datashare_predicates.h"

namespace OHOS {
namespace DataShare {
static napi_ref __thread constructor_ = nullptr;

napi_value DataSharePredicatesProxy::GetConstructor(napi_env env)
{
    napi_value cons;
    if (constructor_ != nullptr) {
        NAPI_CALL(env, napi_get_reference_value(env, constructor_, &cons));
        return cons;
    }
    napi_property_descriptor descriptors[] = {
        DECLARE_NAPI_FUNCTION("equalTo", EqualTo),
        DECLARE_NAPI_FUNCTION("notEqualTo", NotEqualTo),
        DECLARE_NAPI_FUNCTION("beginWrap", BeginWrap),
        DECLARE_NAPI_FUNCTION("endWrap", EndWrap),
        DECLARE_NAPI_FUNCTION("or", Or),
        DECLARE_NAPI_FUNCTION("and", And),
        DECLARE_NAPI_FUNCTION("contains", Contains),
        DECLARE_NAPI_FUNCTION("beginsWith", BeginsWith),
        DECLARE_NAPI_FUNCTION("endsWith", EndsWith),
        DECLARE_NAPI_FUNCTION("isNull", IsNull),
        DECLARE_NAPI_FUNCTION("isNotNull", IsNotNull),
        DECLARE_NAPI_FUNCTION("like", Like),
        DECLARE_NAPI_FUNCTION("unlike", Unlike),
        DECLARE_NAPI_FUNCTION("glob", Glob),
        DECLARE_NAPI_FUNCTION("between", Between),
        DECLARE_NAPI_FUNCTION("notBetween", NotBetween),
        DECLARE_NAPI_FUNCTION("greaterThan", GreaterThan),
        DECLARE_NAPI_FUNCTION("lessThan", LessThan),
        DECLARE_NAPI_FUNCTION("greaterThanOrEqualTo", GreaterThanOrEqualTo),
        DECLARE_NAPI_FUNCTION("lessThanOrEqualTo", LessThanOrEqualTo),
        DECLARE_NAPI_FUNCTION("orderByAsc", OrderByAsc),
        DECLARE_NAPI_FUNCTION("orderByDesc", OrderByDesc),
        DECLARE_NAPI_FUNCTION("distinct", Distinct),
        DECLARE_NAPI_FUNCTION("limit", Limit),
        DECLARE_NAPI_FUNCTION("groupBy", GroupBy),
        DECLARE_NAPI_FUNCTION("indexedBy", IndexedBy),
        DECLARE_NAPI_FUNCTION("in", In),
        DECLARE_NAPI_FUNCTION("notIn", NotIn),
        DECLARE_NAPI_FUNCTION("prefixKey", PrefixKey),
        DECLARE_NAPI_FUNCTION("inKeys", InKeys),
    };
    NAPI_CALL(env, napi_define_class(env, "DataSharePredicates", NAPI_AUTO_LENGTH, New, nullptr,
                       sizeof(descriptors) / sizeof(napi_property_descriptor), descriptors, &cons));
    NAPI_CALL(env, napi_create_reference(env, cons, 1, &constructor_));
    return cons;
}

void DataSharePredicatesProxy::Init(napi_env env, napi_value exports)
{
    napi_value cons = GetConstructor(env);
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, exports, "DataSharePredicates", cons));
}

napi_value DataSharePredicatesProxy::New(napi_env env, napi_callback_info info)
{
    napi_value new_target;
    NAPI_CALL(env, napi_get_new_target(env, info, &new_target));
    bool is_constructor = (new_target != nullptr);

    napi_value thiz;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thiz, nullptr));

    if (is_constructor) {
        auto* proxy = new (std::nothrow) DataSharePredicatesProxy();
        if (proxy == nullptr) {
            return nullptr;
        }
        proxy->SetInstance(std::make_shared<DataSharePredicates>());
        napi_status ret = napi_wrap(env, thiz, proxy, DataSharePredicatesProxy::Destructor, nullptr, nullptr);
        if (ret != napi_ok) {
            delete proxy;
            return nullptr;
        }
        return thiz;
    }

    napi_value cons = GetConstructor(env);
    if (cons == nullptr) {
        return nullptr;
    }

    napi_value output;
    NAPI_CALL(env, napi_new_instance(env, cons, 0, nullptr, &output));

    return output;
}

std::shared_ptr<DataShareAbsPredicates> DataSharePredicatesProxy::GetNativePredicates(
    const napi_env env, const napi_value arg)
{
    if (arg == nullptr) {
        return nullptr;
    }
    DataSharePredicatesProxy* proxy = nullptr;
    napi_unwrap(env, arg, reinterpret_cast<void**>(&proxy));
    return proxy->GetInstance();
}

void DataSharePredicatesProxy::Destructor(napi_env env, void* nativeObject, void*)
{
    DataSharePredicatesProxy* proxy = static_cast<DataSharePredicatesProxy*>(nativeObject);
    delete proxy;
}

DataSharePredicatesProxy::~DataSharePredicatesProxy()
{
    SetInstance(nullptr);
}

DataSharePredicatesProxy::DataSharePredicatesProxy() {}

std::shared_ptr<DataShareAbsPredicates> DataSharePredicatesProxy::GetNativePredicates(
    napi_env env, napi_callback_info info)
{
    DataSharePredicatesProxy* predicatesProxy = nullptr;
    napi_value thiz;
    napi_get_cb_info(env, info, nullptr, nullptr, &thiz, nullptr);
    napi_unwrap(env, thiz, reinterpret_cast<void**>(&predicatesProxy));
    return predicatesProxy->GetInstance();
}

napi_value DataSharePredicatesProxy::EqualTo(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 2;
    napi_value args[2] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 1, "Invalid argvs!");
    std::string field = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, args[1], &valueType);
    if (status != napi_ok) {
        return thiz;
    }
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    switch (valueType) {
        case napi_number: {
            double value;
            napi_get_value_double(env, args[1], &value);
            nativePredicates->EqualTo(field, value);
            break;
        }
        case napi_boolean: {
            bool value = false;
            napi_get_value_bool(env, args[1], &value);
            nativePredicates->EqualTo(field, value);
            break;
        }
        case napi_string: {
            std::string value = DataShareJSUtils::Convert2String(env, args[1], DataShareJSUtils::DEFAULT_BUF_SIZE);
            nativePredicates->EqualTo(field, value);
            break;
        }
        default:
            break;
    }
    return thiz;
}

napi_value DataSharePredicatesProxy::NotEqualTo(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 2;
    napi_value args[2] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 1, "Invalid argvs!");
    std::string field = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, args[1], &valueType);
    if (status != napi_ok) {
        return thiz;
    }
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    switch (valueType) {
        case napi_number: {
            double value;
            napi_get_value_double(env, args[1], &value);
            nativePredicates->NotEqualTo(field, value);
            break;
        }
        case napi_boolean: {
            bool value = false;
            napi_get_value_bool(env, args[1], &value);
            nativePredicates->NotEqualTo(field, value);
            break;
        }
        case napi_string: {
            std::string value = DataShareJSUtils::Convert2String(env, args[1], DataShareJSUtils::DEFAULT_BUF_SIZE);
            nativePredicates->NotEqualTo(field, value);
            break;
        }
        default:
            break;
    }
    return thiz;
}

napi_value DataSharePredicatesProxy::BeginWrap(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    napi_get_cb_info(env, info, nullptr, nullptr, &thiz, nullptr);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->BeginWrap();
    return thiz;
}

napi_value DataSharePredicatesProxy::EndWrap(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    napi_get_cb_info(env, info, nullptr, nullptr, &thiz, nullptr);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->EndWrap();
    return thiz;
}

napi_value DataSharePredicatesProxy::Or(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    napi_get_cb_info(env, info, nullptr, nullptr, &thiz, nullptr);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->Or();
    return thiz;
}

napi_value DataSharePredicatesProxy::And(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    napi_get_cb_info(env, info, nullptr, nullptr, &thiz, nullptr);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->And();
    return thiz;
}

napi_value DataSharePredicatesProxy::Contains(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 2;
    napi_value args[2] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 1, "Invalid argvs!");
    std::string field = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    std::string value = DataShareJSUtils::ConvertAny2String(env, args[1]);

    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->Contains(field, value);
    return thiz;
}

napi_value DataSharePredicatesProxy::BeginsWith(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 2;
    napi_value args[2] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 1, "Invalid argvs!");
    std::string field = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    std::string value = DataShareJSUtils::ConvertAny2String(env, args[1]);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->BeginsWith(field, value);
    return thiz;
}

napi_value DataSharePredicatesProxy::EndsWith(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 2;
    napi_value args[2] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 1, "Invalid argvs!");
    std::string field = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    std::string value = DataShareJSUtils::ConvertAny2String(env, args[1]);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->EndsWith(field, value);
    return thiz;
}

napi_value DataSharePredicatesProxy::IsNull(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 1;
    napi_value args[1] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 0, "Invalid argvs!");
    std::string field = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->IsNull(field);
    return thiz;
}

napi_value DataSharePredicatesProxy::IsNotNull(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 1;
    napi_value args[1] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 0, "Invalid argvs!");
    std::string field = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->IsNotNull(field);
    return thiz;
}

napi_value DataSharePredicatesProxy::Like(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 2;
    napi_value args[2] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 1, "Invalid argvs!");
    std::string field = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    std::string value = DataShareJSUtils::ConvertAny2String(env, args[1]);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->Like(field, value);
    return thiz;
}

napi_value DataSharePredicatesProxy::Unlike(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 2;
    napi_value args[2] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 1, "Invalid argvs!");
    std::string field = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    std::string value = DataShareJSUtils::ConvertAny2String(env, args[1]);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->Unlike(field, value);
    return thiz;
}

napi_value DataSharePredicatesProxy::Glob(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 2;
    napi_value args[2] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 1, "Invalid argvs!");
    std::string field = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    std::string value = DataShareJSUtils::ConvertAny2String(env, args[1]);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->Glob(field, value);
    return thiz;
}

napi_value DataSharePredicatesProxy::Between(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 3;
    napi_value args[3] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    //'argc > 2' represents determine if 'argc' is greater than '2'
    NAPI_ASSERT(env, argc > 2, "Invalid argvs!");
    std::string field = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    std::string low = DataShareJSUtils::ConvertAny2String(env, args[1]);
    std::string high = DataShareJSUtils::ConvertAny2String(env, args[2]);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->Between(field, low, high);
    return thiz;
}

napi_value DataSharePredicatesProxy::NotBetween(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 3;
    napi_value args[3] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    //'argc > 2' represents determine if 'argc' is greater than '2'
    NAPI_ASSERT(env, argc > 2, "Invalid argvs!");
    std::string field = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    std::string low = DataShareJSUtils::ConvertAny2String(env, args[1]);
    std::string high = DataShareJSUtils::ConvertAny2String(env, args[2]);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->NotBetween(field, low, high);
    return thiz;
}

napi_value DataSharePredicatesProxy::GreaterThan(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 2;
    napi_value args[2] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 1, "Invalid argvs!");
    std::string field = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, args[1], &valueType);
    if (status != napi_ok) {
        return thiz;
    }
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    switch (valueType) {
        case napi_number: {
            double value;
            napi_get_value_double(env, args[1], &value);
            nativePredicates->GreaterThan(field, value);
            break;
        }
        case napi_string: {
            std::string value = DataShareJSUtils::Convert2String(env, args[1], DataShareJSUtils::DEFAULT_BUF_SIZE);
            nativePredicates->GreaterThan(field, value);
            break;
        }
        default:
            break;
    }
    return thiz;
}

napi_value DataSharePredicatesProxy::LessThan(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 2;
    napi_value args[2] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 1, "Invalid argvs!");
    std::string field = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, args[1], &valueType);
    if (status != napi_ok) {
        return thiz;
    }
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    switch (valueType) {
        case napi_number: {
            double value;
            napi_get_value_double(env, args[1], &value);
            nativePredicates->LessThan(field, value);
            break;
        }
        case napi_string: {
            std::string value = DataShareJSUtils::Convert2String(env, args[1], DataShareJSUtils::DEFAULT_BUF_SIZE);
            nativePredicates->LessThan(field, value);
            break;
        }
        default:
            break;
    }
    return thiz;
}

napi_value DataSharePredicatesProxy::GreaterThanOrEqualTo(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 2;
    napi_value args[2] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 1, "Invalid argvs!");
    std::string field = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, args[1], &valueType);
    if (status != napi_ok) {
        return thiz;
    }
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    switch (valueType) {
        case napi_number: {
            double value;
            napi_get_value_double(env, args[1], &value);
            nativePredicates->GreaterThanOrEqualTo(field, value);
            break;
        }
        case napi_string: {
            std::string value = DataShareJSUtils::Convert2String(env, args[1], DataShareJSUtils::DEFAULT_BUF_SIZE);
            nativePredicates->GreaterThanOrEqualTo(field, value);
            break;
        }
        default:
            break;
    }
    return thiz;
}

napi_value DataSharePredicatesProxy::LessThanOrEqualTo(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 2;
    napi_value args[2] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 1, "Invalid argvs!");
    std::string field = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, args[1], &valueType);
    if (status != napi_ok) {
        return thiz;
    }
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    switch (valueType) {
        case napi_number: {
            double value;
            napi_get_value_double(env, args[1], &value);
            nativePredicates->LessThanOrEqualTo(field, value);
            break;
        }
        case napi_string: {
            std::string value = DataShareJSUtils::Convert2String(env, args[1], DataShareJSUtils::DEFAULT_BUF_SIZE);
            nativePredicates->LessThanOrEqualTo(field, value);
            break;
        }
        default:
            break;
    }
    return thiz;
}

napi_value DataSharePredicatesProxy::OrderByAsc(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 1;
    napi_value args[1] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 0, "Invalid argvs!");
    std::string field = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->OrderByAsc(field);
    return thiz;
}

napi_value DataSharePredicatesProxy::OrderByDesc(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 1;
    napi_value args[1] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 0, "Invalid argvs!");
    std::string field = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->OrderByDesc(field);
    return thiz;
}

napi_value DataSharePredicatesProxy::Distinct(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    napi_get_cb_info(env, info, nullptr, nullptr, &thiz, nullptr);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->Distinct();
    return thiz;
}

napi_value DataSharePredicatesProxy::Limit(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 2;
    napi_value args[2] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 1, "Invalid argvs!");
    int number = 0;
    napi_get_value_int32(env, args[0], &number);
    int offset = 0;
    napi_get_value_int32(env, args[1], &offset);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->Limit(number, offset);
    return thiz;
}

napi_value DataSharePredicatesProxy::GroupBy(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 1;
    napi_value args[1] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 0, "Invalid argvs!");
    std::vector<std::string> fields =
        DataShareJSUtils::Convert2StrVector(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->GroupBy(fields);
    return thiz;
}

napi_value DataSharePredicatesProxy::IndexedBy(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 1;
    napi_value args[1] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 0, "Invalid argvs!");
    std::string indexName = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->IndexedBy(indexName);
    return thiz;
}

napi_value DataSharePredicatesProxy::In(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 2;
    napi_value args[2] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 1, "Invalid argvs!");
    std::string field = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    std::vector<std::string> values =
        DataShareJSUtils::Convert2StrVector(env, args[1], DataShareJSUtils::DEFAULT_BUF_SIZE);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->In(field, values);
    return thiz;
}

napi_value DataSharePredicatesProxy::NotIn(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 2;
    napi_value args[2] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 1, "Invalid argvs!");
    std::string field = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    std::vector<std::string> values =
        DataShareJSUtils::Convert2StrVector(env, args[1], DataShareJSUtils::DEFAULT_BUF_SIZE);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->NotIn(field, values);
    return thiz;
}

napi_value DataSharePredicatesProxy::PrefixKey(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 1;
    napi_value args[1] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 0, "Invalid argvs!");
    std::string prefix = DataShareJSUtils::Convert2String(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->KeyPrefix(prefix);
    return thiz;
}

napi_value DataSharePredicatesProxy::InKeys(napi_env env, napi_callback_info info)
{
    napi_value thiz;
    size_t argc = 1;
    napi_value args[1] = { 0 };
    napi_get_cb_info(env, info, &argc, args, &thiz, nullptr);
    NAPI_ASSERT(env, argc > 0, "Invalid argvs!");
    std::vector<std::string> keys =
        DataShareJSUtils::Convert2StrVector(env, args[0], DataShareJSUtils::DEFAULT_BUF_SIZE);
    auto nativePredicates = GetNativePredicates(env, info);
    if (nativePredicates == nullptr) {
        return thiz;
    }
    nativePredicates->InKeys(keys);
    return thiz;
}
} // namespace DataShare
} // namespace OHOS