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

#include "web_data_base_ios.h"
#include "AceWebControllerBridge.h"
using namespace OHOS::NWebError;

namespace OHOS::Plugin {
bool WebDataBaseIOS::ExistHttpAuthCredentials()
{
    return existHttpAuthCredentialsOC();
}

ErrCode WebDataBaseIOS::DeleteHttpAuthCredentials()
{
    bool result = deleteHttpAuthCredentialsOC();
    if (!result) {
        return INIT_ERROR;
    }
    return NO_ERROR;
}

ErrCode WebDataBaseIOS::SaveHttpAuthCredentials(const std::string& host, const std::string& realm,
    const std::string& username, const char* password)
{
    bool result = saveHttpAuthCredentialsOC(host, realm, username, password);
    if (!result) {
        return INIT_ERROR;
    }
    return NO_ERROR;
}

ErrCode WebDataBaseIOS::GetHttpAuthCredentials(const std::string& host, const std::string& realm,
    std::string& username, char* password, uint32_t passwordSize)
{
    bool result = getHttpAuthCredentialsOC(host, realm, username, password, passwordSize);
    if (!result) {
        return INIT_ERROR;
    }
    return NO_ERROR;
}
} // namespace OHOS::Plugin