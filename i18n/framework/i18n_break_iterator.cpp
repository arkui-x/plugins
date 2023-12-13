/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "i18n_break_iterator.h"

#include "brkiter.h"
#include "string"
#include "utypes.h"

namespace OHOS {
namespace Global {
namespace I18n {
I18nBreakIterator::I18nBreakIterator(std::string localeTag)
{
    UErrorCode status = U_ZERO_ERROR;
    iter = icu::BreakIterator::createLineInstance(localeTag.c_str(), status);
    if (!U_SUCCESS(status)) {
        if (iter != nullptr) {
            delete iter;
        }
        iter = nullptr;
    }
}

I18nBreakIterator::~I18nBreakIterator()
{
    if (iter != nullptr) {
        delete iter;
    }
}

int32_t I18nBreakIterator::Current()
{
    if (iter != nullptr) {
        return iter->current();
    }
    return OFF_BOUND;
}

int32_t I18nBreakIterator::First()
{
    if (iter != nullptr) {
        return iter->first();
    }
    return OFF_BOUND;
}

int32_t I18nBreakIterator::Last()
{
    if (iter != nullptr) {
        return iter->last();
    }
    return OFF_BOUND;
}

int32_t I18nBreakIterator::Previous()
{
    if (iter != nullptr) {
        return iter->previous();
    }
    return OFF_BOUND;
}

int32_t I18nBreakIterator::Next(int32_t number)
{
    if (iter != nullptr) {
        return iter->next(number);
    }
    return OFF_BOUND;
}

int32_t I18nBreakIterator::Next()
{
    if (iter != nullptr) {
        return iter->next();
    }
    return OFF_BOUND;
}

int32_t I18nBreakIterator::Following(int32_t offset)
{
    if (iter != nullptr) {
        return iter->following(offset);
    }
    return OFF_BOUND;
}

void I18nBreakIterator::SetText(const char *text)
{
    if (iter != nullptr) {
        ftext = text;
        iter->setText(ftext);
    }
}

void I18nBreakIterator::GetText(std::string &str)
{
    if (iter != nullptr) {
        ftext.toUTF8String(str);
    }
}

bool I18nBreakIterator::IsBoundary(int32_t offset)
{
    if (iter != nullptr) {
        return iter->isBoundary(offset);
    }
    return false;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS