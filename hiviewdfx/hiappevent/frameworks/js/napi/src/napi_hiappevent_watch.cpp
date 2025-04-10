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
#include "napi_hiappevent_watch.h"

#include <string>

#include "app_event_observer_mgr.h"
#include "app_event_stat.h"
#include "hiappevent_base.h"
#include "hiappevent_config.h"
#include "hiappevent_verify.h"
#include "hilog/log.h"
#include "napi_app_event_holder.h"
#include "napi_app_event_watcher.h"
#include "napi_error.h"
#include "napi_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "NapiWatch"

namespace OHOS {
namespace HiviewDFX {
namespace NapiHiAppEventWatch {
namespace {
const std::string NAME_PROPERTY = "name";
const std::string COND_PROPERTY = "triggerCondition";
const std::string COND_PROPS[] = { "row", "size", "timeOut" };
const std::string FILTERS_PROPERTY = "appEventFilters";
const std::string FILTERS_DOMAIN_PROP = "domain";
const std::string FILTERS_TYPES_PROP = "eventTypes";
const std::string FILTERS_NAMES_PROP = "names";
const std::string TRIGGER_PROPERTY = "onTrigger";
const std::string RECEIVE_PROPERTY = "onReceive";
constexpr int BIT_MASK = 1;
constexpr unsigned int BIT_ALL_TYPES = 0xff;

bool IsValidName(const napi_env env, const napi_value name, int& errCode)
{
    if (name == nullptr) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg(NAME_PROPERTY));
        errCode = NapiError::ERR_PARAM;
        return false;
    }
    if (!NapiUtil::IsString(env, name)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg(NAME_PROPERTY, "string"));
        errCode = NapiError::ERR_PARAM;
        return false;
    }
    if (!IsValidWatcherName(NapiUtil::GetString(env, name))) {
        NapiUtil::ThrowError(env, NapiError::ERR_INVALID_WATCHER_NAME, "Invalid watcher name.");
        errCode = NapiError::ERR_INVALID_WATCHER_NAME;
        return false;
    }
    return true;
}

bool IsValidCondition(const napi_env env, const napi_value cond, int& errCode)
{
    if (cond == nullptr) {
        return true;
    }
    if (!NapiUtil::IsObject(env, cond)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg(COND_PROPERTY, "TriggerCondition"));
        errCode = NapiError::ERR_PARAM;
        return false;
    }
    for (auto& propName : COND_PROPS) {
        napi_value propValue = NapiUtil::GetProperty(env, cond, propName);
        if (propValue == nullptr) {
            continue;
        }
        if (!NapiUtil::IsNumber(env, propValue)) {
            NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg(propName, "number"));
            errCode = NapiError::ERR_PARAM;
            return false;
        }
    }
    return true;
}

bool IsValidFilter(const napi_env env, const napi_value filter, int& errCode)
{
    napi_value domain = NapiUtil::GetProperty(env, filter, FILTERS_DOMAIN_PROP);
    if (domain == nullptr) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg(FILTERS_DOMAIN_PROP));
        errCode = NapiError::ERR_PARAM;
        return false;
    }
    if (!NapiUtil::IsString(env, domain)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg(FILTERS_DOMAIN_PROP, "string"));
        errCode = NapiError::ERR_PARAM;
        return false;
    }
    if (!IsValidDomain(NapiUtil::GetString(env, domain))) {
        NapiUtil::ThrowError(env, NapiError::ERR_INVALID_FILTER_DOMAIN, "Invalid filtering event domain.");
        errCode = NapiError::ERR_INVALID_FILTER_DOMAIN;
        return false;
    }
    napi_value types = NapiUtil::GetProperty(env, filter, FILTERS_TYPES_PROP);
    if (types != nullptr && !NapiUtil::IsArrayType(env, types, napi_number)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg(FILTERS_TYPES_PROP, "EventType[]"));
        errCode = NapiError::ERR_PARAM;
        return false;
    }
    napi_value names = NapiUtil::GetProperty(env, filter, FILTERS_NAMES_PROP);
    if (names != nullptr && !NapiUtil::IsArrayType(env, names, napi_string)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg(FILTERS_NAMES_PROP, "string[]"));
        errCode = NapiError::ERR_PARAM;
        return false;
    }
    return true;
}

bool IsValidFilters(const napi_env env, const napi_value filters, int& errCode)
{
    if (filters == nullptr) {
        return true;
    }
    if (!NapiUtil::IsArrayType(env, filters, napi_object)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg(FILTERS_PROPERTY, "AppEventFilter[]"));
        errCode = NapiError::ERR_PARAM;
        return false;
    }

    size_t len = NapiUtil::GetArrayLength(env, filters);
    for (size_t i = 0; i < len; i++) {
        napi_value filter = NapiUtil::GetElement(env, filters, i);
        if (!IsValidFilter(env, filter, errCode)) {
            return false;
        }
    }
    return true;
}

bool IsValidTrigger(const napi_env env, const napi_value trigger, int& errCode)
{
    if (trigger == nullptr) {
        return true;
    }
    if (!NapiUtil::IsFunction(env, trigger)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg(TRIGGER_PROPERTY, "function"));
        errCode = NapiError::ERR_PARAM;
        return false;
    }
    return true;
}

bool IsValidReceive(const napi_env env, const napi_value receive, int& errCode)
{
    if (receive == nullptr) {
        return true;
    }
    if (!NapiUtil::IsFunction(env, receive)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg(RECEIVE_PROPERTY, "function"));
        errCode = NapiError::ERR_PARAM;
        return false;
    }
    return true;
}

bool IsValidWatcher(const napi_env env, const napi_value watcher, int& errCode)
{
    if (!NapiUtil::IsObject(env, watcher)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("watcher", "Watcher"));
        errCode = NapiError::ERR_PARAM;
        return false;
    }
    return IsValidName(env, NapiUtil::GetProperty(env, watcher, NAME_PROPERTY), errCode) &&
           IsValidCondition(env, NapiUtil::GetProperty(env, watcher, COND_PROPERTY), errCode) &&
           IsValidFilters(env, NapiUtil::GetProperty(env, watcher, FILTERS_PROPERTY), errCode) &&
           IsValidTrigger(env, NapiUtil::GetProperty(env, watcher, TRIGGER_PROPERTY), errCode) &&
           IsValidReceive(env, NapiUtil::GetProperty(env, watcher, RECEIVE_PROPERTY), errCode);
}

int GetConditionValue(const napi_env env, const napi_value cond, const std::string& name)
{
    if (auto value = NapiUtil::GetProperty(env, cond, name); value != nullptr) {
        return NapiUtil::GetInt32(env, value);
    }
    return 0;
}

std::string GetName(const napi_env env, const napi_value watcher)
{
    return NapiUtil::GetString(env, NapiUtil::GetProperty(env, watcher, NAME_PROPERTY));
}

TriggerCondition GetCondition(const napi_env env, const napi_value watcher)
{
    TriggerCondition resCond = {
        .row = 0,
        .size = 0,
        .timeout = 0
    };
    napi_value cond = NapiUtil::GetProperty(env, watcher, COND_PROPERTY);
    if (cond == nullptr) {
        return resCond;
    }

    size_t index = 0;
    int row = GetConditionValue(env, cond, COND_PROPS[index++]);
    if (row < 0) {
        NapiUtil::ThrowError(env, NapiError::ERR_INVALID_COND_ROW, "Invalid row value.");
        return resCond;
    }
    resCond.row = row;

    int size = GetConditionValue(env, cond, COND_PROPS[index++]);
    if (size < 0) {
        NapiUtil::ThrowError(env, NapiError::ERR_INVALID_COND_SIZE, "Invalid size value.");
        return resCond;
    }
    resCond.size = size;

    int timeout = GetConditionValue(env, cond, COND_PROPS[index++]);
    if (timeout < 0) {
        NapiUtil::ThrowError(env, NapiError::ERR_INVALID_COND_TIMEOUT, "Invalid timeout value.");
        return resCond;
    }
    resCond.timeout = timeout * HiAppEvent::TIMEOUT_STEP;
    return resCond;
}

void GetFilters(const napi_env env, const napi_value watcher, std::vector<AppEventFilter>& filters)
{
    napi_value filtersValue = NapiUtil::GetProperty(env, watcher, FILTERS_PROPERTY);
    if (filtersValue == nullptr) {
        return;
    }
    size_t len = NapiUtil::GetArrayLength(env, filtersValue);
    for (size_t i = 0; i < len; i++) {
        napi_value filterValue = NapiUtil::GetElement(env, filtersValue, i);
        std::string domain = NapiUtil::GetString(env, NapiUtil::GetProperty(env, filterValue, FILTERS_DOMAIN_PROP));
        napi_value namesValue = NapiUtil::GetProperty(env, filterValue, FILTERS_NAMES_PROP);
        std::unordered_set<std::string> names;
        if (namesValue != nullptr) {
            NapiUtil::GetStringsToSet(env, namesValue, names);
        }
        napi_value typesValue = NapiUtil::GetProperty(env, filterValue, FILTERS_TYPES_PROP);
        if (typesValue == nullptr) {
            filters.emplace_back(AppEventFilter(domain, names, BIT_ALL_TYPES));
            continue;
        }
        std::vector<int> types;
        NapiUtil::GetInt32s(env, typesValue, types);
        unsigned int filterType = 0;
        for (auto type : types) {
            if (!IsValidEventType(type)) {
                std::string errMsg = NapiUtil::CreateErrMsg(FILTERS_TYPES_PROP, "EventType[]");
                NapiUtil::ThrowError(env, NapiError::ERR_PARAM, errMsg);
                continue;
            }
            filterType |= (BIT_MASK << type);
        }
        filterType = filterType > 0 ? filterType : BIT_ALL_TYPES;
        filters.emplace_back(AppEventFilter(domain, names, filterType));
    }
}

napi_value CreateHolder(const napi_env env, size_t argc, const napi_value argv[])
{
    napi_value constructor = nullptr;
    if (napi_get_reference_value(env, NapiAppEventHolder::constructor_, &constructor) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get constructor of the holder");
        return NapiUtil::CreateNull(env);
    }
    napi_value holder = nullptr;
    if (napi_new_instance(env, constructor, argc, argv, &holder) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get new instance for holder");
        return NapiUtil::CreateNull(env);
    }
    return holder;
}
} // namespace

napi_value AddWatcher(const napi_env env, const napi_value watcher, uint64_t beginTime)
{
    int errCode = NapiError::ERR_OK;
    if (!IsValidWatcher(env, watcher, errCode)) {
        HILOG_ERROR(LOG_CORE, "invalid watcher");
        AppEventStat::WriteApiEndEvent("addWatcher", beginTime, AppEventStat::FAILED, errCode);
        return NapiUtil::CreateNull(env);
    }
    std::vector<AppEventFilter> filters;
    GetFilters(env, watcher, filters);
    std::string name = GetName(env, watcher);
    if (name.empty()) {
        HILOG_ERROR(LOG_CORE, "invalid name");
        AppEventStat::WriteApiEndEvent("addWatcher", beginTime, AppEventStat::FAILED, NapiError::ERR_OK);
        return NapiUtil::CreateNull(env);
    }
    TriggerCondition cond = GetCondition(env, watcher);
    auto watcherPtr = std::make_shared<NapiAppEventWatcher>(name, filters, cond);
    napi_value trigger = NapiUtil::GetProperty(env, watcher, TRIGGER_PROPERTY);
    if (trigger != nullptr) {
        watcherPtr->InitTrigger(env, trigger);
    }
    napi_value receiver = NapiUtil::GetProperty(env, watcher, RECEIVE_PROPERTY);
    if (receiver != nullptr) {
        watcherPtr->InitReceiver(env, receiver);
    }
    int64_t observerSeq = AppEventObserverMgr::GetInstance().RegisterObserver(watcherPtr);
    if (observerSeq <= 0) {
        HILOG_ERROR(LOG_CORE, "invalid observer sequence");
        AppEventStat::WriteApiEndEvent("addWatcher", beginTime, AppEventStat::FAILED, NapiError::ERR_OK);
        return NapiUtil::CreateNull(env);
    }
    constexpr size_t holderParamNum = 2;
    napi_value holderParams[holderParamNum] = {
        NapiUtil::CreateString(env, name),
        NapiUtil::CreateInt64(env, observerSeq)
    };
    napi_value holder = CreateHolder(env, holderParamNum, holderParams);
    watcherPtr->InitHolder(env, holder);
    AppEventStat::WriteApiEndEvent("addWatcher", beginTime, AppEventStat::SUCCESS, NapiError::ERR_OK);
    return holder;
}

napi_value RemoveWatcher(const napi_env env, const napi_value watcher, uint64_t beginTime)
{
    if (!NapiUtil::IsObject(env, watcher)) {
        AppEventStat::WriteApiEndEvent("removeWatcher", beginTime, AppEventStat::FAILED, NapiError::ERR_PARAM);
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("watcher", "Watcher"));
        return NapiUtil::CreateUndefined(env);
    }
    int errCode = NapiError::ERR_OK;
    if (!IsValidName(env, NapiUtil::GetProperty(env, watcher, NAME_PROPERTY), errCode)) {
        AppEventStat::WriteApiEndEvent("removeWatcher", beginTime, AppEventStat::FAILED, errCode);
        return NapiUtil::CreateUndefined(env);
    }
    (void)AppEventObserverMgr::GetInstance().UnregisterObserver(GetName(env, watcher), ObserverType::WATCHER);
    AppEventStat::WriteApiEndEvent("removeWatcher", beginTime, AppEventStat::SUCCESS, NapiError::ERR_OK);
    return NapiUtil::CreateUndefined(env);
}
} // namespace NapiHiAppEventWatch
} // namespace HiviewDFX
} // namespace OHOS
