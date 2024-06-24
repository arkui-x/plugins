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

#ifndef DATASHARE_PREDICATES_H
#define DATASHARE_PREDICATES_H

#include <string>

#include "datashare_abs_predicates.h"
#include "datashare_errno.h"
#include "datashare_predicates_object.h"
#include "datashare_predicates_objects.h"

namespace OHOS {
namespace DataShare {
#define EXPORT __attribute__ ((visibility("default")))
class DataSharePredicates : public DataShareAbsPredicates {
public:

    /**
     * @brief Constructor.
     */
    EXPORT DataSharePredicates()
    {
    }

    /**
     * @brief Constructor.
     *
     * A parameterized constructor used to create an DataSharePredicates instance.
     *
     * @param OperationList Indicates the operation list of the database.
     */
    EXPORT explicit DataSharePredicates(std::vector<OperationItem> operList) : operations_(std::move(operList))
    {
    }

    /**
     * @brief Destructor.
     */
    EXPORT ~DataSharePredicates()
    {
    }

    /**
     * @brief The EqualTo of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    EXPORT DataSharePredicates *EqualTo(const std::string &field, const SingleValue &value);

    /**
     * @brief The NotEqualTo of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    EXPORT DataSharePredicates *NotEqualTo(const std::string &field, const SingleValue &value);

    /**
     * @brief The GreaterThan of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    EXPORT DataSharePredicates *GreaterThan(const std::string &field, const SingleValue &value);

    /**
     * @brief The LessThan of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    EXPORT DataSharePredicates *LessThan(const std::string &field, const SingleValue &value);

    /**
     * @brief The GreaterThanOrEqualTo of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    EXPORT DataSharePredicates *GreaterThanOrEqualTo(const std::string &field, const SingleValue &value);

    /**
     * @brief The LessThanOrEqualTo of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    EXPORT DataSharePredicates *LessThanOrEqualTo(const std::string &field, const SingleValue &value);

    /**
     * @brief The In of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    EXPORT DataSharePredicates *In(const std::string &field, const MutliValue &values);

    /**
     * @brief The NotIn of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    EXPORT DataSharePredicates *NotIn(const std::string &field, const MutliValue &values);

    /**
     * @brief BeginWrap.
     */
    EXPORT DataSharePredicates *BeginWrap();

    /**
     * @brief EndWrap.
     */
    EXPORT DataSharePredicates *EndWrap();

    /**
     * @brief Or.
     */
    EXPORT DataSharePredicates *Or();

    /**
     * @brief And.
     */
    EXPORT DataSharePredicates *And();

    /**
     * @brief The Contains of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    EXPORT DataSharePredicates *Contains(const std::string &field, const std::string &value);

    /**
     * @brief The BeginsWith of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    EXPORT DataSharePredicates *BeginsWith(const std::string &field, const std::string &value);

    /**
     * @brief The EndsWith of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    EXPORT DataSharePredicates *EndsWith(const std::string &field, const std::string &value);

    /**
     * @brief The IsNull of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    EXPORT DataSharePredicates *IsNull(const std::string &field);

    /**
     * @brief The IsNotNull of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    EXPORT DataSharePredicates *IsNotNull(const std::string &field);

    /**
     * @brief The Like of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    EXPORT DataSharePredicates *Like(const std::string &field, const std::string &value);

    /**
     * @brief The Unlike of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    EXPORT DataSharePredicates *Unlike(const std::string &field, const std::string &value);

    /**
     * @brief The Glob of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    EXPORT DataSharePredicates *Glob(const std::string &field, const std::string &value);

    /**
     * @brief The Between of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    EXPORT DataSharePredicates *Between(const std::string &field, const std::string &low, const std::string &high);

    /**
     * @brief The NotBetween of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    EXPORT DataSharePredicates *NotBetween(const std::string &field, const std::string &low, const std::string &high);

    /**
     * @brief The OrderByAsc of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    EXPORT DataSharePredicates *OrderByAsc(const std::string &field);

    /**
     * @brief The OrderByDesc of the predicate.
     *
     * @param field Indicates the target field.
     * @param value Indicates the queried value.
     */
    EXPORT DataSharePredicates *OrderByDesc(const std::string &field);

    /**
     * @brief Distinct predicate condition.
     */
    EXPORT DataSharePredicates *Distinct();

    /**
     * @brief The Limit of the predicate.
     *
     * @param number Indicates the target number.
     * @param offset Indicates the queried value.
     */
    EXPORT DataSharePredicates *Limit(const int number, const int offset);

    /**
     * @brief The GroupBy of the predicate.
     *
     * @param field Indicates the target field.
     */
    EXPORT DataSharePredicates *GroupBy(const std::vector<std::string> &fields);

    /**
     * @brief The IndexedBy of the predicate.
     *
     * @param indexName indicates the query condition.
     */
    EXPORT DataSharePredicates *IndexedBy(const std::string &indexName);

    /**
     * @brief The KeyPrefix of the predicate.
     *
     * @param Search by prefix conditions.
     */
    EXPORT DataSharePredicates *KeyPrefix(const std::string &prefix);

    /**
     * @brief The InKeys of the predicate.
     *
     * @param Query based on key conditions.
     */
    EXPORT DataSharePredicates *InKeys(const std::vector<std::string> &keys);

    /**
     * @brief The CrossJoin of the predicate.
     *
     * @param tableName indicates the query condition.
     */
    EXPORT DataSharePredicates *CrossJoin(const std::string &tableName);

    /**
     * @brief The InnerJoin of the predicate.
     *
     * @param tableName indicates the query condition.
     */
    EXPORT DataSharePredicates *InnerJoin(const std::string &tableName);

    /**
     * @brief The LeftOuterJoin of the predicate.
     *
     * @param tableName indicates the query condition.
     */
    EXPORT DataSharePredicates *LeftOuterJoin(const std::string &tableName);

    /**
     * @brief The Using of the predicate.
     *
     * @param field Indicates the target field.
     */
    EXPORT DataSharePredicates *Using(const std::vector<std::string> &fields);

    /**
     * @brief The On of the predicate.
     *
     * @param field Indicates the target field.
     */
    EXPORT DataSharePredicates *On(const std::vector<std::string> &fields);

    /**
     * @brief The GetOperationList of the predicate.
     */
    EXPORT const std::vector<OperationItem> &GetOperationList() const;

    /**
     * @brief The GetWhereClause of the predicate.
     */
    EXPORT std::string GetWhereClause() const;

    /**
     * @brief The SetWhereClause of the predicate.
     *
     * @param Query based on the whereClause.
     */
    EXPORT int SetWhereClause(const std::string &whereClause);

    /**
     * @brief The GetWhereArgs of the predicate.
     */
    EXPORT std::vector<std::string> GetWhereArgs() const;

    /**
     * @brief The SetWhereArgs of the predicate.
     *
     * @param Query based on whereArgs conditions.
     */
    EXPORT int SetWhereArgs(const std::vector<std::string> &whereArgs);

    /**
     * @brief The GetOrder of the predicate.
     */
    EXPORT std::string GetOrder() const;

    /**
     * @brief The SetOrder of the predicate.
     *
     * @param Query based on order conditions..
     */
    EXPORT int SetOrder(const std::string &order);

    /**
     * @brief The GetSettingMode of the predicate.
     */
    EXPORT int16_t GetSettingMode() const;

    /**
     * @brief The SetSettingMode of the predicate.
     */
    EXPORT void SetSettingMode(int16_t settingMode);

private:
    void SetOperationList(OperationType operationType, const MutliValue &param);

    void SetOperationList(
        OperationType operationType, const SingleValue &param1, const MutliValue &param2);

    void SetOperationList(OperationType operationType, const SingleValue &para1 = {},
        const SingleValue &para2 = {}, const SingleValue &para3 = {});

    void ClearQueryLanguage();
    std::vector<OperationItem> operations_;
    std::string whereClause_;
    std::vector<std::string> whereArgs_;
    std::string order_;
    int16_t settingMode_ = {};
};
} // namespace DataShare
} // namespace OHOS
#endif