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
#ifndef OHOS_GLOBAL_I18N_DATE_TIME_SEQUENCE_H
#define OHOS_GLOBAL_I18N_DATE_TIME_SEQUENCE_H

#include <string>
#include <unordered_map>
#include "locale_info.h"

namespace OHOS {
namespace Global {
namespace I18n {

class DateTimeSequence {
public:
   static std::string GetDateOrder(const std::string& locale);

private:
   static std::string ModifyOrder(std::string& pattern);
   const static std::unordered_map<std::string, std::string> DATE_ORDER_MAP;
   static void ProcessNormal(char ch, int* order, size_t orderSize, int* lengths, size_t lengsSize);
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif
