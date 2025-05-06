/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "number_format.h"

#include "ohos/init_data.h"
#include "locale_config.h"
#include "utils.h"

namespace OHOS {
namespace Global {
namespace I18n {
bool NumberFormat::icuInitialized = NumberFormat::Init();

std::unordered_map<std::string, UNumberUnitWidth> NumberFormat::unitStyle = {
    { "long", UNumberUnitWidth::UNUM_UNIT_WIDTH_FULL_NAME },
    { "short", UNumberUnitWidth::UNUM_UNIT_WIDTH_SHORT },
    { "narrow", UNumberUnitWidth::UNUM_UNIT_WIDTH_NARROW }
};

std::unordered_map<std::string, UNumberUnitWidth> NumberFormat::currencyStyle = {
    { "symbol", UNumberUnitWidth::UNUM_UNIT_WIDTH_SHORT },
    { "code", UNumberUnitWidth::UNUM_UNIT_WIDTH_ISO_CODE },
    { "name", UNumberUnitWidth::UNUM_UNIT_WIDTH_FULL_NAME },
    { "narrowSymbol", UNumberUnitWidth::UNUM_UNIT_WIDTH_NARROW }
};

std::unordered_map<std::string, UNumberSignDisplay> NumberFormat::signAutoStyle = {
    { "auto", UNumberSignDisplay::UNUM_SIGN_AUTO },
    { "never", UNumberSignDisplay::UNUM_SIGN_NEVER },
    { "always", UNumberSignDisplay::UNUM_SIGN_ALWAYS },
    { "exceptZero", UNumberSignDisplay::UNUM_SIGN_EXCEPT_ZERO }
};

std::unordered_map<std::string, UNumberSignDisplay> NumberFormat::signAccountingStyle = {
    { "auto", UNumberSignDisplay::UNUM_SIGN_ACCOUNTING },
    { "never", UNumberSignDisplay::UNUM_SIGN_NEVER },
    { "always", UNumberSignDisplay::UNUM_SIGN_ACCOUNTING_ALWAYS },
    { "exceptZero", UNumberSignDisplay::UNUM_SIGN_ACCOUNTING_EXCEPT_ZERO }
};

std::unordered_map<UMeasurementSystem, std::string> NumberFormat::measurementSystem = {
    { UMeasurementSystem::UMS_SI, "SI" },
    { UMeasurementSystem::UMS_US, "US" },
    { UMeasurementSystem::UMS_UK, "UK" },
};

std::unordered_map<std::string, UNumberUnitWidth> NumberFormat::defaultUnitStyle = {
    { "tablet", UNumberUnitWidth::UNUM_UNIT_WIDTH_FULL_NAME },
    { "2in1", UNumberUnitWidth::UNUM_UNIT_WIDTH_FULL_NAME },
    { "tv", UNumberUnitWidth::UNUM_UNIT_WIDTH_FULL_NAME },
    { "pc", UNumberUnitWidth::UNUM_UNIT_WIDTH_FULL_NAME },
    { "wearable", UNumberUnitWidth::UNUM_UNIT_WIDTH_NARROW },
    { "liteWearable", UNumberUnitWidth::UNUM_UNIT_WIDTH_NARROW },
    { "watch", UNumberUnitWidth::UNUM_UNIT_WIDTH_NARROW }
};

std::unordered_map<std::string, UNumberUnitWidth> NumberFormat::defaultCurrencyStyle = {
    { "wearable", UNumberUnitWidth::UNUM_UNIT_WIDTH_NARROW },
    { "liteWearable", UNumberUnitWidth::UNUM_UNIT_WIDTH_NARROW },
    { "watch", UNumberUnitWidth::UNUM_UNIT_WIDTH_NARROW }
};

std::map<std::string, std::string> NumberFormat::RelativeTimeFormatConfigs = {
    { "numeric", "auto" }
};

NumberFormat::NumberFormat(const std::vector<std::string> &localeTags, std::map<std::string, std::string> &configs)
{
    SetDefaultStyle();
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::LocaleBuilder> builder = nullptr;
    builder = std::make_unique<icu::LocaleBuilder>();
    ParseConfigs(configs);
    for (size_t i = 0; i < localeTags.size(); i++) {
        std::string curLocale = localeTags[i];
        locale = icu::Locale::forLanguageTag(icu::StringPiece(curLocale), status);
        if (status != U_ZERO_ERROR) {
            status = U_ZERO_ERROR;
            continue;
        }
        if (LocaleInfo::GetValidLocales().count(locale.getLanguage()) > 0) {
            localeInfo = std::make_unique<LocaleInfo>(curLocale, configs);
            CreateRelativeTimeFormat(curLocale);
            if (!localeInfo->InitSuccess()) {
                continue;
            }
            locale = localeInfo->GetLocale();
            localeBaseName = localeInfo->GetBaseName();
            numberFormat = icu::number::NumberFormatter::withLocale(locale);
            icu::MeasureUnit::getAvailable(unitArray, MAX_UNIT_NUM, status);
            if (!U_SUCCESS(status)) {
                status = U_ZERO_ERROR;
                continue;
            }
            createSuccess = true;
            break;
        }
    }
    if (!createSuccess) {
        std::string systemLocale = LocaleConfig::GetSystemLocaleWithExtParam();
        localeInfo = std::make_unique<LocaleInfo>(systemLocale, configs);
        CreateRelativeTimeFormat(systemLocale);
        if (localeInfo->InitSuccess()) {
            locale = localeInfo->GetLocale();
            localeBaseName = localeInfo->GetBaseName();
            numberFormat = icu::number::NumberFormatter::withLocale(locale);
            icu::MeasureUnit::getAvailable(unitArray, MAX_UNIT_NUM, status);
            if (U_SUCCESS(status)) {
                createSuccess = true;
            }
        }
    }
    if (createSuccess) {
        InitProperties();
    }
}

NumberFormat::~NumberFormat()
{
}

void NumberFormat::CreateRelativeTimeFormat(const std::string& locale)
{
    if (unitUsage == "elapsed-time-second") {
        std::vector<std::string> locales = { locale };
        relativeTimeFormat = std::make_unique<RelativeTimeFormat>(locales,
            RelativeTimeFormatConfigs);
    }
}

void NumberFormat::InitProperties()
{
    if (!currency.empty()) {
        UErrorCode status = U_ZERO_ERROR;
        numberFormat =
            numberFormat.unit(icu::CurrencyUnit(icu::UnicodeString(currency.c_str()).getBuffer(), status));
        if (currencyDisplay != UNumberUnitWidth::UNUM_UNIT_WIDTH_SHORT) {
            numberFormat = numberFormat.unitWidth(currencyDisplay);
        }
    }
    if (!styleString.empty() && styleString == "percent") {
        numberFormat = numberFormat.unit(icu::NoUnit::percent())
            .scale(icu::number::Scale::powerOfTen(2)) // 2 means divided by 100
            .precision(icu::number::Precision::fixedFraction(0));
    }
    if (!styleString.empty() && styleString == "unit") {
        for (icu::MeasureUnit curUnit : unitArray) {
            if (!strcmp(curUnit.getSubtype(), unit.c_str())) {
                numberFormat = numberFormat.unit(curUnit);
                unitType = curUnit.getType();
            }
        }
        UErrorCode status = U_ZERO_ERROR;
        UMeasurementSystem measSys = ulocdata_getMeasurementSystem(localeBaseName.c_str(), &status);
        if (U_SUCCESS(status) && measSys >= 0) {
            unitMeasSys = measurementSystem[measSys];
        }
        numberFormat = numberFormat.unitWidth(unitDisplay);
        numberFormat = numberFormat.precision(icu::number::Precision::maxFraction(DEFAULT_FRACTION_DIGITS));
    }
    if (!useGrouping.empty()) {
        numberFormat = numberFormat.grouping((useGrouping == "true") ?
            UNumberGroupingStrategy::UNUM_GROUPING_AUTO : UNumberGroupingStrategy::UNUM_GROUPING_OFF);
    }
    if (!currencySign.empty() || !signDisplayString.empty()) {
        numberFormat = numberFormat.sign(signDisplay);
    }
    if (!notationString.empty()) {
        numberFormat = numberFormat.notation(notation);
    }
    InitDigitsProperties();
}

void NumberFormat::InitDigitsProperties()
{
    int32_t status = 0;
    if (!maximumSignificantDigits.empty() || !minimumSignificantDigits.empty()) {
        int32_t maxSignificantDigits = ConvertString2Int(maximumSignificantDigits, status);
        if (status == 0) {
            numberFormat = numberFormat.precision(icu::number::Precision::maxSignificantDigits(maxSignificantDigits));
        }

        status = 0;
        int32_t minSignificantDigits = ConvertString2Int(minimumSignificantDigits, status);
        if (status == 0) {
            numberFormat = numberFormat.precision(icu::number::Precision::minSignificantDigits(minSignificantDigits));
        }
    } else {
        int32_t minIntegerDigits = ConvertString2Int(minimumIntegerDigits, status);
        if (status == 0 && minIntegerDigits > 1) {
            numberFormat =
                numberFormat.integerWidth(icu::number::IntegerWidth::zeroFillTo(minIntegerDigits));
        }

        status = 0;
        int32_t minFractionDigits = ConvertString2Int(minimumFractionDigits, status);
        if (status == 0) {
            numberFormat =
                numberFormat.precision(icu::number::Precision::minFraction(minFractionDigits));
        }

        status = 0;
        int32_t maxFractionDigits = ConvertString2Int(maximumFractionDigits, status);
        if (status == 0) {
            numberFormat =
                numberFormat.precision(icu::number::Precision::maxFraction(maxFractionDigits));
        }
    }
}

void NumberFormat::ParseConfigs(std::map<std::string, std::string> &configs)
{
    if (configs.count("signDisplay") > 0) {
        signDisplayString = configs["signDisplay"];
    }
    if (signAutoStyle.count(signDisplayString) > 0) {
        signDisplay = signAutoStyle[signDisplayString];
    }
    if (configs.count("style") > 0) {
        styleString = configs["style"];
    }
    if (styleString == "unit" && configs.count("unit") > 0) {
        unit = configs["unit"];
        if (configs.count("unitDisplay") > 0) {
            unitDisplayString = configs["unitDisplay"];
            if (unitStyle.count(unitDisplayString) > 0) {
                unitDisplay = unitStyle[unitDisplayString];
            }
        }
        if (configs.count("unitUsage") > 0) {
            unitUsage = configs["unitUsage"];
        }
    }
    if (styleString == "currency" && configs.count("currency") > 0) {
        currency = configs["currency"];
        if (configs.count("currencySign") > 0) {
            currencySign = configs["currencySign"];
        }
        if (currencySign.compare("accounting") == 0 && signAccountingStyle.count(signDisplayString) > 0) {
            signDisplay = signAccountingStyle[signDisplayString];
        }
        if (configs.count("currencyDisplay") > 0 && currencyStyle.count(configs["currencyDisplay"]) > 0) {
            currencyDisplayString = configs["currencyDisplay"];
            currencyDisplay = currencyStyle[currencyDisplayString];
        }
    }
    ParseDigitsConfigs(configs);
}

void NumberFormat::ParseDigitsConfigs(std::map<std::string, std::string> &configs)
{
    if (configs.count("notation") > 0) {
        notationString = configs["notation"];
        if (notationString == "scientific") {
            notation = icu::number::Notation::scientific();
        } else if (notationString == "engineering") {
            notation = icu::number::Notation::engineering();
        }
        if (notationString == "compact" && configs.count("compactDisplay") > 0) {
            compactDisplay = configs["compactDisplay"];
            if (compactDisplay == "long") {
                notation = icu::number::Notation::compactLong();
            } else {
                notation = icu::number::Notation::compactShort();
            }
        }
    }
    if (configs.count("minimumIntegerDigits") > 0) {
        minimumIntegerDigits = configs["minimumIntegerDigits"];
    }
    if (configs.count("minimumFractionDigits") > 0) {
        minimumFractionDigits = configs["minimumFractionDigits"];
    }
    if (configs.count("maximumFractionDigits") > 0) {
        maximumFractionDigits = configs["maximumFractionDigits"];
    }
    if (configs.count("minimumSignificantDigits") > 0) {
        minimumSignificantDigits = configs["minimumSignificantDigits"];
    }
    if (configs.count("maximumSignificantDigits") > 0) {
        maximumSignificantDigits = configs["maximumSignificantDigits"];
    }
    if (configs.count("numberingSystem") > 0) {
        numberingSystem = configs["numberingSystem"];
    }
    if (configs.count("useGrouping") > 0) {
        useGrouping = configs["useGrouping"];
    }
    if (configs.count("localeMatcher") > 0) {
        localeMatcher = configs["localeMatcher"];
    }
}

void NumberFormat::SetUnit(std::string &preferredUnit)
{
    if (preferredUnit.empty()) {
        return;
    }
    for (icu::MeasureUnit curUnit : unitArray) {
        if (!strcmp(curUnit.getSubtype(), preferredUnit.c_str())) {
            numberFormat = numberFormat.unit(curUnit);
        }
    }
}

std::string NumberFormat::Format(double number)
{
    if (!createSuccess) {
        return "";
    }
    double finalNumber = number;
    std::string finalUnit = unit;
    if ((unitUsage == "size-file-byte" || unitUsage == "size-shortfile-byte") && ConvertByte(finalNumber, finalUnit)) {
        SetUnit(finalUnit);
        SetPrecisionWithByte(finalNumber, finalUnit);
    } else if (unitUsage == "elapsed-time-second" && ConvertDate(finalNumber, finalUnit) &&
        relativeTimeFormat != nullptr) {
        return relativeTimeFormat->Format(finalNumber, finalUnit);
    } else if (!unitUsage.empty()) {
        std::vector<std::string> preferredUnits;
        if (unitUsage == "default") {
            GetDefaultPreferredUnit(localeInfo->GetRegion(), unitType, preferredUnits);
        } else {
            GetPreferredUnit(localeInfo->GetRegion(), unitUsage, preferredUnits);
        }
        std::map<double, std::string> preferredValuesOverOne;
        std::map<double, std::string> preferredValuesUnderOne;
        double num = number;
        for (size_t i = 0; i < preferredUnits.size(); i++) {
            int status = Convert(num, unit, unitMeasSys, preferredUnits[i], unitMeasSys);
            if (!status) {
                continue;
            }
            if (num >= 1) {
                preferredValuesOverOne.insert(std::make_pair(num, preferredUnits[i]));
            } else {
                preferredValuesUnderOne.insert(std::make_pair(num, preferredUnits[i]));
            }
        }
        std::string preferredUnit;
        if (preferredValuesOverOne.size() > 0) {
            finalNumber = preferredValuesOverOne.begin()->first;
            preferredUnit = preferredValuesOverOne.begin()->second;
        } else if (preferredValuesUnderOne.size() > 0) {
            finalNumber = preferredValuesUnderOne.rbegin()->first;
            preferredUnit = preferredValuesUnderOne.rbegin()->second;
        }
        SetUnit(preferredUnit);
    }
    std::string result;
    UErrorCode status = U_ZERO_ERROR;
    numberFormat.formatDouble(finalNumber, status).toString(status).toUTF8String(result);
    return result;
}

void NumberFormat::GetResolvedOptions(std::map<std::string, std::string> &map)
{
    map.insert(std::make_pair("locale", localeBaseName));
    if (!styleString.empty()) {
        map.insert(std::make_pair("style", styleString));
    }
    if (!currency.empty()) {
        map.insert(std::make_pair("currency", currency));
    }
    if (!currencySign.empty()) {
        map.insert(std::make_pair("currencySign", currencySign));
    }
    if (!currencyDisplayString.empty()) {
        map.insert(std::make_pair("currencyDisplay", currencyDisplayString));
    }
    if (!signDisplayString.empty()) {
        map.insert(std::make_pair("signDisplay", signDisplayString));
    }
    if (!compactDisplay.empty()) {
        map.insert(std::make_pair("compactDisplay", compactDisplay));
    }
    if (!unitDisplayString.empty()) {
        map.insert(std::make_pair("unitDisplay", unitDisplayString));
    }
    if (!unitUsage.empty()) {
        map.insert(std::make_pair("unitUsage", unitUsage));
    }
    if (!unit.empty()) {
        map.insert(std::make_pair("unit", unit));
    }
    GetDigitsResolvedOptions(map);
}

void NumberFormat::GetDigitsResolvedOptions(std::map<std::string, std::string> &map)
{
    if (!numberingSystem.empty()) {
        map.insert(std::make_pair("numberingSystem", numberingSystem));
    } else if (!(localeInfo->GetNumberingSystem()).empty()) {
        map.insert(std::make_pair("numberingSystem", localeInfo->GetNumberingSystem()));
    } else {
        UErrorCode status = U_ZERO_ERROR;
        auto numSys = std::unique_ptr<icu::NumberingSystem>(icu::NumberingSystem::createInstance(locale, status));
        if (U_SUCCESS(status)) {
            map.insert(std::make_pair("numberingSystem", numSys->getName()));
        }
    }
    if (!useGrouping.empty()) {
        map.insert(std::make_pair("useGrouping", useGrouping));
    }
    if (!minimumIntegerDigits.empty()) {
        map.insert(std::make_pair("minimumIntegerDigits", minimumIntegerDigits));
    }
    if (!minimumFractionDigits.empty()) {
        map.insert(std::make_pair("minimumFractionDigits", minimumFractionDigits));
    }
    if (!maximumFractionDigits.empty()) {
        map.insert(std::make_pair("maximumFractionDigits", maximumFractionDigits));
    }
    if (!minimumSignificantDigits.empty()) {
        map.insert(std::make_pair("minimumSignificantDigits", minimumSignificantDigits));
    }
    if (!maximumSignificantDigits.empty()) {
        map.insert(std::make_pair("maximumSignificantDigits", maximumSignificantDigits));
    }
    if (!localeMatcher.empty()) {
        map.insert(std::make_pair("localeMatcher", localeMatcher));
    }
    if (!notationString.empty()) {
        map.insert(std::make_pair("notation", notationString));
    }
}

void NumberFormat::SetPrecisionWithByte(double number, const std::string& finalUnit)
{
    int32_t FractionDigits = -1;

    // 100 is the threshold between different decimal
    if (finalUnit == "byte" || number >= 100) {
        FractionDigits = 0;
    } else if (number < 1) {
        // 2 is the number of significant digits in the decimal
        FractionDigits = 2;
    // 10 is the threshold between different decimal
    } else if (number < 10) {

        if (unitUsage == "size-shortfile-byte") {
            FractionDigits = 1;
        } else {
            // 2 is the number of significant digits in the decimal
            FractionDigits = 2;
        }
    } else {
        if (unitUsage == "size-shortfile-byte") {
            FractionDigits = 0;
        } else {
            // 2 is the number of significant digits in the decimal
            FractionDigits = 2;
        }
    }
    if (FractionDigits != -1) {
        numberFormat = numberFormat.precision(icu::number::Precision::minMaxFraction(FractionDigits, FractionDigits));
    }
}

std::string NumberFormat::GetCurrency() const
{
    return currency;
}

std::string NumberFormat::GetCurrencySign() const
{
    return currencySign;
}

std::string NumberFormat::GetStyle() const
{
    return styleString;
}

std::string NumberFormat::GetNumberingSystem() const
{
    return numberingSystem;
}

std::string NumberFormat::GetUseGrouping() const
{
    return useGrouping;
}

std::string NumberFormat::GetMinimumIntegerDigits() const
{
    return minimumIntegerDigits;
}

std::string NumberFormat::GetMinimumFractionDigits() const
{
    return minimumFractionDigits;
}

std::string NumberFormat::GetMaximumFractionDigits() const
{
    return maximumFractionDigits;
}

std::string NumberFormat::GetMinimumSignificantDigits() const
{
    return minimumSignificantDigits;
}

std::string NumberFormat::GetMaximumSignificantDigits() const
{
    return maximumSignificantDigits;
}

std::string NumberFormat::GetLocaleMatcher() const
{
    return localeMatcher;
}

bool NumberFormat::Init()
{
    return true;
}

void NumberFormat::SetDefaultStyle()
{
    auto plugin = Plugin::INTL::Create();
    if (!plugin) {
        return;
    }
    std::string deviceType = plugin->GetDeviceType();
    if (defaultUnitStyle.find(deviceType) != defaultUnitStyle.end()) {
        unitDisplay = defaultUnitStyle[deviceType];
    }
    if (defaultCurrencyStyle.find(deviceType) != defaultCurrencyStyle.end()) {
        currencyDisplay = defaultCurrencyStyle[deviceType];
    }
}
} // namespace I18n
} // namespace Global
} // namespace OHOS
