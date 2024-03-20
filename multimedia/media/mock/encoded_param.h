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

#ifndef HISYSEVENT_INTERFACE_ENCODE_INCLUDE_ENCODED_PARAM_H
#define HISYSEVENT_INTERFACE_ENCODE_INCLUDE_ENCODED_PARAM_H

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "raw_data_base_def.h"
#include "raw_data_encoder.h"
#include "raw_data.h"

namespace OHOS {
namespace HiviewDFX {
namespace Encoded {
class EncodedParam {
public:
    EncodedParam(const std::string& key);
    virtual ~EncodedParam();

public:
    virtual std::string& GetKey();
    virtual std::shared_ptr<RawData> GetRawData();
    virtual void SetRawData(std::shared_ptr<RawData> rawData);
    virtual bool Encode();

protected:
    virtual bool EncodeKey();
    virtual bool EncodeValueType() = 0;
    virtual bool EncodeValue() = 0;

protected:
    std::string key_;
    std::shared_ptr<RawData> rawData_;
    bool hasEncoded_ = false;
};

template<typename T,
    std::enable_if_t<std::is_same_v<std::decay_t<T>, uint8_t> || std::is_same_v<std::decay_t<T>, uint16_t> ||
    std::is_same_v<std::decay_t<T>, uint32_t> || std::is_same_v<std::decay_t<T>, uint64_t>>* = nullptr>
class UnsignedVarintEncodedParam : public EncodedParam {
public:
    UnsignedVarintEncodedParam(const std::string& key, T val): EncodedParam(key)
    {
        val_ = val;
    }

    virtual bool EncodeValueType() override
    {
        if (rawData_ == nullptr) {
            return false;
        }
        return RawDataEncoder::ValueTypeEncoded(*rawData_, false, ValueType::UINT64, 0);
    }

    virtual bool EncodeValue() override
    {
        if (rawData_ == nullptr) {
            return false;
        }
        return RawDataEncoder::UnsignedVarintEncoded(*rawData_, EncodeType::VARINT, val_);
    }

private:
    T val_;
};

template<typename T,
    std::enable_if_t<std::is_same_v<std::decay_t<T>, uint8_t> || std::is_same_v<std::decay_t<T>, uint16_t> ||
    std::is_same_v<std::decay_t<T>, uint32_t> || std::is_same_v<std::decay_t<T>, uint64_t>>* = nullptr>
class UnsignedVarintEncodedArrayParam : public EncodedParam {
public:
    UnsignedVarintEncodedArrayParam(const std::string& key, const std::vector<T>& vals): EncodedParam(key)
    {
        unsigned int index = 0;
        for (auto item = vals.begin(); item != vals.end(); item++) {
            index++;
            if (index > MAX_ARRAY_SIZE) {
                break;
            }
            vals_.emplace_back(*item);
        }
    }

    virtual bool EncodeValueType() override
    {
        if (rawData_ == nullptr) {
            return false;
        }
        return RawDataEncoder::ValueTypeEncoded(*rawData_, true, ValueType::UINT64, 0);
    }

    virtual bool EncodeValue() override
    {
        if (rawData_ == nullptr) {
            return false;
        }
        bool ret = RawDataEncoder::UnsignedVarintEncoded(*rawData_, EncodeType::LENGTH_DELIMITED, vals_.size());
        for (auto item : vals_) {
            ret = ret && RawDataEncoder::UnsignedVarintEncoded(*rawData_, EncodeType::VARINT, item);
        }
        return ret;
    }

private:
    std::vector<T> vals_;
};

template<typename T,
    std::enable_if_t<std::is_same_v<std::decay_t<T>, bool> || std::is_same_v<std::decay_t<T>, int8_t> ||
    std::is_same_v<std::decay_t<T>, int16_t> || std::is_same_v<std::decay_t<T>, int32_t> ||
    std::is_same_v<std::decay_t<T>, int64_t>>* = nullptr>
class SignedVarintEncodedParam : public EncodedParam {
public:
    SignedVarintEncodedParam(const std::string& key, T val): EncodedParam(key)
    {
        val_ = val;
    }

    virtual bool EncodeValueType() override
    {
        if (rawData_ == nullptr) {
            return false;
        }
        return RawDataEncoder::ValueTypeEncoded(*rawData_, false, ValueType::INT64, 0);
    }

    virtual bool EncodeValue() override
    {
        if (rawData_ == nullptr) {
            return false;
        }
        return RawDataEncoder::SignedVarintEncoded(*rawData_, EncodeType::VARINT, val_);
    }

private:
    T val_;
};

template<typename T,
    std::enable_if_t<std::is_same_v<std::decay_t<T>, bool> || std::is_same_v<std::decay_t<T>, int8_t> ||
    std::is_same_v<std::decay_t<T>, int16_t> || std::is_same_v<std::decay_t<T>, int32_t> ||
    std::is_same_v<std::decay_t<T>, int64_t>>* = nullptr>
class SignedVarintEncodedArrayParam : public EncodedParam {
public:
    SignedVarintEncodedArrayParam(const std::string& key, const std::vector<T>& vals): EncodedParam(key)
    {
        unsigned int index = 0;
        for (auto item = vals.begin(); item != vals.end(); item++) {
            index++;
            if (index > MAX_ARRAY_SIZE) {
                break;
            }
            vals_.emplace_back(*item);
        }
    }

    virtual bool EncodeValueType() override
    {
        if (rawData_ == nullptr) {
            return false;
        }
        return RawDataEncoder::ValueTypeEncoded(*rawData_, true, ValueType::INT64, 0);
    }

    virtual bool EncodeValue() override
    {
        if (rawData_ == nullptr) {
            return false;
        }
        bool ret = RawDataEncoder::UnsignedVarintEncoded(*rawData_, EncodeType::LENGTH_DELIMITED, vals_.size());
        for (auto item : vals_) {
            ret = ret && RawDataEncoder::SignedVarintEncoded(*rawData_, EncodeType::VARINT, item);
        }
        return ret;
    }

private:
    std::vector<T> vals_;
};

template<typename T,
    std::enable_if_t<std::is_same_v<std::decay_t<T>, float> || std::is_same_v<std::decay_t<T>, double>>* = nullptr>
class FloatingNumberEncodedParam : public EncodedParam {
public:
    FloatingNumberEncodedParam(const std::string& key, T val): EncodedParam(key)
    {
        val_ = val;
    }

    virtual bool EncodeValueType() override
    {
        if (rawData_ == nullptr) {
            return false;
        }
        auto valueType = ValueType::UNKNOWN;
        if (std::is_same_v<std::decay_t<T>, float>) {
            valueType = ValueType::FLOAT;
        }
        if (std::is_same_v<std::decay_t<T>, double>) {
            valueType = ValueType::DOUBLE;
        }
        return RawDataEncoder::ValueTypeEncoded(*rawData_, false, valueType, 0);
    }

    virtual bool EncodeValue() override
    {
        if (rawData_ == nullptr) {
            return false;
        }
        return RawDataEncoder::FloatingNumberEncoded(*rawData_, val_);
    }

private:
    T val_;
};

template<typename T,
    std::enable_if_t<std::is_same_v<std::decay_t<T>, float> || std::is_same_v<std::decay_t<T>, double>>* = nullptr>
class FloatingNumberEncodedArrayParam : public EncodedParam {
public:
    FloatingNumberEncodedArrayParam(const std::string& key, const std::vector<T>& vals): EncodedParam(key)
    {
        unsigned int index = 0;
        for (auto item = vals.begin(); item != vals.end(); item++) {
            index++;
            if (index > MAX_ARRAY_SIZE) {
                break;
            }
            vals_.emplace_back(*item);
        }
    }

    virtual bool EncodeValueType() override
    {
        if (rawData_ == nullptr) {
            return false;
        }
        auto valueType = ValueType::UNKNOWN;
        if (std::is_same_v<std::decay_t<T>, float>) {
            valueType = ValueType::FLOAT;
        }
        if (std::is_same_v<std::decay_t<T>, double>) {
            valueType = ValueType::DOUBLE;
        }
        return RawDataEncoder::ValueTypeEncoded(*rawData_, true, valueType, 0);
    }

    virtual bool EncodeValue() override
    {
        if (rawData_ == nullptr) {
            return false;
        }
        bool ret = RawDataEncoder::UnsignedVarintEncoded(*rawData_, EncodeType::LENGTH_DELIMITED, vals_.size());
        for (auto item : vals_) {
            ret = ret && RawDataEncoder::FloatingNumberEncoded(*rawData_, item);
        }
        return ret;
    }

private:
    std::vector<T> vals_;
};

class StringEncodedParam : public EncodedParam {
public:
    StringEncodedParam(const std::string& key, const std::string& val): EncodedParam(key)
    {
        val_ = val;
    }

    virtual bool EncodeValueType() override
    {
        if (rawData_ == nullptr) {
            return false;
        }
        return RawDataEncoder::ValueTypeEncoded(*rawData_, false, ValueType::STRING, 0);
    }

    virtual bool EncodeValue() override
    {
        if (rawData_ == nullptr) {
            return false;
        }
        return RawDataEncoder::StringValueEncoded(*rawData_, val_);
    }

private:
    std::string val_;
};

class StringEncodedArrayParam : public EncodedParam {
public:
    StringEncodedArrayParam(const std::string& key, const std::vector<std::string>& vals): EncodedParam(key)
    {
        unsigned int index = 0;
        for (auto item = vals.begin(); item != vals.end(); item++) {
            index++;
            if (index > MAX_ARRAY_SIZE) {
                break;
            }
            vals_.emplace_back(*item);
        }
    }

    virtual bool EncodeValueType() override
    {
        if (rawData_ == nullptr) {
            return false;
        }
        return RawDataEncoder::ValueTypeEncoded(*rawData_, true, ValueType::STRING, 0);
    }

    virtual bool EncodeValue() override
    {
        if (rawData_ == nullptr) {
            return false;
        }
        bool ret = RawDataEncoder::UnsignedVarintEncoded(*rawData_, EncodeType::LENGTH_DELIMITED, vals_.size());
        for (auto item : vals_) {
            ret = ret && RawDataEncoder::StringValueEncoded(*rawData_, item);
        }
        return ret;
    }

private:
    std::vector<std::string> vals_;
};
} // namespace Encoded
} // namespace HiviewDFX
} // namespace OHOS

#endif // HISYSEVENT_INTERFACE_ENCODE_INCLUDE_ENCODED_PARAM_H