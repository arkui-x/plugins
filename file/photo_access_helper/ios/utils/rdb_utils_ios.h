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

#ifndef RDB_UTILS_IOS_H
#define RDB_UTILS_IOS_H

#include <string>
#include "datashare_abs_predicates.h"
#include "value_object.h"
#include "rdb_utils_result.h"

namespace OHOS {
namespace Media {

/**
 * The RdbUtilsIos class of RDB.
 */
class API_EXPORT RdbUtilsIos {
public:

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
     * @brief Convert DataShare::DataShareAbsPredicates to RdbUtilsResult.
     *
     * @param table Indicates the table name.
     */
    API_EXPORT static std::optional<RdbUtilsResult> ToPredicates(
        const DataShareAbsPredicates &predicates, const bool isAlbum);

private:
    static bool NoSupport(const OperationItem &item, RdbUtilsResult &result);
    static bool EqualTo(const OperationItem &item, RdbUtilsResult &result);
    static bool And(const OperationItem &item, RdbUtilsResult &result);
    static bool In(const OperationItem &item, RdbUtilsResult &result);
    static bool OrderByAsc(const OperationItem &item, RdbUtilsResult &result);
    static bool OrderByDesc(const OperationItem &item, RdbUtilsResult &result);
    static bool Limit(const OperationItem &item, RdbUtilsResult &result);
    static bool Offset(const OperationItem &item, RdbUtilsResult &result);
    RdbUtilsIos();
    ~RdbUtilsIos();
    static OHOS::NativeRdb::ValueObject ToValueObject(const DataSharePredicatesObject &predicatesObject);
    using OperateHandler = bool (*)(const OperationItem &, RdbUtilsResult &);
    static constexpr OperateHandler HANDLERS[DataShare::LAST_TYPE] = {
        [DataShare::INVALID_OPERATION] = &RdbUtilsIos::NoSupport,
        [DataShare::EQUAL_TO] = &RdbUtilsIos::EqualTo,
        [DataShare::NOT_EQUAL_TO] = &RdbUtilsIos::NoSupport,
        [DataShare::GREATER_THAN] = &RdbUtilsIos::NoSupport,
        [DataShare::LESS_THAN] = &RdbUtilsIos::NoSupport,
        [DataShare::GREATER_THAN_OR_EQUAL_TO] = &RdbUtilsIos::NoSupport,
        [DataShare::LESS_THAN_OR_EQUAL_TO] = &RdbUtilsIos::NoSupport,
        [DataShare::AND] = &RdbUtilsIos::And,
        [DataShare::OR] = &RdbUtilsIos::NoSupport,
        [DataShare::IS_NULL] = &RdbUtilsIos::NoSupport,
        [DataShare::IS_NOT_NULL] = &RdbUtilsIos::NoSupport,
        [DataShare::SQL_IN] = &RdbUtilsIos::In,
        [DataShare::NOT_IN] = &RdbUtilsIos::NoSupport,
        [DataShare::LIKE] = &RdbUtilsIos::NoSupport,
        [DataShare::UNLIKE] = &RdbUtilsIos::NoSupport,
        [DataShare::ORDER_BY_ASC] = &RdbUtilsIos::OrderByAsc,
        [DataShare::ORDER_BY_DESC] = &RdbUtilsIos::OrderByDesc,
        [DataShare::LIMIT] = &RdbUtilsIos::Limit,
        [DataShare::OFFSET] = &RdbUtilsIos::NoSupport,
        [DataShare::BEGIN_WARP] = &RdbUtilsIos::NoSupport,
        [DataShare::END_WARP] = &RdbUtilsIos::NoSupport,
        [DataShare::BEGIN_WITH] = &RdbUtilsIos::NoSupport,
        [DataShare::END_WITH] = &RdbUtilsIos::NoSupport,
        [DataShare::IN_KEY] = &RdbUtilsIos::NoSupport,
        [DataShare::DISTINCT] = &RdbUtilsIos::NoSupport,
        [DataShare::GROUP_BY] = &RdbUtilsIos::NoSupport,
        [DataShare::INDEXED_BY] = &RdbUtilsIos::NoSupport,
        [DataShare::CONTAINS] = &RdbUtilsIos::NoSupport,
        [DataShare::GLOB] = &RdbUtilsIos::NoSupport,
        [DataShare::BETWEEN] = &RdbUtilsIos::NoSupport,
        [DataShare::NOTBETWEEN] = &RdbUtilsIos::NoSupport,
        [DataShare::KEY_PREFIX] = &RdbUtilsIos::NoSupport,
        [DataShare::CROSSJOIN] = &RdbUtilsIos::NoSupport,
        [DataShare::INNERJOIN] = &RdbUtilsIos::NoSupport,
        [DataShare::LEFTOUTERJOIN] = &RdbUtilsIos::NoSupport,
        [DataShare::USING] = &RdbUtilsIos::NoSupport,
        [DataShare::ON] = &RdbUtilsIos::NoSupport,
    };
};
} // namespace Media
} // namespace OHOS
#endif // RDB_UTILS_IOS_H
