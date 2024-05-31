/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "i18n_normalizer.h"

namespace OHOS {
namespace Global {
namespace I18n {
I18nNormalizer::I18nNormalizer(I18nNormalizerMode mode, I18nErrorCode &errorCode)
{
    UErrorCode status = U_ZERO_ERROR;
    if (mode == I18nNormalizerMode::NFC) {
        normalizer = icu::Normalizer2::getNFCInstance(status);
    } else if (mode == I18nNormalizerMode::NFD) {
        normalizer = icu::Normalizer2::getNFDInstance(status);
    } else if (mode == I18nNormalizerMode::NFKC) {
        normalizer = icu::Normalizer2::getNFKCInstance(status);
    } else {
        // mode == I18nNormalizerMode::NFKD
        normalizer = icu::Normalizer2::getNFKDInstance(status);
    }
    if (U_FAILURE(status)) {
        errorCode = I18nErrorCode::FAILED;
    }
}

I18nNormalizer::~I18nNormalizer()
{
}

std::string I18nNormalizer::Normalize(const char *text, int32_t length, I18nErrorCode &errorCode)
{
    UErrorCode status = U_ZERO_ERROR;
    icu::UnicodeString input(text, length);
    icu::UnicodeString output = normalizer->normalize(input, status);
    if (U_FAILURE(status)) {
        errorCode = I18nErrorCode::FAILED;
        return "";
    }
    std::string result;
    output.toUTF8String(result);
    return result;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS