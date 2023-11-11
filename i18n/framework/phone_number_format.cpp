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
#include "phone_number_format.h"

#include <dlfcn.h>

#include "localebuilder.h"
#include "locale_config.h"
#include "locid.h"
#include "map"
#include "new"
#include "set"
#include "string"
#include "utility"


namespace OHOS {
namespace Global {
namespace I18n {
const int RECV_CHAR_LEN = 128;
using i18n::phonenumbers::PhoneNumberUtil;

PhoneNumberFormat::PhoneNumberFormat(const std::string &countryTag,
                                     const std::map<std::string, std::string> &options)
{
    util = PhoneNumberUtil::GetInstance();
    if (LocaleConfig::IsValidRegion(countryTag)) {
        country = countryTag;
    } else {
        icu::Locale locale = icu::Locale::createFromName(countryTag.c_str());
        country = locale.getCountry();
    }
    std::string type = "";
    auto search = options.find("type");
    if (search != options.end()) {
        type = search->second;
    }

    std::map<std::string, PhoneNumberUtil::PhoneNumberFormat> type2PhoneNumberFormat = {
        {"E164", PhoneNumberUtil::PhoneNumberFormat::E164},
        {"RFC3966", PhoneNumberUtil::PhoneNumberFormat::RFC3966},
        {"INTERNATIONAL", PhoneNumberUtil::PhoneNumberFormat::INTERNATIONAL},
        {"NATIONAL", PhoneNumberUtil::PhoneNumberFormat::NATIONAL}
    };

    std::set<std::string> validType = {"E164", "RFC3966", "INTERNATIONAL", "NATIONAL"};
    if (validType.find(type) != validType.end()) {
        phoneNumberFormat = type2PhoneNumberFormat[type];
    } else {
        phoneNumberFormat = PhoneNumberUtil::PhoneNumberFormat::NATIONAL;
    }
}

PhoneNumberFormat::~PhoneNumberFormat()
{
    if (g_dynamicHandler != NULL) {
        dlclose(g_dynamicHandler);
        g_dynamicHandler = nullptr;
        g_func = nullptr;
    }
}

std::unique_ptr<PhoneNumberFormat> PhoneNumberFormat::CreateInstance(const std::string &countryTag,
    const std::map<std::string, std::string> &options)
{
    std::unique_ptr<PhoneNumberFormat> phoneNumberFormat = std::make_unique<PhoneNumberFormat>(countryTag, options);
    if (phoneNumberFormat->GetPhoneNumberUtil() == nullptr) {
        return nullptr;
    }
    return phoneNumberFormat;
}

PhoneNumberUtil* PhoneNumberFormat::GetPhoneNumberUtil()
{
    return util;
}

bool PhoneNumberFormat::isValidPhoneNumber(const std::string &number) const
{
    i18n::phonenumbers::PhoneNumber phoneNumber;
    PhoneNumberUtil::ErrorType type = util->Parse(number, country, &phoneNumber);
    if (type != PhoneNumberUtil::ErrorType::NO_PARSING_ERROR) {
        return false;
    }
    return util->IsValidNumber(phoneNumber);
}

std::string PhoneNumberFormat::format(const std::string &number) const
{
    i18n::phonenumbers::PhoneNumber phoneNumber;
    PhoneNumberUtil::ErrorType type = util->Parse(number, country, &phoneNumber);
    if (type != PhoneNumberUtil::ErrorType::NO_PARSING_ERROR) {
        return "";
    }
    std::string formatted_number;
    util->Format(phoneNumber, phoneNumberFormat, &formatted_number);
    return formatted_number;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS
