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

#ifndef RDB_UTILS_H
#define RDB_UTILS_H

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "foundation/distributeddatamgr/relational_store/interfaces/inner_api/rdb/include/result_set.h"
#include "abs_predicates.h"
#include "datashare_abs_predicates.h"
#include "rdb_predicates.h"
#include "value_object.h"

namespace OHOS {
namespace RdbDataShareAdapter {
/**
 * The RdbUtils class of RDB.
 */
class API_EXPORT RdbUtils {
public:
    /**
     * @brief Use RdbPredicates replace NativeRdb::RdbPredicates namespace.
     */
    using RdbPredicates = NativeRdb::RdbPredicates;

    /**
     * @brief Use ResultSet replace NativeRdb::ResultSet namespace.
     */
    using ResultSet = NativeRdb::ResultSet;

    /**
     * @brief Use DataShareAbsPredicates replace DataShare::DataShareAbsPredicates namespace.
     */
    using DataShareAbsPredicates = DataShare::DataShareAbsPredicates;

    /**
     * @brief Use OperationItem replace DataShare::OperationItem namespace.
     */
    using OperationItem = DataShare::OperationItem;

    /**
     * @brief Use DataSharePredicatesObject replace DataShare::SingleValue namespace.
     */
    using DataSharePredicatesObject = DataShare::SingleValue;

    /**
     * @brief Convert DataShare::DataShareAbsPredicates to NativeRdb::RdbPredicates.
     *
     * @param table Indicates the table name.
     */
    API_EXPORT static RdbPredicates ToPredicates(
        const DataShareAbsPredicates &predicates, const std::string &table);

    API_EXPORT static OHOS::NativeRdb::ValueObject ToValueObject(const DataSharePredicatesObject &predicatesObject);

private:
    static void NoSupport(const OperationItem &item, RdbPredicates &query);
    static void EqualTo(const OperationItem &item, RdbPredicates &predicates);
    static void NotEqualTo(const OperationItem &item, RdbPredicates &predicates);
    static void GreaterThan(const OperationItem &item, RdbPredicates &predicates);
    static void LessThan(const OperationItem &item, RdbPredicates &predicates);
    static void GreaterThanOrEqualTo(const OperationItem &item, RdbPredicates &predicates);
    static void LessThanOrEqualTo(const OperationItem &item, RdbPredicates &predicates);
    static void And(const OperationItem &item, RdbPredicates &predicates);
    static void Or(const OperationItem &item, RdbPredicates &predicates);
    static void IsNull(const OperationItem &item, RdbPredicates &predicates);
    static void IsNotNull(const OperationItem &item, RdbPredicates &predicates);
    static void In(const OperationItem &item, RdbPredicates &predicates);
    static void NotIn(const OperationItem &item, RdbPredicates &predicates);
    static void Like(const OperationItem &item, RdbPredicates &predicates);
    static void NotLike(const OperationItem &item, RdbPredicates &predicates);
    static void OrderByAsc(const OperationItem &item, RdbPredicates &predicates);
    static void OrderByDesc(const OperationItem &item, RdbPredicates &predicates);
    static void Limit(const OperationItem &item, RdbPredicates &predicates);
    static void Offset(const OperationItem &item, RdbPredicates &predicates);
    static void BeginWrap(const OperationItem &item, RdbPredicates &predicates);
    static void EndWrap(const OperationItem &item, RdbPredicates &predicates);
    static void BeginsWith(const OperationItem &item, RdbPredicates &predicates);
    static void EndsWith(const OperationItem &item, RdbPredicates &predicates);
    static void Distinct(const OperationItem &item, RdbPredicates &predicates);
    static void GroupBy(const OperationItem &item, RdbPredicates &predicates);
    static void IndexedBy(const OperationItem &item, RdbPredicates &predicates);
    static void Contains(const OperationItem &item, RdbPredicates &predicates);
    static void NotContains(const OperationItem &item, RdbPredicates &predicates);
    static void Glob(const OperationItem &item, RdbPredicates &predicates);
    static void Between(const OperationItem &item, RdbPredicates &predicates);
    static void NotBetween(const OperationItem &item, RdbPredicates &predicates);
    static void CrossJoin(const OperationItem &item, RdbPredicates &predicates);
    static void InnerJoin(const OperationItem &item, RdbPredicates &predicates);
    static void LeftOuterJoin(const OperationItem &item, RdbPredicates &predicates);
    static void Using(const OperationItem &item, RdbPredicates &predicates);
    static void On(const OperationItem &item, RdbPredicates &predicates);
    RdbUtils();
    ~RdbUtils();

    using OperateHandler = void (*)(const OperationItem &, RdbPredicates &);
    static constexpr OperateHandler HANDLERS[DataShare::LAST_TYPE] = {
        [DataShare::INVALID_OPERATION] = &RdbUtils::NoSupport,
        [DataShare::EQUAL_TO] = &RdbUtils::EqualTo,
        [DataShare::NOT_EQUAL_TO] = &RdbUtils::NotEqualTo,
        [DataShare::GREATER_THAN] = &RdbUtils::GreaterThan,
        [DataShare::LESS_THAN] = &RdbUtils::LessThan,
        [DataShare::GREATER_THAN_OR_EQUAL_TO] = &RdbUtils::GreaterThanOrEqualTo,
        [DataShare::LESS_THAN_OR_EQUAL_TO] = &RdbUtils::LessThanOrEqualTo,
        [DataShare::AND] = &RdbUtils::And,
        [DataShare::OR] = &RdbUtils::Or,
        [DataShare::IS_NULL] = &RdbUtils::IsNull,
        [DataShare::IS_NOT_NULL] = &RdbUtils::IsNotNull,
        [DataShare::SQL_IN] = &RdbUtils::In,
        [DataShare::NOT_IN] = &RdbUtils::NotIn,
        [DataShare::LIKE] = &RdbUtils::Like,
        [DataShare::UNLIKE] = &RdbUtils::NoSupport,
        [DataShare::ORDER_BY_ASC] = &RdbUtils::OrderByAsc,
        [DataShare::ORDER_BY_DESC] = &RdbUtils::OrderByDesc,
        [DataShare::LIMIT] = &RdbUtils::Limit,
        [DataShare::OFFSET] = &RdbUtils::Offset,
        [DataShare::BEGIN_WARP] = &RdbUtils::BeginWrap,
        [DataShare::END_WARP] = &RdbUtils::EndWrap,
        [DataShare::BEGIN_WITH] = &RdbUtils::BeginsWith,
        [DataShare::END_WITH] = &RdbUtils::EndsWith,
        [DataShare::IN_KEY] = &RdbUtils::NoSupport,
        [DataShare::DISTINCT] = &RdbUtils::Distinct,
        [DataShare::GROUP_BY] = &RdbUtils::GroupBy,
        [DataShare::INDEXED_BY] = &RdbUtils::IndexedBy,
        [DataShare::CONTAINS] = &RdbUtils::Contains,
        [DataShare::GLOB] = &RdbUtils::Glob,
        [DataShare::BETWEEN] = &RdbUtils::Between,
        [DataShare::NOTBETWEEN] = &RdbUtils::NotBetween,
        [DataShare::KEY_PREFIX] = &RdbUtils::NoSupport,
        [DataShare::CROSSJOIN] = &RdbUtils::CrossJoin,
        [DataShare::INNERJOIN] = &RdbUtils::InnerJoin,
        [DataShare::LEFTOUTERJOIN] = &RdbUtils::LeftOuterJoin,
        [DataShare::USING] = &RdbUtils::Using,
        [DataShare::ON] = &RdbUtils::On,
    };
};
} // namespace RdbDataShareAdapter
} // namespace OHOS
#endif // RDB_UTILS_H
