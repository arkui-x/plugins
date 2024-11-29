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
#include "rdb_utils_ios.h"
#include "log.h"
#include "raw_data_parser.h"

using namespace OHOS::Media;
using namespace OHOS::DataShare;

constexpr RdbUtilsIos::OperateHandler RdbUtilsIos::HANDLERS[LAST_TYPE];

std::optional<RdbUtilsResult> RdbUtilsIos::ToPredicates(const DataShareAbsPredicates &predicates, const bool isAlbum)
{
    RdbUtilsResult rdbUtilsResult(isAlbum);

    const auto &operations = predicates.GetOperationList();
    for (const auto &oper : operations) {
        if (oper.operation >= 0 && oper.operation < LAST_TYPE) {
            if((*HANDLERS[oper.operation])(oper, rdbUtilsResult)) {
                return std::nullopt;
            }
        }
    }
    return rdbUtilsResult;
}

OHOS::NativeRdb::ValueObject RdbUtilsIos::ToValueObject(const DataSharePredicatesObject &predicatesObject)
{
    if (auto *val = std::get_if<int>(&predicatesObject.value)) {
        return ValueObject(*val);
    }
    ValueObject::Type value;
    RawDataParser::Convert(std::move(predicatesObject.value), value);
    return value;
}

bool RdbUtilsIos::NoSupport(const OperationItem &item, RdbUtilsResult &result)
{
    LOGE("invalid operation:%{public}d", item.operation);
    return false;
}

bool RdbUtilsIos::EqualTo(const OperationItem &item, RdbUtilsResult &result)
{
    return result.EqualTo(item.GetSingle(0), ToValueObject(item.singleParams[1]));
}

bool RdbUtilsIos::And(const DataShare::OperationItem &item, RdbUtilsResult &result)
{
    return result.And();
}

bool RdbUtilsIos::In(const DataShare::OperationItem &item, RdbUtilsResult &result)
{
    return result.In(item.GetSingle(0), MutliValue(item.multiParams[0]));
}

bool RdbUtilsIos::OrderByAsc(const DataShare::OperationItem &item, RdbUtilsResult &result)
{
    return result.OrderByAsc(item.GetSingle(0));
}

bool RdbUtilsIos::OrderByDesc(const DataShare::OperationItem &item, RdbUtilsResult &result)
{
    return result.OrderByDesc(item.GetSingle(0));
}

bool RdbUtilsIos::Limit(const DataShare::OperationItem &item, RdbUtilsResult &result)
{
    return result.Limit(item.GetSingle(0)) || result.Offset(item.GetSingle(1));
}

RdbUtilsIos::RdbUtilsIos()
{
}

RdbUtilsIos::~RdbUtilsIos()
{
}
