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

#include "common_event_data.h"

#include "common_event.h"

namespace OHOS {
namespace Plugin {
CommonEventData::CommonEventData(
    const std::string& event, const std::string& bunduleName, int code, const std::string& data)
    : event_(event), bunduleName_(bunduleName), code_(code), data_(data)
{}

std::string CommonEventData::GetEvent() const
{
    return event_;
}

void CommonEventData::SetEvent(const std::string& event)
{
    event_ = event;
}

std::string CommonEventData::GetBunduleName() const
{
    return bunduleName_;
}

void CommonEventData::SetBunduleName(const std::string& bundleName)
{
    bunduleName_ = bundleName;
}

int CommonEventData::GetCode() const
{
    return code_;
}

void CommonEventData::SetCode(int code)
{
    code_ = code;
}

std::string CommonEventData::GetData() const
{
    return data_;
}

void CommonEventData::SetData(const std::string& data)
{
    data_ = data;
}
} // namespace Plugin
} // namespace OHOS