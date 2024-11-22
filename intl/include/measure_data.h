/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifndef OHOS_GLOBAL_I18N_MEASURE_DATA_H
#define OHOS_GLOBAL_I18N_MEASURE_DATA_H

#include <vector>
#include <string>
#include "unicode/measunit.h"

namespace OHOS {
namespace Global {
namespace I18n {
uint32_t GetMask(const std::string &region);
void GetDefaultPreferredUnit(const std::string &region, const std::string &type, std::vector<std::string> &units);
void GetFallbackPreferredUnit(const std::string &region, const std::string &usage, std::vector<std::string> &units);
void GetRestPreferredUnit(const std::string &region, const std::string &usage, std::vector<std::string> &units);
void GetPreferredUnit(const std::string &region, const std::string &usage, std::vector<std::string> &units);
void ComputeFactorValue(const std::string &unit, const std::string &measSys, std::vector<double> &factors);
double ComputeSIPrefixValue(const std::string &unit);
void ComputePowerValue(const std::string &unit, const std::string &measSys, std::vector<double> &factors);
int ComputeValue(const std::string &unit, const std::string &measSys, std::vector<double> &factors);
int Convert(double &value, const std::string &fromUnit, const std::string &fromMeasSys, const std::string &toUnit,
    const std::string &toMeasSys);
bool ConvertByte(double& number, std::string& unit);
bool ConvertDate(double& number, std::string& unit);
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif