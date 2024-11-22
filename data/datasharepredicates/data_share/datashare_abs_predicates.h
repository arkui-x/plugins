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

#ifndef DATASHARE_ABSPREDICATES_H
#define DATASHARE_ABSPREDICATES_H

#include <memory>
#include <string>
#include <vector>
#include <list>
#include "datashare_predicates_def.h"

namespace OHOS {
namespace DataShare {
class DataShareAbsPredicates {
public:

    /**
     * @brief Destructor.
     */
    virtual ~DataShareAbsPredicates() {}

    /**
     * @brief The EqualTo of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    virtual DataShareAbsPredicates *EqualTo(const std::string &field, const SingleValue &value) = 0;

    /**
     * @brief The NotEqualTo of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    virtual DataShareAbsPredicates *NotEqualTo(const std::string &field, const SingleValue &value) = 0;

    /**
     * @brief The GreaterThan of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    virtual DataShareAbsPredicates *GreaterThan(const std::string &field, const SingleValue &value) = 0;

    /**
     * @brief The LessThan of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    virtual DataShareAbsPredicates *LessThan(const std::string &field, const SingleValue &value) = 0;

    /**
     * @brief The GreaterThanOrEqualTo of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    virtual DataShareAbsPredicates *GreaterThanOrEqualTo(const std::string &field, const SingleValue &value) = 0;

    /**
     * @brief The LessThanOrEqualTo of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    virtual DataShareAbsPredicates *LessThanOrEqualTo(const std::string &field, const SingleValue &value) = 0;

    /**
     * @brief The In of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    virtual DataShareAbsPredicates *In(const std::string &field, const MutliValue &value) = 0;

    /**
     * @brief The NotIn of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    virtual DataShareAbsPredicates *NotIn(const std::string &field, const MutliValue &value) = 0;

    /**
     * @brief BeginWrap.
     */
    virtual DataShareAbsPredicates *BeginWrap() = 0;

    /**
     * @brief EndWrap.
     */
    virtual DataShareAbsPredicates *EndWrap() = 0;

    /**
     * @brief Or.
     */
    virtual DataShareAbsPredicates *Or() = 0;

    /**
     * @brief And.
     */
    virtual DataShareAbsPredicates *And() = 0;

    /**
     * @brief The Contains of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    virtual DataShareAbsPredicates *Contains(const std::string &field, const std::string &value) = 0;

    /**
     * @brief The BeginsWith of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    virtual DataShareAbsPredicates *BeginsWith(const std::string &field, const std::string &value) = 0;

    /**
     * @brief The EndsWith of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    virtual DataShareAbsPredicates *EndsWith(const std::string &field, const std::string &value) = 0;

    /**
     * @brief The IsNull of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    virtual DataShareAbsPredicates *IsNull(const std::string &field) = 0;

    /**
     * @brief The IsNotNull of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    virtual DataShareAbsPredicates *IsNotNull(const std::string &field) = 0;

    /**
     * @brief The Like of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    virtual DataShareAbsPredicates *Like(const std::string &field, const std::string &value) = 0;

    /**
     * @brief The Unlike of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    virtual DataShareAbsPredicates *Unlike(const std::string &field, const std::string &value) = 0;

    /**
     * @brief The Glob of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    virtual DataShareAbsPredicates *Glob(const std::string &field, const std::string &value) = 0;

    /**
     * @brief The Between of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    virtual DataShareAbsPredicates *Between(const std::string &field,
        const std::string &low, const std::string &high) = 0;

    /**
     * @brief The NotBetween of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    virtual DataShareAbsPredicates *NotBetween(const std::string &field,
        const std::string &low, const std::string &high) = 0;

    /**
     * @brief The OrderByAsc of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    virtual DataShareAbsPredicates *OrderByAsc(const std::string &field) = 0;

    /**
     * @brief The OrderByDesc of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    virtual DataShareAbsPredicates *OrderByDesc(const std::string &field) = 0;

    /**
     * @brief Distinct predicate condition.
     */
    virtual DataShareAbsPredicates *Distinct() = 0;

    /**
     * @brief The Limit of the predicate.
     *
     * @param number Indicates the target number.
     * @param offset Indicates the queried value.
     */
    virtual DataShareAbsPredicates *Limit(const int number, const int offset) = 0;

    /**
     * @brief The GroupBy of the predicate.
     *
     * @param field Indicates the target field.
     */
    virtual DataShareAbsPredicates *GroupBy(const std::vector<std::string> &fields) = 0;

    /**
     * @brief The IndexedBy of the predicate.
     *
     * @param indexName indicates the query condition.
     */
    virtual DataShareAbsPredicates *IndexedBy(const std::string &indexName) = 0;

    /**
     * @brief The KeyPrefix of the predicate.
     *
     * @param Search by prefix conditions.
     */
    virtual DataShareAbsPredicates *KeyPrefix(const std::string &prefix) = 0;

    /**
     * @brief The InKeys of the predicate.
     *
     * @param Query based on key conditions.
     */
    virtual DataShareAbsPredicates *InKeys(const std::vector<std::string> &keys) = 0;

    /**
     * @brief The CrossJoin of the predicate.
     *
     * @param tableName indicates the query condition.
     */
    virtual DataShareAbsPredicates *CrossJoin(const std::string &tableName) = 0;

    /**
     * @brief The InnerJoin of the predicate.
     *
     * @param tableName indicates the query condition.
     */
    virtual DataShareAbsPredicates *InnerJoin(const std::string &tableName) = 0;

    /**
     * @brief The LeftOuterJoin of the predicate.
     *
     * @param tableName indicates the query condition.
     */
    virtual DataShareAbsPredicates *LeftOuterJoin(const std::string &tableName) = 0;

    /**
     * @brief The Using of the predicate.
     *
     * @param field Indicates the target field.
     */
    virtual DataShareAbsPredicates *Using(const std::vector<std::string> &fields) = 0;

    /**
     * @brief The On of the predicate.
     *
     * @param field Indicates the target field.
     */
    virtual DataShareAbsPredicates *On(const std::vector<std::string> &fields) = 0;

    /**
     * @brief The GetOperationList of the predicate.
     */
    virtual const std::vector<OperationItem>& GetOperationList() const = 0;

    /**
     * @brief The GetWhereClause of the predicate.
     */
    virtual std::string GetWhereClause() const = 0;

    /**
     * @brief The SetWhereClause of the predicate.
     *
     * @param Query based on the whereClause.
     */
    virtual int SetWhereClause(const std::string &whereClause) = 0;

    /**
     * @brief The GetWhereArgs of the predicate.
     */
    virtual std::vector<std::string> GetWhereArgs() const = 0;

    /**
     * @brief The SetWhereArgs of the predicate.
     *
     * @param Query based on whereArgs conditions.
     */
    virtual int SetWhereArgs(const std::vector<std::string> &whereArgs) = 0;

    /**
     * @brief The GetOrder of the predicate.
     */
    virtual std::string GetOrder() const = 0;

    /**
     * @brief The SetOrder of the predicate.
     *
     * @param Query based on order conditions..
     */
    virtual int SetOrder(const std::string &order) = 0;

    /**
     * @brief The GetSettingMode of the predicate.
     */
    virtual int16_t GetSettingMode() const = 0;
};
} // namespace DataShare
} // namespace OHOS

#endif