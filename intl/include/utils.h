/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except", "in compliance with the License.
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
#ifndef OHOS_GLOBAL_I18N_UTILS_H
#define OHOS_GLOBAL_I18N_UTILS_H

#include <string>
#include <vector>

namespace OHOS {
namespace Global {
namespace I18n {
void Split(const std::string &src, const std::string &sep, std::vector<std::string> &dest);
int32_t ConvertString2Int(const std::string &numberStr, int32_t& status);
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif