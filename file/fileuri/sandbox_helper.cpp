/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "sandbox_helper.h"

#include <cctype>

namespace OHOS {
namespace AppFileService {
namespace {
constexpr char PERCENT = '%';
constexpr int HEX_BASE = 16;
constexpr int HEX_LOW_MASK = 0x0F;
constexpr int HEX_SHIFT = 4;
constexpr int DECIMAL_OFFSET = 10;
constexpr int ENCODE_MAX_CHARS = 3;
constexpr int DECODE_OFFSET = 2;

inline bool IsSafeChar(unsigned char c)
{
    return std::isalnum(c) || c == '!' || c == '*' || c == '(' || c == ')' ||
        c == '-' || c == '_' || c == '.' || c == '~' || c == '/';
}

inline bool IsHex(char c)
{
    return std::isxdigit(static_cast<unsigned char>(c));
}

inline int HexToInt(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + DECIMAL_OFFSET;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + DECIMAL_OFFSET;
    }
    return 0;
}

inline char IntToHexUpper(unsigned int value)
{
    static const char* hexChars = "0123456789ABCDEF";
    return hexChars[value & HEX_LOW_MASK];
}
} // namespace

bool SandboxHelper::IsValidPath(const std::string& path)
{
    if (path.empty()) {
        return true;
    }

    std::string segment;
    segment.reserve(path.size());
    for (char ch : path) {
        if (ch == '/' || ch == '\\') {
            if (segment == "..") {
                return false;
            }
            segment.clear();
            continue;
        }
        segment.push_back(ch);
    }

    if (segment == "..") {
        return false;
    }
    return true;
}

std::string SandboxHelper::Encode(const std::string& uri)
{
    std::string encoded;
    encoded.reserve(uri.size() * ENCODE_MAX_CHARS);
    for (unsigned char c : uri) {
        if (IsSafeChar(c)) {
            encoded.push_back(static_cast<char>(c));
            continue;
        }
        encoded.push_back(PERCENT);
        encoded.push_back(IntToHexUpper((c >> HEX_SHIFT) & HEX_LOW_MASK));
        encoded.push_back(IntToHexUpper(c & HEX_LOW_MASK));
    }
    return encoded;
}

std::string SandboxHelper::Decode(const std::string& uri)
{
    std::string decoded;
    decoded.reserve(uri.size());
    size_t i = 0;
    while (i < uri.size()) {
        if (uri[i] == PERCENT && (i + DECODE_OFFSET) < uri.size() &&
            IsHex(uri[i + 1]) && IsHex(uri[i + DECODE_OFFSET])) {
            int high = HexToInt(uri[i + 1]);
            int low = HexToInt(uri[i + DECODE_OFFSET]);
            decoded.push_back(static_cast<char>((high * HEX_BASE) + low));
            i += DECODE_OFFSET + 1;
            continue;
        }
        decoded.push_back(uri[i]);
        ++i;
    }
    return decoded;
}
} // namespace AppFileService
} // namespace OHOS
