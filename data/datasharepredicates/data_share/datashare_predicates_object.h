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

#ifndef DATASHARE_PREDICATES_OBJECT_H
#define DATASHARE_PREDICATES_OBJECT_H

#include <variant>
#include <string>
#include <vector>

namespace OHOS {
namespace DataShare {
/**
 * @brief DataShare Predicates Object Type .
 */
enum class DataSharePredicatesObjectType {
    /** Predicates Object Type is null.*/
    TYPE_NULL = 0x00,
    /** Predicates Object Type is int.*/
    TYPE_INT,
    /** Predicates Object Type is double.*/
    TYPE_DOUBLE,
    /** Predicates Object Type is string.*/
    TYPE_STRING,
    /** Predicates Object Type is bool.*/
    TYPE_BOOL,
    /** Predicates Object Type is long.*/
    TYPE_LONG,
};

/**
 * @brief Use ObjectType replace DataSharePredicatesObjectType namespace.
 */
using ObjectType = DataSharePredicatesObjectType;

/**
 * The SingleValue class.
 */
class SingleValue {
public:

    /**
     * @brief Use Type replace variant namespace.
     */
    using Type = std::variant<std::monostate, int, int64_t, double, std::string, bool>;
    Type value;

    /**
     * @brief Constructor.
     */
    SingleValue() = default;

    /**
     * @brief Destructor.
     */
    ~SingleValue() = default;

    /**
     * @brief Constructor.
     */
    SingleValue(Type val) noexcept : value(std::move(val))
    {
    }

    /**
     * @brief Move constructor.
     */
    SingleValue(SingleValue &&val) noexcept :value(std::move(val.value))
    {
    }

    /**
     * @brief Copy constructor.
     */
    SingleValue(const SingleValue &val) : value(val.value) {}
    SingleValue &operator=(SingleValue &&object) noexcept
    {
        if (this == &object) {
            return *this;
        }
        value = std::move(object.value);
        return *this;
    }

    SingleValue &operator=(const SingleValue &object)
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
    SingleValue(int val) : value(val) {}

    /**
     * @brief constructor.
     *
     * @param int64_t Specifies the parameter of the type.
     */
    SingleValue(int64_t val) : value(val) {}

    /**
     * @brief constructor.
     *
     * @param idoublent Specifies the parameter of the type.
     */
    SingleValue(double val) : value(val) {}

    /**
     * @brief constructor.
     *
     * @param bool Specifies the parameter of the type.
     */
    SingleValue(bool val) : value(val) {}

    /**
     * @brief Copy constructor.
     *
     * @param char Specifies the parameter of the type.
     */
    SingleValue(const char *val) : value(std::string(val)) {}

    /**
     * @brief Move constructor.
     *
     * @param string Specifies the parameter of the type.
     */
    SingleValue(std::string val) : value(std::move(val)) {}
    operator int () const
    {
        if (std::get_if<int>(&value) != nullptr) {
            return std::get<int>(value);
        } else {
            return {};
        }
    }
    operator int64_t () const
    {
        if (std::get_if<int64_t>(&value) != nullptr) {
            return std::get<int64_t>(value);
        } else {
            return {};
        }
    }
    operator double () const
    {
        if (std::get_if<double>(&value) != nullptr) {
            return std::get<double>(value);
        } else {
            return {};
        }
    }
    operator bool () const
    {
        if (std::get_if<bool>(&value) != nullptr) {
            return std::get<bool>(value);
        } else {
            return {};
        }
    }
    operator std::string () const
    {
        if (std::get_if<std::string>(&value) != nullptr) {
            return std::get<std::string>(value);
        } else {
            return {};
        }
    }
};
} // namespace DataShare
} // namespace OHOS
#endif