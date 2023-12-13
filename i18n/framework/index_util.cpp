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
#include "index_util.h"

#include "locale_config.h"
#include "locid.h"
#include "string"
#include "unistr.h"
#include "utypes.h"
#include "vector"

namespace OHOS {
namespace Global {
namespace I18n {
IndexUtil::IndexUtil(const std::string &localeTag)
{
    UErrorCode status = U_ZERO_ERROR;
    if (localeTag == "") {
        icu::Locale locale(LocaleConfig::GetSystemLocale().c_str());
        index = std::make_unique<icu::AlphabeticIndex>(locale, status);
    } else {
        icu::Locale locale(localeTag.c_str());
        index = std::make_unique<icu::AlphabeticIndex>(locale, status);
    }
    if (U_SUCCESS(status)) {
        createSuccess = true;
    }
}

IndexUtil::~IndexUtil()
{
}

std::vector<std::string> IndexUtil::GetIndexList()
{
    UErrorCode status = U_ZERO_ERROR;
    std::vector<std::string> indexList;
    if (!createSuccess) {
        return indexList;
    }
    index->resetBucketIterator(status);
    while (index->nextBucket(status)) {
        if (U_SUCCESS(status)) {
            icu::UnicodeString unicodeString = index->getBucketLabel();
            std::string label;
            unicodeString.toUTF8String(label);
            indexList.push_back(label);
        }
    }
    return indexList;
}

void IndexUtil::AddLocale(const std::string &localeTag)
{
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale(localeTag.c_str());
    if (createSuccess) {
        index->addLabels(locale, status);
    }
}

std::string IndexUtil::GetIndex(const std::string &String)
{
    if (!createSuccess) {
        return "";
    }
    UErrorCode status = U_ZERO_ERROR;
    icu::UnicodeString unicodeString(String.c_str());
    int32_t bucketNumber = index->getBucketIndex(unicodeString, status);
    if (!U_SUCCESS(status)) {
        return "";
    }
    index->resetBucketIterator(status);
    for (int32_t i = 0; i <= bucketNumber; i++) {
        index->nextBucket(status);
        if (!U_SUCCESS(status)) {
            return "";
        }
    }
    icu::UnicodeString label = index->getBucketLabel();
    std::string result;
    label.toUTF8String(result);
    return result;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS