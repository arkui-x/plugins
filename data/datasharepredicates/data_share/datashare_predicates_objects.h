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

#ifndef DATASHARE_PREDICATES_OBJECTS_H
#define DATASHARE_PREDICATES_OBJECTS_H

#include <variant>
#include <string>
#include <vector>

namespace OHOS {
namespace DataShare {
/**
 * @brief DataShare Predicates Object Type .
 */
enum class DataSharePredicatesObjectsType {
    /** Predicates Object Types is null.*/
    TYPE_NULL = 0x06,
    /** Predicates Object Types is int.*/
    TYPE_INT_VECTOR,
    /** Predicates Object Types is long.*/
    TYPE_LONG_VECTOR,
    /** Predicates Object Types is double.*/
    TYPE_DOUBLE_VECTOR,
    /** Predicates Object Types is string.*/
    TYPE_STRING_VECTOR,
};

/**
 * @brief Use ObjectType replace DataSharePredicatesObjectsType namespace.
 */
using ObjectsType = DataSharePredicatesObjectsType;
class MutliValue {
public:

    /**
     * @brief Use Type replace variant namespace.
     */
    using Type = std::variant<std::monostate, std::vector<int>, std::vector<int64_t>,
        std::vector<std::string>, std::vector<double>>;
    Type value;

    /**
     * @brief Constructor.
     */
    MutliValue() = default;

    /**
     * @brief Destructor.
     */
    ~MutliValue() = default;

    /**
     * @brief Move Constructor.
     */
    MutliValue(MutliValue::Type val) noexcept : value(std::move(val))
    {
    }

    /**
     * @brief Copy constructor.
     */
    MutliValue(const MutliValue &val) : value(val.value) {}
    MutliValue &operator=(MutliValue &&object) noexcept
    {
        if (this == &object) {
            return *this;
        }
        value = std::move(object.value);
        return *this;
    }
    MutliValue &operator=(const MutliValue &object)
    {
        if (this == &object) {
            return *this;
        }
        value = object.value;
        return *this;
    }

    /**
     * @brief constructor.
     *
     * @param int Specifies the parameter of the type.
     */
    MutliValue(const std::vector<int> &val) : value(val) {}

    /**
     * @brief constructor.
     *
     * @param int64_t Specifies the parameter of the type.
     */
    MutliValue(const std::vector<int64_t> &val) : value(val) {}

    /**
     * @brief constructor.
     *
     * @param double Specifies the parameter of the type.
     */
    MutliValue(const std::vector<double> &val) : value(val) {}

    /**
     * @brief constructor.
     *
     * @param string Specifies the parameter of the type.
     */
    MutliValue(const std::vector<std::string> &val) : value(val) {}

    operator std::vector<int> () const
    {
        if (std::get_if<std::vector<int>>(&value) != nullptr) {
            return std::get<std::vector<int>>(value);
        } else {
            return {};
        }
    }
    operator std::vector<int64_t> () const
    {
        if (std::get_if<std::vector<int64_t>>(&value) != nullptr) {
            return std::get<std::vector<int64_t>>(value);
        } else {
            return {};
        }
    }
    operator std::vector<double> () const
    {
        if (std::get_if<std::vector<double>>(&value) != nullptr) {
            return std::get<std::vector<double>>(value);
        } else {
            return {};
        }
    }
    operator std::vector<std::string> () const
    {
        if (std::get_if<std::vector<std::string>>(&value) != nullptr) {
            return std::get<std::vector<std::string>>(value);
        } else {
            return {};
        }
    }
};
} // namespace DataShare
} // namespace OHOS
#endif