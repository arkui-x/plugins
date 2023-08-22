/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_BRIDGE_ENCODABLE_VALUE_H
#define PLUGINS_BRIDGE_ENCODABLE_VALUE_H

#include <map>
#include <string>
#include <variant>
#include <vector>

namespace OHOS::Plugin::Bridge {
enum class CodecableValueType {
    T_NULL = 0,
    T_BOOL,
    T_INT32,
    T_INT64,
    T_DOUBLE,
    T_STRING,
    T_LIST_UINT8,
    T_LIST_BOOL,
    T_LIST_INT32,
    T_LIST_INT64,
    T_LIST_DOUBLE,
    T_LIST_STRING,
    T_MAP,
    T_COMPOSITE_LIST,
};

class CodecableValue;

using CodecableList = std::vector<CodecableValue>;
using CodecableMap = std::map<CodecableValue, CodecableValue>;
using CodecableValueVariant = std::variant<std::monostate,                                      // index 0
                                            bool,                                               // index 1
                                            int32_t,                                            // index 2
                                            int64_t,                                            // index 3
                                            double,                                             // index 4
                                            std::string,                                        // index 5
                                            std::vector<uint8_t>,                               // index 6
                                            std::vector<bool>,                                  // index 7
                                            std::vector<int32_t>,                               // index 8
                                            std::vector<int64_t>,                               // index 9
                                            std::vector<double>,                                // index 10
                                            std::vector<std::string>,                           // index 11
                                            CodecableMap,                                       // index 12
                                            CodecableList>;                                     // index 13  

class CodecableValue : public CodecableValueVariant {
public:
    using super = CodecableValueVariant;
    using super::super;
    using super::operator=;

    explicit CodecableValue() = default;
    explicit CodecableValue(const char* string) : super(std::string(string)) {}
    
    template <class T>
    constexpr explicit CodecableValue(T&& t) noexcept : super(t) {}

    CodecableValue& operator=(const char* other)
    {
        *this = std::string(other);
        return *this;
    }

    bool IsNull() const { return std::holds_alternative<std::monostate>(*this); }

    int64_t LongValue() const
    {
        if (std::holds_alternative<int32_t>(*this)) {
            return std::get<int32_t>(*this);
        }
        return std::get<int64_t>(*this);
    }
};
} // namespace OHOS::Plugin::Bridge
#endif 