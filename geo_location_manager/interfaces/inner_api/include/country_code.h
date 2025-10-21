/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef COUNTRY_CODE_H
#define COUNTRY_CODE_H

#include <parcel.h>
#include <string>

#include "constant_definition.h"
#include "country_code.h"
#include "string_ex.h"

namespace OHOS {
namespace Location {
class CountryCode : public Parcelable {
public:
    CountryCode()
    {
        countryCodeStr_ = "";
        countryCodeType_ = COUNTRY_CODE_FROM_LOCALE;
    }

    explicit CountryCode(CountryCode& country)
    {
        SetCountryCodeStr(country.GetCountryCodeStr());
        SetCountryCodeType(country.GetCountryCodeType());
    }

    ~CountryCode() override = default;

    inline bool IsSame(CountryCode& country) const
    {
        return (countryCodeStr_ == country.GetCountryCodeStr()) &&
            (countryCodeType_ == country.GetCountryCodeType());
    }

    inline bool IsMoreReliable(int type) const
    {
        return (countryCodeType_ > type);
    }

    inline std::string GetCountryCodeStr() const
    {
        return countryCodeStr_;
    }

    inline void SetCountryCodeStr(std::string country)
    {
        countryCodeStr_ = country;
    }

    inline int GetCountryCodeType() const
    {
        return countryCodeType_;
    }

    inline void SetCountryCodeType(int type)
    {
        countryCodeType_ = type;
    }

    bool Marshalling(Parcel& parcel) const override
    {
        return true;
    }

    std::string ToString()
    {
        std::string str = "countryCodeStr_ : " + countryCodeStr_ +
            ", countryCodeType_ : " + std::to_string(countryCodeType_);
        return str;
    }

private:
    std::string countryCodeStr_;
    int countryCodeType_;
};
} // namespace Location
} // namespace OHOS
#endif // COUNTRY_CODE_H