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
#ifndef OHOS_GLOBAL_I18N_PHONE_NUMBER_FORMAT_H
#define OHOS_GLOBAL_I18N_PHONE_NUMBER_FORMAT_H

#include <map>
#include <set>
#include <string>

#include "memory"
#include "cpp/src/phonenumbers/phonenumberutil.h"

namespace OHOS {
namespace Global {
namespace I18n {
using i18n::phonenumbers::PhoneNumberUtil;
using ExposeLocationName = void (*)(const char*, const char*, char*);
using GetBlockedRegionName = bool (*)(const char*, const char*);
using GetBlockedPhoneLocationName = int (*)(const char*, const char*, char*);
using GetReplacedPhoneLocationName = void (*)(const char*, const char*, char*);

class PhoneNumberFormat {
public:
    PhoneNumberFormat(const std::string &countryTag, const std::map<std::string, std::string> &options);
    virtual ~PhoneNumberFormat();
    bool isValidPhoneNumber(const std::string &number) const;
    std::string format(const std::string &number) const;
    static std::unique_ptr<PhoneNumberFormat> CreateInstance(const std::string &countryTag,
                                                             const std::map<std::string, std::string> &options);

private:
    PhoneNumberUtil* GetPhoneNumberUtil();
    PhoneNumberUtil *util;
    std::string country;
    PhoneNumberUtil::PhoneNumberFormat phoneNumberFormat;
    void* g_dynamicHandler = nullptr;
    ExposeLocationName g_func = nullptr;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif
