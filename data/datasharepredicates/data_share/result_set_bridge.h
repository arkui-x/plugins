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

#ifndef DATASHARE_RESULT_SET_BRIDGE_H
#define DATASHARE_RESULT_SET_BRIDGE_H

#include <memory>
#include <string>
namespace OHOS {
namespace DataShare {
// build the bridge between the database's ResultSet and DataShare's ResultSet
class ResultSetBridge {
public:
    class Writer {
    public:
        /**
         * Allocate a row unit and its directory.
         */
        virtual int AllocRow() = 0;

        /**
         * Free the last row.
         */
        virtual int FreeLastRow() = 0;

        /**
         * Write Null data to the shared block.
         */
        virtual int Write(uint32_t column) = 0;

        /**
         * Write long data to the shared block.
         */
        virtual int Write(uint32_t column, int64_t value) = 0;

        /**
         * Write Double data to the shared block.
         */
        virtual int Write(uint32_t column, double value) = 0;

        /**
         * Write blob data to the shared block.
         */
        virtual int Write(uint32_t column, const uint8_t *value, size_t size) = 0;

        /**
         * Write string data to the shared block.
         */
        virtual int Write(uint32_t column, const char *value, size_t size) = 0;
    };

    virtual ~ResultSetBridge() {}

    /**
     * Return a string array holding the names of all of the columns in the
     * result set.
     *
     * return the names of the columns contains in this query result.
     */
    virtual int GetAllColumnNames(std::vector<std::string> &columnNames) = 0;

    /**
     * Return the numbers of rows in the result set.
     */
    virtual int GetRowCount(int32_t &count) = 0;

    /**
     * Called when the position of the result set changes
     */
    virtual int OnGo(int32_t startRowIndex, int32_t targetRowIndex, Writer &writer) = 0;
};
} // namespace DataShare
} // namespace OHOS
#endif
