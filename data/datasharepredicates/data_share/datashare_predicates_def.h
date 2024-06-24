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


#ifndef DATASHARE_PREDICATES_DEF_H
#define DATASHARE_PREDICATES_DEF_H

#include <string>
#include <vector>
#include "datashare_predicates_object.h"
#include "datashare_predicates_objects.h"
namespace OHOS {
namespace DataShare {
/**
 * @brief SetOperationList parameters.
 */
typedef enum {
    /** Indicates the parameters is INVALID_OPERATION.*/
    INVALID_OPERATION = 0x0,
    /** Indicates the parameters is EQUAL_TO.*/
    EQUAL_TO,
    /** Indicates the parameters is NOT_EQUAL_TO.*/
    NOT_EQUAL_TO,
    /** Indicates the parameters is GREATER_THAN.*/
    GREATER_THAN,
    /** Indicates the parameters is LESS_THAN.*/
    LESS_THAN,
    /** Indicates the parameters is GREATER_THAN_OR_EQUAL_TO.*/
    GREATER_THAN_OR_EQUAL_TO,
    /** Indicates the parameters is LESS_THAN_OR_EQUAL_TO.*/
    LESS_THAN_OR_EQUAL_TO,
    /** Indicates the parameters is AND.*/
    AND,
    /** Indicates the parameters is OR.*/
    OR,
    /** Indicates the parameters is IS_NULL.*/
    IS_NULL,
    /** Indicates the parameters is IS_NOT_NULL.*/
    IS_NOT_NULL,
    /** Indicates the parameters is SQL_IN.*/
    SQL_IN,
    /** Indicates the parameters is NOT_IN.*/
    NOT_IN,
    /** Indicates the parameters is LIKE.*/
    LIKE,
    /** Indicates the parameters is UNLIKE.*/
    UNLIKE,
    /** Indicates the parameters is ORDER_BY_ASC.*/
    ORDER_BY_ASC,
    /** Indicates the parameters is ORDER_BY_DESC.*/
    ORDER_BY_DESC,
    /** Indicates the parameters is LIMIT.*/
    LIMIT,
    /** Indicates the parameters is OFFSET.*/
    OFFSET,
    /** Indicates the parameters is BEGIN_WARP.*/
    BEGIN_WARP,
    /** Indicates the parameters is END_WARP.*/
    END_WARP,
    /** Indicates the parameters is BEGIN_WITH.*/
    BEGIN_WITH,
    /** Indicates the parameters is END_WITH.*/
    END_WITH,
    /** Indicates the parameters is IN_KEY.*/
    IN_KEY,
    /** Indicates the parameters is DISTINCT.*/
    DISTINCT,
    /** Indicates the parameters is GROUP_BY.*/
    GROUP_BY,
    /** Indicates the parameters is INDEXED_BY.*/
    INDEXED_BY,
    /** Indicates the parameters is CONTAINS.*/
    CONTAINS,
    /** Indicates the parameters is GLOB.*/
    GLOB,
    /** Indicates the parameters is BETWEEN.*/
    BETWEEN,
    /** Indicates the parameters is NOTBETWEEN.*/
    NOTBETWEEN,
    /** Indicates the parameters is KEY_PREFIX.*/
    KEY_PREFIX,
    /** Indicates the parameters is CROSSJOIN.*/
    CROSSJOIN,
    /** Indicates the parameters is INNERJOIN.*/
    INNERJOIN,
    /** Indicates the parameters is LEFTOUTERJOIN.*/
    LEFTOUTERJOIN,
    /** Indicates the parameters is USING.*/
    USING,
    /** Indicates the parameters is ON.*/
    ON,
    /** Indicates the parameters is LAST_TYPE.*/
    LAST_TYPE
} OperationType;

/**
 * @brief OperationItem.
 */
struct OperationItem {
    /** operation.*/
    int32_t operation;
    /** The type of singleParams*/
    std::vector<SingleValue::Type> singleParams;
    /** The type of multiParams*/
    std::vector<MutliValue::Type> multiParams;
    /** SingleValue GetSingle.*/
    inline SingleValue GetSingle(int32_t index) const
    {
        return singleParams[index];
    }
};

/**
 * @brief typedef.
 */
typedef enum : int16_t {
    /** The type of INVALID_MODE*/
    INVALID_MODE,
    /** The type of QUERY_LANGUAGE*/
    QUERY_LANGUAGE,
    /** The type of PREDICATES_METHOD*/
    PREDICATES_METHOD
} SettingMode;
} // namespace DataShare
} // namespace OHOS

#endif