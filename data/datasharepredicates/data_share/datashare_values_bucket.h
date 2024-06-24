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

#ifndef DATASHARE_VALUES_BUCKET_H
#define DATASHARE_VALUES_BUCKET_H

#include "datashare_value_object.h"

#include <map>
#include <set>

namespace OHOS {
namespace DataShare {
class DataShareValuesBucket {
public:
    /**
     * @brief Constructor.
     */
    DataShareValuesBucket() = default;
    /**
     * @brief Constructor.
     *
     * @param string Specifies the parameter of the type.
     * @param values is the value of the corresponding type.
     */
    explicit DataShareValuesBucket(std::map<std::string, DataShareValueObject::Type> values)
        : valuesMap(std::move(values)){};
    /**
     * @brief Destructor.
     */
    ~DataShareValuesBucket() = default;
    /**
     * @brief Function of Put.
     *
     * @param columnName is name of the corresponding column.
     * @param value Indicates the value of columnName data to put or update.
     */
    void Put(const std::string &columnName, const DataShareValueObject &value = {})
    {
        valuesMap.insert(std::make_pair(columnName, value.value));
    }
    /**
     * @brief Function of Clear.
     */
    void Clear()
    {
        valuesMap.clear();
    }
    /**
     * @brief Function of IsEmpty.
     */
    bool IsEmpty() const
    {
        return valuesMap.empty();
    }
    /**
     * @brief Function of get string type object.
     *
     * @param columnName is name of the corresponding column.
     * @param isValid The obtained value is valid.
     */
    DataShareValueObject Get(const std::string &columnName, bool &isValid) const
    {
        auto iter = valuesMap.find(columnName);
        if (iter == valuesMap.end()) {
            isValid = false;
            return {};
        }
        isValid = true;
        return iter->second;
    }

    std::map<std::string, DataShareValueObject::Type> valuesMap;
};
} // namespace DataShare
} // namespace OHOS
#endif