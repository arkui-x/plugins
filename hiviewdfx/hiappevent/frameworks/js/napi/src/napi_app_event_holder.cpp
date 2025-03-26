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
#include "napi_app_event_holder.h"

#include <cinttypes>

#include "app_event_store.h"
#include "hilog/log.h"
#include "napi_error.h"
#include "napi_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "NapiHolder"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr size_t PARAM_NUM = 1;
constexpr int DEFAULT_ROW_NUM = 1;
constexpr int DEFAULT_SIZE = 512 * 1024; // 512 * 1024: 512KB
const std::string HOLDER_CLASS_NAME = "AppEventPackageHolder";

int64_t GetObserverSeqByName(const std::string& name)
{
    int64_t observerSeq = -1;
    if (observerSeq = AppEventStore::GetInstance().QueryObserverSeq(name); observerSeq <= 0) {
        HILOG_WARN(LOG_CORE, "failed to query seq by name=%{public}s", name.c_str());
        return -1;
    }
    return observerSeq;
}
} // namespace
thread_local napi_ref NapiAppEventHolder::constructor_ = nullptr;

NapiAppEventHolder::NapiAppEventHolder(const std::string& name, int64_t observerSeq)
    : name_(name), observerSeq_(observerSeq), hasSetRow_(false), hasSetSize_(false)
{
    takeRow_ = DEFAULT_ROW_NUM;
    takeSize_ = DEFAULT_SIZE;
    packageId_ = 0; // id is incremented from 0

    // if the seq is invalid, need to get seq by the name(for js constructor)
    if (observerSeq_ <= 0) {
        HILOG_WARN(LOG_CORE, "get seq by name=%{public}s", name_.c_str());
        observerSeq_ = GetObserverSeqByName(name_);
    }
}

napi_value NapiAppEventHolder::NapiConstructor(napi_env env, napi_callback_info info)
{
    size_t paramNum = PARAM_NUM;
    napi_value params[PARAM_NUM] = { 0 };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisVar, nullptr));

    if (paramNum < PARAM_NUM) {
        HILOG_ERROR(LOG_CORE, "hodler failed to construct: invalid param num");
        return nullptr;
    }
    auto holder = new (std::nothrow) NapiAppEventHolder(NapiUtil::GetString(env, params[0]));
    if (holder == nullptr) {
        return nullptr;
    }
    napi_wrap(
        env, thisVar, holder,
        [](napi_env env, void* data, void* hint) {
            NapiAppEventHolder* holder = (NapiAppEventHolder*)data;
            delete holder;
        },
        nullptr, nullptr);
    return thisVar;
}

napi_value NapiAppEventHolder::NapiExport(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("setRow", NapiSetRow),
        DECLARE_NAPI_FUNCTION("setSize", NapiSetSize),
        DECLARE_NAPI_FUNCTION("takeNext", NapiTakeNext)
    };
    napi_value holderClass = nullptr;
    napi_define_class(env, HOLDER_CLASS_NAME.c_str(), HOLDER_CLASS_NAME.size(), NapiConstructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &holderClass);
    NapiUtil::SetNamedProperty(env, exports, HOLDER_CLASS_NAME, holderClass);
    constructor_ = NapiUtil::CreateReference(env, holderClass);
    return exports;
}

bool GetNumberParam(
    napi_env env, napi_callback_info info, const char* name, int& num, NapiAppEventHolder*& holder)
{
    size_t paramNum = PARAM_NUM;
    napi_value params[PARAM_NUM] = { 0 };
    napi_value thisVar = nullptr;
    if (napi_get_cb_info(env, info, &paramNum, params, &thisVar, nullptr) != napi_ok) {
        return false;
    }
    if (paramNum < PARAM_NUM) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg(name));
        return false;
    }
    if (!NapiUtil::IsNumber(env, params[0])) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg(name, "number"));
        return false;
    }
    num = NapiUtil::GetInt32(env, params[0]);
    if (napi_unwrap(env, thisVar, (void**)&holder) != napi_ok || holder == nullptr) {
        return false;
    }
    return true;
}

napi_value NapiAppEventHolder::NapiSetRow(napi_env env, napi_callback_info info)
{
    int num = 0;
    NapiAppEventHolder* holder = nullptr;
    if (!GetNumberParam(env, info, "size", num, holder)) {
        return nullptr;
    }
    if (num <= 0) {
        NapiUtil::ThrowError(env, NapiError::ERR_INVALID_SIZE, "Invalid size value.");
        return NapiUtil::CreateUndefined(env);
    }
    holder->SetRow(num);
    return NapiUtil::CreateUndefined(env);
}

napi_value NapiAppEventHolder::NapiSetSize(napi_env env, napi_callback_info info)
{
    int num = 0;
    NapiAppEventHolder* holder = nullptr;
    if (!GetNumberParam(env, info, "size", num, holder)) {
        return nullptr;
    }
    if (num < 0) {
        NapiUtil::ThrowError(env, NapiError::ERR_INVALID_SIZE, "Invalid size value.");
        return NapiUtil::CreateUndefined(env);
    }
    holder->SetSize(num);
    return NapiUtil::CreateUndefined(env);
}

napi_value NapiAppEventHolder::NapiTakeNext(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));
    NapiAppEventHolder* holder = nullptr;
    if (napi_unwrap(env, thisVar, (void**)&holder) != napi_ok || holder == nullptr) {
        return NapiUtil::CreateNull(env);
    }
    auto package = holder->TakeNext();
    if (package == nullptr) {
        return NapiUtil::CreateNull(env);
    }
    napi_value packageObj = NapiUtil::CreateObject(env);
    NapiUtil::SetNamedProperty(env, packageObj, "packageId", NapiUtil::CreateInt32(env, package->packageId));
    NapiUtil::SetNamedProperty(env, packageObj, "row", NapiUtil::CreateInt32(env, package->row));
    NapiUtil::SetNamedProperty(env, packageObj, "size", NapiUtil::CreateInt32(env, package->size));
    NapiUtil::SetNamedProperty(env, packageObj, "data", NapiUtil::CreateStrings(env, package->data));
    NapiUtil::SetNamedProperty(env, packageObj, "appEventInfos", NapiUtil::CreateEventInfoArray(env, package->events));
    return packageObj;
}

void NapiAppEventHolder::SetRow(int row)
{
    HILOG_INFO(LOG_CORE, "hodler seq=%{public}" PRId64 " set row=%{public}d", observerSeq_, row);
    takeRow_ = row;
    hasSetRow_ = true;
}

void NapiAppEventHolder::SetSize(int size)
{
    HILOG_INFO(LOG_CORE, "hodler seq=%{public}" PRId64 " set size=%{public}d", observerSeq_, size);
    takeSize_ = size;
    hasSetSize_ = true;
}

std::shared_ptr<AppEventPackage> NapiAppEventHolder::TakeNext()
{
    std::vector<std::shared_ptr<AppEventPack>> events;
    bool shouldTakeSize = hasSetSize_ && !hasSetRow_;
    int rowNum = shouldTakeSize ? 0 : takeRow_;
    if (AppEventStore::GetInstance().QueryEvents(events, observerSeq_, rowNum) != 0) {
        HILOG_WARN(LOG_CORE, "failed to query events, seq=%{public}" PRId64, observerSeq_);
        return nullptr;
    }
    if (events.empty()) {
        HILOG_WARN(LOG_CORE, "end to query events, seq=%{public}" PRId64, observerSeq_);
        return nullptr;
    }
    std::vector<int64_t> eventSeqs;
    std::vector<std::string> eventStrs;
    size_t totalSize = 0;
    auto package = std::make_shared<AppEventPackage>();
    for (auto event : events) {
        std::string eventStr = event->GetEventStr();
        if (shouldTakeSize && static_cast<int>(totalSize + eventStr.size()) > takeSize_) {
            HILOG_INFO(LOG_CORE, "stop to take data, totalSize=%{public}zu, takeSize=%{public}d", totalSize, takeSize_);
            break;
        }
        totalSize += eventStr.size();
        eventStrs.emplace_back(eventStr);
        eventSeqs.emplace_back(event->GetSeq());
        package->events.emplace_back(event);
    }
    if (eventStrs.empty()) {
        HILOG_INFO(LOG_CORE, "take data is empty, seq=%{public}" PRId64, observerSeq_);
        return nullptr;
    }
    if (!AppEventStore::GetInstance().DeleteData(observerSeq_, eventSeqs)) {
        HILOG_INFO(LOG_CORE, "failed to delete mapping data, seq=%{public}" PRId64, observerSeq_);
        return nullptr;
    }
    package->packageId = packageId_++;
    package->row = static_cast<int>(eventStrs.size());
    package->size = static_cast<int>(totalSize);
    package->data = eventStrs;
    return package;
}
} // namespace HiviewDFX
} // namespace OHOS
