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
#define LOG_TAG "RdbUtils"

#include "rdb_utils.h"
#include "log.h"
#include "raw_data_parser.h"

using namespace OHOS::RdbDataShareAdapter;
using namespace OHOS::DataShare;
using namespace OHOS::NativeRdb;

constexpr RdbUtils::OperateHandler RdbUtils::HANDLERS[LAST_TYPE];

RdbPredicates RdbUtils::ToPredicates(const DataShareAbsPredicates &predicates, const std::string &table)
{
    RdbPredicates rdbPredicates(table);
    if (predicates.GetSettingMode() == QUERY_LANGUAGE) {
        rdbPredicates.SetWhereClause(predicates.GetWhereClause());
        rdbPredicates.SetWhereArgs(predicates.GetWhereArgs());
        rdbPredicates.SetOrder(predicates.GetOrder());
    }

    const auto &operations = predicates.GetOperationList();
    for (const auto &oper : operations) {
        if (oper.operation >= 0 && oper.operation < LAST_TYPE) {
            (*HANDLERS[oper.operation])(oper, rdbPredicates);
        }
    }
    return rdbPredicates;
}

OHOS::NativeRdb::ValueObject RdbUtils::ToValueObject(const DataSharePredicatesObject &predicatesObject)
{
    if (auto *val = std::get_if<int>(&predicatesObject.value)) {
        return ValueObject(*val);
    }
    ValueObject::Type value;
    RawDataParser::Convert(std::move(predicatesObject.value), value);
    return value;
}

void RdbUtils::NoSupport(const OperationItem &item, RdbPredicates &query)
{
    LOGE("invalid operation:%{public}d", item.operation);
}

void RdbUtils::EqualTo(const OperationItem &item, RdbPredicates &predicates)
{
    predicates.EqualTo(item.GetSingle(0), ToValueObject(item.singleParams[1]));
}

void RdbUtils::NotEqualTo(const OperationItem &item, RdbPredicates &predicates)
{
    predicates.NotEqualTo(item.GetSingle(0), ToValueObject(item.singleParams[1]));
}

void RdbUtils::GreaterThan(const OperationItem &item, RdbPredicates &predicates)
{
    predicates.GreaterThan(item.GetSingle(0), ToValueObject(item.singleParams[1]));
}

void RdbUtils::LessThan(const OperationItem &item, RdbPredicates &predicates)
{
    predicates.LessThan(item.GetSingle(0), ToValueObject(item.singleParams[1]));
}

void RdbUtils::GreaterThanOrEqualTo(const OperationItem &item, RdbPredicates &predicates)
{
    predicates.GreaterThanOrEqualTo(item.GetSingle(0), ToValueObject(item.singleParams[1]));
}

void RdbUtils::LessThanOrEqualTo(const OperationItem &item, RdbPredicates &predicates)
{
    predicates.LessThanOrEqualTo(item.GetSingle(0), ToValueObject(item.singleParams[1]));
}

void RdbUtils::And(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.And();
}

void RdbUtils::Or(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.Or();
}

void RdbUtils::IsNull(const OperationItem &item, RdbPredicates &predicates)
{
    predicates.IsNull(item.GetSingle(0));
}

void RdbUtils::IsNotNull(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.IsNotNull(item.GetSingle(0));
}

void RdbUtils::In(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.In(item.GetSingle(0), MutliValue(item.multiParams[0]));
}

void RdbUtils::NotIn(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.NotIn(item.GetSingle(0), MutliValue(item.multiParams[0]));
}

void RdbUtils::Like(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.Like(item.GetSingle(0), ToValueObject(item.singleParams[1]));
}

void RdbUtils::NotLike(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.NotLike(item.GetSingle(0), ToValueObject(item.singleParams[1]));
}

void RdbUtils::OrderByAsc(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.OrderByAsc(item.GetSingle(0));
}

void RdbUtils::OrderByDesc(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.OrderByDesc(item.GetSingle(0));
}

void RdbUtils::Limit(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.Limit(item.GetSingle(0));
    predicates.Offset(item.GetSingle(1));
}

void RdbUtils::Offset(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.Offset(item.GetSingle(0));
}

void RdbUtils::BeginWrap(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.BeginWrap();
}

void RdbUtils::EndWrap(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.EndWrap();
}

void RdbUtils::BeginsWith(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.BeginsWith(item.GetSingle(0), ToValueObject(item.singleParams[1]));
}

void RdbUtils::EndsWith(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.EndsWith(item.GetSingle(0), ToValueObject(item.singleParams[1]));
}

void RdbUtils::Distinct(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.Distinct();
}

void RdbUtils::GroupBy(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.GroupBy(MutliValue(item.multiParams[0]));
}

void RdbUtils::IndexedBy(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.IndexedBy(item.GetSingle(0));
}

void RdbUtils::Contains(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.Contains(item.GetSingle(0), ToValueObject(item.singleParams[1]));
}

void RdbUtils::NotContains(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.NotContains(item.GetSingle(0), ToValueObject(item.singleParams[1]));
}


void RdbUtils::Glob(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.Glob(item.GetSingle(0), ToValueObject(item.singleParams[1]));
}

void RdbUtils::Between(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    // singleParams[2] is another param
    predicates.Between(item.GetSingle(0), ToValueObject(item.singleParams[1]), ToValueObject(item.singleParams[2]));
}

void RdbUtils::NotBetween(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    // singleParams[2] is another param
    predicates.NotBetween(item.GetSingle(0), ToValueObject(item.singleParams[1]), ToValueObject(item.singleParams[2]));
}

void RdbUtils::CrossJoin(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.CrossJoin(item.GetSingle(0));
}

void RdbUtils::InnerJoin(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.InnerJoin(item.GetSingle(0));
}

void RdbUtils::LeftOuterJoin(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.LeftOuterJoin(item.GetSingle(0));
}

void RdbUtils::Using(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.Using(MutliValue(item.multiParams[0]));
}

void RdbUtils::On(const DataShare::OperationItem &item, RdbPredicates &predicates)
{
    predicates.On(MutliValue(item.multiParams[0]));
}

RdbUtils::RdbUtils()
{
}

RdbUtils::~RdbUtils()
{
}
