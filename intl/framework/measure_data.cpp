/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "measure_data.h"
#include <string>
#include <unordered_map>
#include <cmath>
#include "utils.h"

namespace OHOS {
namespace Global {
namespace I18n {
using namespace std;
const double FT_TO_M = 0.3048;
const double FT2_TO_M2 = FT_TO_M * FT_TO_M;
const double FT3_TO_M3 = FT_TO_M * FT2_TO_M2;
const double IN3_TO_M3 = FT3_TO_M3 / (12 * 12 * 12);
const double GAL_IMP_TO_M3 = 0.00454609;
const double GAL_TO_M3 = 231 * IN3_TO_M3;
const int REGION_OFFSET = 7;
const int BASE_VALUE_SIZE = 2;
const int FACTOR_SIZE = 2;
const int CHAR_OFFSET = 48;
const int MAX_UNIT_NUM = 500;

const std::unordered_map<std::string, std::vector<std::string>> USAGE_001 {
    { "area-land-agricult", { "hectare" } },
    { "area-land-commercl", { "hectare" } },
    { "area-land-residntl", { "hectare" } },
    { "length-person", { "centimeter" } },
    { "length-person-small", { "centimeter" } },
    { "length-rainfall", { "millimeter" } },
    { "length-road", { "meter" } },
    { "length-road-small", { "meter" } },
    { "length-snowfall", { "centimeter" } },
    { "length-vehicle", { "meter" } },
    { "length-visiblty", { "meter" } },
    { "length-visiblty-small", { "meter" } },
    { "speed-road-travel", { "kilometer-per-hour" } },
    { "speed-wind", { "kilometer-per-hour" } },
    { "temperature-person", { "celsius" } },
    { "temperature-weather", { "celsius" } },
    { "volume-vehicle-fuel", { "liter" } },
};

const std::unordered_map<std::string, std::vector<std::string>> USAGE_DEFAULT_001 {
    { "length", { "kilometer", "meter", "centimeter" } },
    { "area", { "square-kilometer", "hectare", "square-meter", "square-centimeter" } },
    { "speed", { "kilometer-per-hour" } },
    { "temperature", { "celsius" } },
    { "volume", { "cubic-meter", "cubic-centimeter" } },
};

const std::unordered_map<std::string, std::vector<std::string>> USAGE_AT {
    { "length-person", { "meter", "centimeter" } },
};

const std::unordered_map<std::string, std::vector<std::string>> USAGE_BR {
    { "length-person-informal", { "meter", "centimeter" } },
    { "length-rainfall", { "centimeter" } },
};

const std::unordered_map<std::string, std::vector<std::string>> USAGE_BS {
    { "temperature-weather", { "fahrenheit" } },
};

const std::unordered_map<std::string, std::vector<std::string>> USAGE_CA {
    { "length-person", { "inch" } },
    { "length-person-informal", { "foot", "inch" } },
    { "length-person-small-informal", { "inch" } },
};

const std::unordered_map<std::string, std::vector<std::string>> USAGE_CN {
    { "length-person-informal", { "meter", "centimeter" } },
};

const std::unordered_map<std::string, std::vector<std::string>> USAGE_DE {
    { "length-person-informal", { "meter", "centimeter" } },
    { "length-visiblty", { "meter" } },
};

const std::unordered_map<std::string, std::vector<std::string>> USAGE_GB {
    { "area-land-agricult", { "acre" } },
    { "area-land-commercl", { "acre" } },
    { "area-land-residntl", { "acre" } },
    { "length-person", { "inch" } },
    { "length-person-informal", { "foot", "inch" } },
    { "length-person-small-informal", { "inch" } },
    { "length-road", { "yard" } },
    { "length-road-small", { "yard" } },
    { "length-vehicle", { "foot", "inch" } },
    { "length-visiblty", { "mile", "foot" } },
    { "length-visiblty-small", { "foot" } },
    { "speed-road-travel", { "mile-per-hour" } },
};

const std::unordered_map<std::string, std::vector<std::string>> USAGE_DEFAULT_GB {
    { "length", { "mile", "foot", "inch" } },
    { "area", { "square-mile", "acre", "square-foot", "square-inch" } },
    { "speed", { "mile-per-hour" } },
    { "volume", { "cubic-foot", "cubic-inch" } },
};

const std::unordered_map<std::string, std::vector<std::string>> USAGE_KR {
    { "speed-wind", { "meter-per-second" } },
};

const std::unordered_map<std::string, std::vector<std::string>> USAGE_MX {
    { "length-person-informal", { "meter", "centimeter" } },
    { "length-vehicle", { "meter", "centimeter" } },
};

const std::unordered_map<std::string, std::vector<std::string>> USAGE_NL {
    { "length-person-informal", { "meter", "centimeter" } },
    { "length-visiblty", { "meter" } },
};

const std::unordered_map<std::string, std::vector<std::string>> USAGE_NO {
    { "length-person-informal", { "meter", "centimeter" } },
    { "speed-wind", { "meter-per-second" } },
};

const std::unordered_map<std::string, std::vector<std::string>> USAGE_SE {
    { "length-person", { "meter", "centimeter" } },
    { "length-road", { "mile-scandinavian", "kilometer" } },
    { "length-road-informal", { "mile-scandinavian" } },
    { "speed-wind", { "meter-per-second" } },
};

const std::unordered_map<std::string, std::vector<std::string>> USAGE_US {
    { "area-land-agricult", { "acre" } },
    { "area-land-commercl", { "acre" } },
    { "area-land-residntl", { "acre" } },
    { "length-person", { "inch" } },
    { "length-person-informal", { "foot", "inch" } },
    { "length-person-small", { "inch" } },
    { "length-rainfall", { "inch" } },
    { "length-road", { "mile" } },
    { "length-road-small", { "foot" } },
    { "length-snowfall", { "inch" } },
    { "length-vehicle", { "foot", "inch" } },
    { "length-visiblty", { "mile", "foot" } },
    { "length-visiblty-small", { "foot" } },
    { "speed-road-travel", { "mile-per-hour" } },
    { "speed-wind", { "mile-per-hour" } },
    { "temperature-person", { "fahrenheit" } },
    { "temperature-weather", { "fahrenheit" } },
    { "volume-vehicle-fuel", { "gallon" } },
};

const std::unordered_map<std::string, std::vector<std::string>> USAGE_DEFAULT_US {
    { "length", { "mile", "foot", "inch" } },
    { "area", { "square-mile", "acre", "square-foot", "square-inch" } },
    { "speed", { "mile-per-hour" } },
    { "temperature", { "fahrenheit" } },
    { "volume", { "cubic-foot", "cubic-inch" } },
};

const std::unordered_map<std::string, std::vector<double>> CONVERT_FACTORS {
    { "acre", { FT2_TO_M2 * 43560, 0 } },
    { "hectare", { 10000, 0 } },
    { "dunam", { 1000, 0 } },
    { "fathom", { FT_TO_M * 6, 0 } },
    { "foot", { FT_TO_M, 0 } },
    { "furlong", { FT_TO_M * 660, 0 } },
    { "inch", { FT_TO_M / 12, 0 } },
    { "meter", { 1, 0 } },
    { "mile", { FT_TO_M * 5280, 0 } },
    { "mile-scandinavian", { 10000, 0 } },
    { "nautical-mile", { 1852, 0 } },
    { "point", { FT_TO_M / 864, 0 } },
    { "yard", { FT_TO_M * 3, 0 } },
    { "knot", { 1852 / 3600.0, 0 } },
    { "kilometer-per-hour", { 1000 / 3600.0, 0 } },
    { "celsius", { 1, 273.15 }},
    { "fahrenheit", {5.0 / 9, 2298.35 / 9 }},
    { "kelvin", { 1, 0 } },
    { "acre-foot", { 43560 * FT3_TO_M3, 0 } },
    { "bushel", { GAL_IMP_TO_M3 * 8, 0 } },
    { "bushel-UK", { GAL_IMP_TO_M3 * 8, 0 } },
    { "bushel-US", { IN3_TO_M3 * 2150.42, 0 } },
    { "cup", { GAL_IMP_TO_M3 / 16, 0 } },
    { "cup-UK", { GAL_IMP_TO_M3 / 16, 0 } },
    { "cup-US", { GAL_TO_M3 / 16, 0 } },
    { "cup-metric", {0.00025, 0 } },
    { "fluid-ounce", { GAL_IMP_TO_M3 / 128, 0 } },
    { "fluid-ounce-imperial", { GAL_IMP_TO_M3 / 160, 0 } },
    { "gallon", { GAL_TO_M3, 0 } },
    { "gallon-imperial", { GAL_IMP_TO_M3, 0 } },
    { "liter", {0.001, 0 } },
    { "pint-metric", { 0.0005, 0 } },
    { "pint", { GAL_IMP_TO_M3 / 8, 0 } },
    { "pint-UK", { GAL_IMP_TO_M3 / 8, 0 } },
    { "pint-US", { GAL_TO_M3 / 8, 0 } },
    { "quart", { GAL_IMP_TO_M3 / 4, 0 } },
    { "quart-UK", { GAL_IMP_TO_M3 / 4, 0 } },
    { "quart-US", { GAL_TO_M3 / 4, 0 } },
    { "tablespoon", { GAL_IMP_TO_M3 / 256, 0 } },
    { "tablespoon-UK", { GAL_IMP_TO_M3 / 256, 0 } },
    { "tablespoon-US", { GAL_TO_M3 / 256, 0 } },
    { "teaspoon", { GAL_IMP_TO_M3 / (16 * 48), 0 } },
    { "teaspoon-UK", { GAL_IMP_TO_M3 / (16 * 48), 0 } },
    { "teaspoon-US", { GAL_TO_M3 / (16 * 48), 0 } },
    { "barrel", { GAL_IMP_TO_M3 * 36, 0 } },
    { "barrel-UK", { GAL_IMP_TO_M3 * 36, 0 } },
    { "barrel-US", { GAL_TO_M3 * 42, 0 } },
    { "hour", { 3600, 0 } },
    { "second", { 1, 0 } },
};

const std::unordered_map<std::string, std::string> CONVERT_TO_UNIT {
    { "acre", "square-meter" },
    { "hectare", "square-meter" },
    { "dunam", "square-meter" },
    { "fathom", "meter" },
    { "foot", "meter" },
    { "furlong", "meter" },
    { "inch", "meter" },
    { "meter", "meter" },
    { "mile", "meter" },
    { "mile-scandinavian", "meter" },
    { "nautical-mile", "meter" },
    { "point", "meter" },
    { "yard", "meter" },
    { "knot", "meter-per-second" },
    { "kilometer-per-hour", "meter-per-second" },
    { "celsius", "kelvin" },
    { "fahrenheit", "kelvin" },
    { "kelvin", "kelvin" },
    { "acre-foot", "cubic-meter" },
    { "bushel", "cubic-meter" },
    { "bushel-UK", "cubic-meter" },
    { "bushel-US", "cubic-meter" },
    { "cup", "cubic-meter" },
    { "cup-UK", "cubic-meter" },
    { "cup-US", "cubic-meter" },
    { "cup-metric", "cubic-meter" },
    { "fluid-ounce", "cubic-meter" },
    { "fluid-ounce-imperial", "cubic-meter" },
    { "gallon", "cubic-meter" },
    { "gallon-imperial", "cubic-meter" },
    { "liter", "cubic-meter" },
    { "pint-metric", "cubic-meter" },
    { "pint", "cubic-meter" },
    { "pint-UK", "cubic-meter" },
    { "pint-US", "cubic-meter" },
    { "quart", "cubic-meter" },
    { "quart-UK", "cubic-meter" },
    { "quart-US", "cubic-meter" },
    { "tablespoon", "cubic-meter" },
    { "tablespoon-UK", "cubic-meter" },
    { "tablespoon-US", "cubic-meter" },
    { "teaspoon", "cubic-meter" },
    { "teaspoon-UK", "cubic-meter" },
    { "teaspoon-US", "cubic-meter" },
    { "barrel", "cubic-meter" },
    { "barrel-US", "cubic-meter" },
    { "barrel-UK", "cubic-meter" },
    { "hour", "second" },
    { "second", "second" },
};

const std::unordered_map<std::string, double> PREFIX_VALUE {
    { "deci", pow(10, -1) },
    { "centi", pow(10, -2) },
    { "milli", pow(10, -3) },
    { "micro", pow(10, -6) },
    { "nano", pow(10, -9) },
    { "pico", pow(10, -12) },
    { "kilo", pow(10, 3) },
    { "hecto", pow(10, 2) },
    { "mega", pow(10, 6) },
};

const std::vector<std::string> BYTE_VALUE {
    "byte",
    "kilobyte",
    "megabyte",
    "gigabyte",
    "terabyte",
    "petabyte"
};

const std::vector<std::pair<std::string, double>> DATE_VALUE {
    { "second", 1 },
    { "minute", 60 },
    { "hour", 60 },
    { "day", 24},
    { "month", 30 },
    { "year", 365 / 30 }
};

const std::unordered_map<std::string, double> POWER_VALUE {
    { "square-", 2 },
    { "cubic-", 3 },
};

uint32_t GetMask(const string &region)
{
    uint32_t firstChar = (region.c_str()[0] - CHAR_OFFSET);
    uint32_t secondChar = (region.c_str()[1] - CHAR_OFFSET);
    return (firstChar << REGION_OFFSET) | secondChar;
}

void GetDefaultPreferredUnit(const string &region, const string &type, vector<string> &units)
{
    switch (GetMask(region)) {
        case 0x00000B92: {
            if (USAGE_DEFAULT_GB.count(type) > 0) {
                units.assign(USAGE_DEFAULT_GB.at(type).begin(), USAGE_DEFAULT_GB.at(type).end());
            }
            break;
        }
        case 0x000012A3: {
            if (USAGE_DEFAULT_US.count(type) > 0) {
                units.assign(USAGE_DEFAULT_US.at(type).begin(), USAGE_DEFAULT_US.at(type).end());
            }
            break;
        }
        default: {
            if (USAGE_DEFAULT_001.count(type) > 0) {
                units.assign(USAGE_DEFAULT_001.at(type).begin(), USAGE_DEFAULT_001.at(type).end());
            }
            break;
        }
    }
}

void GetFallbackPreferredUnit(const string &region, const string &usage, vector<string> &units)
{
    switch (GetMask(region)) {
        case 0x00000EA8: {
            if (USAGE_MX.count(usage) > 0) {
                units.assign(USAGE_MX.at(usage).begin(), USAGE_MX.at(usage).end());
                return;
            }
            break;
        }
        case 0x00000F1C: {
            if (USAGE_NL.count(usage) > 0) {
                units.assign(USAGE_NL.at(usage).begin(), USAGE_NL.at(usage).end());
                return;
            }
            break;
        }
        case 0x00000F1F:
        case 0x0000101C:
        case 0x00001125: {
            if (USAGE_NO.count(usage) > 0) {
                units.assign(USAGE_NO.at(usage).begin(), USAGE_NO.at(usage).end());
                return;
            }
            break;
        }
        case 0x00001195: {
            if (USAGE_SE.count(usage) > 0) {
                units.assign(USAGE_SE.at(usage).begin(), USAGE_SE.at(usage).end());
                return;
            }
            break;
        }
        case 0x000012A3: {
            if (USAGE_US.count(usage) > 0) {
                units.assign(USAGE_US.at(usage).begin(), USAGE_US.at(usage).end());
                return;
            }
            break;
        }
        default: {
            if (USAGE_001.count(usage) > 0) {
                units.assign(USAGE_001.at(usage).begin(), USAGE_001.at(usage).end());
                return;
            }
            break;
        }
    }
}

void GetRestPreferredUnit(const string &region, const string &usage, vector<string> &units)
{
    switch (GetMask(region)) {
        case 0x00000991:
        case 0x00000C9E: {
            if (USAGE_CA.count(usage) > 0) {
                units.assign(USAGE_CA.at(usage).begin(), USAGE_CA.at(usage).end());
                return;
            }
            break;
        }
        case 0x0000099E:
        case 0x00000A1B:
        case 0x00001024: {
            if (USAGE_CN.count(usage) > 0) {
                units.assign(USAGE_CN.at(usage).begin(), USAGE_CN.at(usage).end());
                return;
            }
            break;
        }
        case 0x00000A15: {
            if (USAGE_DE.count(usage) > 0) {
                units.assign(USAGE_DE.at(usage).begin(), USAGE_DE.at(usage).end());
                return;
            }
            break;
        }
        case 0x00000B92: {
            if (USAGE_GB.count(usage) > 0) {
                units.assign(USAGE_GB.at(usage).begin(), USAGE_GB.at(usage).end());
                return;
            }
            break;
        }
        case 0x00000DA2: {
            if (USAGE_KR.count(usage) > 0) {
                units.assign(USAGE_KR.at(usage).begin(), USAGE_KR.at(usage).end());
                return;
            }
            break;
        }
        default: {
            GetFallbackPreferredUnit(region, usage, units);
            break;
        }
    }
    if (!units.size()) {
        GetDefaultPreferredUnit(region, usage, units);
    }
}

void GetPreferredUnit(const string &region, const string &usage, vector<string> &units)
{
    switch (GetMask(region)) {
        case 0x000008A4:
        case 0x00000915:
        case 0x00000A2A:
        case 0x00000A97:
        case 0x00000AA3:
        case 0x00000B22:
        case 0x00000C1B:
        case 0x00000C94:
        case 0x00000C9C:
        case 0x00000CA4:
        case 0x00000D1F:
        case 0x00000EA9:
        case 0x00001191:
        case 0x00001222:
        case 0x0000131E: {
            if (USAGE_AT.count(usage) > 0) {
                units.assign(USAGE_AT.at(usage).begin(), USAGE_AT.at(usage).end());
                return;
            }
            break;
        }
        case 0x00000922: {
            if (USAGE_BR.count(usage) > 0) {
                units.assign(USAGE_BR.at(usage).begin(), USAGE_BR.at(usage).end());
                return;
            }
            break;
        }
        case 0x00000923:
        case 0x0000092A:
        case 0x00001022:
        case 0x00001027: {
            if (USAGE_BS.count(usage) > 0) {
                units.assign(USAGE_BS.at(usage).begin(), USAGE_BS.at(usage).end());
                return;
            }
            break;
        }
        default: {
            GetRestPreferredUnit(region, usage, units);
            break;
        }
    }
    if (!units.size()) {
        GetRestPreferredUnit(region, usage, units);
    }
}

void ComputeFactorValue(const string &unit, const string &measSys, vector<double> &factors)
{
    string unitKey = unit + "-" + measSys;
    if (CONVERT_FACTORS.count(unitKey) > 0) {
        factors[0] = CONVERT_FACTORS.at(unitKey)[0];
        factors[1] = CONVERT_FACTORS.at(unitKey)[1];
        return;
    }
    if (CONVERT_FACTORS.count(unit) > 0) {
        factors[0] = CONVERT_FACTORS.at(unit)[0];
        factors[1] = CONVERT_FACTORS.at(unit)[1];
    }
}

double ComputeSIPrefixValue(const string &unit)
{
    for (auto& prefixValue : PREFIX_VALUE) {
        if (!unit.rfind(prefixValue.first, 0)) {
            return prefixValue.second;
        }
    }
    return 0.0;
}

void ComputePowerValue(const string &unit, const string &measSys, vector<double> &factors)
{
    for (auto& powerValue : POWER_VALUE) {
        if (!unit.rfind(powerValue.first, 0)) {
            string baseUnit = unit.substr(powerValue.first.length());
            double value = ComputeSIPrefixValue(baseUnit);
            double compare = 0.0;
            if (fabs(value - compare) < 1e-6) {
                ComputeFactorValue(baseUnit, measSys, factors);
            }
            factors[0] = pow(factors[0], powerValue.second);
        }
    }
}

int ComputeValue(const string &unit, const string &measSys, vector<double> &factors)
{
    if (unit.find("-per-") != string::npos) {
        vector<string> baseValues;
        Split(unit, "-per-", baseValues);
        if (baseValues.size() == BASE_VALUE_SIZE) {
            vector<double> numerator = { 1.0, 0.0 };
            int status = ComputeValue(baseValues[0], measSys, numerator);
            if (!status) {
                return 0;
            }
            vector<double> denominator = { 1.0, 0.0 };
            status = ComputeValue(baseValues[1], measSys, denominator);
            if (!status) {
                return 0;
            }
            factors[0] = numerator[0] / denominator[0];
            return 1;
        }
    }
    double compare = 0.0;
    factors[0] = ComputeSIPrefixValue(unit);
    if (fabs(factors[0] - compare) < 1e-6) {
        ComputePowerValue(unit, measSys, factors);
    }
    if (fabs(factors[0] - compare) < 1e-6) {
        ComputeFactorValue(unit, measSys, factors);
    }
    if (fabs(factors[0] - compare) < 1e-6) {
        factors[0] = 1.0;
        return 0;
    }
    return 1;
}

int Convert(double &value, const string &fromUnit, const string &fromMeasSys, const string &toUnit,
    const string &toMeasSys)
{
    double baseResult = 0.0;
    double result = 0.0;
    vector<double> fromFactors = {0.0, 0.0};
    string fromUnitType;
    string toUnitType;
    icu::MeasureUnit unitArray[MAX_UNIT_NUM];
    UErrorCode icuStatus = U_ZERO_ERROR;
    icu::MeasureUnit::getAvailable(unitArray, MAX_UNIT_NUM, icuStatus);
    if (!U_SUCCESS(icuStatus)) {
        return 0;
    }
    for (icu::MeasureUnit curUnit : unitArray) {
        if (!strcmp(curUnit.getSubtype(), fromUnit.c_str())) {
            fromUnitType = curUnit.getType();
        }
        if (!strcmp(curUnit.getSubtype(), toUnit.c_str())) {
            toUnitType = curUnit.getType();
        }
    }
    if (fromUnitType.empty() || toUnitType.empty() || strcmp(fromUnitType.c_str(), toUnitType.c_str())) {
        return 0;
    }
    int status = ComputeValue(fromUnit, fromMeasSys, fromFactors);
    if (!status) {
        return 0;
    }
    vector<double> toFactors = {0.0, 0.0};
    status = ComputeValue(toUnit, toMeasSys, toFactors);
    if (!status) {
        return 0;
    }
    if (fromFactors.size() == FACTOR_SIZE) {
        baseResult = fromFactors[0] * value + fromFactors[1];
    }
    if (toFactors.size() == FACTOR_SIZE) {
        result = (baseResult - toFactors[1]) / toFactors[0];
    }
    value = result;
    return 1;
}

bool ConvertByte(double& number, std::string& unit)
{
    double tempNumber = number;
    if (tempNumber < 0) {
        tempNumber *= -1;
    }
    std::string tempUnit = unit;
    size_t indexFirst = -1;
    for (size_t i = 0; i < BYTE_VALUE.size(); i++) {
        if (BYTE_VALUE[i] == tempUnit) {
            indexFirst = i;
            break;
        }
    }
    bool status = (indexFirst >= 0);
    size_t indexSecond = indexFirst;
    for (size_t i = indexFirst; i > 0; i--) {
        if (tempNumber < 1) {
            // 10 ^ 3 is the threshold for carry
            tempNumber *= pow(10, 3);
            tempUnit = BYTE_VALUE[i - 1];
            indexSecond = i - 1;
        }
    }
    
    for (size_t i = indexSecond + 1; i < BYTE_VALUE.size(); i++) {
        // 900 is the threshold for carry
        if (tempNumber > 900) {
            // 10 ^ 3 is the threshold for carry
            tempNumber /= pow(10, 3);
            tempUnit = BYTE_VALUE[i];
        } else {
            break;
        }
    }

    if (status) {
        number = tempNumber;
        unit = tempUnit;
        return true;
    }
    return false;
}

bool ConvertDate(double& number, std::string& unit)
{
    double tempNumber = number;
    if (tempNumber < 0) {
        tempNumber *= -1;
    }
    std::string tempUnit = unit;

    size_t indexFirst = -1;
    for (size_t i = 0; i < DATE_VALUE.size(); i++) {
        if (DATE_VALUE[i].first == tempUnit) {
            indexFirst = i;
            break;
        }
    }
    bool status = (indexFirst >= 0);
    size_t indexSecond = indexFirst;
    for (size_t i = indexFirst; i > 0; i--) {
        if (tempNumber < 1) {
            tempNumber *= DATE_VALUE[i].second;
            tempUnit = DATE_VALUE[i - 1].first;
            indexSecond = i - 1;
        }
    }

    for (size_t i = indexSecond + 1; i < DATE_VALUE.size(); i++) {
        if ((tempNumber / DATE_VALUE[i].second) >= 1) {
            tempNumber /= DATE_VALUE[i].second;
            tempUnit = DATE_VALUE[i].first;
        } else {
            break;
        }
    }

    if (status) {
        number = floor(tempNumber);
        number *= -1;
        unit = tempUnit;
        return true;
    }
    return false;
}

} // namespace I18n
} // namespace Global
} // namespace OHOS