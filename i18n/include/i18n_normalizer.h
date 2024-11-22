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

#ifndef OHOS_GLOBAL_I18N_I18N_NORMALIZER_H
#define OHOS_GLOBAL_I18N_I18N_NORMALIZER_H

#include "i18n_types.h"
#include "normalizer2.h"

namespace OHOS {
namespace Global {
namespace I18n {
class I18nNormalizer {
public:
    I18nNormalizer(I18nNormalizerMode mode, I18nErrorCode &errorCode);
    ~I18nNormalizer();
    std::string Normalize(const char *text, int32_t length, I18nErrorCode &errorCode);

private:
    const icu::Normalizer2 *normalizer = nullptr;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif