/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "character.h"

#include <sys/types.h>
#include <set>
#include <string>

#include "cctype"
#include "map"
#include "string"
#include "umachine.h"
#include "unicode/unistr.h"
#include "urename.h"

namespace OHOS {
namespace Global {
namespace I18n {
static std::set<UCharDirection> RTLDirectionSet = {
    U_RIGHT_TO_LEFT,
    U_RIGHT_TO_LEFT_ARABIC,
    U_RIGHT_TO_LEFT_EMBEDDING,
    U_RIGHT_TO_LEFT_OVERRIDE,
    U_RIGHT_TO_LEFT_ISOLATE
};

bool IsDigit(const std::string &character)
{
    icu::UnicodeString unicodeString(character.c_str());
    UChar32 char32 = unicodeString.char32At(0);
    return u_isdigit(char32);
}

bool IsSpaceChar(const std::string &character)
{
    icu::UnicodeString unicodeString(character.c_str());
    UChar32 char32 = unicodeString.char32At(0);
    return u_isJavaSpaceChar(char32);
}

bool IsWhiteSpace(const std::string &character)
{
    icu::UnicodeString unicodeString(character.c_str());
    UChar32 char32 = unicodeString.char32At(0);
    return u_isWhitespace(char32);
}

bool IsRTLCharacter(const std::string &character)
{
    icu::UnicodeString unicodeString(character.c_str());
    UChar32 char32 = unicodeString.char32At(0);
    UCharDirection direction = u_charDirection(char32);
    if (RTLDirectionSet.find(direction) != RTLDirectionSet.end()) {
        return true;
    }
    return false;
}

bool IsIdeoGraphic(const std::string &character)
{
    icu::UnicodeString unicodeString(character.c_str());
    UChar32 char32 = unicodeString.char32At(0);
    return u_hasBinaryProperty(char32, UCHAR_IDEOGRAPHIC);
}

bool IsLetter(const std::string &character)
{
    icu::UnicodeString unicodeString(character.c_str());
    UChar32 char32 = unicodeString.char32At(0);
    return isalpha(char32);
}

bool IsLowerCase(const std::string &character)
{
    icu::UnicodeString unicodeString(character.c_str());
    UChar32 char32 = unicodeString.char32At(0);
    return u_islower(char32);
}

bool IsUpperCase(const std::string &character)
{
    icu::UnicodeString unicodeString(character.c_str());
    UChar32 char32 = unicodeString.char32At(0);
    return u_isupper(char32);
}

std::map<UCharCategory, std::string> categoryMap = {
    { U_UNASSIGNED, "U_UNASSIGNED" },
    { U_GENERAL_OTHER_TYPES, "U_GENERAL_OTHER_TYPES" },
    { U_UPPERCASE_LETTER, "U_UPPERCASE_LETTER" },
    { U_LOWERCASE_LETTER, "U_LOWERCASE_LETTER" },
    { U_TITLECASE_LETTER, "U_TITLECASE_LETTER" },
    { U_MODIFIER_LETTER, "U_MODIFIER_LETTER" },
    { U_OTHER_LETTER, "U_OTHER_LETTER" },
    { U_NON_SPACING_MARK, "U_NON_SPACING_MARK" },
    { U_ENCLOSING_MARK, "U_ENCLOSING_MARK" },
    { U_COMBINING_SPACING_MARK, "U_COMBINING_SPACING_MARK" },
    { U_DECIMAL_DIGIT_NUMBER, "U_DECIMAL_DIGIT_NUMBER" },
    { U_LETTER_NUMBER, "U_LETTER_NUMBER" },
    { U_OTHER_NUMBER, "U_OTHER_NUMBER" },
    { U_SPACE_SEPARATOR, "U_SPACE_SEPARATOR" },
    { U_LINE_SEPARATOR, "U_LINE_SEPARATOR" },
    { U_PARAGRAPH_SEPARATOR, "U_PARAGRAPH_SEPARATOR" },
    { U_CONTROL_CHAR, "U_CONTROL_CHAR" },
    { U_FORMAT_CHAR, "U_FORMAT_CHAR" },
    { U_PRIVATE_USE_CHAR, "U_PRIVATE_USE_CHAR" },
    { U_SURROGATE, "U_SURROGATE" },
    { U_DASH_PUNCTUATION, "U_DASH_PUNCTUATION" },
    { U_START_PUNCTUATION, "U_START_PUNCTUATION" },
    { U_END_PUNCTUATION, "U_END_PUNCTUATION" },
    { U_CONNECTOR_PUNCTUATION, "U_CONNECTOR_PUNCTUATION" },
    { U_OTHER_PUNCTUATION, "U_OTHER_PUNCTUATION" },
    { U_MATH_SYMBOL, "U_MATH_SYMBOL" },
    { U_CURRENCY_SYMBOL, "U_CURRENCY_SYMBOL" },
    { U_MODIFIER_SYMBOL, "U_MODIFIER_SYMBOL" },
    { U_OTHER_SYMBOL, "U_OTHER_SYMBOL" },
    { U_INITIAL_PUNCTUATION, "U_INITIAL_PUNCTUATION" },
    { U_FINAL_PUNCTUATION, "U_FINAL_PUNCTUATION" },
    { U_CHAR_CATEGORY_COUNT, "U_CHAR_CATEGORY_COUNT" },
};

std::string GetType(const std::string &character)
{
    icu::UnicodeString unicodeString(character.c_str());
    UChar32 char32 = unicodeString.char32At(0);
    int8_t category = u_charType(char32);
    return categoryMap[UCharCategory(category)];
}
} // namespace I18n
} // namespace Global
} // namespace OHOS