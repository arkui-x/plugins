/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "entity_recognizer.h"

#include "log.h"

namespace OHOS {
namespace Global {
namespace I18n {
EntityRecognizer::EntityRecognizer(icu::Locale& locale)
{
    std::string region = locale.getCountry();
    std::string language = locale.getLanguage();
    phoneNumberMatched = new PhoneNumberMatched(region);
    if (phoneNumberMatched == nullptr) {
        LOGE("PhoneNumberMatched construct failed.");
    }
    dateTimeMatched = new DateTimeMatched(language);
    if (dateTimeMatched == nullptr) {
        LOGE("DateTimeMatched construct failed.");
    }
}

EntityRecognizer::~EntityRecognizer()
{
    if (phoneNumberMatched != nullptr) {
        delete phoneNumberMatched;
    }
    if (dateTimeMatched != nullptr) {
        delete dateTimeMatched;
    }
}

std::vector<std::vector<int>> EntityRecognizer::FindEntityInfo(std::string& message)
{
    icu::UnicodeString messageStr = message.c_str();
    messageStr = ConvertQanChar(messageStr);
    std::vector<std::vector<int>> EntityInfo;
    if (phoneNumberMatched == nullptr) {
        LOGE("FindEntityInfo failed because phoneNumberMatched is nullptr.");
        return EntityInfo;
    }
    std::vector<int> phoneNumberInfo = phoneNumberMatched->GetMatchedPhoneNumber(messageStr);
    EntityInfo.push_back(phoneNumberInfo);
    if (dateTimeMatched == nullptr) {
        LOGE("FindEntityInfo failed because dateTimeMatched is nullptr.");
        return EntityInfo;
    }
    std::vector<int> dateTimeInfo = dateTimeMatched->GetMatchedDateTime(messageStr);
    EntityInfo.push_back(dateTimeInfo);
    return EntityInfo;
}

icu::UnicodeString EntityRecognizer::ConvertQanChar(icu::UnicodeString& instr)
{
    icu::UnicodeString result = "";
    icu::UnicodeString fwchstrFirst = "：／．＼∕，！（）？﹡；：﹣—－【】－＋＝｛％｝１２３４５６７８９０ａｂｃｄｅｆｇｈｉ";
    icu::UnicodeString fwchstrSecond = "ｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺ";
    icu::UnicodeString fwchstr = fwchstrFirst + fwchstrSecond;
    icu::UnicodeString hwchstrFirst = ":/.\\/,!()?*;:---[]-+={%}1234567890abcdefghi";
    icu::UnicodeString hwchstrSecond = "jklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    icu::UnicodeString hwchstr = hwchstrFirst + hwchstrSecond;
    int len = instr.length();
    for (int i = 0; i < len; i++) {
        int index = fwchstr.indexOf(instr[i]);
        if (index == -1) {
            result += instr[i];
        } else {
            result += hwchstr[index];
        }
    }
    return result;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS