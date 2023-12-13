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
#ifndef GLOBAL_I18N_CHARACTER_H
#define GLOBAL_I18N_CHARACTER_H

#include <string>

#include "unicode/uchar.h"

namespace OHOS {
namespace Global {
namespace I18n {
bool IsDigit(const std::string &character);
bool IsSpaceChar(const std::string &character);
bool IsWhiteSpace(const std::string &character);
bool IsRTLCharacter(const std::string &character);
bool IsIdeoGraphic(const std::string &character);
bool IsLetter(const std::string &character);
bool IsLowerCase(const std::string &character);
bool IsUpperCase(const std::string &character);
std::string GetType(const std::string &character);
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif
