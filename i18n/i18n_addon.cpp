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
#include <chrono>
#include <unordered_map>
#include <vector>

#include "character.h"
#include "error_util.h"
#include "i18n_calendar.h"
#include "icu_data.h"
#include "log.h"
#include "locale_info.h"
#include "locale_matcher.h"
#include "napi_utils.h"
#include "node_api.h"
#include "plugin_utils.h"
#include "unicode/locid.h"
#include "unicode/datefmt.h"
#include "unicode/smpdtfmt.h"
#include "unicode/translit.h"
#include "utils.h"

#include "i18n_addon.h"

#ifdef ANDROID_PLATFORM
#include "plugins/i18n/android/java/jni/i18n_plugin_jni.h"
#endif

namespace OHOS {
namespace Global {
namespace I18n {
static thread_local napi_ref* g_constructor = nullptr;
static thread_local napi_ref* g_brkConstructor = nullptr;
static thread_local napi_ref* g_timezoneConstructor = nullptr;
static thread_local napi_ref g_indexUtilConstructor = nullptr;
static thread_local napi_ref* g_transConstructor = nullptr;
static thread_local napi_ref* g_normalizerConstructor = nullptr;
static std::unordered_map<std::string, UCalendarDateFields> g_fieldsMap {
    { "era", UCAL_ERA },
    { "year", UCAL_YEAR },
    { "month", UCAL_MONTH },
    { "week_of_year", UCAL_WEEK_OF_YEAR },
    { "week_of_month", UCAL_WEEK_OF_MONTH },
    { "date", UCAL_DATE },
    { "day_of_year", UCAL_DAY_OF_YEAR },
    { "day_of_week", UCAL_DAY_OF_WEEK },
    { "day_of_week_in_month", UCAL_DAY_OF_WEEK_IN_MONTH },
    { "ap_pm", UCAL_AM_PM },
    { "hour", UCAL_HOUR },
    { "hour_of_day", UCAL_HOUR_OF_DAY },
    { "minute", UCAL_MINUTE },
    { "second", UCAL_SECOND },
    { "millisecond", UCAL_MILLISECOND },
    { "zone_offset", UCAL_ZONE_OFFSET },
    { "dst_offset", UCAL_DST_OFFSET },
    { "year_woy", UCAL_YEAR_WOY },
    { "dow_local", UCAL_DOW_LOCAL },
    { "extended_year", UCAL_EXTENDED_YEAR },
    { "julian_day", UCAL_JULIAN_DAY },
    { "milliseconds_in_day", UCAL_MILLISECONDS_IN_DAY },
    { "is_leap_month", UCAL_IS_LEAP_MONTH },
};
static std::unordered_set<std::string> g_fieldsInFunctionAdd {
    "year", "month", "date", "hour", "minute", "second", "millisecond",
    "week_of_year", "week_of_month", "day_of_year", "day_of_week",
    "day_of_week_in_month", "hour_of_day", "milliseconds_in_day",
};
static std::unordered_map<std::string, CalendarType> g_typeMap {
    { "buddhist", CalendarType::BUDDHIST },
    { "chinese", CalendarType::CHINESE },
    { "coptic", CalendarType::COPTIC },
    { "ethiopic", CalendarType::ETHIOPIC },
    { "hebrew", CalendarType::HEBREW },
    { "gregory", CalendarType::GREGORY },
    { "indian", CalendarType::INDIAN },
    { "islamic_civil", CalendarType::ISLAMIC_CIVIL },
    { "islamic_tbla", CalendarType::ISLAMIC_TBLA },
    { "islamic_umalqura", CalendarType::ISLAMIC_UMALQURA },
    { "japanese", CalendarType::JAPANESE },
    { "persion", CalendarType::PERSIAN },
};

const char *I18nAddon::NORMALIZER_MODE_NFC_NAME = "NFC";
const char *I18nAddon::NORMALIZER_MODE_NFD_NAME = "NFD";
const char *I18nAddon::NORMALIZER_MODE_NFKC_NAME = "NFKC";
const char *I18nAddon::NORMALIZER_MODE_NFKD_NAME = "NFKD";

I18nAddon::I18nAddon() : env_(nullptr) {}

I18nAddon::~I18nAddon()
{
}

void I18nAddon::Destructor(napi_env env, void *nativeObject, void *hint)
{
    if (!nativeObject) {
        return;
    }
    reinterpret_cast<I18nAddon *>(nativeObject)->~I18nAddon();
}

napi_value I18nAddon::CreateUnicodeObject(napi_env env, napi_status &initStatus)
{
    napi_status status = napi_ok;
    napi_value character = nullptr;
    status = napi_create_object(env, &character);
    if (status != napi_ok) {
        LOGE("Failed to create character object at init");
        initStatus = napi_generic_failure;
        return nullptr;
    }
    napi_property_descriptor characterProperties[] = {
        DECLARE_NAPI_FUNCTION("isDigit", IsDigitAddon),
        DECLARE_NAPI_FUNCTION("isSpaceChar", IsSpaceCharAddon),
        DECLARE_NAPI_FUNCTION("isWhitespace", IsWhiteSpaceAddon),
        DECLARE_NAPI_FUNCTION("isRTL", IsRTLCharacterAddon),
        DECLARE_NAPI_FUNCTION("isIdeograph", IsIdeoGraphicAddon),
        DECLARE_NAPI_FUNCTION("isLetter", IsLetterAddon),
        DECLARE_NAPI_FUNCTION("isLowerCase", IsLowerCaseAddon),
        DECLARE_NAPI_FUNCTION("isUpperCase", IsUpperCaseAddon),
        DECLARE_NAPI_FUNCTION("getType", GetTypeAddon),
    };
    status = napi_define_properties(env, character,
                                    sizeof(characterProperties) / sizeof(napi_property_descriptor),
                                    characterProperties);
    if (status != napi_ok) {
        LOGE("Failed to set properties of character at init");
        initStatus = napi_generic_failure;
        return nullptr;
    }
    return character;
}

napi_value I18nAddon::CreateI18nUtilObject(napi_env env, napi_status &initStatus)
{
    napi_value i18nUtil = nullptr;
    napi_status status = napi_create_object(env, &i18nUtil);
    if (status != napi_ok) {
        LOGE("Failed to create I18nUtil object at init");
        initStatus = napi_generic_failure;
        return nullptr;
    }
    napi_property_descriptor i18nUtilProperties[] = {
        DECLARE_NAPI_FUNCTION("getDateOrder", GetDateOrder),
        DECLARE_NAPI_FUNCTION("getBestMatchLocale", GetBestMatchLocale)
    };
    status = napi_define_properties(env, i18nUtil, sizeof(i18nUtilProperties) / sizeof(napi_property_descriptor),
        i18nUtilProperties);
    if (status != napi_ok) {
        LOGE("Failed to set properties of I18nUtil at init");
        initStatus = napi_generic_failure;
        return nullptr;
    }
    return i18nUtil;
}

napi_value I18nAddon::CreateI18nNormalizerObject(napi_env env, napi_status &initStatus)
{
    napi_value i18nNormalizer = nullptr;
    napi_status status = napi_create_object(env, &i18nNormalizer);
    if (status != napi_ok) {
        LOGE("Failed to create I18nNormalizer object at init");
        initStatus = napi_generic_failure;
        return nullptr;
    }
    napi_property_descriptor i18nNormalizerProperties[] = {
        DECLARE_NAPI_FUNCTION("getInstance", GetI18nNormalizerInstance)
    };
    status = napi_define_properties(env, i18nNormalizer,
        sizeof(i18nNormalizerProperties) / sizeof(napi_property_descriptor), i18nNormalizerProperties);
    if (status != napi_ok) {
        LOGE("Failed to set properties of I18nNormalizer at init");
        initStatus = napi_generic_failure;
        return nullptr;
    }
    return i18nNormalizer;
}

napi_status I18nAddon::SetEnumValue(napi_env env, napi_value enumObj, const char* enumName, int32_t enumVal)
{
    napi_value name = nullptr;
    napi_status status = napi_create_string_utf8(env, enumName, NAPI_AUTO_LENGTH, &name);
    if (status != napi_ok) {
        return status;
    }
    napi_value value = nullptr;
    status = napi_create_int32(env, enumVal, &value);
    if (status != napi_ok) {
        return status;
    }
    status = napi_set_property(env, enumObj, name, value);
    if (status != napi_ok) {
        return status;
    }
    status = napi_set_property(env, enumObj, value, name);
    if (status != napi_ok) {
        return status;
    }
    return napi_ok;
}

napi_value I18nAddon::CreateI18NNormalizerModeEnum(napi_env env, napi_status &initStatus)
{
    napi_value i18nNormalizerModel = nullptr;
    napi_status status = napi_create_object(env, &i18nNormalizerModel);
    if (status != napi_ok) {
        initStatus = napi_generic_failure;
        return nullptr;
    }
    status = SetEnumValue(env, i18nNormalizerModel, NORMALIZER_MODE_NFC_NAME, NORMALIZER_MODE_NFC);
    if (status != napi_ok) {
        initStatus = napi_generic_failure;
        return nullptr;
    }
    status = SetEnumValue(env, i18nNormalizerModel, NORMALIZER_MODE_NFD_NAME, NORMALIZER_MODE_NFD);
    if (status != napi_ok) {
        initStatus = napi_generic_failure;
        return nullptr;
    }
    status = SetEnumValue(env, i18nNormalizerModel, NORMALIZER_MODE_NFKC_NAME, NORMALIZER_MODE_NFKC);
    if (status != napi_ok) {
        initStatus = napi_generic_failure;
        return nullptr;
    }
    status = SetEnumValue(env, i18nNormalizerModel, NORMALIZER_MODE_NFKD_NAME, NORMALIZER_MODE_NFKD);
    if (status != napi_ok) {
        initStatus = napi_generic_failure;
        return nullptr;
    }
    return i18nNormalizerModel;
}

napi_value I18nAddon::Init(napi_env env, napi_value exports)
{
    napi_status initStatus = napi_ok;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_PROPERTY("I18NUtil", CreateI18nUtilObject(env, initStatus)),
        DECLARE_NAPI_FUNCTION("getLineInstance", GetLineInstance),
        DECLARE_NAPI_FUNCTION("getInstance", GetIndexUtil),
        DECLARE_NAPI_PROPERTY("Unicode", CreateUnicodeObject(env, initStatus)),
        DECLARE_NAPI_FUNCTION("is24HourClock", Is24HourClock),
        DECLARE_NAPI_FUNCTION("getTimeZone", GetI18nTimeZone),
        DECLARE_NAPI_FUNCTION("getCalendar", GetCalendar),
        DECLARE_NAPI_FUNCTION("isRTL", IsRTL),
        DECLARE_NAPI_PROPERTY("Transliterator", CreateTransliteratorObject(env, initStatus)),
        DECLARE_NAPI_PROPERTY("TimeZone", CreateTimeZoneObject(env, initStatus)),
        DECLARE_NAPI_PROPERTY("System", CreateSystemObject(env, initStatus)),
        DECLARE_NAPI_PROPERTY("Normalizer", CreateI18nNormalizerObject(env, initStatus)),
        DECLARE_NAPI_PROPERTY("NormalizerMode", CreateI18NNormalizerModeEnum(env, initStatus)),
        DECLARE_NAPI_FUNCTION("getSystemLanguage", GetSystemLanguage),
    };
    initStatus = napi_define_properties(env, exports, sizeof(properties) \
        / sizeof(napi_property_descriptor), properties);
    if (initStatus != napi_ok) {
        LOGE("Failed to set properties at init");
        return nullptr;
    }
    return exports;
}

void GetOptionValue(napi_env env, napi_value options, const std::string &optionName,
    std::string &value)
{
    napi_value optionValue = nullptr;
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, options, &type);
    if (status != napi_ok && type != napi_object) {
        LOGE("Get option failed, option is not an object");
        return;
    }
    bool hasProperty = false;
    napi_status propStatus = napi_has_named_property(env, options, optionName.c_str(), &hasProperty);
    if (propStatus == napi_ok && hasProperty) {
        status = napi_get_named_property(env, options, optionName.c_str(), &optionValue);
        if (status == napi_ok) {
            size_t len;
            napi_get_value_string_utf8(env, optionValue, nullptr, 0, &len);
            std::vector<char> optionBuf(len + 1);
            status = napi_get_value_string_utf8(env, optionValue, optionBuf.data(), len + 1, &len);
            if (status != napi_ok) {
                LOGE("Failed to get string item");
                return;
            }
            value = optionBuf.data();
        }
    }
}

void GetOptionMap(napi_env env, napi_value option, std::map<std::string, std::string> &map)
{
    if (option != nullptr) {
        size_t len;
        napi_get_value_string_utf8(env, option, nullptr, 0, &len);
        std::vector<char> styleBuf(len + 1);
        napi_status status = napi_get_value_string_utf8(env, option, styleBuf.data(), len + 1, &len);
        if (status != napi_ok) {
            LOGE("Failed to get string item");
            return;
        }
        map.insert(std::make_pair("unitDisplay", styleBuf.data()));
    }
}

napi_value I18nAddon::GetDateOrder(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    size_t len = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    std::vector<char> languageBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], languageBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGE("Failed to get locale string for GetDateOrder");
        return nullptr;
    }
    UErrorCode icuStatus = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(languageBuf.data(), icuStatus);
    if (icuStatus != U_ZERO_ERROR) {
        LOGE("Failed to create locale for GetDateOrder");
        return nullptr;
    }
    icu::SimpleDateFormat* formatter = static_cast<icu::SimpleDateFormat*>
        (icu::DateFormat::createDateInstance(icu::DateFormat::EStyle::kDefault, locale));
    if (icuStatus != U_ZERO_ERROR || formatter == nullptr) {
        LOGE("Failed to create SimpleDateFormat");
        return nullptr;
    }
    std::string tempValue;
    icu::UnicodeString unistr;
    formatter->toPattern(unistr);
    unistr.toUTF8String<std::string>(tempValue);
    std::string value = ModifyOrder(tempValue);
    napi_value result;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Failed to create string item");
        return nullptr;
    }
    return result;
}

LocaleInfo* ProcessJsParamLocale(napi_env env, napi_value argv)
{
    int32_t code = 0;
    std::string localeTag = NAPIUtils::GetString(env, argv, code);
    if (code != 0) {
        LOGE("GetBestMatchLocale get param locale failed.");
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, true);
        return nullptr;
    }
    UErrorCode icuStatus = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(localeTag.data(), icuStatus);
    if (U_FAILURE(icuStatus) || !IsValidLocaleTag(locale)) {
        LOGE("GetBestMatchLocale param locale Invalid.");
        ErrorUtil::NapiThrow(env, I18N_NOT_VALID, true);
        return nullptr;
    }
    return new LocaleInfo(localeTag);
}

bool ProcessJsParamLocaleList(napi_env env, napi_value argv, std::vector<LocaleInfo*> &candidateLocales,
    LocaleInfo *requestLocale)
{
    std::vector<std::string> localeTagList;
    if (!NAPIUtils::GetStringArrayFromJsParam(env, argv, "localeList", localeTagList)) {
        napi_throw_type_error(env, nullptr, "ProcessJsParamLocaleList: Failed to obtain the parameter.");
        return false;
    }
    if (localeTagList.size() == 0) {
        return true;
    }
    for (auto it = localeTagList.begin(); it != localeTagList.end(); ++it) {
        UErrorCode icuStatus = U_ZERO_ERROR;
        icu::Locale locale = icu::Locale::forLanguageTag(it->data(), icuStatus);
        if (U_FAILURE(icuStatus) || !IsValidLocaleTag(locale)) {
            LOGE("GetBestMatchLocale param localeList Invalid: %{public}s.", it->data());
            ErrorUtil::NapiThrow(env, I18N_NOT_VALID, "locale of localeList", "a valid locale", true);
            return false;
        }
        LocaleInfo *temp = new LocaleInfo(*it);
        if (LocaleMatcher::Match(requestLocale, temp)) {
            candidateLocales.push_back(temp);
        } else {
            delete temp;
        }
    }
    return true;
}

void ReleaseParam(LocaleInfo *locale, std::vector<LocaleInfo*> &candidateLocales)
{
    delete locale;
    for (auto it = candidateLocales.begin(); it != candidateLocales.end(); ++it) {
        delete *it;
    }
}

napi_value I18nAddon::GetBestMatchLocale(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok || argc < 2) { // 2 is the request param num.
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, "locale or localeList", "", true);
        return nullptr;
    }
    LocaleInfo *requestLocale = ProcessJsParamLocale(env, argv[0]);
    if (requestLocale == nullptr) {
        return nullptr;
    }
    std::vector<LocaleInfo*> candidateLocales;
    bool isValidParam = ProcessJsParamLocaleList(env, argv[1], candidateLocales, requestLocale);
    if (!isValidParam) {
        ReleaseParam(requestLocale, candidateLocales);
        return nullptr;
    }
    std::string bestMatchLocaleTag = "";
    if (candidateLocales.size() > 0) {
        LocaleInfo *bestMatch = candidateLocales[0];
        for (size_t i = 1; i < candidateLocales.size(); ++i) {
            if (LocaleMatcher::IsMoreSuitable(bestMatch, candidateLocales[i], requestLocale) < 0) {
                bestMatch = candidateLocales[i];
            }
        }
        bestMatchLocaleTag = bestMatch->ToString();
    }
    ReleaseParam(requestLocale, candidateLocales);
    napi_value result = nullptr;
    status = napi_create_string_utf8(env, bestMatchLocaleTag.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Create format stirng failed.");
        return nullptr;
    }
    return result;
}

std::string I18nAddon::ModifyOrder(std::string &pattern)
{
    int order[3] = { 0 }; // total 3 elements 'y', 'M'/'L', 'd'
    int lengths[4] = { 0 }; // first elements is the currently found elememnts, thus 4 elements totally.
    bool flag = true;
    for (size_t i = 0; i < pattern.length(); ++i) {
        char ch = pattern[i];
        if (flag && std::isalpha(ch)) {
            ProcessNormal(ch, order, 3, lengths, 4); // 3, 4 are lengths of these arrays
        } else if (ch == '\'') {
            flag = !flag;
        }
    }
    std::unordered_map<char, int> pattern2index = {
        { 'y', 1 },
        { 'L', 2 },
        { 'd', 3 },
    };
    std::string ret;
    for (int i = 0; i < 3; ++i) { // 3 is the size of orders
        auto it = pattern2index.find(order[i]);
        if (it == pattern2index.end()) {
            continue;
        }
        int index = it->second;
        if ((lengths[index] > 0) && (lengths[index] <= 6)) { // 6 is the max length of a filed
            ret.append(lengths[index], order[i]);
        }
        if (i < 2) { // 2 is the size of the order minus one
            ret.append(1, '-');
        }
    }
    return ret;
}

void I18nAddon::ProcessNormal(char ch, int *order, size_t orderSize, int *lengths, size_t lengsSize)
{
    char adjust;
    int index = -1;
    if (ch == 'd') {
        adjust = 'd';
        index = 3; // 3 is the index of 'd'
    } else if ((ch == 'L') || (ch == 'M')) {
        adjust = 'L';
        index = 2; // 2 is the index of 'L'
    } else if (ch == 'y') {
        adjust = 'y';
        index = 1;
    } else {
        return;
    }
    if ((index < 0) || (index >= static_cast<int>(lengsSize))) {
        return;
    }
    if (lengths[index] == 0) {
        if (lengths[0] >= 3) { // 3 is the index of order
            return;
        }
        order[lengths[0]] = static_cast<int>(adjust);
        ++lengths[0];
        lengths[index] = 1;
    } else {
        ++lengths[index];
    }
}

napi_value I18nAddon::InitTransliterator(napi_env env, napi_value exports)
{
    napi_status status = napi_ok;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("transform", Transform),
    };
    napi_value constructor = nullptr;
    status = napi_define_class(env, "Transliterator", NAPI_AUTO_LENGTH, TransliteratorConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        LOGE("Failed to define transliterator class at Init");
        return nullptr;
    }
    g_transConstructor = new (std::nothrow) napi_ref;
    if (!g_transConstructor) {
        LOGE("Failed to create trans ref at init");
        return nullptr;
    }
    status = napi_create_reference(env, constructor, 1, g_transConstructor);
    if (status != napi_ok) {
        LOGE("Failed to create trans reference at init");
        return nullptr;
    }
    return exports;
}

napi_value I18nAddon::TransliteratorConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int32_t code = 0;
    std::string idTag = NAPIUtils::GetString(env, argv[0], code);
    if (code) {
        return nullptr;
    }
    std::unique_ptr<I18nAddon> obj = nullptr;
    obj = std::make_unique<I18nAddon>();
    status =
        napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()), I18nAddon::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("TransliteratorConstructor: Wrap II18nAddon failed");
        return nullptr;
    }
    if (!obj->InitTransliteratorContext(env, info, idTag)) {
        obj.release();
        return nullptr;
    }
    obj.release();
    return thisVar;
}

bool I18nAddon::InitTransliteratorContext(napi_env env, napi_callback_info info, const std::string &idTag)
{
    UErrorCode status = U_ZERO_ERROR;
    icu::UnicodeString unistr = icu::UnicodeString::fromUTF8(idTag);
    icu::Transliterator *trans = icu::Transliterator::createInstance(unistr, UTransDirection::UTRANS_FORWARD, status);
    if ((status != U_ZERO_ERROR) || (trans == nullptr)) {
        return false;
    }
    transliterator_ = std::unique_ptr<icu::Transliterator>(trans);
    return transliterator_ != nullptr;
}

napi_value I18nAddon::CreateTransliteratorObject(napi_env env, napi_status &initStatus)
{
    napi_status status = napi_ok;
    napi_value transliterator = nullptr;
    status = napi_create_object(env, &transliterator);
    if (status != napi_ok) {
        LOGE("Failed to create transliterator object at init");
        initStatus = napi_generic_failure;
        return nullptr;
    }
    napi_property_descriptor transProperties[] = {
        DECLARE_NAPI_FUNCTION("getAvailableIDs", GetAvailableIDs),
        DECLARE_NAPI_FUNCTION("getInstance", GetTransliteratorInstance)
    };
    status = napi_define_properties(env, transliterator,
                                    sizeof(transProperties) / sizeof(napi_property_descriptor),
                                    transProperties);
    if (status != napi_ok) {
        LOGE("Failed to set properties of transliterator at init");
        initStatus = napi_generic_failure;
        return nullptr;
    }
    return transliterator;
}

napi_value I18nAddon::Transform(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->transliterator_) {
        LOGE("Get Transliterator object failed");
        return nullptr;
    }
    if (argc == 0) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    size_t len = 0;
    status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        LOGE("Get field length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGE("Get string value failed");
        return nullptr;
    }
    icu::UnicodeString unistr = icu::UnicodeString::fromUTF8(buf.data());
    obj->transliterator_->transliterate(unistr);
    std::string temp;
    unistr.toUTF8String(temp);
    napi_value value;
    status = napi_create_string_utf8(env, temp.c_str(), NAPI_AUTO_LENGTH, &value);
    if (status != napi_ok) {
        LOGE("Get field length failed");
        return nullptr;
    }
    return value;
}

napi_value I18nAddon::GetAvailableIDs(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value *argv = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    UErrorCode icuStatus = U_ZERO_ERROR;
    icu::StringEnumeration *strenum = icu::Transliterator::getAvailableIDs(icuStatus);
    if (icuStatus != U_ZERO_ERROR) {
        LOGE("Failed to get available ids");
        if (strenum) {
            delete strenum;
        }
        return nullptr;
    }

    napi_value result = nullptr;
    napi_create_array(env, &result);
    uint32_t i = 0;
    const char *temp = nullptr;
    if (strenum == nullptr) {
        return nullptr;
    }
    while ((temp = strenum->next(nullptr, icuStatus)) != nullptr) {
        if (icuStatus != U_ZERO_ERROR) {
            break;
        }
        napi_value val = nullptr;
        napi_create_string_utf8(env, temp, strlen(temp), &val);
        napi_set_element(env, result, i, val);
        ++i;
    }
    delete strenum;
    return result;
}

napi_value I18nAddon::GetTransliteratorInstance(napi_env env, napi_callback_info info)
{
    size_t argc = 1; // retrieve 2 arguments
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_value constructor = nullptr;
    napi_status status = napi_get_reference_value(env, *g_transConstructor, &constructor);
    if (status != napi_ok) {
        LOGE("Failed to create reference at GetTransliterator");
        return nullptr;
    }
    napi_value result = nullptr;
    status = napi_new_instance(env, constructor, 1, argv, &result); // 2 arguments
    if (status != napi_ok) {
        LOGE("Get Transliterator create instance failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::IsDigitAddon(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int32_t code = 0;
    std::string character = NAPIUtils::GetString(env, argv[0], code);
    if (code) {
        return nullptr;
    }
    bool isDigit = IsDigit(character);
    napi_value result = nullptr;
    status = napi_get_boolean(env, isDigit, &result);
    if (status != napi_ok) {
        LOGE("Create isDigit boolean value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::IsSpaceCharAddon(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int32_t code = 0;
    std::string character = NAPIUtils::GetString(env, argv[0], code);
    if (code) {
        return nullptr;
    }
    bool isSpaceChar = IsSpaceChar(character);
    napi_value result = nullptr;
    status = napi_get_boolean(env, isSpaceChar, &result);
    if (status != napi_ok) {
        LOGE("Create isSpaceChar boolean value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::IsWhiteSpaceAddon(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int32_t code = 0;
    std::string character = NAPIUtils::GetString(env, argv[0], code);
    if (code) {
        return nullptr;
    }
    bool isWhiteSpace = IsWhiteSpace(character);
    napi_value result = nullptr;
    status = napi_get_boolean(env, isWhiteSpace, &result);
    if (status != napi_ok) {
        LOGE("Create isWhiteSpace boolean value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::IsRTLCharacterAddon(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int32_t code = 0;
    std::string character = NAPIUtils::GetString(env, argv[0], code);
    if (code) {
        return nullptr;
    }
    bool isRTLCharacter = IsRTLCharacter(character);
    napi_value result = nullptr;
    status = napi_get_boolean(env, isRTLCharacter, &result);
    if (status != napi_ok) {
        LOGE("Create isRTLCharacter boolean value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::IsIdeoGraphicAddon(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int32_t code = 0;
    std::string character = NAPIUtils::GetString(env, argv[0], code);
    if (code) {
        return nullptr;
    }
    bool isIdeoGraphic = IsIdeoGraphic(character);
    napi_value result = nullptr;
    status = napi_get_boolean(env, isIdeoGraphic, &result);
    if (status != napi_ok) {
        LOGE("Create isIdeoGraphic boolean value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::IsLetterAddon(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int32_t code = 0;
    std::string character = NAPIUtils::GetString(env, argv[0], code);
    if (code) {
        return nullptr;
    }
    bool isLetter = IsLetter(character);
    napi_value result = nullptr;
    status = napi_get_boolean(env, isLetter, &result);
    if (status != napi_ok) {
        LOGE("Create isLetter boolean value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::IsLowerCaseAddon(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int32_t code = 0;
    std::string character = NAPIUtils::GetString(env, argv[0], code);
    if (code) {
        return nullptr;
    }
    bool isLowerCase = IsLowerCase(character);
    napi_value result = nullptr;
    status = napi_get_boolean(env, isLowerCase, &result);
    if (status != napi_ok) {
        LOGE("Create isLowerCase boolean value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::IsUpperCaseAddon(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int32_t code = 0;
    std::string character = NAPIUtils::GetString(env, argv[0], code);
    if (code) {
        return nullptr;
    }
    bool isUpperCase = IsUpperCase(character);
    napi_value result = nullptr;
    status = napi_get_boolean(env, isUpperCase, &result);
    if (status != napi_ok) {
        LOGE("Create isUpperCase boolean value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetTypeAddon(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int32_t code = 0;
    std::string character = NAPIUtils::GetString(env, argv[0], code);
    if (code) {
        return nullptr;
    }
    std::string type = GetType(character);
    napi_value result = nullptr;
    status = napi_create_string_utf8(env, type.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Create getType string value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetSystemLanguage(napi_env env, napi_callback_info info)
{
    std::string value = LocaleConfig::GetSystemLanguage();
    napi_value result = nullptr;
    napi_status status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Failed to create string item");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetSystemRegion(napi_env env, napi_callback_info info)
{
    std::string value = LocaleConfig::GetSystemRegion();
    napi_value result = nullptr;
    napi_status status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Failed to create string item");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetSystemLocale(napi_env env, napi_callback_info info)
{
    std::string value = LocaleConfig::GetSystemLocale();
    napi_value result = nullptr;
    napi_status status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Failed to create string item");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetDisplayLanguage(napi_env env, napi_callback_info info)
{
    return I18nAddon::GetDisplayLanguageImpl(env, info, false);
}

napi_value I18nAddon::GetDisplayLanguageWithError(napi_env env, napi_callback_info info)
{
    return I18nAddon::GetDisplayLanguageImpl(env, info, true);
}

napi_value I18nAddon::GetDisplayLanguageImpl(napi_env env, napi_callback_info info, bool throwError)
{
    // Need to get three parameters to get the display Language.
    size_t argc = 3;
    napi_value argv[3] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    } else if (argc < FUNC_ARGS_COUNT) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, "language or locale", "", throwError);
        return nullptr;
    }
    NAPIUtils::VerifyType(env, "language", "string", argv[0]);
    size_t len = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    std::vector<char> localeBuf(len + 1);
    if (!GetNapiStringValueWithError(env, argv[0], len, localeBuf.data(), "0")) {
        return nullptr;
    }
    NAPIUtils::VerifyType(env, "locale", "string", argv[1]);
    napi_get_value_string_utf8(env, argv[1], nullptr, 0, &len);
    std::vector<char> displayLocaleBuf(len + 1);
    if (!GetNapiStringValueWithError(env, argv[1], len, displayLocaleBuf.data(), "1")) {
        return nullptr;
    }
    if (!LocaleConfig::IsValidTag(displayLocaleBuf.data())) {
        ErrorUtil::NapiThrow(env, I18N_NOT_VALID, "locale", "a valid locale", throwError);
        return nullptr;
    }
    bool sentenceCase = true;
    if (argc > FUNC_ARGS_COUNT) {
        napi_get_value_bool(env, argv[FUNC_ARGS_COUNT], &sentenceCase);
    }
    std::string value = LocaleConfig::GetDisplayLanguage(localeBuf.data(), displayLocaleBuf.data(), sentenceCase);
    if (value.length() == 0) {
        LOGE("GetDisplayLanguageImpl: result is empty.");
        return nullptr;
    }
    napi_value result = nullptr;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("GetDisplayLanguageImpl: Failed to create string item");
        return nullptr;
    }
    return result;
}

bool I18nAddon::GetNapiStringValueWithError(napi_env env, napi_value napiValue, size_t len, char* valueBuf,
    std::string index)
{
    napi_status status = napi_get_value_string_utf8(env, napiValue, valueBuf, len + 1, &len);
    if (status != napi_ok) {
        LOGE("GetNapiStringValueWithError: Failed to get string item argv[%{public}s]", index.c_str());
        return false;
    }
    return true;
}

napi_value I18nAddon::GetDisplayCountry(napi_env env, napi_callback_info info)
{
    return I18nAddon::GetDisplayCountryImpl(env, info, false);
}

napi_value I18nAddon::GetDisplayCountryWithError(napi_env env, napi_callback_info info)
{
    return I18nAddon::GetDisplayCountryImpl(env, info, true);
}

napi_value I18nAddon::GetDisplayCountryImpl(napi_env env, napi_callback_info info, bool throwError)
{
    // Need to get three parameters to get the display country.
    size_t argc = 3;
    napi_value argv[3] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return NAPIUtils::CreateString(env, "");
    } else if (argc < FUNC_ARGS_COUNT) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, "country or locale", "", throwError);
        return NAPIUtils::CreateString(env, "");
    }
    size_t len = 0;
    NAPIUtils::VerifyType(env, "country", "string", argv[0]);
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    std::vector<char> localeBuf(len + 1);
    if (!GetCountryNapiValueWithError(env, argv[0], len, localeBuf.data(), "0")) {
        return NAPIUtils::CreateString(env, "");
    }
    NAPIUtils::VerifyType(env, "locale", "string", argv[1]);
    napi_get_value_string_utf8(env, argv[1], nullptr, 0, &len);
    std::vector<char> displayLocaleBuf(len + 1);
    if (!GetCountryNapiValueWithError(env, argv[1], len, displayLocaleBuf.data(), "1")) {
        return NAPIUtils::CreateString(env, "");
    }
    LocaleInfo localeInfo(localeBuf.data());
    if (!LocaleConfig::IsValidRegion(localeBuf.data()) && localeInfo.GetRegion() == "") {
        ErrorUtil::NapiThrow(env, I18N_NOT_VALID, "country", "a valid region", throwError);
        return NAPIUtils::CreateString(env, "");
    } else if (!LocaleConfig::IsValidTag(displayLocaleBuf.data())) {
        ErrorUtil::NapiThrow(env, I18N_NOT_VALID, "locale", "a valid locale", throwError);
        return NAPIUtils::CreateString(env, "");
    }
    bool sentenceCase = true;
    if (argc > FUNC_ARGS_COUNT) {
        napi_get_value_bool(env, argv[FUNC_ARGS_COUNT], &sentenceCase);
    }
    std::string value = LocaleConfig::GetDisplayRegion(localeBuf.data(), displayLocaleBuf.data(), sentenceCase);
    if (value.length() == 0) {
        LOGE("GetDisplayCountryImpl: result is empty.");
        return NAPIUtils::CreateString(env, "");
    }
    napi_value result = nullptr;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("GetDisplayCountryImpl: Failed to create string item");
        return NAPIUtils::CreateString(env, "");
    }
    return result;
}

bool I18nAddon::GetCountryNapiValueWithError(napi_env env, napi_value napiValue, size_t len, char* valueBuf,
    std::string index)
{
    napi_status status = napi_get_value_string_utf8(env, napiValue, valueBuf, len + 1, &len);
    if (status != napi_ok) {
        LOGE("GetCountryNapiValueWithError: Failed to get string item argv[%{public}s]", index.c_str());
        return false;
    }
    return true;
}

napi_value I18nAddon::IsRTL(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    size_t len = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    std::vector<char> localeBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], localeBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGE("Failed to get string item");
        return nullptr;
    }
    bool isRTL = LocaleConfig::IsRTL(localeBuf.data());
    napi_value result = nullptr;
    status = napi_get_boolean(env, isRTL, &result);
    if (status != napi_ok) {
        LOGE("IsRTL failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::InitPhoneNumberFormat(napi_env env, napi_value exports)
{
    napi_status status = napi_ok;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("isValidNumber", IsValidPhoneNumber),
        DECLARE_NAPI_FUNCTION("format", FormatPhoneNumber),
    };

    napi_value constructor;
    status = napi_define_class(env, "PhoneNumberFormat", NAPI_AUTO_LENGTH, PhoneNumberFormatConstructor, nullptr,
                               sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        LOGE("Define class failed when InitPhoneNumberFormat");
        return nullptr;
    }

    status = napi_set_named_property(env, exports, "PhoneNumberFormat", constructor);
    if (status != napi_ok) {
        LOGE("Set property failed when InitPhoneNumberFormat");
        return nullptr;
    }
    return exports;
}

void GetOptionValue(napi_env env, napi_value options, const std::string &optionName,
                    std::map<std::string, std::string> &map)
{
    napi_value optionValue = nullptr;
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, options, &type);
    if (status != napi_ok && type != napi_object) {
        LOGE("Get option failed, option is not an object");
        return;
    }
    bool hasProperty = false;
    napi_status propStatus = napi_has_named_property(env, options, optionName.c_str(), &hasProperty);
    if (propStatus == napi_ok && hasProperty) {
        status = napi_get_named_property(env, options, optionName.c_str(), &optionValue);
        if (status == napi_ok) {
            size_t len = 0;
            napi_get_value_string_utf8(env, optionValue, nullptr, 0, &len);
            std::vector<char> optionBuf(len + 1);
            status = napi_get_value_string_utf8(env, optionValue, optionBuf.data(), len + 1, &len);
            if (status != napi_ok) {
                return;
            }
            map.insert(make_pair(optionName, optionBuf.data()));
        }
    }
}

napi_value I18nAddon::PhoneNumberFormatConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    size_t len = 0;
    status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        LOGE("Get country tag length failed");
        return nullptr;
    }
    std::vector<char> country (len + 1);
    status = napi_get_value_string_utf8(env, argv[0], country.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGE("Get country tag failed");
        return nullptr;
    }
    std::map<std::string, std::string> options;
    GetOptionValue(env, argv[1], "type", options);
    std::unique_ptr<I18nAddon> obj = nullptr;
    obj = std::make_unique<I18nAddon>();
    status = napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()),
                       I18nAddon::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("Wrap I18nAddon failed");
        return nullptr;
    }
    if (!obj->InitPhoneNumberFormatContext(env, info, country.data(), options)) {
        return nullptr;
    }
    obj.release();
    return thisVar;
}

bool I18nAddon::InitPhoneNumberFormatContext(napi_env env, napi_callback_info info, const std::string &country,
                                             const std::map<std::string, std::string> &options)
{
    napi_value global = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok) {
        LOGE("Get global failed");
        return false;
    }
    env_ = env;
    phonenumberfmt_ = PhoneNumberFormat::CreateInstance(country, options);

    return phonenumberfmt_ != nullptr;
}

napi_value I18nAddon::IsValidPhoneNumber(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }

    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        LOGE("Get phone number length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGE("Get phone number failed");
        return nullptr;
    }

    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->phonenumberfmt_) {
        LOGE("GetPhoneNumberFormat object failed");
        return nullptr;
    }

    bool isValid = obj->phonenumberfmt_->isValidPhoneNumber(buf.data());

    napi_value result = nullptr;
    status = napi_get_boolean(env, isValid, &result);
    if (status != napi_ok) {
        LOGE("Create boolean failed");
        return nullptr;
    }

    return result;
}

napi_value I18nAddon::FormatPhoneNumber(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }

    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        LOGE("Get phone number length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGE("Get phone number failed");
        return nullptr;
    }

    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->phonenumberfmt_) {
        LOGE("Get PhoneNumberFormat object failed");
        return nullptr;
    }

    std::string formattedPhoneNumber = obj->phonenumberfmt_->format(buf.data());

    napi_value result = nullptr;
    status = napi_create_string_utf8(env, formattedPhoneNumber.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Create format phone number failed");
        return nullptr;
    }
    return result;
}

bool I18nAddon::GetStringArrayFromJsParam(
    napi_env env, napi_value &jsArray, const std::string& valueName, std::vector<std::string> &strArray)
{
    if (jsArray == nullptr) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, valueName, "", true);
        return false;
    }
    bool isArray = false;
    napi_status status = napi_is_array(env, jsArray, &isArray);
    if (status != napi_ok) {
        return false;
    } else if (!isArray) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, valueName, "an Array", true);
        return false;
    }
    uint32_t arrayLength = 0;
    napi_get_array_length(env, jsArray, &arrayLength);
    napi_value element = nullptr;
    int32_t code = 0;
    for (uint32_t i = 0; i < arrayLength; ++i) {
        napi_get_element(env, jsArray, i, &element);
        std::string str = NAPIUtils::GetString(env, element, code);
        if (code != 0) {
            napi_throw_type_error(env, nullptr, "GetStringArrayFromJsParam: Failed to obtain the parameter.");
            return false;
        }
        strArray.push_back(str);
    }
    return true;
}

napi_value I18nAddon::InitI18nCalendar(napi_env env, napi_value exports)
{
    napi_status status = napi_ok;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("setTime", SetTime),
        DECLARE_NAPI_FUNCTION("set", Set),
        DECLARE_NAPI_FUNCTION("getTimeZone", GetTimeZone),
        DECLARE_NAPI_FUNCTION("setTimeZone", SetTimeZone),
        DECLARE_NAPI_FUNCTION("getFirstDayOfWeek", GetFirstDayOfWeek),
        DECLARE_NAPI_FUNCTION("setFirstDayOfWeek", SetFirstDayOfWeek),
        DECLARE_NAPI_FUNCTION("getMinimalDaysInFirstWeek", GetMinimalDaysInFirstWeek),
        DECLARE_NAPI_FUNCTION("setMinimalDaysInFirstWeek", SetMinimalDaysInFirstWeek),
        DECLARE_NAPI_FUNCTION("get", Get),
        DECLARE_NAPI_FUNCTION("add", Add),
        DECLARE_NAPI_FUNCTION("getDisplayName", GetDisplayName),
        DECLARE_NAPI_FUNCTION("getTimeInMillis", GetTimeInMillis),
        DECLARE_NAPI_FUNCTION("isWeekend", IsWeekend),
        DECLARE_NAPI_FUNCTION("compareDays", CompareDays)
    };
    napi_value constructor = nullptr;
    status = napi_define_class(env, "I18nCalendar", NAPI_AUTO_LENGTH, CalendarConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        LOGE("Failed to define class at Init");
        return nullptr;
    }
    g_constructor = new (std::nothrow) napi_ref;
    if (!g_constructor) {
        LOGE("Failed to create ref at init");
        return nullptr;
    }
    status = napi_create_reference(env, constructor, 1, g_constructor);
    if (status != napi_ok) {
        LOGE("Failed to create reference at init");
        return nullptr;
    }
    return exports;
}

napi_value I18nAddon::CalendarConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    argv[0] = nullptr;
    argv[1] = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int32_t code = 0;
    std::string localeTag = NAPIUtils::GetString(env, argv[0], code);
    if (code) {
        return nullptr;
    }
    CalendarType type = GetCalendarType(env, argv[1]);
    std::unique_ptr<I18nAddon> obj = nullptr;
    obj = std::make_unique<I18nAddon>();
    status =
        napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()), I18nAddon::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("CalendarConstructor: Wrap II18nAddon failed");
        return nullptr;
    }
    if (!obj->InitCalendarContext(env, info, localeTag, type)) {
        return nullptr;
    }
    obj.release();
    return thisVar;
}

CalendarType I18nAddon::GetCalendarType(napi_env env, napi_value value)
{
    CalendarType type = CalendarType::UNDEFINED;
    if (value != nullptr) {
        napi_valuetype valueType = napi_valuetype::napi_undefined;
        napi_typeof(env, value, &valueType);
        if (valueType != napi_valuetype::napi_string) {
            napi_throw_type_error(env, nullptr, "Parameter type does not match");
            return type;
        }
        int32_t code = 0;
        std::string calendarType = NAPIUtils::GetString(env, value, code);
        if (code) {
            return type;
        }
        if (g_typeMap.find(calendarType) != g_typeMap.end()) {
            type = g_typeMap[calendarType];
        }
    }
    return type;
}

bool I18nAddon::InitCalendarContext(napi_env env, napi_callback_info info, const std::string &localeTag,
    CalendarType type)
{
    calendar_ = std::make_unique<I18nCalendar>(localeTag, type);
    return calendar_ != nullptr;
}

napi_value I18nAddon::GetCalendar(napi_env env, napi_callback_info info)
{
    size_t argc = 2; // retrieve 2 arguments
    napi_value argv[2] = { 0 };
    argv[0] = nullptr;
    argv[1] = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_value constructor = nullptr;
    napi_status status = napi_get_reference_value(env, *g_constructor, &constructor);
    if (status != napi_ok) {
        LOGE("Failed to create reference at GetCalendar");
        return nullptr;
    }
    if (argc < FUNC_ARGS_COUNT) {
        status = napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, argv + 1);
        if (status != napi_ok) {
            return nullptr;
        }
    }
    napi_value result = nullptr;
    status = napi_new_instance(env, constructor, 2, argv, &result); // 2 arguments
    if (status != napi_ok) {
        LOGE("Get calendar create instance failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetDate(napi_env env, napi_value value)
{
    if (!value) {
        return nullptr;
    }
    napi_value funcGetDateInfo = nullptr;
    napi_status status = napi_get_named_property(env, value, "valueOf", &funcGetDateInfo);
    if (status != napi_ok) {
        LOGE("Get method valueOf failed");
        return nullptr;
    }
    napi_value ret_value = nullptr;
    status = napi_call_function(env, value, funcGetDateInfo, 0, nullptr, &ret_value);
    if (status != napi_ok) {
        LOGE("Get milliseconds failed");
        return nullptr;
    }
    return ret_value;
}

void I18nAddon::SetMilliseconds(napi_env env, napi_value value)
{
    if (!value) {
        return;
    }
    double milliseconds = 0;
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return;
    }
    napi_status status = napi_get_value_double(env, value, &milliseconds);
    if (status != napi_ok) {
        LOGE("Retrieve milliseconds failed");
        return;
    }
    if (calendar_ != nullptr) {
        calendar_->SetTime(milliseconds);
    }
}

napi_value I18nAddon::Set(napi_env env, napi_callback_info info)
{
    size_t argc = 6; // Set may have 6 arguments
    napi_value argv[6] = { 0 };
    for (size_t i = 0; i < argc; ++i) {
        argv[i] = nullptr;
    }
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_status status = napi_ok;
    int32_t times[3] = { 0 }; // There are at least 3 arguments.
    for (int i = 0; i < 3; ++i) { // There are at least 3 arguments.
        napi_typeof(env, argv[i], &valueType);
        if (valueType != napi_valuetype::napi_number) {
            napi_throw_type_error(env, nullptr, "Parameter type does not match");
            return nullptr;
        }
        status = napi_get_value_int32(env, argv[i], times + i);
        if (status != napi_ok) {
            LOGE("Retrieve time value failed");
            return nullptr;
        }
    }
    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->calendar_) {
        LOGE("Get calendar object failed");
        return nullptr;
    }
    obj->calendar_->Set(times[0], times[1], times[2]); // 2 is the index of date
    obj->SetField(env, argv[3], UCalendarDateFields::UCAL_HOUR_OF_DAY); // 3 is the index of hour
    obj->SetField(env, argv[4], UCalendarDateFields::UCAL_MINUTE); // 4 is the index of minute
    obj->SetField(env, argv[5], UCalendarDateFields::UCAL_SECOND); // 5 is the index of second
    return nullptr;
}

napi_value I18nAddon::SetTime(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    argv[0] = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (argc == 0) {
        return nullptr;
    }
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->calendar_) {
        LOGE("Get calendar object failed");
        return nullptr;
    }
    napi_valuetype type = napi_valuetype::napi_undefined;
    status = napi_typeof(env, argv[0], &type);
    if (status != napi_ok) {
        return nullptr;
    }
    if (type == napi_valuetype::napi_number) {
        obj->SetMilliseconds(env, argv[0]);
        return nullptr;
    } else {
        napi_value val = GetDate(env, argv[0]);
        if (!val) {
            return nullptr;
        }
        obj->SetMilliseconds(env, val);
        return nullptr;
    }
}

void I18nAddon::SetField(napi_env env, napi_value value, UCalendarDateFields field)
{
    if (!value) {
        return;
    }
    int32_t val = 0;
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType == napi_valuetype::napi_undefined) {
        return;
    }
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return;
    }
    napi_status status = napi_get_value_int32(env, value, &val);
    if (status != napi_ok) {
        LOGE("Retrieve field failed");
        return;
    }
    if (calendar_ != nullptr) {
        calendar_->Set(field, val);
    }
}

napi_value I18nAddon::SetTimeZone(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    argv[0] = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        LOGE("Get timezone length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGE("Get timezone failed");
        return nullptr;
    }
    std::string timezone(buf.data());
    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->calendar_) {
        LOGE("Get calendar object failed");
        return nullptr;
    }
    obj->calendar_->SetTimeZone(timezone);
    return nullptr;
}

napi_value I18nAddon::GetTimeZone(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value *argv = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->calendar_) {
        LOGE("Get calendar object failed");
        return nullptr;
    }
    std::string temp = obj->calendar_->GetTimeZone();
    napi_value result = nullptr;
    status = napi_create_string_utf8(env, temp.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Create timezone string failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetFirstDayOfWeek(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value *argv = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->calendar_) {
        LOGE("Get calendar object failed");
        return nullptr;
    }
    int32_t temp = obj->calendar_->GetFirstDayOfWeek();
    napi_value result = nullptr;
    status = napi_create_int32(env, temp, &result);
    if (status != napi_ok) {
        LOGE("Create int32 failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetMinimalDaysInFirstWeek(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value *argv = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->calendar_) {
        LOGE("Get calendar object failed");
        return nullptr;
    }
    int32_t temp = obj->calendar_->GetMinimalDaysInFirstWeek();
    napi_value result = nullptr;
    status = napi_create_int32(env, temp, &result);
    if (status != napi_ok) {
        LOGE("Create int32 failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::SetFirstDayOfWeek(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    argv[0] = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int32_t value = 0;
    napi_status status = napi_get_value_int32(env, argv[0], &value);
    if (status != napi_ok) {
        LOGE("Get int32 failed");
        return nullptr;
    }
    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->calendar_) {
        LOGE("Get calendar object failed");
        return nullptr;
    }
    obj->calendar_->SetFirstDayOfWeek(value);
    return nullptr;
}

napi_value I18nAddon::SetMinimalDaysInFirstWeek(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    argv[0] = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int32_t value = 0;
    napi_status status = napi_get_value_int32(env, argv[0], &value);
    if (status != napi_ok) {
        LOGE("Get int32 failed");
        return nullptr;
    }
    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->calendar_) {
        LOGE("Get calendar object failed");
        return nullptr;
    }
    obj->calendar_->SetMinimalDaysInFirstWeek(value);
    return nullptr;
}

napi_value I18nAddon::Get(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    argv[0] = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        LOGE("Get field length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGE("Get field failed");
        return nullptr;
    }
    std::string field(buf.data());
    if (g_fieldsMap.find(field) == g_fieldsMap.end()) {
        LOGE("Invalid field");
        return nullptr;
    }
    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->calendar_) {
        LOGE("Get calendar object failed");
        return nullptr;
    }
    int32_t value = obj->calendar_->Get(g_fieldsMap[field]);
    napi_value result = nullptr;
    status = napi_create_int32(env, value, &result);
    if (status != napi_ok) {
        LOGE("Create int32 failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::Add(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        LOGE("can not obtain add function param.");
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, true);
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        LOGE("Parameter type does not match");
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, true);
        return nullptr;
    }
    int32_t code = 0;
    std::string field = GetAddField(env, argv[0], code);
    if (code) {
        return nullptr;
    }
    napi_typeof(env, argv[1], &valueType);
    if (valueType != napi_valuetype::napi_number) {
        LOGE("Parameter type does not match");
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, true);
        return nullptr;
    }
    int32_t amount;
    status = napi_get_value_int32(env, argv[1], &amount);
    if (status != napi_ok) {
        LOGE("Can not obtain add function param.");
        ErrorUtil::NapiThrow(env, I18N_NOT_VALID, true);
        return nullptr;
    }
    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->calendar_) {
        LOGE("Get calendar object failed");
        return nullptr;
    }
    obj->calendar_->Add(g_fieldsMap[field], amount);
    return nullptr;
}

napi_value I18nAddon::GetDisplayName(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    argv[0] = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int32_t code = 0;
    std::string localeTag = NAPIUtils::GetString(env, argv[0], code);
    if (code) {
        return nullptr;
    }
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->calendar_) {
        LOGE("Get calendar object failed");
        return nullptr;
    }
    if (!obj->calendar_) {
        return nullptr;
    }
    std::string name = obj->calendar_->GetDisplayName(localeTag);
    napi_value result = nullptr;
    status = napi_create_string_utf8(env, name.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Create calendar name string failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetTimeInMillis(napi_env env, napi_callback_info info)
{
    bool flag = true;
    size_t argc = 0;
    napi_value *argv = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->calendar_) {
        LOGE("Get calendar object failed");
        flag = false;
    }
    UDate temp = 0;
    if (flag) {
        temp = obj->calendar_->GetTimeInMillis();
    }
    napi_value result = nullptr;
    status = napi_create_double(env, temp, &result);
    if (status != napi_ok) {
        LOGE("Create UDate failed");
        napi_create_double(env, 0, &result);
    }
    return result;
}

napi_value I18nAddon::IsWeekend(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    argv[0] = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    bool isWeekEnd = false;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    do {
        if (status != napi_ok || !obj || !obj->calendar_) {
            LOGE("Get calendar object failed");
            break;
        }
        if (argc == 0) {
            isWeekEnd = obj->calendar_->IsWeekend();
        } else {
            napi_value funcGetDateInfo = nullptr;
            status = napi_get_named_property(env, argv[0], "valueOf", &funcGetDateInfo);
            if (status != napi_ok) {
                LOGE("Get method now failed");
                break;
            }
            napi_value value = nullptr;
            status = napi_call_function(env, argv[0], funcGetDateInfo, 0, nullptr, &value);
            if (status != napi_ok) {
                LOGE("Get milliseconds failed");
                break;
            }
            double milliseconds = 0;
            status = napi_get_value_double(env, value, &milliseconds);
            if (status != napi_ok) {
                LOGE("Retrieve milliseconds failed");
                break;
            }
            UErrorCode error = U_ZERO_ERROR;
            isWeekEnd = obj->calendar_->IsWeekend(milliseconds, error);
        }
    } while (false);
    napi_value result = nullptr;
    status = napi_get_boolean(env, isWeekEnd, &result);
    if (status != napi_ok) {
        LOGE("Create boolean failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::CompareDays(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_value result = nullptr;
    UDate milliseconds = 0;
    napi_status status = napi_get_date_value(env, argv[0], &milliseconds);
    if (status != napi_ok) {
        LOGE("compareDays function param is not Date");
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, true);
        return nullptr;
    }

    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->calendar_) {
        LOGE("Get calendar object failed");
        status = napi_create_int32(env, 0, &result); // if error return 0
        return result;
    }

    int32_t diff_date = obj->calendar_->CompareDays(milliseconds);
    status = napi_create_int32(env, diff_date, &result);
    return result;
}

std::string I18nAddon::GetAddField(napi_env &env, napi_value &value, int32_t &code)
{
    std::string field = NAPIUtils::GetString(env, value, code);
    if (code != 0) {
        LOGE("can't get string from js array param.");
        ErrorUtil::NapiThrow(env, I18N_NOT_VALID, true);
        return field;
    }
    if (g_fieldsInFunctionAdd.find(field) == g_fieldsInFunctionAdd.end()) {
        code = 1;
        LOGE("Parameter rangs do not match");
        ErrorUtil::NapiThrow(env, I18N_NOT_VALID, true);
        return field;
    }
    return field;
}


napi_value I18nAddon::InitIndexUtil(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getIndexList", GetIndexList),
        DECLARE_NAPI_FUNCTION("addLocale", AddLocale),
        DECLARE_NAPI_FUNCTION("getIndex", GetIndex)
    };

    napi_value constructor = nullptr;
    napi_status status = napi_define_class(env, "IndexUtil", NAPI_AUTO_LENGTH, IndexUtilConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        LOGE("Define class failed when InitIndexUtil");
        return nullptr;
    }

    status = napi_create_reference(env, constructor, 1, &g_indexUtilConstructor);
    if (status != napi_ok) {
        LOGE("Failed to create reference at init");
        return nullptr;
    }
    return exports;
}

napi_value I18nAddon::BreakIteratorConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int32_t code = 0;
    std::string localeTag = NAPIUtils::GetString(env, argv[0], code);
    if (code) {
        return nullptr;
    }
    std::unique_ptr<I18nAddon> obj = nullptr;
    obj = std::make_unique<I18nAddon>();
    status =
        napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()), I18nAddon::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("BreakIteratorConstructor: Wrap II18nAddon failed");
        return nullptr;
    }
    obj->brkiter_ = std::make_unique<I18nBreakIterator>(localeTag);
    if (!obj->brkiter_) {
        LOGE("Wrap BreakIterator failed");
        return nullptr;
    }
    obj.release();
    return thisVar;
}

napi_value I18nAddon::InitBreakIterator(napi_env env, napi_value exports)
{
    napi_status status = napi_ok;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("current", Current),
        DECLARE_NAPI_FUNCTION("first", First),
        DECLARE_NAPI_FUNCTION("last", Last),
        DECLARE_NAPI_FUNCTION("next", Next),
        DECLARE_NAPI_FUNCTION("previous", Previous),
        DECLARE_NAPI_FUNCTION("setLineBreakText", SetText),
        DECLARE_NAPI_FUNCTION("following", Following),
        DECLARE_NAPI_FUNCTION("getLineBreakText", GetText),
        DECLARE_NAPI_FUNCTION("isBoundary", IsBoundary),
    };
    napi_value constructor = nullptr;
    status = napi_define_class(env, "BreakIterator", NAPI_AUTO_LENGTH, BreakIteratorConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        LOGE("Failed to define class BreakIterator at Init");
        return nullptr;
    }
    g_brkConstructor = new (std::nothrow) napi_ref;
    if (!g_brkConstructor) {
        LOGE("Failed to create brkiterator ref at init");
        return nullptr;
    }
    status = napi_create_reference(env, constructor, 1, g_brkConstructor);
    if (status != napi_ok) {
        LOGE("Failed to create reference g_brkConstructor at init");
        return nullptr;
    }
    return exports;
}

napi_value I18nAddon::GetLineInstance(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_value constructor = nullptr;
    napi_status status = napi_get_reference_value(env, *g_brkConstructor, &constructor);
    if (status != napi_ok) {
        LOGE("Failed to create reference at GetLineInstance");
        return nullptr;
    }
    if (argc == 0) {
        return nullptr;
    }
    napi_value result = nullptr;
    status = napi_new_instance(env, constructor, 1, argv, &result); // 1 arguments
    if (status != napi_ok) {
        LOGE("GetLineInstance create instance failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::Current(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value *argv = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->brkiter_) {
        LOGE("Get BreakIterator object failed");
        return nullptr;
    }
    int value = obj->brkiter_->Current();
    napi_value result = nullptr;
    status = napi_create_int32(env, value, &result);
    if (status != napi_ok) {
        LOGE("Create int32_t value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::First(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value *argv = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->brkiter_) {
        LOGE("Get BreakIterator object failed");
        return nullptr;
    }
    int value = obj->brkiter_->First();
    napi_value result = nullptr;
    status = napi_create_int32(env, value, &result);
    if (status != napi_ok) {
        LOGE("Create int32_t value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::Last(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value *argv = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->brkiter_) {
        LOGE("Get BreakIterator object failed");
        return nullptr;
    }
    int value = obj->brkiter_->Last();
    napi_value result = nullptr;
    status = napi_create_int32(env, value, &result);
    if (status != napi_ok) {
        LOGE("Create int32_t value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::Previous(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value *argv = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->brkiter_) {
        LOGE("Get BreakIterator object failed");
        return nullptr;
    }
    int value = obj->brkiter_->Previous();
    napi_value result = nullptr;
    status = napi_create_int32(env, value, &result);
    if (status != napi_ok) {
        LOGE("Create int32_t value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::Next(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->brkiter_) {
        LOGE("Get BreakIterator object failed");
        return nullptr;
    }
    int value = 1;
    if (argc > 0) {
        napi_valuetype valueType = napi_valuetype::napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        if (valueType != napi_valuetype::napi_number) {
            napi_throw_type_error(env, nullptr, "Parameter type does not match");
            return nullptr;
        }
        status = napi_get_value_int32(env, argv[0], &value);
        if (status != napi_ok) {
            LOGE("Retrieve next value failed");
            return nullptr;
        }
    }
    value = obj->brkiter_->Next(value);
    napi_value result = nullptr;
    status = napi_create_int32(env, value, &result);
    if (status != napi_ok) {
        LOGE("Create int32_t value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::SetText(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->brkiter_) {
        LOGE("Get BreakIterator object failed");
        return nullptr;
    }
    if (argc == 0) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    size_t len = 0;
    status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        LOGE("Get field length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGE("Get string value failed");
        return nullptr;
    }
    obj->brkiter_->SetText(buf.data());
    return nullptr;
}

napi_value I18nAddon::GetText(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value *argv = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->brkiter_) {
        LOGE("Get BreakIterator object failed");
        return nullptr;
    }
    napi_value value = nullptr;
    std::string temp;
    obj->brkiter_->GetText(temp);
    status = napi_create_string_utf8(env, temp.c_str(), NAPI_AUTO_LENGTH, &value);
    if (status != napi_ok) {
        LOGE("Get field length failed");
        return nullptr;
    }
    return value;
}

napi_value I18nAddon::Following(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->brkiter_) {
        LOGE("Get BreakIterator object failed");
        return nullptr;
    }
    if (argc == 0) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    int value;
    status = napi_get_value_int32(env, argv[0], &value);
    if (status != napi_ok) {
        LOGE("Retrieve following value failed");
        return nullptr;
    }
    value = obj->brkiter_->Following(value);
    napi_value result = nullptr;
    status = napi_create_int32(env, value, &result);
    if (status != napi_ok) {
        LOGE("Create int32_t value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::IsBoundary(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->brkiter_) {
        LOGE("Get BreakIterator object failed");
        return nullptr;
    }
    if (argc == 0) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    int value;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    status = napi_get_value_int32(env, argv[0], &value);
    if (status != napi_ok) {
        LOGE("Retrieve following value failed");
        return nullptr;
    }
    bool boundary = obj->brkiter_->IsBoundary(value);
    napi_value result = nullptr;
    status = napi_get_boolean(env, boundary, &result);
    if (status != napi_ok) {
        LOGE("Create boolean failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::IndexUtilConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    std::string localeTag = "";
    if (argc > 0) {
        napi_valuetype valueType = napi_valuetype::napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        if (valueType != napi_valuetype::napi_string) {
            napi_throw_type_error(env, nullptr, "Parameter type does not match");
            return nullptr;
        }
        size_t len = 0;
        status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
        if (status != napi_ok) {
            LOGE("Get locale length failed");
            return nullptr;
        }
        std::vector<char> localeBuf(len + 1);
        status = napi_get_value_string_utf8(env, argv[0], localeBuf.data(), len + 1, &len);
        if (status != napi_ok) {
            LOGE("Get locale failed");
            return nullptr;
        }
        localeTag = localeBuf.data();
    }
    std::unique_ptr<I18nAddon> obj = nullptr;
    obj = std::make_unique<I18nAddon>();
    status =
        napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()), I18nAddon::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("IndexUtilConstructor: Wrap II18nAddon failed");
        return nullptr;
    }
    if (!obj->InitIndexUtilContext(env, info, localeTag)) {
        return nullptr;
    }
    obj.release();
    return thisVar;
}

bool I18nAddon::InitIndexUtilContext(napi_env env, napi_callback_info info, const std::string &localeTag)
{
    napi_value global = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok) {
        LOGE("Get global failed");
        return false;
    }
    env_ = env;
    indexUtil_ = std::make_unique<IndexUtil>(localeTag);
    return indexUtil_ != nullptr;
}

napi_value I18nAddon::GetIndexUtil(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_value constructor = nullptr;
    napi_status status = napi_get_reference_value(env, g_indexUtilConstructor, &constructor);
    if (status != napi_ok) {
        LOGE("Failed to create reference at GetIndexUtil");
        return nullptr;
    }
    napi_value result = nullptr;
    if (argc == 0) {
        status = napi_new_instance(env, constructor, 0, argv, &result);
    } else {
        status = napi_new_instance(env, constructor, 1, argv, &result);
    }
    if (status != napi_ok) {
        LOGE("Get calendar create instance failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetIndexList(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv[0];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->indexUtil_) {
        LOGE("GetPhoneNumberFormat object failed");
        return nullptr;
    }

    std::vector<std::string> indexList = obj->indexUtil_->GetIndexList();
    napi_value result = nullptr;
    status = napi_create_array_with_length(env, indexList.size(), &result);
    if (status != napi_ok) {
        LOGE("Failed to create array");
        return nullptr;
    }
    for (size_t i = 0; i < indexList.size(); i++) {
        napi_value element = nullptr;
        status = napi_create_string_utf8(env, indexList[i].c_str(), NAPI_AUTO_LENGTH, &element);
        if (status != napi_ok) {
            LOGE("Failed to create string item");
            return nullptr;
        }
        status = napi_set_element(env, result, i, element);
        if (status != napi_ok) {
            LOGE("Failed to set array item");
            return nullptr;
        }
    }
    return result;
}

napi_value I18nAddon::AddLocale(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        LOGE("Get locale length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGE("Get locale failed");
        return nullptr;
    }
    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->indexUtil_) {
        LOGE("Get IndexUtil object failed");
        return nullptr;
    }
    obj->indexUtil_->AddLocale(buf.data());
    return nullptr;
}

napi_value I18nAddon::GetIndex(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        LOGE("Get String length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGE("Get String failed");
        return nullptr;
    }
    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->indexUtil_) {
        LOGE("Get IndexUtil object failed");
        return nullptr;
    }
    std::string index = obj->indexUtil_->GetIndex(buf.data());
    napi_value result = nullptr;
    status = napi_create_string_utf8(env, index.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("GetIndex Failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::Is24HourClock(napi_env env, napi_callback_info info)
{
    bool is24HourClock = LocaleConfig::Is24HourClock();
    napi_value result = nullptr;
    napi_status status = napi_get_boolean(env, is24HourClock, &result);
    if (status != napi_ok) {
        LOGE("Failed to create boolean item");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::SetAppPreferredLanguage(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    status = napi_typeof(env, argv[0], &valueType);
    if (status != napi_ok) {
        return nullptr;
    }
    if (valueType != napi_valuetype::napi_string) {
        LOGE("SetAppPreferredLanguage Parameter type is not string");
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, "language", "string", true);
        return nullptr;
    }

    size_t len = 0;
    status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        LOGE("SetAppPreferredLanguage: Get string failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGE("SetAppPreferredLanguage: Create string failed");
        return nullptr;
    }
    std::string localeTag(buf.data());

    UErrorCode icuStatus = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(localeTag.data(), icuStatus);
    if (U_FAILURE(icuStatus) || !(IsValidLocaleTag(locale) || localeTag.compare("default") == 0)) {
        LOGE("SetAppPreferredLanguage does not support this locale");
        ErrorUtil::NapiThrow(env, I18N_NOT_VALID, "language", "a valid language", true);
        return nullptr;
    }
    LocaleConfig::SetAppPreferredLanguage(localeTag);
    return nullptr;
}

napi_value I18nAddon::GetAppPreferredLanguage(napi_env env, napi_callback_info info)
{
    std::string language = LocaleConfig::GetAppPreferredLanguage();
    napi_value result = nullptr;
    napi_status status = napi_create_string_utf8(env, language.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("getAppPreferredLanguage: create string result failed");
        return nullptr;
    }
    return result;
}

bool I18nAddon::ParseStringParam(napi_env env, napi_value argv, bool throwError, std::string &strParam)
{
    if (argv == nullptr) {
        LOGE("Missing parameter");
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, throwError);
        return false;
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv, &valueType);
    if (valueType != napi_valuetype::napi_string) {
        ErrorUtil::NapiThrow(env, I18N_NOT_VALID, throwError);
        return false;
    }
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv, nullptr, 0, &len);
    if (status != napi_ok) {
        LOGE("get string parameter length failed");
        ErrorUtil::NapiThrow(env, I18N_NOT_VALID, throwError);
        return false;
    }
    std::vector<char> res(len + 1);
    status = napi_get_value_string_utf8(env, argv, res.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGE("get string parameter failed");
        return false;
    }
    strParam = res.data();
    return true;
}

napi_value I18nAddon::InitI18nTimeZone(napi_env env, napi_value exports)
{
    napi_status status = napi_ok;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getID", GetID),
        DECLARE_NAPI_FUNCTION("getDisplayName", GetTimeZoneDisplayName),
        DECLARE_NAPI_FUNCTION("getRawOffset", GetRawOffset),
        DECLARE_NAPI_FUNCTION("getOffset", GetOffset),
    };
    napi_value constructor = nullptr;
    status = napi_define_class(env, "TimeZone", NAPI_AUTO_LENGTH, I18nTimeZoneConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        LOGE("Failed to define class TimeZone at Init");
        return nullptr;
    }
    g_timezoneConstructor = new (std::nothrow) napi_ref;
    if (!g_timezoneConstructor) {
        LOGE("Failed to create TimeZone ref at init");
        return nullptr;
    }
    status = napi_create_reference(env, constructor, 1, g_timezoneConstructor);
    if (status != napi_ok) {
        LOGE("Failed to create reference g_timezoneConstructor at init");
        return nullptr;
    }
    return exports;
}

napi_value I18nAddon::I18nTimeZoneConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    std::string zoneID = "";
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    if (argc > 0) {
        napi_typeof(env, argv[0], &valueType);
        if (valueType != napi_valuetype::napi_string) {
            return nullptr;
        }
        int32_t code = 0;
        zoneID = NAPIUtils::GetString(env, argv[0], code);
        if (code != 0) {
            return nullptr;
        }
    }
    if (argc < FUNC_ARGS_COUNT) {
        return nullptr;
    }
    napi_typeof(env, argv[1], &valueType);
    if (valueType != napi_valuetype::napi_boolean) {
        return nullptr;
    }
    bool isZoneID = false;
    status = napi_get_value_bool(env, argv[1], &isZoneID);
    if (status != napi_ok) {
        return nullptr;
    }
    std::unique_ptr<I18nAddon> obj = std::make_unique<I18nAddon>();
    status =
        napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()), I18nAddon::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        return nullptr;
    }
    obj->timezone_ = I18nTimeZone::CreateInstance(zoneID, isZoneID);
    if (!obj->timezone_) {
        return nullptr;
    }
    obj.release();
    return thisVar;
}

napi_value I18nAddon::GetI18nTimeZone(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (argc == 0) {
        napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &argv[0]);
    }
    return StaticGetTimeZone(env, argv, true);
}

napi_value I18nAddon::GetID(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value *argv = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->timezone_) {
        LOGE("Get TimeZone object failed");
        return nullptr;
    }
    std::string result = obj->timezone_->GetID();
    napi_value value = nullptr;
    status = napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &value);
    if (status != napi_ok) {
        LOGE("Create result failed");
        return nullptr;
    }
    return value;
}

bool I18nAddon::GetStringFromJS(napi_env env, napi_value argv, std::string &jsString)
{
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv, nullptr, 0, &len);
    if (status != napi_ok) {
        LOGE("Failed to get string length");
        return false;
    }
    std::vector<char> argvBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv, argvBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGE("Failed to get string item");
        return false;
    }
    jsString = argvBuf.data();
    return true;
}

int32_t I18nAddon::GetParameter(napi_env env, napi_value *argv, std::string &localeStr, bool &isDST)
{
    if (!argv[0]) {
        return 0;  // 0 represents no parameter.
    }
    napi_status status = napi_ok;
    if (!argv[1]) {
        napi_valuetype valueType = napi_valuetype::napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        if (valueType == napi_valuetype::napi_string) {
            bool valid = GetStringFromJS(env, argv[0], localeStr);
            if (!valid) {
                return -1;  // -1 represents Invalid parameter.
            }
            return 1;  // 1 represents one string parameter.
        } else if (valueType == napi_valuetype::napi_boolean) {
            status = napi_get_value_bool(env, argv[0], &isDST);
            if (status != napi_ok) {
                return -1;  // -1 represents Invalid parameter.
            }
            return 2;  // 2 represents one boolean parameter.
        } else {
            return -1;  // -1 represents Invalid parameter.
        }
    }
    napi_valuetype valueType0 = napi_valuetype::napi_undefined;
    napi_valuetype valueType1 = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType0);  // 0 represents first parameter
    napi_typeof(env, argv[1], &valueType1);  // 0 represents second parameter
    if (valueType0 != napi_valuetype::napi_string || valueType1 != napi_valuetype::napi_boolean) {
        return -1;  // -1 represents Invalid parameter.
    }
    bool valid = GetStringFromJS(env, argv[0], localeStr);
    if (!valid) {
        return -1;  // -1 represents Invalid parameter.
    }
    status = napi_get_value_bool(env, argv[1], &isDST);
    if (status != napi_ok) {
        return -1;  // -1 represents Invalid parameter.
    }
    return 3;  // 3 represents one string parameter and one bool parameter.
}

napi_value I18nAddon::GetTimeZoneDisplayName(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }

    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->timezone_) {
        LOGE("Get TimeZone object failed");
        return nullptr;
    }

    std::string locale;
    bool isDST = false;
    int32_t parameterStatus = GetParameter(env, argv, locale, isDST);

    std::string result;
    if (parameterStatus == -1) {  // -1 represents Invalid parameter.
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    } else if (parameterStatus == 0) {
        result = obj->timezone_->GetDisplayName();
    } else if (parameterStatus == 1) {  // 1 represents one string parameter.
        result = obj->timezone_->GetDisplayName(locale);
    } else if (parameterStatus == 2) {  // 2 represents one boolean parameter.
        result = obj->timezone_->GetDisplayName(isDST);
    } else {
        result = obj->timezone_->GetDisplayName(locale, isDST);
    }

    napi_value value = nullptr;
    status = napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &value);
    if (status != napi_ok) {
        LOGE("Create result failed");
        return nullptr;
    }
    return value;
}

napi_value I18nAddon::GetOffset(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }

    double date = 0;
    if (argc > 0) {
        napi_valuetype valueType = napi_valuetype::napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        if (valueType != napi_valuetype::napi_number) {
            LOGE("Invalid parameter type");
            return nullptr;
        }
        status = napi_get_value_double(env, argv[0], &date);
        if (status != napi_ok) {
            LOGE("Get parameter date failed");
            return nullptr;
        }
    } else {
        auto time = std::chrono::system_clock::now();
        auto since_epoch = time.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch);
        date = (double)millis.count();
    }

    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->timezone_) {
        LOGE("Get TimeZone object failed");
        return nullptr;
    }
    int32_t result = obj->timezone_->GetOffset(date);
    napi_value value = nullptr;
    status = napi_create_int32(env, result, &value);
    if (status != napi_ok) {
        LOGE("Create result failed");
        return nullptr;
    }
    return value;
}

napi_value I18nAddon::GetRawOffset(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value *argv = nullptr;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    I18nAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || !obj || !obj->timezone_) {
        LOGE("Get TimeZone object failed");
        return nullptr;
    }
    int32_t result = obj->timezone_->GetRawOffset();
    napi_value value = nullptr;
    status = napi_create_int32(env, result, &value);
    if (status != napi_ok) {
        LOGE("Create result failed");
        return nullptr;
    }
    return value;
}

napi_value I18nAddon::CreateTimeZoneObject(napi_env env, napi_status &initStatus)
{
    napi_status status = napi_ok;
    napi_value timezone = nullptr;
    status = napi_create_object(env, &timezone);
    if (status != napi_ok) {
        LOGE("Failed to create timezone object at init");
        initStatus = napi_generic_failure;
        return nullptr;
    }
    napi_property_descriptor timezoneProperties[] = {
        DECLARE_NAPI_FUNCTION("getAvailableIDs", GetAvailableTimezoneIDs),
    };
    status = napi_define_properties(env, timezone,
                                    sizeof(timezoneProperties) / sizeof(napi_property_descriptor),
                                    timezoneProperties);
    if (status != napi_ok) {
        LOGE("Failed to set properties of timezone at init");
        initStatus = napi_generic_failure;
        return nullptr;
    }
    return timezone;
}

napi_value I18nAddon::CreateSystemObject(napi_env env, napi_status &initStatus)
{
    napi_status status = napi_ok;
    napi_value system = nullptr;
    status = napi_create_object(env, &system);
    if (status != napi_ok) {
        LOGE("Failed to create system object at init");
        initStatus = napi_generic_failure;
        return nullptr;
    }
    napi_property_descriptor systemProperties[] = {
        DECLARE_NAPI_FUNCTION("getDisplayCountry", GetDisplayCountryWithError),
        DECLARE_NAPI_FUNCTION("getDisplayLanguage", GetDisplayLanguageWithError),
        DECLARE_NAPI_FUNCTION("getSystemLanguage", GetSystemLanguage),
        DECLARE_NAPI_FUNCTION("getSystemRegion", GetSystemRegion),
        DECLARE_NAPI_FUNCTION("getSystemLocale", GetSystemLocale),
        DECLARE_NAPI_FUNCTION("is24HourClock", Is24HourClock),
        DECLARE_NAPI_STATIC_FUNCTION("setAppPreferredLanguage", SetAppPreferredLanguage),
        DECLARE_NAPI_STATIC_FUNCTION("getAppPreferredLanguage", GetAppPreferredLanguage),
    };
    status = napi_define_properties(env, system,
                                    sizeof(systemProperties) / sizeof(napi_property_descriptor),
                                    systemProperties);
    if (status != napi_ok) {
        LOGE("Failed to set properties of system at init");
        initStatus = napi_generic_failure;
        return nullptr;
    }
    return system;
}

napi_value I18nAddon::GetAvailableTimezoneIDs(napi_env env, napi_callback_info info)
{
    I18nErrorCode errorCode = I18nErrorCode::SUCCESS;
    std::set<std::string> timezoneIDs = I18nTimeZone::GetAvailableIDs(errorCode);
    if (errorCode != I18nErrorCode::SUCCESS) {
        return nullptr;
    }
    napi_value result = nullptr;
    napi_status status = napi_create_array_with_length(env, timezoneIDs.size(), &result);
    if (status != napi_ok) {
        LOGE("Failed to create array");
        return nullptr;
    }
    size_t index = 0;
    for (std::set<std::string>::iterator it = timezoneIDs.begin(); it != timezoneIDs.end(); ++it) {
        napi_value value = nullptr;
        status = napi_create_string_utf8(env, (*it).c_str(), NAPI_AUTO_LENGTH, &value);
        if (status != napi_ok) {
            LOGE("Failed to create string item");
            return nullptr;
        }
        status = napi_set_element(env, result, index, value);
        if (status != napi_ok) {
            LOGE("Failed to set array item");
            return nullptr;
        }
        ++index;
    }
    return result;
}

napi_value I18nAddon::StaticGetTimeZone(napi_env env, napi_value *argv, bool isZoneID)
{
    napi_value constructor = nullptr;
    napi_status status = napi_get_reference_value(env, *g_timezoneConstructor, &constructor);
    if (status != napi_ok) {
        LOGE("Failed to create reference at StaticGetTimeZone");
        return nullptr;
    }
    napi_value newArgv[2] = { 0 };
    newArgv[0] = argv[0];
    status = napi_get_boolean(env, isZoneID, &newArgv[1]);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_value result = nullptr;
    status = napi_new_instance(env, constructor, 2, newArgv, &result); // 2 is parameter num
    if (status != napi_ok) {
        LOGE("StaticGetTimeZone create instance failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::InitCharacter(napi_env env, napi_value exports)
{
    napi_status status = napi_ok;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("isDigit", IsDigitAddon),
        DECLARE_NAPI_FUNCTION("isSpaceChar", IsSpaceCharAddon),
        DECLARE_NAPI_FUNCTION("isWhitespace", IsWhiteSpaceAddon),
        DECLARE_NAPI_FUNCTION("isRTL", IsRTLCharacterAddon),
        DECLARE_NAPI_FUNCTION("isIdeograph", IsIdeoGraphicAddon),
        DECLARE_NAPI_FUNCTION("isLetter", IsLetterAddon),
        DECLARE_NAPI_FUNCTION("isLowerCase", IsLowerCaseAddon),
        DECLARE_NAPI_FUNCTION("isUpperCase", IsUpperCaseAddon),
        DECLARE_NAPI_FUNCTION("getType", GetTypeAddon)
    };

    napi_value constructor = nullptr;
    status = napi_define_class(env, "Character", NAPI_AUTO_LENGTH, ObjectConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        LOGE("Define class failed when InitCharacter");
        return nullptr;
    }

    status = napi_set_named_property(env, exports, "Character", constructor);
    if (status != napi_ok) {
        LOGE("Set property failed when InitCharacter");
        return nullptr;
    }
    return exports;
}

napi_value I18nAddon::ObjectConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv[0];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    std::unique_ptr<I18nAddon> obj = nullptr;
    obj = std::make_unique<I18nAddon>();
    status =
        napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()), I18nAddon::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("Wrap I18nAddon failed");
        return nullptr;
    }
    obj.release();
    return thisVar;
}

napi_value I18nAddon::GetI18nNormalizerInstance(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        LOGE("Failed to get parameter of Normalizer.createInstance");
        return nullptr;
    }

    napi_value constructor = nullptr;
    status = napi_get_reference_value(env, *g_normalizerConstructor, &constructor);
    if (status != napi_ok) {
        LOGE("Failed to create reference of normalizer Constructor");
        return nullptr;
    }

    napi_value result = nullptr;
    status = napi_new_instance(env, constructor, argc, argv, &result);
    if (status != napi_ok) {
        LOGE("create normalizer instance failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::InitI18nNormalizer(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("normalize", Normalize)
    };
    napi_value constructor = nullptr;
    napi_status status = napi_define_class(env, "Normalizer", NAPI_AUTO_LENGTH, I18nNormalizerConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        LOGE("Failed to define class Normalizer at Init");
        return nullptr;
    }
    g_normalizerConstructor = new (std::nothrow) napi_ref;
    if (!g_normalizerConstructor) {
        LOGE("Failed to create Normalizer ref at init");
        return nullptr;
    }
    status = napi_create_reference(env, constructor, 1, g_normalizerConstructor);
    if (status != napi_ok) {
        LOGE("Failed to create reference g_normalizerConstructor at init.");
        return nullptr;
    }
    return exports;
}

napi_value I18nAddon::I18nNormalizerConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    if (argc == 0) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, true);
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);

    if (valueType != napi_valuetype::napi_number) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, true);
    }
    int32_t normalizerMode;
    status = napi_get_value_int32(env, argv[0], &normalizerMode);
    if (status != napi_ok) {
        return nullptr;
    }
    if (normalizerMode != NORMALIZER_MODE_NFC && normalizerMode != NORMALIZER_MODE_NFD &&
        normalizerMode != NORMALIZER_MODE_NFKC && normalizerMode != NORMALIZER_MODE_NFKD) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, true);
    }

    std::unique_ptr<I18nAddon> obj = std::make_unique<I18nAddon>();
    status =
        napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()), I18nAddon::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        return nullptr;
    }
    I18nNormalizerMode mode = I18nNormalizerMode(normalizerMode);
    I18nErrorCode errorCode = I18nErrorCode::SUCCESS;
    obj->normalizer_ = std::make_unique<I18nNormalizer>(mode, errorCode);
    if (errorCode != I18nErrorCode::SUCCESS || !obj->normalizer_) {
        return nullptr;
    }
    obj.release();
    return thisVar;
}

napi_value I18nAddon::Normalize(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    if (argc == 0) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, true);
    }
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        LOGE("Invalid parameter type");
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, true);
    }
    int32_t code = 0;
    std::string text = NAPIUtils::GetString(env, argv[0], code);
    if (code != 0) {
        return nullptr;
    }

    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->normalizer_ == nullptr) {
        LOGE("Get Normalizer object failed");
        return nullptr;
    }
    I18nErrorCode errorCode = I18nErrorCode::SUCCESS;
    std::string normalizedText = obj->normalizer_->Normalize(text.c_str(), static_cast<int32_t>(text.length()),
        errorCode);
    if (errorCode != I18nErrorCode::SUCCESS) {
        return nullptr;
    }
    napi_value result = nullptr;
    status = napi_create_string_utf8(env, normalizedText.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("Create result failed");
        return nullptr;
    }
    return result;
}

napi_value Init(napi_env env, napi_value exports)
{
    napi_value val = I18nAddon::Init(env, exports);
    val = I18nAddon::InitPhoneNumberFormat(env, val);
    val = I18nAddon::InitBreakIterator(env, val);
    val = I18nAddon::InitI18nCalendar(env, val);
    val = I18nAddon::InitIndexUtil(env, val);
    val = I18nAddon::InitI18nTimeZone(env, val);
    val = I18nAddon::InitTransliterator(env, val);
    val = I18nAddon::InitCharacter(env, val);
    val = I18nAddon::InitI18nNormalizer(env, val);
    return val;
}

static napi_module g_i18nModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "i18n",
    .nm_priv = nullptr,
    .reserved = { 0 }
};

#ifdef ANDROID_PLATFORM
static void I18NPluginJniRegister()
{
    const char className[] = "ohos.ace.plugin.i18nplugin.I18NPlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&Plugin::I18NPluginJni::Register, className);
}
#endif

extern "C" __attribute__((constructor)) void I18nRegister()
{
#ifdef ANDROID_PLATFORM
    I18NPluginJniRegister();
#else
    InitIcuData();
#endif
    napi_module_register(&g_i18nModule);
}
} // namespace I18n
} // namespace Global
} // namespace OHOS
