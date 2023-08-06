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

#include "base64_utils.h"

#include <array>

namespace OHOS::NetStack::Base64 {
static std::string BASE64_CHARS = /* NOLINT */
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static constexpr const uint32_t CHAR_ARRAY_LENGTH_THREE = 3;
static constexpr const uint32_t CHAR_ARRAY_LENGTH_FOUR = 4;

enum BASE64_ENCODE_CONSTANT : uint8_t {
    BASE64_ENCODE_MASK1 = 0xfc,
    BASE64_ENCODE_MASK2 = 0x03,
    BASE64_ENCODE_MASK3 = 0x0f,
    BASE64_ENCODE_MASK4 = 0x3f,
    BASE64_ENCODE_MASK5 = 0xf0,
    BASE64_ENCODE_MASK6 = 0xc0,
    BASE64_ENCODE_OFFSET2 = 2,
    BASE64_ENCODE_OFFSET4 = 4,
    BASE64_ENCODE_OFFSET6 = 6,
    BASE64_ENCODE_INDEX0 = 0,
    BASE64_ENCODE_INDEX1 = 1,
    BASE64_ENCODE_INDEX2 = 2,
};

enum BASE64_DECODE_CONSTANT : uint8_t {
    BASE64_DECODE_MASK1 = 0x30,
    BASE64_DECODE_MASK2 = 0xf,
    BASE64_DECODE_MASK3 = 0x3c,
    BASE64_DECODE_MASK4 = 0x3,
    BASE64_DECODE_OFFSET2 = 2,
    BASE64_DECODE_OFFSET4 = 4,
    BASE64_DECODE_OFFSET6 = 6,
    BASE64_DECODE_INDEX0 = 0,
    BASE64_DECODE_INDEX1 = 1,
    BASE64_DECODE_INDEX2 = 2,
    BASE64_DECODE_INDEX3 = 3,
};

static inline bool IsBase64Char(const char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

static void MakeCharFour(const std::array<uint8_t, CHAR_ARRAY_LENGTH_THREE>& charArrayThree,
    std::array<uint8_t, CHAR_ARRAY_LENGTH_FOUR>& charArrayFour)
{
    uint8_t table[CHAR_ARRAY_LENGTH_FOUR] = {
        static_cast<uint8_t>((charArrayThree[BASE64_ENCODE_INDEX0] & BASE64_ENCODE_MASK1) >> BASE64_ENCODE_OFFSET2),
        static_cast<uint8_t>(((charArrayThree[BASE64_ENCODE_INDEX0] & BASE64_ENCODE_MASK2) << BASE64_ENCODE_OFFSET4) +
                            ((charArrayThree[BASE64_ENCODE_INDEX1] & BASE64_ENCODE_MASK5) >> BASE64_ENCODE_OFFSET4)),
        static_cast<uint8_t>(((charArrayThree[BASE64_ENCODE_INDEX1] & BASE64_ENCODE_MASK3) << BASE64_ENCODE_OFFSET2) +
                            ((charArrayThree[BASE64_ENCODE_INDEX2] & BASE64_ENCODE_MASK6) >> BASE64_ENCODE_OFFSET6)),
        static_cast<uint8_t>(charArrayThree[BASE64_ENCODE_INDEX2] & BASE64_ENCODE_MASK4),
    };
    for (size_t index = 0; index < CHAR_ARRAY_LENGTH_FOUR; ++index) {
        charArrayFour[index] = table[index];
    }
}

static void MakeCharTree(const std::array<uint8_t, CHAR_ARRAY_LENGTH_FOUR>& charArrayFour,
    std::array<uint8_t, CHAR_ARRAY_LENGTH_THREE>& charArrayThree)
{
    uint8_t table[CHAR_ARRAY_LENGTH_THREE] = {
        static_cast<uint8_t>((charArrayFour[BASE64_DECODE_INDEX0] << BASE64_DECODE_OFFSET2) +
                            ((charArrayFour[BASE64_DECODE_INDEX1] & BASE64_DECODE_MASK1) >> BASE64_DECODE_OFFSET4)),
        static_cast<uint8_t>(((charArrayFour[BASE64_DECODE_INDEX1] & BASE64_DECODE_MASK2) << BASE64_DECODE_OFFSET4) +
                            ((charArrayFour[BASE64_DECODE_INDEX2] & BASE64_DECODE_MASK3) >> BASE64_DECODE_OFFSET2)),
        static_cast<uint8_t>(((charArrayFour[BASE64_DECODE_INDEX2] & BASE64_DECODE_MASK4) << BASE64_DECODE_OFFSET6) +
                            charArrayFour[BASE64_DECODE_INDEX3]),
    };
    for (size_t index = 0; index < CHAR_ARRAY_LENGTH_THREE; ++index) {
        charArrayThree[index] = table[index];
    }
}

std::string Encode(const std::string &source)
{
    auto it = source.begin();
    std::string ret;
    size_t index = 0;
    std::array<uint8_t, CHAR_ARRAY_LENGTH_THREE> charArrayThree = { 0 };
    std::array<uint8_t, CHAR_ARRAY_LENGTH_FOUR> charArrayFour = { 0 };

    while (it != source.end()) {
        charArrayThree[index] = *it;
        ++index;
        ++it;
        if (index != CHAR_ARRAY_LENGTH_THREE) {
            continue;
        }
        MakeCharFour(charArrayThree, charArrayFour);
        for (auto idx : charArrayFour) {
            ret += BASE64_CHARS[idx];
        }
        index = 0;
    }
    if (index == 0) {
        return ret;
    }

    for (auto i = index; i < CHAR_ARRAY_LENGTH_THREE; ++i) {
        charArrayThree[i] = 0;
    }
    MakeCharFour(charArrayThree, charArrayFour);

    for (size_t i = 0; i < index + 1; ++i) {
        ret += BASE64_CHARS[charArrayFour[i]];
    }

    while (index < CHAR_ARRAY_LENGTH_THREE) {
        ret += '=';
        ++index;
    }
    return ret;
}

std::string Decode(const std::string &encoded)
{
    auto it = encoded.begin();
    size_t index = 0;
    std::array<uint8_t, CHAR_ARRAY_LENGTH_THREE> charArrayThree = { 0 };
    std::array<uint8_t, CHAR_ARRAY_LENGTH_FOUR> charArrayFour = { 0 };
    std::string ret;

    while (it != encoded.end() && IsBase64Char(*it)) {
        charArrayFour[index] = *it;
        ++index;
        ++it;
        if (index != CHAR_ARRAY_LENGTH_FOUR) {
            continue;
        }
        for (index = 0; index < CHAR_ARRAY_LENGTH_FOUR; ++index) {
            charArrayFour[index] = BASE64_CHARS.find(static_cast<char>(charArrayFour[index]));
        }
        MakeCharTree(charArrayFour, charArrayThree);
        for (auto idx : charArrayThree) {
            ret += static_cast<char>(idx);
        }
        index = 0;
    }
    if (index == 0) {
        return ret;
    }

    for (auto i = index; i < CHAR_ARRAY_LENGTH_FOUR; ++i) {
        charArrayFour[i] = 0;
    }
    for (unsigned char &i : charArrayFour) {
        i = BASE64_CHARS.find(static_cast<char>(i));
    }
    MakeCharTree(charArrayFour, charArrayThree);

    for (size_t i = 0; i < index - 1; i++) {
        ret += static_cast<char>(charArrayThree[i]);
    }
    return ret;
}
} // namespace OHOS::NetStack::Http::Base64
