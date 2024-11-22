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

#ifndef DATASHARE_VALUE_OBJECT_H
#define DATASHARE_VALUE_OBJECT_H

#include <variant>
#include <string>
#include <vector>
namespace OHOS {
namespace DataShare {
constexpr int INVALID_TYPE = -1;
constexpr int DATA_SHARE_NO_ERROR = 0;
/**
 * @brief DataShare Predicates Object Type .
 */
enum DataShareValueObjectType : int32_t {
    /** Predicates Object Types is null.*/
    TYPE_NULL = 0,
    /** Predicates Object Types is int.*/
    TYPE_INT,
    /** Predicates Object Types is double.*/
    TYPE_DOUBLE,
    /** Predicates Object Types is string.*/
    TYPE_STRING,
    /** Predicates Object Types is bool.*/
    TYPE_BOOL,
    /** Predicates Object Types is blob.*/
    TYPE_BLOB,
};

class DataShareValueObject {
public:
    /**
     * @brief Use Type replace variant namespace.
     */
    using Type = std::variant<std::monostate, int64_t, double, std::string, bool, std::vector<uint8_t>>;
    Type value;

    /**
     * @brief Constructor.
     */
    DataShareValueObject() = default;
    /**
     * @brief Destructor.
     */
    ~DataShareValueObject() = default;
    /**
     * @brief constructor.
     */
    DataShareValueObject(const Type &object) noexcept : value(object) { };
    /**
     * @brief Move Constructor.
     */
    DataShareValueObject(DataShareValueObject &&object) noexcept : value(std::move(object.value)) { };
    /**
     * @brief constructor.
     */
    DataShareValueObject(const DataShareValueObject &object) : value(object.value) {};
    /**
     * @brief constructor.
     *
     * @param int Specifies the parameter of the type.
     */
    DataShareValueObject(int val) : value(static_cast<int64_t>(val)) {};
    /**
     * @brief constructor.
     *
     * @param int64_t Specifies the parameter of the type.
     */
    DataShareValueObject(int64_t val) : value(val) {};
    /**
     * @brief constructor.
     *
     * @param double Specifies the parameter of the type.
     */
    DataShareValueObject(double val) : value(val) {};
    /**
     * @brief constructor.
     *
     * @param bool Specifies the parameter of the type.
     */
    DataShareValueObject(bool val) : value(val) {};
    /**
     * @brief constructor.
     *
     * @param string Specifies the parameter of the type.
     */
    DataShareValueObject(std::string val) : value(std::move(val)) {};
    /**
     * @brief constructor.
     *
     * @param char Specifies the parameter of the type.
     */
    DataShareValueObject(const char *val) : DataShareValueObject(std::string(val)) {};
    /**
     * @brief constructor.
     *
     * @param uint8_t Specifies the parameter of the type.
     */
    DataShareValueObject(std::vector<uint8_t> blob) : value(std::move(blob)) {};
    /**
     * @brief constructor.
     *
     * @param int Specifies the parameter of the type.
     */
    DataShareValueObject &operator=(DataShareValueObject &&object) noexcept
    {
        if (this == &object) {
            return *this;
        }
        value = std::move(object.value);
        return *this;
    };
    DataShareValueObject &operator=(const DataShareValueObject &object)
    {
        if (this == &object) {
            return *this;
        }
        value = object.value;
        return *this;
    }

    operator int () const
    {
        if (std::get_if<int64_t>(&value) != nullptr) {
            return static_cast<int>(std::get<int64_t>(value));
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
    operator std::vector<uint8_t> () const
    {
        if (std::get_if<std::vector<uint8_t>>(&value) != nullptr) {
            return std::get<std::vector<uint8_t>>(value);
        } else {
            return {};
        }
    }
};
} // namespace DataShare
} // namespace OHOS
#endif